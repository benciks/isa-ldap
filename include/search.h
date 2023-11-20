/**
 * @file search.h
 * @brief This file contains the csv manipulation and search filter functions
 * @author Simon Bencik <xbenci01>
 */
#ifndef SEARCH_H
#define SEARCH_H

#include <string>
#include <vector>

/**
 * @struct FileEntry
 * @brief The entry in the CSV file
 */
struct FileEntry {
  std::string cn;
  std::string uid;
  std::string mail;
};

/**
 * @struct EqType
 * @brief The equality match type
 */
struct EqType {
  std::string type;
  std::string value;
};

/**
 * @struct SubsType
 * @brief The substring match type
 */
struct SubsType {
  std::string type;
  std::string initial;
  std::vector<std::string> any;
  std::string final;
};

/**
 * @enum FilterType
 * @brief The type of filter
 */
enum FilterType {
  EqualityMatch = 0xA3,
  SubstringMatch = 0xA4,
  AND = 0xA0,
  OR = 0xA1,
  NOT = 0xA2,
  ALL = 0x87,
};

/**
 * @struct Filter
 * @brief Tree like structure for filters
 */
struct Filter {
  FilterType type;
  EqType equalityMatch;
  SubsType substringMatch;
  std::vector<Filter> filters;
};

/**
 * @brief Read the CSV file
 * @param filename The name of the CSV file
 * @return The entries in the CSV file
 */
std::vector<FileEntry> readCSV(const std::string &filename);

/**
 * @brief Function to recursively filter the entry
 * @param filter The filter to apply
 * @param entry The entry to filter
 * @return Whether the entry matches the filter
 */
bool filterEntry(const Filter &filter, const FileEntry &entry);

/**
 * @brief Function to apply the equality match to entry
 * @param eqMatch The equality match to apply
 * @param entry The entry to apply the equality match to
 * @return Whether the entry matches the equality match
 */
bool applyEqualityMatch(const EqType &eqMatch, const FileEntry &entry);

/**
 * @brief Function to apply the substring match to entry
 * @param subsMatch The substring match to apply
 * @param entry The entry to apply the substring match to
 * @return Whether the entry matches the substring match
 */
bool applySubstringMatch(const SubsType &subsMatch, const FileEntry &entry);

/**
 * @brief Function to apply the AND filter to entry
 * @param filters The filters to apply
 * @param entry The entry to apply the filters to
 */
bool applyAND(const std::vector<Filter> &filters, const FileEntry &entry);

/**
 * @brief Function to apply the OR filter to entry
 * @param filters The filters to apply
 * @param entry The entry to apply the filters to
 * @return Whether the entry matches the OR filter
 */
bool applyOR(const std::vector<Filter> &filters, const FileEntry &entry);

/**
 * @brief Function to apply the NOT filter to entry
 * @param filter The filter to apply
 * @param entry The entry to apply the filter to
 * @return Whether the entry matches the NOT filter
 */
bool applyNOT(const Filter &filter, const FileEntry &entry);

#endif