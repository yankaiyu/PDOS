all:
	g++ *.cpp *.h -o pdos.bin -fopenmp
opt:
	g++ *.cpp *.h -fopenmp -O3 -o pdos.bin
run:
	./pdos.bin data.txt
clean:
	rm -rf pdos.bin
