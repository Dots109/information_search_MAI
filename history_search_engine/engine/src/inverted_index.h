#ifndef INVERTED_INDEX_H
#define INVERTED_INDEX_H

#include <string>
#include <vector>
#include "hash_table.h"
#include "tokenizer.h"

struct PostingList {
    std::vector<std::string> doc_ids;
    std::vector<int> frequencies;
};

class InvertedIndex {
private:
    HashTable<PostingList> index;
    std::vector<std::string> documents;
    size_t total_docs;
    
public:
    InvertedIndex();
    void addDocument(const std::string& doc_id, const std::vector<Token>& tokens);
    PostingList* getPostingList(const std::string& term);
    size_t getVocabularySize() const;
    size_t getTotalDocuments() const;
    void saveToFile(const std::string& filename);
    void loadFromFile(const std::string& filename);
};

#endif
