#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define IMX_GPIO_NR(bank, nr)		(((bank) - 1) * 32 + (nr))


// #define DEBUG
#ifdef DEBUG
    #define print(format, ...) printf(format"\r\n", ##__VA_ARGS__);
#else
    #define print(format, ...)
#endif

#define IMX_GPIO_NR(bank, nr)         (((bank) - 1) * 32 + (nr))

#define GPIO_IN0_mio                  IMX_GPIO_NR(1, 15)
#define GPIO_IN1_mio                  IMX_GPIO_NR(1, 14)
#define GPIO_IN2_mio                  IMX_GPIO_NR(1, 13)
#define GPIO_IN3_mio                  IMX_GPIO_NR(1, 12)
#define GPIO_OUT0_mio                 IMX_GPIO_NR(2, 4)
#define GPIO_OUT1_mio                 IMX_GPIO_NR(2, 5)
#define GPIO_OUT2_mio                 IMX_GPIO_NR(2, 6)
#define GPIO_OUT3_mio                 IMX_GPIO_NR(2, 7)

#define GPIO_IN0_CMD_mio			66
#define GPIO_IN1_CMD_mio			67
#define GPIO_IN2_CMD_mio			68
#define GPIO_IN3_CMD_mio			69
#define GPIO_OUT0_CMD_mio			70
#define GPIO_OUT1_CMD_mio			71
#define GPIO_OUT2_CMD_mio			72
#define GPIO_OUT3_CMD_mio			73

#define GPIO_CAN_TX_CON2			7
#define GPIO_CAN_RX_CON2			8

char part_MIO_GPIO[8] = {
	GPIO_IN0_mio,
	GPIO_IN1_mio,
	GPIO_IN2_mio,
	GPIO_IN3_mio,
	GPIO_OUT0_mio,
	GPIO_OUT1_mio,
	GPIO_OUT2_mio,
	GPIO_OUT3_mio,
};

char part_MIO_I2C3[2] = {
	IMX_GPIO_NR(1, 3),
	IMX_GPIO_NR(1, 6),
};

char part_CON2_CAN[2] = {
	GPIO_CAN_TX_CON2,
	GPIO_CAN_RX_CON2,
};

char part_CON2_I2C2[2] = {
	IMX_GPIO_NR(4, 12),
	IMX_GPIO_NR(4, 13),
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

int gpio_export(unsigned int gpio)
{
	int fd, len;
	char str_gpio[4];
	struct gpio_exp *new_gpio, *g;

	if ((fd = open("/sys/class/gpio/export", O_WRONLY)) < 0)
	{
		return -1;
	}
	len = snprintf(str_gpio, sizeof(str_gpio), "%d", gpio);
	write(fd, str_gpio, len);
	close(fd);

	return 0;
}

int gpio_unexport(unsigned int gpio)
{
	int fd, len;
	char str_gpio[4];
	struct gpio_exp *g, *temp, *prev_g = NULL;

	if ((fd = open("/sys/class/gpio/unexport", O_WRONLY)) < 0)
		return -1;

	len = snprintf(str_gpio, sizeof(str_gpio), "%d", gpio);
	write(fd, str_gpio, len);
	close(fd);

	return 0;
}

int gpio_set_direction(unsigned int gpio, unsigned int in_flag)
{
	int fd;
	char filename[34];
	
	snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/direction", gpio);
	if ((fd = open(filename, O_WRONLY)) < 0)
		return -1;
	
	if (in_flag)
		write(fd, "in", 3);
	else
		write(fd, "out", 4);
	
	close(fd);
	return 0;
}

int gpio_set_value(unsigned int gpio, unsigned int value)
{
	int fd;
	char filename[34];

	snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/value", gpio);
	print("%s", filename);
	if ((fd = open(filename, O_WRONLY)) < 0)
		return -1;

	if (value)
		write(fd, "1", 2);
	else
		write(fd, "0", 2);

	close(fd);

	return 0;
}

int gpio_get_value(unsigned int gpio)
{
	int fd;
	char filename[34];
	char val_buf[2] = {0};

	snprintf(filename, sizeof(filename), "/sys/class/gpio/gpio%d/value", gpio);
	print("%s", filename);
	if ((fd = open(filename, O_RDONLY)) < 0)
		return -1;

	read(fd, val_buf, 1);

	close(fd);

	return atoi(val_buf);
}

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
"        mio: mio connector\r\n" \
"        can1: can1 connector\r\n" \
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
"     [buildroot@root ~]#  ./gpiotool -p can1 -m t\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  ./gpiotool -p can1 -m i -i 1\r\n" \
"     1\r\n" \
"     [buildroot@root ~]#  ./gpiotool -p can1 -m o -i 0 -v 0\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  ./gpiotool -p can1 -m i -i 1\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  ./gpiotool -p can1 -m o -i 0 -v 1\r\n" \
"     0\r\n" \
"     [buildroot@root ~]#  ./gpiotool -p can1 -m i -i 1\r\n" \
"     1\r\n" \
"     [buildroot@root ~]#\r\n" \
	);
}

int main(int argc, char** argv)
{
	int ch = 0;
	int i = 0;
	int j = 0;
	int ret = 0;

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
			if ((sizeof(part_MIO_GPIO)/sizeof(part_MIO_GPIO[0])) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_export(part_MIO_GPIO[gpio_info.index]);
			gpio_set_direction(part_MIO_GPIO[gpio_info.index], 1);
			gpio_info.value = gpio_get_value(part_MIO_GPIO[gpio_info.index]);
			gpio_unexport(part_MIO_GPIO[gpio_info.index]);
			printf( "%d\r\n", gpio_info.value);
		} else if (gpio_info.mode == 'o') {
			if ((sizeof(part_MIO_GPIO)/sizeof(part_MIO_GPIO[0])) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_export(part_MIO_GPIO[gpio_info.index]);
			gpio_set_direction(part_MIO_GPIO[gpio_info.index], 0);
			gpio_set_value(part_MIO_GPIO[gpio_info.index], gpio_info.value);
			gpio_unexport(part_MIO_GPIO[gpio_info.index]);
			printf( "0\r\n");
		} else if (gpio_info.mode == 't') {
			int times = (sizeof(part_MIO_GPIO)/sizeof(part_MIO_GPIO[0]) / 2);
			for (i = 0; i < (sizeof(part_MIO_GPIO)/sizeof(part_MIO_GPIO[0]) / 2) ; i++) {
				gpio_export(part_MIO_GPIO[i]);
				gpio_export(part_MIO_GPIO[i + times]);
				gpio_set_direction(part_MIO_GPIO[i], 0);
				gpio_set_direction(part_MIO_GPIO[i + times], 1);
				print("set gpio over");

				for (j = 0; j < 2; j++) {
					print("test");
					gpio_set_value(part_MIO_GPIO[i], j % 2);
					usleep(10000);
					ret = gpio_get_value(part_MIO_GPIO[i + times]);
					
					if (ret != (j % 2)) {
						printf("-1\r\n");
						exit(-1);
					}
				}
				gpio_export(part_MIO_GPIO[i]);
				gpio_export(part_MIO_GPIO[i + times]);

			}

			printf("0\r\n");
		}
	}

	if (strcmp(gpio_info.part, "can1") == 0) {
		print("can1 mode");
		if (gpio_info.mode == 'i') {
			if ((sizeof(part_CON2_CAN) / sizeof(part_CON2_CAN[0])) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_export(part_CON2_CAN[gpio_info.index]);
			gpio_set_direction(part_CON2_CAN[gpio_info.index], 1);
			gpio_info.value = gpio_get_value(part_CON2_CAN[gpio_info.index]);
			gpio_unexport(part_CON2_CAN[gpio_info.index]);
			printf( "%d\r\n", gpio_info.value);
		} else if (gpio_info.mode == 'o') {
			if ((sizeof(part_CON2_CAN) / sizeof(part_CON2_CAN[0])) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_export(part_CON2_CAN[gpio_info.index]);
			gpio_set_direction(part_CON2_CAN[gpio_info.index], 0);
			gpio_set_value(part_CON2_CAN[gpio_info.index], gpio_info.value);
			gpio_unexport(part_CON2_CAN[gpio_info.index]);
			printf( "0\r\n");
		} else if (gpio_info.mode == 't') {
			int times = (sizeof(part_CON2_CAN)/sizeof(part_CON2_CAN[0]) / 2);
			for (i = 0; i < times; i++) {

				gpio_export(part_CON2_CAN[i]);
				gpio_export(part_CON2_CAN[i] + times);
				gpio_set_direction(part_CON2_CAN[i], 0);
				gpio_set_direction(part_CON2_CAN[i] + times, 1);
				print("set gpio over");

				for (j = 0; j < 2; j++) {
					print("test");
					gpio_set_value(part_CON2_CAN[i], j % 2);
					usleep(10000);
					ret = gpio_get_value(part_CON2_CAN[i] + times);
					
					if (ret != (j % 2)) {
						printf("-1\r\n");
						exit(-1);
					}
				}

				gpio_export(part_CON2_CAN[i]);
				gpio_export(part_CON2_CAN[i] + times);
			}

			printf("0\r\n");
		}
	}

	if (strcmp(gpio_info.part, "i2c2") == 0) {
		print("i2c2 mode");
		if (gpio_info.mode == 'i') {
			if ((sizeof(part_CON2_I2C2) / sizeof(part_CON2_I2C2[0])) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_export(part_CON2_I2C2[gpio_info.index]);
			gpio_set_direction(part_CON2_I2C2[gpio_info.index], 1);
			gpio_info.value = gpio_get_value(part_CON2_I2C2[gpio_info.index]);
			gpio_unexport(part_CON2_I2C2[gpio_info.index]);
			printf( "%d\r\n", gpio_info.value);
		} else if (gpio_info.mode == 'o') {
			if ((sizeof(part_CON2_I2C2) / sizeof(part_CON2_I2C2[0])) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_export(part_CON2_I2C2[gpio_info.index]);
			gpio_set_direction(part_CON2_I2C2[gpio_info.index], 0);
			gpio_set_value(part_CON2_I2C2[gpio_info.index], gpio_info.value);
			gpio_unexport(part_CON2_I2C2[gpio_info.index]);
			printf( "0\r\n");
		} else if (gpio_info.mode == 't') {
			int times = (sizeof(part_CON2_I2C2)/sizeof(part_CON2_I2C2[0]) / 2);
			for (i = 0; i < times; i++) {

				gpio_export(part_CON2_I2C2[i]);
				gpio_export(part_CON2_I2C2[i + times]);
				gpio_set_direction(part_CON2_I2C2[i], 0);
				gpio_set_direction(part_CON2_I2C2[i + times], 1);
				print("set gpio over");

				for (j = 0; j < 2; j++) {
					print("test");
					gpio_set_value(part_CON2_I2C2[i], j % 2);
					usleep(10000);
					ret = gpio_get_value(part_CON2_I2C2[i + times]);
					
					if (ret != (j % 2)) {
						printf("-1\r\n");
						exit(-1);
					}
				}

				gpio_export(part_CON2_I2C2[i]);
				gpio_export(part_CON2_I2C2[i + times]);
			}

			printf("0\r\n");
		}
	}

	if (strcmp(gpio_info.part, "i2c3") == 0) {
		print("i2c3 mode");
		if (gpio_info.mode == 'i') {
			if ((sizeof(part_MIO_I2C3) / sizeof(part_MIO_I2C3[0])) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_export(part_MIO_I2C3[gpio_info.index]);
			gpio_set_direction(part_MIO_I2C3[gpio_info.index], 1);
			gpio_info.value = gpio_get_value(part_MIO_I2C3[gpio_info.index]);
			gpio_unexport(part_MIO_I2C3[gpio_info.index]);
			printf( "%d\r\n", gpio_info.value);
		} else if (gpio_info.mode == 'o') {
			if ((sizeof(part_MIO_I2C3) / sizeof(part_MIO_I2C3[0])) <= gpio_info.index)
				printf( "-1\r\n");

			gpio_export(part_MIO_I2C3[gpio_info.index]);
			gpio_set_direction(part_MIO_I2C3[gpio_info.index], 0);
			gpio_set_value(part_MIO_I2C3[gpio_info.index], gpio_info.value);
			gpio_unexport(part_MIO_I2C3[gpio_info.index]);
			printf( "0\r\n");
		} else if (gpio_info.mode == 't') {
			int times = (sizeof(part_MIO_I2C3)/sizeof(part_MIO_I2C3[0]) / 2);
			for (i = 0; i < times; i++) {

				gpio_export(part_MIO_I2C3[i]);
				gpio_export(part_MIO_I2C3[i + times]);
				gpio_set_direction(part_MIO_I2C3[i], 0);
				gpio_set_direction(part_MIO_I2C3[i + times], 1);
				print("set gpio over");

				for (j = 0; j < 2; j++) {
					print("test");
					gpio_set_value(part_MIO_I2C3[i], j % 2);
					print("set gpio value: %d", j % 2);
					usleep(10000);
					ret = gpio_get_value(part_MIO_I2C3[i + times]);
					print("get gpio value: %d", ret);
					
					if (ret != (j % 2)) {
						printf("-1\r\n");
						exit(-1);
					}
				}

				gpio_export(part_MIO_I2C3[i]);
				gpio_export(part_MIO_I2C3[i + times]);
			}

			printf("0\r\n");
		}
	}
	
	close( gpio_info.fd );

	return 0;
}
