#include "zipf_analyzer.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>

ZipfAnalyzer::ZipfAnalyzer() : total_terms(0) {}

void ZipfAnalyzer::addTerm(const std::string& term) {
    int count = 0;
    if (term_counts.find(term, count)) {
        term_counts.insert(term, count + 1);
    } else {
        term_counts.insert(term, 1);
    }
    total_terms++;
}

void ZipfAnalyzer::saveToCSV(const std::string& filename) {
    std::vector<TermFrequency> frequencies;
    
    term_counts.iterate([&frequencies](const std::string& term, const int& freq) {
        TermFrequency tf;
        tf.term = term;
        tf.frequency = freq;
        frequencies.push_back(tf);
    });
    
    std::sort(frequencies.begin(), frequencies.end(),
              [](const TermFrequency& a, const TermFrequency& b) {
                  return a.frequency > b.frequency;
              });
    
    std::ofstream out(filename);
    if (!out.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return;
    }
    
    out << "rank,term,frequency,zipf_prediction,log_rank,log_frequency\n";
    
    int max_freq = frequencies.empty() ? 1 : frequencies[0].frequency;
    
    for (size_t i = 0; i < frequencies.size() && i < 5000; ++i) {
        int rank = i + 1;
        double zipf_pred = static_cast<double>(max_freq) / rank;
        double log_rank = std::log10(rank);
        double log_freq = std::log10(frequencies[i].frequency);
        
        out << rank << ","
            << frequencies[i].term << ","
            << frequencies[i].frequency << ","
            << zipf_pred << ","
            << log_rank << ","
            << log_freq << "\n";
    }
    
    out.close();
    std::cout << "Zipf analysis saved to: " << filename << std::endl;
}

void ZipfAnalyzer::printStatistics() {
    std::cout << "\n=== Zipf Law Statistics ===" << std::endl;
    std::cout << "Total terms processed: " << total_terms << std::endl;
    std::cout << "Unique terms: " << term_counts.size() << std::endl;
    std::cout << "Vocabulary richness: " 
              << (100.0 * term_counts.size() / total_terms) << "%" << std::endl;
}
