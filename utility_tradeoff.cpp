#include <chrono>
#include <pthread.h>
#include "functions.h"

using namespace std::chrono;
using namespace std;

struct method4_thread_data
{
	int start_limit, end_limit;
	vector<float> output;
};

struct method3_thread_data
{
	int row_start, row_end;
	vector<float> output;
};

vector<float> baseline;
VideoCapture cap;
string video_name, video_path;

float getUtility(vector<float> &new_data)
{
	assert(baseline.size() == new_data.size());
	float error = 0;
	for (int i=0; i<baseline.size(); i++) {
		error += (baseline[i] - new_data[i])*(baseline[i] - new_data[i]);
	}
	return (error/new_data.size());
}

pair<float, float> method1(int sub_sample_param)
{
	auto start = high_resolution_clock::now();
	vector<float> new_data = process_frames(cap, sub_sample_param, 1024, 576, false, false, "", 0, 1000).at(0);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	float utility_val = getUtility(new_data);
	float timer = duration.count()/(float) 1000000;
	pair<float, float> out(timer, utility_val);
	return out;
}

pair<float, float> method2(int resX, int resY)
{
	auto start = high_resolution_clock::now();
	vector<float> new_data = process_frames(cap, 1, resX, resY, false, false, "", 0, 1000).at(0);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	float utility_val = getUtility(new_data);
	float timer = duration.count()/(float) 1000000;
	pair<float, float> out(timer, utility_val);
	return out;
}

void* process_frames_helper(void* arg)
{
	method4_thread_data* args_output;
	args_output = (method4_thread_data*) arg;
	VideoCapture cap1(video_path);
	args_output->output = process_frames(cap1, 1, 1024, 576, false, false, "", args_output->start_limit, args_output->end_limit)[0];
	pthread_exit(NULL);
}

void* process_frames_helper_m3(void* arg)
{
	method3_thread_data* args_output;
	args_output = (method3_thread_data*) arg;
	VideoCapture cap1(video_path);
	args_output->output = process_frames(cap1, 1, 1024, 576, true, false, "", 0, 1000, args_output->row_start, args_output->row_end)[0];
	pthread_exit(NULL);
}

// (ranges[i], ranges[i+1]) determine the range of rows a thread will process
void method3(vector<int> ranges)
{
	int NUM_THREADS = ranges.size() - 1;
	vector<float> new_data(999);
	
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void* status;
	method3_thread_data args_output[NUM_THREADS]; 
	
	// Initialize and set thread joinable
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	auto start = high_resolution_clock::now();
	for (int i=0; i<NUM_THREADS; i++) {
		args_output[i].row_start = ranges[i], args_output[i].row_end = ranges[i+1];
		int	rc = pthread_create(&threads[i], &attr, process_frames_helper_m3, (void *)&args_output[i]);
		if (rc) {
			cout << "Unable to create thread" << endl;
			return;
		}
	}
	
	pthread_attr_destroy(&attr);
	for (int i=0; i<NUM_THREADS; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			cout << "Error: Unable to join" << endl;
			return;
		}
		cout << "Thread " << i << " done" << endl;
	}
	// All threads are done
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	cout << duration.count() << endl;
	fstream out;
	out.open("./out_images/out2.txt", ios::out);
	for (int i=0; i<NUM_THREADS; i++) {
		for (int j=0; j<args_output[i].output.size(); j++) {
			new_data[j] += args_output[i].output[j];
		}
	}
	
	for (int i=0; i<new_data.size(); i++) out << i << "," << new_data[i] << ",1" << endl;
	out.close();
}

// (ranges[i], ranges[i+1]) determine the range of frames a thread will process
void method4(vector<int> ranges)
{
	int NUM_THREADS = ranges.size() - 1, FRAME_COUNT = cap.get(CAP_PROP_FRAME_COUNT);
	vector<float> new_data(FRAME_COUNT);
	
	pthread_t threads[NUM_THREADS];
	pthread_attr_t attr;
	void* status;
	method4_thread_data args_output[NUM_THREADS]; 
	
	// Initialize and set thread joinable
   	pthread_attr_init(&attr);
   	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	auto start = high_resolution_clock::now();
	for (int i=0; i<NUM_THREADS; i++) {
		args_output[i].start_limit = ranges[i], args_output[i].end_limit = ranges[i+1];
		int	rc = pthread_create(&threads[i], &attr, process_frames_helper, (void *)&args_output[i]);
		if (rc) {
			cout << "Unable to create thread" << endl;
			return;
		}
	}
	
	pthread_attr_destroy(&attr);
	for (int i=0; i<NUM_THREADS; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			cout << "Error: Unable to join" << endl;
			return;
		}
		cout << "Thread " << i << " done" << endl;
	}
	// All threads are done
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	cout << duration.count() << endl;
	fstream out;
	out.open("./out_images/out1.txt", ios::out);
	for (int i=0; i<NUM_THREADS; i++) {
		for (int j=0; j<args_output[i].output.size(); j++) {
			new_data[ranges[i]+j] = args_output[i].output[j];
		}
	}
	
	for (int i=0; i<new_data.size(); i++) out << i << "," << new_data[i] << ",1" << endl;
	out.close();
}





// main code starts here

int main(int argc, char* argv[])
{
	// generate the homograph matrix to project and crop each image
	genHomograph("./images/empty.jpg");
	
	// input the name if not entered already
	if (argc == 1){
		cout << "Enter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
	}
	else video_name = argv[1];
	video_path = "./images/" + video_name + ".mp4";
	cap = VideoCapture(video_path);
	
	while(!cap.isOpened()){
		// keep taking the imput till a video path is valid and runnable
		cout << "The video could not be opened, please use a different path." << endl;
		cout << "Enter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
		video_path = "./images/" + video_name + ".mp4";
		VideoCapture cap(video_path);
	}
	vector<vector<float>> base = process_frames(cap, 1, 1024, 576, true, false, "", 0, 1000);
	baseline = base.at(0);
	
	int method;
	if(argc <= 2){
		cout << "Which method of utility-tradeoff analysis to be implemented? ";
		cin >> method;
	}
	else method = stoi(argv[2]);
	
	if(method == 1)
	{
		fstream outFile;
		outFile.open("./out_images/out_method_1.txt", ios::out);
		cout << "Starting with X = " << 1 << endl;
		pair<float, float> out = method1(1);
		outFile << 1 << ", " << out.first << ", " << out.second << endl;
		cout << 1 << ", " << out.first << ", " << out.second << endl;
		cout << "X = " + to_string(1) + " done" << endl;
		for(int j = 10; j <= 100; j+=10){
			cout << "Starting with X = " << j << endl;
			pair<float, float> out = method1(j);
			outFile << j << ", " << out.first << ", " << out.second << endl;
			cout << j << ", " << out.first << ", " << out.second << endl;
			cout << "X = " + to_string(j) + " done" << endl;
		}
	}
	if(method == 2)
	{
		
	}
	if(method == 3)
	{
		// int  X;
		// if(argc <= 3){
			// cout << "Enter the parameter, number of frames to be skipped: ";
			// cin >> X;
		// }
		// else X = argv[3];
	}
	if(method == 4)
	{
		// int  X;
		// if(argc <= 3){
			// cout << "Enter the parameter, number of frames to be skipped: ";
			// cin >> X;
		// }
		// else X = argv[3];
	}
	
	// cout << cap.get(CAP_PROP_FRAME_COUNT) << endl;
	// vector<int> v = {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, cap.get(CAP_PROP_FRAME_COUNT)};
	// vector<int> v1 = {0, cap.get(CAP_PROP_FRAME_COUNT)};
	// vector<int> v2 = {10, 20, 30, 40, 50};
	// vector<int> v3 = {10, 50};
	// // method4(v3);
	// // method4(v2);
	// // method4(v1);
	// method4(v);
	
	// vector<int> v1 = {0, 1023};
	// vector<int> v2 = {0, 511, 1023};
	// vector<int> v4 = {0, 255, 511, 767, 1023};
	// vector<int> v8 = {0, 127, 255, 383, 511, 639, 767, 895, 1023};
	// method3(v8);
}