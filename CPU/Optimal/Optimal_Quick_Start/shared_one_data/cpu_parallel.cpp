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
#include <omp.h>
#include "cpu_parallel.h"
using namespace std;

/*
 * Class CPUParallel
 */
CPUParallel::CPUParallel(const char* filename) {
    /* Read data file in */
    DataUtils* data_utils_ptr = DataUtils::getInstance();
    //cout<<"Current num of nodes = "<<data_utils_ptr->getNumOfNodes()<<endl;
    
    ifstream data_file;
    data_file.open(filename);
    if (data_file.is_open()) {
        //cout<<"File opened"<<endl;
        int id1, id2;
        while (data_file.eof() == false) {
            data_file>>id1>>id2;
            //cout<<id1<<" "<<id2<<endl;
            data_utils_ptr->addEdge(id1, id2);
        }
    } else {
        cout<<"File did not open"<<endl;
        exit(0);
    }
    data_file.close();
    
    int total_user_count = data_utils_ptr->getNumOfNodes();
    cout<<"#Node := "<<data_utils_ptr->getNumOfNodes()<<endl;
    cout<<"#Edge := "<<data_utils_ptr->getNumOfEdges()<<endl;

    /* Initialization */
    cout<<">>>Initializing. It may take a while..."<<endl;
    this->raw_data_ptr = data_utils_ptr;
    this->all_result_ptr = ResultsAllUsers::getInstance();
    this->all_user_list = data_utils_ptr->getAllNodes();

    unordered_set<int> all_user_set;
    int user_count = all_user_list.size();
    
    for (int i = 0; i < user_count; i++) {
        all_user_set.insert(all_user_list[i]);
    }
    
    for (int i = 0; i < all_user_list.size(); i++) {
        this->all_result_ptr->addUserById(all_user_list[i], total_user_count);
    }

    cout<<">>>Initialization finished!"<<endl;
}

/* Return the degree of separation betwen user 1 and user 2. */
vector<int> CPUParallel::getDOS(int user_id1, int user_id2) {
    ResultsPerUser* user_results_ptr = this->all_result_ptr->getResultsByUser(user_id1);
    vector<int> result_path;
    if (user_results_ptr == NULL) {
        return result_path;
    }

    vector<OneLevelInfo>* all_level_info_list = user_results_ptr->getAllLevelInfoList();
    int total_level_count = all_level_info_list->size();

    for (int i = 0; i < total_level_count; i++) {
        vector<UserTrace>* one_level_user_list = (*all_level_info_list)[i].getCurrentLevelUserList();
        int user_this_level_count = one_level_user_list->size();
        for (int j = 0; j < user_this_level_count; j++) {
            if ((*one_level_user_list)[j].user_id == user_id2) {
                // Found solution. Reconstructe path backwards from user_id2 to user_id1
                int current_level = i - 1;
                int previous_id = (*one_level_user_list)[j].previous_id;
                int current_id = user_id2;
                while (current_level >= 0) {
                    result_path.push_back(current_id);
                    vector<UserTrace>* previous_level_user_list = (*all_level_info_list)[current_level].getCurrentLevelUserList();
                    for (int k = 0; k < previous_level_user_list->size(); k++) {
                        if ((*previous_level_user_list)[k].user_id == previous_id) {
                            current_id = previous_id;
                            previous_id = (*previous_level_user_list)[k].previous_id;
                            break;
                        }
                    }
                    current_level--;
                }
                result_path.push_back(current_id); // push back the last node excluded by the while loop
                return result_path;
            }
        }
    }

    return result_path;
}

/* Search for all users and deepen their dos info by one level */
void CPUParallel::deepenOneLevel() {
    cout<<">>>Deepen by one level. It may take a while..."<<endl;

    int user_count = all_user_list.size();

    /* Search one by one for each user. Should go parallel here */
    #pragma omp parallel for
    for (int i = 0; i < user_count; i++) {
        all_result_ptr->getResultsByUser(all_user_list[i])->deepenOneLevel();
    }
}