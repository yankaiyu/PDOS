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
#include <set>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <cuda_runtime.h>
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>

__global__ void search_first_level(int *input_user1, int *input_user2, int *level_content, int *parent_content, int *friend_list, int *num_node, int *num_edge, int *num_friend, int *bound)
{
  int thid = threadIdx.x;
  int start_pos = blockIdx.x*blockDim.x;
  int j = 0;
  if(thid+start_pos < *num_node)
  {
    
    for(int i = 0; i < *num_edge; i++)
    {
      if(input_user1[i] == (thid+start_pos))
      {
        friend_list[(thid+start_pos)*(*num_node)+input_user2[i]] = input_user2[i];
        level_content[(thid+start_pos)*(*num_node)+j] = input_user2[i];
        j++;
      }
      if(input_user2[i] == (thid+start_pos))
      {
        friend_list[(thid+start_pos)*(*num_node)+input_user1[i]] = input_user1[i];
        level_content[(thid+start_pos)*(*num_node)+j] = input_user1[i];
        j++;
      }
    }
    num_friend[(thid+start_pos)] = j;
    bound[(thid+start_pos)] = j;
  }
}

__global__ void search_other_level(int *input_user1, int *input_user2, int *level_content, int *parent_content, int *friend_list, int *num_node, int *num_friend, int *in_bound_1, int *in_bound_2)
{
  int thid = threadIdx.x;
  int start_pos = blockIdx.x*blockDim.x;
  if(thid+start_pos < *num_node)
  {
    int l_bound = in_bound_1[thid+start_pos];
    int u_bound = in_bound_2[thid+start_pos];
    in_bound_1[thid+start_pos] = u_bound;
    for(int k = l_bound; k < u_bound; k++)
    {
      int friend_t = level_content[(thid+start_pos)*(*num_node)+k];  
      if(friend_t == -1)
      {
        break;
      }
      else
      {
        for(int n = 0; n < num_friend[friend_t]; n++)
        {
          int temp = level_content[friend_t*(*num_node)+n];
          if((thid+start_pos)*(*num_node)+temp > (*num_node)*(*num_node))
            printf("friend_t: %d num_friend: %d\n", friend_t,num_friend[friend_t]);
          if(friend_list[(thid+start_pos)*(*num_node)+temp] == -1)
          {
            friend_list[(thid+start_pos)*(*num_node)+temp] = temp;
            level_content[(thid+start_pos)*(*num_node)+in_bound_2[thid+start_pos]] = temp;
            parent_content[(thid+start_pos)*(*num_node)+in_bound_2[thid+start_pos]] =(thid+start_pos)*(*num_node)+k;
            in_bound_2[thid+start_pos]++;
          }   
        } 
      }               
    }
  }
}

__global__ void find(int *user1, int *user2, int *friend_list, int *level_content, int *parent_content, int *num_node, int *output, int *outsize, int *found)
{

  int thid = threadIdx.x;
  int start_pos = blockIdx.x*blockDim.x;
  int parent_index = 0;
  *outsize = 0;
  if(thid == 0 && start_pos == 0)
  {
    *found = 0;
  }
  if(thid+start_pos < *num_node)
  {
    if(level_content[(*user1)*(*num_node)+(thid+start_pos)] == *user2)
    {
      *found = 1;
      parent_index = parent_content[(*user1)*(*num_node)+(thid+start_pos)];
      output[*outsize] = level_content[(*user1)*(*num_node)+(thid+start_pos)];
      (*outsize)++;
      while(parent_index != -1)
      {
        output[*outsize] = level_content[parent_index];
        parent_index = parent_content[parent_index];
        (*outsize)++;
      }
      // printf("Parent index: %d\n, Parent: %d thread: %d\n",parent_index, level_content[parent_index], thid+start_pos);
    }
  }
}

int main(int argc, const char * argv[]) {
    if (argc < 2) {
        std::cout<<"Please enter the path of input data file\n";
        return 0;
    }

    struct timeval starttime,endtime;

    thrust::device_vector<int> user1;
    thrust::device_vector<int> user2;
    std::set<int> unique_user;

    gettimeofday(&starttime,NULL);
    
    //reading file
    std::ifstream data_file;
    data_file.open(argv[1]);
    if (data_file.is_open()) {
        //cout<<"File opened"<<endl;
        int id1, id2;
        while (data_file.eof() == false) {
            data_file>>id1>>id2;
	    user1.push_back(id1);
            user2.push_back(id2);
            unique_user.insert(id1);
            unique_user.insert(id2);
        }
    } else {
        std::cout<<"File did not open\n";
        exit(0);
    }
    data_file.close();
    
    int num_node = unique_user.size();
    int num_edge = user1.size();
    std::cout<<"#Node := "<<num_node<<"\n";
    std::cout<<"#Edge := "<<num_edge<<"\n";
 
    gettimeofday(&endtime,NULL);
    long long time = ((endtime.tv_sec * 1000000 + endtime.tv_usec) - (starttime.tv_sec * 1000000 + starttime.tv_usec));
    printf(">>>Initializaton takes := %lld microseconds \n\n", time);

    int search_depth;
    std::cout<<"#Level you want to search: ";
    std::cin>>search_depth;

    int num_threads = num_node;
    int num_blocks = ceil(num_threads/512.0);

    std::cout<<"#threads: "<<num_threads<<" #Blocks: "<<num_blocks<<"\n";

    dim3 dimGrid(num_blocks);
    dim3 dimBlock(512);

    float para_time;
    cudaEvent_t start,stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop); 
    cudaEventRecord(start);

    int *device_user1 = thrust::raw_pointer_cast(&user1[0]);
    int *device_user2 = thrust::raw_pointer_cast(&user2[0]);
    int *device_level_content;
    int *device_parent_content;
    int *device_num_friend;
    int *friend_list; 
    int *device_num_node;
    int *device_num_edge;
    int *inbound_1;
    int *inbound_2;

    cudaError_t error; 

    cudaMalloc((int**)&device_level_content,num_node*num_node*sizeof(int));
    cudaMalloc((int**)&device_parent_content,num_node*num_node*sizeof(int));
    cudaMalloc((int**)&friend_list,num_node*num_node*sizeof(int));
    cudaMalloc((int**)&device_num_friend,num_node*sizeof(int));
    cudaMalloc((int**)&device_num_node,sizeof(int));
    cudaMalloc((int**)&device_num_edge,sizeof(int));
    cudaMalloc((int**)&inbound_1,num_node*sizeof(int));
    cudaMalloc((int**)&inbound_2,num_node*sizeof(int));

    error = cudaGetLastError();
    if(error != cudaSuccess)
    {
      printf("CUDA first malloc error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }

    cudaMemset((int**)device_level_content,-1,num_node*num_node);
    cudaMemset((int**)device_parent_content,-1,num_node*num_node);
    cudaMemset((int**)friend_list,-1,num_node*num_node);
    cudaMemset((int**)inbound_1,0,num_node);
    cudaMemcpy(device_num_node, &num_node, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(device_num_edge, &num_edge, sizeof(int), cudaMemcpyHostToDevice);

    error = cudaGetLastError();    
    if(error != cudaSuccess)
    {
      printf("CUDA first memset memcpy error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }

    search_first_level<<<dimGrid,dimBlock>>>(device_user1, device_user2, device_level_content, device_parent_content, friend_list, device_num_node, device_num_edge, device_num_friend,inbound_2);

    for(int i = 1; i < search_depth; i++)
    {      
      search_other_level<<<dimGrid,dimBlock>>>(device_user1, device_user2, device_level_content, device_parent_content, friend_list, device_num_node, device_num_friend, inbound_1, inbound_2);
    } 
    error = cudaGetLastError();
    if(error != cudaSuccess)
    {
      printf("CUDA search_level error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }

    cudaFree(device_num_edge);
    cudaFree(device_num_friend);
    cudaFree(inbound_1);
    cudaFree(inbound_2);

    error = cudaGetLastError();
    if(error != cudaSuccess)
    {
      printf("CUDA first free error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }

    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&para_time,start,stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    printf("\nParallel Search Time:%f microseconds",para_time);


    bool should_continue = true;
    while (should_continue) {
      int user_id1, user_id2;
      std::set<int>::iterator it;
      do
      {
        std::cout<<"\n";
        std::cout<<"Enter first user's ID: ";
        std::cin >> user_id1;
        it = unique_user.find(user_id1);
        if(it == unique_user.end())
          std::cout << "User Not Exist\n";
      }
      while(it == unique_user.end());
     
      do
      {
        std::cout<<"\n";
        std::cout<<"Enter second user's ID: ";
        std::cin >> user_id2;
        it = unique_user.find(user_id2);
        if(it == unique_user.end())
          std::cout << "User Not Exist\n";
      }
      while(it == unique_user.end());

      int* device_input1;
      int* device_input2;
      int* device_output;
      int* device_size;
      int* found;

     
      cudaMalloc((int**)&device_input1,sizeof(int));
      cudaMalloc((int**)&device_input2,sizeof(int));
      cudaMalloc((int**)&device_size,sizeof(int));
      cudaMalloc((int**)&device_output,search_depth*sizeof(int));
      cudaMalloc((int**)&found,sizeof(int));
      error = cudaGetLastError();
      if(error != cudaSuccess)
      {
        printf("CUDA second malloc error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      cudaMemset((int**)device_output,0,search_depth);
      cudaMemset((int**)device_size,0,1);
      cudaMemcpy(device_input1, &user_id1, sizeof(int), cudaMemcpyHostToDevice);
      error = cudaGetLastError();
      if(error != cudaSuccess)
      {
        printf("CUDA second memset memcpy error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      cudaMemcpy(device_input2, &user_id2, sizeof(int), cudaMemcpyHostToDevice);



      find<<<dimGrid, dimBlock>>>(device_input1, device_input2, friend_list,device_level_content, device_parent_content, device_num_node, device_output, device_size,found);
      error = cudaGetLastError();
      if(error != cudaSuccess)
      {
        printf("CUDA find error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      int* found_host = new int;
      int* size = new int;
      cudaMemcpy(found_host, found, sizeof(int), cudaMemcpyDeviceToHost);
      cudaMemcpy(size, device_size, sizeof(int), cudaMemcpyDeviceToHost);

      int* result = new int[*size];
      cudaMemcpy(result, device_output, (*size)*sizeof(int), cudaMemcpyDeviceToHost);

      cudaFree(device_input1);
      cudaFree(device_input2);
      cudaFree(device_size);
      cudaFree(device_output);
      cudaFree(found);

      if(*found_host == 0)
        std::cout<<"\nUser "<<user_id1<<" and User "<<user_id2<<" are not connected within search level := "<<search_depth<<"\n\n";
      else
      {
        std::cout<<"\nDegree of Separation between User "<<user_id1<<" and User "<<user_id2<<" is "<<(*size)<<"\n\n";
        for (int i = 0; i < (*size); i++) {
          std::cout<<result[i]<<"<-";
        }      
        std::cout<<user_id1<<"\n";
      }

      std::cout<<"\nDo you want to continue? 1 for yes and 0 for no: ";
      std::cin>>should_continue;
    }

   
    cudaFree(device_level_content);
    cudaFree(device_parent_content);
    cudaFree(friend_list);
    cudaFree(device_num_node);


    return 0;
}
