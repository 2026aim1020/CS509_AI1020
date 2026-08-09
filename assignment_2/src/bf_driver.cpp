
// Usage:
//   ./bf_driver <graph_input_file>



#include "../../assignment_1/include/csr.h" 
#include "../include/bellman_ford.h"                  
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>

static void print_distance(std::ostream &out, double d)
{
    if (std::isinf(d))
    {
        out << "INF";
        return;
    }
    double rounded = std::round(d);
    if (std::abs(d - rounded) < 1e-9)
        out << static_cast<long long>(rounded);
    else
    {
        out.precision(6);
        out << d;
    }
}

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

  
    if (!read_adjacency_list(path, g))
    {
        std::cerr << "Error: could not read or parse input file '" << path << "'.\n";
        return 1;
    }
    if (g.source < 0 || g.source >= g.V)
    {
        std::cerr << "Error: source vertex " << g.source << " is out of range [0, " << g.V - 1 << "].\n";
        return 1;
    }


    CSR csr = build_csr(g);

   
    auto t1 = std::chrono::high_resolution_clock::now();
    BellmanFordResult result = bellman_ford_csr(csr, g.source);
    auto t2 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

   
    std::cout << "Algorithm: Bellman-Ford\n";
    std::cout << "Source: " << g.source << "\n";

    if (result.negative_cycle)
    {
        std::cout << "Negative cycle: true\n";
    }
    else
    {
        std::cout << "Vertex Distance\n";
        for (int v = 0; v < csr.V; ++v)
        {
            std::cout << v << ' ';
            print_distance(std::cout, result.distance[v]);
            std::cout << "\n";
        }
        std::cout << "Negative cycle: none\n";
    }

    std::cout << "Execution time: " << ms << " ms\n";
    return 0;
}
