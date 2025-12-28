#include "mongo_reader.h"
#include <iostream>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>

std::vector<DocumentData> MongoReader::readFromMongoDB(
    const std::string& uri,
    const std::string& db_name,
    const std::string& collection_name) {
    
    std::vector<DocumentData> documents;
    
    try {
        static mongocxx::instance instance{};
        
        mongocxx::client client{mongocxx::uri{uri}};
        auto db = client[db_name];
        auto collection = db[collection_name];
        
        std::cout << "Connected to MongoDB: " << uri << std::endl;
        
        int64_t total = collection.count_documents({});
        std::cout << "Total documents in collection: " << total << std::endl;
        
        auto cursor = collection.find({});
        
        int count = 0;
        for (auto&& doc : cursor) {
            DocumentData data;
            
            auto url_elem = doc["url"];
            if (url_elem && url_elem.type() == bsoncxx::type::k_utf8) {
                data.url = std::string(url_elem.get_utf8().value);
            }
            
            auto html_elem = doc["html_content"];
            if (html_elem && html_elem.type() == bsoncxx::type::k_utf8) {
                data.html_content = std::string(html_elem.get_utf8().value);
            }
            
            auto source_elem = doc["source"];
            if (source_elem && source_elem.type() == bsoncxx::type::k_utf8) {
                data.source = std::string(source_elem.get_utf8().value);
            }
            
            if (!data.url.empty() && !data.html_content.empty()) {
                documents.push_back(data);
                count++;
                
                if (count % 1000 == 0) {
                    std::cout << "Loaded " << count << "/" << total << " documents" << std::endl;
                }
            }
        }
        
        std::cout << "Successfully loaded " << documents.size() << " documents from MongoDB" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "MongoDB error: " << e.what() << std::endl;
    }
    
    return documents;
}

std::string MongoReader::decodeEntities(const std::string& text) {
    std::string result = text;
    
    struct Entity {
        std::string encoded;
        std::string decoded;
    };
    
    std::vector<Entity> entities = {
        {"&nbsp;", " "},
        {"&amp;", "&"},
        {"&lt;", "<"},
        {"&gt;", ">"},
        {"&quot;", "\""},
        {"&#39;", "'"},
        {"&mdash;", "—"},
        {"&ndash;", "–"}
    };
    
    for (const auto& entity : entities) {
        size_t pos = 0;
        while ((pos = result.find(entity.encoded, pos)) != std::string::npos) {
            result.replace(pos, entity.encoded.length(), entity.decoded);
            pos += entity.decoded.length();
        }
    }
    
    return result;
}

std::string MongoReader::stripHTML(const std::string& html) {
    std::string result;
    bool in_tag = false;
    bool in_script = false;
    bool in_style = false;
    
    for (size_t i = 0; i < html.length(); ++i) {
        if (html[i] == '<') {
            in_tag = true;
            
            if (i + 6 < html.length() && html.substr(i, 7) == "<script") {
                in_script = true;
            } else if (i + 5 < html.length() && html.substr(i, 6) == "<style") {
                in_style = true;
            }
        } else if (html[i] == '>') {
            in_tag = false;
            
            if (i >= 8 && html.substr(i - 8, 9) == "</script>") {
                in_script = false;
            } else if (i >= 7 && html.substr(i - 7, 8) == "</style>") {
                in_style = false;
            }
            
            result += " ";
        } else if (!in_tag && !in_script && !in_style) {
            result += html[i];
        }
    }
    
    return decodeEntities(result);
}