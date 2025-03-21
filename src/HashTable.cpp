#include "HashTable.h"
#include <cstdlib>
#include <ctime>

HashTable::HashTable() {
    table.resize(TABLE_SIZE, nullptr);
}

HashTable::~HashTable() {
    clear();
}

int HashTable::hashFunction(int value) const {
    return value % TABLE_SIZE;
}

void HashTable::insert(int value) {
    if (find(value)) {
        return;
    }
    int index = hashFunction(value);
    Node* newNode = new Node{value, nullptr};
    
    if (!table[index]) {
        table[index] = newNode;
    } else {
        Node* current = table[index];
        while (current->next) {
            current = current->next;
        }
        current->next = newNode;
    }
}

bool HashTable::remove(int value) {
    int index = hashFunction(value);
    Node* current = table[index];
    Node* prev = nullptr;

    while (current) {
        if (current->value == value) {
            if (prev) {
                prev->next = current->next;
            } else {
                table[index] = current->next;
            }
            delete current;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

bool HashTable::find(int value) const {
    int index = hashFunction(value);
    Node* current = table[index];
    
    while (current) {
        if (current->value == value) {
            return true;
        }
        current = current->next;
    }
    return false;
}

void HashTable::clear() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node* current = table[i];
        while (current) {
            Node* temp = current;
            current = current->next;
            delete temp;
        }
        table[i] = nullptr;
    }
}

void HashTable::fillRandom(int count) {
    clear();
    srand(time(nullptr));
    for (int i = 0; i < count; i++) {
        insert(rand() % 100); // Random numbers 0-99
    }
}