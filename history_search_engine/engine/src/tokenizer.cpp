#include "tokenizer.h"

Tokenizer::Tokenizer() : total_length(0) {
    stats.total_tokens = 0;
    stats.avg_length = 0.0;
    stats.unique_count = 0;
}

bool Tokenizer::isRussianByte(unsigned char c) const {
    return (c >= 0xD0 && c <= 0xD1);
}

bool Tokenizer::isLatinLetter(char c) const {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

bool Tokenizer::isDigit(char c) const {
    return c >= '0' && c <= '9';
}

std::string Tokenizer::toLower(const std::string& str) const {
    std::string result;
    result.reserve(str.length());
    
    for (size_t i = 0; i < str.length(); ++i) {
        unsigned char c = str[i];
        
        if (c >= 'A' && c <= 'Z') {
            result += (c + 32);
        }
        else if (i + 1 < str.length()) {
            unsigned char next = str[i + 1];
            
            if (c == 0xD0 && next >= 0x90 && next <= 0xAF) {
                result += c;
                result += (next + 0x20);
                ++i;
            }
            else if (c == 0xD0 && next == 0x81) {
                result += "\xD1\x91";
                ++i;
            }
            else {
                result += c;
                if (isRussianByte(c)) {
                    result += next;
                    ++i;
                }
            }
        }
        else {
            result += c;
        }
    }
    
    return result;
}

std::vector<Token> Tokenizer::tokenize(const std::string& text) {
    std::vector<Token> tokens;
    std::string current;
    size_t position = 0;
    size_t i = 0;
    
    while (i < text.length()) {
        unsigned char c = text[i];
        
        if (c <= 32 || c == ',' || c == '.' || c == '!' || c == '?' ||
            c == ';' || c == ':' || c == '"' || c == '\'' || c == '(' ||
            c == ')' || c == '[' || c == ']' || c == '{' || c == '}' ||
            c == '<' || c == '>' || c == '/' || c == '\\' || c == '|' ||
            c == '=' || c == '+' || c == '*' || c == '&') {
            
            if (current.length() >= 2) {
                bool is_number = true;
                for (char ch : current) {
                    if (!isDigit(ch)) {
                        is_number = false;
                        break;
                    }
                }
                
                if (!is_number) {
                    Token token;
                    token.text = toLower(current);
                    token.position = position;
                    tokens.push_back(token);
                    
                    total_length += token.text.length();
                }
            }
            
            current.clear();
            position = i + 1;
            ++i;
            continue;
        }
        
        if (c == 0xD0 || c == 0xD1) {
            if (i + 1 < text.length()) {
                current += text[i];
                current += text[i + 1];
                i += 2;
            } else {
                ++i;
            }
        }
        else if (isLatinLetter(c) || (isDigit(c) && !current.empty()) || 
                 (c == '-' && !current.empty())) {
            current += text[i];
            ++i;
        }
        else {
            if (current.length() >= 2) {
                bool is_number = true;
                for (char ch : current) {
                    if (!isDigit(ch)) {
                        is_number = false;
                        break;
                    }
                }
                
                if (!is_number) {
                    Token token;
                    token.text = toLower(current);
                    token.position = position;
                    tokens.push_back(token);
                    
                    total_length += token.text.length();
                }
            }
            
            current.clear();
            position = i + 1;
            ++i;
        }
    }
    
    if (current.length() >= 2) {
        bool is_number = true;
        for (char ch : current) {
            if (!isDigit(ch)) {
                is_number = false;
                break;
            }
        }
        
        if (!is_number) {
            Token token;
            token.text = toLower(current);
            token.position = position;
            tokens.push_back(token);
            
            total_length += token.text.length();
        }
    }
    
    stats.total_tokens += tokens.size();
    if (stats.total_tokens > 0) {
        stats.avg_length = static_cast<double>(total_length) / stats.total_tokens;
    }
    
    return tokens;
}

TokenizerStats Tokenizer::getStats() const {
    return stats;
}
