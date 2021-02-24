all:
	g++ angleCorrection_cropping.cpp -o run.out -std=c++11 `pkg-config --cflags --libs opencv4`
clean:
	rm *.out