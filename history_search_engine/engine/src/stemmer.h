#ifndef STEMMER_H
#define STEMMER_H

#include <string>
#include <vector>

class Stemmer {
private:
    struct Suffix {
        std::string text;
        size_t min_base_length;
    };
    
    std::vector<Suffix> perfective;
    std::vector<Suffix> reflexive;
    std::vector<Suffix> adjective;
    std::vector<Suffix> participle;
    std::vector<Suffix> verb;
    std::vector<Suffix> noun;
    std::vector<Suffix> superlative;
    std::vector<Suffix> derivational;
    
    bool endsWith(const std::string& word, const std::string& suffix) const;
    bool removeSuffix(std::string& word, const std::vector<Suffix>& suffixes) const;
    bool isVowel(unsigned char c1, unsigned char c2) const;
    size_t findRV(const std::string& word) const;
    void removeI(std::string& word) const;
    
public:
    Stemmer();
    std::string stem(const std::string& word);
};

#endif
