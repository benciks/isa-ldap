#ifndef BER_H
#define BER_H

#include "../include/search.h"
#include <iostream>
#include <vector>

class BERParser {
public:
  BERParser(std::vector<unsigned char> &buffer);
  virtual ~BERParser(){};

  bool getTag(unsigned char &tag);
  bool getLength(unsigned char &length);
  bool getInteger(unsigned char &integer);
  bool getBool(unsigned char &boolean);
  bool getEnum(unsigned char &enumeration);
  bool getOctetString(std::vector<unsigned char> &octetString);
  bool getSequence(std::vector<unsigned char> &sequence);
  bool getSubstringFilter(SubsType &subsMatch);
  bool getFilter(Filter &filter);
  bool isEnd();

private:
  std::vector<unsigned char> &buffer;
  size_t pos;
};

#endif