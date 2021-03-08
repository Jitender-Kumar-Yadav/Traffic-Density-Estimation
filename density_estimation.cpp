#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

using namespace std;
using namespace cv;

string video_name, video_path;
vector<Point2f> pts_src, pts_dest;
vector<int> queue_car, dynamic_car;
Mat homo, frame_curr, frame_next, frame, inter;

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

int get_counters(Mat gray_frame) {
	// Function to get number of contours which can represent a vehicle in a frame
	vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( gray_frame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );	// get the list of contours
    int count_contours = 0;
    for (auto contour: contours) {
    	if (contourArea(contour) >= 25){							// if area of contour > limit, then, it represents vehicle
    		count_contours += 1;
    	}
    }
    return count_contours;
}

int queueDensity(Mat frame, Mat f) {
	Mat gray_frame, kernel = Mat(3,3,CV_8UC1,1);
	cvtColor(frame, gray_frame, COLOR_RGBA2GRAY);					// convert rgb frame to gray
	dilate(gray_frame, gray_frame, kernel);							// dilate the image
	return get_counters(gray_frame);
} 

int dynamicDensity(Mat frame, Mat frame_next, Mat f) {
	Mat gray_frame, gray_frame_next, diff_frame, kernel = Mat(3,3,CV_8UC1,1);
	cvtColor(frame, gray_frame, COLOR_RGBA2GRAY);
	cvtColor(frame_next, gray_frame_next, COLOR_RGBA2GRAY);			// convert rgb frame to gray
	absdiff(gray_frame_next, gray_frame, diff_frame);				// get the difference between frames
	dilate(diff_frame, diff_frame, kernel);							// dilate the image
	return get_counters(diff_frame);
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
	VideoCapture cap(video_path);
	
	while(!cap.isOpened()){
		// keep taking the imput till a video path is valid and runnable
		cout << "The video could not be opened, please use a different path." << endl;
		cout << "Enter the name of the video to estimate density of traffic: ";
		cin >> video_name;											// input the name and address of the video from the user
		video_path = "./images/" + video_name + ".jpg";
		VideoCapture cap(video_path);
	}
	
	cap >> frame;													// capture first frame
	if(frame.empty()){
		cout << "The video was empty.";
		return 1;	
	}
	resize(frame, inter, Size(1024, 576));
	frame_curr = correction_crop(inter);							// frame correction
	while(true){
		queue_car.push_back(queueDensity(frame_curr, frame));		// store the static density of the frame
		cap >> frame;
		if(frame.empty()) break;
		resize(frame, inter, Size(1024, 576));
		frame_next = correction_crop(inter);						// frame correction
		dynamic_car.push_back(dynamicDensity(frame_curr, frame_next, frame));	// store the dynamic density of the frames
		frame_curr = frame_next;									// move to next frame
	}
}