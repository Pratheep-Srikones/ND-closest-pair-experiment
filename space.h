#ifndef SPACE_H
#define SPACE_H

#include <vector>
#include <random>
#include <algorithm>
#include <execution>
#include <cmath>

template <int N>
struct Point{
	std::array<float,N> coordinates;
	static float eucledianDistance(const Point<N>& p1, const Point<N>& p2);
};

template <int N>
inline float Point<N>::eucledianDistance(const Point<N>& p1, const Point<N>& p2){
	float sumOfSquares = 0.0f;
        for (int i = 0; i < N; ++i) {
            float diff = p1.coordinates[i] - p2.coordinates[i];
            sumOfSquares += (diff * diff);
        }
        return std::sqrt(sumOfSquares);
}


template <int dim, long size>
struct Space{
	int dimension;
	int pointsSize;
	std::vector<Point<dim>> points;
	Space();

};


template <int dim, long size>
inline Space<dim,size>::Space():dimension(dim),pointsSize(size),points(size){
	std::for_each(std::execution::par,std::begin(points),std::end(points),[](auto& point){
		thread_local std::random_device rd;
    	thread_local std::mt19937 eng(rd());
		std::uniform_real_distribution<float> dist(0.0f,1000.0f);

		for(auto& coor:point.coordinates){
			coor = dist(eng);
		}
	});
	
}



#endif
