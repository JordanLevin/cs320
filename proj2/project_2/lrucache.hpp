#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>

struct DoubleLL {
    struct Node {
        Node* next = nullptr;
        Node* prev = nullptr;
        int val;
        Node(Node* n, Node* p, int v): next{n}, prev{p}, val{v}{}
        Node(int v): val{v}{}
    };
    Node* head = nullptr;

    Node* insert(int val){
        if(head == nullptr){
            head = new Node(nullptr, nullptr, val);
            tail = head;
        }
        else{
            head = new Node(head, nullptr, val);
        }
        return head;
    }
};

struct LRU {
    DoubleLL entries;
    std::unordered_map<int, DoubleLL::Node*> map;

    int getLRU(){
        return entries.tail->val;
    }

    void access(int val){
        if(map.find(val) == map.end()){
            map[val] = entries.insert(val);
        }
        else{
            DoubleLL::Node* n = map[val];
            //take node n out of the list
            if(n->prev)
                n->prev->next = n->next;
            if(n->next)
                n->next->prev = n->prev;
            n->next = head;
            n->prev = nullptr;
            entries.head = newnode;
        }
    }
};

#endif
