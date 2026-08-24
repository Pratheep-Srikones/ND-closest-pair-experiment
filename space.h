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

/// Container representing an N-dimensional space populated with random points.
template <std::size_t Dim, std::size_t Size>
struct Space {
    static constexpr std::size_t dimension = Dim;
    static constexpr std::size_t points_size = Size;

    std::vector<Point<Dim>> points;

    /// Initializes points with uniform random coordinates in [min_val, max_val].
    explicit Space(float min_val = 0.0f, float max_val = 1000.0f) : points(Size) {
        std::for_each(std::execution::par, points.begin(), points.end(), [min_val, max_val](Point<Dim>& point) {
            thread_local std::random_device rd;
            thread_local std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dist(min_val, max_val);

            for (auto& coord : point.coordinates) {
                coord = dist(gen);
            }
        });
    }
};

#endif // SPACE_H
