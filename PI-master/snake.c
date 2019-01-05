#define _GNU_SOURCE
#define DEV_INPUT_EVENT "/dev/input"
#define EVENT_DEV_NAME "event"
#define DEV_FB "/dev"
#define FB_DEV_NAME "fb"

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
	//int pixel = rand() %1000000;
	memset(fb, 0, 128);
	fb->pixel[apple.x][apple.y]=0xF800;
	//fb->pixel[apple.x][apple.y]= pixel;
	for(seg_i = snake.tail; seg_i->next; seg_i=seg_i->next) {
		fb->pixel[seg_i->x][seg_i->y] = 0x7E0;
		//fb->pixel[seg_i->x][seg_i->y] = pixel;
	}
	fb->pixel[seg_i->x][seg_i->y]=0xFFFF;
	//fb->pixel[seg_i->x][seg_i->y]= pixel;
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
		new_tail = malloc(sizeof(struct segment_t));
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

void change_dir(int dir)
{
	printf("dir: %d\n", dir);
	printf("shx: %d\n", snake.head.x);
	printf("shy: %d\n", snake.head.y);
	printf("snake.heading: %d\n\n", snake.heading);
	switch (dir) {
		case 1:
				snake.heading = UP;
				printf("up\n");
			
			break;
		case 2:
				snake.heading = RIGHT;
				printf("right\n");
		
			break;
		case 3:

				snake.heading = DOWN;
				printf("down\n");
			
			break;
		case 4:

				snake.heading = LEFT;
				printf("left\n");
			
			break;
	}
}

int safe_move(int hx, int hy)
{
	struct segment_t *seg_i;
	for(seg_i = snake.tail; seg_i->next; seg_i=seg_i->next) {
		if (hx == seg_i->x && hy == seg_i->y) {
			printf("collision at x: %d, y: %d\n", hx, hy);
			return 0;
		}
	}
	if (hx < 0 || hx > 7 || hy < 0 || hy > 7) {
		return 0;
	}
	return 1;
}

int handle_events(int *p)
{
	// My position
	int hx = snake.head.x;

	int hy = snake.head.y;

	enum direction_t heading = snake.heading;

	int i = 0, flag = 0;

	while (hx != apple.x || hy != apple.y) 
	{			
		if (hx > apple.x) 
		{
			if (safe_move(hx -1, hy))
			{
				p[i++] = 1;
				--hx;
				heading = UP;
				printf("go up\n");
			}
			else {
				flag = 1; //invalidate
				break;Coll
			}
		}
		else if (hy < apple.y)
		{
			if (safe_move(hx, hy +1)) 
			{
				p[i++] = 2;
				++hy;
				heading = RIGHT;
				printf("go right\n");
			}
			else 
			{
				flag = 1; //invalidate
				break;
			}
		}
		else if (hx < apple.x) {
			if (safe_move(hx +1, hy)) {
				p[i++] = 3;
				++hx;
				heading = DOWN;
				printf("go down\n");
			}
			else {
				flag = 1; //invalidate
				break;
			}
		}
		else if (hy > apple.y) {
			if (safe_move(hx, hy -1)) {
				p[i++] = 4;
				--hy;
				heading = LEFT;
				printf("go left\n");
			}
			else {
				flag = 1; //invalidate
				break;
			}
		}
		else {
			flag = 1; //invalidate
			break;
		}	
	}

	//switch (heading) { 
	//	case UP:
	//		if (hx == 0 || !safe_move(hx -1, hy)) {  //avoid wall if apple on wall
	//			if (safe_move(hx, hy +1) && hy < 4) {
	//				p[i++] = 2;
	//				++hy;
	//				heading = RIGHT;
	//				printf("go right\n");
	//			}
	//			else if (safe_move(hx, hy -1) && hy > 3) {
	//				p[i++] = 4;
	//				--hy;
	//				heading = LEFT;
	//				printf("go left\n");
	//			}
	//		}
	//		break;
	//	case RIGHT:
	//		if (hy == 7 || !safe_move(hx, hy +1)) {
	//			if (safe_move(hx -1, hy) && hx > 3) {
	//				p[i++] = 1;
	//				--hx;
	//				heading = UP;
	//				printf("go up\n");
	//			}
	//			else if (safe_move(hx +1, hy) && hx < 4) {
	//				p[i++] = 3;
	//				++hx;
	//				heading = DOWN;
	//				printf("go down\n");
	//			}
	//		}
	//		break;
	//	case DOWN:
	//		if (hx == 7 || !safe_move(hx +1, hy)) {
	//			if (safe_move(hx, hy +1) && hy < 4) {
	//				p[i++] = 2;
	//				++hy;
	//				heading = RIGHT;
	//				printf("go right\n");
	//			}
	//			else if (safe_move(hx, hy -1) && hy > 3) {
	//				p[i++] = 4;
	//				--hy;
	//				heading = LEFT;
	//				printf("go left\n");
	//			}
	//		}
	//		break;
	//	case LEFT:
	//		if (hy == 0 || !safe_move(hx, hy -1)) {
	//			if (safe_move(hx -1, hy) && hx > 3) {
	//				p[i++] = 1;
	//				--hx;
	//				heading = UP;
	//				printf("go up\n");
	//			}
	//			else if (safe_move(hx +1, hy) && hx < 4) {
	//				p[i++] = 3;
	//				++hx;
	//				heading = DOWN;
	//				printf("go down\n");
	//			}
	//		}
	//		break;
	//	}
	return i;
}

int main(int argc, char* args[])
{
	int ret = 0;
	int fbfd = 0;
	int path[64];
	int count = 0;
	int i = 0;

	srand(time(NULL));

	fbfd = open_fbdev("RPi-Sense FB");
	if (fbfd <= 0) {
		ret = fbfd;
		printf("Error: cannot open framebuffer device.\n");
		goto err_ev; 
	}
	
	fb = mmap(0, 128, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
	if (!fb) {
		ret = EXIT_FAILURE;
		printf("Failed to mmap.\n");
		goto err_fb;
	}
	memset(fb, 0, 128);

	snake.tail = &snake.head;
	reset();
	while (running) {
		if (path[i] == 0) {
			i = 0;
			count = handle_events(path);
		}
		printf("i: %d\n", i);
		printf("count: %d\n", count);
		printf("path[i]: %d\n", path[i]);
		if (count != 0)
			change_dir(path[i++]);
		game_logic();
		if (check_collision(0))
			reset();
		render();
		if (i == count) 
			path[i] = 0;
		usleep (250000);
	}
	memset(fb, 0, 128);
	reset();
	munmap(fb, 128);
err_fb:
	close(fbfd);
err_ev:
	return ret;
}
 
