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

//changefinder(a,dim,&changes){
//	if(dim ==0){
//		return change.pushback(a)
//	}
//	b = a
//	c = a
//	b[dim-1] -= 1
//	c[dim-1] += 1
//
//	changefinder(b,dim-1,changes);
//	changefinder(c,dim-1,changes);
//	changefinder(a,dim-1,changes);
//	
//}

//vector<array[dim]> changes;
//changefinder(array<int,dim> a{},dim,changes);
//float findmin(grid_i,gridhashmap,pi){
//	min = inf
//	for change in changes{
//		neighbour = change+grid_i
//		for all ps in gridhashmap.get(hash(neighbour)){
//			dist = Point::euclideandistance(pi,ps)
//			if(dist<min){min = dist}
//
//		}
//
//	}	
//    
//
//}
//


float findMinDistGridBased(){
	//1. init delta = Point::euclideiandist(p0,p1)
	//if(delta == 0.0f){return delta}
	//3. init gridhashmap;
	//2. for each pi -> grid_i = pi/delta here i starts from 0 
	// 	
	// 	min_dist = findmin(grid_i, gridhashmap,pi)
	// 	gridhashmap <- hash(grid_i) pi
	// 	if(min_dist<delta)
	// 		delta = min_dist
	// 		gridhashmap.erase
	// 		if(delta==0.0f){return delta;}
	// 		for(j = p0-> pi)
	// 			insert hash(j/delta) -> gridhashmap
	// 	end if
	// return delta;
	return 0.0f;	

}


int main(){
	Space<10,100000000> s;
	std::cout<<"space generated"<<std::endl;
	/*   
        for(auto& p : s.points) {                                                                                                            
            for(auto& coor:p.coordinates){
		std::cout<<coor<<" ";
	    }                        
	    std::cout<<std::endl;
        }*/
	auto mindistance = findMinDistBruteforce(s);
	std::cout<<"min distance is "<<mindistance<<std::endl;
	return 0;
}
