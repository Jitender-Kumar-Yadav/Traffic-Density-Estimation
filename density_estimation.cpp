#include "functions.h"

string video_name, video_path;

// main code starts here

int main(int argc, char* argv[]){
	// generate the homograph matrix to project and crop each image
	genHomograph("./images/empty.jpg");
	
	// input the name if not entered already
	if (argc == 1){
		cout << "Enter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
	}
	else video_name = argv[1];
	video_path = "./images/" + video_name + ".mp4"; 
	VideoCapture cap(video_path);
	
	while(!cap.isOpened()){
		// keep taking the imput till a video path is valid and runnable
		cout << "The video could not be opened, please use a different path." << endl;
		cout << "Enter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
		video_path = "./images/" + video_name + ".mp4";
		VideoCapture cap(video_path);
	}
	process_frames(cap, 1, 1024, 576, true, true, "out", 0, cap.get(CAP_PROP_FRAME_COUNT));
}