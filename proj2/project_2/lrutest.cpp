#include "lrucache.hpp"
#include <iostream>


int main(int argc, char *argv[])
{
    LRU l;
    l.access(0);
    l.access(1);
    l.access(2);
    l.access(3);
    std::cout << l.getLRU() << std::endl;
    l.access(0);
    std::cout << l.getLRU() << std::endl;
    l.access(3);
    std::cout << l.getLRU() << std::endl;
    l.access(1);
    std::cout << l.getLRU() << std::endl;
    
    return 0;
}
