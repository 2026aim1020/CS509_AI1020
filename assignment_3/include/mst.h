#ifndef MST_H
#define MST_H

#include "../../assignment_1/include/csr.h"
#include <vector>

struct MSTEdge
{
    int u;
    int v;
    double weight;
};

struct MSTResult
{
    std::vector<MSTEdge> edges;
    double total_weight = 0.0;
};

// Kruskal's algorithm: sorts edges by weight and uses Union-Find  to
// avoid cycles. Edge extraction/sorting is part of the algorithm.
MSTResult kruskal_mst(const CSR &csr);

// Prim's algorithm: grows a tree from `start` using a min-priority queue.
MSTResult prim_mst(const CSR &csr, int start = 0);

#endif
