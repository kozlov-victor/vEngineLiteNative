#pragma once
#include <string>

template<typename U>
class VHashTable {
private:
    static const size_t TABLE_SIZE = 128; // Size of the hash table
    struct Entry {
        std::string key;
        U value;
        Entry* next;
    };
    Entry* table[TABLE_SIZE];
    size_t numElements;

    static size_t hashFunction(const std::string& key) {
        size_t hash = 0;
        for (size_t i = 0; i < key.length(); i++) {
            hash = (hash * 31) + key.c_str()[i];
        }
        return hash % TABLE_SIZE;
    }

public:
    VHashTable() :numElements(0) {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            table[i] = nullptr;
        }
    }

    ~VHashTable() {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            Entry* entry = table[i];
            while (entry != nullptr) {
                Entry* prev = entry;
                entry = entry->next;
                delete prev;
            }
        }
    }

    template<typename F>void forEach(F callback) {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            Entry* entry = table[i];
            while (entry != nullptr) {
                callback(entry->key, entry->value);
                entry = entry->next;
            }
        }
    }

    void put(const std::string& key, const U& value) {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        while (entry != nullptr) {
            if (entry->key == key) {
                entry->value = value;
                return;
            }
            entry = entry->next;
        }
        entry = new Entry{ key, value, table[hash] };
        table[hash] = entry;
        ++this->numElements;
    }

    void putIfAbsent(const std::string& key, const U& value) {
        if (!has(key)) put(key, value);
    }

    U get(const std::string& key) const {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        while (entry != nullptr) {
            if (entry->key == key) {
                return entry->value;
            }
            entry = entry->next;
        }
        printf("VHashTable::error::");
        printf("VHashTable::error::");
        printf("VHashTable::error::");
        printf("VHashTable::error::");
        printf("VHashTable::no such value: %s", key.c_str());
        printf("VHashTable::error::");
        printf("VHashTable::error::");
        printf("VHashTable::error::");
        printf("VHashTable::error::");
        return U(); // Return default value if key not found
    }

    bool has(const std::string& key) const {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        while (entry != nullptr) {
            if (entry->key == key) {
                return true;
            }
            entry = entry->next;
        }
        return false;
    }

    void remove(const std::string& key) {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        Entry* prev = nullptr;
        while (entry != nullptr) {
            if (entry->key == key) {
                if (prev != nullptr) {
                    prev->next = entry->next;
                }
                else {
                    table[hash] = entry->next;
                }
                delete entry;
                --this->numElements;
                return;
            }
            prev = entry;
            entry = entry->next;
        }
    }

    void clear() {
        for (size_t i = 0; i < TABLE_SIZE; i++) {
            Entry* entry = table[i];
            while (entry != nullptr) {
                const Entry* prev = entry;
                entry = entry->next;
                delete prev;
            }
            table[i] = nullptr;
        }
        this->numElements = 0;
    }

    U& operator[](const std::string& key) {
        size_t hash = hashFunction(key);
        Entry* entry = table[hash];
        while (entry != nullptr) {
            if (entry->key == key) {
                return entry->value;
            }
            entry = entry->next;
        }
        entry = new Entry{ key, U(), table[hash] };
        table[hash] = entry;
        ++this->numElements;
        return entry->value;
    }

    size_t size() const {
        return this->numElements;
    }

    VHashTable(const VHashTable&) = delete;
    VHashTable& operator=(const VHashTable&) = delete;

};
