#include <First_fit.h>

size_t Allocator_FirstFit::get_free_memory()
{
    size_t ans=0;
    Block *cur = head;
    while (cur != nullptr)
    {
        if (cur->free)
            ans += cur->size;
        cur = cur->next;
    }
    return ans;
}

Allocator_FirstFit::Allocator_FirstFit(void *realMemory, size_t memory_size)
{
    head = (Block *)(realMemory);

    head->free = true;
    head->next = nullptr;
    head->size = memory_size - sizeof(Block);
}

void *Allocator_FirstFit::alloc(size_t block_size)
{
    Block *current = head;
    while (current != nullptr)
    {
        if (current->free && current->size >= block_size)
        {
            if (current->size > block_size + sizeof(Block))
            {
                Block *freeBlock = (Block *)((char *)current + sizeof(Block) + block_size);
                freeBlock->size = current->size - block_size - sizeof(Block);
                freeBlock->next = current->next;
                freeBlock->free = true;

                current->size = block_size;
                current->next = freeBlock;
                current->free = false;
                return (void *)(current + 1);
            }
            else
            {
                current->free = false;
                return (void *)(current + 1);
            }
        }
        current = current->next;
    }
    return nullptr; // у аллокатора закончилась память
}

void Allocator_FirstFit::free(void *block)
{
    if (block == nullptr)
        return;

    Block *current = (Block *)block - 1;
    current->free = true;

    // Пытаемся объеденить с предыдущим свободным блоком
    Block *previous = head; // находим предыдущего
    if (current != previous)
    {
        while (previous->next != current)
        {
            previous = previous->next;
        }
        if (previous->free) // пред свободен->объединяем
        {
            previous->size += sizeof(Block) + current->size;
            previous->next = current->next;
            current = previous;
        }
    }

    // Пытаемся объединенить со следующим свободным блоком
    if (current->next != nullptr && current->next->free)
    {
        current->size += sizeof(Block) + current->next->size;
        current->next = current->next->next;
    }
}