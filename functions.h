#ifndef traffic_density_estimation
#define traffic_density_estimation
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;
using namespace cv;

vector<Point2f> pts_src, pts_dest;
Mat homo, input_img, view_img, output_img, proj_img, crop_img;

Ptr<BackgroundSubtractor> pBackSub;

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

void genHomograph(string emptyFile)
{
	Mat input_img = imread(emptyFile);								//open an image of the empty road
	Mat view_img;
	resize(input_img, view_img, Size(1024, 576));					//resize the image to be shown
	namedWindow("Point Selection", 1);								// open a window
	setMouseCallback("Point Selection", mouseCallBack, NULL);		// allow selection of the points
	imshow("Point Selection", view_img);							// open the image in the required window
	cout << "Please select first point." << endl;
	waitKey(0);
}

Mat crop(Mat frame){
	// crop the image
	cv::Range cols(472, 800);
	cv::Range rows(52, 840);
	Mat cropped_img = frame(rows, cols);
	return cropped_img;
}

Mat correction(Mat frame, Mat f)
{
	// warp source image to destination based on homography
	Mat output_img;
	warpPerspective(frame, output_img, homo, f.size());
	return output_img;
}

Mat correction_crop(Mat frame, Mat f)
{
	return crop(correction(frame, f));
}

int getContours(Mat gray_frame) {
	// Function to get number of contours which can represent a vehicle in a frame
	vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( gray_frame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );	// get the list of contours
    int count_contours = 0;
    for (auto contour: contours) {
    	if (contourArea(contour) >= 1000){							// if area of contour > limit, then, it represents a vehicle
    		count_contours += 1;
    	}
    }
    return count_contours;
}

float dynamicDensity(Mat frame, Mat frame_next) {
	assert(!frame.empty());
	assert(!frame_next.empty());
	Mat gray_frame, gray_frame_next, diff_frame, img_thresh;
	cvtColor(frame, gray_frame, COLOR_RGBA2GRAY);
	cvtColor(frame_next, gray_frame_next, COLOR_RGBA2GRAY);
	GaussianBlur(gray_frame, gray_frame, Size(3, 3), 0);
	GaussianBlur(gray_frame_next, gray_frame_next, Size(3, 3), 0);
	absdiff(gray_frame, gray_frame_next, diff_frame);
	threshold(diff_frame, img_thresh, 16, 255.0, THRESH_BINARY);
	Mat structuringElement2x2 = getStructuringElement(MORPH_RECT, cv::Size(2, 2));
	dilate(img_thresh, img_thresh, structuringElement2x2);
	dilate(img_thresh, img_thresh, structuringElement2x2);
	dilate(img_thresh, img_thresh, structuringElement2x2);
	return getContours(img_thresh);
}

vector<vector<float>> process_frames(VideoCapture cap, int sub_sample_param, int res_X, int res_Y, bool console_out, bool file_out, string file, int start_limit, int end_limit, int col_start = 0, int col_end = 1023) {
	// extracting the density
	Mat frame_curr, frame_next, frame, inter, empty;
	int count = 0;
	vector<float> queue_car;
	vector<float> dynamic_car;
	fstream output;
	if(file_out){
		output.open("./out_images/" + file + ".txt", ios::out);
	}
	cap >> frame;													// capture first frame
	if(frame.empty()){
		cout << "The video was empty.";
		return {};
	}
	cap.set(CAP_PROP_POS_FRAMES, 2210);
	cap >> empty;
	resize(empty, inter, Size(1024, 576));
	empty = correction_crop(inter, empty);
	resize(empty, empty, Size(res_X, res_Y));
	cv::Range cols(col_start, col_end);
	cv::Range rows(0, 575);
	if(!(col_start == 0 && col_end == 1023)) empty = empty(rows, cols);
	cap.set(CAP_PROP_POS_FRAMES, start_limit);
	cap >> frame;
	resize(frame, inter, Size(1024, 576));
	frame_curr = correction_crop(inter, frame);						// frame correction
	resize(frame_curr, frame_curr, Size(res_X, res_Y));
	if(!(col_start == 0 && col_end == 1023)) frame_curr = frame_curr(rows, cols);
	int sub_sample = 0;
	float current_res, current_res_dynamic;
	while(true) {
		count = count + 1;
		if (start_limit + count == end_limit) break;
		if (sub_sample == 0) {
			cap >> frame;
			if(frame.empty()) break;
			resize(frame, inter, Size(1024, 576));
			frame_next = correction_crop(inter, frame);				// frame correction
			resize(frame_next, frame_next, Size(res_X, res_Y));
			if(!(col_start == 0 && col_end == 1023)) frame_next = frame_next(rows, cols);
			current_res = dynamicDensity(frame_curr, empty)/(float)7.1;
			current_res_dynamic = dynamicDensity(frame_curr, frame_next)/(float)10.9;
			queue_car.push_back(current_res);						// store the static density of the frame
			dynamic_car.push_back(current_res_dynamic);				// store the dynamic density of the frames
			sub_sample += 1;
		}
		else {
			queue_car.push_back(current_res);
			dynamic_car.push_back(current_res_dynamic);
			sub_sample += 1;
		}
		if (sub_sample == sub_sample_param) {
			sub_sample = 0;
		}
		if (waitKey(10)==27) break;
		frame_curr = frame_next;									// move to next frame
		if(file_out) output << count << ", " << queue_car[count - 1] << ", " << dynamic_car[count - 1] << endl;
		if(console_out) cout << count << ", " << queue_car[count - 1] << ", " << dynamic_car[count - 1] << endl;
	}
	vector<vector<float>> out;
	out.push_back(queue_car);
	out.push_back(dynamic_car);
	if(file_out) output.close();
	return out;
}

#endif