// PlayVideo.cpp : Defines the entry point for the console application.
//

#include "stdafx.h" 
#include "opencv2/opencv.hpp"
#include <iostream>

using namespace cv;
using namespace std;



int PlayVideoFromFile(char * file)
{
	//open the video file for reading
	VideoCapture cap(file);

	// if not success, exit program
	if (cap.isOpened() == false)
	{
		cout << "Cannot open the video file" << endl;
		cin.get(); //wait for any key press
		return -1;
	}

	//Uncomment the following line if you want to start the video in the middle
	//cap.set(CAP_PROP_POS_MSEC, 300); 

	//get the frames rate of the video
	double fps = cap.get(CAP_PROP_FPS);
	cout << "Frames per seconds : " << fps << endl;

	String window_name = "My First Video";

	namedWindow(window_name, WINDOW_NORMAL); //create a window

	while (true)
	{
		Mat frame;
		bool bSuccess = cap.read(frame); // read a new frame from video 

										 //Breaking the while loop at the end of the video
		if (bSuccess == false)
		{
			cout << "Found the end of the video" << endl;
			break;
		}

		//show the frame in the created window
		imshow(window_name, frame);

		//wait for for 10 ms until any key is pressed.  
		//If the 'Esc' key is pressed, break the while loop.
		//If the any other key is pressed, continue the loop 
		//If any key is not pressed withing 10 ms, continue the loop
		if (waitKey((1000 / fps)*0.8) == 27)
		{
			cout << "Esc key is pressed by user. Stoppig the video" << endl;
			break;
		}
	}
	destroyWindow(window_name);
	return 0;
}


int PlayVideoFromCamera()
{
	//Open the default video camera
	VideoCapture cap(0);

	// if not success, exit program
	if (cap.isOpened() == false)
	{
		cout << "Cannot open the video camera" << endl;
		cin.get(); //wait for any key press
		return -1;
	}

	double dWidth = cap.get(CAP_PROP_FRAME_WIDTH); //get the width of frames of the video
	double dHeight = cap.get(CAP_PROP_FRAME_HEIGHT); //get the height of frames of the video

	cout << "Resolution of the video : " << dWidth << " x " << dHeight << endl;

	string window_name = "My Camera Feed";
	namedWindow(window_name); //create a window called "My Camera Feed"

	while (true)
	{
		Mat frame;
		bool bSuccess = cap.read(frame); // read a new frame from video 

										 //Breaking the while loop if the frames cannot be captured
		if (bSuccess == false)
		{
			cout << "Video camera is disconnected" << endl;
			cin.get(); //Wait for any key press
			break;
		}

		//show the frame in the created window
		imshow(window_name, frame);

		//wait for for 10 ms until any key is pressed.  
		//If the 'Esc' key is pressed, break the while loop.
		//If the any other key is pressed, continue the loop 
		//If any key is not pressed withing 10 ms, continue the loop 
		if (waitKey(10) == 27)
		{
			cout << "Esc key is pressed by user. Stoppig the video" << endl;
			break;
		}
	}
	destroyWindow(window_name);
	return 0;
}


int SaveVideoFromCamera()
{
	VideoCapture cap(0);

	if (cap.isOpened() == false)
	{
		cout << "Could not Open the Camera" << endl;
		cin.get();
		return -1;
	}

	int frame_width   = static_cast<int>(cap.get(CAP_PROP_FRAME_WIDTH));
	int frame_height = static_cast<int>(cap.get(CAP_PROP_FRAME_HEIGHT));

	Size frame_size(frame_width, frame_height);
	int fps = 20;

	//VideoWriter oVideoWriter("./MyCapture.avi", VideoWriter::fourcc('M', 'J', 'P', 'G'), fps, frame_size, true);
	VideoWriter oVideoWriter("./MyCapture.avi", VideoWriter::fourcc('M', 'P', '4', '2'), fps, frame_size, true);

	if (oVideoWriter.isOpened() == false)
	{
		cout << "Cannot save the video to a file, Writer opened failed" << endl;
		cin.get();
		return -1;
	}

	string window_name = "My Camera Feed";
	namedWindow(window_name);


	while (true)
	{
		Mat frame;
		bool isSuccess = cap.read(frame);

		if (isSuccess == false)
		{
			cout << "Video Camera is disconnected" << endl;
			cin.get();
			break;
		}

		oVideoWriter.write(frame);

		imshow(window_name, frame);

		if (waitKey(20) == 27)
		{
			cout << "ESC key is pressed by the user, Stopping the video" << endl;
			break;
		}
	}

	oVideoWriter.release();
	destroyWindow(window_name);
	
	return 0;
}


int main(int argc, char* argv[])
{
	int ret = 0;

	cout << "Please choose the function you want to run" << endl
		 << "1 : Play Video From Camera" << endl
		 << "2 : Play Video From File" << endl
		 << "3:  Save Video From Camera to File" << endl;

	switch (cin.get())
	{
    	case '1':
    		ret = PlayVideoFromCamera();
    		if (ret != 0)
    		{
    			cout << "Play Video Fraom Camera Failed, Will Try Play From File" << endl;
    		}
    		break;
    	case '2':
    		ret = PlayVideoFromFile("./test.mp4");
    		if (ret != 0)
    		{
    			cout << "Play Video From File Failed" << endl;
    		}    
    		break;
    	case '3':
			ret = SaveVideoFromCamera();
			if (ret != 0)
			{
				cout << "Save Video From Camera to File Failed" << endl;
			}
    		break;
    	default:
    		break;
	}

	return ret;
}

