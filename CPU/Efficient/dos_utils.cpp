//
//  dos_utils.cpp
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#include "dos_utils.h"
#include <omp.h>
//#include <iostream>
using namespace std;

/*
 * Class UserTrace
 */
UserTrace::UserTrace(int user_id, int previous_id) {
    this->user_id = user_id;
    this->previous_id = previous_id;
}

/*
 * Class OneLevelInfo
 */
OneLevelInfo::OneLevelInfo(int current_level) {
    this->current_level = current_level;
}

int OneLevelInfo::getCurrentLevel() {
    return current_level;
}

void OneLevelInfo::addUser(UserTrace user) {
    current_level_user_list.push_back(user);
}

/* Get the list of users that can be reached at this path level */
vector<UserTrace>* OneLevelInfo::getCurrentLevelUserList() {
    return &current_level_user_list;
}

/* Get the number of users that can be reached at this path level */
int OneLevelInfo::getCurrentLevelUserNum() {
    return current_level_user_list.size();
}

/*
 * Class ResultsPerUser
 */
ResultsPerUser::ResultsPerUser(int user_id) {
    this->user_id = user_id;
    addUserAtLevel(user_id, 0);
}

/* Init the friend lists for this user */
void ResultsPerUser::initFriendList(unordered_map<int, vector<int> > raw_data_map) {
    this->raw_data_map = raw_data_map;
}

/* Get this user's id */
int ResultsPerUser::getUserId() {
    return user_id;
}

/* Get users that can be reached by this user at all levels already searched */
vector<OneLevelInfo>* ResultsPerUser::getAllLevelInfoList() {
    return &all_level_info_list;
}

/* Add a new user that can be reached by this user to a level being explored now */
void ResultsPerUser::addUserAtLevel(int user_id, int level) {
    int list_size = all_level_info_list.size();
    if (list_size < level + 1) {
        // Has not searched to required level depth
        // Insert blank level info to required depth
        for (int i = list_size; i < level + 1; i++) {
            all_level_info_list.push_back(OneLevelInfo(i));
        }
    }

    // Insert new user into current level info
    all_level_info_list[level].addUser(UserTrace(user_id));
    return;
}

/* Add a OneLevel object containing all new users that can be reached by this user at one level */
void ResultsPerUser::addOneLevel(OneLevelInfo one_level_info) {
    int list_size = all_level_info_list.size();
    int level = one_level_info.getCurrentLevel();
    if (list_size < level + 1) {
        // Has not searched to required level depth
        // Insert blank level info to required depth - 1
        for (int i = list_size; i < level; i++) {
            all_level_info_list.push_back(OneLevelInfo(i));
        }
        all_level_info_list.push_back(one_level_info);
    } else {
        // Has searched to required level depth, insert by user id
        vector<UserTrace>* new_user_list = one_level_info.getCurrentLevelUserList();
        int new_user_list_size = new_user_list->size();
        for (int i = 0; i < new_user_list_size; i++) {
            all_level_info_list[level].addUser((*new_user_list)[i]);
        }
    }
}

/* Search for current user and deepen his/her dos info by one level */
void ResultsPerUser::searchAll(int group_idx, unordered_map<int, int>* user_to_group_map) {
    bool should_continue = true;

    while (should_continue) {
        int current_level = all_level_info_list.size();
        vector<UserTrace>* previous_level_users = all_level_info_list[current_level - 1].getCurrentLevelUserList();
        int previous_level_user_count = all_level_info_list[current_level - 1].getCurrentLevelUserNum();

        OneLevelInfo new_level(current_level);

        /* Search through new users found in previous deepest level to deepen path by one */
        #pragma parallel for
        for (int i = 0; i < previous_level_user_count; i++) {
            int user_id = (*previous_level_users)[i].user_id;
            vector<int> friend_list_of_user = raw_data_map.find(user_id)->second;
            int friends_count = friend_list_of_user.size();

            for (int j = 0; j < friends_count; j++) {
                unordered_map<int, int>::iterator it = user_to_group_map->find(friend_list_of_user[j]);
                if (it != user_to_group_map->end() && it->second == -1) {
                    // Find a path to a new user who has not been reached by this user before.
                    // Record it using UserTrace object and result vector
                    new_level.addUser(UserTrace(friend_list_of_user[j], user_id));
                    it->second = group_idx;
                }
            } 
        }

        if (new_level.getCurrentLevelUserNum() == 0) {
            // No more user can be obtained from this level on. Stop search.
            should_continue = false;;
        } else {
            // Add the OneLevel object containing all new users find at this level into this user's results
            addOneLevel(new_level);
        }
    }
    
    return;
}

/*
 * Class ResultsAllUsers
 */
 ResultsAllUsers::ResultsAllUsers() {}

 ResultsAllUsers* ResultsAllUsers::getInstance() {
    if (!instance) {
        instance = new ResultsAllUsers();
    }
    return instance;
}

/* Init the group each user belongs to */
void ResultsAllUsers::initUserToGroupMap(vector<int> all_user_list) {
    vector<int>::iterator it;
    for (it = all_user_list.begin(); it != all_user_list.end(); it++) {
        user_to_group_map.insert(pair<int, int>(*it, -1));
    }
}

/* Get the ResultsPerUser objects for all users in a vector */
vector<ResultsPerUser>* ResultsAllUsers::getAllResults() {
    return &user_result_list;
}

/* Get the ResultsPerUser object of specified user */
ResultsPerUser* ResultsAllUsers::getResultsByUser(int user_id) {
    vector<ResultsPerUser>::iterator it;

    for (it = user_result_list.begin(); it != user_result_list.end(); it++) {
        if (it->getUserId() == user_id) {
            return &(*it);
        }
    }

    return NULL;
}

/* Get the map from user_id to the group idx it belongs to */
unordered_map<int, int>* ResultsAllUsers::getUserToGroupMap() {
    return &user_to_group_map;
}

/* Get the map from group idx to the root user of this group */
unordered_map<int, int>* ResultsAllUsers::getGroupToRootUserMap() {
    return &group_to_root_user_map;
}

/* Resize the vector with the number of users */
void ResultsAllUsers::resizeUserVector(int user_count) {
    user_result_list.resize(user_count, ResultsPerUser(0));
}

/* Add an user object with specified ID */
void ResultsAllUsers::addUserById(int user_id) {
    int user_list_size = user_result_list.size();

    for (int i = 0; i < user_list_size; i++) {
        if (user_result_list[i].getUserId() == user_id) {
            // User Id already stored in the list, return without inserting
            return;
        }
    }

    // User Id doesn't exist, add new entry
    user_result_list.push_back(ResultsPerUser(user_id));

    return;
}

/* Add an user object with specified ID at assigned position */
void ResultsAllUsers::addUserByIdAt(int user_id, int idx) {
    int user_list_size = user_result_list.size();
    user_result_list[idx] = ResultsPerUser(user_id);
    return;
}

void ResultsAllUsers::addUserByResults(ResultsPerUser results_per_user) {
    int user_id = results_per_user.getUserId();
    int user_list_size = user_result_list.size();

    for (int i = 0; i < user_list_size; i++) {
        if (user_result_list[i].getUserId() == user_id) {
            // Merge old and new results into the existing user's results
            vector<OneLevelInfo>* new_info_list = results_per_user.getAllLevelInfoList();
            int new_list_size = new_info_list->size();

            for (int j = 0; j < new_list_size; j++) {
                user_result_list[i].addOneLevel((*new_info_list)[j]);
            }

            return;
        }
    }

    // User Id doesn't exist, add new entry
    user_result_list.push_back(results_per_user);

    return;
}

ResultsAllUsers *ResultsAllUsers::instance = NULL;