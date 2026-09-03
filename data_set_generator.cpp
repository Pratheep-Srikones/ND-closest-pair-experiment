#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "space.h"

namespace fs = std::filesystem;

using namespace std;

template <size_t Dim>
bool save_points(const string &filepath, const vector<Point<Dim>> &points) {
  ofstream out(filepath, ios::binary);
  if (!out.is_open()) {
    cerr << "error: could not open file for writing: " << filepath << endl;
    return false;
  }

  const char magic[4] = {'N', 'D', 'P', 'F'};
  uint32_t dim = static_cast<uint32_t>(Dim);
  uint32_t count = static_cast<uint32_t>(points.size());

  out.write(magic, sizeof(magic));
  out.write(reinterpret_cast<const char *>(&dim), sizeof(dim));
  out.write(reinterpret_cast<const char *>(&count), sizeof(count));

  out.write(reinterpret_cast<const char *>(points.data()),
            count * sizeof(Point<Dim>));

  out.close();
  return out.good();
}

template <size_t Dim>
bool load_points(const string &filepath, vector<Point<Dim>> &points) {
  ifstream in(filepath, ios::binary);
  if (!in.is_open()) {
    cerr << "error: could not open file for reading: " << filepath << endl;
    return false;
  }

  char magic[4];
  uint32_t dim;
  uint32_t count;

  in.read(magic, 4);

  if (std::string(magic, 4) != "NDPT") {
    cerr << "error: invalid file format (wrong magic bytes).\n";
    return false;
  }

  in.read(reinterpret_cast<char *>(&dim), sizeof(dim));
  in.read(reinterpret_cast<char *>(&count), sizeof(count));

  if (dim != Dim) {
    cerr << "error: dimension mismatch. file says " << dim << ", expected "
         << Dim << endl;
    return false;
  }

  points.resize(count);
  in.read(reinterpret_cast<char *>(points.data()), count * sizeof(Point<Dim>));

  in.close();
  return in.good();
}

template <size_t Dim>
vector<Point<Dim>> generate_uniform(size_t n, uint64_t seed = 42,
                                    float min_val = 0.0f,
                                    float max_val = 1000.0f) {
  mt19937_64 gen(seed);
  uniform_real_distribution<float> dist(min_val, max_val);

  vector<Point<Dim>> points(n);

  for (size_t i = 0; i < n; i++) {
    for (size_t d = 0; d < Dim; ++d) {
      points[i].coordinates[d] = dist(gen);
    }
  }

  return points;
}

template <size_t Dim>
vector<Point<Dim>> generate_adversarial(size_t n, float min_val = 0.0f,
                                        float max_val = 1000.0f) {
  vector<Point<Dim>> points(n);
  if (n == 0) {
    return points;
  }

  float range = max_val - min_val;
  float num_pairs = static_cast<float>(n) / 2.0f;
  if (num_pairs < 1.0f) {
    num_pairs = 1.0f;
  }

  float y_spacing = range / (num_pairs + 1.0f);
  float current_pair_dist = y_spacing * 0.9f;

  float current_y = min_val + y_spacing;

  for (size_t i = 0; i < n; i += 2) {
    Point<Dim> p1, p2;
    for (size_t d = 0; d < Dim; ++d) {
      p1.coordinates[d] = min_val;
      p2.coordinates[d] = min_val;
    }

    if (Dim > 1) {
      for (size_t d = 1; d < Dim; ++d) {
        p1.coordinates[d] = current_y;
        p2.coordinates[d] = current_y;
      }
      p1.coordinates[0] = min_val;
      p2.coordinates[0] = min_val + current_pair_dist;
    }

    points[i] = p1;

    if (i + 1 < n) {
      points[i + 1] = p2;
    }

    current_y += y_spacing;
    current_pair_dist *= 0.95f;
  }

  return points;
}

template <size_t Dim>
vector<Point<Dim>> generate_clustered(size_t n, size_t num_clusters = 5,
                                      uint32_t seed = 32, float min_val = 0.0f,
                                      float max_val = 1000.0f) {
  mt19937_64 gen(seed);
  uniform_real_distribution<float> center_dist(min_val + 100.0f,
                                               max_val - 100.0f);
  uniform_int_distribution<size_t> cluster_choice(0, num_clusters - 1);
  normal_distribution<float> offset_dist(0.0f, (max_val - min_val) * 0.03f);

  vector<array<float, Dim>> centers(num_clusters);
  for (size_t k = 0; k < num_clusters; ++k) {
    for (size_t d = 0; d < Dim; ++d) {
      centers[k][d] = center_dist(gen);
    }
  }

  vector<Point<Dim>> points(n);
  for (size_t i = 0; i < n; ++i) {
    size_t c = cluster_choice(gen);
    for (size_t d = 0; d < Dim; ++d) {
      float val = centers[c][d] + offset_dist(gen);
      points[i].coordinates[d] = std::clamp(val, min_val, max_val);
    }
  }

  return points;
}
