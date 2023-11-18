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
    entries.push_back({cn, uid, mail});
  }

  return entries;
}

std::vector<FileEntry> searchByCN(const std::vector<FileEntry> &entries,
                                  const std::string &cn) {
  std::vector<FileEntry> result;
  for (const auto &entry : entries) {
    if (entry.cn == cn) {
      result.push_back(entry);
    }
  }
  return result;
}

std::vector<FileEntry> searchByUID(const std::vector<FileEntry> &entries,
                                   const std::string &uid) {
  std::vector<FileEntry> result;
  for (const auto &entry : entries) {
    if (entry.uid == uid) {
      result.push_back(entry);
    }
  }
  return result;
}

std::vector<FileEntry> searchByMail(const std::vector<FileEntry> &entries,
                                    const std::string &mail) {
  std::vector<FileEntry> result;
  for (const auto &entry : entries) {
    if (entry.mail == mail) {
      result.push_back(entry);
    }
  }
  return result;
}