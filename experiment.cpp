#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <execution>
#include <fstream>
#include <ctime>
#include <sstream>

#include "closest_pair.h"
#include "space.h"

using namespace std;

std::string current_timestamp() {
    std::time_t t = std::time(nullptr);
    char mbstr[100];
    if (std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%d %H:%M:%S", std::localtime(&t))) {
        return mbstr;
    }
    return "Unknown";
}

struct Stats {
    double mean;
    double median;
    double std_dev;
};

void log_to_csv(const std::string& space_type, size_t dim, size_t num_points, 
                const std::string& input_order, std::string algorithm,
                int iterations, float min_dist, const Stats& st,
                const std::vector<double>& raw_times) {
    std::string filename = "experiment_results.csv";
    std::ifstream check_file(filename);
    bool file_exists = check_file.good();
    check_file.close();
    
    std::ofstream file(filename, std::ios::app);
    if (!file_exists) {
        file << "Timestamp,Space_Type,Dimensions,Num_Points,Input_Order,Algorithm,Iterations,Min_Distance,Mean_Time_ms,Median_Time_ms,StdDev_Time_ms,Raw_Times_ms\n";
    }
    
    // Trim trailing spaces from algorithm name
    algorithm.erase(algorithm.find_last_not_of(" ") + 1);

    // Format raw times as a JSON-like array inside quotes so CSV parsers treat it as one column
    std::ostringstream raw_ss;
    raw_ss << "\"[";
    for (size_t i = 0; i < raw_times.size(); ++i) {
        raw_ss << std::fixed << std::setprecision(5) << raw_times[i];
        if (i < raw_times.size() - 1) raw_ss << ", ";
    }
    raw_ss << "]\"";

    file << current_timestamp() << ","
         << space_type << ","
         << dim << ","
         << num_points << ","
         << input_order << ","
         << algorithm << ","
         << iterations << ","
         << std::fixed << std::setprecision(5) << min_dist << ","
         << std::fixed << std::setprecision(5) << st.mean << ","
         << std::fixed << std::setprecision(5) << st.median << ","
         << std::fixed << std::setprecision(5) << st.std_dev << ","
         << raw_ss.str() << "\n";
}

Stats print_statistics(std::vector<double>& times) {
    Stats st = {0,0,0};
    if (times.empty()) return st;
    size_t n = times.size();
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    st.mean = sum / n;
    double variance_sum = 0.0;
    for (double time : times) {
        variance_sum += (time - st.mean) * (time - st.mean);
    }

    double variance = variance_sum / (n > 1 ? n - 1 : 1);
    st.std_dev = std::sqrt(variance);
    
    // We make a copy of times to sort so we don't scramble the original raw sequence
    std::vector<double> sorted_times = times;
    std::sort(sorted_times.begin(), sorted_times.end());
    
    if (n % 2 == 0) {
        st.median = (sorted_times[n/2 - 1] + sorted_times[n/2]) / 2.0;
    } else {
        st.median = sorted_times[n / 2];
    }
    
    std::cout << "    Mean     : " << st.mean << " ms\n";
    std::cout << "    Median   : " << st.median << " ms\n";
    std::cout << "    Std Dev  : " << st.std_dev << " ms\n";
    
    return st;
}

template <size_t Dim, size_t NumPoints>
float run_algorithm_multipleTimes(Space<Dim,NumPoints>& s, int k, bool isRand, const string& label, const string& space_type, const string& input_order) {
    std::vector<double> execution_times;
    execution_times.reserve(k);
    float min_val = 0.0f;
    
    for(int i = 0; i < k; i++) {
        auto start = chrono::high_resolution_clock::now();
        if (isRand) {
            TimePoint inner_start;
            min_val = find_min_dist_grid_based_randomized(s, &inner_start, false);
            start = inner_start; // Override the start time so we don't include the deep copy overhead!
        } else {
            min_val = find_min_dist_grid_based(s, false);
        }
        auto end = chrono::high_resolution_clock::now();
        chrono::duration<double, milli> ms = end - start;
        execution_times.push_back(ms.count());
    }
    
    std::cout << "  > " << label << " | Min Dist: " << min_val << "\n";
    Stats st = print_statistics(execution_times);
    std::cout << "\n";
    
    log_to_csv(space_type, Dim, NumPoints, input_order, label, k, min_val, st, execution_times);
    
    return min_val;
}

template <size_t Dim, size_t NumPoints>
void run_normal_space_test(const string& test_name) {
    cout << "==============================================================================\n";
    cout << "[NORMAL SPACE] " << test_name << " [" << NumPoints << " points in " << Dim << "D]\n";
    cout << "==============================================================================\n";

    Space<Dim, NumPoints> space;
    int iterations = 10;
    
    cout << "--- 1. Original Generation Order ---\n";
    run_algorithm_multipleTimes(space, iterations, false, "Deterministic Grid", "Normal", "Original");
    run_algorithm_multipleTimes(space, iterations, true,  "Randomized Grid   ", "Normal", "Original");

    cout << "--- 2. Sorted Order (Axis Ascending) ---\n";
    space.sort_points(SortStrategy::AxisAscending, 0);
    run_algorithm_multipleTimes(space, iterations, false, "Deterministic Grid", "Normal", "Sorted_X_Axis");
    run_algorithm_multipleTimes(space, iterations, true,  "Randomized Grid   ", "Normal", "Sorted_X_Axis");
}

template <size_t Dim, size_t NumPoints>
void run_adversarial_space_test(const string& test_name) {
    cout << "==============================================================================\n";
    cout << "[ADVERSARIAL SPACE] " << test_name << " [" << NumPoints << " points in " << Dim << "D]\n";
    cout << "==============================================================================\n";

    auto space = Space<Dim, NumPoints>::create_adversarial_space();
    int iterations = 10;
    
    cout << "--- 1. Adversarial Generation Order ---\n";
    run_algorithm_multipleTimes(space, iterations, false, "Deterministic Grid", "Adversarial", "Ladder_of_Pairs");
    run_algorithm_multipleTimes(space, iterations, true,  "Randomized Grid   ", "Adversarial", "Ladder_of_Pairs");
}

int main() {
    cout << fixed << setprecision(5);
    cout << "\nStarting Exhaustive Closest Pair Performance Experiments...\n";
    cout << "This will run 2D, 3D, 5D, 7D, and 9D across Small, Medium, and Large datasets.\n";
    cout << "All results are continuously logged to experiment_results.csv.\n\n";

    // ================== 2D Tests ==================
    run_normal_space_test<2, 2000>("2D Small Set");
    run_adversarial_space_test<2, 2000>("2D Small Set");

    run_normal_space_test<2, 10000>("2D Medium Set");
    run_adversarial_space_test<2, 10000>("2D Medium Set");
    
    run_normal_space_test<2, 100000>("2D Large Set");
    run_adversarial_space_test<2, 100000>("2D Large Set");

    // ================== 3D Tests ==================
    run_normal_space_test<3, 2000>("3D Small Set");
    run_adversarial_space_test<3, 2000>("3D Small Set");

    run_normal_space_test<3, 10000>("3D Medium Set");
    run_adversarial_space_test<3, 10000>("3D Medium Set");
    
    run_normal_space_test<3, 100000>("3D Large Set");
    run_adversarial_space_test<3, 100000>("3D Large Set");

    // ================== 5D Tests ==================
    run_normal_space_test<5, 2000>("5D Small Set");
    run_adversarial_space_test<5, 2000>("5D Small Set");

    run_normal_space_test<5, 10000>("5D Medium Set");
    run_adversarial_space_test<5, 10000>("5D Medium Set");

    run_normal_space_test<5, 80000>("5D Large Set");
    run_adversarial_space_test<5, 80000>("5D Large Set");

    // ================== 7D Tests ==================
    run_normal_space_test<7, 2000>("7D Small Set");
    run_adversarial_space_test<7, 2000>("7D Small Set");

    run_normal_space_test<7, 10000>("7D Medium Set");
    run_adversarial_space_test<7, 10000>("7D Medium Set");

    run_normal_space_test<7, 60000>("7D Large Set");
    run_adversarial_space_test<7, 60000>("7D Large Set");

    // ================== 9D Tests (Extreme Grid Overhead) ==================
    run_normal_space_test<9, 2000>("9D Small Set");
    run_adversarial_space_test<9, 2000>("9D Small Set");

    run_normal_space_test<9, 10000>("9D Medium Set");
    run_adversarial_space_test<9, 10000>("9D Medium Set");

    run_normal_space_test<9, 40000>("9D Large Set");
    run_adversarial_space_test<9, 40000>("9D Large Set");

    cout << "All exhaustive experiments completed successfully!\n";
    return 0;
}
