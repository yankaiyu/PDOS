all:
	g++ *.cpp *.h -o pdos.bin -fopenmp -std=c++11
opt:
	g++ *.cpp *.h -fopenmp -std=c++11 -O3 -o pdos.bin
run:
	./pdos.bin data.txt
clean:
	rm -rf pdos.bin
