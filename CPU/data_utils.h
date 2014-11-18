//
//  data_utils.h
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#ifndef __Parallel_Degree_of_Separation__data_utils__
#define __Parallel_Degree_of_Separation__data_utils__

#include <stdio.h>
#include <map>
#include <vector>
using namespace std;

class DataUtils {
private:
    DataUtils();
    static DataUtils* instance;
    
    int num_of_edges;
    map<int, vector<int> > raw_data_map;
    
public:
    static DataUtils* getInstance();
    void insertEdge(int, int);
    int getNumOfNodes();
    int getNumOfEdges();
};

#endif /* defined(__Parallel_Degree_of_Separation__data_utils__) */