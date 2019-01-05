#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME "event"
#define DEV_FB "/dev"
#define FB_DEV_NAME "fb"

#include "RTIMULib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include <linux/input.h>
#include <linux/fb.h>

enum direction_t{
	UP,
	RIGHT,
	DOWN,
	LEFT,
	NONE,
};
struct segment_t {
	struct segment_t *next;
	int x;
	int y;
};
struct snake_t {
	struct segment_t head;
	struct segment_t *tail;
	enum direction_t heading;
};
struct apple_t {
	int x;
	int y;
};

struct fb_t {
	uint16_t pixel[8][8];
};

int running = 1;

struct snake_t snake = {
	{NULL, 4, 4},
	NULL,
	NONE,
};
struct apple_t apple = {
	4, 4,
};

struct fb_t *fb;

RTVector3 lrtv;

static int is_event_device(const struct dirent *dir)
{
	return strncmp(EVENT_DEV_NAME, dir->d_name,
		       strlen(EVENT_DEV_NAME)-1) == 0;
}
static int is_framebuffer_device(const struct dirent *dir)
{
	return strncmp(FB_DEV_NAME, dir->d_name,
		       strlen(FB_DEV_NAME)-1) == 0;
}

static int open_evdev(const char *dev_name)
{
	struct dirent **namelist;
	int i, ndev;
	int fd = -1;

	ndev = scandir(DEV_INPUT_EVENT, &namelist, is_event_device, versionsort);
	if (ndev <= 0)
		return ndev;

	for (i = 0; i < ndev; i++)
	{
		char fname[64];
		char name[256];

		snprintf(fname, sizeof(fname),
			 "%s/%s", DEV_INPUT_EVENT, namelist[i]->d_name);
		fd = open(fname, O_RDONLY);
		if (fd < 0)
			continue;
		ioctl(fd, EVIOCGNAME(sizeof(name)), name);
		if (strcmp(dev_name, name) == 0)
			break;
		close(fd);
	}

	for (i = 0; i < ndev; i++)
		free(namelist[i]);

	return fd;
}

static int open_fbdev(const char *dev_name)
{
	struct dirent **namelist;
	int i, ndev;
	int fd = -1;
	struct fb_fix_screeninfo fix_info;

	ndev = scandir(DEV_FB, &namelist, is_framebuffer_device, versionsort);
	if (ndev <= 0)
		return ndev;

	for (i = 0; i < ndev; i++)
	{
		char fname[64];
		char name[256];

		snprintf(fname, sizeof(fname),
			 "%s/%s", DEV_FB, namelist[i]->d_name);
		fd = open(fname, O_RDWR);
		if (fd < 0)
			continue;
		ioctl(fd, FBIOGET_FSCREENINFO, &fix_info);
		if (strcmp(dev_name, fix_info.id) == 0)
			break;
		close(fd);
		fd = -1;
	}
	for (i = 0; i < ndev; i++)
		free(namelist[i]);

	return fd;
}

void render()
{
	struct segment_t *seg_i;
	memset(fb, 0, 128);
	fb->pixel[apple.x][apple.y]=0xF800;
	for(seg_i = snake.tail; seg_i->next; seg_i=seg_i->next) {
		fb->pixel[seg_i->x][seg_i->y] = 0x7E0;
	}
	fb->pixel[seg_i->x][seg_i->y]=0xFFFF;
}

int check_collision(int appleCheck)
{
	struct segment_t *seg_i;

	if (appleCheck) {
		for (seg_i = snake.tail; seg_i; seg_i=seg_i->next) {
			if (seg_i->x == apple.x && seg_i->y == apple.y)
				return 1;
			}
		return 0;
	}

	for(seg_i = snake.tail; seg_i->next; seg_i=seg_i->next) {
		if (snake.head.x == seg_i->x && snake.head.y == seg_i->y)
			return 1;
	}

	if (snake.head.x < 0 || snake.head.x > 7 ||
	    snake.head.y < 0 || snake.head.y > 7) {
		return 1;
	}
	return 0;
}

void game_logic(void)
{
	struct segment_t *seg_i;
	struct segment_t *new_tail;
	for(seg_i = snake.tail; seg_i->next; seg_i=seg_i->next) {
		seg_i->x = seg_i->next->x;
		seg_i->y = seg_i->next->y;
	}
	if (check_collision(1)) {
		new_tail = (segment_t*)malloc(sizeof(struct segment_t));
		if (!new_tail) {
			printf("Ran out of memory.\n");
			running = 0;
			return;
		}
		new_tail->x=snake.tail->x;
		new_tail->y=snake.tail->y;
		new_tail->next=snake.tail;
		snake.tail = new_tail;

		while (check_collision(1)) {
			apple.x = rand() % 8;
			apple.y = rand() % 8;
		}
	}
	switch (snake.heading) {
		case LEFT:
			seg_i->y--;
			break;
		case DOWN:
			seg_i->x++;
			break;
		case RIGHT:
			seg_i->y++;
			break;
		case UP:
			seg_i->x--;
			break;
	}
}

void reset(void)
{
	struct segment_t *seg_i;
	struct segment_t *next_tail;
	seg_i=snake.tail;
	while (seg_i->next) {
		next_tail=seg_i->next;
		free(seg_i);
		seg_i=next_tail;
	}
	snake.tail=seg_i;
	snake.tail->next=NULL;
	snake.tail->x=2;
	snake.tail->y=3;
	apple.x = rand() % 8;
	apple.y = rand() % 8;
	snake.heading = NONE;
}

void change_dir(unsigned int code)
{
	switch (code) {
		case 1:
			if (snake.heading != DOWN)
				snake.heading = UP;
			break;
		case 2:
			if (snake.heading != LEFT)
				snake.heading = RIGHT;
			break;
		case 3:
			if (snake.heading != UP)
				snake.heading = DOWN;
			break;
		case 4:
			if (snake.heading != RIGHT)
				snake.heading = LEFT;
			break;
	}
}

void handle_events(RTVector3 &rtv)
{
	if (fabs(rtv.x() * RTMATH_RAD_TO_DEGREE - lrtv.x() * RTMATH_RAD_TO_DEGREE) > 
	fabs(rtv.y() * RTMATH_RAD_TO_DEGREE - lrtv.y() * RTMATH_RAD_TO_DEGREE)) { //if roll > pitch 
		if (fabs(rtv.x() * RTMATH_RAD_TO_DEGREE - lrtv.x() * RTMATH_RAD_TO_DEGREE) > 2.5) {
			if (rtv.x() * RTMATH_RAD_TO_DEGREE < 0) {
				change_dir(1); //go up
				printf("up\n");
			}
			else {
				change_dir(3); //go down
				printf("down\n");
			}
		}
	}
	else {
		if (fabs(rtv.y() * RTMATH_RAD_TO_DEGREE - lrtv.y() * RTMATH_RAD_TO_DEGREE) > 2.5) {
			if (rtv.y() * RTMATH_RAD_TO_DEGREE < 0) {
				change_dir(2); //go right
				printf("right\n");
			}
			else {
				change_dir(4); //go left
				printf("left\n");
			}
		}
	}

	lrtv = rtv;
}

int main(int argc, char* args[])
{
    	RTIMUSettings *settings = new RTIMUSettings("RTIMULib"); //using RTIMULib here allows it to use the .ini file generated by RTIMULibDemo.

    	RTIMU *imu = RTIMU::createIMU(settings);

	if ((imu == NULL) || (imu->IMUType() == RTIMU_TYPE_NULL)) {
       		printf("No IMU found\n");
        	exit(1);
    	}
	
    	imu->IMUInit();
	imu->setSlerpPower(0.02);
    	imu->setGyroEnable(true);
    	imu->setAccelEnable(true);
    	imu->setCompassEnable(false);

	int ret = 0;
	int fbfd = 0;
	
	srand (time(NULL));

	fbfd = open_fbdev("RPi-Sense FB");
	if (fbfd <= 0) {
		ret = fbfd;
		printf("Error: cannot open framebuffer device.\n");
		goto err_ev; 
	}
	
	fb = (fb_t*)mmap(0, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (!fb) {
		ret = EXIT_FAILURE;
		printf("Failed to mmap.\n");
		goto err_fb;
	}
	memset(fb, 0, 128);

	snake.tail = &snake.head;
	reset();
	while (running) {
		usleep(imu->IMUGetPollInterval() * 1000);	//poll at the rate recommended by the IMU
		while (imu->IMURead()) {
			RTIMU_DATA imuData = imu->getIMUData();
			handle_events(imuData.fusionPose);
			printf("%s\n",RTMath::displayDegrees("", imuData.fusionPose));
		}
		game_logic();
		if (check_collision(0)) {
			reset();
		}
		render();
		usleep (300000);
	}
	memset(fb, 0, 128);
	reset();
	munmap(fb, 128);
err_fb:
	close(fbfd);
err_ev:
	return ret;
}
