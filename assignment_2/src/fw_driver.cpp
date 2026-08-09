// Usage:
//   ./fw_driver <matrix_input_file>
//


#include "floyd_warshall.h"
#include <chrono>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Error: missing input file.\n"
                  << "Usage: " << argv[0] << " <matrix_input_file>\n";
        return 1;
    }

    std::string path = argv[1];
    DenseMatrix M;

    if (!read_fw_input(path, M))
    {
        std::cerr << "Error: could not read or parse input file '" << path << "'.\n";
        return 1;
    }


    auto t1 = std::chrono::high_resolution_clock::now();
    FloydWarshallResult result = floyd_warshall(M);
    auto t2 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

  
    std::cout << "Algorithm: Floyd-Warshall\n";

    if (result.negative_cycle)
    {
        std::cout << "Negative cycle: true\n";
    }
    else
    {
        std::cout << "Distance matrix:\n";
        print_fw_matrix(result.distance);
        std::cout << "Negative cycle: none\n";
    }

    std::cout << "Execution time: " << ms << " ms\n";
    return 0;
}
