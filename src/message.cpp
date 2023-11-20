#include <sys/socket.h>

#include "../include/message.h"
#include "../include/search.h"

// For each message, we need to parse the message ID and the protocol op
void LDAPMessage::init() {
  std::vector<unsigned char> tmpSeq;
  parser.getSequence(tmpSeq);

  parser.getInteger(messageID);

  unsigned char length;
  parser.getTag(protocolOp);
  parser.getLength(length);
}

void Bind::parse() {
  std::cout << "Bind request <-" << std::endl;
  unsigned char version;
  parser.getInteger(version);

  std::cout << "Version: " << (int)version << std::endl;

  std::vector<unsigned char> name;
  parser.getOctetString(name);
  std::cout << "Name: " << std::string(name.begin(), name.end()) << std::endl;
}

void Bind::respond(int fd, std::string inputFile) {
  std::cout << "Bind response ->" << std::endl;
  std::vector<unsigned char> response;

  // Add the sequence
  response.push_back(0x30);

  // Add the length
  response.push_back(0x0C);

  // Add the message ID
  response.push_back(0x02);
  response.push_back(0x01);
  std::cout << "Message ID: " << (int)messageID << std::endl;
  response.push_back(messageID);

  // Add the protocol op
  response.push_back(0x61);

  // Add the length
  response.push_back(0x07);

  // Add the bind response
  response.push_back(0x0A);
  response.push_back(0x01);
  response.push_back(0x00);

  // Add the matched DN
  response.push_back(0x04);
  response.push_back(0x00);

  // Add the diagnostic message
  response.push_back(0x04);
  response.push_back(0x00);

  // Send the response
  send(fd, response.data(), response.size(), 0);
}

void Search::parse() {
  std::cout << "Search request <-" << std::endl;
  parser.getOctetString(baseObject);
  parser.getEnum(scope);
  parser.getEnum(derefAliases);
  parser.getInteger(sizeLimit);
  parser.getInteger(timeLimit);
  parser.getBool(typesOnly);
  parser.getFilter(filter);
}

void Search::addAttribute(std::vector<unsigned char> &message,
                          const std::string &type, const std::string &value) {
  // Start the attribute SEQUENCE
  message.push_back(0x30);
  int attributeStartPos = message.size();
  message.push_back(0x00); // Placeholder for length

  // Attribute type
  message.push_back(0x04);
  message.push_back(static_cast<unsigned char>(type.size())); // length
  for (unsigned char c : type) {
    message.push_back(c);
  }

  // Attribute value set
  message.push_back(0x31);
  int setValueStartPos = message.size();
  message.push_back(0x00); // Placeholder for length

  // Attribute value
  message.push_back(0x04);

  // print the value size
  message.push_back(static_cast<unsigned char>(value.size())); // length
  for (unsigned char c : value) {
    message.push_back(c);
  }

  // update set len
  message[setValueStartPos] =
      static_cast<unsigned char>(message.size() - setValueStartPos - 1);

  // update attr len
  message[attributeStartPos] =
      static_cast<unsigned char>(message.size() - attributeStartPos - 1);
}

void Search::sendSearchResEntry(const FileEntry &entry, int fd,
                                unsigned char messageId) {
  std::vector<unsigned char> message;

  // LDAPMessage sequence
  message.push_back(0x30);
  message.push_back(0x00); // Placeholder for length

  // Message ID
  message.push_back(0x02);
  message.push_back(0x01);
  message.push_back(messageID);

  // ProtocolOp
  message.push_back(0x64);
  int searchResEntryStartPos = message.size();
  message.push_back(0x00); // Placeholder for length

  // ObjectName (DN)
  std::string uidString(entry.uid.begin(), entry.uid.end());
  std::string dn = "uid=" + uidString;

  // Append the DN
  message.push_back(0x04);
  message.push_back(static_cast<unsigned char>(dn.size()));
  for (char c : dn) {
    message.push_back(c);
  }

  // Attributes SEQUENCE
  message.push_back(0x30);
  int attributesSeqStartPos = message.size();
  message.push_back(0x00);

  // Add the attributes
  addAttribute(message, "cn", entry.cn);
  addAttribute(message, "mail", entry.mail);

  // update sequence len
  message[attributesSeqStartPos] =
      static_cast<unsigned char>(message.size() - attributesSeqStartPos - 1);

  // update protocol op len
  message[searchResEntryStartPos] =
      static_cast<unsigned char>(message.size() - searchResEntryStartPos - 1);

  // update message len
  message[1] = static_cast<unsigned char>(message.size() - 2);

  // Send the message
  send(fd, message.data(), message.size(), 0);
}

void Search::sendSearchResDone(int fd, unsigned char messageId) {
  // Add the search result done message
  std::vector<unsigned char> done;

  // Sequence
  done.push_back(0x30);
  done.push_back(0x0c);

  // messageID
  done.push_back(0x02);
  done.push_back(0x01);
  done.push_back(messageID);

  // protocolOp
  done.push_back(0x65);
  done.push_back(0x07);

  // resultCode
  done.push_back(0x0a);
  done.push_back(0x01);
  done.push_back(0x00);

  // matchedDN
  done.push_back(0x04);
  done.push_back(0x00);

  // errorMessage
  done.push_back(0x04);
  done.push_back(0x00);
  send(fd, done.data(), done.size(), 0);
}

void Search::respond(int fd, std::string inputFile) {
  std::cout << "Search response ->" << std::endl;

  auto entries = readCSV(inputFile);
  size_t count = 0;

  // Apply filter to each entry
  for (const auto &entry : entries) {
    if (sizeLimit != 0 && count >= sizeLimit) {
      break;
    }
    if (filterEntry(filter, entry)) {
      sendSearchResEntry(entry, fd, messageID);
      count++;
    }
  }

  sendSearchResDone(fd, messageID);
}

void Unbind::parse() { std::cout << "Unbind request <-" << std::endl; }

void Unbind::respond(int fd, std::string inputFile) {
  std::cout << "Unbind response ->" << std::endl;
};

// Determine the type of request and create the appropriate object
std::unique_ptr<LDAPMessage>
createLDAPRequest(std::vector<unsigned char> &buffer) {
  unsigned char protocolOp = buffer[5];

  switch (protocolOp) {
  case 0x60:
    return std::make_unique<Bind>(buffer);
  case 0x63:
    return std::make_unique<Search>(buffer);
  case 0x42:
    return std::make_unique<Unbind>(buffer);
  default:
    std::cerr << "Unknown protocol op: " << std::hex << (int)protocolOp
              << std::endl;
    return nullptr;
  }
}
