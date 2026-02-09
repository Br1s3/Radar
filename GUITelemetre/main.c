#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <fcntl.h>       
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>

#include <raylib.h>

#define FPS 60
#define WIDTH 800
#define HEIGHT 600
#define MAX_RANGE 3.5f

void ttyInit(int fd, struct termios *tty)
{
    tcgetattr(fd, tty);
    tty->c_iflag = IGNPAR;
    tty->c_cflag |= (CLOCAL | CREAD);
    tty->c_cflag &= ~(CSIZE | PARENB);
    tty->c_cflag |= CS8;
    tty->c_cflag |= CRTSCTS;
    if (cfsetospeed(tty, B1000000) < 0 || cfsetispeed(tty, B1000000) < 0) {
	fprintf(stderr, "ERROR: Can not set the bauderate\n");
	exit(-1);
    }
    tcsetattr(fd, TCSANOW, tty);
}

int main(int argc, char *argv[])
{
    argc--;argv++;
    if (argc < 1) {
	fprintf(stdout, "ERROR: Filepath argument missing\n");
	return 0;
    }
    
    int fd = open(argv[0], O_RDONLY | O_NOCTTY | O_NONBLOCK);
    if (fd < 0) {
	fprintf(stderr, "ERROR: Can not open the file: %s", argv[0]);
	return 0;
    }
    
    struct termios tty;
    ttyInit(fd, &tty);
    
    char buf[10];
    int n = 0;
    InitWindow(WIDTH, HEIGHT, "Radar");
    SetTargetFPS(FPS);
    while (!WindowShouldClose()) {
	n = read(fd, buf, sizeof(buf));
	// if (n > 0) write(1, buf, n);
	float distance = atof(buf);
	if (distance > MAX_RANGE) distance = MAX_RANGE;
	BeginDrawing();
	char Todraw[10];
	sprintf(Todraw, "%.3f m", distance);
	DrawText(Todraw, WIDTH/2, HEIGHT/2+200+25, 20, GRAY);
	ClearBackground(BLACK);
	for (int i = 0; i < 8; i++) {
	    DrawCircleSectorLines((Vector2){WIDTH/2, HEIGHT/2+200}, i*50, -180, 0, 1, GREEN);
	    DrawLine(WIDTH/2, HEIGHT/2+200, (50*7)*cos(i*M_PI*(1.f/7.f) + M_PI)+WIDTH/2, (50*7)*sin(i*M_PI*(1.f/7.f) + M_PI)+HEIGHT/2+200, GREEN);
	}

	DrawCircle(WIDTH/2, HEIGHT/2+200-(7*50)*(distance/MAX_RANGE), 5, RED);
	DrawFPS(20, 20);
	EndDrawing();
    }
    
    close(fd);
    return 0;
}
