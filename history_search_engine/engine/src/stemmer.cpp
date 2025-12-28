#include "stemmer.h"

Stemmer::Stemmer() {
    perfective = {
        {"\xD0\xB2\xD1\x88\xD0\xB8\xD1\x81\xD1\x8C", 0},
        {"\xD0\xB2\xD1\x88\xD0\xB8", 0},
        {"\xD0\xB2", 0}
    };
    
    reflexive = {
        {"\xD1\x81\xD1\x8F", 0},
        {"\xD1\x81\xD1\x8C", 0}
    };
    
    adjective = {
        {"\xD0\xB5\xD0\xB5", 0},
        {"\xD0\xB8\xD0\xB5", 0},
        {"\xD1\x8B\xD0\xB5", 0},
        {"\xD0\xBE\xD0\xB5", 0},
        {"\xD0\xB8\xD0\xBC\xD0\xB8", 0},
        {"\xD1\x8B\xD0\xBC\xD0\xB8", 0},
        {"\xD0\xB5\xD0\xB9", 0},
        {"\xD0\xB8\xD0\xB9", 0},
        {"\xD1\x8B\xD0\xB9", 0},
        {"\xD0\xBE\xD0\xB9", 0},
        {"\xD0\xB5\xD0\xBC", 0},
        {"\xD0\xB8\xD0\xBC", 0},
        {"\xD1\x8B\xD0\xBC", 0},
        {"\xD0\xBE\xD0\xBC", 0},
        {"\xD0\xB8\xD1\x85", 0},
        {"\xD1\x8B\xD1\x85", 0},
        {"\xD1\x83\xD1\x8E", 0},
        {"\xD1\x8E\xD1\x8E", 0},
        {"\xD0\xB0\xD1\x8F", 0},
        {"\xD1\x8F\xD1\x8F", 0}
    };
    
    participle = {
        {"\xD0\xB5\xD0\xBC", 0},
        {"\xD0\xBD\xD0\xBD", 0},
        {"\xD1\x88", 0},
        {"\xD1\x89", 0}
    };
    
    verb = {
        {"\xD1\x83\xD0\xB9\xD1\x82\xD0\xB5", 0},
        {"\xD0\xB5\xD0\xB9\xD1\x82\xD0\xB5", 0},
        {"\xD0\xB9\xD1\x82\xD0\xB5", 0},
        {"\xD1\x83\xD1\x8E\xD1\x82", 0},
        {"\xD1\x8E\xD1\x82", 0},
        {"\xD1\x83\xD1\x8E", 0},
        {"\xD1\x8E", 0}
    };
    
    noun = {
        {"\xD0\xB8\xD1\x8F\xD0\xBC\xD0\xB8", 0},
        {"\xD1\x8C\xD0\xBC\xD0\xB8", 0},
        {"\xD0\xB0\xD0\xBC\xD0\xB8", 0},
        {"\xD0\xB8\xD0\xB5\xD0\xBC", 0},
        {"\xD0\xB8\xD0\xB5\xD0\xB9", 0},
        {"\xD0\xB8\xD1\x8F\xD1\x85", 0},
        {"\xD0\xB8\xD1\x8F\xD0\xBC", 0},
        {"\xD0\xB8\xD0\xB8", 0},
        {"\xD0\xB8\xD1\x8F", 0},
        {"\xD0\xB5\xD0\xB2", 0},
        {"\xD0\xBE\xD0\xB2", 0},
        {"\xD1\x8C\xD0\xB5", 0},
        {"\xD1\x8C\xD1\x8F", 0},
        {"\xD0\xB5\xD0\xB2", 0},
        {"\xD0\xB0\xD0\xBC", 0},
        {"\xD0\xB5\xD0\xBC", 0},
        {"\xD0\xB0\xD1\x85", 0},
        {"\xD1\x8C\xD1\x8E", 0},
        {"\xD0\xB8\xD1\x8E", 0},
        {"\xD0\xB5\xD0\xB9", 0},
        {"\xD0\xB8\xD0\xB9", 0},
        {"\xD0\xB8\xD0\xB5", 0},
        {"\xD1\x8C\xD0\xB5", 0},
        {"\xD0\xB5\xD0\xB5", 0},
        {"\xD0\xBE", 0},
        {"\xD0\xB0", 0},
        {"\xD0\xB5", 0},
        {"\xD0\xB8", 0},
        {"\xD1\x8B", 0},
        {"\xD1\x8C", 0},
        {"\xD1\x8E", 0},
        {"\xD1\x8F", 0}
    };
    
    superlative = {
        {"\xD0\xB5\xD0\xB9\xD1\x88", 0}
    };
    
    derivational = {
        {"\xD0\xBE\xD1\x81\xD1\x82", 0},
        {"\xD0\xBE\xD1\x81\xD1\x82\xD1\x8C", 0}
    };
}

bool Stemmer::endsWith(const std::string& word, const std::string& suffix) const {
    if (word.length() < suffix.length()) return false;
    return word.substr(word.length() - suffix.length()) == suffix;
}

bool Stemmer::removeSuffix(std::string& word, const std::vector<Suffix>& suffixes) const {
    for (const auto& suffix : suffixes) {
        if (endsWith(word, suffix.text)) {
            size_t new_len = word.length() - suffix.text.length();
            if (new_len >= suffix.min_base_length) {
                word = word.substr(0, new_len);
                return true;
            }
        }
    }
    return false;
}

bool Stemmer::isVowel(unsigned char c1, unsigned char c2) const {
    if (c1 == 0xD0) {
        return (c2 == 0xB0 || c2 == 0xB5 || c2 == 0xB8 || 
                c2 == 0xBE || c2 == 0xB3 || c2 == 0xBB || c2 == 0xB8);
    }
    if (c1 == 0xD1) {
        return (c2 == 0x8B || c2 == 0x8D || c2 == 0x8E || c2 == 0x8F);
    }
    return false;
}

size_t Stemmer::findRV(const std::string& word) const {
    for (size_t i = 0; i + 1 < word.length(); i += 2) {
        if (isVowel(word[i], word[i + 1])) {
            return i + 2;
        }
    }
    return word.length();
}

void Stemmer::removeI(std::string& word) const {
    if (word.length() >= 2 && word[word.length() - 2] == '\xD0' && 
        word[word.length() - 1] == '\xB8') {
        word = word.substr(0, word.length() - 2);
    }
}

std::string Stemmer::stem(const std::string& word) {
    if (word.length() < 4) return word;
    
    std::string result = word;
    
    removeSuffix(result, perfective);
    
    if (!removeSuffix(result, reflexive)) {
        removeSuffix(result, reflexive);
    }
    
    if (removeSuffix(result, adjective)) {
        removeSuffix(result, participle);
    } else {
        if (!removeSuffix(result, verb)) {
            removeSuffix(result, noun);
        }
    }
    
    removeI(result);
    removeSuffix(result, derivational);
    removeSuffix(result, superlative);
    
    return result;
}
