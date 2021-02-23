#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

string input_image_path;
vector<Point2f> pts_src, pts_dest;
Mat input_img, output_img;

void take_input()
{
	cout << "Enter input image path " << endl;
	cin >> input_image_path;
}

void mouseCallBack(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN) {
		if (pts_src.size() < 4) {
			cout << "Point " << pts_src.size()+1 << " is " << x << " " << y << endl;
			pts_src.push_back(Point2f(x,y));
			if (pts_src.size() == 4) {
				pts_dest = {Point2f(472,52), Point2f(472,830), Point2f(800,830), Point2f(800,52)};

				// Homography matrix
				Mat homo = findHomography(pts_src, pts_dest);

				// warp source image to destination based on homography
				warpPerspective(input_img, output_img, homo, input_img.size()); // to be replaced with dest_img.size()

				// Crop the image
				Rect crop(472,52,800,830);
				Mat cropped_img = output_img(crop);

				// save the images
				imwrite("./images/out.jpg", output_img);
				imwrite("./images/cropped_out.jpg", cropped_img);

				cout << "Done!!! bamzzinga" << endl;
				// May be we can close the window now
			}
		}
	}
}


int main(int argc, char* argv[]) 
{
	take_input();
	// read the input image
	input_img = imread(input_image_path);
	if (input_img.empty()) {
		cout << "Empty image" << endl;
		return -1;
	}

	namedWindow("Input window", 1);
	setMouseCallback("Input window", mouseCallBack, NULL);
	imshow("Input window", input_img);

	waitKey(0);
}