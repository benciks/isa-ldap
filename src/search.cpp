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

    entries.push_back({std::vector<unsigned char>(cn.begin(), cn.end()),
                       std::vector<unsigned char>(uid.begin(), uid.end()),
                       std::vector<unsigned char>(mail.begin(), mail.end())});
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
  if (eqMatch.type == std::vector<unsigned char>({'c', 'n'})) {
    return entry.cn == eqMatch.value;
  } else if (eqMatch.type == std::vector<unsigned char>({'u', 'i', 'd'})) {
    return entry.uid == eqMatch.value;
  } else if (eqMatch.type == std::vector<unsigned char>({'m', 'a', 'i', 'l'})) {
    return entry.mail == eqMatch.value;
  }

  return false;
}

bool applySubstringMatch(const SubsType &subsMatch, const FileEntry &entry) {
  std::vector<FileEntry> result;

  std::string initial(subsMatch.initial.begin(), subsMatch.initial.end());
  std::string any(subsMatch.any.begin(), subsMatch.any.end());
  std::string final(subsMatch.final.begin(), subsMatch.final.end());

  std::string cn(entry.cn.begin(), entry.cn.end());
  std::string uid(entry.uid.begin(), entry.uid.end());
  std::string mail(entry.mail.begin(), entry.mail.end());

  std::string entryValue;
  // Determine what attribute we are matching on
  if (subsMatch.type == std::vector<unsigned char>({'c', 'n'})) {
    entryValue = cn;
  } else if (subsMatch.type == std::vector<unsigned char>({'u', 'i', 'd'})) {
    entryValue = uid;
  } else if (subsMatch.type ==
             std::vector<unsigned char>({'m', 'a', 'i', 'l'})) {
    entryValue = mail;
  }

  if (!initial.empty()) {
    if (entryValue.find(initial) != 0) {
      return false;
    }
  }

  if (!any.empty()) {
    std::cout << "any: " << any << std::endl;
    if (entryValue.find(any) == std::string::npos) {
      return false;
    }
  }

  if (!final.empty()) {
    size_t finalPos = entryValue.rfind(final);
    if (finalPos == std::string::npos ||
        finalPos != entryValue.length() - final.length()) {
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
