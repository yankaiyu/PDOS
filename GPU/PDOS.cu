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
#include <map>
#include <vector>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>
#include <cuda_runtime.h>
#include <thrust/host_vector.h>
#include <thrust/scan.h>
#include <thrust/device_vector.h>

#define limit 8000*8000
__global__ void relation(int *users, int *input_user1, int *input_user2,int *num_edge, int *num_node, int *base)
{
  int thid = threadIdx.x;
  int start_pos = blockIdx.x*blockDim.x;
  int user = users[thid+start_pos+*base];
  if(thid+start_pos+*base < *num_node)
  {
    for(int i = 0; i < *num_edge; i++)
    {
      if(input_user1[i] == user)
        input_user1[i] = thid+start_pos+*base;
      if(input_user2[i] == user)
        input_user2[i] = thid+start_pos+*base;   
     }
  }
}
__global__ void search_first_level(int *users, int *input_user1, int *input_user2, int *level_content, int *parent_content, int *friend_list, int *num_node, int *num_edge, int *num_friend, int *bound, int *offset, int *first_level, int *base)
{
  int thid = threadIdx.x;
  int start_pos = blockIdx.x*blockDim.x;
  int j = 0;
  if(thid+start_pos+*base < *num_node)
  {
    for(int i = 0; i < *num_edge; i++)
    {
      if(input_user1[i] == thid+start_pos+*base)
      {
//if(user == 5)
//{  printf("Find User 30!!!!!!! His Friend: %d\n ",input_user2[i]);
//}
        friend_list[(thid+start_pos)*(*num_node)+input_user2[i]] = input_user2[i];
        level_content[(thid+start_pos)*(*num_node)+j] = input_user2[i];
        first_level[offset[thid+start_pos+*base]+j] = input_user2[i];
        j++;
      }
      if(input_user2[i] == thid+start_pos+*base)
      {
//if(user == 5)
//{  printf("Find User 30!!!!!!! His Friend: %d\n ",input_user1[i]);
//}
 
        friend_list[(thid+start_pos)*(*num_node)+input_user1[i]] = input_user1[i];
        level_content[(thid+start_pos)*(*num_node)+j] = input_user1[i];
        first_level[offset[thid+start_pos+*base]+j] = input_user1[i];
        j++;
      }
    }
    num_friend[(thid+start_pos+*base)] = j;
    bound[(thid+start_pos+*base)] = j;
  }
}

__global__ void search_other_level(int *input_user1, int *input_user2, int *level_content, int *parent_content, int *friend_list, int *num_node, int *num_friend, int *in_bound_1, int *in_bound_2,int *offset, int *first_level, int *base, int *count)
{
  int thid = threadIdx.x;
  int start_pos = blockIdx.x*blockDim.x+*base;
  if(thid+start_pos-*base < *count)
  {
    int l_bound = in_bound_1[thid+start_pos];
    int u_bound = in_bound_2[thid+start_pos];
    in_bound_1[thid+start_pos] = u_bound;

    for(int k = l_bound; k < u_bound; k++)
    {
      int friend_t = level_content[(thid+start_pos-*base)*(*num_node)+k];  
      if(friend_t == -1)
      {
        break;
      }
      else
      {
        for(int n = 0; n < num_friend[friend_t]; n++)
        {
          int temp = first_level[offset[friend_t]+n];
          if((thid+start_pos-*base)*(*num_node)+temp > (*count)*(*num_node))
            printf("AAAAAAA %d %d %d %d %d\n",temp,(thid+start_pos-*base)*(*num_node)+temp,(*count)*(*num_node), thid+start_pos-*base,(*count) );
          if(friend_list[(thid+start_pos-*base)*(*num_node)+temp] == -1 && temp != thid+start_pos)
          {
            friend_list[(thid+start_pos-*base)*(*num_node)+temp] = temp;
            level_content[(thid+start_pos-*base)*(*num_node)+in_bound_2[thid+start_pos]] = temp;
            parent_content[(thid+start_pos-*base)*(*num_node)+in_bound_2[thid+start_pos]] = k;

            in_bound_2[thid+start_pos]++;
          }   
        } 
      }               
    }
  }
}

__global__ void find(int *user2,int *friend_list, int *level_content, int *parent_content, int *num_node, int *output, int *outsize, int *found, int *base)
{

  int thid = threadIdx.x;
  int start_pos = blockIdx.x*blockDim.x+*base;
  int parent_index = 0;
  if(thid+start_pos < *num_node)
  {

    if(level_content[(thid+start_pos)] == *user2  && friend_list[*user2] != -1 )
    {

      *found = 1;
      parent_index = parent_content[(thid+start_pos)];
      output[*outsize] = level_content[(thid+start_pos)];
      (*outsize)++;
      while(parent_index != -1)
      {
        output[*outsize] = level_content[parent_index];
        parent_index = parent_content[parent_index];
        (*outsize)++;
      }
    // printf("OUTSIZE %d\n", *outsize);
    }
  }
}

std::map<int, std::vector<int> > unique_user;
void addEdge(int user_id1, int user_id2) {
    std::map<int, std::vector<int> >::iterator it;    
    // Insert edge into user_id1's firend list
    it = unique_user.find(user_id1);
    if (it == unique_user.end()) {
        std::vector<int> friend_list(1, user_id2);
        unique_user.insert(std::pair<int, std::vector<int> >(user_id1, friend_list));
    } else {
        it->second.push_back(user_id2);
    }

    // Insert edge into id2's firend list
    it = unique_user.find(user_id2);
    if (it == unique_user.end()) {
        std::vector<int> friend_list(1, user_id1);
        unique_user.insert(std::pair<int, std::vector<int> >(user_id2, friend_list));
    } else {
        it->second.push_back(user_id1);
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
        addEdge(id1,id2);
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

    int *offset = new int [num_node];
    int *users = new int[num_node];

    std::map<int, std::vector<int> >::iterator it;    
    int ttt = 0;
    for(it = unique_user.begin(); it != unique_user.end(); ++it)
    {
      users[ttt] = it->first;
      offset[ttt] = it->second.size();
      ttt++;
    }
    if(ttt != num_node)
      std::cout<<"ERROR!!!!! \n";
    int add = offset[num_node-1];
    thrust::exclusive_scan(offset, offset + num_node , offset); // in-place scan

    gettimeofday(&endtime,NULL);
    long long time = ((endtime.tv_sec * 1000000 + endtime.tv_usec) - (starttime.tv_sec * 1000000 + starttime.tv_usec));
    printf(">>>Initializaton takes := %lld microseconds \n\n", time);

    int search_depth;
    std::cout<<"#Level you want to search: ";
    std::cin>>search_depth;

    int num_threads = limit/num_node;
    int block_size = num_threads > 512 ? 512 : num_threads;
    int num_blocks = ceil(num_threads/block_size);

    std::cout<<"#threads: "<<num_threads<<" #Blocks: "<<num_blocks<<"\n";

    dim3 dimGrid(num_blocks);
    dim3 dimBlock(block_size);

    float para_time;
    cudaEvent_t start,stop;
    cudaEventCreate(&start);
    cudaEventCreate(&stop); 
    cudaEventRecord(start);
    long long num_num = (long long)num_node*num_node;
    int *host_level_content = new int [num_num];
    int *host_parent_content = new int [num_num];
    int *host_friend_list = new int [num_num];


    int *device_user1 = thrust::raw_pointer_cast(&user1[0]);
    int *device_user2 = thrust::raw_pointer_cast(&user2[0]);
    int *device_offset;
    int *device_users;
    int *device_level_content;
    int *device_parent_content;
    int *device_num_friend;
    int *friend_list; 
    int *device_num_node;
    int *device_num_edge;
    int *inbound_1;
    int *inbound_2;
    int *device_first_level;
    int *device_base;
    int *device_count;

    cudaError_t error; 

    cudaMalloc((int**)&device_num_friend,num_node*sizeof(int));
    cudaMalloc((int**)&device_offset,num_node*sizeof(int));
    cudaMalloc((int**)&device_users,num_node*sizeof(int));
    cudaMalloc((int**)&device_first_level,(offset[num_node-1]+add)*sizeof(int));
    cudaMalloc((int**)&device_num_node,sizeof(int));
    cudaMalloc((int**)&device_num_edge,sizeof(int));
    cudaMalloc((int**)&inbound_1,num_node*sizeof(int));
    cudaMalloc((int**)&inbound_2,num_node*sizeof(int));
    cudaMalloc((int**)&device_base,sizeof(int));
    cudaMalloc((int**)&device_count,sizeof(int));

    cudaMemset((int**)inbound_1,0,num_node*4);
    cudaMemcpy(device_num_node, &num_node, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(device_num_edge, &num_edge, sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(device_offset, offset, num_node*sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(device_users, users, num_node*sizeof(int), cudaMemcpyHostToDevice);
    error = cudaGetLastError();
    if(error != cudaSuccess)
    {
      printf("CUDA first error: %s\n", cudaGetErrorString(error));
      exit(-1);
    }
    long long count = 0;
    long long base = 0;    
    count = limit/num_node; 
    if(count > num_node)
      count = num_node;
    do
    {    
      cudaMemcpy(device_base, &base, sizeof(int), cudaMemcpyHostToDevice);
      relation<<<dimGrid,dimBlock>>>(device_users, device_user1, device_user2, device_num_edge, device_num_node, device_base);
      base = base + count;
      if(base+count > num_node)
        count = num_node - base;
    }while(base < num_node);


    base = 0;   
    count = limit/num_node; 
    if(count > num_node)
      count = num_node;
    do{
      cudaMalloc((int**)&device_level_content,count*num_node*sizeof(int));
      cudaMalloc((int**)&device_parent_content,count*num_node*sizeof(int));
      cudaMalloc((int**)&friend_list,count*num_node*sizeof(int));

      error = cudaGetLastError();    
      if(error != cudaSuccess)
      {
        printf("CUDA second error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      cudaMemset((int**)device_level_content,-1,count*num_node*4);
      cudaMemset((int**)device_parent_content,-1,count*num_node*4);
      cudaMemset((int**)friend_list,-1,count*num_node*4);
      cudaMemcpy(device_base, &base, sizeof(int), cudaMemcpyHostToDevice);
      cudaMemcpy(device_count, &count, sizeof(int), cudaMemcpyHostToDevice);
      error = cudaGetLastError();    
      if(error != cudaSuccess)
      {
        printf("CUDA third error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      search_first_level<<<dimGrid,dimBlock>>>(device_users,device_user1, device_user2, device_level_content, device_parent_content, friend_list, device_num_node, device_num_edge, device_num_friend,inbound_2, device_offset, device_first_level,device_base);

      for(int i = 1; i < search_depth; i++)
      {      
        search_other_level<<<dimGrid,dimBlock>>>(device_user1, device_user2, device_level_content, device_parent_content, friend_list, device_num_node, device_num_friend, inbound_1, inbound_2,device_offset, device_first_level,device_base,device_count);
      } 
      error = cudaGetLastError();
      if(error != cudaSuccess)
      {
        printf("CUDA search_level error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      cudaMemcpy((host_level_content+base*num_node), device_level_content, count*num_node*4, cudaMemcpyDeviceToHost);
      cudaMemcpy((host_parent_content+base*num_node), device_parent_content, count*num_node*4, cudaMemcpyDeviceToHost);
    //  printf("BASE: %d Count %d Node %d\n",base, count, num_node);
    //  if(base*num_node + count*num_node > num_node*num_node)
    //    printf("OUT OF BOUND!!!!!!!  %d %d %d %d\n",base*num_node,count*num_node,base*num_node + count*num_node,num_node*num_node);
      cudaMemcpy((host_friend_list+base*num_node), friend_list, count*num_node*4, cudaMemcpyDeviceToHost);
      cudaFree(device_level_content);
      cudaFree(device_parent_content);
      cudaFree(friend_list);
      base = base + count;
      if(base+count > num_node)
        count = num_node - base;

    }while(base < num_node);

    cudaFree(device_num_edge);
    cudaFree(device_num_friend);
    cudaFree(inbound_1);
    cudaFree(inbound_2);
    cudaFree(device_base);
    cudaFree(device_count);
    cudaFree(device_offset);
    cudaFree(device_users);

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
      std::map<int, std::vector<int> >::iterator it;
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
    
      int base_base;
      int base_base_1;
      int base_found = 0;
      int base_found_1 = 0;
      for(int i = 0; i < num_node; i++)
      {
        if(users[i] == user_id1)
        {
          base_base = i;
          base_found = 1;
        }
        if(users[i] == user_id2)
        {
          base_base_1 = i;
          base_found_1 = 1;
        }
        if(base_found == 1 && base_found_1 == 1)
          break;
      }
     // printf("BASEBASE %d %d\n", base_base, users[base_base]);
      int* device_input2;
      int* device_output;
      int* device_size;
      int* found;
      int *found_host = new int;
      int *size = new int;
      int *result = new int[*size];

      cudaMalloc((int**)&device_level_content,num_node*sizeof(int));
      cudaMalloc((int**)&device_parent_content,num_node*sizeof(int));
      cudaMalloc((int**)&friend_list,num_node*sizeof(int));
      cudaMalloc((int**)&device_input2,sizeof(int));
      cudaMalloc((int**)&device_size,sizeof(int));
      cudaMalloc((int**)&device_output,search_depth*sizeof(int));
      cudaMalloc((int**)&found,sizeof(int));
      cudaMalloc((int**)&device_base,sizeof(int));
      error = cudaGetLastError();
      if(error != cudaSuccess)
      {
        printf("CUDA second malloc error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      cudaMemset((int**)device_output,0,search_depth*4);
      cudaMemset((int**)device_size,0,4);
      cudaMemset((int**)found,0,4);
      cudaMemcpy(device_input2, &base_base_1, sizeof(int), cudaMemcpyHostToDevice);
      cudaMemcpy(device_level_content, host_level_content+base_base*num_node, num_node*sizeof(int), cudaMemcpyHostToDevice);
      cudaMemcpy(device_parent_content, host_parent_content+base_base*num_node, num_node*sizeof(int), cudaMemcpyHostToDevice);
      cudaMemcpy(friend_list, host_friend_list+base_base*num_node, num_node*sizeof(int), cudaMemcpyHostToDevice);
      error = cudaGetLastError();
      if(error != cudaSuccess)
      {
        printf("CUDA second memset memcpy error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      base = 0;
      count = limit/num_node; 
      if(count > num_node)
        count = num_node;
      do{
        
        cudaMemcpy(device_base, &base, sizeof(int), cudaMemcpyHostToDevice);
        find<<<dimGrid, dimBlock>>>(device_input2, friend_list,device_level_content, device_parent_content, device_num_node, device_output, device_size,found,device_base);
        cudaMemcpy(found_host, found, sizeof(int), cudaMemcpyDeviceToHost);

        base = base + count;
        if(base+count > num_node)
          count = num_node - base;
      }while(base < num_node && !(*found_host));
      
      error = cudaGetLastError();
      if(error != cudaSuccess)
      {
        printf("CUDA find error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }


      cudaMemcpy(size, device_size, sizeof(int), cudaMemcpyDeviceToHost);
      cudaMemcpy(result, device_output, (*size)*sizeof(int), cudaMemcpyDeviceToHost);

      //printf("SIZE %d\n",*size);

      error = cudaGetLastError();
      if(error != cudaSuccess)
      {
        printf("CUDA find error: %s\n", cudaGetErrorString(error));
        exit(-1);
      }

      cudaFree(device_input2);
      cudaFree(device_size);
      cudaFree(device_output);
      cudaFree(found);
      cudaFree(device_level_content);
      cudaFree(device_parent_content);
      cudaFree(device_base);

      if(*found_host == 0)
        std::cout<<"\nUser "<<user_id1<<" and User "<<user_id2<<" are not connected within search level := "<<search_depth<<"\n\n";
      else
      {
        std::cout<<"\nDegree of Separation between User "<<user_id1<<" and User "<<user_id2<<" is "<<(*size)<<"\n\n";
        for (int i = 0; i < (*size); i++) {
          std::cout<<users[result[i]]<<"<-";
        }      
        std::cout<<user_id1<<"\n";
      }

      std::cout<<"\nDo you want to continue? 1 for yes and 0 for no: ";
      std::cin>>should_continue;
      delete[] result;
      delete size;
      delete found_host;
    }

   delete[] offset;
   delete[] users;
   delete[] host_level_content;
   delete[] host_parent_content;
   delete[] host_friend_list;


    return 0;
}
