#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <fstream>

using namespace std;
using namespace cv;

string video_name, video_path;
vector<Point2f> pts_src , pts_dest;
vector<int> queue_car, dynamic_car;
Mat homo, frame_curr, frame_next, frame, inter, fgmask, empty;
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

Mat correction_crop(Mat frame, Mat f){
	// warp source image to destination based on homography
	Mat output_img;
	warpPerspective(frame, output_img, homo, f.size());
	// crop the image
	Rect crop(472,52,328,788);
	Mat cropped_img = output_img(crop);
	return cropped_img;
}

int getContours(Mat gray_frame) {
	// Function to get number of contours which can represent a vehicle in a frame
	vector<vector<Point> > contours;
    vector<Vec4i> hierarchy;
    findContours( gray_frame, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE );	// get the list of contours
    int count_contours = 0;
    for (auto contour: contours) {
    	if (contourArea(contour) >= 1000){							// if area of contour > limit, then, it represents vehicle
    		count_contours += 1;
    	}
    }
    return count_contours;
}

Mat process_queue(Mat frame) {
	Mat gray_frame, img_thresh;
	Mat structuringElement2x2 = getStructuringElement(MORPH_RECT, cv::Size(2, 2));
	cvtColor(frame, gray_frame, COLOR_RGBA2GRAY);
	GaussianBlur(gray_frame, gray_frame, Size(3, 3), 0);
	threshold(gray_frame, img_thresh, 150, 255.0, THRESH_BINARY);
	dilate(img_thresh, img_thresh, structuringElement2x2);
	return img_thresh;
}

int queueDensity(Mat frame) {
	Mat gray_frame, img_thresh;
	Mat structuringElement2x2 = getStructuringElement(MORPH_RECT, cv::Size(2, 2));
	cvtColor(frame, gray_frame, COLOR_RGBA2GRAY);
	GaussianBlur(gray_frame, gray_frame, Size(3, 3), 0);
	threshold(gray_frame, img_thresh, 150, 255.0, THRESH_BINARY);
	dilate(img_thresh, img_thresh, structuringElement2x2);
	return getContours(img_thresh);
} 

Mat process_dynamic(Mat frame, Mat frame_next) {
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
	
	return img_thresh;
}

int dynamicDensity(Mat frame, Mat frame_next) {
	Mat img = process_dynamic(frame, frame_next);
	return getContours(img);
}





int main(int argc, char* argv[]){
	// generate the homograph matrix to project and crop each image
	genHomograph("./images/empty.jpg");
	
	// input the name if 
	if (argc == 1){
		cout << "Enter the name of the video to estimate density of traffic with extension: ";
		cin >> video_name;											// input the name and address of the video from the user
	}
	else video_name = argv[1];
	video_path = "./images/" + video_name + ".mp4"; 
	VideoCapture cap(video_path);
	
	while(!cap.isOpened()){
		// keep taking the imput till a video path is valid and runnable
		cout << "The video could not be opened, please use a different path." << endl;
		cout << "Enter the name of the video to estimate density of traffic with extension: ";
		cin >> video_name;											// input the name and address of the video from the user
		video_path = "./images/" + video_name + ".mp4";
		VideoCapture cap(video_path);
	}

	
	// extracting the density
	fstream output;
	output.open("./out_images/out.txt", ios::out);
	int count = 0;
	cap >> frame;													// capture first frame
	if(frame.empty()){
		cout << "The video was empty.";
		return 1;
	}

	cap.set(CAP_PROP_POS_FRAMES, 2210);
	cap >> empty;
	resize(empty, inter, Size(1024, 576));
	empty = correction_crop(inter, empty);

	imshow("haha", empty);
	waitKey(0);
	cap.set(CAP_PROP_POS_FRAMES, 0);
	cap >> frame;

	resize(frame, inter, Size(1024, 576));
	frame_curr = correction_crop(inter, frame);							// frame correction

	pBackSub = createBackgroundSubtractorMOG2();
	while(true){

		// pBackSub->apply(frame_curr, fgmask);
		
		count = count + 1;
		queue_car.push_back(dynamicDensity(frame_curr, empty));				// store the static density of the frame
		cap >> frame;
		if(frame.empty()) break;
		resize(frame, inter, Size(1024, 576));
		frame_next = correction_crop(inter, frame);						// frame correction
		/*Mat img = process_dynamic(frame_curr, frame_next);
		imshow("haha", img);*/
		if (waitKey(10)==27) break;
		dynamic_car.push_back(dynamicDensity(frame_curr, frame_next));	// store the dynamic density of the frames
		frame_curr = frame_next;									// move to next frame
		// cout << count << ", " << queue_car[count - 1] << ", " << dynamic_car[count - 1] << endl;
		output << count << ", " << queue_car[count - 1] << ", " << dynamic_car[count - 1] << endl;
	}
	output.close();
}