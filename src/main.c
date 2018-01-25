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
"USAGE\r\n" \
"    -d: device node, default is /dev/gpio_mio\r\n" \
"    -m: \r\n" \
"        i: input\r\n" \
"        o: output\r\n" \
"    -i: index\r\n" \
"    -p: part, this is hardware mode\r\n" \
"        m: mio\r\n" \
"    -v: value, this just for output mode\r\n" \
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

	while((ch = getopt(argc, argv, "d:m:i:p:v:")) != -1)  
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
			gpio_info.value = ioctl(gpio_info.fd, part_mio[gpio_info.index]);
			printf( "%d", gpio_info.value);
		} else if (gpio_info.mode == 'o') {
			ioctl(gpio_info.fd, part_mio[gpio_info.index + 4], gpio_info.value);
			printf( "0");
		} else if (gpio_info.mode == 't') {
			for (i = 0; i < (sizeof(part_mio)/sizeof(part_mio[0]) / 2) ; i++) {
				for (j = 0; j < 2; j++) {
					ioctl(gpio_info.fd, part_mio[i + 4], j % 2);
					usleep(10000);
					gpio_info.value = ioctl(gpio_info.fd, part_mio[i]);
					
					if (gpio_info.value != (j % 2)) {
						printf("-1");
						exit(-1);
					}
				}

			}
		}
	}
	
	close( gpio_info.fd );

	return 0;
}
