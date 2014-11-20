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
#include <unordered_map>
#include <vector>
using namespace std;

class DataUtils {
private:
    DataUtils();
    static DataUtils* instance;
    
    int num_of_edges;
    unordered_map<int, vector<int> > raw_data_map;
    vector<int> user_list;

public:
    static DataUtils* getInstance();
    void addEdge(int user_id1, int user_id2);
    int getNumOfNodes();
    int getNumOfEdges();
    vector<int> getAllNodes();
    unordered_map<int, vector<int> > getRawDataMap();
};

#endif /* defined(__Parallel_Degree_of_Separation__data_utils__) */
