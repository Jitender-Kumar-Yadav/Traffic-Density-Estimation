#include <chrono>
#include <pthread.h>
#include <cmath>
#include "functions.h"

using namespace std::chrono;
using namespace std;

// Structure to hold argument and output for multithreading in part 4
struct method4_thread_data
{
	int start_limit, end_limit;
	vector<float> output;
};

// Structure to hold argument and output for multithreading in part 3
struct method3_thread_data
{
	int col_start, col_end;
	vector<float> output;
};

vector<float> baseline;
VideoCapture cap;
string video_name, video_path;

// Function to compare new output with baseline output
float getUtility(vector<float> &new_data)
{
	float error = 0;
	for (int i=0; i<new_data.size(); i++) {
		float temp = (baseline[i] - new_data[i])/baseline[i];
		error += temp*temp;
	}
	return sqrt(abs(error/new_data.size()));
}


/*
	description - Simulate method 1
	arg - sub_sample_param :- Number of sub sampling frames
	output - Utility and execution time
*/
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

/*
	description - Simulate method 2
	arg - resX, resY :- Cropping parameters
	output - Utility and execution time
*/
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

/*
	description - Thread function for multithreading in method 4
*/
void* process_frames_helper(void* arg)
{
	method4_thread_data* args_output;
	args_output = (method4_thread_data*) arg;
	VideoCapture cap1(video_path);
	args_output->output = process_frames(cap1, 1, 1024, 576, false, false, "", args_output->start_limit, args_output->end_limit)[0];
	pthread_exit(NULL);
}

/*
	description - Thread function for multithreading in method 3
*/
void* process_frames_helper_m3(void* arg)
{
	method3_thread_data* args_output;
	args_output = (method3_thread_data*) arg;
	VideoCapture cap1(video_path);
	args_output->output = process_frames(cap1, 1, 1024, 576, false, false, "", 0, 1000, args_output->col_start, args_output->col_end)[0];
	pthread_exit(NULL);
}

/*
	description - Simulate method 3
	arg - (ranges[i], ranges[i+1]) denotes the cropping parameters for ith thread
	output - Utility and execution time
*/
pair<float, float> method3(vector<int> ranges)
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
		args_output[i].col_start = ranges[i], args_output[i].col_end = ranges[i+1];
		int	rc = pthread_create(&threads[i], &attr, process_frames_helper_m3, (void *)&args_output[i]);
		if (rc) {
			cout << "Unable to create thread" << endl;
			return {};
		}
	}
	
	pthread_attr_destroy(&attr);
	for (int i=0; i<NUM_THREADS; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			cout << "Error: Unable to join" << endl;
			return {};
		}
	}
	// All threads are done
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	
	for (int i=0; i<NUM_THREADS; i++) {
		for (int j=0; j<args_output[i].output.size(); j++) {
			new_data[j] += args_output[i].output[j];
		}
	}
	float utility_val = getUtility(new_data);
	float timer = duration.count()/(float) 1000000;
	pair<float, float> outval(timer, utility_val);
	return outval;
}

/*
	description - Simulate method 4
	arg - (ranges[i], ranges[i+1]) denotes the number of frames to be processed by ith thread
	output - Utility and execution time
*/
pair<float, float> method4(vector<int> ranges)
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
			return {};
		}
	}
	
	pthread_attr_destroy(&attr);
	for (int i=0; i<NUM_THREADS; i++) {
		int rc = pthread_join(threads[i], &status);
		if (rc) {
			cout << "Error: Unable to join" << endl;
			return {};
		}
	}
	// All threads are done
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	
	for (int i=0; i<NUM_THREADS; i++) {
		for (int j=0; j<args_output[i].output.size(); j++) {
			new_data[ranges[i]+j] = args_output[i].output[j];
		}
	}
	float utility_val = getUtility(new_data);
	float timer = duration.count()/(float) 1000000;
	pair<float, float> outval(timer, utility_val);
	return outval;
}

// Function to generate input for method1 and method4
vector<int> genRange(int method, int numThreads)
{
	int maximum = method == 3 ? 1023 : cap.get(CAP_PROP_FRAME_COUNT);
	vector<int> out;
	for(int i = 0; i <= numThreads; i++){
		out.push_back((maximum*i)/numThreads);
	}
	return out;
}





// main code starts here

int main(int argc, char* argv[])
{
	// generate the homograph matrix to project and crop each image
	genHomograph("./images/empty.jpg");
	
	// input the name if not entered already
	if (argc == 1){
		cout << "\nEnter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
	}
	else video_name = argv[1];
	video_path = "./images/" + video_name + ".mp4";
	cap = VideoCapture(video_path);
	
	while(!cap.isOpened()){
		// keep taking the imput till a video path is valid and runnable
		cout << "The video could not be opened, please use a different path." << endl;
		cout << "\nEnter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
		video_path = "./images/" + video_name + ".mp4";
		VideoCapture cap(video_path);
	}
	
	cout << "\nGenerating Baseline" << endl;
	vector<vector<float>> base = process_frames(cap, 1, 1024, 576, false, false, "", 0, cap.get(CAP_PROP_FRAME_COUNT));
	baseline = base.at(0);
	cout << "Generated Baseline\n" << endl;
	
	int method;
	if(argc <= 2){
		cout << "Which method of utility-tradeoff analysis to be implemented? ";
		cin >> method;
	}
	else{
		method = stoi(argv[2]);
	}
	cout << " " << endl;
	
	// Input - Output interactions
	while(method != 0)
	{
		if(method == 1)
		{
			fstream outFile;
			outFile.open("./out_images/out_method_1.txt", ios::out);
			cout << "Starting with X = " << 1 << endl;
			cap = VideoCapture(video_path);
			pair<float, float> out = method1(1);
			outFile << 1 << ", " << out.first << ", " << out.second << endl;
			cout << 1 << ", " << out.first << ", " << out.second << endl;
			cout << "X = " + to_string(1) + " done" << "\n" << endl;
			for(int j = 5; j <= 100; j+=5){
				cout << "Starting with X = " << j << endl;
				cap = VideoCapture(video_path);
				pair<float, float> out = method1(j);
				outFile << j << ", " << out.first << ", " << out.second << endl;
				cout << j << ", " << out.first << ", " << out.second << endl;
				cout << "X = " + to_string(j) + " done" << "\n" << endl;
			}
		}
		else if(method == 2)
		{
			fstream outFile;
			outFile.open("./out_images/out_method_2.txt", ios::out);
			int res_X = 1024, res_Y = 576;
			for(int j = 1; j <= 10; j++){
				cout << "Starting with resolution " << res_X << "x" << res_Y << endl;
				cap = VideoCapture(video_path);
				pair<float, float> out = method2(res_X, res_Y);
				outFile << j << ", " << out.first << ", " << out.second << endl;
				cout << j << ", " << out.first << ", " << out.second << endl;
				cout << "Resolution " << res_X << "x" << res_Y << " done" << "\n" << endl;
				res_X = 1024 * (10 - j)/10;
				res_Y = 576 * (10 - j)/10;
			}
		}
		else if(method == 3 || method == 4)
		{
			fstream outFile;
			outFile.open("./out_images/out_method_" + to_string(method) + ".txt", ios::out);
			for(int numthreads = 1; numthreads <= 8; numthreads++){
				cout << "Starting with " << numthreads << " threads" << endl;
				pair<float, float> out = method == 3 ? method3(genRange(method, numthreads)) : method4(genRange(method, numthreads));
				outFile << numthreads << ", " << out.first << ", " << out.second << endl;
				cout << numthreads << ", " << out.first << ", " << out.second << endl;
				cout << numthreads << " threads done" << "\n" << endl;
			}
		}
		else cout << "Method must be 1 or 2 or 3 or 4 only." << endl;
		cout << "Enter the method of utility-tradeoff analysis to be implemented, enter 0 to stop (0/1/2/3/4): ";
		cin >> method;
		cout << " " << endl;
	}
}