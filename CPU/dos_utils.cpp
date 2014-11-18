//
//  dos_utils.cpp
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#include "dos_utils.h"
#include <iostream> //!!!!!!!
using namespace std;

/*
 * Class OneLevelInfo
 */
OneLevelInfo::OneLevelInfo(int current_level) {
    this->current_level = current_level;
}

int OneLevelInfo::getCurrentLevel() {
    return current_level;
}

void OneLevelInfo::addUser(int user_id) {
    current_level_user_list.push_back(user_id);
}

vector<int> OneLevelInfo::getCurrentLevelUserList() {
    return current_level_user_list;
}

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

int ResultsPerUser::getUserId() {
    return user_id;
}

vector<OneLevelInfo> ResultsPerUser::getAllLevelInfoList() {
    return all_level_info_list;
}

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
    all_level_info_list[level].addUser(user_id);
    return;
}

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
        vector<int> new_user_list = one_level_info.getCurrentLevelUserList();
        int new_user_list_size = new_user_list.size();
        for (int i = 0; i < new_user_list_size; i++) {
            all_level_info_list[level].addUser(new_user_list[i]);
        }
    }
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

vector<ResultsPerUser> ResultsAllUsers::getAllResults() {
    return user_result_list;
}

ResultsPerUser* ResultsAllUsers::getOneUserResults(int user_id) {
    vector<ResultsPerUser>::iterator it;

    for (it = user_result_list.begin(); it != user_result_list.end(); it++) {
        if (it->getUserId() == user_id) {
            return &(*it);
        }
    }

    return NULL;
}

void ResultsAllUsers::addUserById(int user_id) {
    int user_list_size = user_result_list.size();

    for (int i = 0; i < user_list_size; i++) {
        if (user_result_list[i].getUserId() == user_id) {
            // User Id already stored in the list, return without inserting
            return;
        }
    }

    // User Id doesn't exist, add new entry
    //cout<<">>>Push back user_id: "<<user_id<<endl;
    user_result_list.push_back(ResultsPerUser(user_id));
    //cout<<">>>Pushed back user_id: "<<user_id<<endl;

    return;
}

void ResultsAllUsers::addUserByResults(ResultsPerUser results_per_user) {
    int user_id = results_per_user.getUserId();
    int user_list_size = user_result_list.size();

    for (int i = 0; i < user_list_size; i++) {
        if (user_result_list[i].getUserId() == user_id) {
            // Merge old and new results into the existing user's results
            vector<OneLevelInfo> new_info_list = results_per_user.getAllLevelInfoList();
            int new_list_size = new_info_list.size();

            for (int j = 0; j < new_list_size; j++) {
                user_result_list[i].addOneLevel(new_info_list[j]);
            }

            return;
        }
    }

    // User Id doesn't exist, add new entry
    user_result_list.push_back(ResultsPerUser(user_id));

    return;
}

ResultsAllUsers *ResultsAllUsers::instance = NULL;