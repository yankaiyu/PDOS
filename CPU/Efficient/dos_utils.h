//
//  dos_utils.h
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#ifndef __Parallel_Degree_of_Separation__dos_utils__
#define __Parallel_Degree_of_Separation__dos_utils__

#include <stdio.h>
#include <vector>
#include <set>
#include <unordered_map>
#include <omp.h>
using namespace std;

/*
 * Struct to record current step's user id and the parent user linking to this one
 * Used to reconstruct the path between two users
 */
class UserTrace {
public:
	int user_id;
	int previous_id;
	UserTrace(int user_id, int previous_id = -1);
};

/*
 * Store users that can be reached from one user with minimum distance == current_level
 */
class OneLevelInfo {
	int current_level;
	vector<UserTrace> current_level_user_list;
public:
	OneLevelInfo(int current_level);
	int getCurrentLevel();
	void addUser(UserTrace user);
	vector<UserTrace>* getCurrentLevelUserList();
	int getCurrentLevelUserNum();
};

/*
 * Store all users that can be reached from one user with distance <= current_level
 */
class ResultsPerUser {
	int user_id;
	vector<OneLevelInfo> all_level_info_list;
	unordered_map<int, vector<int> > raw_data_map;
public:
	ResultsPerUser(int user_id);
	void initFriendList(unordered_map<int, vector<int> > raw_data_map);
	int getUserId();
	vector<OneLevelInfo>* getAllLevelInfoList();
	void addUserAtLevel(int user_id, int level);
	void addOneLevel(OneLevelInfo one_level_info);
	void searchAll(int group_idx, unordered_map<int, int>* user_to_group_map, omp_lock_t* writelock);
};

/*
 * Store all users that can be reached from all users
 */
class ResultsAllUsers {
private:
	omp_lock_t writelock;

    ResultsAllUsers();
    static ResultsAllUsers* instance;
	vector<ResultsPerUser> user_result_list;
	unordered_map<int, int> user_to_group_map;
	unordered_map<int, int> group_to_root_user_map;
public:
	static ResultsAllUsers* getInstance();
	void initFriendList(unordered_map<int, vector<int> > raw_data_map);
	void initUserToGroupMap(vector<int> all_user_list);
	omp_lock_t* getWriteLock();
	vector<ResultsPerUser>* getAllResults();
	ResultsPerUser* getResultsByUser(int user_id);
	unordered_map<int, int>* getUserToGroupMap();
	unordered_map<int, int>* getGroupToRootUserMap();
	void resizeUserVector(int user_count);
	void addUserById(int user_id);
	void addUserByIdAt(int user_id, int idx);
	void addUserByResults(ResultsPerUser results_per_user);
};

#endif /* defined(__Parallel_Degree_of_Separation__dos_utils__) */
