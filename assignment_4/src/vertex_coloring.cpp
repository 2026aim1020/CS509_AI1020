#include "../include/vertex_coloring.h"
#include <algorithm>
#include <numeric>
#include <unordered_set>

bool validate_coloring_input(const AdjacencyList &g, std::string &err)
{
    if (g.V <= 0)
    {
        err = "graph must have at least one vertex";
        return false;
    }

    // Build a quick membership check for "does u list v" so we can confirm
    // the adjacency list is symmetric (each undirected edge appears twice).
    std::vector<std::unordered_set<int>> has(g.V);
    for (int u = 0; u < g.V; ++u)
        for (const Edge &e : g.adj[u])
            has[u].insert(e.to);

    for (int u = 0; u < g.V; ++u)
    {
        for (const Edge &e : g.adj[u])
        {
            if (e.to == u)
            {
                err = "self-loop detected at vertex " + std::to_string(u);
                return false;
            }
            if (e.to < 0 || e.to >= g.V)
            {
                err = "out-of-range neighbour id " + std::to_string(e.to) +
                      " in adjacency list of vertex " + std::to_string(u);
                return false;
            }
            if (has[e.to].find(u) == has[e.to].end())
            {
                err = "asymmetric edge: " + std::to_string(u) + " -> " + std::to_string(e.to) +
                      " has no matching " + std::to_string(e.to) + " -> " + std::to_string(u);
                return false;
            }
        }
    }
    return true;
}

ColoringResult greedy_vertex_coloring(const CSR &csr)
{
    ColoringResult result;
    int V = csr.V;
    result.color.assign(V, -1);

    // Order vertices by non-increasing degree (Welsh-Powell heuristic).
    std::vector<int> order(V);
    std::iota(order.begin(), order.end(), 0);
    std::sort(order.begin(), order.end(), [&](int a, int b)
              {
                  int deg_a = csr.row_ptr[a + 1] - csr.row_ptr[a];
                  int deg_b = csr.row_ptr[b + 1] - csr.row_ptr[b];
                  return deg_a > deg_b;
              });

    // forbidden_at[c] == v  means color c is currently forbidden for vertex v.
    // Stamping with the vertex id (instead of clearing a bool array every
    // iteration) keeps each vertex's work proportional to its own degree.
    std::vector<int> forbidden_at(V + 1, -1);
    for (int v : order)
    {
        for (int k = csr.row_ptr[v]; k < csr.row_ptr[v + 1]; ++k)
        {
            int c = result.color[csr.col_idx[k]];
            if (c >= 0)
                forbidden_at[c] = v;
        }
        int c = 0;
        while (forbidden_at[c] == v)
            ++c;
        result.color[v] = c;
        result.colors_used = std::max(result.colors_used, c + 1);
    }

    result.valid = check_coloring_valid(csr, result.color);
    return result;
}

bool check_coloring_valid(const CSR &csr, const std::vector<int> &color)
{
    for (int u = 0; u < csr.V; ++u)
    {
        for (int k = csr.row_ptr[u]; k < csr.row_ptr[u + 1]; ++k)
        {
            int v = csr.col_idx[k];
            if (color[u] == color[v])
                return false;
        }
    }
    return true;
}
