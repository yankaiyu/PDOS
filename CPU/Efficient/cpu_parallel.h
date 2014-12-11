//
//  cpu_parallel.h
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#ifndef __Parallel_Degree_of_Separation__cpu_parallel__
#define __Parallel_Degree_of_Separation__cpu_parallel__

#include <stdio.h>
#include <vector>
#include "dos_utils.h"
#include "data_utils.h"
using namespace std;

/*
 * Interface for parallel verison of CPU based Degree of Separation
 */
class CPUParallel {
private:
	DataUtils* raw_data_ptr;
	ResultsAllUsers* all_result_ptr;
	vector<int> all_user_list;
	unordered_set<int> all_user_set;
public:
	CPUParallel(const char* filename);
	vector<int> getDOS(int user_id1, int user_id2);
	vector<int> constructPath(int root_user_id, int child_user_id);
	bool hasUser(int user_id);
	void searchAll();
};


#endif /* defined(__Parallel_Degree_of_Separation__cpu_parallel__) */
