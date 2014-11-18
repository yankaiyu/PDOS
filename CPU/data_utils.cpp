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

void DataUtils::addEdge(int user_id1, int user_id2) {
    map<int, vector<int> >::iterator it;
    
    // Insert edge into user_id1's firend list
    it = raw_data_map.find(user_id1);
    if (it == raw_data_map.end()) {
        vector<int> friend_list(1, user_id2);
        raw_data_map.insert(pair<int, vector<int> >(user_id1, friend_list));
    } else {
        it->second.push_back(user_id2);
    }

    // Insert edge into id2's firend list
    it = raw_data_map.find(user_id2);
    if (it == raw_data_map.end()) {
        vector<int> friend_list(1, user_id1);
        raw_data_map.insert(pair<int, vector<int> >(user_id2, friend_list));
    } else {
        it->second.push_back(user_id1);
    }
    
    num_of_edges++;
}

int DataUtils::getNumOfNodes() {
    return raw_data_map.size();
}

int DataUtils::getNumOfEdges() {
    return num_of_edges;
}

vector<int> DataUtils::getAllNodes() {
    vector<int> result;

    map<int, vector<int> >::iterator it;
    for (it = raw_data_map.begin(); it != raw_data_map.end(); it++) {
        result.push_back(it->first);
    }

    return result;
}

DataUtils *DataUtils::instance = NULL;