/**
 * @file message.h
 * @brief This file contains the LDAPMessage class and its subclasses
 * @author Simon Bencik <xbenci01>
 */
#ifndef REQUEST_H
#define REQUEST_H

#include "../include/ber.h"
#include "../include/search.h"
#include <iostream>
#include <memory>
#include <string>

/**
 * @enum LDAPRequestType
 * @brief The type of LDAP request
 *
 */
enum class LDAPRequestType {
  Bind,
  Search,
  Unbind,
};

/**
 * @class LDAPMessage
 * @brief The base class for all LDAP messages
 */
class LDAPMessage {
public:
  LDAPMessage(std::vector<unsigned char> &buffer) : parser(buffer) { init(); }
  virtual ~LDAPMessage() {}

  /**
   * @brief Get the message ID
   */
  virtual void parse() = 0;

  /**
   * @brief Respond to the LDAP message
   * @param fd The file descriptor to write to
   * @param inputFile The input file to read from
   */
  virtual void respond(int fd, std::string inputFile) = 0;

protected:
  /**
   * @brief The message ID
   */
  unsigned char messageID;
  /**
   * @brief The protocol op
   */
  unsigned char protocolOp;
  /**
   * @brief The BER parser instance
   */
  BERParser parser;

  /**
   * @brief Initialize the LDAP message
   */
  void init();
};

/**
 * @class Bind
 * @brief The Bind class for parsing requests and responding to them
 */
class Bind : public LDAPMessage {
public:
  Bind(std::vector<unsigned char> &buffer) : LDAPMessage(buffer) {}
  /**
   * @brief Parse the Bind request
   */
  void parse() override;
  /**
   * @brief Respond to the Bind request
   */
  void respond(int fd, std::string inputFile) override;

private:
};

/**
 * @class Search
 * @brief The Search class for parsing requests and responding to them
 */
class Search : public LDAPMessage {
public:
  Search(std::vector<unsigned char> &buffer) : LDAPMessage(buffer) {}
  /**
   * @brief Parse the Search request
   */
  void parse() override;
  /**
   * @brief Respond to the Search request
   */
  void respond(int fd, std::string inputFile) override;

private:
  /**
   * @brief The base object
   */
  std::vector<unsigned char> baseObject;
  /**
   * @brief The scope
   */
  unsigned char scope;
  /**
   * @brief The deref aliases
   */
  unsigned char derefAliases;
  /**
   * @brief The size limit
   */
  unsigned char sizeLimit = 0;
  /**
   * @brief The time limit
   */
  unsigned char timeLimit;
  /**
   * @brief The types only
   */
  unsigned char typesOnly;
  /**
   * @brief The filter
   */
  Filter filter;

  /**
   * @brief Add attribute to the response
   * @param response The response to add to
   * @param type The type of the attribute
   * @param value The value of the attribute
   */
  void addAttribute(std::vector<unsigned char> &response,
                    const std::string &type, const std::string &value);

  /**
   * @brief Send the search result entry
   * @param entry The entry to send
   * @param fd The file descriptor to write to
   */
  void sendSearchResEntry(const FileEntry &entry, int fd);
  /**
   * @brief Send the search result done
   * @param fd The file descriptor to write to
   */
  void sendSearchResDone(int fd, bool sizeLimitReached);
};

/**
 * @class Unbind
 * @brief The Unbind class for parsing requests and responding to them
 */
class Unbind : public LDAPMessage {
public:
  Unbind(std::vector<unsigned char> &buffer) : LDAPMessage(buffer) {}
  /**
   * @brief Parse the Unbind request
   */
  void parse() override;
  /**
   * @brief Respond to the Unbind request (This one is just to comply with
   * polymorphism)
   */
  void respond(int fd, std::string inputFile) override;

private:
};

/**
 * @brief Create an LDAP request from the buffer
 * @param buffer The buffer to create the request from
 * @return The LDAP request
 */
std::unique_ptr<LDAPMessage>
createLDAPRequest(std::vector<unsigned char> &buffer);

#endif