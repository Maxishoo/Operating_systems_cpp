#include <First_fit.h>
#include <Buddy.h>

#include <sys/times.h> // for times
#include <vector>
#include <thread>
#include <random>

template <typename T>
void print(T st)
{
    std::cout << st << '\n';
}

int main()
{
    struct tms start, end;
    long clocks_per_sec = sysconf(_SC_CLK_TCK);
    long clocks;
    times(&start);

    size_t size = 10024 * 1024 * 8;
    void *mem1 = malloc(size); // выдеяем кб памяти обоим
    Allocator_Buddy al1(mem1, size);
    int blocks_count = 100000;
    std::vector<void *> ptrs1(blocks_count);
    for (int j = 0; j < 20; j++)
    {
        ptrs1[j] = al1.alloc(1);
        print(j);
    }
    times(&end);
    clocks = end.tms_utime - start.tms_utime;
    printf("Time taken: %lf sec.\n", (double)clocks / clocks_per_sec);

    // for (int j = 0; j < blocks_count; j++)
    // {
    //     al1.free(ptrs1[j]);
    // }
    free(mem1);
}
