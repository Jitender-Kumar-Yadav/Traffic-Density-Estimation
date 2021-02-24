all:
	g++ bird_view.cpp -o cam.out -std=c++11 `pkg-config --cflags --libs opencv4`
clean:
	rm *.out