#include <iostream>
#include <string>
#include <random>

int main()
{
    std::string st_in, st_out;

    std::cin>>st_in;
    while (st_in!="q")
    {
        st_out = "";
        for (int i = 0; i < st_in.size(); i++)
        {
            if (!(st_in[i] == 'a' || st_in[i] == 'e' || st_in[i] == 'i' || st_in[i] == 'o' || st_in[i] == 'u' ||
                st_in[i] == 'A' || st_in[i] == 'E' || st_in[i] == 'I' || st_in[i] == 'O' || st_in[i] == 'U'))
            {
                st_out += st_in[i];
            }
        }
        std::cout << st_out<<'\n';
        std::cin>>st_in;
    }
    return 0;
}