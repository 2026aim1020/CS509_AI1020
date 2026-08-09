#include "../include/bellman_ford.h"
#include <limits>

BellmanFordResult bellman_ford_csr(const CSR &csr, int source)
{
    BellmanFordResult res;
    const double INF = std::numeric_limits<double>::infinity();
    res.distance.assign(csr.V, INF);
    if (source < 0 || source >= csr.V)
    {
        return res; // leaves everything INF/unreachable
    }
    res.distance[source] = 0.0;


    for (int pass = 0; pass < csr.V - 1; ++pass)
    {
        bool changed = false;
        for (int u = 0; u < csr.V; ++u)
        {
            if (res.distance[u] == INF)
                continue; // u not reachable yet, nothing to relax from it
            for (int idx = csr.row_ptr[u]; idx < csr.row_ptr[u + 1]; ++idx)
            {
                int v = csr.col_idx[idx];
                double w = csr.values[idx];
                double cand = res.distance[u] + w;
                if (cand < res.distance[v])
                {
                    res.distance[v] = cand;
                    changed = true;
                }
            }
        }
        if (!changed)
            break; 
    }

    // One extra pass: if anything can still be relaxed, a negative-weight
    // cycle is reachable
    for (int u = 0; u < csr.V && !res.negative_cycle; ++u)
    {
        if (res.distance[u] == INF)
            continue;
        for (int idx = csr.row_ptr[u]; idx < csr.row_ptr[u + 1]; ++idx)
        {
            int v = csr.col_idx[idx];
            double w = csr.values[idx];
            if (res.distance[u] + w < res.distance[v])
            {
                res.negative_cycle = true;
                break;
            }
        }
    }

    return res;
}
