#include <cstdio>
#include <iostream>
#include <raspicam/raspicam_cv.h>
#include <opencv2/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <math.h>

#define PI 3.1415926

using namespace std;
using namespace cv;

Mat mask_oper(Mat img);
void coordinate(Mat img_mask, Mat img, Mat img_hsv);
int camStart();

Mat mask_oper(Mat img){

	Mat img_hsv;
	Mat img_mask;

	cvtColor(img, img_hsv, COLOR_BGR2HSV);
	inRange(img_hsv, Scalar(26,64,71), Scalar(36, 255, 255), img_mask); //yellow
	inRange(img_hsv, Scalar(26,64,71), Scalar(36, 255, 255), img_mask);//white

	cv::imshow("frame",img_mask);
	cv::imshow("hsv",img_hsv);

	coordinate(img_mask, img, img_hsv);   

	return img_mask;
}


void coordinate(Mat img_mask, Mat img, Mat img_hsv){

	int x,y,area,left,top,width,height; 
	Mat img_labels, stats, centroids, contours;
	int numOfLables = connectedComponentsWithStats(img_mask, img_labels,
			stats, centroids, 8, CV_32S);

	//영역박스 그리기
	int max = -1, idx = 0;
	for (int j = 1; j < numOfLables; j++) {

		area = stats.at<int>(j, CC_STAT_AREA);

		if (max < area){
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

	circle(img, Point(x, y), 5 , Scalar(255, 0, 0), 1);


	Canny(img_mask, contours, 125, 350);

	// 선 감지 위한 허프 변환
	std::vector<cv::Vec2f> lines;
	cv::HoughLines(contours, lines, 
			1,PI/180, // 단계별 크기
			80);  // 투표(vote) 최대 개수

	// 선 그리기
	Mat result(contours.rows, contours.cols, CV_8U, Scalar(255));
	cout << "Lines detected: " << lines.size() << endl;

	// 선 벡터를 반복해 선 그리기
	std::vector<cv::Vec2f>::const_iterator it= lines.begin();

	for(int i=0; i<5; i++){
		while (it!=lines.end()) {

			float rho = (*it)[0];   // 첫 번째 요소는 rho 거리
			float theta = (*it)[1]; // 두 번째 요소는 델타 각도
			float center=(*it)[2];


			if (theta < PI/4. || theta > 3.*PI/4.) { // 수직 행

				cv::Point pt1(rho/cos(theta), 0); // 첫 행에서 해당 선의 교차점   
				cv::Point pt2((rho-result.rows*sin(theta))/cos(theta), result.rows);

				cv::line(img, pt1, pt2, cv::Scalar(255), 1);    // 선으로 그리기
			} 

			std::cout << "line: (" << rho  <<","<< abs(theta/(PI/180)) -90<<")\n";


			++it;

		}
	}

	rectangle(img, Point(left, top), Point(left + width, top + height),
			Scalar(0, 0, 255), 1);

	std::cout<<"--------------------------------------------------------"<<"\n"<<"x:"<<x<<"\n"<<endl;   
	std::cout<<"y:"<<y<<"\n"<<endl;

	cv::imshow("picamera test",img);
}


int camStart(){

	raspicam::RaspiCam_Cv Camera;
	Mat image;

	Camera.set( CV_CAP_PROP_FORMAT, CV_8UC3);
	Camera.set( CV_CAP_PROP_FRAME_WIDTH, 640);
	Camera.set( CV_CAP_PROP_FRAME_HEIGHT, 480);

	if (!Camera.open()) {cerr<<"Error opening the camera"<<endl; return -1;}

	while(1) {

		Camera.grab();
		Camera.retrieve(image);

		mask_oper(image);

		if (waitKey(20) == 27) break;

	}
	Camera.release();
} 
