#ifndef SEARCH_H
#define SEARCH_H

#include <string>
#include <vector>

struct FileEntry {
  std::string cn;
  std::string uid;
  std::string mail;
};

struct EqType {
  std::string type;
  std::string value;
};

struct SubsType {
  std::string type;
  std::string initial;
  std::vector<std::string> any;
  std::string final;
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