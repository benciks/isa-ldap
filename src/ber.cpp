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

bool BERParser::getSubstringFilter(SubsType &subs) {
  unsigned char tag;
  if (!getTag(tag)) {
    return false;
  }

  unsigned char length;
  if (!getLength(length)) {
    return false;
  }

  if (tag == 0x80) {
    subs.initial = std::vector<unsigned char>(buffer.begin() + pos,
                                              buffer.begin() + pos + length);
  } else if (tag == 0x81) {
    subs.any = std::vector<unsigned char>(buffer.begin() + pos,
                                          buffer.begin() + pos + length);
  } else if (tag == 0x82) {
    subs.final = std::vector<unsigned char>(buffer.begin() + pos,
                                            buffer.begin() + pos + length);
  } else {
    std::cerr << "Expected tag 0x80, 0x81, or 0x82, got " << std::hex
              << (int)tag << std::endl;
    return false;
  }

  pos += length;
  return true;
}

bool BERParser::getFilter(Filter &filter) {
  // If following tag is a sequence, we catched attribute list
  unsigned char tag;
  unsigned char length;

  if (!getTag(tag)) {
    return false;
  }

  if (!getLength(length)) {
    return false;
  }

  filter.type = static_cast<FilterType>(tag);

  std::vector<unsigned char> seq;
  size_t endOfFilter = pos + length;

  // Get the filter type
  switch (filter.type) {
  case FilterType::ALL:
    break;
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

    // Parse sequence
    if (!getSequence(seq)) {
      return false;
    }

    // parse substring filters
    if (!getSubstringFilter(filter.substringMatch)) {
      return false;
    }

    break;
  case FilterType::AND:
  case FilterType::OR:
  case FilterType::NOT:
    while (pos < endOfFilter) {
      Filter nestedFilter;

      if (!getFilter(nestedFilter)) {
        return false;
        break;
      }

      filter.filters.push_back(nestedFilter);
    }
    break;

  default:
    break;
  }

  return true;
}
