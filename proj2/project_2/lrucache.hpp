#ifndef LRUCACHE_H
#define LRUCACHE_H

#include <unordered_map>
#include <iostream>

struct DoubleLL {
    struct Node {
        Node* next = nullptr;
        Node* prev = nullptr;
        int val;
        Node(Node* n, Node* p, int v): next{n}, prev{p}, val{v}{}
        Node(int v): val{v}{}
    };
    Node* head = nullptr;
    Node* tail = nullptr;

    Node* insert(int val){
        if(head == nullptr){
            head = new Node(nullptr, nullptr, val);
            tail = head;
        }
        else{
            Node* n = new Node(val);
            n->next = head;
            head->prev = n;
            head = n;
        }
        return head;
    }
    void print(){
        for(Node* n = head; n != nullptr; n = n->next)
            std::cout << n->val << "->";
        std::cout << std::endl;
    }
};

struct LRU {
    DoubleLL entries;
    std::unordered_map<int, DoubleLL::Node*> map;

    int getLRU(){
        return entries.tail->val;
    }

    void access(int val){
        if(map.count(val) == 0){
            map[val] = entries.insert(val);
        }
        else{
            DoubleLL::Node* n = map[val];
            if(n == entries.head)
                return;
            //take node n out of the list
            if(n->prev)
                n->prev->next = n->next;
            if(n->next)
                n->next->prev = n->prev;
            else if(n->prev)
                entries.tail = n->prev;
            n->next = entries.head;
            n->prev = nullptr;
            entries.head = n;
        }
    }
};

#endif
