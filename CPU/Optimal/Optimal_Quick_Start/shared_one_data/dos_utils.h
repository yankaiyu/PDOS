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
#include <unordered_set>
#include <unordered_map>
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
	//bool operator<(UserTrace) const;
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
	unordered_set<int> reached_user_set;
	int total_user_count;
public:
	ResultsPerUser(int user_id, int total_user_count);
	int getUserId();
	vector<OneLevelInfo>* getAllLevelInfoList();
	void addUserAtLevel(int user_id, int level);
	void addOneLevel(OneLevelInfo one_level_info);
	void deepenOneLevel();
};

/*
 * Store all users that can be reached from all users
 */
class ResultsAllUsers {
private:
    ResultsAllUsers();
    static ResultsAllUsers* instance;
	vector<ResultsPerUser> user_result_list;
public:
	static ResultsAllUsers* getInstance();
	vector<ResultsPerUser>* getAllResults();
	ResultsPerUser* getResultsByUser(int user_id);
	void addUserById(int user_id, int total_user_count);
	void addUserByResults(ResultsPerUser results_per_user);
};

#endif /* defined(__Parallel_Degree_of_Separation__dos_utils__) */
