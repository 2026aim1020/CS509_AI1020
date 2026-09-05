#include "../include/pagerank.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>

bool read_pagerank_input(const std::string &path, PageRankInput &in, std::string &err)
{
    std::ifstream file(path);
    if (!file.is_open())
    {
        err = "could not open file: " + path;
        return false;
    }

    AdjacencyList &g = in.graph;
    std::string line;

    if (!std::getline(file, line))
    {
        err = "missing V E header line";
        return false;
    }
    {
        std::istringstream ss(line);
        if (!(ss >> g.V >> g.E))
        {
            err = "malformed V E header line";
            return false;
        }
    }
    if (g.V <= 0)
    {
        err = "V must be positive";
        return false;
    }
    g.weighted = false;
    g.adj.assign(g.V, {});

    for (int i = 0; i < g.V; ++i)
    {
        if (!std::getline(file, line))
        {
            err = "unexpected end of file while reading adjacency rows";
            return false;
        }
        while (line.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            if (!std::getline(file, line))
            {
                err = "unexpected end of file while reading adjacency rows";
                return false;
            }
        }

        std::istringstream ss(line);
        int u, outdeg;
        if (!(ss >> u >> outdeg))
        {
            err = "malformed adjacency row " + std::to_string(i);
            return false;
        }
        if (u < 0 || u >= g.V)
        {
            err = "out-of-range vertex id " + std::to_string(u);
            return false;
        }
        if (outdeg < 0)
        {
            err = "negative outdegree on vertex " + std::to_string(u);
            return false;
        }

        g.adj[u].reserve(outdeg);
        for (int k = 0; k < outdeg; ++k)
        {
            int to;
            if (!(ss >> to))
            {
                err = "declared outdegree does not match neighbour count for vertex " + std::to_string(u);
                return false;
            }
            if (to < 0 || to >= g.V)
            {
                err = "out-of-range neighbour id " + std::to_string(to) + " for vertex " + std::to_string(u);
                return false;
            }
            g.adj[u].push_back({to, 1.0});
        }
        double extra;
        if (ss >> extra)
        {
            err = "declared outdegree does not match neighbour count for vertex " + std::to_string(u);
            return false;
        }
    }

    // Trailing DAMPING / TOLERANCE / MAX_ITERATIONS lines, in any order.
    bool has_damping = false, has_tol = false, has_iter = false;
    while (std::getline(file, line))
    {
        std::istringstream ss(line);
        std::string tag;
        if (!(ss >> tag))
            continue;
        if (tag == "DAMPING")
        {
            ss >> in.damping;
            has_damping = true;
        }
        else if (tag == "TOLERANCE")
        {
            ss >> in.tolerance;
            has_tol = true;
        }
        else if (tag == "MAX_ITERATIONS")
        {
            ss >> in.max_iterations;
            has_iter = true;
        }
    }

    if (!has_damping || in.damping <= 0.0 || in.damping >= 1.0)
    {
        err = "DAMPING must be present and in (0, 1)";
        return false;
    }
    if (!has_tol || in.tolerance <= 0.0)
    {
        err = "TOLERANCE must be present and positive";
        return false;
    }
    if (!has_iter || in.max_iterations <= 0)
    {
        err = "MAX_ITERATIONS must be present and positive";
        return false;
    }

    return true;
}

PageRankResult pagerank(const CSR &csr, double damping, double tolerance, int max_iterations)
{
    PageRankResult result;
    int N = csr.V;
    std::vector<double> rank(N, N > 0 ? 1.0 / N : 0.0);
    std::vector<double> next(N, 0.0);

    // Precompute outdegree per vertex.
    std::vector<int> outdeg(N);
    for (int u = 0; u < N; ++u)
        outdeg[u] = csr.row_ptr[u + 1] - csr.row_ptr[u];

    int iter = 0;
    bool converged = false;
    for (; iter < max_iterations; ++iter)
    {
        std::fill(next.begin(), next.end(), 0.0);
        double dangling_sum = 0.0;

        for (int u = 0; u < N; ++u)
        {
            if (outdeg[u] == 0)
            {
                dangling_sum += rank[u];
                continue;
            }
            double share = rank[u] / outdeg[u];
            for (int k = csr.row_ptr[u]; k < csr.row_ptr[u + 1]; ++k)
                next[csr.col_idx[k]] += share;
        }

        double base = (1.0 - damping) / N;
        double dangling_term = damping * dangling_sum / N;
        double total_change = 0.0;
        for (int v = 0; v < N; ++v)
        {
            double new_rank = base + damping * next[v] + dangling_term;
            total_change += std::fabs(new_rank - rank[v]);
            next[v] = new_rank;
        }
        std::swap(rank, next);

        if (total_change <= tolerance)
        {
            converged = true;
            ++iter; // count this iteration
            break;
        }
    }
    if (!converged)
        iter = max_iterations;

    result.rank = rank;
    result.iterations = iter;
    result.converged = converged;
    return result;
}
