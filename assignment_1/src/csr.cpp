#include "csr.h"
#include <fstream>
#include <sstream>
#include <iostream>

bool read_adjacency_list(const std::string &path, AdjacencyList &g)
{
    std::ifstream in(path);
    if (!in.is_open())
        return false;

    std::string line;

    // First non-empty line: "V E"
    if (!std::getline(in, line))
        return false;
    {
        std::istringstream ss(line);
        if (!(ss >> g.V >> g.E))
            return false;
    }
    if (g.V <= 0)
        return false;

    g.adj.assign(g.V, {});
    bool weighted_known = false;

    for (int i = 0; i < g.V; ++i)
    {
        if (!std::getline(in, line))
            return false;
        // Skip accidental blank lines.
        while (line.find_first_not_of(" \t\r\n") == std::string::npos)
        {
            if (!std::getline(in, line))
                return false;
        }

        std::istringstream ss(line);
        int u, degree;
        if (!(ss >> u >> degree))
            return false;
        if (u < 0 || u >= g.V)
            return false;

        std::vector<double> rest;
        double tok;
        while (ss >> tok)
            rest.push_back(tok);

        bool line_weighted;
        if (degree == 0)
        {
            line_weighted = weighted_known ? g.weighted : false;
        }
        else if (static_cast<int>(rest.size()) == degree)
        {
            line_weighted = false;
        }
        else if (static_cast<int>(rest.size()) == 2 * degree)
        {
            line_weighted = true;
        }
        else
        {
            return false; // wrong line
        }

        if (!weighted_known)
        {
            g.weighted = line_weighted;
            weighted_known = true;
        }

        g.adj[u].reserve(degree);
        if (line_weighted)
        {
            for (int k = 0; k < degree; ++k)
            {
                int to = static_cast<int>(rest[2 * k]);
                double w = rest[2 * k + 1];
                g.adj[u].push_back({to, w});
            }
        }
        else
        {
            for (int k = 0; k < degree; ++k)
            {
                int to = static_cast<int>(rest[k]);
                g.adj[u].push_back({to, 1.0});
            }
        }
    }

    // Optional trailing "SOURCE s" line.
    while (std::getline(in, line))
    {
        std::istringstream ss(line);
        std::string tag;
        if (ss >> tag)
        {
            if (tag == "SOURCE")
            {
                ss >> g.source;
            }
            break;
        }
    }

    return true;
}

CSR build_csr(const AdjacencyList &g)
{
    CSR csr;
    csr.V = g.V;

    csr.row_ptr.assign(g.V + 1, 0);
    for (int u = 0; u < g.V; ++u)
    {
        csr.row_ptr[u + 1] = csr.row_ptr[u] + static_cast<int>(g.adj[u].size());
    }
    csr.E = csr.row_ptr[g.V];

    csr.col_idx.resize(csr.E);
    csr.values.resize(csr.E);

    for (int u = 0; u < g.V; ++u)
    {
        int base = csr.row_ptr[u];         // base index for each vertex
        const auto &neighbours = g.adj[u]; //(to,w)
        for (size_t k = 0; k < neighbours.size(); ++k)
        {
            csr.col_idx[base + k] = neighbours[k].to;
            csr.values[base + k] = neighbours[k].weight;
        }
    }

    return csr;
}

void print_csr(const CSR &csr)
{
    std::cout << "row_ptr: ";
    for (int v : csr.row_ptr)
        std::cout << v << ' ';
    std::cout << "\ncol_idx: ";
    for (int v : csr.col_idx)
        std::cout << v << ' ';
    std::cout << "\nvalues:  ";
    for (double v : csr.values)
        std::cout << v << ' ';
    std::cout << "\n";
}
