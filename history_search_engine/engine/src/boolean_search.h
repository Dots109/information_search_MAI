#ifndef BOOLEAN_SEARCH_H
#define BOOLEAN_SEARCH_H

#include <string>
#include <vector>
#include "inverted_index.h"
#include "stemmer.h"

enum class Operator {
    NONE,
    AND,
    OR,
    NOT
};

struct QueryToken {
    std::string term;
    Operator op;
};

struct SearchResult {
    std::string url;
    int relevance_score;
};

class BooleanSearch {
private:
    InvertedIndex* index;
    Stemmer* stemmer;
    
    std::vector<QueryToken> parseQuery(const std::string& query);
    std::vector<std::string> intersect(const std::vector<std::string>& a, 
                                       const std::vector<std::string>& b);
    std::vector<std::string> unionSets(const std::vector<std::string>& a,
                                       const std::vector<std::string>& b);
    std::vector<std::string> difference(const std::vector<std::string>& a,
                                        const std::vector<std::string>& b);
    std::vector<std::string> executeQuery(const std::vector<QueryToken>& tokens);
    
public:
    BooleanSearch(InvertedIndex* idx, Stemmer* stem);
    std::vector<SearchResult> search(const std::string& query);
    std::vector<SearchResult> searchWithRanking(const std::string& query);
};

#endif
