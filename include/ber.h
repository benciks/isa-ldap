#ifndef BER_H
#define BER_H

#include <iostream>
#include <vector>

struct EqType {
  std::vector<unsigned char> type;
  std::vector<unsigned char> value;
};

struct SubsType {
  std::vector<unsigned char> type;
  std::vector<unsigned char> initial;
  std::vector<unsigned char> any;
  std::vector<unsigned char> final;
};

enum FilterType {
  EqualityMatch = 0xA3,
  SubstringMatch = 0xA4,
  AND = 0xA0,
  OR = 0xA1,
  NOT = 0xA2,
};

struct Filter {
  FilterType type;
  EqType equalityMatch;
  SubsType substringMatch;
  std::vector<Filter> filters;
};

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
  bool getFilter(Filter &filter);
  bool isEnd();

private:
  std::vector<unsigned char> &buffer;
  size_t pos;
};

#endif