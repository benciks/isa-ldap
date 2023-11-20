/**
 * @file search.cpp
 * @brief This file contains the csv manipulation and search filter functions
 * implementation
 * @author Simon Bencik <xbenci01>
 */
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "../include/search.h"

std::vector<FileEntry> readCSV(const std::string &filename) {
  std::vector<FileEntry> entries;
  std::ifstream file(filename);
  std::string line;

  while (std::getline(file, line)) {
    std::stringstream ss(line);
    std::string cn, uid, mail;
    std::getline(ss, cn, ';');
    std::getline(ss, uid, ';');
    std::getline(ss, mail, ';');

    // Change crlf to lf
    cn.erase(std::remove(cn.begin(), cn.end(), '\r'), cn.end());
    uid.erase(std::remove(uid.begin(), uid.end(), '\r'), uid.end());
    mail.erase(std::remove(mail.begin(), mail.end(), '\r'), mail.end());

    entries.push_back({cn, uid, mail});
  }

  return entries;
}

bool filterEntry(const Filter &filter, const FileEntry &entry) {
  switch (filter.type) {
  case FilterType::ALL:
    return true;
  case FilterType::EqualityMatch:
    return applyEqualityMatch(filter.equalityMatch, entry);
  case FilterType::SubstringMatch:
    return applySubstringMatch(filter.substringMatch, entry);
  case FilterType::AND:
    return applyAND(filter.filters, entry);
  case FilterType::OR:
    return applyOR(filter.filters, entry);
  case FilterType::NOT:
    if (filter.filters.size() != 1) {
      std::cerr << "Expected 1 filter for NOT, got " << filter.filters.size()
                << std::endl;
      return {};
    }
    return applyNOT(filter.filters[0], entry);
  default:
    return false;
  }
}

bool applyEqualityMatch(const EqType &eqMatch, const FileEntry &entry) {
  if (eqMatch.type == "cn") {
    return entry.cn == eqMatch.value;
  } else if (eqMatch.type == "uid") {
    return entry.uid == eqMatch.value;
  } else if (eqMatch.type == "mail") {
    return entry.mail == eqMatch.value;
  }

  return false;
}

bool applySubstringMatch(const SubsType &subsMatch, const FileEntry &entry) {
  std::string entryValue;
  // Determine what attribute we are matching on
  if (subsMatch.type == "cn") {
    entryValue = entry.cn;
  } else if (subsMatch.type == "uid") {
    entryValue = entry.uid;
  } else if (subsMatch.type == "mail") {
    entryValue = entry.mail;
  }

  if (!subsMatch.initial.empty()) {
    if (entryValue.find(subsMatch.initial) != 0) {
      return false;
    }
  }

  // Check any parts in sequence
  size_t startPos =
      subsMatch.initial.empty()
          ? 0
          : entryValue.find(subsMatch.initial) + subsMatch.initial.size();
  for (const auto &part : subsMatch.any) {
    size_t anyPos = entryValue.find(part, startPos);
    if (anyPos == std::string::npos) {
      return false;
    }
    startPos = anyPos + part.size();
  }

  if (!subsMatch.final.empty()) {
    size_t finalPos = entryValue.rfind(subsMatch.final);
    if (finalPos == std::string::npos ||
        finalPos != entryValue.length() - subsMatch.final.length()) {
      return false;
    }
  }

  return true;
}

bool applyAND(const std::vector<Filter> &filters, const FileEntry &entry) {
  for (const auto &filter : filters) {
    if (!filterEntry(filter, entry)) {
      return false;
    }
  }

  return true;
}

bool applyOR(const std::vector<Filter> &filters, const FileEntry &entry) {
  for (const auto &filter : filters) {
    if (filterEntry(filter, entry)) {
      return true;
    }
  }

  return false;
}

bool applyNOT(const Filter &filter, const FileEntry &entry) {
  if (filterEntry(filter, entry)) {
    return false;
  }

  return true;
}
