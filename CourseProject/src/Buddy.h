#pragma once
#include <iostream>
#include <string>

#define MINLEVEL 4

class Allocator_Buddy
{
    union Block
    {
        int size;
        Block *next;
    };

private:
    void *buffer; // буффер
    Block **queues;
    int levels;

    bool is_bro(Block* bro1, Block*bro2, int teck_level);
    int log2_top(size_t x);
    int log2_down(size_t x);

public:
    Allocator_Buddy(void *realMemory, size_t memory_size);

    void *alloc(size_t block_size);

    void free(void *block);

    void print(std::string st);

    size_t get_free_memory();
};