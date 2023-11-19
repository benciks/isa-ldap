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

std::vector<FileEntry> filterEntries(const Filter &filter,
                                     const std::vector<FileEntry> &entries) {

  switch (filter.type) {
  case FilterType::ALL:
    return entries;
  case FilterType::EqualityMatch:
    return applyEqualityMatch(filter.equalityMatch, entries);
  case FilterType::SubstringMatch:
    return applySubstringMatch(filter.substringMatch, entries);
  case FilterType::AND:
    return applyAND(filter.filters, entries);
  case FilterType::OR:
    return applyOR(filter.filters, entries);
  case FilterType::NOT:
    if (filter.filters.size() != 1) {
      std::cerr << "Expected 1 filter for NOT, got " << filter.filters.size()
                << std::endl;
      return {};
    }
    return applyNOT(filter.filters[0], entries);
  default:
    break;
  }

  // Return empty vector if we don't recognize the filter type
  return std::vector<FileEntry>();
}

std::vector<FileEntry>
applyEqualityMatch(const EqType &eqMatch,
                   const std::vector<FileEntry> &entries) {
  std::vector<FileEntry> result;

  if (eqMatch.type == std::vector<unsigned char>({'c', 'n'})) {
    for (const auto &entry : entries) {
      if (entry.cn == eqMatch.value) {
        result.push_back(entry);
      }
    }
  } else if (eqMatch.type == std::vector<unsigned char>({'u', 'i', 'd'})) {
    for (const auto &entry : entries) {
      if (entry.uid == eqMatch.value) {
        result.push_back(entry);
      }
    }
  } else if (eqMatch.type == std::vector<unsigned char>({'m', 'a', 'i', 'l'})) {
    for (const auto &entry : entries) {
      if (entry.mail == eqMatch.value) {
        result.push_back(entry);
      }
    }
  }

  std::cout << "EqualityMatch" << std::endl;
  return result;
}

std::vector<FileEntry>
applySubstringMatch(const SubsType &subsMatch,
                    const std::vector<FileEntry> &entries) {
  std::vector<FileEntry> result;

  std::string initial(subsMatch.initial.begin(), subsMatch.initial.end());
  std::string any(subsMatch.any.begin(), subsMatch.any.end());
  std::string final(subsMatch.final.begin(), subsMatch.final.end());

  for (const auto &entry : entries) {
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

    bool match = true;

    if (!initial.empty()) {
      if (entryValue.find(initial) != 0) {
        match = false;
      }
    }

    if (!any.empty()) {
      if (entryValue.find(any) == std::string::npos) {
        match = false;
      }
    }

    if (!final.empty()) {
      if (entryValue.find(final) != entryValue.length() - final.length()) {
        match = false;
      }
    }

    if (match) {
      result.push_back(entry);
    }
  }

  std::cout << "SubstringMatch" << std::endl;
  return result;
}

std::vector<FileEntry> applyAND(const std::vector<Filter> &filters,
                                const std::vector<FileEntry> &entries) {
  std::vector<FileEntry> result;
  for (const auto &filter : filters) {
    auto filteredEntries = filterEntries(filter, entries);

    // Print result
    for (const auto &entry : filteredEntries) {
      std::cout << "and cn: " << std::string(entry.cn.begin(), entry.cn.end())
                << std::endl;
    }
    std::cout << std::endl;

    if (result.empty()) {
      result = filteredEntries;
    } else {
      std::sort(result.begin(), result.end());
      std::sort(filteredEntries.begin(), filteredEntries.end());

      std::vector<FileEntry> temp;
      std::set_intersection(result.begin(), result.end(),
                            filteredEntries.begin(), filteredEntries.end(),
                            std::back_inserter(temp));
      result = temp;
    }
  }
  std::cout << "AND" << std::endl;
  return result;
}

std::vector<FileEntry> applyOR(const std::vector<Filter> &filters,
                               const std::vector<FileEntry> &entries) {
  std::vector<FileEntry> result;

  for (const auto &filter : filters) {
    // Print filter type
    auto filteredEntries = filterEntries(filter, entries);

    // Print result
    for (const auto &entry : filteredEntries) {
      std::cout << "or cn: " << std::string(entry.cn.begin(), entry.cn.end())
                << std::endl;
    }
    std::cout << std::endl;

    std::sort(filteredEntries.begin(), filteredEntries.end());

    std::vector<FileEntry> temp;
    std::set_union(result.begin(), result.end(), filteredEntries.begin(),
                   filteredEntries.end(), std::back_inserter(temp));

    result = temp;
  }

  // Remove duplicates
  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());

  std::cout << "OR" << std::endl;
  return result;
}

std::vector<FileEntry> applyNOT(const Filter &filter,
                                const std::vector<FileEntry> &entries) {
  std::vector<FileEntry> result;
  auto filteredEntries = filterEntries(filter, entries);

  for (const auto &entry : entries) {
    if (std::find(filteredEntries.begin(), filteredEntries.end(), entry) ==
        filteredEntries.end()) {
      result.push_back(entry);
    }
  }

  std::cout << "NOT" << std::endl;
  return result;
}
