#ifndef REQUEST_H
#define REQUEST_H

#include "../include/ber.h"
#include "../include/search.h"
#include <iostream>
#include <string>

enum class LDAPRequestType {
  Bind,
  Search,
  Unbind,
};

class LDAPMessage {
public:
  LDAPMessage(std::vector<unsigned char> &buffer)
      : buffer(buffer), parser(buffer) {
    init();
  }
  virtual ~LDAPMessage() {}
  virtual void parse() = 0;
  virtual void respond(int fd, std::string inputFile) = 0;

protected:
  unsigned char messageID;
  unsigned char protocolOp;
  std::vector<unsigned char> &buffer;
  BERParser parser;

  void init();
};

// TODO: If we receive openssl authentication, we need to respond with a
// protcolError
class Bind : public LDAPMessage {
public:
  Bind(std::vector<unsigned char> &buffer) : LDAPMessage(buffer) {}
  void parse() override;
  void respond(int fd, std::string inputFile) override;

private:
};

class Search : public LDAPMessage {
public:
  Search(std::vector<unsigned char> &buffer) : LDAPMessage(buffer) {}
  void parse() override;
  void respond(int fd, std::string inputFile) override;

private:
  std::vector<unsigned char> baseObject;
  unsigned char scope;
  unsigned char derefAliases;
  unsigned char sizeLimit = 0;
  unsigned char timeLimit;
  unsigned char typesOnly;
  Filter filter;

  void addAttribute(std::vector<unsigned char> &response,
                    const std::string &type, const std::string &value);
  void sendSearchResEntry(const FileEntry &entry, int fd,
                          unsigned char messageID);
  void sendSearchResDone(int fd, unsigned char messageID);
};

class Unbind : public LDAPMessage {
public:
  Unbind(std::vector<unsigned char> &buffer) : LDAPMessage(buffer) {}
  void parse() override;
  void respond(int fd, std::string inputFile) override;

private:
};

std::unique_ptr<LDAPMessage>
createLDAPRequest(std::vector<unsigned char> &buffer);

#endif