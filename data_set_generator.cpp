#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "space.h"

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
