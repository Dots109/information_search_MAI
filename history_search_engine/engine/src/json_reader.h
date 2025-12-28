#ifndef JSON_READER_H
#define JSON_READER_H

#include <string>
#include <vector>

struct DocumentData {
    std::string url;
    std::string html_content;
    std::string source;
};

class JSONReader {
public:
    static std::vector<DocumentData> readFromFile(const std::string& filename);
    static std::string stripHTML(const std::string& html);
    static std::string decodeEntities(const std::string& text);
};

#endif