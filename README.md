PDOS (Parallel Degree of Separation)
=============================
PDOS is short for Parallel Degree of Separation. It is a parallel algorithm designed and implemented by Kaiyu Yan and Xiaoyu Zheng from University of Michigan as a course project for calculating user pairs' degree of separation in a social network. It contains both CPU verison and GPU verison.

How to run
=============================
CPU Verision:

There are two versions under CPU folder, "Optimal" for optimal solution, "Efficient" for time/space efficient but non-optimal solution. For either version, enter its folder and use "make" in command line to complie with g++. Use "make opt" to complie with -O3 flag. You can also use "make clean" to delete the executable file. Note: The CPU version makes use of OpenMP functions, so please make sure the g++ complier you use supports OpenMp features.

The default data is stored in data.txt. To run using this data, just use command "make run". To use your own data, either replace the content of data.txt or use "./pdos.bin $DATA_FILE_PATH". By default, only one core will be used. To use multiple cores, please specify the number of cores by command "./pdos.bin $DATA_FILE_PATH $CORE_NUMBER". For example, if you want to run using data in file sample_data.txt with 4 cores, the command will be "./pdos.bin sample_data.txt 4".

GPU Version:

There are only one GPU version of finding the optimal solution, which is develoepd on the basis of CUDA. Enter "GPU" folder and use "make" in command line to complie with NVCC. The default data is stored in data.txt. To run using this data, just use command "make run". To use your own data, either replace the content of data.txt or use "./test $DATA_FILE_PATH".

