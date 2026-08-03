
#include "gemm.h"
#include <chrono>
#include <iostream>
#include <string>
#include <cmath>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Error: missing input file.\n"
                  << "Usage: " << argv[0] << " <input_file> [block_size]\n";
        return 1;
    }

    std::string path = argv[1];
    int block_size = (argc >= 3) ? std::stoi(argv[2]) : 32;

    int M, K, N;
    Matrix A, B;

    // --- setup / I/O: NOT timed ---
    if (!read_gemm_input(path, M, K, N, A, B))
    {
        std::cerr << "Error: could not read or parse input file '" << path << "'.\n";
        return 1;
    }

    Matrix C_simple, C_blocking;

    // GEMM Simple
    auto t1 = std::chrono::high_resolution_clock::now();
    gemm_simple(A, B, C_simple);
    auto t2 = std::chrono::high_resolution_clock::now();
    double simple_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

    //GEMM Blocking
    auto t3 = std::chrono::high_resolution_clock::now();
    gemm_blocking(A, B, C_blocking, block_size);
    auto t4 = std::chrono::high_resolution_clock::now();
    double blocking_ms = std::chrono::duration<double, std::milli>(t4 - t3).count();

    std::cout << "Algorithm: GEMM Simple\n";
    std::cout << "Result matrix:\n";
    print_matrix(C_simple);
    std::cout << "Execution time: " << simple_ms << " ms\n\n";

    std::cout << "Algorithm: GEMM Blocking\n";
    std::cout << "Result matrix:\n";
    print_matrix(C_blocking);
    std::cout << "Execution time: " << blocking_ms << " ms\n";
    std::cout << "Block size: " << block_size << "\n";

   // checking result is same
    bool match = (C_simple.rows == C_blocking.rows && C_simple.cols == C_blocking.cols);
    if (match)
    {
        for (size_t idx = 0; idx < C_simple.data.size() && match; ++idx)
        {
            if (std::abs(C_simple.data[idx] - C_blocking.data[idx]) > 1e-6)
                match = false;
        }
    }
    if (!match)
    {
        std::cerr << "\nWARNING: simple and blocking results do NOT match!\n";
    }

    return 0;
}
