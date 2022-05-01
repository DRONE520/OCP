#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <stdio.h>
#include <ctime>
#include <cmath>
#include <fstream>
#include <string>
#include <regex>
#include <vector>

#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
using namespace std;
using namespace cv;


class LabelParameters
{
public:
	bool isOuterIsCircle;
	int innerContours;
	int innerCircles;
	int innerOthers;

	LabelParameters(bool isOuterIsCirc, int innerCon, int innerCirc, int innerOth)
	{
		this->isOuterIsCircle = isOuterIsCirc;
		this->innerContours = innerCon;
		this->innerCircles = innerCirc;
		this->innerOthers = innerOth;
	}

	~LabelParameters()
	{
		delete& this->isOuterIsCircle;
		delete& this->innerContours;
		delete& this->innerCircles;
		delete& this->innerOthers;
	}
};


const string currentDateTime() {
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
	return buf;
}


bool frameAnalizer(Mat& frame, LabelParameters* lParam) {
	// Подготовка изображения.
	Mat processed = frame.clone();
	cvtColor(processed, processed, COLOR_BGR2GRAY);
	GaussianBlur(processed, processed, Size(3, 3), 1);
	threshold(processed, processed, 127, 255, THRESH_BINARY);

	// Нахождение контуров.
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	Mat contourOutput = processed.clone();
	findContours(contourOutput, contours, hierarchy, RETR_TREE, CHAIN_APPROX_NONE);

	// Анализ иерархии.
	bool isLabelOnImage = false;
	if (!contours.empty() && !hierarchy.empty()) {
		bool isNextTrue = false;
		bool isNextNextTrue = false;
		bool isPrevTrue = false;
		bool isPrevPrevTrue = false;

		for (int i = 0; i < hierarchy.size(); i++) {
			// Если у контура есть дочерний...
			if (hierarchy[i][2] != -1) {
				int inner = hierarchy[i][2];

				// Если у дочернего контура есть вложенные контуры...
				if (hierarchy[inner][2] != -1) {
					int inner_inner = hierarchy[inner][2];

					// Проверка "следующего" контура.
					if (hierarchy[inner_inner][0] != -1) {
						int next = hierarchy[inner_inner][0];
						if (hierarchy[next][3] == inner && hierarchy[next][2] != -1) {
							isNextTrue = true;
						}

						if (hierarchy[next][0] != -1 && hierarchy[next][0] != next && hierarchy[next][0] != inner_inner) {
						    int next_next = hierarchy[next][0];
						    if (hierarchy[next_next][3] == inner && hierarchy[next_next][2] != -1) {
						        isNextNextTrue = true;
						    }
						}
					}

					// Проверка "предыдущего" контура.
					if (hierarchy[inner_inner][1] != -1) {
						int prev = hierarchy[inner_inner][1];
						if (hierarchy[prev][3] == inner && hierarchy[prev][2] != -1) {
							isPrevTrue = true;
						}

						if (hierarchy[prev][1] != -1 && hierarchy[prev][1] != prev && hierarchy[prev][1] != inner_inner) {
							int prev_prev = hierarchy[prev][1];
							if (hierarchy[prev_prev][3] == inner && hierarchy[prev_prev][2] != -1) {
								isPrevPrevTrue = true;
							}
						}
					}
				}
			}
		}

		if (isNextNextTrue && isNextTrue || isPrevTrue && isPrevPrevTrue) {
			isLabelOnImage = true;
		}
	}

	drawContours(frame, contours, -1, Scalar(0, 0, 255), 1);
	return isLabelOnImage;
}


int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "ru");

	// Параметры метки.
	LabelParameters* lParam = new LabelParameters(true, 11, 9, 2);

	VideoCapture camera(0);
	if (!camera.isOpened()) {
		cerr << "ERROR: Could not open camera..." << endl;
		return 1;
	}

	// Создание именованного окна.
	// namedWindow("Camera", WINDOW_AUTOSIZE);

	// Переменные для расчёта количества кадров.
	time_t startTime, curTime;
	time(&startTime);
	int numFramesCaptured = 0;
	double secElapsed;
	double curFPS;
	double averageFPS = 0.0;

	Mat frame;
	while (1) {
		// Получение текущего кадра.
		Mat frame;
		camera >> frame;

		// Количество кадров в секунду.
		numFramesCaptured++;
		time(&curTime);
		double secElapsed = difftime(curTime, startTime);
		double curFPS = (double)numFramesCaptured / secElapsed;

		// Обработка inf значений.
		if (isinf(curFPS)) curFPS = NULL;

		// Проверка, что кадр получен.
		if (frame.empty())
			break;

		// Отображение кадра.
		bool isLabel = frameAnalizer(frame, lParam);

		// Вывод информации о кадре.
		cout << "[" << currentDateTime() << "] - " << "Fps: " << round(curFPS) << " - Height: " << "None" << " - Detected: ";
		(isLabel) ? cout << "True\n" : cout << "False\n";
	}
	camera.release();
	return 0;
}
