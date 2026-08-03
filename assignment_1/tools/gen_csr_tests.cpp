// makes extra random edges to reach the target
// average degree, avoiding self-loops and duplicate edges.

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <set>
#include <random>
#include <fstream>
#include <sstream>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif

static const std::vector<long long> SIZES = {10, 100, 10000, 50000, 100000};
static const int AVG_DEGREE = 4; // extra random edges beyond the spanning path, per vertex on average

// Build an undirected, connected graph on V vertices.
static long long build_undirected_graph(long long V, std::mt19937 &rng,
                                        std::vector<std::vector<long long>> &adj)
{
    std::set<std::pair<long long, long long>> edges;

    for (long long u = 0; u + 1 < V; ++u)
    {
        edges.insert({u, u + 1});
    }

    // Extra random edges for a denser graph.
    long long extra = (V * AVG_DEGREE) / 2;
    long long target = (V - 1) + extra;
    long long attempts = 0;
    long long max_attempts = extra * 10 + 10;
    std::uniform_int_distribution<long long> dist(0, V - 1);

    while (static_cast<long long>(edges.size()) < target && attempts < max_attempts)
    {
        ++attempts;
        long long u = dist(rng);
        long long v = dist(rng);
        if (u == v)
            continue;
        long long a = std::min(u, v), b = std::max(u, v);
        edges.insert({a, b});
    }

    adj.assign(V, {});
    for (const auto &e : edges)
    {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    return static_cast<long long>(edges.size());
}

static void write_unweighted(const std::string &path, long long V,
                             const std::vector<std::vector<long long>> &adj, long long E)
{
    std::ofstream f(path);
    f << V << " " << E << "\n";
    for (long long u = 0; u < V; ++u)
    {
        const auto &nb = adj[u];
        f << u << " " << nb.size();
        for (long long n : nb)
            f << " " << n;
        f << "\n";
    }
    f << "SOURCE 0\n";
}

static void write_weighted(const std::string &path, long long V,
                           const std::vector<std::vector<long long>> &adj, long long E,
                           std::mt19937 &rng)
{
    std::uniform_int_distribution<int> wdist(1, 20); // positive weight, as required
    std::ofstream f(path);
    f << V << " " << E << "\n";
    for (long long u = 0; u < V; ++u)
    {
        const auto &nb = adj[u];
        f << u << " " << nb.size();
        for (long long n : nb)
        {
            f << " " << n << " " << wdist(rng);
        }
        f << "\n";
    }
    f << "SOURCE 0\n";
}

int main(int argc, char **argv)
{
    std::string out_dir = (argc > 1) ? argv[1] : ".";
    MKDIR(out_dir.c_str()); // no-op if it already exists

    std::mt19937 rng(65); 

    for (long long V : SIZES)
    {
        std::vector<std::vector<long long>> adj;
        long long E = build_undirected_graph(V, rng, adj);

        std::ostringstream uw_name, w_name;
        uw_name << out_dir << "/graph_unweighted_" << V << ".txt";
        w_name << out_dir << "/graph_weighted_" << V << ".txt";

        write_unweighted(uw_name.str(), V, adj, E);
        std::printf("wrote %s (V=%lld, E=%lld)\n", uw_name.str().c_str(), V, E);

        write_weighted(w_name.str(), V, adj, E, rng);
        std::printf("wrote %s (V=%lld, E=%lld)\n", w_name.str().c_str(), V, E);
    }

    return 0;
}
