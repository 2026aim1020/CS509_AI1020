
// g++ -O2 -std=c++17 -static-libgcc -static-libstdc++ -pthread \
//   -Iassignment_1/include \
//   -Iassignment_2/include \
//   -Iassignment_3/include \
//   assignment_1/src/csr.cpp \
//   assignment_1/src/gemm.cpp \
//   assignment_2/src/bellman_ford.cpp \
//   assignment_2/src/floyd_warshall.cpp \
//   assignment_3/src/mst.cpp \
//   common_wrapper/wrapper.cpp \
//   -o wrapper.exe

// Master wrapper Asks which assignment to run, then hands off
// to that assignment's own menu - or runs every assignment's default test
// suite concurrently across all of them at once.
//

//
// Build (from the project root, i.e. the parent of assignment_1/ and
// assignment_2/):
//   ./common_wrapper/build.sh
//   (produces ./wrapper at the project root - see build.sh for why)
//
// Run (from the project root, so the default test-folder paths resolve):
//   ./wrapper

#include "../assignment_1/include/csr.h"              // Assignment 1
#include "../assignment_1/include/gemm.h"             // Assignment 1
#include "../assignment_2/include/bellman_ford.h"     // Assignment 2 
#include "../assignment_2/include/floyd_warshall.h"   // Assignment 2 
#include "../assignment_3/include/mst.h"               // Assignment 3

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <functional>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;


namespace common
{

    std::mutex print_mutex;

    void locked_print(const std::string &s)
    {
        std::lock_guard<std::mutex> lock(print_mutex);
        std::cout << s;
        std::cout.flush();
    }

    std::vector<std::string> list_txt_files(const std::string &dir)
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

    std::vector<std::string> filter_prefix(const std::vector<std::string> &files, const std::string &prefix)
    {
        std::vector<std::string> out;
        for (const auto &f : files)
        {
            std::string base = fs::path(f).filename().string();
            if (base.rfind(prefix, 0) == 0)
                out.push_back(f);
        }
        return out;
    }

    std::string prompt_line(const std::string &msg)
    {
        std::cout << msg;
        std::string line;
        std::getline(std::cin, line);
        return line;
    }

    int prompt_int(const std::string &msg, int default_val)
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

} 

// ===========================================================================
// Assignment 1: GEMM + CSR Graph Conversion 
// ===========================================================================
namespace a1
{

    using common::list_txt_files;
    using common::locked_print;
    using common::prompt_int;
    using common::prompt_line;

    void run_gemm_file(const std::string &path, int block_size)
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

        out << "[GEMM] " << path << " | " << M << "x" << K << " * " << K << "x" << N
            << " | simple=" << simple_ms << " ms"
            << " | blocking(bs=" << block_size << ")=" << blocking_ms << " ms"
            << " | " << (match ? "MATCH" : "MISMATCH!!") << "\n";
        locked_print(out.str());
    }
    void run_gemm_suite(const std::vector<std::string> &files, int bs)
    {
        for (auto &f : files)
            run_gemm_file(f, bs);
    }

    void run_csr_file(const std::string &path)
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
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        out << "[CSR]  " << path << " | V=" << g.V << " E=" << g.E
            << " weighted=" << (g.weighted ? "yes" : "no")
            << " | entries=" << csr.E << " | conversion=" << ms << " ms\n";
        locked_print(out.str());
    }
    void run_csr_suite(const std::vector<std::string> &files)
    {
        for (auto &f : files)
            run_csr_file(f);
    }

   

    void menu_gemm()
    {
        std::cout << "\n-- GEMM --\n1. Run a single test file\n2. Run all *.txt files in a folder\n3. Back\n";
        int c = prompt_int("Choose: ", 3);
        if (c == 1)
        {
            std::string path = prompt_line("Input file path: ");
            int bs = prompt_int("Block size [default 32]: ", 32);
            run_gemm_file(path, bs);
        }
        else if (c == 2)
        {
            std::string dir = prompt_line("Folder path [default assignment_1/tests/gemm]: ");
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
    void menu_csr()
    {
        std::cout << "\n-- CSR Graph Conversion --\n1. Run a single test file\n2. Run all *.txt files in a folder\n3. Back\n";
        int c = prompt_int("Choose: ", 3);
        if (c == 1)
            run_csr_file(prompt_line("Input file path: "));
        else if (c == 2)
        {
            std::string dir = prompt_line("Folder path [default assignment_1/tests/csr]: ");
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
  
    void menu_run_all()
    {
        std::string gemm_dir = prompt_line("GEMM folder [default assignment_1/tests/gemm]: ");
        if (gemm_dir.empty())
            gemm_dir = "assignment_1/tests/gemm";
        std::string graph_dir = prompt_line("Graph folder (CSR) [default assignment_1/tests/csr]: ");
        if (graph_dir.empty())
            graph_dir = "assignment_1/tests/csr";
        int bs = prompt_int("GEMM block size [default 32]: ", 32);

        auto gemm_files = list_txt_files(gemm_dir);
        auto graph_files = list_txt_files(graph_dir);
        if (gemm_files.empty() && graph_files.empty())
        {
            std::cout << "No test files found.\n";
            return;
        }

        std::cout << "Launching GEMM/CSR concurrently...\n\n";
        auto t0 = std::chrono::high_resolution_clock::now();
        std::thread t1(run_gemm_suite, gemm_files, bs);
        std::thread t2(run_csr_suite, graph_files);
      
        t1.join();
        t2.join();
       
        auto t_end = std::chrono::high_resolution_clock::now();
        std::cout << "\nAssignment 1 finished. Wall-clock: "
                  << std::chrono::duration<double, std::milli>(t_end - t0).count() << " ms\n";
    }

  
    void open_submenu()
    {
        while (true)
        {
            std::cout << "\n== Assignment 1: GEMM + CSR==\n"
                      << "1. Run GEMM\n2. Run CSR Graph Conversion\n"
                      << "3. Run All (this assignment, concurrently)\n4. Back to main menu\n";
            int c = prompt_int("Choose an option: ", 4);
            switch (c)
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
                return;
            default:
                std::cout << "Invalid choice, try again.\n";
                break;
            }
        }
    }

    // Non-interactive: appends this assignment's default-folder suites as
    // threads onto `threads`, for the cross-assignment "Run EVERYTHING" mode.
    void launch_concurrent(std::vector<std::thread> &threads)
    {
        auto gemm_files = list_txt_files("assignment_1/tests/gemm");
        auto graph_files = list_txt_files("assignment_1/tests/csr");
        threads.emplace_back(run_gemm_suite, gemm_files, 32);
        threads.emplace_back(run_csr_suite, graph_files);
    }

} 

// ===========================================================================
// Assignment 2 - Bellman-Ford + Floyd-Warshall
// ===========================================================================
namespace a2i
{

    using common::list_txt_files;
    using common::locked_print;
    using common::prompt_int;
    using common::prompt_line;

    void run_bf_file(const std::string &path)
    {
        std::ostringstream out;
        AdjacencyList g;
        if (!read_adjacency_list(path, g) || g.source < 0 || g.source >= g.V)
        {
            out << "[BF]   " << path << " -> ERROR: could not read/parse file (or bad source)\n";
            locked_print(out.str());
            return;
        }
        CSR csr = build_csr(g);
        auto t1 = std::chrono::high_resolution_clock::now();
        BellmanFordResult res = bellman_ford_csr(csr, g.source);
        auto t2 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        out << "[BF]   " << path << " | V=" << g.V << " source=" << g.source
            << " | " << (res.negative_cycle ? "NEGATIVE CYCLE" : "ok") << " | time=" << ms << " ms\n";
        locked_print(out.str());
    }
    void run_bf_suite(const std::vector<std::string> &files)
    {
        for (auto &f : files)
            run_bf_file(f);
    }

    void run_fw_file(const std::string &path)
    {
        std::ostringstream out;
        DenseMatrix M;
        if (!read_fw_input(path, M))
        {
            out << "[FW]   " << path << " -> ERROR: could not read/parse file\n";
            locked_print(out.str());
            return;
        }
        auto t1 = std::chrono::high_resolution_clock::now();
        FloydWarshallResult res = floyd_warshall(M);
        auto t2 = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
        out << "[FW]   " << path << " | V=" << M.V
            << " | " << (res.negative_cycle ? "NEGATIVE CYCLE" : "ok") << " | time=" << ms << " ms\n";
        locked_print(out.str());
    }
    void run_fw_suite(const std::vector<std::string> &files)
    {
        for (auto &f : files)
            run_fw_file(f);
    }

    void run_cross_check_file(const std::string &path)
    {
        std::ostringstream out;
        AdjacencyList g;
        if (!read_adjacency_list(path, g))
        {
            out << "[XCHK] " << path << " -> ERROR: could not read/parse file\n";
            locked_print(out.str());
            return;
        }
        CSR csr = build_csr(g);
        DenseMatrix M = csr_to_dense_matrix(csr);
        FloydWarshallResult fw = floyd_warshall(M);
        int mismatches = 0;
        for (int s = 0; s < csr.V; ++s)  //for each source
        {
            BellmanFordResult bf = bellman_ford_csr(csr, s);
            if (bf.negative_cycle != fw.negative_cycle)
            {
                ++mismatches;
                continue;
            }
            if (bf.negative_cycle)
                continue;
            for (int v = 0; v < csr.V; ++v)  // check for each vertex destination for s source
            {
                double a = bf.distance[v], b = fw.distance.at(s, v);            // bf dist and fw dist of s,v
                bool ok = (std::isinf(a) && std::isinf(b)) || std::abs(a - b) < 1e-6;  // if both are inreachable or dist < 1e-6
                if (!ok)
                {
                    ++mismatches;
                    break;
                }
            }
        }
        out << "[XCHK] " << path << " | V=" << csr.V << " | "
            << (mismatches == 0 ? "ALL SOURCES MATCH" : "MISMATCHES FOUND")
            << " (" << (csr.V - mismatches) << "/" << csr.V << ")\n";
        locked_print(out.str());
    }

    void menu_bf()
    {
        std::cout << "\n-- Bellman-Ford --\n1. Run a single test file\n2. Run all *.txt files in a folder\n3. Back\n";
        int c = prompt_int("Choose: ", 3);
        if (c == 1)
            run_bf_file(prompt_line("Input file path: "));
        else if (c == 2)
        {
            std::string dir = prompt_line("Folder path [default assignment_2/tests/bf_gen]: ");
            if (dir.empty())
                dir = "assignment_2/tests/bf_gen";
            auto files = list_txt_files(dir);
            if (files.empty())
            {
                std::cout << "No .txt files found in " << dir << "\n";
                return;
            }
            std::cout << "Running " << files.size() << " Bellman-Ford test file(s)...\n";
            run_bf_suite(files);
        }
    }
    void menu_fw()
    {
        std::cout << "\n-- Floyd-Warshall --\n1. Run a single test file\n2. Run all *.txt files in a folder\n3. Back\n";
        int c = prompt_int("Choose: ", 3);
        if (c == 1)
            run_fw_file(prompt_line("Input file path: "));
        else if (c == 2)
        {
            std::string dir = prompt_line("Folder path [default assignment_2/tests/fw_gen]: ");
            if (dir.empty())
                dir = "assignment_2/tests/fw_gen";
            auto files = list_txt_files(dir);
            if (files.empty())
            {
                std::cout << "No .txt files found in " << dir << "\n";
                return;
            }
            std::cout << "Running " << files.size() << " Floyd-Warshall test file(s)...\n";
            run_fw_suite(files);
        }
    }
    void menu_cross_check()
    {
        std::cout << "\n-- Cross-check (BF every source vs FW) --\n";
        std::string dir = prompt_line("Folder of BF-format files [default assignment_2/tests/bf_gen]: ");
        if (dir.empty())
            dir = "assignment_2/tests/bf_gen";
        auto files = list_txt_files(dir);
        if (files.empty())
        {
            std::cout << "No .txt files found in " << dir << "\n";
            return;
        }
        std::cout << "Cross-checking " << files.size() << " file(s)...\n";
        for (auto &f : files)
            run_cross_check_file(f);
    }
    void menu_run_all()
    {
        std::string bf_dir = prompt_line("BF folder [default assignment_2/tests/bf_gen]: ");
        if (bf_dir.empty())
            bf_dir = "assignment_2/tests/bf_gen";
        std::string fw_dir = prompt_line("FW folder [default assignment_2/tests/fw_gen]: ");
        if (fw_dir.empty())
            fw_dir = "assignment_2/tests/fw_gen";
        auto bf_files = list_txt_files(bf_dir);
        auto fw_files = list_txt_files(fw_dir);
        if (bf_files.empty() && fw_files.empty())
        {
            std::cout << "No test files found.\n";
            return;
        }

        std::cout << "Launching Bellman-Ford and Floyd-Warshall concurrently...\n\n";
        auto t0 = std::chrono::high_resolution_clock::now();
        std::thread t1(run_bf_suite, bf_files);
        std::thread t2(run_fw_suite, fw_files);
        t1.join();
        t2.join();
        auto t_end = std::chrono::high_resolution_clock::now();
        std::cout << "\nAssignment 2 finished. Wall-clock: "
                  << std::chrono::duration<double, std::milli>(t_end - t0).count() << " ms\n";
    }

    void open_submenu()
    {
        while (true)
        {
            std::cout << "\n== Assignment 2 - Individual: Bellman-Ford + Floyd-Warshall ==\n"
                      << "1. Run Bellman-Ford\n2. Run Floyd-Warshall\n"
                      << "3. Cross-check BF vs FW (required for V=10, 100)\n"
                      << "4. Run All (this assignment)\n5. Back to main menu\n";
            int c = prompt_int("Choose an option: ", 5);
            switch (c)
            {
            case 1:
                menu_bf();
                break;
            case 2:
                menu_fw();
                break;
            case 3:
                menu_cross_check();
                break;
            case 4:
                menu_run_all();
                break;
            case 5:
                return;
            default:
                std::cout << "Invalid choice, try again.\n";
                break;
            }
        }
    }

    void launch_concurrent(std::vector<std::thread> &threads)
    {
        auto bf_files = list_txt_files("assignment_2/tests/bf_gen");
        auto fw_files = list_txt_files("assignment_2/tests/fw_gen");
        threads.emplace_back(run_bf_suite, bf_files);
        threads.emplace_back(run_fw_suite, fw_files);
    }

}

// ===========================================================================
// Assignment 3 (MST with Kruskal and Prim's)
// ===========================================================================
namespace a3{
    void run_mst_file(const std::string &path)
    {
        AdjacencyList g;
        if (!read_adjacency_list(path, g))
        {
            std::cout << path << " -> ERROR: could not read/parse file\n";
            return;
        }

        // Preprocessing: adjacency-list -> CSR. NOT part of the timed algorithm.
        CSR csr = build_csr(g);

        auto t1 = std::chrono::high_resolution_clock::now();
        MSTResult kr = kruskal_mst(csr);
        auto t2 = std::chrono::high_resolution_clock::now();
        double kr_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();

        auto t3 = std::chrono::high_resolution_clock::now();
        MSTResult pr = prim_mst(csr, 0);
        auto t4 = std::chrono::high_resolution_clock::now();
        double pr_ms = std::chrono::duration<double, std::milli>(t4 - t3).count();

        std::cout << "\nFile: " << path << " | V=" << g.V << " E=" << g.E << "\n";

        std::cout << "Algorithm: Kruskal's MST\nMST edges:\n";

        if (g.V <= 1000)
        {
            for (const auto &e : kr.edges)
                std::cout << e.u << " " << e.v << " " << e.weight << "\n";
        }
        std::cout << "Total MST weight: " << kr.total_weight << "\n";
        std::cout << "Execution time: " << kr_ms << " ms\n\n";

        std::cout << "Algorithm: Prim's MST\nMST edges:\n";
        if (g.V <= 1000)
        {
            for (const auto &e : pr.edges)
                std::cout << e.u << " " << e.v << " " << e.weight << "\n";
        }
        std::cout << "Total MST weight: " << pr.total_weight << "\n";
        std::cout << "Execution time: " << pr_ms << " ms\n";

        bool equal = std::fabs(kr.total_weight - pr.total_weight) < 1e-6;
        std::cout << "\nWeights equal: " << (equal ? "Yes" : "No") << "\n";
    }

    void run_mst_suite(const std::vector<std::string> &files)
    {
        for (const auto &f : files)
            run_mst_file(f);
    }

    void open_menu()
    {
        while (true)
        {
            std::cout << "\n== Assignment 3 (Individual): MST - Kruskal + Prim ==\n"
                      << "1. Run a single test file\n"
                      << "2. Run all *.txt files in a folder\n"
                      << "3. Back\n";
            int c = common::prompt_int("Choose: ", 3);
            if (c == 1)
            {
                run_mst_file(common::prompt_line("Input file path: "));
            }
            else if (c == 2)
            {
                std::string dir = common::prompt_line("Folder path [default tests]: ");
                if (dir.empty())
                    dir = "assignment_3/tests";
                auto files = common::list_txt_files(dir);
                if (files.empty())
                {
                    std::cout << "No .txt files found in " << dir << "\n";
                    continue;
                }
                std::cout << "Running " << files.size() << " MST test file(s)...\n";
                run_mst_suite(files);
            }
            else
            {
                return;
            }
        }
    }

    void launch_concurrent(std::vector<std::thread> &threads)
    {
        auto files = common::list_txt_files("assignment_3/tests");
        threads.emplace_back(run_mst_suite, files);
    }
}


// ===========================================================================
// Registry + main menu
// ===========================================================================

struct AssignmentModule
{
    std::string label;
    std::function<void()> open_submenu;
    std::function<void(std::vector<std::thread> &)> launch_concurrent;
};

int main()
{
    std::cout << "==================================================\n"
              << " CS509 - Master Task Runner\n"
              << "==================================================\n";

    std::vector<AssignmentModule> modules = {
        {"Assignment 1 (GEMM + CSR )", a1::open_submenu, a1::launch_concurrent},
        {"Assignment 2 (Bellman-Ford + Floyd-Warshall)", a2i::open_submenu, a2i::launch_concurrent},
        {"Assignment 3 (MST - Kruskal + Prim)", a3::open_menu, a3::launch_concurrent},
    };

    while (true)
    {
        std::cout << "\nWhich assignment would you like to run?\n";
        for (size_t i = 0; i < modules.size(); ++i)
        {
            std::cout << (i + 1) << ". " << modules[i].label << "\n";
        }
        int run_all_choice = static_cast<int>(modules.size()) + 1;
        int exit_choice = static_cast<int>(modules.size()) + 2;
        std::cout << run_all_choice << ". Run EVERYTHING (every assignment)\n";
        std::cout << exit_choice << ". Exit\n";

        int choice = common::prompt_int("Choose an option: ", exit_choice);

        if (choice >= 1 && choice <= static_cast<int>(modules.size()))
        {
            modules[choice - 1].open_submenu();
        }
        else if (choice == run_all_choice)
        {
            std::cout << "Launching every assignment's default test suite concurrently...\n\n";
            std::vector<std::thread> threads;
            auto t0 = std::chrono::high_resolution_clock::now();
            for (auto &m : modules)
                m.launch_concurrent(threads);
            for (auto &t : threads)
                t.join();
            auto t_end = std::chrono::high_resolution_clock::now();
            std::cout << "\nEVERYTHING finished (" << threads.size() << " suites across "
                      << modules.size() << " assignments). Total wall-clock: "
                      << std::chrono::duration<double, std::milli>(t_end - t0).count() << " ms\n"
                      << std::endl;
        }
        else if (choice == exit_choice)
        {
            std::cout << "Goodbye.\n";
            return 0;
        }
        else
        {
            std::cout << "Invalid choice, try again.\n";
        }
    }
}
