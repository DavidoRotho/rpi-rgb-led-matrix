// -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil; -*-
// Small example how to use the library.
// For more examples, look at demo-main.cc
//
// This code is public domain
// (but note, that the led-matrix library this depends on is GPL v2)

#include "led-matrix.h"

#include <unistd.h>
#include <math.h>
#include <stdio.h>
#include <signal.h>
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <thread>
#include <chrono>
using namespace cv;
using namespace std;

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

volatile bool interrupt_received = false;
static void InterruptHandler(int signo) {
	interrupt_received = true;
}




static void DrawOnCanvas(Canvas *canvas, VideoCapture cap) {
	/*
	 * Let's create a simple animation. We use the canvas to draw
	 * pixels. We wait between each step to have a slower animation.
	 */
	canvas->Fill(0, 0, 255);
	Mat frame;

	printf("\nCanvas Size: %d, %d\n\n", canvas->width(), canvas->height());
	while (true)
	{
		// wait for a new frame from camera and store it into 'frame'
		
		//printf("Read frame!\n");
		//printf("Matrix is %d by %d\n", frame.rows, frame.cols);
		cap.read(frame);
		// check if we succeeded
		if (frame.empty()) {
			cerr << "ERROR! blank frame grabbed\n";
			break;
		}
		int stride = 2;
		for (float i = 0; i < 192; i++)
			for (float j = 0; j < 192; j++) 
			{
				if (interrupt_received)
					return;
				Vec3b p = frame.at<Vec3b>(48+(j*stride), 128+(i*stride));
				canvas->SetPixel(i, j, p[2], p[1], p[0]);
			}
	}
}

void fourCCStringFromCode(int code, char fourCC[5]) {
    for (int i = 0; i < 4; i++) {
        fourCC[3 - i] = code >> (i * 8);
    }
    fourCC[4] = '\0';
}

int main(int argc, char *argv[]) {
	//--- INITIALIZE VIDEOCAPTURE
	VideoCapture cap;
	int deviceID = 0;             // 0 = open default camera
	int apiID = cv::CAP_V4L2;      // 0 = autodetect default API
	// open selected camera using selected API
	cap.open(deviceID, apiID);
	Mat frame;
	// check if we succeeded
	if (!cap.isOpened()) {
			cerr << "ERROR! Unable to open camera\n";
			return -1;
	}
    printf("Frame width: %f\n", cap.get(cv::CAP_PROP_FRAME_WIDTH));
    printf("Frame height: %f\n", cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    printf("Frame rate: %f\n", cap.get(cv::CAP_PROP_FPS));
	char fourCC[5];
	int fcd = cap.get(cv::CAP_PROP_FOURCC);
	fourCCStringFromCode(fcd, fourCC);
	printf("FourCC: %s\n", fourCC);
	printf("Format: %f\n", cap.get(cv::CAP_PROP_FORMAT));
	std::this_thread::sleep_for(std::chrono::milliseconds(512));
	//--- GRAB AND WRITE LOOP
	cout << "Start grabbing" << endl;
	cap.read(frame);
	//printf("Read frame!\n");
	//printf("Matrix is %d by %d\n", frame.rows, frame.cols);
	// check if we succeeded
	RGBMatrix::Options defaults;
	defaults.hardware_mapping = "regular";  // or e.g. "adafruit-hat"
	defaults.rows = 32;
	defaults.chain_length = 1;
	defaults.parallel = 1;
	defaults.show_refresh_rate = true;
	Canvas *canvas = RGBMatrix::CreateFromFlags(&argc, &argv, &defaults);
	if (canvas == NULL)
		return 1;

	// It is always good to set up a signal handler to cleanly exit when we
	// receive a CTRL-C for instance. The DrawOnCanvas() routine is looking
	// for that.
	signal(SIGTERM, InterruptHandler);
	signal(SIGINT, InterruptHandler);

	DrawOnCanvas(canvas, cap);    // Using the canvas.

	// Animation finished. Shut down the RGB matrix.
	canvas->Clear();
	delete canvas;

	return 0;
}

