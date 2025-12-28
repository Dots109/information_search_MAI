#include "boolean_search.h"
#include <algorithm>
#include <sstream>
#include <cctype>

BooleanSearch::BooleanSearch(InvertedIndex* idx, Stemmer* stem) 
    : index(idx), stemmer(stem) {}

std::vector<QueryToken> BooleanSearch::parseQuery(const std::string& query) {
    std::vector<QueryToken> tokens;
    std::istringstream iss(query);
    std::string word;
    Operator current_op = Operator::NONE;
    
    while (iss >> word) {
        std::string upper = word;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
        
        if (upper == "AND") {
            current_op = Operator::AND;
        } else if (upper == "OR") {
            current_op = Operator::OR;
        } else if (upper == "NOT") {
            current_op = Operator::NOT;
        } else {
            QueryToken token;
            token.term = word;
            token.op = current_op;
            tokens.push_back(token);
            current_op = Operator::AND;
        }
    }
    
    return tokens;
}

std::vector<std::string> BooleanSearch::intersect(
    const std::vector<std::string>& a,
    const std::vector<std::string>& b) {
    
    std::vector<std::string> result;
    size_t i = 0, j = 0;
    
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            result.push_back(a[i]);
            ++i; ++j;
        } else if (a[i] < b[j]) {
            ++i;
        } else {
            ++j;
        }
    }
    
    return result;
}

std::vector<std::string> BooleanSearch::unionSets(
    const std::vector<std::string>& a,
    const std::vector<std::string>& b) {
    
    std::vector<std::string> result;
    size_t i = 0, j = 0;
    
    while (i < a.size() && j < b.size()) {
        if (a[i] == b[j]) {
            result.push_back(a[i]);
            ++i; ++j;
        } else if (a[i] < b[j]) {
            result.push_back(a[i]);
            ++i;
        } else {
            result.push_back(b[j]);
            ++j;
        }
    }
    
    while (i < a.size()) result.push_back(a[i++]);
    while (j < b.size()) result.push_back(b[j++]);
    
    return result;
}

std::vector<std::string> BooleanSearch::difference(
    const std::vector<std::string>& a,
    const std::vector<std::string>& b) {
    
    std::vector<std::string> result;
    size_t i = 0, j = 0;
    
    while (i < a.size()) {
        if (j >= b.size() || a[i] < b[j]) {
            result.push_back(a[i]);
            ++i;
        } else if (a[i] == b[j]) {
            ++i; ++j;
        } else {
            ++j;
        }
    }
    
    return result;
}

std::vector<std::string> BooleanSearch::executeQuery(
    const std::vector<QueryToken>& tokens) {
    
    if (tokens.empty()) return {};
    
    std::vector<std::string> result;
    
    for (const auto& token : tokens) {
        std::string stemmed = stemmer->stem(token.term);
        
        PostingList* posting = index->getPostingList(stemmed);
        if (!posting) continue;
        
        std::vector<std::string> current_docs = posting->doc_ids;
        std::sort(current_docs.begin(), current_docs.end());
        
        if (result.empty()) {
            if (token.op != Operator::NOT) {
                result = current_docs;
            }
        } else {
            switch (token.op) {
                case Operator::AND:
                    result = intersect(result, current_docs);
                    break;
                case Operator::OR:
                    result = unionSets(result, current_docs);
                    break;
                case Operator::NOT:
                    result = difference(result, current_docs);
                    break;
                case Operator::NONE:
                    result = intersect(result, current_docs);
                    break;
            }
        }
    }
    
    return result;
}

std::vector<SearchResult> BooleanSearch::search(const std::string& query) {
    auto query_tokens = parseQuery(query);
    auto doc_ids = executeQuery(query_tokens);
    
    std::vector<SearchResult> results;
    for (const auto& doc_id : doc_ids) {
        SearchResult result;
        result.url = doc_id;
        result.relevance_score = 1;
        results.push_back(result);
    }
    
    return results;
}

std::vector<SearchResult> BooleanSearch::searchWithRanking(const std::string& query) {
    auto query_tokens = parseQuery(query);
    auto doc_ids = executeQuery(query_tokens);
    
    std::vector<SearchResult> results;
    
    for (const auto& doc_id : doc_ids) {
        SearchResult result;
        result.url = doc_id;
        result.relevance_score = 0;
        
        for (const auto& token : query_tokens) {
            std::string stemmed = stemmer->stem(token.term);
            PostingList* posting = index->getPostingList(stemmed);
            
            if (posting) {
                for (size_t i = 0; i < posting->doc_ids.size(); ++i) {
                    if (posting->doc_ids[i] == doc_id) {
                        result.relevance_score += posting->frequencies[i];
                        break;
                    }
                }
            }
        }
        
        results.push_back(result);
    }
    
    std::sort(results.begin(), results.end(),
              [](const SearchResult& a, const SearchResult& b) {
                  return a.relevance_score > b.relevance_score;
              });
    
    return results;
}
