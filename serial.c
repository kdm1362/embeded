#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <time.h>

//로보베이직에서 송신하는 불필요한 값을 제거해야함
//한 명령에 수신된 마지막 한개의 값만 반환가능
//필요할 경우 구조체 혹은 배열으로 확장필요

int action(int fd, int key) //명령을 보내기 위해 사용
{
	int isEnd = 0;
	int received[20] = {0,};
	int index = 1;	//0 일경우 received 세그폴트

	//38명령이 씹혔을 경우 멈추지 않게 하기위한 코드
	time_t picker;
	time(&picker);

	//명령 송신
	printf("%d : sending \n", key);
	fflush(stdout);
	serialPutchar(fd, key);

	// switch 문으로 일일이 delay설정하지 않고 38(동작완료코드)가 왔는지 확인하는 코드로 대체
	while (isEnd != 1) {
		if (time(NULL) - picker <= 5) { //한 동작은 5초 내에 끝이나야함
			while (serialDataAvail(fd))
			{
				received[index] = serialGetchar(fd);
				printf("%d : received \n", received[index]);
				if (received[index] == 38) isEnd = 1;	//시리얼 통신을 수신할 수 있는지 체크

				fflush(stdout);
				index++;
				delay(30);		//값이 작을수록 잛은 동작에서 반응이 빨라짐
			}
		}
		else {
			action(fd, BASIC_ATTITUDE_2);
			return -1;	//통신이상으로 38반응이 5초이상 없을경우 무한루프 방지
		}
	}
	return received[index-1];	//수신한 값이 없을경우 0을 반환
}

int start_uart() { //시리얼 초기화
	int fd;

	if ((fd = serialOpen("/dev/ttyAMA0", 4800)) < 0)
	{
		fprintf(stderr, "Unable to open: %s\n", strerror(errno));
		return -1;
	}

	if (wiringPiSetup() == -1)
	{
		fprintf(stdout, "Unable to start wiringPi : %s\n", strerror(errno));
		return -1;
	}

	return fd;
}
