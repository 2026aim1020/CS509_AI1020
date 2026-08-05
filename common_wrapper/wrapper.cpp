// Build:
 //   g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -pthread \
  -Iassignment_1/include \
  assignment_1/src/gemm.cpp \
  assignment_1/src/csr.cpp \
  common_wrapper/wrapper.cpp \
  -o wrapper.exe
//
// Run:
//   ./wrapper.exe

#include "../assignment_1/include/gemm.h"
#include "../assignment_1/include/csr.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;

static std::mutex g_print_mutex; // keeps concurrent output from interleaving mid-line


static void locked_print(const std::string &s)
{
    std::lock_guard<std::mutex> lock(g_print_mutex);
    std::cout << s;
    std::cout.flush();
}

static std::vector<std::string> list_txt_files(const std::string &dir)
{
    std::vector<std::string> files;
    if (!fs::exists(dir) || !fs::is_directory(dir))
        return files;
    for (const auto &entry : fs::directory_iterator(dir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".txt")
        {
            files.push_back(entry.path().string());
        }
    }
    std::sort(files.begin(), files.end());
    return files;
}

static std::string prompt_line(const std::string &msg)
{
    std::cout << msg;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

static int prompt_int(const std::string &msg, int default_val)
{
    std::string line = prompt_line(msg);
    if (line.empty())
        return default_val;
    try
    {
        return std::stoi(line);
    }
    catch (...)
    {
        return default_val;
    }
}


static void run_gemm_file(const std::string &path, int block_size)
{
    std::ostringstream out;

    int M, K, N;
    Matrix A, B;
    if (!read_gemm_input(path, M, K, N, A, B))
    {
        out << "[GEMM] " << path << " -> ERROR: could not read/parse file\n";
        locked_print(out.str());
        return;
    }

    Matrix C_simple, C_blocking;

    auto t1 = std::chrono::high_resolution_clock::now();
    gemm_simple(A, B, C_simple);
    auto t2 = std::chrono::high_resolution_clock::now();
    double simple_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

    auto t3 = std::chrono::high_resolution_clock::now();
    gemm_blocking(A, B, C_blocking, block_size);
    auto t4 = std::chrono::high_resolution_clock::now();
    double blocking_ms = std::chrono::duration<double, std::milli>(t4 - t3).count();

    bool match = (C_simple.rows == C_blocking.rows && C_simple.cols == C_blocking.cols);
    if (match)
    {
        for (size_t i = 0; i < C_simple.data.size() && match; ++i)
        {
            if (std::abs(C_simple.data[i] - C_blocking.data[i]) > 1e-6)
                match = false;
        }
    }

    out << "[GEMM] " << path
        << " | " << M << "x" << K << " * " << K << "x" << N
        << " | simple=" << simple_ms << " ms"
        << " | blocking(bs=" << block_size << ")=" << blocking_ms << " ms"
        << " | " << (match ? "MATCH" : "MISMATCH!!") << "\n";
    locked_print(out.str());
}

static void run_gemm_suite(const std::vector<std::string> &files, int block_size)
{
    for (const auto &f : files)
        run_gemm_file(f, block_size);
}


static void run_csr_file(const std::string &path)
{
    std::ostringstream out;

    AdjacencyList g;
    if (!read_adjacency_list(path, g))
    {
        out << "[CSR]  " << path << " -> ERROR: could not read/parse file\n";
        locked_print(out.str());
        return;
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    CSR csr = build_csr(g);
    auto t2 = std::chrono::high_resolution_clock::now();
    double convert_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

    out << "[CSR]  " << path
        << " | V=" << g.V << " E=" << g.E
        << " weighted=" << (g.weighted ? "yes" : "no")
        << " | entries=" << csr.E
        << " | conversion=" << convert_ms << " ms\n";
    locked_print(out.str());
}

static void run_csr_suite(const std::vector<std::string> &files)
{
    for (const auto &f : files)
        run_csr_file(f);
}


static void menu_gemm()
{
    std::cout << "\n-- GEMM --\n"
              << "1. Run a single test file\n"
              << "2. Run all *.txt files in a folder\n"
              << "3. Back\n";
    int choice = prompt_int("Choose: ", 3);

    if (choice == 1)
    {
        std::string path = prompt_line("Input file path: ");
        int bs = prompt_int("Block size [default 32]: ", 32);
        run_gemm_file(path, bs);
    }
    else if (choice == 2)
    {
        std::string dir = prompt_line("Folder path [default tests/gemm]: ");
        if (dir.empty())
            dir = "assignment_1/tests/gemm";
        int bs = prompt_int("Block size [default 32]: ", 32);
        auto files = list_txt_files(dir);
        if (files.empty())
        {
            std::cout << "No .txt files found in " << dir << "\n";
            return;
        }
        std::cout << "Running " << files.size() << " GEMM test file(s)...\n";
        run_gemm_suite(files, bs);
    }
}

static void menu_csr()
{
    std::cout << "\n-- CSR Graph Conversion --\n"
              << "1. Run a single test file\n"
              << "2. Run all *.txt files in a folder\n"
              << "3. Back\n";
    int choice = prompt_int("Choose: ", 3);

    if (choice == 1)
    {
        std::string path = prompt_line("Input file path: ");
        run_csr_file(path);
    }
    else if (choice == 2)
    {
        std::string dir = prompt_line("Folder path [default tests/csr]: ");
        if (dir.empty())
            dir = "assignment_1/tests/csr";
        auto files = list_txt_files(dir);
        if (files.empty())
        {
            std::cout << "No .txt files found in " << dir << "\n";
            return;
        }
        std::cout << "Running " << files.size() << " CSR test file(s)...\n";
        run_csr_suite(files);
    }
}

// Runs the full GEMM suite and the full CSR suite AT THE SAME TIME, each on
// its own thread, then waits for both to finish.
static void menu_run_all()
{
    std::string gemm_dir = prompt_line("GEMM folder [default tests/gemm]: ");
    if (gemm_dir.empty())
        gemm_dir = "assignment_1/tests/gemm";
    std::string csr_dir = prompt_line("CSR folder [default tests/csr]: ");
    if (csr_dir.empty())
        csr_dir = "assignment_1/tests/csr";
    int bs = prompt_int("GEMM block size [default 32]: ", 32);

    auto gemm_files = list_txt_files(gemm_dir);
    auto csr_files = list_txt_files(csr_dir);

    if (gemm_files.empty() && csr_files.empty())
    {
        std::cout << "No test files found in either folder.\n";
        return;
    }

    std::cout << "Launching GEMM (" << gemm_files.size()
              << " files) and CSR (" << csr_files.size()
              << " files) concurrently...\n\n";

    auto wall_start = std::chrono::high_resolution_clock::now();

    std::thread gemm_thread(run_gemm_suite, gemm_files, bs);
    std::thread csr_thread(run_csr_suite, csr_files);

    gemm_thread.join();
    csr_thread.join();

    auto wall_end = std::chrono::high_resolution_clock::now();
    double wall_ms = std::chrono::duration<double, std::milli>(wall_end - wall_start).count();

    std::cout << "\nAll tests completed in " << wall_ms << " ms.\n";
}


int main()
{
    std::cout << "======================================\n"
              << " CS509 Assignment - Task Runner\n"
              << " Author: Sanket Bandgar\n"
              << "======================================\n";

    while (true)
    {
        std::cout << "\nMain menu:\n"
                  << "1. Run GEMM\n"
                  << "2. Run CSR Graph Conversion\n"
                  << "3. Run All (GEMM + CSR at the same time)\n"
                  << "4. Exit\n";
        int choice = prompt_int("Choose an option: ", 4);

        switch (choice)
        {
        case 1:
            menu_gemm();
            break;
        case 2:
            menu_csr();
            break;
        case 3:
            menu_run_all();
            break;
        case 4:
            std::cout << "Goodbye.\n";
            return 0;
        default:
            std::cout << "Invalid choice, try again.\n";
            break;
        }
    }
}
