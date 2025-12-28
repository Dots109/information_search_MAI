#include "inverted_index.h"
#include <fstream>
#include <iostream>

InvertedIndex::InvertedIndex() : total_docs(0) {}

void InvertedIndex::addDocument(const std::string& doc_id, const std::vector<Token>& tokens) {
    documents.push_back(doc_id);
    total_docs++;
    
    HashTable<int> term_freq;
    
    for (const auto& token : tokens) {
        int freq = 0;
        if (term_freq.find(token.text, freq)) {
            term_freq.insert(token.text, freq + 1);
        } else {
            term_freq.insert(token.text, 1);
        }
    }
    
    term_freq.iterate([this, &doc_id](const std::string& term, const int& freq) {
        PostingList pl;
        if (index.find(term, pl)) {
            pl.doc_ids.push_back(doc_id);
            pl.frequencies.push_back(freq);
            index.insert(term, pl);
        } else {
            PostingList new_pl;
            new_pl.doc_ids.push_back(doc_id);
            new_pl.frequencies.push_back(freq);
            index.insert(term, new_pl);
        }
    });
}

PostingList* InvertedIndex::getPostingList(const std::string& term) {
    return index.get(term);
}

size_t InvertedIndex::getVocabularySize() const {
    return index.size();
}

size_t InvertedIndex::getTotalDocuments() const {
    return total_docs;
}

void InvertedIndex::saveToFile(const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "Cannot open file for writing: " << filename << std::endl;
        return;
    }
    
    size_t num_docs = documents.size();
    out.write(reinterpret_cast<const char*>(&num_docs), sizeof(size_t));
    
    for (const auto& doc : documents) {
        size_t len = doc.length();
        out.write(reinterpret_cast<const char*>(&len), sizeof(size_t));
        out.write(doc.c_str(), len);
    }
    
    size_t vocab_size = index.size();
    out.write(reinterpret_cast<const char*>(&vocab_size), sizeof(size_t));
    
    index.iterate([&out](const std::string& term, const PostingList& pl) {
        size_t term_len = term.length();
        out.write(reinterpret_cast<const char*>(&term_len), sizeof(size_t));
        out.write(term.c_str(), term_len);
        
        size_t num_postings = pl.doc_ids.size();
        out.write(reinterpret_cast<const char*>(&num_postings), sizeof(size_t));
        
        for (size_t i = 0; i < num_postings; ++i) {
            size_t doc_len = pl.doc_ids[i].length();
            out.write(reinterpret_cast<const char*>(&doc_len), sizeof(size_t));
            out.write(pl.doc_ids[i].c_str(), doc_len);
            
            int freq = pl.frequencies[i];
            out.write(reinterpret_cast<const char*>(&freq), sizeof(int));
        }
    });
    
    out.close();
    std::cout << "Index saved to: " << filename << std::endl;
}

void InvertedIndex::loadFromFile(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "Cannot open file for reading: " << filename << std::endl;
        return;
    }
    
    size_t num_docs;
    in.read(reinterpret_cast<char*>(&num_docs), sizeof(size_t));
    
    documents.clear();
    for (size_t i = 0; i < num_docs; ++i) {
        size_t len;
        in.read(reinterpret_cast<char*>(&len), sizeof(size_t));
        
        std::string doc;
        doc.resize(len);
        in.read(&doc[0], len);
        documents.push_back(doc);
    }
    
    total_docs = documents.size();
    
    size_t vocab_size;
    in.read(reinterpret_cast<char*>(&vocab_size), sizeof(size_t));
    
    for (size_t i = 0; i < vocab_size; ++i) {
        size_t term_len;
        in.read(reinterpret_cast<char*>(&term_len), sizeof(size_t));
        
        std::string term;
        term.resize(term_len);
        in.read(&term[0], term_len);
        
        size_t num_postings;
        in.read(reinterpret_cast<char*>(&num_postings), sizeof(size_t));
        
        PostingList pl;
        for (size_t j = 0; j < num_postings; ++j) {
            size_t doc_len;
            in.read(reinterpret_cast<char*>(&doc_len), sizeof(size_t));
            
            std::string doc_id;
            doc_id.resize(doc_len);
            in.read(&doc_id[0], doc_len);
            
            int freq;
            in.read(reinterpret_cast<char*>(&freq), sizeof(int));
            
            pl.doc_ids.push_back(doc_id);
            pl.frequencies.push_back(freq);
        }
        
        index.insert(term, pl);
    }
    
    in.close();
    std::cout << "Index loaded from: " << filename << std::endl;
}
