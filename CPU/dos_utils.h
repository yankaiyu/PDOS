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
using namespace std;

/*
 * Store users that can be reached from one user with minimum distance == current_level
 */
class OneLevelInfo {
	int current_level;
	vector<int> current_level_user_list;
public:
	OneLevelInfo(int current_level);
	int getCurrentLevel();
	void addUser(int user_id);
	vector<int>& getCurrentLevelUserList();
	int getCurrentLevelUserNum();
};

/*
 * Store all users that can be reached from one user with distance <= current_level
 */
class ResultsPerUser {
	int user_id;
	vector<OneLevelInfo> all_level_info_list;
public:
	ResultsPerUser(int user_id);
	int getUserId();
	void addUserAtLevel(int user_id, int level);
	void addOneLevel(OneLevelInfo one_level_info);
};

#endif /* defined(__Parallel_Degree_of_Separation__dos_utils__) */
