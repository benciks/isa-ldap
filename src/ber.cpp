#include "../include/ber.h"
#include <iostream>
#include <vector>

BERParser::BERParser(std::vector<unsigned char> &buffer)
    : buffer(buffer), pos(0) {
  if (buffer.size() < 2) {
    std::cout << buffer.size() << std::endl;
    std::cerr << "Buffer too small" << std::endl;
    return;
  }
}

bool BERParser::getTag(unsigned char &tag) {
  tag = buffer[pos++];
  return true;
}

bool BERParser::getLength(unsigned char &length) {
  unsigned char tmpLength = buffer[pos++];
  // Determine if the length is long form
  if (tmpLength & 0x80) {
    unsigned char lengthBytes = length & 0x7F;

    if (lengthBytes > 4) {
      std::cerr << "Length too long" << std::endl;
      return false;
    }

    // Construct longform length
    while (lengthBytes--) {
      length = (length << 8) | buffer[pos++];
    }

    // Print length
    std::cout << "Long form Length: " << std::hex << (int)length << std::endl;
    return true;
  }
  length = tmpLength;
  return true;
}

bool BERParser::getInteger(unsigned char &integer) {
  unsigned char tag;
  unsigned char length;
  if (!getTag(tag)) {
    return false;
  }

  if (tag != 0x02) {
    std::cerr << "Expected tag 0x02, got " << std::hex << (int)tag << std::endl;
    return false;
  }

  if (!getLength(length)) {
    return false;
  }

  integer = buffer[pos++];
  return true;
}

bool BERParser::getBool(unsigned char &boolean) {
  unsigned char tag;
  unsigned char length;
  if (!getTag(tag)) {
    return false;
  }

  if (tag != 0x01) {
    std::cerr << "Expected tag 0x01, got " << std::hex << (int)tag << std::endl;
    return false;
  }

  if (!getLength(length)) {
    return false;
  }

  boolean = buffer[pos++];
  return true;
}

bool BERParser::getEnum(unsigned char &enumeration) {
  unsigned char tag;
  unsigned char length;
  if (!getTag(tag)) {
    return false;
  }

  if (tag != 0x0A) {
    std::cerr << "Expected tag 0x0A, got " << std::hex << (int)tag << std::endl;
    return false;
  }

  if (!getLength(length)) {
    return false;
  }

  enumeration = buffer[pos++];
  return true;
}

bool BERParser::getOctetString(std::vector<unsigned char> &ostring) {
  unsigned char tag;
  unsigned char length;

  if (!getTag(tag)) {
    return false;
  }

  if (tag != 0x04) {
    std::cerr << "Expected tag 0x04, got " << std::hex << (int)tag << std::endl;
    return false;
  }

  if (!getLength(length)) {
    return false;
  }

  ostring = std::vector<unsigned char>(buffer.begin() + pos,
                                       buffer.begin() + pos + length);

  pos += length;

  return true;
}

bool BERParser::getSequence(std::vector<unsigned char> &sequence) {
  unsigned char tag;
  unsigned char length;

  if (!getTag(tag)) {
    return false;
  }

  if (tag != 0x30) {
    std::cerr << "Expected tag 0x30, got " << std::hex << (int)tag << std::endl;
    return false;
  }

  if (!getLength(length)) {
    return false;
  }

  sequence = std::vector<unsigned char>(buffer.begin() + pos,
                                        buffer.begin() + pos + length);
  // pos += length;
  return true;
}

bool BERParser::isEnd() { return pos == buffer.size(); }

bool BERParser::getFilter(Filter &filter) {
  unsigned char tag;
  unsigned char length;

  if (!getTag(tag)) {
    return false;
  }

  if (!getLength(length)) {
    return false;
  }

  filter.type = static_cast<FilterType>(tag);

  // Get the filter type
  switch (filter.type) {
  case FilterType::EqualityMatch:
    if (!getOctetString(filter.equalityMatch.type)) {
      return false;
    }

    if (!getOctetString(filter.equalityMatch.value)) {
      return false;
    }

    break;
  case FilterType::SubstringMatch:
    if (!getOctetString(filter.substringMatch.type)) {
      return false;
    }

    // Parse the substrings
    while (!isEnd()) {
      unsigned char tag;
      unsigned char length;

      if (!getTag(tag)) {
        return false;
      }

      if (tag == 0x80) {
        // Initial
        if (!getOctetString(filter.substringMatch.initial)) {
          return false;
        }
      } else if (tag == 0x81) {
        // Any
        if (!getOctetString(filter.substringMatch.any)) {
          return false;
        }
      } else if (tag == 0x82) {
        // Final
        if (!getOctetString(filter.substringMatch.final)) {
          return false;
        }
      } else {
        std::cerr << "Expected tag 0x80, 0x81, or 0x82, got " << std::hex
                  << (int)tag << std::endl;
        return false;
      }
    }
    break;
  case FilterType::AND:
  case FilterType::OR:
  case FilterType::NOT:
    // parse nested filters
    while (!isEnd()) {
      Filter nestedFilter;

      if (!getFilter(nestedFilter)) {
        return false;
      }

      filter.filters.push_back(nestedFilter);
    }
    break;

  default:
    break;
  }
}