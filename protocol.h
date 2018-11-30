#define CRAWL_RE 1
#define FREQUENCIES_FORWARD_STEPS_OFTEN 2
#define STAIR_RIGHT_FOOT_CLIMB_1CM 3
#define MY_MADE_STAIRS_DOWN 4
#define LOWERING_THE_LEVER 5
#define RIGHT_TURN_3 6
#define LEFT_TURN_20 7
#define OPENING_DOORS 8
#define RIGHT_TURN_20 9
#define FORWARD_RUNNING_50 10
#define CONTINUOUS_ADVANCE 11
#define CONTINUOUS_REVERSE 12
#define RIGHT_SIDE_70_CONSECUTIVE 13
#define LEFT_SIDE_70_CONSECUTIVE 14
#define LEFT_SIDE_20 15
#define LEG_MOTOR_MODE3 16 // REMOTE CONTROL POWER BUTTON
#define HEAD_LEFT_90_DEGREES 17
#define GYRO_OFF 18
#define RIGHT_TURN_60 19
#define RIGHT_SIDE_20 20
#define HEAD_LEFT_AND_RIGHT_CENTER 21
#define LEFT_TURN_45 22
#define ERROR_SOUND 23
#define RIGHT_TURN_45 24
#define LEFT_TURN_60 25
#define BASIC_ATTITUDE_2 26
#define HEAD_RIGHT_90_DEGREES 27
#define HEAD_LEFT_45_DEGREES 28
#define FORWARD_DOWNWARD_80_DEGREES 29
#define HEAD_RIGHT_45_DEGREES 30
#define FORWARD_DOWNWARD_60_DEGREES 31
#define BACKWARD_OFTEN_STEPS 32


// opencv protocol
#define DETECT_NONE 0
#define DETECT_RED 1
#define DETECT_GREEN 2
#define DETECT_BLUE 3
#define DETECT_YELLOW 4

#define DETECT_OBJECT 1
#define DETECT_LINE 2



typedef struct detectedImage {
	int x; //x좌표
	int y; //y좌표
	int label_x, label_y;
	int degree; //기울기
	bool s_flag; //성공 여부 확인 0실패 1성공
	int detected_object; //호출단에서 검출할 색을 지정
	int detected_color;
	int detector_flag;
	int valve_flag;
	detectedImage() {
		x = 0;
		y = 0;
		label_x = 0;
		label_y = 0;
		degree = 0;
		s_flag = 0;
		detected_object = 0;
		detected_color = 0;
		detector_flag = 0;
		valve_flag = 0;

	}
} detectedImage;
