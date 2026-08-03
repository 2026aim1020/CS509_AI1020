#include "csr.h"
#include <chrono>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Error: missing input file.\n"
                  << "Usage: " << argv[0] << " <graph_input_file>\n";
        return 1;
    }

    std::string path = argv[1];
  
    AdjacencyList g;

    // --- file reading / parsing: NOT timed ---
    if (!read_adjacency_list(path, g))
    {
        std::cerr << "Error: could not read or parse input file '" << path << "'.\n";
        return 1;
    }


    auto t1 = std::chrono::high_resolution_clock::now();
    CSR csr = build_csr(g);
    auto t2 = std::chrono::high_resolution_clock::now();
    double convert_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

    std::cout << "Graph: V=" << g.V << " E=" << g.E
              << " weighted=" << (g.weighted ? "yes" : "no")
              << " source=" << g.source << "\n";
    std::cout << "CSR entries stored: " << csr.E << "\n";

   
    print_csr(csr);
    std::cout << "CSR conversion time: " << convert_ms << " ms\n";

    return 0;
}
