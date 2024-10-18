#include <iostream>
#include <fstream>
#include <unistd.h>
#include <pthread.h>
#include <vector>

using namespace std;

int K, M, N, H, W, tk;
vector<double> A;
vector<double> B;
vector<double> S;

void print_matrix(vector<double> &mt, ostream *out, int &M, int &N)
{
    (*out) << M << " " << N << "\n";
    for (int i = 0; i < N * M; i++)
    {
        (*out) << mt[i];
        if ((i + 1) % N != 0)
            (*out) << " ";
        if (i % N == N - 1 && i != N * M - 1)
            (*out) << "\n";
    }
    (*out) << '\n';
}

void read_matrix(vector<double> &mt, istream *in, int &M, int &N)
{
    double el;
    for (int i = 0; i < N * M; i++)
    {
        (*in) >> el;
        mt.push_back(el);
    }
}

double get_x_y(int x, int y, vector<double> &mt, int &M, int &N)
{
    if (x < 0 or x >= N)
        return 0;
    if (y < 0 or y >= M)
        return 0;

    return mt[y * N + x];
}

void set_x_y(double val, int x, int y, vector<double> &mt, int &M, int &N)
{
    if (x < 0 or x >= N)
        return;
    if (y < 0 or y >= M)
        return;
    mt[y * N + x] = val;
}

void calculate_cell(int &x, int &y, vector<double> &A, vector<double> &B, vector<double> &S)
{
    double sum = 0;
    for (int i = 0; i < H; ++i)
    {
        for (int j = 0; j < W; j++)
        {
            sum += get_x_y(j, i, B, H, W) * get_x_y(x + j - W / 2, y + i - H / 2, A, M, N);
        }
    }
    set_x_y(sum, x, y, S, M, N);
}

void *Thread_func(void *args)
{
    pair<int, int> *st_end = (pair<int, int> *)args;
    int start = st_end->first;
    int end = st_end->second;
    for (int i = start; i < end; ++i)
    {
        int x, y;
        x = i % N;
        y = i / N;
        if (tk % 2 == 0)
            calculate_cell(x, y, A, B, S);
        else
            calculate_cell(x, y, S, B, A);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    char Mode = argv[2][0];

    ostream *out;
    istream *in;

    ofstream outf;
    ifstream inf;

    switch (Mode)
    {
    case 'f': // file
        inf.open("test.txt");
        in = &inf;
        outf.open("out.txt");
        out = &outf;
        break;
    case 'i': // console
        out = &cout;
        in = &cin;
        break;
    default:
        printf("incorrect mode: %c\n", Mode);
        break;
    }
    (*in) >> K;
    (*in) >> M >> N;
    read_matrix(A, in, M, N);

    (*in) >> H >> W;
    read_matrix(B, in, H, W);
    S.resize(N * M, 0);

    int THREAD_COUNT = min(atoi(argv[1]), M * N);
    pthread_t *thread_ids = new pthread_t[K]; // идентификаторы потоков

    int err = 0;
    vector<pair<int, int>> coords(THREAD_COUNT);
    for (tk = 0; tk < K; ++tk)
    {
        for (int thread = 0; thread < THREAD_COUNT; ++thread)
        {
            int p = N * M;
            coords[thread] = {thread * p / THREAD_COUNT, (thread + 1) * p / THREAD_COUNT};
            err = pthread_create(&thread_ids[thread], NULL, Thread_func, &coords[thread]);//0 if ok, other err code
            //(id, attr(приоритета потока, стека и других параметров), yказатель на функцию, args)
            if (err)
            {
                (*out) << "Error in thread create";
                exit(1);
            }
        }
        for (int thread = 0; thread < THREAD_COUNT; ++thread)
        {
            if (pthread_join(thread_ids[thread], NULL) != 0)
            { // (id, результат ф-ии потока)
                (*out) << "Error with join threads";
                exit(1);
            }
        }
    }
    if (K % 2 == 0)
        print_matrix(S, out, M, N);
    else
        print_matrix(A, out, M, N);

    delete [] thread_ids;
    if (Mode == 'f')
    {
        outf.close();
        inf.close();
    }
}
