#include <cstdio>
#include <iostream>
#include <raspicam/raspicam_cv.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include "protocol.h"


#define PI 3.1415926

#define LOW_RED_H 150
#define HIGH_RED_H 180
#define LOW_RED_S 30
#define HIGH_RED_S 255
#define LOW_RED_V 30
#define HIGH_RED_V 255

#define LOW_GREEN_H 30
#define HIGH_GREEN_H 90
#define LOW_GREEN_S 50
#define HIGH_GREEN_S 255
#define LOW_GREEN_V 40
#define HIGH_GREEN_V 255

#define LOW_BLUE_H 80
#define HIGH_BLUE_H 140
#define LOW_BLUE_S 65
#define HIGH_BLUE_S 255
#define LOW_BLUE_V 65
#define HIGH_BLUE_V 255

#define LOW_YELLOW_H 12
#define HIGH_YELLOW_H 179
#define LOW_YELLOW_S 110
#define HIGH_YELLOW_S 255
#define LOW_YELLOW_V 50
#define HIGH_YELLOW_V 255

#define WIDTH 640
#define HEIGHT 480


using namespace std;
using namespace cv;


detectedImage robot_capture(raspicam::RaspiCam_Cv &Camera, int obj_color, int detector_flag);
raspicam::RaspiCam_Cv cam_init();
void cam_finalize(raspicam::RaspiCam_Cv &Camera);

detectedImage detect_object(Mat img_mask);
detectedImage detect_line(Mat img_mask);



detectedImage mask_oper(int obj_color, int detector_flag) {
	detectedImage im;//test
	Mat img_hsv;
	Mat img_mask;
	Mat img;
	img = imread("current_img.bmp");
	cvtColor(img, img_hsv, COLOR_BGR2HSV);


	switch (obj_color) {

	case DETECT_RED:
		inRange(img_hsv, Scalar(LOW_RED_H, LOW_RED_S, LOW_RED_V), Scalar(HIGH_RED_H, HIGH_RED_S, HIGH_RED_V), img_mask); //red
		//inRange(img_hsv, Scalar(LOW_RED_H, LOW_RED_S, LOW_RED_V), Scalar(HIGH_RED_H, HIGH_RED_S, HIGH_RED_V), img_mask); //red
		break;

	case DETECT_GREEN:
		inRange(img_hsv, Scalar(LOW_GREEN_H, LOW_GREEN_S, LOW_GREEN_V), Scalar(HIGH_GREEN_H, HIGH_GREEN_S, HIGH_GREEN_V), img_mask); //green
		break;

	case DETECT_BLUE:
		inRange(img_hsv, Scalar(LOW_BLUE_H, LOW_BLUE_S, LOW_BLUE_V), Scalar(HIGH_BLUE_H, HIGH_BLUE_S, HIGH_BLUE_V), img_mask); //blue
		break;

	case DETECT_YELLOW:
		inRange(img_hsv, Scalar(LOW_YELLOW_H, LOW_YELLOW_S, LOW_YELLOW_V), Scalar(HIGH_YELLOW_H, HIGH_YELLOW_S, HIGH_YELLOW_V), img_mask);	//yellow
		break;

	}


//열림연산 (침식을 통해 잡음이 제거되고 팽창을 통해 크기가 줄어든 영역을 다시 키움
	erode(img_mask, img_mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	dilate(img_mask, img_mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	//닫힘연산(팽창을 통해 작은 구멍들을 채우고 침식을 통해 커진 영역을 다시 줄임)
	dilate(img_mask, img_mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));
	erode(img_mask, img_mask, getStructuringElement(MORPH_ELLIPSE, Size(5, 5)));

	cv::imshow("binary", img_mask);
	cv::imshow("hsv", img_hsv);

	//detect_object(img_mask, img, img_hsv);	

	//imwrite("preproc_img.bmp",img_mask);


	switch (detector_flag) {

	case DETECT_OBJECT:
		im = detect_object(img_mask);
		break;

	case DETECT_LINE:
		im = detect_line(img_mask);
		break;

	}

	std::cout<<"im.x: "<<im.x<<"\nim.y: "<<im.y<<"\nim.degree: "<<im.degree<<"\n";
	return im;
}
detectedImage detect_object(Mat img_mask) {

	detectedImage img_info_hor, img_info_ver, img_info;

	int x=0, y=0, area, left, top, width, height;  // label info

	Mat img_labels, stats, centroids, contours; //

	//Mat img_mask = imread("preproc_img.bmp");

	Mat img = imread("current_img.bmp");

	int numOfLables = connectedComponentsWithStats(img_mask, img_labels,
		stats, centroids, 8, CV_32S);

	//영역박스 그리기
	int max = -1, idx = 0;
	for (int j = 1; j < numOfLables; j++) {

		area = stats.at<int>(j, CC_STAT_AREA);

		if (max < area) {
			max = area;
			idx = j;
		}

		x = centroids.at<double>(idx, 0); //중심좌표
		y = centroids.at<double>(idx, 1);



	}


	left = stats.at<int>(idx, CC_STAT_LEFT);
	top = stats.at<int>(idx, CC_STAT_TOP);
	width = stats.at<int>(idx, CC_STAT_WIDTH);
	height = stats.at<int>(idx, CC_STAT_HEIGHT);

	circle(img, Point(x, y), 5, Scalar(255, 0, 0), 1);


	//red valve check
	img_info.valve_flag = width > height ? 1 : 0;  // 1 : valve closed 0: valve opend

	img_info.label_x = x > WIDTH ? 0 : x;
	img_info.label_y = y > HEIGHT ? 0 : y;

	rectangle(img, Point(left, top), Point(left + width, top + height), Scalar(0, 0, 255), 1);
	img_info.label_x = x;
	img_info.label_y = y;

	std::cout << "\n" << "label_x:" << img_info.label_x << "\n" << endl;
	std::cout << "label_y:" << img_info.label_y << "\n" << endl;


	cv::imwrite("object.png", img);
	cv::imshow("picamera test", img);

	//detect_line(img_mask,img);
	std::cout << "valve_flag :" << img_info.valve_flag << endl;

	return img_info;
}
detectedImage detect_line(Mat img_mask) {

	detectedImage img_info_hor, img_info_ver, img_info;
	Mat contours;

	Mat img = imread("current_img.bmp");
	//Mat img_mask = imread("preproc_img.bmp");

	Canny(img_mask, contours, 125, 350);

	// 선 감지 위한 허프 변환
	std::vector<cv::Vec2f> lines;
	cv::HoughLines(contours, lines, 1, PI / 180, 90);
	/* 단계별 크기*/   /* 투표(vote) 최대 개수*/
// 윤곽선 그리기
	Mat result(contours.rows, contours.cols, CV_8U, Scalar(255));
	cout << "--------------------------------------------------------\n" << "Lines detected: " << lines.size() << endl;

	// 선 벡터를 반복해 선 그리기
	std::vector<cv::Vec2f>::const_iterator it = lines.begin();

	for (int i = 0; i < 3; i++) {
		while (it != lines.end()) {

			float rho = (*it)[0];   // 첫 번째 요소는 rho 거리
			float theta = (*it)[1]; // 두 번째 요소는 델타 각도
			float center = (*it)[2];

			if (theta < PI / 4. || theta > 3.*PI / 4.) { // 수직 행

				cv::Point pt1(rho / cos(theta), 0); // 첫 행에서 해당 선의 교차점
				cv::Point pt2((rho - result.rows*sin(theta)) / cos(theta), result.rows);

				cv::line(img, pt1, pt2, cv::Scalar(255), 2); 	// 선으로 그리기
			//	std::cout<<"pt1.x:"<<pt1.x<<"pt1.y:"<<pt1.y<<"\n";
		//		std::cout<<"pt2.x:"<<pt2.x<<"pt2.y:"<<pt2.y<<"\n";
				img_info_ver.x = (int)((pt1.x + pt2.x) / 2);
				img_info_ver.y = (int)((pt1.y + pt2.y) / 2);
				img_info_ver.degree = (int)(theta / (PI / 180));
				img_info_ver.detected_object++;

				//if(img_info_ver.x
				/*
				center_line_x1=0;
				center_line_y1=0;
				center_line_x2=0;
				center_line_y2=0;

				 */

				circle(img, Point(img_info_ver.x, img_info_ver.y), 5, Scalar(255), 1);

				//circle(img, Point(img_info_ver.x, img_info_ver.y), 5 , Scalar(128,65,217), 1);
				//circle(img, Point(img_info_ver.x/lines.size(), img_info_ver.y/lines.size()), 5 , Scalar(128,65,217), 1);

				//circle(img, Point(img_info_ver.center_line_x/lines.size(), img_info_ver.center_line_y/lines.size()), 5 , Scalar(255,255,255), 1);

				//cv::line(img,Point(img_info_ver.x ,img_info_ver.y), cv::Scalar(255),2);

			}
			else { // 수평 행

				cv::Point pt1(0, rho / sin(theta)); // 첫 번째 열에서 해당 선의 교차점  
				cv::Point pt2(result.cols, (rho - result.cols*cos(theta)) / sin(theta));
				// 마지막 열에서 해당 선의 교차점
				cv::line(img, pt1, pt2, cv::Scalar(255, 100, 255), 2); //  선으로 그리기
		//		std::cout<<"pt1.x:"<<pt1.x<<"pt1.y:"<<pt1.y<<"\n";
		//		std::cout<<"pt2.x:"<<pt2.x<<"pt2.y:"<<pt2.y<<"\n";
				img_info_hor.x = (int)((pt1.x + pt2.x) / 2);
				img_info_hor.y = (int)((pt1.y + pt2.y) / 2);

				img_info_hor.degree = (int)(theta / (PI / 180));
				img_info_hor.detected_object++;
				//(int)((img_info_hor.x +img_info_hor.y))

			}

			std::cout << "degree: (" << (theta / (PI / 180)) << ")\n";

			//	std:: cout << "center: ("<<center<<")\n";

			++it;

			//std::cout<< "valve_flag :" << img_info.valve_flag <<endl;


		}
	}

	cv::imwrite("line.png", img);
	cv::imshow("image", img);

	return img_info_ver;
}

raspicam::RaspiCam_Cv cam_init() {
	raspicam::RaspiCam_Cv Camera;

	Camera.set(CV_CAP_PROP_FORMAT, CV_8UC3);
	Camera.set(CV_CAP_PROP_FRAME_WIDTH, WIDTH);
	Camera.set(CV_CAP_PROP_FRAME_HEIGHT, HEIGHT);
	Camera.set( CV_CAP_PROP_EXPOSURE, 2.5);
    Camera.set( CV_CAP_PROP_BRIGHTNESS, 45);
	//int obj_color=DETECT_YELLOW;
	if (!Camera.open()) { cerr << "Error opening the camera" << endl; }

	return Camera;
}
detectedImage robot_capture(raspicam::RaspiCam_Cv &Camera, int obj_color, int detector_flag) {
	Mat image;
	detectedImage result;

	Camera.grab();
	Camera.retrieve(image);

	cv::imwrite("current_img.bmp", image);

	result = mask_oper(obj_color, detector_flag);
	if (waitKey(20) == 27);
	
	return result;
}
void cam_finalize(raspicam::RaspiCam_Cv &Camera) {
	Camera.release();
}


/*
int main(int argc, char **argv) {
	robot_capture();
	mask_oper(DETECT_RED, DETECT_OBJECT);
}
*/

/* int main(int argc, char **argv) {
	robot_capture();
	//
	char det;
	int num = 0;
	string file;
	Mat image;

	while (1) {
		cout << "test? ";
		cin >> num >> det;
		if (num < 0) break;

		file = "image";
		file.append(to_string(num));
		file.append(".jpg");
		cout << file << endl;
		image = imread(file, 1);
		resize(image, image, Size(640, 480), 0, 0, INTER_LINEAR);

		try {
			mask_oper(DETECT_YELLOW, DETECT_LINE);
			if (waitKey(20) == 27) break;
		}
		catch (...) {
			continue;
		}
	}
	
	//
	//mask_oper();
}
*/
