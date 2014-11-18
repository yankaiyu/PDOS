//
//  cpu_parallel.cpp
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#include <cstdlib>
#include <iostream>
#include <fstream>
#include "cpu_parallel.h"
using namespace std;

/*
 * Class CPUParallel
 */
CPUParallel::CPUParallel(const char* filename) {
    /* Read data file in */
    DataUtils* data_utils_ptr = DataUtils::getInstance();
    cout<<"Current num of nodes = "<<data_utils_ptr->getNumOfNodes()<<endl;
    
    ifstream data_file;
    data_file.open(filename);
    if (data_file.is_open()) {
        //cout<<"File opened"<<endl;
        int id1, id2;
        while (data_file.eof() == false) {
            data_file>>id1>>id2;
            cout<<id1<<" "<<id2<<endl;
            data_utils_ptr->addEdge(id1, id2);
        }
    } else {
        cout<<"File did not open"<<endl;
        exit(0);
    }
    data_file.close();
    
    cout<<"Current num of nodes = "<<data_utils_ptr->getNumOfNodes()<<endl;
    cout<<"Current num of edges = "<<data_utils_ptr->getNumOfEdges()<<endl;

    /* Initialization */
    this->raw_data_ptr = data_utils_ptr;
    this->all_result_ptr = ResultsAllUsers::getInstance();
    this->all_user_list = data_utils_ptr->getAllNodes();

    cout<<">>>before insert user_ids at level 0"<<endl;
    for (int i = 0; i < all_user_list.size(); i++) {
        this->all_result_ptr->addUserById(all_user_list[i]);
    }
    cout<<">>>after insert user_ids at level 0"<<endl;
}

/* Return the degree of separation betwen user 1 and user 2. Return -1 if not connected */
int CPUParallel::getDOS(int user_id1, int user_id2) {
    ResultsPerUser* user_results_ptr = this->all_result_ptr->getOneUserResults(user_id1);
    if (user_results_ptr == NULL) {
        return -1;
    }

    vector<OneLevelInfo> all_level_info_list = user_results_ptr->getAllLevelInfoList();
    int total_level_count = all_level_info_list.size();

    for (int i = 0; i < total_level_count; i++) {
        vector<int> one_level_user_list = all_level_info_list[i].getCurrentLevelUserList();
        int user_this_level_count = one_level_user_list.size();
        for (int j = 0; j < user_this_level_count; j++) {
            if (one_level_user_list[j] == user_id2) {
                return i;
            }
        }
    }

    return -1;
}

/* Search for all users and extend their dos info by one level */
void CPUParallel::deepenOneLevel() {
    //
}
