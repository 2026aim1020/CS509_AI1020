// Converts a Bellman-Ford format adjacency-list file into a Floyd-Warshall
// dense-matrix file describing the exact same graph, so the required
// cross-check 

//   ./bf_to_fw_convert <bf_format_file> <output_fw_file>

#include "../../assignment_1/include/csr.h"
#include "floyd_warshall.h"
#include <cmath>
#include <iostream>
#include <string>

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        std::cerr << "Usage: " << argv[0] << " <bf_format_file> <output_fw_file>\n";
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

    FILE *out = std::fopen(argv[2], "w");
    if (!out)
    {
        std::cerr << "Error: could not open '" << argv[2] << "' for writing.\n";
        return 1;
    }
    std::fprintf(out, "%d\n", M.V);
    for (int i = 0; i < M.V; ++i)
    {
        for (int j = 0; j < M.V; ++j)
        {
            double v = M.at(i, j);
            if (std::isinf(v))
                std::fprintf(out, "INF");
            else
                std::fprintf(out, "%lld", static_cast<long long>(v));
            if (j + 1 < M.V)
                std::fprintf(out, " ");
        }
        std::fprintf(out, "\n");
    }
    std::fclose(out);

    std::cout << "Wrote " << argv[2] << " (V=" << M.V << ") from " << argv[1] << "\n";
    return 0;
}
