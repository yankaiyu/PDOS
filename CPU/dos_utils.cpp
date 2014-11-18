//
//  data_utils.cpp
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#include "dos_utils.h"

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

vector<int>& OneLevelInfo::getCurrentLevelUserList() {
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

void ResultsPerUser::addUserAtLevel(int user_id, int level) {
    int list_size = all_level_info_list.size();
    if (list_size < level + 1) {
        for (int i = list_size; i < level; i++) {
            all_level_info_list.push_back(OneLevelInfo(i));
        }
    }
    all_level_info_list[level].addUser(user_id);
    return;
}

void ResultsPerUser::addOneLevel(OneLevelInfo one_level_info) {
    int list_size = all_level_info_list.size();
    int level = one_level_info.getCurrentLevel();
    if (list_size < level + 1) {
        for (int i = list_size; i < level - 1; i++) {
            all_level_info_list.push_back(OneLevelInfo(i));
        }
        all_level_info_list.push_back(one_level_info);
    } else {
        vector<int> new_user_list = one_level_info.getCurrentLevelUserList();
        int new_user_list_size = new_user_list.size();
        for (int i = 0; i < new_user_list_size; i++) {
            all_level_info_list[level].addUser(new_user_list[i]);
        }
    }
}