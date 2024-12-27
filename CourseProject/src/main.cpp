#include <First_fit.h>
#include <Buddy.h>

#include <sys/mman.h>
#include <chrono>
#include <vector>
#include <thread>
#include <random>

#define NOW std::chrono::high_resolution_clock::now()

template <typename T>
void print(T st)
{
    std::cout << st << '\n';
}

int main()
{
    print("Creating allocators..");

    size_t size = 10 * 1024 * 1024;
    //MAP_PRIVATE | MAP_ANONYMOUS: Память анонимная и изменения видны только текущему процессу
    void *mem1 = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    void *mem2 = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    Allocator_FirstFit al1(mem1, size);
    Allocator_Buddy al2(mem2, size);
    std::cout << "All memory: " << al1.get_free_memory() << ' ' << al2.get_free_memory() << '\n';

    print("Created");

    print("TEST 1 -- allocate lots of memory of a single little size");
    int cycl_count = 5;
    std::cout << "Cycles count = " << cycl_count << "\n\n";
    int blocks_count = 10000;
    int t1_sz[cycl_count] = {1, 4, 8, 16, 32, 64};

    for (int i = 0; i < cycl_count; i++)
    {
        std::cout << "Block size = " << t1_sz[i] << " Block count = " << blocks_count << std::endl;

        print("Testing...");
        /////////////allocate
        std::vector<void *> ptrs1(blocks_count);
        auto start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < blocks_count; j++)
        {
            ptrs1[j] = al1.alloc(1 << t1_sz[i]);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::vector<void *> ptrs2(blocks_count);
        start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < blocks_count; j++)
        {
            ptrs2[j] = al2.alloc(1 << t1_sz[i]);
        }
        end = std::chrono::high_resolution_clock::now();
        auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        /////////////free
        start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < blocks_count; j++)
        {
            al1.free(ptrs1[j]);
        }
        end = std::chrono::high_resolution_clock::now();
        auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < blocks_count; j++)
        {
            al2.free(ptrs2[j]);
        }
        end = std::chrono::high_resolution_clock::now();
        auto duration4 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Result Allocate:\nFirst-fit: " << duration1 << " Buddy:" << duration2 << " " << float(duration1.count()) / float(duration2.count()) << "\n";
        std::cout << "Result Free:\nFirst-fit: " << duration3 << " Buddy:" << duration4 << " " << float(duration3.count()) / float(duration4.count()) << "\n\n";
    }

    {
        print("\nTEST 2 -- allocate a lot of memory in the face of high fragmentation");
        print("Allocate 5000 blocks size 100...");

        int blocks_count = 10000;
        std::vector<void *> frament_blocks1(blocks_count);

        for (int i = 0; i < blocks_count; ++i)
        {
            frament_blocks1[i] = al1.alloc(100);
        }
        print("Free all even blocks alloc1...");
        for (int i = 0; i < blocks_count; ++i)
        {
            if (i % 2 == 0)
                al1.free(frament_blocks1[i]);
        }

        std::vector<void *> frament_blocks2(blocks_count);

        for (int i = 0; i < blocks_count; ++i)
        {
            frament_blocks2[i] = al2.alloc(100);
        }
        print("Free all even blocks alloc2...");
        for (int i = 0; i < blocks_count; ++i)
        {
            if (i % 2 == 0)
                al2.free(frament_blocks2[i]);
        }
        print("Fragmentation completed");

        std::cout << "Block size = " << 80 << " Block count = " << blocks_count / 2 << std::endl;

        print("Testing...");
        /////////////allocate
        std::vector<void *> ptrs1(blocks_count / 2);
        auto start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < blocks_count / 2; j++)
        {
            ptrs1[j] = al1.alloc(70);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::vector<void *> ptrs2(blocks_count / 2);
        start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < blocks_count / 2; j++)
        {
            ptrs2[j] = al2.alloc(80);
        }
        end = std::chrono::high_resolution_clock::now();
        auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        /////////////free
        start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < blocks_count / 2; j++)
        {
            al1.free(ptrs1[j]);
        }
        end = std::chrono::high_resolution_clock::now();
        auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        start = std::chrono::high_resolution_clock::now();
        for (int j = 0; j < blocks_count / 2; j++)
        {
            al2.free(ptrs2[j]);
        }
        end = std::chrono::high_resolution_clock::now();
        auto duration4 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

        std::cout << "Result Allocate:\nFirst-fit: " << duration1 << " Buddy:" << duration2 << " " << float(duration1.count()) / float(duration2.count()) << "\n";
        std::cout << "Result Free:\nFirst-fit: " << duration3 << " Buddy:" << duration4 << " " << float(duration3.count()) / float(duration4.count()) << "\n\n";

        for (int i = 0; i < blocks_count; ++i)
        {
            if (i % 2 != 0) // Free only the odd blocks, as even blocks were already freed
            {
                al1.free(frament_blocks1[i]);
                al2.free(frament_blocks2[i]);
            }
        }
    }
    {
        print("\nTEST 3 -- allocate random blocks");
        int num_iterations = 5;
        int num_blocks = 10000; // Use a lot of blocks
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> distrib(4, 1024); // Random block sizes up to 1KB

        for (int iter = 0; iter < num_iterations; ++iter)
        {
            std::vector<size_t> sizes(num_blocks);
            std::vector<void *> ptrs1(num_blocks);
            std::vector<void *> ptrs2(num_blocks);

            // Generate random block sizes
            for (int i = 0; i < num_blocks; ++i)
            {
                sizes[i] = distrib(gen);
            }

            auto start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < num_blocks; ++i)
            {
                ptrs1[i] = al1.alloc(sizes[i]);
            }
            auto end = std::chrono::high_resolution_clock::now();
            auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < num_blocks; ++i)
            {
                ptrs2[i] = al2.alloc(sizes[i]);
            }
            end = std::chrono::high_resolution_clock::now();
            auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < num_blocks; ++i)
            {
                al1.free(ptrs1[i]);
            }
            end = std::chrono::high_resolution_clock::now();
            auto duration3 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            start = std::chrono::high_resolution_clock::now();
            for (int i = 0; i < num_blocks; ++i)
            {
                al2.free(ptrs2[i]);
            }
            end = std::chrono::high_resolution_clock::now();
            auto duration4 = std::chrono::duration_cast<std::chrono::microseconds>(end - start);

            std::cout << "Iteration " << iter + 1 << ":\n";
            std::cout << "Result Allocate:\nFirst-fit: " << duration1 << " Buddy:" << duration2 << " " << (duration1.count() > 0 && duration2.count() > 0 ? float(duration1.count()) / float(duration2.count()) : 0) << "\n";
            std::cout << "Result Free:\nFirst-fit: " << duration3 << " Buddy:" << duration4 << " " << (duration3.count() > 0 && duration4.count() > 0 ? float(duration3.count()) / float(duration4.count()) : 0) << "\n\n";
        }
    }

    {
        print("\nTEST 4 -- Usage factor First-fit");
        const int num_requests = 10000;    // Количество запросов на выделение памяти
        const size_t max_block_size = 512; // Максимальный размер блока

        std::uniform_int_distribution<size_t> dist(16, max_block_size); // Равномерное распределение
        std::random_device rd;
        std::mt19937 gen(rd());

        size_t total_allocated = 0;
        size_t memory_size = al1.get_free_memory();
        std::cout << "All memory: " << memory_size << '\n';
        std::vector<void *> ptrs(num_requests);
        for (int i = 0; i < num_requests; ++i)
        {
            size_t req_size = dist(gen);
            ptrs[i] = al1.alloc(req_size);

            total_allocated += req_size;
        }

        double utilization_factor = (double)(total_allocated) / (memory_size - al1.get_free_memory());

        std::cout << "Total allocated: " << total_allocated << " bytes" << std::endl;
        std::cout << "Free memory: " << al1.get_free_memory() << " bytes" << std::endl;
        std::cout << "Utilization factor: " << utilization_factor << std::endl;
    }
    {
        print("\nTEST 5 -- Usage factor Buddy");
        const int num_requests = 10000;    // Количество запросов на выделение памяти
        const size_t max_block_size = 512; // Максимальный размер блока

        std::uniform_int_distribution<size_t> dist(16, max_block_size); // Равномерное распределение
        std::random_device rd;
        std::mt19937 gen(rd());

        size_t total_allocated = 0;
        size_t memory_size = al2.get_free_memory();
        std::cout << "All memory: " << memory_size << '\n';
        std::vector<void *> ptrs(num_requests);
        for (int i = 0; i < num_requests; ++i)
        {
            size_t req_size = dist(gen);
            ptrs[i] = al2.alloc(req_size);

            total_allocated += req_size;
        }

        double utilization_factor = (double)(total_allocated) / (memory_size - al2.get_free_memory());

        std::cout << "Total allocated: " << total_allocated << " bytes" << std::endl;
        std::cout << "Free memory: " << al2.get_free_memory() << " bytes" << std::endl;
        std::cout << "Utilization factor: " << utilization_factor << std::endl;
    }

    // Освобождаем память
    munmap(mem1, size);
    munmap(mem2, size);
}
