#include "../include/mst.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <queue>
#include <vector>

// Disjoint Set Union (Union-Find) with path compression + union by rank.
struct DSU
{
    std::vector<int> parent, rnk;
    DSU(int n) : parent(n), rnk(n, 0)
    {
        std::iota(parent.begin(), parent.end(), 0);
    }
    int find(int x)
    {
        while (parent[x] != x)
        {
            parent[x] = parent[parent[x]];
            x = parent[x];
        }
        return x;
    }
    bool unite(int a, int b)
    {
        a = find(a);
        b = find(b);
        if (a == b)
            return false;
        if (rnk[a] < rnk[b])
            std::swap(a, b);
        parent[b] = a;
        if (rnk[a] == rnk[b])
            ++rnk[a];
        return true;
    }
};

struct RawEdge
{
    int u, v;
    double w;
};

MSTResult kruskal_mst(const CSR &csr)
{
    MSTResult result;
    int V = csr.V;

    // Extract each undirected edge once (u < v), since the CSR/adjacency
    // list stores both directions for undirected graphs.
    std::vector<RawEdge> edges;
    edges.reserve(csr.E / 2 + 1);
    for (int u = 0; u < V; ++u)
    {
        for (int k = csr.row_ptr[u]; k < csr.row_ptr[u + 1]; ++k)
        {
            int v = csr.col_idx[k];
            if (v > u)
                edges.push_back({u, v, csr.values[k]});
        }
    }

    std::sort(edges.begin(), edges.end(),
              [](const RawEdge &a, const RawEdge &b)
              { return a.w < b.w; });

    DSU dsu(V);
    result.edges.reserve(V > 0 ? V - 1 : 0);
    for (const auto &e : edges)
    {
        if (static_cast<int>(result.edges.size()) == V - 1)
            break;
        if (dsu.unite(e.u, e.v))
        {
            result.edges.push_back({e.u, e.v, e.w});
            result.total_weight += e.w;
        }
    }
    return result;
}

MSTResult prim_mst(const CSR &csr, int start)
{
    MSTResult result;
    int V = csr.V;
    if (V == 0)
        return result;

    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> min_edge(V, INF);
    std::vector<int> parent(V, -1);
    std::vector<bool> in_tree(V, false);

    using P = std::pair<double, int>; // (weight, vertex)
    std::priority_queue<P, std::vector<P>, std::greater<P>> pq;

    min_edge[start] = 0.0;
    pq.push({0.0, start});

    int included = 0;
    while (!pq.empty() && included < V)
    {
        auto [w, u] = pq.top();
        pq.pop();
        if (in_tree[u])
            continue;
        in_tree[u] = true;
        ++included;

        if (parent[u] != -1)
        {
            result.edges.push_back({parent[u], u, w});
            result.total_weight += w;
        }

        for (int k = csr.row_ptr[u]; k < csr.row_ptr[u + 1]; ++k)
        {
            int v = csr.col_idx[k];
            double wt = csr.values[k];
            if (!in_tree[v] && wt < min_edge[v])
            {
                min_edge[v] = wt;
                parent[v] = u;
                pq.push({wt, v});
            }
        }
    }

    return result;
}

