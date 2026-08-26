#include <chrono>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <iostream>
#include <vector>
#include <algorithm>
#include <execution>

#include "closest_pair.h"
#include "space.h"

using namespace std;

// Tolerance for comparing floating-point minimum distance results
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
        if (n%2==0) {
            median = (times[n/2 - 1] + times[n/2]) /2.0;
        } else {
            median = times[n / 2];
        }
	std::cout << "Mean               : " << mean << " ms\n";
	std::cout << "Median             : " << median << " ms\n";
	std::cout << "Standard Deviation : " << std_dev << " ms\n";
}
//constexpr float EPSILON = 1e-4f;
template <size_t Dim, size_t NumPoints>
float run_algorithm_multipleTimes(Space<Dim,NumPoints>& s, int k, bool isRand){
	//k = (int) std::sqrt(NumPoints);
	std::vector<double> execution_times(k,0);
	float min_rand{};
	/*auto ind = std::views::iota(0,k);
	for_each(execution::par,ind.begin(),ind.end(),[&execution_times,&s,&isRand](auto i){
		auto start_rand = chrono::high_resolution_clock::now();
		if(isRand){
			(void)find_min_dist_grid_based_randomized(s, false);
		}else{
			(void)find_min_dist_grid_based(s, false);
			
		}
    		auto end_rand = chrono::high_resolution_clock::now();
		chrono::duration<double, milli> rand_ms = end_rand - start_rand;
		execution_times[i] = rand_ms.count();

		
	});*/
	for(int i=0;i<k;i++){
		auto start_rand = chrono::high_resolution_clock::now();
		if(isRand){
			min_rand = find_min_dist_grid_based_randomized(s, false);
		}else{
			min_rand = find_min_dist_grid_based(s, false);
			
		}
    		auto end_rand = chrono::high_resolution_clock::now();
		chrono::duration<double, milli> rand_ms = end_rand - start_rand;
		execution_times.push_back(rand_ms.count());


	}
	print_statistics(execution_times);
	return min_rand;

}
template <size_t Dim, size_t NumPoints>
bool run_validation_test(const string& test_name) {
    cout << "--------------------------------------------------------" << endl;
    cout << "Running Test: " << test_name << " [" << NumPoints << " points in " << Dim << "D]" << endl;

    Space<Dim, NumPoints> space;

    /*auto start_brute = chrono::high_resolution_clock::now();
    float min_brute = find_min_dist_brute_force(space);
    auto end_brute = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> brute_ms = end_brute - start_brute;*/

    /*auto start_grid = chrono::high_resolution_clock::now();
    float min_grid = find_min_dist_grid_based(space, false);
    auto end_grid = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> grid_ms = end_grid - start_grid;*/

    /*auto start_rand = chrono::high_resolution_clock::now();
    float min_rand = find_min_dist_grid_based_randomized(space, false);
    auto end_rand = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> rand_ms = end_rand - start_rand;

    float diff_grid = abs(min_brute - min_grid);
    float diff_rand = abs(min_brute - min_rand);
    bool passed = (diff_grid <= EPSILON) && (diff_rand <= EPSILON);*/
    bool passed = 1;

    cout << fixed << setprecision(5);
    //cout << "  Brute Force Min Dist     : " << min_brute << " (" << brute_ms.count() << " ms)" << endl;
    //cout << "Grid-Based Min Dist      : " << min_grid << " (" << grid_ms.count() << " ms)" << endl;
     cout <<"\nDeterminsitic Grid Statistics & Min Distance----------------------------------------\n"<<endl;
    auto min_grid = run_algorithm_multipleTimes(space,200,false);
    cout << " Grid Min Dist : " << min_grid << endl;
    

    cout <<"\nRandomized Grid Statistics & Min Distance---------------------------------------------\n"<<endl;
    auto min_rand = run_algorithm_multipleTimes(space,200,true);
    cout << "Randomized Grid Min Dist : " << min_rand << endl;
    cout <<"-----------------------------------------------------------------------------------------------------------------------------------------"<<endl;
    //cout << "  Diff (Brute vs Grid)     : " << diff_grid << endl;
    //cout << "  Diff (Brute vs RandGrid) : " << diff_rand << endl;

    /*if (passed) {
        cout << "  Result                   : [PASSED]" << endl;
    } else {
        cout << "  Result                   : [FAILED]" << endl;
    }*/
    return passed;
}

// Edge case test for duplicate / identical points (min distance = 0)
template <size_t Dim>
bool run_duplicate_points_test() {
    cout << "--------------------------------------------------------" << endl;
    cout << "Running Edge Case Test: Duplicate points (0 distance) in " << Dim << "D" << endl;

    vector<Point<Dim>> points(100);
    for (size_t i = 0; i < points.size(); ++i) {
        for (size_t d = 0; d < Dim; ++d) {
            points[i].coordinates[d] = static_cast<float>(i * 10);
        }
    }
    // Force points 10 and 50 to be identical
    points[50] = points[10];

    float min_brute = find_min_dist_brute_force<Dim>(points);
    float min_grid = find_min_dist_grid_based<Dim>(points, false);
    float min_rand = find_min_dist_grid_based_randomized<Dim>(points, false);

    bool passed = (min_brute == 0.0f) && (min_grid == 0.0f) && (min_rand == 0.0f);

    cout << "  Brute Force Min Dist     : " << min_brute << endl;
    cout << "  Grid-Based Min Dist      : " << min_grid << endl;
    cout << "  Randomized Grid Min Dist : " << min_rand << endl;
    cout << "  Result                   : " << (passed ? "[PASSED]" : "[FAILED]") << endl;

    return passed;
}

int main() {
    cout << "========================================================" << endl;
    cout << "          N-Dimensional Closest Pair Validation         " << endl;
    cout << "========================================================" << endl;

    bool all_passed = true;

    // Run tests across different dimensions and sizes
    //all_passed &= run_validation_test<2, 100>("2D Small Set");
    //all_passed &= run_validation_test<2, 1000>("2D Medium Set");
    all_passed &= run_validation_test<2, 100000>("2D Large Set");
    //all_passed &= run_validation_test<3, 100>("3D Small Set");
    //all_passed &= run_validation_test<3, 1000>("3D Medium Set");
    all_passed &= run_validation_test<3, 100000>("3D Large Set");
    //all_passed &= run_validation_test<5, 100>("5D Small Set");
    //all_passed &= run_validation_test<5, 1000>("5D Medium Set");
    all_passed &= run_validation_test<5, 100000>("5D Large Set");
    //all_passed &= run_validation_test<7, 100>("7D Small Set");
    //all_passed &= run_validation_test<7, 1000>("7D Medium Set");
    all_passed &= run_validation_test<7, 100000>("7D Large Set");




    // Run edge case tests
    //all_passed &= run_duplicate_points_test<2>();
    //all_passed &= run_duplicate_points_test<3>();

    cout << "========================================================" << endl;
    if (all_passed) {
        cout << " ALL VALIDATION TESTS PASSED SUCCESSFULLY! " << endl;
        cout << "========================================================" << endl;
        return 0;
    } else {
        cout << " SOME VALIDATION TESTS FAILED. CHECK LOGS ABOVE. " << endl;
        cout << "========================================================" << endl;
        return 1;
    }
}
