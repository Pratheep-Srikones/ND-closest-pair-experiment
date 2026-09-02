#ifndef CLOSEST_PAIR_H
#define CLOSEST_PAIR_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <random>
#include <unordered_map>
#include <vector>
#include <ranges>
#include "space.h"

/// Represents grid cell indices in N-dimensional space using 64-bit integers to prevent overflow.
template <std::size_t Dim>
using GridCell = std::array<std::int64_t, Dim>;

/// Custom hash functor for GridCell<Dim> allowing use as std::unordered_map keys.
struct ArrayHasher {
    template <std::size_t Dim>
    std::size_t operator()(const GridCell<Dim>& arr) const noexcept {
        std::size_t hash_val = 0;
        for (const auto elem : arr) {
            // Boost hash_combine formula
            hash_val ^= std::hash<std::int64_t>{}(elem) + 0x9e3779b9 + (hash_val << 6) + (hash_val >> 2);
        }
        return hash_val;
    }
};

/// Type alias for the spatial grid hash map.
template <std::size_t Dim>
using GridHashMap = std::unordered_map<GridCell<Dim>, std::vector<Point<Dim>>, ArrayHasher>;

namespace detail {

/// Recursively populates neighbor grid cell offsets (-1, 0, +1 per dimension).
template <std::size_t Dim>
void build_neighbor_offsets_recursive(GridCell<Dim>& current, std::size_t current_dim, std::vector<GridCell<Dim>>& offsets) {
    if (current_dim == 0) {
        offsets.push_back(current);
        return;
    }
    for (std::int64_t offset : {-1, 0, 1}) {
        current[current_dim - 1] = offset;
        build_neighbor_offsets_recursive(current, current_dim - 1, offsets);
    }
}

} // namespace detail

/// Generates all 3^Dim relative cell offsets for neighboring grid cells.
template <std::size_t Dim>
[[nodiscard]] std::vector<GridCell<Dim>> generate_neighbor_offsets() {
    std::vector<GridCell<Dim>> offsets;
    offsets.reserve(static_cast<std::size_t>(std::pow(3, Dim)));
    GridCell<Dim> current{};
    detail::build_neighbor_offsets_recursive(current, Dim, offsets);
    return offsets;
}

/// Maps a point's coordinates to its corresponding grid cell index given cell size delta.
template <std::size_t Dim>
[[nodiscard]] GridCell<Dim> to_grid_cell(const Point<Dim>& point, float delta) noexcept {
    GridCell<Dim> cell{};
    for (std::size_t d = 0; d < Dim; ++d) {
        cell[d] = static_cast<std::int64_t>(std::floor(point.coordinates[d] / delta));
    }
    return cell;
}

/// Searches neighboring grid cells to find the minimum distance from point 'pi' to existing points in grid.
template <std::size_t Dim>
[[nodiscard]] float find_min_in_neighbor_cells(const GridCell<Dim>& center_cell,
                                               const GridHashMap<Dim>& grid_map,
                                               const Point<Dim>& pi,
                                               const std::vector<GridCell<Dim>>& neighbor_offsets) {
    float min_dist = std::numeric_limits<float>::infinity();

    for (const auto& offset : neighbor_offsets) {
        GridCell<Dim> neighbor_cell;
        for (std::size_t d = 0; d < Dim; ++d) {
            neighbor_cell[d] = center_cell[d] + offset[d];
        }

        auto it = grid_map.find(neighbor_cell);
        if (it != grid_map.end()) {
            for (const auto& point : it->second) {
                float dist = pi.distance_to(point);
                if (dist < min_dist) {
                    min_dist = dist;
                }
            }
        }
    }
    return min_dist;
}

/// Finds minimum distance between any pair of points in a vector using Rabin's incremental grid algorithm.
template <std::size_t Dim>
[[nodiscard]] float find_min_dist_grid_based(const std::vector<Point<Dim>>& points, bool verbose = false) {
    const std::size_t size = points.size();
    if (size < 2) {
        return std::numeric_limits<float>::infinity();
    }

    const auto neighbor_offsets = generate_neighbor_offsets<Dim>();

    // Initial grid parameter delta: distance between the first two points
    float delta = points[0].distance_to(points[1]);
    if (delta == 0.0f) {
        return 0.0f;
    }

    GridHashMap<Dim> grid_map;

    for (std::size_t i = 0; i < size; ++i) {
        const auto& pi = points[i];
        GridCell<Dim> cell_i = to_grid_cell(pi, delta);

        // Find minimum distance between pi and points already inserted in surrounding cells
        float min_dist = find_min_in_neighbor_cells(cell_i, grid_map, pi, neighbor_offsets);

        // Insert current point into the grid
        grid_map[cell_i].push_back(pi);

        // If a closer pair is found, update delta and rebuild grid with points seen so far (p0 ... pi)
        if (min_dist < delta) {
            if (verbose) {
                std::cout << "[GridAlgorithm] New min distance: " << min_dist << " -> Rebuilding grid..." << std::endl;
            }
            delta = min_dist;
            grid_map.clear();
            if (delta == 0.0f) {
                return 0.0f;
            }

            for (std::size_t j = 0; j <= i; ++j) {
                const auto& pj = points[j];
                grid_map[to_grid_cell(pj, delta)].push_back(pj);
            }
        }
    }
    return delta;
}

/// Overload for Space<Dim, Size> container.
template <std::size_t Dim, std::size_t Size>
[[nodiscard]] float find_min_dist_grid_based(const Space<Dim, Size>& space, bool verbose = false) {
    return find_min_dist_grid_based<Dim>(space.points, verbose);
}

#include <chrono>

using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock>;

/// Finds minimum distance between any pair of points using Rabin's randomized algorithm (shuffling point order first).
template <std::size_t Dim>
[[nodiscard]] float find_min_dist_grid_based_randomized(std::vector<Point<Dim>> points, TimePoint* out_start, bool verbose = false) {
    // Capture the time AFTER the std::vector deep copy has finished
    if (out_start) {
        *out_start = std::chrono::high_resolution_clock::now();
    }
    
    thread_local std::random_device rd;
    thread_local std::mt19937 g(rd());
    std::shuffle(points.begin(), points.end(), g);
    return find_min_dist_grid_based<Dim>(points, verbose);
}

template <std::size_t Dim>
[[nodiscard]] float find_min_dist_grid_based_randomized(std::vector<Point<Dim>> points, bool verbose = false) {
    return find_min_dist_grid_based_randomized<Dim>(points, nullptr, verbose);
}

/// Overload for Space<Dim, Size> container.
template <std::size_t Dim, std::size_t Size>
[[nodiscard]] float find_min_dist_grid_based_randomized(const Space<Dim, Size>& space, TimePoint* out_start, bool verbose = false) {
    return find_min_dist_grid_based_randomized<Dim>(space.points, out_start, verbose);
}

template <std::size_t Dim, std::size_t Size>
[[nodiscard]] float find_min_dist_grid_based_randomized(const Space<Dim, Size>& space, bool verbose = false) {
    return find_min_dist_grid_based_randomized<Dim>(space.points, nullptr, verbose);
}

/// Finds the minimum distance between any pair of points using O(N^2) brute-force iteration.
template <std::size_t Dim>
[[nodiscard]] float find_min_dist_brute_force(const std::vector<Point<Dim>>& points) {
    const std::size_t size = points.size();
    float min_dist = std::numeric_limits<float>::infinity();
    for (std::size_t i = 0; i < size; ++i) {
        for (std::size_t j = i + 1; j < size; ++j) {
            float dist = points[i].distance_to(points[j]);
            if (dist < min_dist) {
                min_dist = dist;
            }
        }
    }
    return min_dist;
}

/// Overload for Space<Dim, Size> container.
template <std::size_t Dim, std::size_t Size>
[[nodiscard]] float find_min_dist_brute_force(const Space<Dim, Size>& space) {
    return find_min_dist_brute_force<Dim>(space.points);
}

#endif // CLOSEST_PAIR_H
