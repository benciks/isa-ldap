#ifndef SEARCH_H
#define SEARCH_H

#include <string>
#include <vector>

struct FileEntry {
  std::vector<unsigned char> cn;
  std::vector<unsigned char> uid;
  std::vector<unsigned char> mail;

  // Custom comparator for sorting
  bool operator<(const FileEntry &other) const {
    if (cn != other.cn) {
      return cn < other.cn;
    }
    if (uid != other.uid) {
      return uid < other.uid;
    }
    return mail < other.mail;
  }

  bool operator==(const FileEntry &other) const {
    return cn == other.cn && uid == other.uid && mail == other.mail;
  }
};

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
  ALL = 0x87,
};

struct Filter {
  FilterType type;
  EqType equalityMatch;
  SubsType substringMatch;
  std::vector<Filter> filters;
};

std::vector<FileEntry> readCSV(const std::string &filename);

bool filterEntry(const Filter &filter, const FileEntry &entry);

bool applyEqualityMatch(const EqType &eqMatch, const FileEntry &entry);

bool applySubstringMatch(const SubsType &subsMatch, const FileEntry &entry);

bool applyAND(const std::vector<Filter> &filters, const FileEntry &entry);

bool applyOR(const std::vector<Filter> &filters, const FileEntry &entry);

bool applyNOT(const Filter &filter, const FileEntry &entry);

#endif