#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <random>
#include <fcntl.h>

// return (rand() % 100 <= 80) ? 1 : 2;
// fd[0] - read, fd[1] - write

// Запись, создание, обрезание файла
// O_WRONLY | O_CREAT | O_TRUNC

// Права доступа (чтение, запись, выполнение) для владельца, группы, всех
// S_IRUSR | S_IRGRP | S_IROTH
int main()
{
    std::string f1, f2;
    std::cout << "Enter file name for child1: ";
    std::cin >> f1;
    std::cout << "Enter file name for child2: ";
    std::cin >> f2;

    int file1_desc = open(f1.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    int file2_desc = open(f2.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0666);
    // Создаём каналы
    int pipe1[2], pipe2[2];
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1)
    {
        std::cout << "Ошибка при создании pipe";
        exit(1);
    }

    pid_t ch1 = fork();
    switch (ch1)
    {
    case -1:
        std::cout << "Ошибка при выполнении fork 1";
        exit(1);
        break;
    case 0:
        // Закрываем ненужные дескрипторы
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(file2_desc);
        // Перенаправление stdin на pipe и stdout на file1 для child 1
        if (dup2(pipe1[0], STDIN_FILENO) == -1) {
            std::cout << "Ошибка при перенаправлении stdin 1";
            return 1;
        }
        if (dup2(file1_desc, STDOUT_FILENO) == -1) {
            std::cout << "Ошибка при перенаправлении stdout 1";
            return 1;
        }
        close(pipe1[0]);
        close(file1_desc);
        // Запуск дочернего процесса
        execl("./child", "child1", NULL);
        std::cout << "Ошибка при вызове execl 1";
        return 1;
        break;
    default:
        pid_t ch2 = fork();
        switch (ch2)
        {
        case -1:
            std::cout << "Ошибка при выполнении fork 2";
            exit(1);
            break;
        case 0:
            close(pipe2[1]);
            close(pipe1[0]);
            close(pipe1[1]);
            close(file1_desc);
            // Перенаправление stdin на pipe и stdout на file2 для child 2
            if (dup2(pipe2[0], STDIN_FILENO) == -1) {
                std::cout << "Ошибка при перенаправлении stdin 2";
                return 1;
            }
            if (dup2(file2_desc, STDOUT_FILENO) == -1) {
                std::cout << "Ошибка при перенаправлении stdout 2";
                return 1;
            }
            close(pipe2[0]);
            close(file2_desc);
            // Запуск дочернего процесса
            execl("./child", "child2", NULL);
            std::cout << "Ошибка при вызове execl 2";
            return 1;
        default: // Код родительского процесса
            // Закрываем дескрипторы для записи в пайпы
            close(pipe1[0]);
            close(pipe2[0]);
            close(file1_desc);
            close(file2_desc);
            std::string st = "";
            std::cout << "Enter linens. To finish entering lines, enter 'q'.\n";
            std::cin >> st;
            while (st != "q")
            {
                int len = st.size() + 1;
                st += '\n';
                if (rand() % 100 <= 80)
                {
                    std::cout<<"pip1\n";
                    if (write(pipe1[1], st.c_str(), len * sizeof(char)) == -1) // отправляем строку
                    {
                        std::cout << "Ошибка при записи чисел";
                        return 1;
                    }
                }
                else
                {
                    std::cout<<"pip2\n";
                    if (write(pipe2[1], st.c_str(), len * sizeof(char)) == -1)
                    {
                        std::cout << "Ошибка при записи чисел";
                        return 1;
                    }
                    if (write(pipe2[1], "\n", 1 * sizeof(char)) == -1)
                    {
                        std::cout << "Ошибка при записи чисел";
                        return 1;
                    }
                }
                std::cin >> st;
            }

            // Отправляем сигнал завершения
            st = "q\n";
            int status;
            write(pipe1[1], st.c_str(), 2);
            write(pipe2[1], st.c_str(), 2);
            
            // Закрываем ненужные дескрипторы
            close(pipe1[1]);
            close(pipe2[1]);

            // Ждем завершения дочерних процессов
            waitpid(ch1, &status, 0);
            waitpid(ch2, &status, 0);
            break;
        }
        break;
    }
    return 0;
}