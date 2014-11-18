PDOS(Parallel Degree of Separation)
=============================
PDOS is short for Parallel Degree of Separation. It is a parallel algorithm designed and implemented by Kaiyu Yan and Xiaoyu Zheng from University of Michigan as a course project for calculating user pairs' degree of separation in a social network. It contains both CPU verison and GPU verison.

How to run
=============================
CPU Verision:
Enter the CPU folder and use "make" in command line to complie with g++. You can also use "make clean" to delete the executable file. Note: The CPU version make use of OpenMP functions, so please make sure the g++ complier you use support OpenMp features. The default data is stored in data.txt. To run using this data, just use command "make run". To use your own data, either replace the content of data.txt or using "./pdos $DATA_FILE_PATH"
