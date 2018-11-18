#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>

//각 이동 모션은 몇 발자국 이동하는지 확인 필요
#define TURN_LEFT_10				1
#define GO_FORWARD_SLOW_6			2
#define TURN_RIGHT_10				3
#define TURN_LEFT_3					4
#define CHECK_DISTANCE				5
#define TURN_RIGHT_3				6
#define TURN_LEFT_20				7
#define GO_FORWARD_SLOW				8
#define TURN_RIGHT_20				9
#define GO_FORWARD_FAST				10
#define GO_FORWARD_KEEP				11
#define GO_BACKWARD_KEEP			12
#define GO_RIGHT_KEEP				13
#define GO_LEFT_KEEP				14
#define GO_LEFT_20					15
#define SHUTDOWN					16
#define LEFT_HEAD_90				17
#define GYRO_OFF					18
#define TURN_RIGHT_60				19
#define GO_RIGHT_20					20
#define FRONT_HEAD					21
#define TURN_LEFT_45				22
#define ERROR_SOUND					23   //무슨 명령인지 체크 필요.
#define TURN_RIGTH_45				24
#define TURN_LEFT_60				25
#define BASIC_POSTURE				26
#define RIGHT_HEAD_90				27
#define LEFT_HEAD_45				28
#define DOWN_HEAD_80				29
#define RIGHT_HEAD_45				30
#define DOWN_HEAD_60				31
#define GO_BACKWARD_SLOW			32

typedef struct detectedImage {
	int x;            //x좌표
	int y;            //y좌표
	int degree;         //기울기
	int s_flag;         //성공 여부 확인 0실패 1성공
	int detected_object;
};      //영상이 모든 장애물을 합해서 디텍할까? 아니면 각각 디텍할까?

detectedImage current_image;
detectedImage line_image;
detectedImage barricade_image;
detectedImage door_image;

int uart_serial;
int stage_count = 0;

extern int start_uart();
extern int action(int, int);
extern int camStart();

//각 영상 처리는 다른 함수로 확인하는게 편하지 않을까
detectedImage detect_line();         //선 확인
detectedImage detect_barricade();      //장애물 확인
detectedImage detect_stair();         //계단 디텍
detectedImage detect_valve();         //밸브 타워 확인
// detectedImage detect_line_barricade();   //선 + 장애물 확인?(선택필요)

void follow_line();
//각 장애물 단계 함수
void start();
void stairs();
void bridge();
void before_tunnel();
void tunnel();
void before_valve();
void valve();

int main() {
	uart_serial = start_uart();
	if (uart_serial < 0) {
		printf("uart error");
	}
	else {
		while (1) {
			switch (stage_count) {
			case 6: before_tunnel();      break;
			case 7: tunnel();               break;
			case 8: before_valve();      break;
			case 9: valve();               break;
			default: break;
			}
			if (stage_count > 9)
				break;
		}
	}

	return 0;
}


detectedImage detect_line() {                     //선 디텍
	detectedImage image_info;

	//선 영상 처리 코드.

	return image_info;
}

detectedImage detect_barricade() {                  //장애물 디텍
	detectedImage image_info;

	//장애물 영상 처리 코드.

	return image_info;
}
//or
detectedImage detect_barricade() {                  //선 + 장애물 디텍
	detectedImage image_info;

	//선 영상 처리 코드
	//장애물 영상 처리 코드.

	return image_info;
}
detectedImage detect_stair() {                     //계단 디텍
	detectedImage image_info;

	//계단 디텍 코드

	return image_info;
}
detectedImage detect_valve() {                     //문 디텍
	detectedImage image_info;

	//문 영상 처리 코드.

	return image_info;
}

void follow_line() {
	if (current_image.degree > lean_to_the_left) { //선이 왼쪽으로 기울면
		action(uart_serial, TURN_LEFT_10);
	}
	else if (current_image.degree < lean_to_the_right) { //선이 오른쪽으로 기울면
		action(uart_serial, TURN_RIGHT_10);
	}
	else if (current_image.x < lean_to_the_left) { //선이 왼쪽에 있다면
		action(uart_serial, GO_RIGHT_20);
	}
	else if (current_image.x > lean_to_the_right) { //선이 오른쪽에 있다면
		action(uart_serial, GO_LEFT_20);
	}
	else {
		action(uart_serial, GO_FORWARD_5);
	}
}

void start() {
	action(uart_serial, DOWN_HEAD_30);         //머리 각도 조종

	while (1) {
		current_image = detect_line();   //장애물이 없는 구간은 선만 디텍해야 오류가 적을 것 같다.

		if (current_image.s_flag == 0) {   //실패시 자세 조정 필요
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
}

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