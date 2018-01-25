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

#define GPIO_IN0_CMD_mio			66
#define GPIO_IN1_CMD_mio			67
#define GPIO_IN2_CMD_mio			68
#define GPIO_IN3_CMD_mio			69
#define GPIO_OUT0_CMD_mio			70
#define GPIO_OUT1_CMD_mio			71
#define GPIO_OUT2_CMD_mio			72
#define GPIO_OUT3_CMD_mio			73

char part_mio[8] = {
	GPIO_IN0_CMD_mio,
	GPIO_IN1_CMD_mio,
	GPIO_IN2_CMD_mio,
	GPIO_IN3_CMD_mio,
	GPIO_OUT0_CMD_mio,
	GPIO_OUT1_CMD_mio,
	GPIO_OUT2_CMD_mio,
	GPIO_OUT3_CMD_mio,
};

struct GPIO_INFO {
	char dev[32];
	char mode;
	char index;
	char part[16];
	char value;
	int fd;
};
struct GPIO_INFO gpio_info = {
"/dev/gpio_mio",
'i',
0,
"mio",
0,
0
};

void help(void)
{
	printf( \
" USAGE\r\n" \
"    -d: device node, default is /dev/gpio_mio\r\n" \
"    -m: mode\r\n" \
"        i: input mode\r\n" \
"        o: output mode\r\n" \
"        t: test mode\r\n" \
"    -i: pin index\r\n" \
"    -p: part, this is hardware interface\r\n" \
"        default: mio\r\n" \
"    -v: value, this just for output mode\r\n" \
" RETURE:\r\n" \
"     I. input mode\r\n" \
"         1: pin is high\r\n" \
"         0: pin is low\r\n" \
"         -1: pin index error\r\n" \
"     II: output mode\r\n" \
"         0: operation is success\r\n" \
"         -1: pin index error\r\n" \
"     III: test mode\r\n" \
"         0: operation is success\r\n" \
"         -1: operation is failed\r\n" \
" EXAMPLE:\r\n" \
"     [buildroot@root ~]#  gpiotool -m t\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  gpiotool -m i -i 0\r\n" \
"     1\r\n" \
"     [buildroot@root ~]#  gpiotool -m o -i 0 -v 0\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  gpiotool -m i -i 0\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  gpiotool -m o -i 0 -v 1\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  gpiotool -m i -i 0\r\n" \
"     1\r\n" \
"     [buildroot@root ~]#\r\n" \
	);
}

int main(int argc, char** argv)
{
	int ch = 0;
	int i = 0;
	int j = 0;

	if (argc <= 1) {
		help();
		exit(-1);
	}

	while((ch = getopt(argc, argv, "d:m:i:p:v:h")) != -1)  
	{  
		switch(ch) {
			case 'd':
				bzero(gpio_info.dev, sizeof(gpio_info.dev));
				strcpy(gpio_info.dev, optarg); 
				break;
			case 'm':
				gpio_info.mode = *optarg;
				break;
			case 'i':
				gpio_info.index = *optarg - '0';
				break;
			case 'p':
				bzero(gpio_info.part, sizeof(gpio_info.part));
				strcpy(gpio_info.part, optarg); 
				break;
			case 'v':
				gpio_info.value = *optarg - '0';
				break;
			case 'h':
			default:
				help();
				exit(-1);
				break;
		}
	}  

	print("%s, %c, %d, %s, %d", gpio_info.dev, gpio_info.mode, gpio_info.index, gpio_info.part, gpio_info.value);

	gpio_info.fd = open(gpio_info.dev, O_RDWR);
	if (gpio_info.fd == -1)
	{
		print("open device node error.");
		return -1;
	}
	
	if (strcmp(gpio_info.part, "mio") == 0) {
		if (gpio_info.mode == 'i') {
			if ((sizeof(part_mio)/sizeof(part_mio[0]) / 2) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_info.value = ioctl(gpio_info.fd, part_mio[gpio_info.index]);
			printf( "%d\r\n", gpio_info.value);
		} else if (gpio_info.mode == 'o') {
			if ((sizeof(part_mio)/sizeof(part_mio[0]) / 2) <= gpio_info.index)
				printf( "-1\r\n");

			ioctl(gpio_info.fd, part_mio[gpio_info.index + 4], gpio_info.value);
			printf( "0\r\n");
		} else if (gpio_info.mode == 't') {
			for (i = 0; i < (sizeof(part_mio)/sizeof(part_mio[0]) / 2) ; i++) {
				for (j = 0; j < 2; j++) {
					ioctl(gpio_info.fd, part_mio[i + 4], j % 2);
					usleep(10000);
					gpio_info.value = ioctl(gpio_info.fd, part_mio[i]);
					
					if (gpio_info.value != (j % 2)) {
						printf("-1\r\n");
						exit(-1);
					}
				}

			}

			printf("0\r\n");
		}
	}
	
	close( gpio_info.fd );

	return 0;
}
