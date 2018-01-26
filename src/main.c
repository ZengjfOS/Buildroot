#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


// #define DEBUG
#ifdef DEBUG
    #define print(format, ...) printf(format"\r\n", ##__VA_ARGS__);
#else
    #define print(format, ...)
#endif

#define BUZZER_ENABLE	182
#define BUZZER_FREQENCY 183
#define BUZZER_DISABLE	184

struct GPIO_INFO {
	char dev[32];
	char mode[8];
	char freqency[16];
	int fd;
};
struct GPIO_INFO gpio_info = {
"/dev/pwm-buzzer",
"d",
"2000",
0,
};

void help(void)
{
	printf( \
" USAGE\r\n" \
"    -d: device node, default is /dev/gpio_mio\r\n" \
"    -m: mode\r\n" \
"        e: enable pwm\r\n" \
"        f: set freqency\r\n" \
"        d: disable pwm\r\n" \
"        ef/fe: set freqency and start pwm\r\n" \
"        t: test mode\r\n" \
"    -f: freqency for pwm controler\r\n" \
" EXAMPLE\r\n" \
"     [buildroot@root ~]#  ./buzzertool -m t\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  ./buzzertool -m e\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  ./buzzertool -m ef -f 3000\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  ./buzzertool -m d\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#\r\n" \
	);
}

int main(int argc, char** argv)
{
	int ch = 0;
	int freqency = 0;

	if (argc <= 1) {
		help();
		exit(-1);
	}

	while((ch = getopt(argc, argv, "d:m:f:h")) != -1)  
	{  
		switch(ch) {
			case 'd':
				bzero(gpio_info.dev, sizeof(gpio_info.dev));
				strcpy(gpio_info.dev, optarg); 
				break;
			case 'm':
				bzero(gpio_info.mode, sizeof(gpio_info.mode));
				strcpy(gpio_info.mode, optarg); 
				break;
			case 'f':
				bzero(gpio_info.freqency, sizeof(gpio_info.freqency));
				strcpy(gpio_info.freqency, optarg); 
				break;
			case 'h':
			default:
				help();
				exit(-1);
				break;
		}
	}  

	freqency = atoi(gpio_info.freqency);
	print("%s, %s, %d", gpio_info.dev, gpio_info.mode, freqency);

	if ((freqency > 8000) || (freqency < 0)) {
		printf("-1\r\n");
		exit(-1);
	}

	gpio_info.fd = open(gpio_info.dev, O_RDWR);
	if (gpio_info.fd == -1) {
		print("open device node error.");
		return -1;
	}

	if (strcmp(gpio_info.mode, "e") == 0) {
		ioctl(gpio_info.fd, BUZZER_ENABLE, 0);
	} else if (strcmp(gpio_info.mode, "f") == 0) {
		ioctl(gpio_info.fd, BUZZER_FREQENCY, freqency);
	} else if ((strcmp(gpio_info.mode, "ef") == 0) 
			|| (strcmp(gpio_info.mode, "fe") == 0)) {
		ioctl(gpio_info.fd, BUZZER_ENABLE, 0);
		ioctl(gpio_info.fd, BUZZER_FREQENCY, freqency);
	} else if (strcmp(gpio_info.mode, "d") == 0) {
		ioctl(gpio_info.fd, BUZZER_DISABLE, 0);
	} else if (strcmp(gpio_info.mode, "t") == 0) {
		ioctl(gpio_info.fd, BUZZER_ENABLE, 0);
		ioctl(gpio_info.fd, BUZZER_FREQENCY, freqency);
		usleep(100000);
		ioctl(gpio_info.fd, BUZZER_FREQENCY, freqency / 4 * (4 + 1));
		usleep(100000);
		ioctl(gpio_info.fd, BUZZER_FREQENCY, freqency / 4 * (4 + 2));
		usleep(100000);
		ioctl(gpio_info.fd, BUZZER_DISABLE, 0);
	}

	printf("0\r\n");
	
	close( gpio_info.fd );

	return 0;
}
