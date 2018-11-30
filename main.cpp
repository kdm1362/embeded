#include <stdio.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <raspicam/raspicam_cv.h>
#include "protocol.h"

using namespace std;

detectedImage current_image;
detectedImage line_image;
detectedImage barricade_image;
detectedImage door_image;

int uart_serial;
int stage_count = 0;

extern int start_uart();
extern int action(int, int);

extern raspicam::RaspiCam_Cv cam_init();
extern void cam_finalize(raspicam::RaspiCam_Cv &Camera);
extern detectedImage robot_capture(raspicam::RaspiCam_Cv &Camera, int obj_color, int detector_flag);

/*
//각 영상 처리는 다른 함수로 확인하는게 편하지 않을까
detectedImage detect_line();         //선 확인
detectedImage detect_object();		// object confir
detectedImage detect_barricade();      //장애물 확인
detectedImage detect_stair();         //계단 디텍
detectedImage detect_valve();         //밸브 타워 확인
// detectedImage detect_line_barricade();   //선 + 장애물 확인?(선택필요)

void follow_line(raspicam::RaspiCam_Cv Camera);
//각 장애물 단계 함수
void stairs();
void bridge();
void before_tunnel();
void tunnel();
void before_valve();
void valve();
*/
void follow_line(raspicam::RaspiCam_Cv &Camera) ;

int main() {
	raspicam::RaspiCam_Cv Camera = cam_init();
	uart_serial = start_uart();
	if (uart_serial < 0) {
		printf("uart error");
		return -1;
	}
	
	while(1){
		follow_line(Camera);
	}
	
	
	/*
	while (1) {
		switch (stage_count) {
			case 6: before_tunnel();	break;
			case 7: tunnel();			break;
			case 8: before_valve();		break;
			case 9: valve();			break;
			default:					break;
		}
		if (stage_count > 9)
			break;
	}
	*/
	
	cam_finalize(Camera);
	return 0;
}


/*
detectedImage detect_stair() {                     //계단 디텍
	detectedImage image_info;

	//계단 디텍 코드
	image_info = robot_capture(DETECT_RED, DETECT_LINE);

	return image_info;
}
detectedImage detect_valve() {                     //문 디텍
	detectedImage image_info;
	image_info = robot_capture(DETECT_GREEN, DETECT_LINE);
	//문 영상 처리 코드.

	return image_info;
}
*/
void follow_line(raspicam::RaspiCam_Cv &Camera) {
	detectedImage current_image = robot_capture(Camera, DETECT_YELLOW, DETECT_LINE);
	int feedback = 0;
	
	if (current_image.detected_object != 0){	//if detected line
			if (current_image.x < 290)																	// line in left side
				feedback = action(uart_serial, 15);
			else if (current_image.x > 350)																	// line in right side
				feedback = action(uart_serial, 20);
			else if (current_image.degree < 175 && current_image.degree > 150)		// line lean to left 
				feedback = action(uart_serial, 1);
			else if (current_image.degree > 5 && current_image.degree < 40)		// line lean to right
				feedback = action(uart_serial, 3);
			else {  																												// we can go!
				feedback = action(uart_serial, 11);
			}
		}
		else{
			feedback = action(uart_serial, 8);
		}

		std::cout<<current_image.x<<"   "<<current_image.y<<"   "<<current_image.degree<<"   "<<current_image.s_flag<<endl;

}

/*
void stairs() {         //전방으로만 넘어지게 구동 요청, 기어서 가능한지 요구
	int stair_count = 0;
	action(uart_serial, DOWN_HEAD_80);

	while (1) {
		current_image = detect_stair();

		if (current_image.s_flag == 0) {
			continue;
		}
		else if (current_image.detected_object == STAIR && current_image.degree != horizon) {
			if (current_image.degree > lean_to_the_left) { //계단이 왼쪽으로 기울면
				action(uart_serial, TURN_LEFT_10);
			}
			else if (current_image.degree < lean_to_the_right) { //계단이 오른쪽으로 기울면
				action(uart_serial, TURN_RIGHT_10);
			}
		}
		else if (current_image.detected_object == STAIR && current_image.degree == horizon) {
			if (current_image.y == enough_position) {
				if (stair_count < 2)
					action(uart_serial, UP_STAIR);
				else
					action(uart_serial, DOWN_STAIR);
				stair_count++;
			}
			else {
				action(uart_serial, GO_FORWARD_1);
			}
		}
		if (stair_count >= 4)
			break;
	}
	stage_count++;
}

void bridge() {                  //외나무 다리는 그냥 선 트래킹만해서 지나가도 충분할것이라 생각
	action(uart_serial, DOWN_HEAD_30);         //머리 각도 조종

	while (1) {
		current_image = detect_line();   //장애물이 없는 구간은 선만 디텍해야 오류가 적을 것 같다.

		if (current_image.s_flag == 0) {
			continue;
		}
		else if (current_image.detected_object == LINE) {
			follow_line();
		}
		else if (current_image.detected_object == STAIR && current_image.y > enough_position) {
			break;
		}
		else {
			action(uart_serial, GO_FORWARD_1);
		}
	}
	stage_count++;
}         //그냥 외나무 다리 구간부터 문앞 장애물까지 하나의 알고리즘으로 통합하면 어떨까?

void before_tunnel() {
	while (1) {
		current_image = detect_line_barricade();

		if (current_image.s_flag == 0) {
			continue;
		}
		else if (current_image.detected_object == LINE) {
			follow_line();
		}
		else if (current_image.detected_object == BARRICADE) {
			if (current_image.object_size < Near_object) {   //Near_object의 값은 실험필요
				action(uart_serial, GO_FORWARD_1);
			}
			else {
				action(uart_serial, REMOVE_BARRICADE);
			}
		}

		if (action(CHECK_DISTANCE) < enough_distance) {      //적외선으로 터널 감지
			break;
		}
	}
	stage_count++;
}

void tunnel() {
	action(uart_serial, PASS_TUNNEL);
	stage_count++;
}

void before_valve() {
	while (1) {
		current_image = detect_line_barricade();

		if (current_image.s_flag == 0) {
			continue;
		}
		else if (current_image.detected_object == LINE) {
			follow_line();
		}
		else if (current_image.detected_object == BARRICADE) {
			if (current_image.object_size < enough_size) {   //enough_size의 값은 실험필요
				action(uart_serial, GO_FORWARD_1);
			}
			else {
				action(uart_serial, REMOVE_BARRICADE);
			}
		}

		if (current_image.detected_object == VALVE_TOWER && current_image.object_size > enough_size) {      //밸브 타워가 감지되며 동작 수행에 어느정도 접근한다면
			break;
		}
	}
	stage_count++;
}

void valve() {
	while (1) {
		current_image = detect_valve();

		if (current_image.detected_object == VALVE && action(CHECK_DISTANCE) < enough_distance) {      //밸브 타워 접근 확인. 이미지로 크기 처리보다는 적외선에 확실할듯함
			if (current_image.x < lean_to_the_left) { //밸브가 왼쪽에 있다면
				action(uart_serial, GO_RIGHT_20);
			}
			else if (current_image.x > lean_to_the_right) { //밸브가 오른쪽에 있다면
				action(uart_serial, GO_LEFT_20);
			}
			else if (current_image.degree < enough_degree) {      //밸브의 기울어짐이 만족되지 않는다면
				action(uart_serial, TOUCH_VALVE);
			}
			else {
				break;
			}
		}
	}
	stage_count++;
}

*/
