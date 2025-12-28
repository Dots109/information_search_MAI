#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include <vector>
#include <string>

template<typename V>
class HashTable {
private:
    struct Entry {
        std::string key;
        V value;
        bool occupied;
        bool deleted;
        
        Entry() : occupied(false), deleted(false) {}
    };
    
    std::vector<Entry> table;
    size_t capacity;
    size_t count;
    
    size_t hash1(const std::string& key) const {
        size_t h = 0;
        for (char c : key) {
            h = h * 37 + static_cast<unsigned char>(c);
        }
        return h % capacity;
    }
    
    size_t hash2(const std::string& key) const {
        size_t h = 0;
        for (char c : key) {
            h = h * 53 + static_cast<unsigned char>(c);
        }
        return (h % (capacity - 1)) + 1;
    }
    
    void rehash() {
        std::vector<Entry> old_table = table;
        capacity *= 2;
        table.clear();
        table.resize(capacity);
        count = 0;
        
        for (const auto& entry : old_table) {
            if (entry.occupied && !entry.deleted) {
                insert(entry.key, entry.value);
            }
        }
    }
    
public:
    HashTable(size_t initial_capacity = 16384) 
        : capacity(initial_capacity), count(0) {
        table.resize(capacity);
    }
    
    void insert(const std::string& key, const V& value) {
        if (count * 2 >= capacity) {
            rehash();
        }
        
        size_t h1 = hash1(key);
        size_t h2 = hash2(key);
        size_t i = 0;
        
        while (i < capacity) {
            size_t idx = (h1 + i * h2) % capacity;
            
            if (!table[idx].occupied || table[idx].deleted) {
                table[idx].key = key;
                table[idx].value = value;
                table[idx].occupied = true;
                table[idx].deleted = false;
                count++;
                return;
            }
            
            if (table[idx].key == key) {
                table[idx].value = value;
                return;
            }
            
            i++;
        }
    }
    
    bool find(const std::string& key, V& value) const {
        size_t h1 = hash1(key);
        size_t h2 = hash2(key);
        size_t i = 0;
        
        while (i < capacity) {
            size_t idx = (h1 + i * h2) % capacity;
            
            if (!table[idx].occupied) {
                return false;
            }
            
            if (!table[idx].deleted && table[idx].key == key) {
                value = table[idx].value;
                return true;
            }
            
            i++;
        }
        
        return false;
    }
    
    V* get(const std::string& key) {
        size_t h1 = hash1(key);
        size_t h2 = hash2(key);
        size_t i = 0;
        
        while (i < capacity) {
            size_t idx = (h1 + i * h2) % capacity;
            
            if (!table[idx].occupied) {
                return nullptr;
            }
            
            if (!table[idx].deleted && table[idx].key == key) {
                return &table[idx].value;
            }
            
            i++;
        }
        
        return nullptr;
    }
    
    size_t size() const { return count; }
    
    template<typename Callback>
    void iterate(Callback callback) const {
        for (const auto& entry : table) {
            if (entry.occupied && !entry.deleted) {
                callback(entry.key, entry.value);
            }
        }
    }
};

#endif
