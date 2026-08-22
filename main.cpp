#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "space.h"
int main(){
	Space<10,100000000> s;                                                                                                          
        for(auto& p : s.points) {                                                                                                            
            for(auto& coor:p.coordinates){
		std::cout<<coor<<" ";
	    }                        
	    std::cout<<std::endl;
        }
	return 0;
}
