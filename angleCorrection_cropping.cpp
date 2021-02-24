#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

string input_image_path, input_image_name, output_image_name, output_image_path, transformed_image_path;
vector<Point2f> pts_src, pts_dest;
Mat input_img, output_img, view_img, proj_img, crop_img;

void mouseCallBack(int event, int x, int y, int flags, void* userdata)
{
	if (event == EVENT_LBUTTONDOWN) {
		if (pts_src.size() < 4) {
			cout << "You selected point " << pts_src.size()+1 << " as " << x << " " << y << endl;
			pts_src.push_back(Point2f(x,y));
			if (pts_src.size() == 4) {
				pts_dest = {Point2f(472,52), Point2f(472,830), Point2f(800,830), Point2f(800,52)};
				
				// generate Homography matrix
				Mat homo = findHomography(pts_src, pts_dest);
				// warp source image to destination based on homography
				warpPerspective(view_img, output_img, homo, input_img.size()); // to be replaced with dest_img.size()
				// Crop the image
				Rect crop(472,52,328,788);
				Mat cropped_img = output_img(crop);
				// save the images in respective files in the output folder
				imwrite(transformed_image_path, output_img);
				imwrite(output_image_path, cropped_img);
				cout << "Done!!! bamzzinga" << endl;
				destroyWindow("Input window");
			}
			else{
				cout << "Please select next point." << endl;
			}
		}
	}
}

int main(int argc, char* argv[]) 
{
	if (argc == 1) {
		cout << "Enter the name of the image file to be transformed and cropped: ";
		cin >> input_image_name;    // input the name of the file from the user
	}
	else{
		input_image_name = argv[1];
	}
	input_image_path = "./images/" + input_image_name + ".jpg";
	
	if(argc < 3) {
		cout << "Enter a name (without extension) for the output file after being cropped: ";
		cin >> output_image_name;    // input the name of the output file from the user
	}
	else{
		output_image_name = argv[2];
	}
	output_image_path = "./out_images/cropped_" + input_image_name + ".jpg";
	transformed_image_path = "./out_images/projected_" + input_image_name + ".jpg";
	
	input_img = imread(input_image_path);    // read the input image
	if (input_img.empty()) {
		cout << "The requested file was not found." << endl;
		return -1;    // terminate the program
	}
	
	resize(input_img, view_img, Size(1024, 576));    // resize the image to match screen resolution
	namedWindow("Input window", 1);    // open a window
	setMouseCallback("Input window", mouseCallBack, NULL);    // allow selection of the points
	imshow("Input window", view_img);    // open the image in the required window
	cout << "Please select first point." << endl;
	waitKey(0);
	
	// opening the cropped and transformed files
	proj_img = imread(transformed_image_path);
	crop_img = imread(output_image_path);
	namedWindow("projected", WINDOW_NORMAL);    // open a window
	imshow("projected", proj_img);    // open the projected image in the required window
	namedWindow("cropped", WINDOW_NORMAL);    // open a window
	imshow("cropped", crop_img);    // open the cropped image in the required window
	waitKey(0);
}
