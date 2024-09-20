#include <iostream>
#include <unistd.h>

int main()
{
    if(!fork() || !fork())
    {
        fork();
    }
    std::cout<<1<<'\n';
}
