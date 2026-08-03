#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <sys/stat.h>

struct Size
{
    int M, K, N;
};

static void write_case(const std::string &path, int M, int K, int N, std::mt19937 &rng)
{
    std::uniform_int_distribution<int> dist(1, 9); // uniform distribution from 1 to 9
    std::ofstream f(path);
    f << M << " " << K << " " << N << "\n";
    for (int i = 0; i < M; ++i)
    {
        for (int j = 0; j < K; ++j)
        {
            f << dist(rng);
            if (j + 1 < K)
                f << ' ';
        }
        f << "\n";
    }
    for (int i = 0; i < K; ++i)
    {
        for (int j = 0; j < N; ++j)
        {
            f << dist(rng);
            if (j + 1 < N)
                f << ' ';
        }
        f << "\n";
    }
}

int main(int argc, char **argv)
{
    std::string out_dir = (argc > 1) ? argv[1] : ".";
    mkdir(out_dir.c_str(), 0755); // no-op if it already exists

    std::mt19937 rng(71); // seeding for the random number generator

    std::vector<Size> sizes = {
        {2, 3, 2}, // MKN values for the test cases
        {10, 10, 10},
        {64, 64, 64},
        {128, 128, 128},
        {256, 256, 256},
        {512, 512, 512},
    };

    for (size_t i = 0; i < sizes.size(); ++i)
    {
        char name[64];
        std::snprintf(name, sizeof(name), "gemm_test_%02zu.txt", i + 1); 
        std::string path = out_dir + "/" + name;
        write_case(path, sizes[i].M, sizes[i].K, sizes[i].N, rng);
        std::printf("wrote %s (%dx%d * %dx%d)\n", path.c_str(),
                    sizes[i].M, sizes[i].K, sizes[i].K, sizes[i].N);
    }

    return 0;
}
