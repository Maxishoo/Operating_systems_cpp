#include <iostream>
#include <string>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    const char *file_name(argv[1]);
    const char *semaphore_name(argv[2]);
    const char *shared_mem_name(argv[3]);

    int fd = open(file_name, O_RDWR | O_CREAT | O_TRUNC);

    const int SIZE = 4096; // Размер разделяемой памяти
    // Открытие семафора
    sem_t *sem = sem_open(semaphore_name, O_RDWR);
    if (sem == SEM_FAILED)
    {
        std::cerr << "Ошибка открытия семафора" << std::endl;
        exit(1);
    }

    // Открытие shared memory
    int shm_fd = shm_open(shared_mem_name, O_RDWR, 0666);
    if (shm_fd == -1)
    {
        std::cerr << "Ошибка открытия shared memory" << std::endl;
        exit(1);
    }

    // Сопоставление shared memory с памятью процесса
    void *ptr = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (ptr == MAP_FAILED)
    {
        std::cerr << "Ошибка сопоставления shared memory" << std::endl;
        exit(1);
    }

    while (1)
    {
        if (sem_wait(sem) == -1)
        {
            std::cerr << "Ошибка ожидания семафора" << std::endl;
            exit(1);
        }
        std::string st_in((char *)ptr);
        sem_post(sem);

        if (st_in == "!q")
            break;

        std::string st_out = "";
        for (int i = 0; i < st_in.size(); i++)
        {
            if (!(st_in[i] == 'a' || st_in[i] == 'e' || st_in[i] == 'i' || st_in[i] == 'o' || st_in[i] == 'u' ||
                  st_in[i] == 'A' || st_in[i] == 'E' || st_in[i] == 'I' || st_in[i] == 'O' || st_in[i] == 'U'))
            {
                st_out += st_in[i];
            }
        }
        st_out += "\n";
        write(fd, st_out.c_str(), st_out.size());

        sleep(1/8);
    }
    munmap(ptr, SIZE);
    close(shm_fd);
    sem_close(sem);
    sem_unlink(semaphore_name);
    close(fd);
    return 0;
}