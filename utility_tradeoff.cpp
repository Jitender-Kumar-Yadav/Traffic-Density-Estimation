#include <chrono>
#include "functions.h"

using namespace std::chrono;

float getUtility(vector<float> &baseline, vector<float> &new_data) {
	assert(baseline.size() == new_data.size());
	float error = 0;
	for (int i=0; i<baseline.size(); i++) {
		error += (baseline[i] - new_data[i])*(baseline[i] - new_data[i]);
	}
	error = sqrt(error);
	return error/new_data.size();
}

void method1(VideoCapture cap, int sub_sample_param) {
	vector<float> base_line = process_frames(cap, 1, 1024, 576, false, false, "").at(0);
	vector<float> utility, exec_time;
	auto start = high_resolution_clock::now();
	vector<float> new_data = process_frames(cap, sub_sample_param, 1024, 576, false, false, "").at(0);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	float utility_val = getUtility(base_line, new_data);
	exec_time.push_back(duration.count());
	utility.push_back(utility_val);
}





// main code starts here

int main(int argc, char* argv[])
{
	
}