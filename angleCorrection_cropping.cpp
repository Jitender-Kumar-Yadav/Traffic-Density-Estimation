#include "functions.h"

string input_image_path, input_image_name, output_image_name, output_image_path, transformed_image_path;

// main code starts here

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
	
	input_img = imread(input_image_path);    // read the input image
	while (input_img.empty()) {
		cout << "The requested file was not found." << endl;
		cout << "Please enter the name of the image file to be transformed and cropped again: ";
		cin >> input_image_name;    // input the name of the file from the user
		input_image_path = "./images/" + input_image_name + ".jpg";
		input_img = imread(input_image_path);    // read the input image
	}
	
	if(argc < 3) {
		cout << "Enter a name (without extension) for the output file after being cropped: ";
		cin >> output_image_name;    // input the name of the output file from the user
	}
	else{
		output_image_name = argv[2];
	}
	output_image_path = "./out_images/cropped_" + output_image_name + ".jpg";
	transformed_image_path = "./out_images/projected_" + output_image_name + ".jpg";
	
	// generate the homography matrix and correct and crop the input image
	genHomograph(input_image_path);
	resize(input_img, view_img, Size(1024, 576));
	Mat corrected = correction(view_img, input_img);
	imwrite(transformed_image_path, corrected);
	Mat cropped = crop(corrected);
	imwrite(output_image_path, cropped);//correction_crop(corrected, input_img));
	
	// opening the cropped and transformed files
	proj_img = imread(transformed_image_path);
	crop_img = imread(output_image_path);
	namedWindow("projected", WINDOW_NORMAL);    // open a window
	imshow("projected", proj_img);    // open the projected image in the required window
	namedWindow("cropped", WINDOW_NORMAL);    // open a window
	imshow("cropped", crop_img);    // open the cropped image in the required window
	waitKey(0);
}
