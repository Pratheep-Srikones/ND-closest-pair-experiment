#include <chrono>
#include <cstddef>
#include <iostream>

#include "closest_pair.h"
#include "space.h"

using namespace std;

int main() {
    constexpr size_t Dim = 3;
    constexpr size_t NumPoints = 10'000'000;

    cout << "Generating " << NumPoints << " points in " << Dim << "D space..." << endl;
    auto start_gen = chrono::high_resolution_clock::now();
    Space<Dim, NumPoints> space;
    auto end_gen = chrono::high_resolution_clock::now();
    chrono::duration<double> gen_time = end_gen - start_gen;
    cout << "Space generated in " << gen_time.count() << " s." << endl;

    cout << "\n--- Running Deterministic Grid-based Closest Pair ---" << endl;
    auto start_grid = chrono::high_resolution_clock::now();
    float min_dist_grid = find_min_dist_grid_based(space, false);
    auto end_grid = chrono::high_resolution_clock::now();
    chrono::duration<double> grid_time = end_grid - start_grid;
    cout << "Minimum distance (grid-based): " << min_dist_grid << endl;
    cout << "Completed in " << grid_time.count() << " s." << endl;

    cout << "\n--- Running Randomized (Shuffled) Grid-based Closest Pair ---" << endl;
    auto start_rand = chrono::high_resolution_clock::now();
    float min_dist_rand = find_min_dist_grid_based_randomized(space, false);
    auto end_rand = chrono::high_resolution_clock::now();
    chrono::duration<double> rand_time = end_rand - start_rand;
    cout << "Minimum distance (randomized grid): " << min_dist_rand << endl;
    cout << "Completed in " << rand_time.count() << " s." << endl;

    return 0;
}
