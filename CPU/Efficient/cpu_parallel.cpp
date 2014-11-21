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
    
    cout<<"#Node := "<<data_utils_ptr->getNumOfNodes()<<endl;
    cout<<"#Edge := "<<data_utils_ptr->getNumOfEdges()<<endl;

    /* Initialization */
    cout<<">>>Initializing. It may take a while..."<<endl;
    this->raw_data_ptr = data_utils_ptr;
    this->all_result_ptr = ResultsAllUsers::getInstance();
    this->all_user_list = data_utils_ptr->getAllNodes();

    int user_count = all_user_list.size();
    
    for (int i = 0; i < user_count; i++) {
        all_user_set.insert(all_user_list[i]);
    }
    
    for (int i = 0; i < all_user_list.size(); i++) {
        this->all_result_ptr->addUserById(all_user_list[i]);
    }

    //all_result_ptr->initAllUserSet(all_user_set);
    //all_result_ptr->initFriendList(this->raw_data_ptr->getRawDataMap());
    all_result_ptr->initUserToGroupMap(all_user_list);

    cout<<">>>Initialization finished!"<<endl;
}

/* Return the degree of separation betwen user 1 and user 2 */
vector<int> CPUParallel::getDOS(int user_id1, int user_id2) {
    vector<int> result_path;
    unordered_map<int, int>* user_to_group_map = all_result_ptr->getUserToGroupMap();

    int group_idx1 = (*user_to_group_map)[user_id1];
    int group_idx2 = (*user_to_group_map)[user_id2];

    if (group_idx1 != group_idx2 || group_idx1 == -1) {
        return result_path;
    }

    int root_user_id = (*all_result_ptr->getGroupToRootUserMap())[group_idx1];

    vector<int> user_id1_to_root = constructPath(root_user_id, user_id1);
    vector<int> user_id2_to_root = constructPath(root_user_id, user_id2);

    // Count shared parent nodes
    int idx1 = user_id1_to_root.size() - 1;
    int idx2 = user_id2_to_root.size() - 1;
    int shared_node_count = 0;

    while (idx1 >= 0 &&  idx2 >= 0 && user_id1_to_root[idx1] == user_id2_to_root[idx2]) {
        idx1--;
        idx2--;
        shared_node_count++;
    }

    // Merge two list with only one nearest common parent
    for (int i = 0; i < user_id1_to_root.size() - shared_node_count; i++) {
        result_path.push_back(user_id1_to_root[i]);
    }

    for (int i = user_id2_to_root.size() - shared_node_count; i >= 0 ; i--) {
        result_path.push_back(user_id2_to_root[i]);
    }

    return result_path;
}

/* Return a vector contain the path from child_user_id to root_user_id */
vector<int> CPUParallel::constructPath(int root_user_id, int child_user_id) {
    vector<int> result_path;
    ResultsPerUser* user_results_ptr = this->all_result_ptr->getResultsByUser(root_user_id);
    
    if (user_results_ptr == NULL) {
        return result_path;
    }

    vector<OneLevelInfo>* all_level_info_list = user_results_ptr->getAllLevelInfoList();
    int total_level_count = all_level_info_list->size();

    for (int i = 0; i < total_level_count; i++) {
        vector<UserTrace>* one_level_user_list = (*all_level_info_list)[i].getCurrentLevelUserList();
        int user_this_level_count = one_level_user_list->size();
        for (int j = 0; j < user_this_level_count; j++) {
            if ((*one_level_user_list)[j].user_id == child_user_id) {
                // Found solution. Reconstructe path backwards from child_user_id to root_user_id
                int current_level = i - 1;
                int previous_id = (*one_level_user_list)[j].previous_id;
                int current_id = child_user_id;
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
void CPUParallel::searchAll() {
    int user_count = all_user_list.size();

    vector<ResultsPerUser>* all_results = all_result_ptr->getAllResults();
    unordered_map<int, int> user_id_to_vector_idx_map;
    for (int i = 0; i < all_results->size(); i++) {
        int user_id = (*all_results)[i].getUserId();
        user_id_to_vector_idx_map[user_id] = i;
    }

    /* Search one by one for each user. Should go parallel here */
    int current_group_idx = 0;
    vector<int>::iterator it;
    for (it = all_user_list.begin(); it < all_user_list.end(); it++) {
        if (all_result_ptr->getUserToGroupMap()->find(*it)->second != -1) {
            // This user has already been checked, don't need to expand him/her
            continue;
        } else {
            current_group_idx++;
            all_result_ptr->getGroupToRootUserMap()->insert(pair<int, int>(current_group_idx, *it));
            int vector_idx = user_id_to_vector_idx_map[*it];
            (*all_results)[vector_idx].initFriendList(this->raw_data_ptr->getRawDataMap());
            all_result_ptr->getResultsByUser(*it)->searchAll(current_group_idx, all_result_ptr->getUserToGroupMap());
        }
    }

    cout<<">>>Total #Group := "<<current_group_idx<<endl;
}