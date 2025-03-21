#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <vector>

struct Node {
    int value;
    Node* next;
};

class HashTable {
private:
    static const int TABLE_SIZE = 17;
    std::vector<Node*> table;
    int hashFunction(int value) const;
//abcbcbabcab
public:
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