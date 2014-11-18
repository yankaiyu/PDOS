//
//  main.cpp
//  Parallel Degree of Separation
//
//  Created by Cary on 11/16/14.
//  Copyright (c) 2014 Cary. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "data_utils.h"
using namespace std;

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        cout<<"Please enter the path of input data file"<<endl;
        return 0;
    }
    
    DataUtils* data_utils_ptr = DataUtils::getInstance();
    cout<<"Current num of nodes = "<<data_utils_ptr->getNumOfNodes()<<endl;
    
    ifstream data_file;
    data_file.open(argv[1]);
    if (data_file.is_open()) {
        cout<<"File opened"<<endl;
        int id1, id2;
        while (data_file.eof() == false) {
            data_file>>id1>>id2;
            cout<<id1<<" "<<id2<<endl;
            data_utils_ptr->insertEdge(id1, id2);
        }
    } else {
        cout<<"File did not open"<<endl;
    }

    data_file.close();
    
    cout<<"Current num of nodes = "<<data_utils_ptr->getNumOfNodes()<<endl;
    cout<<"Current num of edges = "<<data_utils_ptr->getNumOfEdges()<<endl;
    
    
    
    return 0;
}
