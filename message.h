#ifndef REQUEST_H
#define REQUEST_H

#include "ber.h"
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
  unsigned char sizeLimit;
  unsigned char timeLimit;
  unsigned char typesOnly;
  Filter filter;
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