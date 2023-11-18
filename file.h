#ifndef FILE_H
#define FILE_H

#include <string>
#include <vector>

struct FileEntry {
  std::string cn;
  std::string uid;
  std::string mail;
};

std::vector<FileEntry> readCSV(const std::string &filename);

std::vector<FileEntry> searchByCN(const std::vector<FileEntry> &entries,
                                  const std::string &cn);

std::vector<FileEntry> searchByUID(const std::vector<FileEntry> &entries,
                                   const std::string &uid);

std::vector<FileEntry> searchByMail(const std::vector<FileEntry> &entries,
                                    const std::string &mail);

#endif