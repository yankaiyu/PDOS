//
//  main.cpp
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "cpu_parallel.h"
#include "data_utils.h"
#include "dos_utils.h"
using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        cout<<"Please enter the path of input data file"<<endl;
        return 0;
    }
    
    CPUParallel pdos_cpu(argv[1]);

    int user_id1, user_id2;
    cout<<"Enter first user's ID: ";
    cin >> user_id1;
    cout<<"Enter second user's ID: ";
    cin >> user_id2;

    int dos = pdos_cpu.getDOS(user_id1, user_id2);

    cout<<"Degree of Separation between User "<<user_id1<<" and User "<<user_id2<<" is "<<dos<<endl;
    
    return 0;
}
