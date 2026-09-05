// Generates a random, sparse, directed, unweighted graph in the PageRank
// adjacency-list format, with trailing DAMPING / TOLERANCE / MAX_ITERATIONS
// lines. A couple of "extra" edges are added per vertex to make dangling
// vertices rare (but not impossible) while keeping the graph sparse.
//
// Build: g++ -O2 -std=c++17 tools/generate_pagerank_graph.cpp -o gen_pagerank.exe
// Usage: ./gen_pagerank.exe <V> <E> <output_path> [seed] [damping] [tolerance] [max_iter]

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
        std::cerr << "Usage: " << argv[0] << " <V> <E> <output_path> [seed] [damping] [tolerance] [max_iter]\n";
        return 1;
    }
    long long V = std::stoll(argv[1]);
    long long E = std::stoll(argv[2]);
    std::string out_path = argv[3];
    unsigned seed = argc > 4 ? static_cast<unsigned>(std::stoul(argv[4])) : 42u;
    double damping = argc > 5 ? std::stod(argv[5]) : 0.85;
    double tolerance = argc > 6 ? std::stod(argv[6]) : 1e-4;
    int max_iter = argc > 7 ? std::stoi(argv[7]) : 100;

    long long max_edges = V * (V - 1);
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
        edges.insert({static_cast<int>(a), static_cast<int>(b)});
    }

    std::vector<std::vector<int>> adj(V);
    for (auto &[u, v] : edges)
        adj[u].push_back(v);

    std::ofstream out(out_path);
    out << V << " " << edges.size() << "\n";
    for (int u = 0; u < V; ++u)
    {
        out << u << " " << adj[u].size();
        for (int v : adj[u])
            out << " " << v;
        out << "\n";
    }
    out << "DAMPING " << damping << "\n";
    out << "TOLERANCE " << tolerance << "\n";
    out << "MAX_ITERATIONS " << max_iter << "\n";
    std::cout << "Wrote " << out_path << " with V=" << V << " E=" << edges.size() << "\n";
    return 0;
}
