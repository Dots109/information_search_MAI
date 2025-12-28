#ifndef ZIPF_ANALYZER_H
#define ZIPF_ANALYZER_H

#include <string>
#include <vector>
#include "hash_table.h"

struct TermFrequency {
    std::string term;
    int frequency;
};

class ZipfAnalyzer {
private:
    HashTable<int> term_counts;
    size_t total_terms;
    
public:
    ZipfAnalyzer();
    void addTerm(const std::string& term);
    void saveToCSV(const std::string& filename);
    void printStatistics();
};

#endif
