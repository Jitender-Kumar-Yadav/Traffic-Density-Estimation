#include <chrono>
#include <pthread.h>

#include "functions.h"

using namespace std::chrono;
using namespace std;



struct method4_thread_data {
	int start_limit, end_limit;
	vector<float> output;
};

vector<float> baseline;
VideoCapture cap;
string video_name, video_path;

float getUtility(vector<float> &new_data) {
	assert(baseline.size() == new_data.size());
	float error = 0;
	for (int i=0; i<baseline.size(); i++) {
		error += (baseline[i] - new_data[i])*(baseline[i] - new_data[i]);
	}
	error = sqrt(error);
	return error/new_data.size();
}

void method1( int sub_sample_param) {
	// vector<float> base_line = process_frames(cap, 1, 1024, 576, false, false, "").at(0);
	// vector<float> utility, exec_time;
	auto start = high_resolution_clock::now();
	vector<float> new_data = process_frames(cap, sub_sample_param, 1024, 576, false, false, "", 0, cap.get(CAP_PROP_FRAME_COUNT)).at(0);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	float utility_val = getUtility(new_data);
	// exec_time.push_back(duration.count());
	// utility.push_back(utility_val);
}

void* process_frames_helper(void* arg) {
	method4_thread_data* args_output;
	args_output = (method4_thread_data*) arg;
	VideoCapture cap1(video_path);
	cout << cap1.get(CAP_PROP_FRAME_COUNT) << endl;
	args_output->output = process_frames(cap1, 1, 1024, 576, false, false, "", args_output->start_limit, args_output->end_limit)[0];

	pthread_exit(NULL); 
}

// (ranges[i], ranges[i+1]) determine the range of frames a thread will process
void method4(vector<int> ranges) {
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
	// setNumThreads(1);
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
	cout << cap.get(CAP_PROP_FRAME_COUNT) << endl;
	vector<int> v = {0, 500, 1000, 1500, 2000, 2500, 3000, 3500, 4000, 4500, 5000, 5500, cap.get(CAP_PROP_FRAME_COUNT)};
	vector<int> v1 = {0, cap.get(CAP_PROP_FRAME_COUNT)};
	vector<int> v2 = {10, 20, 30, 40, 50};
	vector<int> v3 = {10, 50};
	// method4(v3);
	// method4(v2);
	// method4(v1);
	method4(v);

	/*auto start = high_resolution_clock::now();
	vector<float> new_data = process_frames(cap, 1, 1024, 576, false, false, "", 10, 14).at(0);
	auto stop = high_resolution_clock::now();
	auto duration = duration_cast<microseconds>(stop - start);
	cout << duration.count() << endl;*/
}