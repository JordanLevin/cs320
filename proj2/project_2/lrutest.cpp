#include "lrucache.hpp"
#include <iostream>
#include <cassert>


int main(int argc, char *argv[])
{
    LRU l;
    l.access(0);
    l.access(1);
    l.access(2);
    l.access(3);
    assert(l.entries.head->val == 3);
    assert(l.entries.tail->val == 0);
    l.access(3);
    assert(l.entries.head->val == 3);
    assert(l.entries.tail->val == 0);
    l.access(2);
    assert(l.entries.head->val == 2);
    assert(l.entries.tail->val == 0);
    l.access(0);
    assert(l.entries.head->val == 0);
    assert(l.entries.tail->val == 1);
    l.access(0);
    l.access(0);
    l.access(0);
    l.access(0);
    assert(l.entries.head->val == 0);
    assert(l.entries.tail->val == 1);
    l.access(2);
    assert(l.entries.head->val == 2);
    assert(l.entries.tail->val == 1);
    return 0;
}
