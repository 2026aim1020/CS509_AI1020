// 10 and 100 vertices run Bellman-Ford from
// every vertex as source and confirm the resulting distances agree with
// the corresponding row of Floyd-Warshall's output.
//
// this takes ONE Bellman-Ford
// format adjacency-list file, builds CSR from it derives a dense matrix from that same CSR
// (csr_to_dense_matrix), and runs both algorithms against that single
// underlying graph.
//
// Usage:
//   ./cross_check <bf_format_graph_file>
//


#include "../../assignment_1/include/csr.h"
#include "bellman_ford.h"
#include "floyd_warshall.h"
#include <cmath>
#include <iostream>
#include <string>

static bool nearly_equal(double a, double b)
{
    if (std::isinf(a) && std::isinf(b))
        return true;
    if (std::isinf(a) != std::isinf(b))
        return false;
    return std::abs(a - b) < 1e-6;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <bf_format_graph_file>\n";
        return 1;
    }

    AdjacencyList g;
    if (!read_adjacency_list(argv[1], g))
    {
        std::cerr << "Error: could not read/parse '" << argv[1] << "'.\n";
        return 1;
    }

    CSR csr = build_csr(g);
    DenseMatrix M = csr_to_dense_matrix(csr);

    FloydWarshallResult fw = floyd_warshall(M);

    std::cout << "Cross-check: Bellman-Ford (every source) vs Floyd-Warshall\n";
    std::cout << "Graph: " << argv[1] << " | V=" << csr.V << "\n\n";

    if (fw.negative_cycle)
    {
        std::cout << "Floyd-Warshall reports a negative cycle; per-source Bellman-Ford "
                     "runs are still checked below where they agree it is unreachable-safe.\n";
    }

    int mismatches = 0;
    for (int s = 0; s < csr.V; ++s)
    { // run Bellman-Ford from every vertex
        BellmanFordResult bf = bellman_ford_csr(csr, s);

        bool bf_neg = bf.negative_cycle;
        bool row_undefined = fw.negative_cycle; // FW result invalid everywhere if it found a cycle

        if (bf_neg != row_undefined)
        {
            std::cout << "Source " << s << ": MISMATCH (BF negative_cycle=" << bf_neg
                      << ", FW negative_cycle=" << row_undefined << ")\n";
            ++mismatches;
            continue;
        }
        if (bf_neg)
        {
            std::cout << "Source " << s << ": both report negative cycle - OK\n";
            continue;
        }

        bool row_ok = true;
        for (int v = 0; v < csr.V; ++v)
        {
            if (!nearly_equal(bf.distance[v], fw.distance.at(s, v)))
            {
                row_ok = false;
                break;
            }
        }
        std::cout << "Source " << s << ": " << (row_ok ? "MATCH" : "MISMATCH") << "\n";
        if (!row_ok)
            ++mismatches;
    }

    std::cout << "\n"
              << (mismatches == 0 ? "ALL SOURCES MATCH" : "MISMATCHES FOUND")
              << " (" << (csr.V - mismatches) << "/" << csr.V << " sources agree)\n";

    return mismatches == 0 ? 0 : 2;
}
