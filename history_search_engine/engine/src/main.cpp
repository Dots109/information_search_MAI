#include "tokenizer.h"
#include "stemmer.h"
#include "inverted_index.h"
#include "boolean_search.h"
#include "zipf_analyzer.h"
#include "json_reader.h"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "=== HISTORY SEARCH ENGINE ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    std::cout << "\nReading documents from: /app/data/documents.json" << std::endl;
    
    auto documents = JSONReader::readFromFile("/app/data/documents.json");
    
    if (documents.empty()) {
        std::cerr << "\nERROR: No documents found!" << std::endl;
        return 1;
    }
    
    std::cout << "\nSuccessfully loaded " << documents.size() << " documents\n" << std::endl;
    
    Tokenizer tokenizer;
    Stemmer stemmer;
    InvertedIndex index;
    ZipfAnalyzer zipf;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "Processing documents..." << std::endl;
    
    for (size_t i = 0; i < documents.size(); ++i) {
        const auto& doc = documents[i];
        
        std::string clean_text = JSONReader::stripHTML(doc.html_content);
        auto tokens = tokenizer.tokenize(clean_text);
        
        std::vector<Token> stemmed_tokens;
        for (auto& token : tokens) {
            std::string stem = stemmer.stem(token.text);
            zipf.addTerm(stem);
            
            Token stemmed_token;
            stemmed_token.text = stem;
            stemmed_token.position = token.position;
            stemmed_tokens.push_back(stemmed_token);
        }
        
        index.addDocument(doc.url, stemmed_tokens);
        
        if ((i + 1) % 500 == 0) {
            std::cout << "✓ Processed " << (i + 1) << "/" << documents.size() 
                      << " documents" << std::endl;
        }
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time).count();
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "=== INDEX STATISTICS ===" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "Vocabulary size: " << index.getVocabularySize() << std::endl;
    std::cout << "Indexed documents: " << documents.size() << std::endl;
    std::cout << "Processing time: " << duration / 1000.0 << " seconds" << std::endl;
    
    std::cout << "\n💾 Saving results..." << std::endl;
    index.saveToFile("/app/output/inverted_index.bin");
    zipf.saveToCSV("/app/output/zipf_analysis.csv");
    zipf.printStatistics();
    
    std::cout << "\n✅ Processing complete!" << std::endl;
    
    return 0;
}