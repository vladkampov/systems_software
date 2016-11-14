#pragma ignored "-Wunused-variable"

#include "Memory.h"


int main() {
    Memory allocator = Memory(7,16);
    char* a = (char *) allocator.mem_alloc(8);
    for (int i = 0; i < 5; i++)
        a[i] = i+1;
    allocator.mem_dump();
    char* b = (char *) allocator.mem_alloc(3);
    for (int i = 0; i < 3; i++)
        b[i] = i+10;
    allocator.mem_dump();
    char* c = (char *) allocator.mem_alloc(2);
    for (int i = 0; i < 2; i++)
        c[i] = i+20;
    allocator.mem_dump();

    char* d = (char *) allocator.mem_alloc(6);
    for (int i = 0; i < 2; i++)
        d[i] = i+30;
    allocator.mem_dump();

    cout << "!!!!!" << endl;
    //a = (char *) allocator.mem_realloc(static_cast<void *> (a), 3);
    a = (char *) allocator.mem_realloc(static_cast<void *> (a), 10);
    b = (char *) allocator.mem_realloc(static_cast<void*> (b), 8);
    c = (char *) allocator.mem_realloc(static_cast<void*> (c), 10);
    allocator.mem_dump();
    allocator.mem_free(static_cast<void*> (c));
    allocator.mem_free(static_cast<void*> (b));
    allocator.mem_free(static_cast<void*> (a));
    allocator.mem_dump();
    return 0;
}