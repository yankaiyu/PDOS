all:
	nvcc -arch=sm_20 PDOS.cu -o pdos.bin
run:
	./pdos.bin data.txt
clean:
	rm -rf pdos.bin
