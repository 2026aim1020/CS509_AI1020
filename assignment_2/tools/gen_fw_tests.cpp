// Generates Floyd-Warshall test matrices at the sizes required by

// Same DAG trick as gen_bf_tests.cpp: every direct edge goes from a lower
// index to a higher index, so the graph can never contain a cycle
// (negative or otherwise), which lets us mix negative and positive weights
// freely without a cycle-detection pass.
//
// Usage:
//   ./gen_fw_tests <out_dir>

#include <cstdio>
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <limits>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

static const std::vector<int> SIZES = {500, 1000, 2000};

int main(int argc, char **argv)
{
    std::string out_dir = (argc > 1) ? argv[1] : ".";
    MKDIR(out_dir.c_str());

    std::mt19937 rng(456);
    std::uniform_int_distribution<int> weight_dist(-5, 20);
    std::uniform_int_distribution<int> extra_edges_dist(1, 4);
    std::uniform_int_distribution<int> gap_dist(1, 30);

    for (int V : SIZES)
    {
        std::vector<std::vector<double>> M(V, std::vector<double>(V,
                                                                  std::numeric_limits<double>::infinity()));
        for (int i = 0; i < V; ++i)
            M[i][i] = 0.0;

        // Spanning forward chain so every vertex is reachable from 0.
        for (int u = 0; u + 1 < V; ++u)
        {
            M[u][u + 1] = weight_dist(rng);
        }
        // Extra forward-only edges for density.
        for (int u = 0; u < V; ++u)
        {
            int extra = extra_edges_dist(rng);
            for (int k = 0; k < extra; ++k)
            {
                int v = u + 1 + gap_dist(rng);
                if (v >= V)
                    continue;
                M[u][v] = weight_dist(rng);
            }
        }

        std::ostringstream name;
        name << out_dir << "/fw_" << V << ".txt";
        std::ofstream f(name.str());
        f << V << "\n";
        for (int i = 0; i < V; ++i)
        {
            for (int j = 0; j < V; ++j)
            {
                if (std::isinf(M[i][j]))
                    f << "INF";
                else
                    f << static_cast<long long>(M[i][j]);
                if (j + 1 < V)
                    f << ' ';
            }
            f << "\n";
        }
        std::printf("wrote %s (V=%d, DAG - no cycles possible)\n", name.str().c_str(), V);
    }
    return 0;
}
