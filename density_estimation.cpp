#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

vector<Point2f> pts_src, pts_dest;
Mat homo;

void mouseCallBack(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN) {
		if (pts_src.size() < 4) {
			cout << "You selected point " << pts_src.size()+1 << " as " << x << " " << y << endl;
			pts_src.push_back(Point2f(x,y));
			if (pts_src.size() == 4) {
				pts_dest = {Point2f(472,52), Point2f(472,830), Point2f(800,830), Point2f(800,52)};				
				homo = findHomography(pts_src, pts_dest);	// generate Homography matrix
				destroyWindow("Point Selection");
			}
			else{
				cout << "Please select next point." << endl;
			}
		}
	}
}

void genHomograph(string emptyFile){
	Mat input_img = imread(emptyFile);									//open an image of the empty road
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
	// Crop the image
	Rect crop(472,52,328,788);
	Mat cropped_img = output_img(crop);
	return cropped_img;
}

int main(int argc, char* argv[]){
	genHomograph("./images/empty.jpg");
}