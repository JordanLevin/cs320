#include "lrucache.hpp"
#include <iostream>
#include <cassert>
#include <cstdlib>

void smalllru(){
    LRU l;
    bool b;
    l.access(0);
    l.access(1);
    for(int i = 0; i < 10000; i++){
        int addr = rand()%2;
        //std::cout << "ACCESSING: " << addr << std::endl;
        l.access(addr);
        b = !addr;
        //std::cout << l.getLRU() << " " << b << std::endl;
        //std::cout << l.entries.head->prev << "->" << l.entries.head << "->"
            //<< l.entries.head->next << "->" << l.entries.head->next->next << std::endl;
        assert(b == l.getLRU());
    }

}

void biglru(){
    LRU l;
    l.access(0);
    l.access(1);
    l.access(2);
    l.access(0);
    l.access(1);
    l.access(2);
    l.access(0);
}

int main(int argc, char *argv[])
{
    biglru();
    return 0;
}

