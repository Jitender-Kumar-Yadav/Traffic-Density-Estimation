#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

string video_name, video_path;
vector<Point2f> pts_src, pts_dest;
vector<float> queue, dynamic;
Mat homo, frame_curr, frame_next, frame;

void mouseCallBack(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN) {
		if (pts_src.size() < 4) {
			cout << "You selected point " << pts_src.size()+1 << " as " << x << " " << y << endl;
			pts_src.push_back(Point2f(x,y));
			if (pts_src.size() == 4) {
				pts_dest = {Point2f(472,52), Point2f(472,830), Point2f(800,830), Point2f(800,52)};				
				homo = findHomography(pts_src, pts_dest);			// generate Homography matrix
				destroyWindow("Point Selection");
			}
			else{
				cout << "Please select next point." << endl;
			}
		}
	}
}

void genHomograph(string emptyFile){
	Mat input_img = imread(emptyFile);								//open an image of the empty road
	Mat view_img;
	resize(input_img, view_img, Size(1024, 576));					//resize the image to be shown
	namedWindow("Point Selection", 1);								// open a window
	setMouseCallback("Point Selection", mouseCallBack, NULL);		// allow selection of the points
	imshow("Point Selection", view_img);							// open the image in the required window
	cout << "Please select first point." << endl;
	waitKey(0);
}

Mat correction_crop(Mat frame){
	// warp source image to destination based on homography
	Mat output_img;
	warpPerspective(frame, output_img, homo, frame.size());
	// crop the image
	Rect crop(472,52,328,788);
	Mat cropped_img = output_img(crop);
	return cropped_img;
}

int main(int argc, char* argv[]){
	// generate the homograph matrix to project and crop each image
	genHomograph("./images/empty.jpg");
	
	// input the name if 
	if (argc == 1){
		cout << "Enter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
	}
	else video_name = argv[1];
	video_path = "./images/" + video_name + ".jpg";
	Videocapture cap(video_path);
	
	while(!cap.isOpened()){
		// keep taking the imput till a video path is valid and runnable
		cout << "The video could not be opened, please use a different path." << endl;
		cout << "Enter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
		video_path = "./images/" + video_name + ".jpg";
		Videocapture cap(video_path);
	}
	
	cap >> frame;														// capture first frame
	if(frame.empty()){
		cout << "The video was empty.";
		return 1;	
	}
	frame_curr = correction_crop(frame);								// frame correction
	while(true){
		queue.push_back(queueDensity(frame_curr));						// store the static density of the frame
		cap >> frame;
		if(frame.empty()) break;
		frame_next = correction_crop(frame);							// frame correction
		dynamic.push_back(dynamicDensity(frame_curr));					// store the dynamic density of the frames
		frame_curr = frame_next;										// move to next frame
	}
}