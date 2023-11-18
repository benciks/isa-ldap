#include <sys/socket.h>

#include "file.h"
#include "message.h"

// TODO: Make this part of the LDAPMessage class

void addAttribute(std::vector<unsigned char> &message, const std::string &type,
                  const std::string &value) {
  // Start the attribute SEQUENCE
  message.push_back(0x30); // SEQUENCE tag
  int attributeStartPos = message.size();
  message.push_back(0x00); // Placeholder for length

  // Attribute type (Octet String)
  message.push_back(0x04); // Octet String tag
  message.push_back(static_cast<unsigned char>(type.size())); // length
  for (char c : type) {
    message.push_back(c);
  }

  // Start the SET for attribute values
  message.push_back(0x31); // SET tag
  int setValueStartPos = message.size();
  message.push_back(0x00); // Placeholder for length

  // Attribute value (Octet String)
  message.push_back(0x04); // Octet String tag
  message.push_back(static_cast<unsigned char>(value.size())); // length
  for (char c : value) {
    message.push_back(c);
  }

  // Update length for the SET
  message[setValueStartPos] =
      static_cast<unsigned char>(message.size() - setValueStartPos - 1);

  // Update length for the attribute SEQUENCE
  message[attributeStartPos] =
      static_cast<unsigned char>(message.size() - attributeStartPos - 1);
}

std::vector<unsigned char> buildSearchResEntry(const FileEntry &entry,
                                               unsigned char messageID,
                                               const std::string &baseDN) {
  std::vector<unsigned char> message;

  // Placeholder for the start of LDAPMessage SEQUENCE
  message.push_back(0x30);
  message.push_back(0x00); // Placeholder for length

  // Message ID
  message.push_back(0x02);      // INTEGER tag
  message.push_back(0x01);      // length
  message.push_back(messageID); // message ID value

  // ProtocolOp: searchResEntry (APPLICATION[4])
  message.push_back(0x64); // searchResEntry tag
  int searchResEntryStartPos = message.size();
  message.push_back(0x00); // Placeholder for length

  // ObjectName (DN)
  std::string dn = "cn=" + entry.cn + "," + baseDN;

  // Append the DN
  message.push_back(0x04);                                  // Octet String tag
  message.push_back(static_cast<unsigned char>(dn.size())); // length
  for (char c : dn) {
    message.push_back(c);
  }

  // Attributes SEQUENCE
  message.push_back(0x30); // SEQUENCE tag
  int attributesSeqStartPos = message.size();
  message.push_back(0x00); // Placeholder for length

  // Add attributes
  addAttribute(message, "cn", entry.cn);
  addAttribute(message, "uid", entry.uid);
  addAttribute(message, "mail", entry.mail);

  // Update length for the attributes SEQUENCE
  message[attributesSeqStartPos] =
      static_cast<unsigned char>(message.size() - attributesSeqStartPos - 1);

  // Update length for the searchResEntry
  message[searchResEntryStartPos] =
      static_cast<unsigned char>(message.size() - searchResEntryStartPos - 1);

  // Update length for the entire LDAPMessage
  message[1] = static_cast<unsigned char>(message.size() - 2);

  return message;
}

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

void Search::respond(int fd, std::string inputFile) {
  std::cout << "Search response ->" << std::endl;

  // EqualityMatch filter
  if (filter.type == FilterType::EqualityMatch) {
    std::string filterType(filter.attributes[0].type.begin(),
                           filter.attributes[0].type.end());
    if (filterType == "cn") {
      std::cout << "Filter type: cn" << std::endl;
      auto entries = readCSV(inputFile);
      std::string cnValue = std::string(filter.attributes[0].value.begin(),
                                        filter.attributes[0].value.end());

      auto result = searchByCN(entries, cnValue);

      for (const auto &entry : result) {
        std::vector res = buildSearchResEntry(
            entry, messageID,
            std::string(baseObject.begin(), baseObject.end()));
        send(fd, res.data(), res.size(), 0);
      }

      // Add the search result done message
      std::vector<unsigned char> done;
      done.push_back(0x30);
      done.push_back(0x0c);
      done.push_back(0x02);
      done.push_back(0x01);
      done.push_back(messageID);
      done.push_back(0x65);
      done.push_back(0x07);
      done.push_back(0x0a);
      done.push_back(0x01);
      done.push_back(0x00);
      done.push_back(0x04);
      done.push_back(0x00);
      done.push_back(0x04);
      done.push_back(0x00);
      send(fd, done.data(), done.size(), 0);
    }
  }
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
