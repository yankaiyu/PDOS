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
 * Store users that can be reached from one user with minimum distance == current_level
 */
class CPUParallel {
private:
	DataUtils* raw_data_ptr;
	ResultsAllUsers* all_result_ptr;
	vector<int> all_user_list;
public:
	CPUParallel(const char* filename);
	vector<int> getDOS(int user_id1, int user_id2);
	void deepenOneLevel();
};


#endif /* defined(__Parallel_Degree_of_Separation__cpu_parallel__) */
