#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <string>
#include <vector>

struct Token {
    std::string text;
    size_t position;
};

struct TokenizerStats {
    size_t total_tokens;
    double avg_length;
    size_t unique_count;
};

class Tokenizer {
private:
    TokenizerStats stats;
    size_t total_length;
    
    bool isRussianByte(unsigned char c) const;
    bool isLatinLetter(char c) const;
    bool isDigit(char c) const;
    std::string toLower(const std::string& str) const;
    
public:
    Tokenizer();
    std::vector<Token> tokenize(const std::string& text);
    TokenizerStats getStats() const;
};

#endif
