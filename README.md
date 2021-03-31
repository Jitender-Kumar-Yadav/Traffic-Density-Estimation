# Traffic-Density-Estimation

angleCorrection_cropping.cpp:
	1. The input file should be stored as a ".jpg" file in the folder "./images/"
	2. Type "make angleC" on the command line. The file shall be compiled.
	3. Type "./angleC.out" on the command line.
	4. Alternatively type "./angleC.out arg1 arg2" or "./angleC.out arg1"
	   where arg1 and arg2 denote the names of the input file to be cropped and the name of the output file without extensions.
	   (arg1 and arg 2 are optional)
	5. The file is stored as two images cropped_arg2 and projected_arg2 in the folder "./out_images/"
	6. Type "make clean" to wipe off the extra generated .out files.

density_estimation.cpp:
	1. The input video should be stored as a ".mp4" file and an image of the empty road as "empty.jpg" in the folder "./images/"
	2. Type "make density" on the command line. The file shall be compiled.
	3. Type "./density.out" on the command line to run the file.
	4. Alternatively type "./density.out arg" where arg is the name of the video file without extension.
	5. The text file containing per frame output is stored as "out.txt" in the folder "./out_images/"
	6. Type "make graph" to plot the graph for the points stored in "out.txt".
	   The graph is stored as "out_graph.png" in the folder "./out_images/"
	7. Type "make clean" to wipe off the extra generated .out files.

utility_tradeoff.cpp
	1. The input video should be stored as a ".mp4" file and an image of the empty road as "empty.jpg" in the folder "./images/"
	2. Type "make utility" on the command line. The file shall be compiled.
	3. Type "./utility.out" on the command line to run the file.
	4. Alternatively type "./utility.out arg1" where arg1 is the name of the video file without extension
	   or "./utility.out arg1 arg2" where arg2 is the method of utility-tradeoff to be executed.
	5. The text files shall be stored as "out_method_1.txt", etc. in the folder "./out_images/"
	6. Type "make utilPlot" to plot the graph for utility-time plots for the stored data.
	   The graph is stored as "out_graph.png" in the folder "./out_images/"
	7. Type "make clean" to wipe off the extra generated .out files.