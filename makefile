all:
	g++ angleCorrection_cropping.cpp -o angleC.out -std=c++11 `pkg-config --cflags --libs opencv4`
	g++ density_estimation.cpp -o density.out -std=c++11 `pkg-config --cflags --libs opencv4`
clean:
	rm *.out