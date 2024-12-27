#include <Buddy.h>

int Allocator_Buddy::log2_top(size_t x) // логарифм с округлением вверх
{
    int log2 = 0;
    size_t x2 = x;
    while (x >>= 1)
    {
        log2++;
    }
    if (x2 != 1 << log2)
    {
        log2++;
    }
    return log2;
}

int Allocator_Buddy::log2_down(size_t x) // логарифм с округлением вниз
{
    int log2 = 0;
    while (x >>= 1)
    {
        log2++;
    }
    return log2;
}

void Allocator_Buddy::print(std::string st) // принтер очередей
{
    std::cout << "Start print " << st << "\n";
    for (int i = 0; i < levels - MINLEVEL; ++i)
    {
        std::cout << "Que " << i << " - ";
        Block *cur = queues[i];
        while (cur != nullptr)
        {
            std::cout << size_t(cur) - size_t(buffer) << " ";
            cur = cur->next;
        }
        std::cout << std::endl;
    }
}

size_t Allocator_Buddy::get_free_memory()
{
    long ans = 0;
    for (int i = 0; i < levels - MINLEVEL; ++i)
    {
        Block *cur = queues[i];
        while (cur != nullptr)
        {
            ans += (1 << (levels - i));
            cur = cur->next;
        }
    }
    return ans;
}

Allocator_Buddy::Allocator_Buddy(void *realMemory, size_t memory_size)
{
    levels = log2_down(memory_size);
    int q_mem = sizeof(Block) * (levels - MINLEVEL + 1);
    levels = log2_down(memory_size - q_mem);
    queues = (Block **)realMemory;

    buffer = (char *)realMemory + q_mem;
    queues[0] = (Block *)buffer;
    queues[0]->next = nullptr;
    for (int i = 1; i <= levels - MINLEVEL; ++i)
        queues[i] = nullptr;
}

void *Allocator_Buddy::alloc(size_t block_size)
{
    if (levels < log2_top(block_size + sizeof(Block))) // если попросили больше чем размер аллокатора
        return nullptr;
    size_t required_level = levels - log2_top(block_size + sizeof(Block)); // определяем какой уровень нам нужен
    if (required_level > levels - MINLEVEL)                                // округляем маленький запрос до размера минимального блока в аллокаторе
        required_level = levels - MINLEVEL;

    if (queues[required_level] != nullptr) // в нужной очередь есть подходящий блок
    {
        Block *ans = (Block *)queues[required_level];
        queues[required_level] = queues[required_level]->next;
        ans->size = required_level;
        return ans + 1;
    }

    // не повезло, находим минимальный  непустой уровень
    int i = required_level - 1;
    for (i; i >= 0; --i)
        if (queues[i] != nullptr) // опааа, нашлии очередь с блоком свободным
            break;

    if (i == -1)        // нет свободных блоков
        return nullptr; // памяти нет :(

    while (queues[required_level] == nullptr) // пока в нужной очереди не появились блоки делим более большие
    {
        Block *big = queues[i]; // убираем из очереди голову
        queues[i] = queues[i]->next;

        Block *bro1, *bro2;
        bro1 = big;
        bro2 = (Block *)((char *)big + (1 << (levels - i)) / 2);

        bro1->next = bro2;
        bro2->next = queues[i + 1];
        queues[i + 1] = bro1;

        ++i;
    }

    // вышли из цикла, значит доделили что в нашей очереди есть блоки. Возвращаем его
    Block *ans = (Block *)queues[required_level];
    queues[required_level] = queues[required_level]->next;
    ans->size = required_level;
    return ans + 1;
}

bool Allocator_Buddy::is_bro(Block *bro1, Block *bro2, int teck_level)
{
    size_t tbro1 = size_t(bro1) - size_t(buffer);
    size_t tbro2 = size_t(bro2) - size_t(buffer);

    size_t mask = 1 << (levels - teck_level);
    mask = ~mask;
    return (tbro1 & mask) == (tbro2 & mask);
}

void Allocator_Buddy::free(void *block)
{
    if (block == nullptr)
        return;
    Block *cur = (Block *)block - 1; // поняли в каком мы блоке сейчас находимся
    int teck_level = cur->size;      // нашли текущий уровень

    cur->next = queues[teck_level];
    queues[teck_level] = cur; // добавили в очередь свободных блоков нужного размера

    // теперь проходимся вверх и пытаемся соеденить cur с двойником
    bool bro_find = true;
    while (bro_find && teck_level >= 0)
    {
        bro_find = false;
        cur = queues[teck_level];
        Block *prev = queues[teck_level];
        while (prev != nullptr)
        {
            if (is_bro(cur, prev->next, teck_level))
            {
                bro_find = true;
                Block *uni = std::min(prev->next, cur);
                prev->next = prev->next->next;
                queues[teck_level] = cur->next;

                uni->next = queues[teck_level - 1];
                queues[teck_level - 1] = uni;
                break;
            }
            prev = prev->next;
        }
        --teck_level;
    }
}