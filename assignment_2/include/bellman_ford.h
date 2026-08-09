#ifndef BELLMAN_FORD_H
#define BELLMAN_FORD_H

#include "../../assignment_1/include/csr.h"
#include <vector>

struct BellmanFordResult
{
    bool negative_cycle = false;
    std::vector<double> distance; // valid only if negative_cycle == false
};

// Directed graph, edges taken straight from CSR (col_idx/values), weights
// may be negative. 
BellmanFordResult bellman_ford_csr(const CSR &csr, int source);

#endif 
