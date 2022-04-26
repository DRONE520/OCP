#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace std;
using namespace cv;


int main(int argc, char* argv[]) 
{
	VideoCapture camera(0);
	if (!camera.isOpened()) {
		cerr << "ERROR: Could not open camera" << endl;
		return 1;
	}
	Mat frame;
	// Получение кадра с камеры.
	camera >> frame;

	if (argc > 1) {
		string filename(argv[1]);
		// Обработка кадра с камера.
		Mat thresh, gray;
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		threshold(gray, thresh, 128, 255, THRESH_BINARY);

		// Нахождение контуров на изображении.
		vector<vector<Point> > contours;
    		Mat contourOutput = thresh.clone();
    		findContours(contourOutput, contours, RETR_LIST, CHAIN_APPROX_NONE);

		// Рисование контуров на изображении.
    		Mat contourImage(thresh.size(), CV_8UC3, Scalar(0,0,0));
    		Scalar colors[3];
    		colors[0] = Scalar(255, 0, 0);
    		colors[1] = Scalar(0, 255, 0);
    		colors[2] = Scalar(0, 0, 255);
    		for (size_t idx = 0; idx < contours.size(); idx++) {
        		drawContours(frame, contours, idx, colors[idx % 3]);
    		}
		imwrite(filename, frame);
	}
	else {
		cout << "Picture from usb-cam not save! Missed first argument!" << endl;	
	}
	return 0;
}
