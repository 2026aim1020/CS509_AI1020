#include "floyd_warshall.h"
#include "../../assignment_1/include/csr.h" 
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <cmath>

static const double INF = std::numeric_limits<double>::infinity();

bool read_fw_input(const std::string &path, DenseMatrix &M)
{
    std::ifstream in(path);
    if (!in.is_open())
        return false;

    int V;
    if (!(in >> V) || V <= 0)
        return false;

    M = DenseMatrix(V);
    for (int i = 0; i < V; ++i)
    {
        for (int j = 0; j < V; ++j)
        {
            std::string tok;
            if (!(in >> tok))
                return false;
            if (tok == "INF" || tok == "inf")
            {
                M.at(i, j) = INF;
            }
            else
            {
                try
                {
                    M.at(i, j) = std::stod(tok);
                }
                catch (...)
                {
                    return false;
                }
            }
        }
    }
    return true;
}

FloydWarshallResult floyd_warshall(const DenseMatrix &M)
{
    FloydWarshallResult res;
    res.distance = M; 
    
    const int V = M.V;
    DenseMatrix &D = res.distance;

    for (int k = 0; k < V; ++k)
    {
        for (int i = 0; i < V; ++i)
        {
            double dik = D.at(i, k);
            if (std::isinf(dik))
                continue; // no path i->k, nothing to relax through k
            for (int j = 0; j < V; ++j)
            {
                double dkj = D.at(k, j);
                if (std::isinf(dkj))
                    continue;
                double cand = dik + dkj;
                if (cand < D.at(i, j))
                {
                    D.at(i, j) = cand;
                }
            }
        }
    }

    for (int i = 0; i < V; ++i)
    {
        if (D.at(i, i) < 0.0)
        {
            res.negative_cycle = true;
            break;
        }
    }

    return res;
}

DenseMatrix csr_to_dense_matrix(const CSR &csr)
{
    DenseMatrix M(csr.V);
    for (int i = 0; i < csr.V; ++i)
    {
        for (int j = 0; j < csr.V; ++j)
        {
            M.at(i, j) = (i == j) ? 0.0 : INF;
        }
    }
    for (int u = 0; u < csr.V; ++u)
    {
        for (int idx = csr.row_ptr[u]; idx < csr.row_ptr[u + 1]; ++idx)
        {
            int v = csr.col_idx[idx];
            double w = csr.values[idx];
            if (u == v)
                continue; //diagonal must stay 0 per spec
            if (w < M.at(u, v))
                M.at(u, v) = w; // keep the smaller weight if duplicate edges exist
        }
    }
    return M;
}

static void print_number(std::ostream &out, double v)
{
    if (std::isinf(v))
    {
        out << "INF";
        return;
    }
    double rounded = std::round(v);
    if (std::abs(v - rounded) < 1e-9)
    {
        out << static_cast<long long>(rounded);
    }
    else
    {
        out.precision(6);
        out << v;
    }
}

void print_fw_matrix(const DenseMatrix &M)
{
    std::ostringstream out;
    for (int i = 0; i < M.V; ++i)
    {
        for (int j = 0; j < M.V; ++j)
        {
            print_number(out, M.at(i, j));
            if (j + 1 < M.V)
                out << ' ';
        }
        out << '\n';
    }
    std::cout << out.str();
}
