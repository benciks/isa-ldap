/**
 * @file ber.h
 * @brief This file contains the BERParser class
 * @author Simon Bencik <xbenci01>
 */
#ifndef BER_H
#define BER_H

#include "../include/search.h"
#include <iostream>
#include <vector>

/**
 * @class BERParser
 * @brief Parses BER encoded data
 */
class BERParser {
public:
  BERParser(std::vector<unsigned char> &buffer);
  virtual ~BERParser(){};

  /**
   * @brief Get the tag from the buffer
   * @param tag The tag to be returned
   */
  bool getTag(unsigned char &tag);

  /**
   * @brief Get the length from the buffer
   * @param length The length to be returned
   */
  bool getLength(unsigned char &length);

  /**
   * @brief Get the integer from the buffer
   * @param integer The integer to be returned
   */
  bool getInteger(unsigned char &integer);

  /**
   * @brief Get the boolean from the buffer
   * @param boolean The boolean to be returned
   */
  bool getBool(unsigned char &boolean);

  /**
   * @brief Get the enumeration from the buffer
   * @param enumeration The enumeration to be returned
   */
  bool getEnum(unsigned char &enumeration);

  /**
   * @brief Get the octet string from the buffer
   * @param octetString The octet string to be returned
   */
  bool getOctetString(std::vector<unsigned char> &octetString);

  /**
   * @brief Get the sequence from the buffer
   * @param sequence The sequence to be returned
   */
  bool getSequence(std::vector<unsigned char> &sequence);

  /**
   * @brief Get the substring filter from the buffer
   * @param subsMatch The substring filter to be returned
   */
  bool getSubstringFilter(SubsType &subsMatch);

  /**
   * @brief Get the filter from the buffer
   * @param filter The filter to be returned
   */
  bool getFilter(Filter &filter);

  /**
   * @brief Check if the end of the buffer has been reached
   */
  bool isEnd();

private:
  /**
   * @brief The buffer to be parsed
   */
  std::vector<unsigned char> &buffer;

  /**
   * @brief The current position in the buffer
   */
  size_t pos;
};

#endif