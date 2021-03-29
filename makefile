angleC:
	g++ angleCorrection_cropping.cpp -o angleC.out -std=c++11 `pkg-config --cflags --libs opencv4`
density:
	g++ density_estimation.cpp -o density.out -std=c++11 `pkg-config --cflags --libs opencv4`
utility:
	g++ utility_tradeoff.cpp -o utility.out -pthread -std=c++11 `pkg-config --cflags --libs opencv4`

graph:
	sudo apt-get install python3-matplotlib
	python3 plotGraph.py
clean:
	rm *.out
