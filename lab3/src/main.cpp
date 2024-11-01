#include <iostream>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <unistd.h>
#include <semaphore.h>

// • O_RDWR: Флаг для открытия файла для чтения и записи.  O_RDONLY | O_WRONLY == O_RDWR
// • O_CREAT: Флаг, который создает файл, если он не существует.
// • O_TRUNC: Флаг, который обрезает файл до нулевого размера, если он уже существует.

int main()
{
    std::string f1, f2;
    std::cout << "Enter file name for child1: ";
    std::cin >> f1;
    std::cout << "Enter file name for child2: ";
    std::cin >> f2;

    const int SIZE = 4096; // size in bytes
    std::cout << "All good\n";
    // create first shared memory file
    const char *name1 = "OS1"; // shared name
    int shm_fd1;               // shared memory file descriptor
    void *ptr1;                // pointer to shared memory object
    shm_fd1 = shm_open(name1, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd1, SIZE);
    ptr1 = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd1, 0);

    // create second shared memory file
    const char *name2 = "OS2"; // shared name
    int shm_fd2;
    void *ptr2;
    shm_fd2 = shm_open(name2, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd2, SIZE);
    ptr2 = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd2, 0);

    // Создаем семафоры
    const char *sem1_name = "/sem1";
    const char *sem2_name = "/sem2";

    sem_t *sem1 = sem_open(sem1_name, O_CREAT | O_EXCL, 0666, 0);
    sem_t *sem2 = sem_open(sem2_name, O_CREAT | O_EXCL, 0666, 0);

    // создаем детей
    pid_t ch1 = fork();
    switch (ch1)
    {
    case -1: // error
        std::cout << "Ошибка при выполнении fork 1";
        exit(1);
        break;
    case 0: // child1 code
        execl("./child", "child1", f1.c_str(), sem1_name, name1, NULL);
        std::cout << "Ошибка при вызове execl 1";
        return 1;
    default: // dad
        pid_t ch2 = fork();
        switch (ch2)
        {
        case -1: // error
            std::cout << "Ошибка при выполнении fork 1";
            exit(1);
            break;
        case 0: // child2 code
            execl("./child", "child2", f2.c_str(), sem2_name, name2, NULL);
            std::cout << "Ошибка при вызове execl 2";
            return 1;
        default: // dady code:)
            std::string st;
            std::cout << "Enter linens. To finish entering lines, enter '!q'.\n";
            while (1)
            {
                std::cin >> st;
                int len = st.size() + 1;
                if (st == "!q")
                    break;

                if (rand() % 100 <= 80)
                {
                    strcpy((char *)ptr1, st.c_str());
                    sem_post(sem1); // Сигнализируем, что файл свободен
                    sleep(1/8);
                    sem_wait(sem1);
                }
                else
                {
                    strcpy((char *)ptr2, st.c_str());
                    sem_post(sem2); // Сигнализируем, что файл свободен
                    sleep(1/8);
                    sem_wait(sem2);
                }
            }

            // Отправляем сигнал завершения детям
            st = "!q";

            // Первый ребенок
            strcpy((char *)ptr1, st.c_str());
            sem_post(sem1);
            sleep(1/8);
            sem_wait(sem1);

            // Второй ребенок
            strcpy((char *)ptr2, st.c_str());
            sem_post(sem2);
            sleep(1/8);
            sem_wait(sem2);

            // Освобождение ресурсов
            munmap(ptr1, SIZE);
            close(shm_fd1);
            shm_unlink(name1); // Удаление разделяемой памяти после завершения работы

            munmap(ptr2, SIZE);
            close(shm_fd2);
            shm_unlink(name2);

            // Закрытие семафоров
            sem_close(sem1);
            sem_close(sem2);
            sem_unlink("/sem1");
            sem_unlink("/sem2");
            break;
        }
        break;
    }
    return 0;
}