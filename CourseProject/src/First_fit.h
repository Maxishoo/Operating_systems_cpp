#pragma once
#include <iostream>
#include <list>

class Allocator_FirstFit
{

    struct Block
    {
        size_t size;
        Block *next;
        bool free;
    };

private:
    Block *head; // голова односвязного списка блоков

public:
    Allocator_FirstFit(void *realMemory, size_t memory_size);

    void *alloc(size_t block_size);

    void free(void *block);

    size_t get_free_memory();
};