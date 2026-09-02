#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <execution>

#include "closest_pair.h"
#include "space.h"

using namespace std;

void print_statistics(std::vector<double>& times) {                                                                                              
    if (times.empty()) return;
    size_t n = times.size();
    double sum = std::accumulate(times.begin(), times.end(), 0.0);
    double mean = sum / n;
    double variance_sum = 0.0;
    for (double time : times) {
        variance_sum += (time-mean) * (time-mean);
    }

    double variance = variance_sum / (n > 1 ? n - 1 : 1);
    double std_dev = std::sqrt(variance);
    std::sort(times.begin(), times.end());
    
    double median = 0.0;
    if (n % 2 == 0) {
        median = (times[n/2 - 1] + times[n/2]) / 2.0;
    } else {
        median = times[n / 2];
    }
    
    std::cout << "    Mean     : " << mean << " ms\n";
    std::cout << "    Median   : " << median << " ms\n";
    std::cout << "    Std Dev  : " << std_dev << " ms\n";
}

template <size_t Dim, size_t NumPoints>
float run_algorithm_multipleTimes(Space<Dim,NumPoints>& s, int k, bool isRand, const string& label) {
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
    print_statistics(execution_times);
    std::cout << "\n";
    
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
    run_algorithm_multipleTimes(space, iterations, false, "Deterministic Grid");
    run_algorithm_multipleTimes(space, iterations, true,  "Randomized Grid   ");

    cout << "--- 2. Sorted Order (Axis Ascending) ---\n";
    space.sort_points(SortStrategy::AxisAscending, 0);
    run_algorithm_multipleTimes(space, iterations, false, "Deterministic Grid");
    run_algorithm_multipleTimes(space, iterations, true,  "Randomized Grid   ");
}

template <size_t Dim, size_t NumPoints>
void run_adversarial_space_test(const string& test_name) {
    cout << "==============================================================================\n";
    cout << "[ADVERSARIAL SPACE] " << test_name << " [" << NumPoints << " points in " << Dim << "D]\n";
    cout << "==============================================================================\n";

    auto space = Space<Dim, NumPoints>::create_adversarial_space();
    int iterations = 10;
    
    cout << "--- 1. Adversarial Generation Order ---\n";
    run_algorithm_multipleTimes(space, iterations, false, "Deterministic Grid");
    run_algorithm_multipleTimes(space, iterations, true,  "Randomized Grid   ");
}

int main() {
    cout << fixed << setprecision(5);
    cout << "\nStarting Closest Pair Performance Experiments...\n\n";

    // 2D Tests
    run_normal_space_test<2, 10000>("2D Medium Set");
    run_adversarial_space_test<2, 10000>("2D Medium Set");
    
    run_normal_space_test<2, 40000>("2D Large Set");
    run_adversarial_space_test<2, 40000>("2D Large Set");

    // 3D Tests
    run_normal_space_test<3, 10000>("3D Medium Set");
    run_adversarial_space_test<3, 10000>("3D Medium Set");
    
    run_normal_space_test<3, 40000>("3D Large Set");
    run_adversarial_space_test<3, 40000>("3D Large Set");

    // 5D Tests
    run_normal_space_test<5, 10000>("5D Medium Set");
    run_adversarial_space_test<5, 10000>("5D Medium Set");

    run_normal_space_test<5, 20000>("5D Large Set");
    run_adversarial_space_test<5, 20000>("5D Large Set");

    // 7D Tests
    run_normal_space_test<7, 10000>("7D Medium Set");
    run_adversarial_space_test<7, 10000>("7D Medium Set");

    run_normal_space_test<7, 20000>("7D Large Set");
    run_adversarial_space_test<7, 20000>("7D Large Set");

    cout << "All experiments completed successfully!\n";
    return 0;
}
