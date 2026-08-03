#ifndef CSR_H
#define CSR_H

#include <vector>
#include <string>

struct Edge
{
    int to;
    double weight;
}; // 1 for unweighted

struct AdjacencyList
{
    int V = 0;
    int E = 0;
    int source = 0;
    bool weighted = false;
    std::vector<std::vector<Edge>> adj; // size V
};


struct CSR{
    int V = 0;
    int E = 0;                  
    std::vector<int> row_ptr;   
    std::vector<int> col_idx;   // size E
    std::vector<double> values; 
};


bool read_adjacency_list(const std::string &path, AdjacencyList &g);


CSR build_csr(const AdjacencyList &g);

void print_csr(const CSR &csr);

#endif
