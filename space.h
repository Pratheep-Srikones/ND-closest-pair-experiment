#ifndef SPACE_H
#define SPACE_H

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <execution>
#include <random>
#include <vector>

/// Represents a point in N-dimensional space.
template <std::size_t Dim>
struct Point {
    std::array<float, Dim> coordinates{};

    /// Calculates the Euclidean distance from this point to another point.
    [[nodiscard]] float distance_to(const Point<Dim>& other) const noexcept {
        float sum_of_squares = 0.0f;
        for (std::size_t i = 0; i < Dim; ++i) {
            float diff = coordinates[i] - other.coordinates[i];
            sum_of_squares += diff * diff;
        }
        return std::sqrt(sum_of_squares);
    }

    /// Static helper to compute Euclidean distance between two points.
    [[nodiscard]] static float euclidean_distance(const Point<Dim>& p1, const Point<Dim>& p2) noexcept {
        return p1.distance_to(p2);
    }

    /// Backward compatibility alias for misspelled legacy method name.
    [[nodiscard]] static float eucledianDistance(const Point<Dim>& p1, const Point<Dim>& p2) noexcept {
        return euclidean_distance(p1, p2);
    }
};

/// Defines the strategy used to order/sort points within N-dimensional space.
enum class SortStrategy {
    AxisAscending,               ///< Sort points along a specific coordinate axis in ascending order.
    AxisDescending,              ///< Sort points along a specific coordinate axis in descending order.
    RandomShuffle,               ///< Uniformly random shuffle of point order.
    DistanceToOriginAscending,   ///< Sort by distance from origin (0, 0, ...) ascending.
    DistanceToOriginDescending,  ///< Sort by distance from origin (0, 0, ...) descending.
    Adversarial                  ///< Order points to maximize grid rebuilds in incremental closest-pair algorithms.
};

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

/// Binary I/O helpers for point datasets
template <std::size_t Dim>
bool save_points_to_bin(const std::string& filepath, const std::vector<Point<Dim>>& points) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "[DatasetIO] Error opening file for writing: " << filepath << "\n";
        return false;
    }

    const char magic[4] = {'N', 'D', 'P', 'T'};
    uint32_t dim = static_cast<uint32_t>(Dim);
    uint64_t count = static_cast<uint64_t>(points.size());

    out.write(magic, sizeof(magic));
    out.write(reinterpret_cast<const char*>(&dim), sizeof(dim));
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    out.write(reinterpret_cast<const char*>(points.data()), count * sizeof(Point<Dim>));

    return out.good();
}

template <std::size_t Dim>
bool load_points_from_bin(const std::string& filepath, std::vector<Point<Dim>>& points) {
    std::ifstream in(filepath, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }

    char magic[4];
    uint32_t dim = 0;
    uint64_t count = 0;

    in.read(magic, 4);
    if (std::string(magic, 4) != "NDPT") {
        std::cerr << "[DatasetIO] Error: invalid magic bytes in " << filepath << "\n";
        return false;
    }

    in.read(reinterpret_cast<char*>(&dim), sizeof(dim));
    in.read(reinterpret_cast<char*>(&count), sizeof(count));

    if (dim != Dim) {
        std::cerr << "[DatasetIO] Error: Dimension mismatch in " << filepath
                  << " (file: " << dim << ", expected: " << Dim << ")\n";
        return false;
    }

    points.resize(count);
    in.read(reinterpret_cast<char*>(points.data()), count * sizeof(Point<Dim>));
    return in.good();
}

/// Container representing an N-dimensional space populated with random points.
template <std::size_t Dim, std::size_t Size>
struct Space {
    static constexpr std::size_t dimension = Dim;
    static constexpr std::size_t points_size = Size;

    std::vector<Point<Dim>> points;

    /// Default constructor creating uninitialized vector of size Size
    Space() : points(Size) {}

    /// Initializes points with uniform random coordinates in [min_val, max_val].
    explicit Space(float min_val, float max_val, uint64_t seed = 42) : points(Size) {
        std::mt19937_64 gen(seed);
        std::uniform_real_distribution<float> dist(min_val, max_val);

        for (auto& point : points) {
            for (auto& coord : point.coordinates) {
                coord = dist(gen);
            }
        }
    }

    [[nodiscard]] static Space<Dim, Size> create_uniform_space(float min_val = 0.0f, float max_val = 1000.0f, uint64_t seed = 42) {
        return Space<Dim, Size>(min_val, max_val, seed);
    }

    [[nodiscard]] static Space<Dim, Size> create_adversarial_space(float min_val = 0.0f, float max_val = 1000.0f) {
        Space<Dim, Size> space;
        if (Size == 0) return space;
        space.points.resize(Size);

        float range = max_val - min_val;
        float num_pairs = static_cast<float>(Size) / 2.0f;
        if (num_pairs < 1.0f) num_pairs = 1.0f;

        // Distribute pairs evenly across the available space on the Y-axis
        float y_spacing = range / (num_pairs + 1.0f);

        //  The distance inside the pair MUST be smaller than the distance between pairs!
        // Otherwise, a point from Pair 1 would be closer to Pair 2 than to its own partner, breaking the logic.
        float current_pair_dist = y_spacing * 0.9f; 
        float distance_decrement = current_pair_dist / (num_pairs * 2.0f); 
        float current_y = min_val + y_spacing;
        for (std::size_t i = 0; i < Size; i += 2) {
            Point<Dim> p1, p2;

            for (std::size_t d = 0; d < Dim; ++d) {
                p1.coordinates[d] = min_val;
                p2.coordinates[d] = min_val;
            }

            if (Dim > 1) {
                for (std::size_t d = 1; d < Dim; ++d) {
                    p1.coordinates[d] = current_y;
                    p2.coordinates[d] = current_y;
                }
                p1.coordinates[0] = min_val;
                p2.coordinates[0] = min_val + current_pair_dist;
            } else {
                p1.coordinates[0] = current_y;
                p2.coordinates[0] = current_y + current_pair_dist;
            }

            space.points[i] = p1;
            if (i + 1 < Size) {
                space.points[i + 1] = p2;
            }
            current_pair_dist -= distance_decrement;
            current_y += y_spacing;
        }

        return space;
    }

    /// Loads points from pre-existing dataset file, or generates and saves them if not found.
    [[nodiscard]] static Space<Dim, Size> get_or_create(const std::string& type = "uniform",
                                                        const std::string& dir = "datasets",
                                                        uint64_t seed = 42) {
        std::filesystem::create_directories(dir);
        std::string filename = dir + "/" + type + "_d" + std::to_string(Dim) + "_n" + std::to_string(Size) + ".bin";

        Space<Dim, Size> space;
        if (std::filesystem::exists(filename)) {
            if (load_points_from_bin<Dim>(filename, space.points) && space.points.size() == Size) {
                std::cout << "[Dataset] Loaded existing cached dataset: " << filename << "\n";
                return space;
            }
            std::cout << "[Dataset] Warning: Failed reading " << filename << ", regenerating...\n";
        }

        std::cout << "[Dataset] Generating and caching dataset: " << filename << "\n";
        if (type == "adversarial") {
            space = create_adversarial_space();
        } else {
            space = create_uniform_space(0.0f, 1000.0f, seed);
        }

        save_points_to_bin<Dim>(filename, space.points);
        return space;
    }

    /// Sorts or orders points according to the given SortStrategy.
    /// @param strategy Strategy enum defining the point ordering method.
    /// @param axis Coordinate dimension axis to sort along (used when strategy is AxisAscending or AxisDescending).
    void sort_points(SortStrategy strategy, std::size_t axis = 0) {
        if (axis >= Dim) {
            axis = 0;
        }

        switch (strategy) {
            case SortStrategy::AxisAscending:
                std::sort(std::execution::par, points.begin(), points.end(), [axis](const Point<Dim>& a, const Point<Dim>& b) {
                    return a.coordinates[axis] < b.coordinates[axis];
                });
                break;

            case SortStrategy::AxisDescending:
                std::sort(std::execution::par, points.begin(), points.end(), [axis](const Point<Dim>& a, const Point<Dim>& b) {
                    return a.coordinates[axis] > b.coordinates[axis];
                });
                break;

            case SortStrategy::RandomShuffle: {
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(points.begin(), points.end(), g);
                break;
            }

            case SortStrategy::DistanceToOriginAscending: {
                Point<Dim> origin{};
                std::sort(std::execution::par, points.begin(), points.end(), [&origin](const Point<Dim>& a, const Point<Dim>& b) {
                    return a.distance_to(origin) < b.distance_to(origin);
                });
                break;
            }

            case SortStrategy::DistanceToOriginDescending: {
                Point<Dim> origin{};
                std::sort(std::execution::par, points.begin(), points.end(), [&origin](const Point<Dim>& a, const Point<Dim>& b) {
                    return a.distance_to(origin) > b.distance_to(origin);
                });
                break;
            }

            case SortStrategy::Adversarial: {
                generate_adversarial_ordering();
                break;
            }
        }
    }

    /// Legacy convenience wrapper to sort points ascending along a specific axis.
    void sort_points_by_axis(int axis) {
        sort_points(SortStrategy::AxisAscending, static_cast<std::size_t>(axis < 0 ? 0 : axis));
    }

private:
    /// Rearranges points into an adversarial order for incremental grid closest pair.
    /// Points are reordered so that consecutive points decrease pairwise distance as frequently as possible,
    /// triggering frequent grid rebuilds.
    void generate_adversarial_ordering() {
        if (points.size() <= 2) return;

        // Sort points along primary axis to establish initial order
        std::sort(points.begin(), points.end(), [](const Point<Dim>& a, const Point<Dim>& b) {
            return a.coordinates[0] < b.coordinates[0];
        });

        // Construct adversarial sequence by interleaved/inward distance progression
        std::vector<Point<Dim>> adversarial_seq;
        adversarial_seq.reserve(points.size());

        std::size_t left = 0;
        std::size_t right = points.size() - 1;

        while (left <= right) {
            if (left == right) {
                adversarial_seq.push_back(points[left]);
                break;
            }
            adversarial_seq.push_back(points[left++]);
            adversarial_seq.push_back(points[right--]);

            if (left < right) {
                std::size_t mid = left + (right - left) / 2;
                adversarial_seq.push_back(points[mid]);
                std::swap(points[mid], points[left]);
                left++;
            }
        }

        points = std::move(adversarial_seq);
    }
};

#endif // SPACE_H
