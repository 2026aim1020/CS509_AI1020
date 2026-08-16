// Each generated graph is guaranteed to be connected by first constructing
// a random spanning tree and then adding extra random edges.

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#define MKDIR(path) _mkdir(path)
#else
#include <sys/stat.h>
#include <sys/types.h>
#define MKDIR(path) mkdir(path, 0777)
#endif


static const std::vector<long long> SIZES = {
    10,
    100,
    10000,
    50000,
    100000};


static const int AVG_DEGREE = 4;

//edge-weight range.
static const int MIN_WEIGHT = 1;
static const int MAX_WEIGHT = 100;


static long long build_mst_graph(
    long long V,
    std::mt19937 &rng,
    std::vector<std::vector<std::pair<long long, int>>> &adj)
{
    std::set<std::pair<long long, long long>> edges;

    // Create a random spanning tree first.
    // This guarantees that the graph is connected.
    std::vector<long long> order(V);

    for (long long i = 0; i < V; ++i)
        order[i] = i;

    std::shuffle(order.begin(), order.end(), rng);

    std::uniform_int_distribution<int> weight_dist(
        MIN_WEIGHT, MAX_WEIGHT);

    for (long long i = 1; i < V; ++i)
    {
        std::uniform_int_distribution<long long> parent_dist(0, i - 1);

        long long u = order[i];
        long long v = order[parent_dist(rng)];

        long long a = std::min(u, v);
        long long b = std::max(u, v);

        edges.insert({a, b});
    }

    // Add extra edges to make the graph denser.
    long long extra = (V * AVG_DEGREE) / 2;
    long long target = (V - 1) + extra;

    // Maximum number of edges
    long long max_edges = V * (V - 1) / 2;

    if (target > max_edges)
        target = max_edges;

    long long attempts = 0;
    long long max_attempts = extra * 20 + 1000;

    std::uniform_int_distribution<long long> vertex_dist(0, V - 1);

    while (static_cast<long long>(edges.size()) < target &&
           attempts < max_attempts)
    {
        ++attempts;

        long long u = vertex_dist(rng);
        long long v = vertex_dist(rng);

        if (u == v)
            continue;

        long long a = std::min(u, v);
        long long b = std::max(u, v);

        edges.insert({a, b});
    }

    // Construct adjacency list
    adj.assign(V, {});

    for (const auto &edge : edges)
    {
        long long u = edge.first;
        long long v = edge.second;

        int weight = weight_dist(rng);

        adj[u].push_back({v, weight});
        adj[v].push_back({u, weight});
    }

    return static_cast<long long>(edges.size());
}

// Write graph 
static void write_graph(
    const std::string &path,
    long long V,
    const std::vector<std::vector<std::pair<long long, int>>> &adj,
    long long E)
{
    std::ofstream out(path);

    if (!out)
    {
        std::cerr << "Error: could not open " << path << "\n";
        return;
    }

    out << V << " " << E << "\n";

    for (long long u = 0; u < V; ++u)
    {
        const auto &neighbors = adj[u];

        out << u << " " << neighbors.size();

        for (const auto &[v, weight] : neighbors)
        {
            out << " " << v << " " << weight;
        }

        out << "\n";
    }

    out.close();
}

int main(int argc, char **argv)
{
  
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0]
                  << " <output_path>\n";
        return 1;
    }

    std::string out_dir = argv[1];

    MKDIR(out_dir.c_str());

    std::mt19937 rng(65);

    for (long long V : SIZES)
    {
        std::vector<std::vector<std::pair<long long, int>>> adj;

        long long E = build_mst_graph(V, rng, adj);

        std::ostringstream filename;
        filename << out_dir
                 << "/graph_mst_"
                 << V
                 << ".txt";

        write_graph(filename.str(), V, adj, E);

        std::printf(
            "wrote %s (V=%lld, E=%lld)\n",
            filename.str().c_str(),
            V,
            E);
    }

    return 0;
}
