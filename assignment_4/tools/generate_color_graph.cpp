// Generates a random, sparse, undirected, unweighted graph in the Vertex
// Coloring adjacency-list format (no self-loops, no parallel edges, each
// edge listed in both endpoints' rows).
//
// Build: g++ -O2 -std=c++17 tools/generate_color_graph.cpp -o gen_color.exe
// Usage: ./gen_color.exe <V> <E> <output_path> [seed]

#include <fstream>
#include <iostream>
#include <random>
#include <set>
#include <string>
#include <vector>

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        std::cerr << "Usage: " << argv[0] << " <V> <E> <output_path> [seed]\n";
        return 1;
    }
    long long V = std::stoll(argv[1]);
    long long E = std::stoll(argv[2]);
    std::string out_path = argv[3];
    unsigned seed = argc > 4 ? static_cast<unsigned>(std::stoul(argv[4])) : 42u;

    long long max_edges = V * (V - 1) / 2;
    if (E > max_edges)
        E = max_edges;

    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<long long> pick(0, V - 1);

    std::set<std::pair<int, int>> edges;
    while (static_cast<long long>(edges.size()) < E)
    {
        long long a = pick(rng), b = pick(rng);
        if (a == b)
            continue;
        int u = static_cast<int>(std::min(a, b));
        int v = static_cast<int>(std::max(a, b));
        edges.insert({u, v});
    }

    std::vector<std::vector<int>> adj(V);
    for (auto &[u, v] : edges)
    {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    std::ofstream out(out_path);
    out << V << " " << edges.size() << "\n";
    for (int u = 0; u < V; ++u)
    {
        out << u << " " << adj[u].size();
        for (int v : adj[u])
            out << " " << v;
        out << "\n";
    }
    std::cout << "Wrote " << out_path << " with V=" << V << " E=" << edges.size() << "\n";
    return 0;
}
