//
//  data_utils.cpp
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#include "data_utils.h"

DataUtils::DataUtils() {
    this->num_of_edges = 0;
}

DataUtils* DataUtils::getInstance() {
    if (!instance) {
        instance = new DataUtils();
    }
    return instance;
}

void DataUtils::insertEdge(int id1, int id2) {
    map<int, vector<int> >::iterator it;
    
    // Insert edge into id1's firend list
    it = raw_data_map.find(id1);
    if (it == raw_data_map.end()) {
        vector<int> a(1, id2);
        raw_data_map.insert(pair<int, vector<int> >(id1, a));
    } else {
        it->second.push_back(id2);
    }

    // Insert edge into id2's firend list
    it = raw_data_map.find(id2);
    if (it == raw_data_map.end()) {
        vector<int> a(1, id1);
        raw_data_map.insert(pair<int, vector<int> >(id2, a));
    } else {
        it->second.push_back(id1);
    }
    
    num_of_edges++;
}

int DataUtils::getNumOfNodes() {
    return this->raw_data_map.size();
}

int DataUtils::getNumOfEdges() {
    return num_of_edges;
}

DataUtils *DataUtils::instance = NULL;