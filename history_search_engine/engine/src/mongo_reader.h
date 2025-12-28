#ifndef MONGO_READER_H
#define MONGO_READER_H

#include <string>
#include <vector>

struct DocumentData {
    std::string url;
    std::string html_content;
    std::string source;
};

class MongoReader {
public:
    static std::vector<DocumentData> readFromMongoDB(
        const std::string& uri,
        const std::string& db_name,
        const std::string& collection_name
    );
    static std::string stripHTML(const std::string& html);
    static std::string decodeEntities(const std::string& text);
};

#endif