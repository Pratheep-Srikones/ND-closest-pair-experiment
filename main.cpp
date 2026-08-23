#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <array>
#include "space.h"


template <int dim, long size>
float findMinDistBruteforce(Space<dim,size>& space){
	float min= INFINITY;
	std::array<Point<dim>,2> closest;
	for(long i=0;i<size;i++ ){
		for(long j=0;j<size;j++){
			if(i==j){continue;}
			float distance = Point<dim>::eucledianDistance(space.points[i],space.points[j]);
			if(distance < min){
				min=distance;
				std::cout<<"current minimum is "<<min<<std::endl;
				closest[0] = space.points[i];
				closest[1] = space.points[j];
			}

		}
	}
	return min;
	


}

struct ArrayHasher {                                                                                                                             
	template <size_t dim>                                                                                                                           
        std::size_t operator()(const std::array<int, dim>& a) const {                                                                                
            std::size_t h = 0;                                                                                                                       
            for (auto e : a) {                                                                                                                       
                h ^= std::hash<int>{}(e) + 0x9e3779b9 + (h<<6)+(h>>2);                                                                         
            }                                                                                                                                        
            return h;                                                                                                                                
        }                                                                                                                                            
};

template <size_t dim>                                                                                                                               
void neighbourhoodFilter(std::array<int, dim> a, int current_dim, std::vector<std::array<int, dim>>& changes) {                                         
        if (current_dim == 0) {                                                                                                                      
            changes.push_back(a);                                                                                                                    
            return;                                                                                                                                  
        }                                                                                                                                            
                                                                                                                                                     
        std::array<int, dim> b = a;                                                                                                                  
        std::array<int, dim> c = a; 
        b[current_dim-1] -=1;                                                                                                                     
        c[current_dim-1] +=1;                                                                 
        neighbourhoodFilter(b, current_dim-1, changes);                                                                                               
       	neighbourhoodFilter(c, current_dim-1, changes);
	neighbourhoodFilter(a, current_dim-1, changes);                                                                                                   
}

template <int dim, typename HashMap>                                                                                                             
float findMinAroundNeighbourhood(std::array<int, dim> grid_i, HashMap& gridhashmap, const Point<dim>& pi, const std::vector<std::array<int, dim>>& changes) {       
	float min = INFINITY;                                                                                                                           
        for (const auto& change : changes) {                                                                                                         
            std::array<int, dim> neighbour;                                                                                                          
            for (int d = 0; d < dim; ++d) {                                                                                                          
                neighbour[d] = change[d]+ grid_i[d];                                                                                                
            }                                                                                
            auto it = gridhashmap.find(neighbour);                                                                                                   
            if (it != gridhashmap.end()) {                                                                                                           
                for (const auto& ps : it->second) {                                                                                                  
                    float dist = Point<dim>::eucledianDistance(pi, ps);                                                                              
                    if (dist < min) {min = dist;}
		}
	    }
	}                                                                                                                                            
        return min;
}  



template <int dim, long size>                                                                                                                    
float findMinDistGridBased(Space<dim, size>& space) {                                                                                                           
	std::vector<std::array<int, dim>> changes;                                                                                                   
        std::array<int, dim> initial_a = {};                                                                                                         
        neighbourhoodFilter<dim>(initial_a, dim, changes);
	if(size<2){return INFINITY;}
        float delta = Point<dim>::eucledianDistance(space.points[0], space.points[1]);                                                               
        if (delta == 0.0f) { return delta; }                                                                                                                                                                                                 
        std::unordered_map<std::array<int, dim>, std::vector<Point<dim>>, ArrayHasher> gridhashmap;                                                  
                                                                                                                           
        for (long i = 0; i < size; ++i) {                                                                                                            
            const auto& pi = space.points[i];                                                                                                               
            std::array<int, dim> grid_i;
            for (int d = 0; d < dim; ++d) {
                grid_i[d] = std::floor(pi.coordinates[d] / delta);
            }
  
            float min_dist = findMinAroundNeighbourhood<dim>(grid_i, gridhashmap, pi, changes);
            
            gridhashmap[grid_i].push_back(pi);
  
            if (min_dist < delta) {
		    std::cout<<"minimum changed to "<<min_dist<<" regridination happening"<<std::endl;
		    delta = min_dist;
		    gridhashmap.clear();
		    if (delta == 0.0f) { return delta; }

		    for (long j = 0; j <= i; ++j) {
			    const auto& pj = space.points[j];
			    std::array<int, dim> grid_j;
			    for (int d = 0; d < dim; ++d) {
				    grid_j[d] = std::floor(pj.coordinates[d] / delta);
			    }
			    gridhashmap[grid_j].push_back(pj);
		    }
	    }
	}
        return delta;
    }




int main(){
	Space<3,1000000> s;
	std::cout<<"space generated"<<std::endl;
	/*   
        for(auto& p : s.points) {                                                                                                            
            for(auto& coor:p.coordinates){
		std::cout<<coor<<" ";
	    }                        
	    std::cout<<std::endl;
        }*/
	//auto mindistancebr = findMinDistBruteforce(s);
	auto mindistance = findMinDistGridBased(s);
	std::cout<<"min distance grid is "<<mindistance<<std::endl;
	//std::cout<<"min distance grid is "<<mindistance<<std::endl;
	return 0;
}
