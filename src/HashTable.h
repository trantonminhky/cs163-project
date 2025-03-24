#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <vector>
#include <utility>
struct Node {
    int value;
    Node* next;
};

class HashTable {
private:
    static const int TABLE_SIZE = 7; // also change in UI.h
    std::vector<Node*> table;
    

public:
    int hashFunction(int value) const;
    HashTable();
    ~HashTable();
    void insert(int value);
    bool remove(int value);
    bool find(int value) const;
    void clear();
    void fillRandom(int count);
    const std::vector<Node*>& getTable() const { return table; }
};

#endif