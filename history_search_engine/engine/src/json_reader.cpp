#include "json_reader.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::vector<DocumentData> JSONReader::readFromFile(const std::string& filename) {
    std::vector<DocumentData> documents;
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return documents;
    }
    
    std::string line;
    int count = 0;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        DocumentData doc;
        
        size_t url_start = line.find("\"url\":\"") + 7;
        size_t url_end = line.find("\"", url_start);
        if (url_start != std::string::npos && url_end != std::string::npos) {
            doc.url = line.substr(url_start, url_end - url_start);
        }
        
        size_t html_start = line.find("\"html_content\":\"") + 16;
        size_t html_end = line.find("\",\"source", html_start);
        if (html_start != std::string::npos && html_end != std::string::npos) {
            doc.html_content = line.substr(html_start, html_end - html_start);
        }
        
        size_t source_start = line.find("\"source\":\"") + 10;
        size_t source_end = line.find("\"", source_start);
        if (source_start != std::string::npos && source_end != std::string::npos) {
            doc.source = line.substr(source_start, source_end - source_start);
        }
        
        if (!doc.url.empty() && !doc.html_content.empty()) {
            documents.push_back(doc);
            count++;
            
            if (count % 1000 == 0) {
                std::cout << "Loaded " << count << " documents" << std::endl;
            }
        }
    }
    
    std::cout << "Total loaded: " << documents.size() << " documents" << std::endl;
    return documents;
}

std::string JSONReader::decodeEntities(const std::string& text) {
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
        {"&ndash;", "–"},
        {"\\n", " "},
        {"\\t", " "},
        {"\\r", " "}
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

std::string JSONReader::stripHTML(const std::string& html) {
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