//
//  main.cpp
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <omp.h>
#include "cpu_parallel.h"
#include "data_utils.h"
#include "dos_utils.h"
using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        cout<<"Please enter the path of input data file"<<endl;
        return 0;
    }

    int num_of_threads = 1;
    if (argc >= 3) {
        num_of_threads = atoi(argv[2]);  
    }
    omp_set_num_threads(num_of_threads);

    double t_begin = omp_get_wtime();
    CPUParallel pdos_cpu(argv[1]);
    double t_end = omp_get_wtime();
    printf(">>>Initializaton takes := %fs with #Core := %d\n\n", t_end - t_begin, num_of_threads);

    int search_depth;
    cout<<"#Level you want to search: ";
    cin>>search_depth;

    t_begin = omp_get_wtime();

    for (int i = 1; i <= search_depth; i++) {
        cout<<">>>Current level := "<<i<<endl;
        double t_one_loop_begin = omp_get_wtime();
        pdos_cpu.deepenOneLevel();
        double t_one_loop_end = omp_get_wtime();
        printf(">>>Time used := %fs at this level\n", t_one_loop_end - t_one_loop_begin);
    }

    cout<<endl<<">>>Search finished!\n";

    t_end = omp_get_wtime();
    printf(">>>Time used := %fs with #Core := %d\n\n", t_end - t_begin, num_of_threads);

    bool should_continue = true;
    while (should_continue) {
        int user_id1, user_id2;
        cout<<endl;
        cout<<"Enter first user's ID: ";
        cin >> user_id1;
        cout<<"Enter second user's ID: ";
        cin >> user_id2;

        vector<int> dos = pdos_cpu.getDOS(user_id1, user_id2);

        if (dos.size() == 0) {
            cout<<"\nUser "<<user_id1<<" and User "<<user_id2<<" are not connected within search level := "<<search_depth<<endl<<endl;
        } else {
            cout<<"\nDegree of Separation between User "<<user_id1<<" and User "<<user_id2<<" is "<<dos.size() - 1<<endl;
            for (int i = 0; i < dos.size() - 1; i++) {
                cout<<dos[i]<<"<-";
            }
            cout<<dos[dos.size() - 1]<<endl;
        }
        
        cout<<"\nDo you want to continue? 1 for yes and 0 for no: ";
        cin>>should_continue;
    }

    cout<<endl;
    
    return 0;
}
