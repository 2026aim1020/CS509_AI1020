// Generates Bellman-Ford test graphs (directed, weighted, possibly

// NO undirected edges as -ve can make -ve cycle

// Negative-cycle safety: every edge respects a fixed topological order
// (u -> v only where u < v), so the graph is a DAG by construction and can
// never contain ANY cycle, negative or otherwise - regardless of how many
// negative weights are assigned. This lets us freely mix negative and
// positive weights at scale without a cycle-detection pass.
//
// Usage:
//   ./gen_bf_tests <out_dir>

#include <cstdio>
#include <string>
#include <vector>
#include <random>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

static const std::vector<long long> SIZES = {10, 100, 10000, 50000, 100000};

int main(int argc, char **argv)
{
    std::string out_dir = (argc > 1) ? argv[1] : ".";
    MKDIR(out_dir.c_str());

    std::mt19937 rng(123);
    std::uniform_int_distribution<int> weight_dist(-5, 20); // mostly positive, some negative
    std::uniform_int_distribution<int> fanout_small(1, 3);  // extra forward edges per vertex

    for (long long V : SIZES)
    {
        // Edge budget: spanning forward chain (V-1) + extra forward edges,
      
        long long extra_per_vertex = (V >= 10000) ? 3 : (V >= 100 ? 4 : 2);

        std::vector<std::vector<std::pair<long long, int>>> adj(V); // adj[u] = {(v, weight)}
        long long E = 0;

        // Guarantee a path 0->1->2->...->(V-1) so every vertex is reachable from 0.
        for (long long u = 0; u + 1 < V; ++u)
        {
            adj[u].push_back({u + 1, weight_dist(rng)});
            ++E;
        }

        // Extra forward-only edges (u -> v, v > u) for density
        std::uniform_int_distribution<long long> gap_dist(1, 50);
        for (long long u = 0; u < V; ++u)
        {
            int extra = fanout_small(rng) % (extra_per_vertex + 1);
            for (int k = 0; k < extra; ++k)
            {
                long long gap = gap_dist(rng);
                long long v = u + 1 + gap;
                if (v >= V)
                    continue;
                adj[u].push_back({v, weight_dist(rng)});
                ++E;
            }
        }

        std::ostringstream name;
        name << out_dir << "/bf_" << V << ".txt";
        std::ofstream f(name.str());
        f << V << " " << E << "\n";
        for (long long u = 0; u < V; ++u)
        {
            f << u << " " << adj[u].size();
            for (auto &e : adj[u])
                f << " " << e.first << " " << e.second;
            f << "\n";
        }
        f << "SOURCE 0\n";

        std::printf("wrote %s (V=%lld, E=%lld, DAG - no cycles possible)\n",
                    name.str().c_str(), V, E);
    }

    return 0;
}
