#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>       
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <termios.h>

#include <raylib.h>

#define FPS 60
#define WIDTH 800
#define HEIGHT 600
#define MAX_RANGE 3.5f

#define DEFER(x, ...)			\
do {					\
    if (x) {				\
        fprintf(stderr, __VA_ARGS__);	\
	return 0;			\
    }					\
} while (0)

void Draw_radar(float dis)
{
    char Todraw[10];
    sprintf(Todraw, "%.3f m", dis);
    
    BeginDrawing();
    DrawText(Todraw, WIDTH/2, HEIGHT/2+200+25, 20, GRAY);
    ClearBackground(BLACK);

    for (int i = 0; i < 8; i++) {
	DrawCircleSectorLines((Vector2){WIDTH/2, HEIGHT/2+200}, i*50, -180, 0, 1, GREEN);
	DrawLine(WIDTH/2, HEIGHT/2+200, (50*7)*cos(i*M_PI*(1.f/7.f) + M_PI)+WIDTH/2, (50*7)*sin(i*M_PI*(1.f/7.f) + M_PI)+HEIGHT/2+200, GREEN);
    }

    DrawCircle(WIDTH/2, HEIGHT/2+200-(7*50)*(dis/MAX_RANGE), 5, RED);
    DrawFPS(20, 20);
    EndDrawing();
}

void ttyInit(int fd)
{
    struct termios tty;
    tcgetattr(fd, &tty);

    cfmakeraw(&tty);
    if (cfsetospeed(&tty, B1000000) < 0 || cfsetispeed(&tty, B1000000) < 0) {
	fprintf(stderr, "ERROR: Can not set the bauderate\n");
	exit(-1);
    }
    tty.c_cc[VMIN]  = 100;
    tty.c_cc[VTIME] = 2; // 2 seconde
    tcsetattr(fd, TCSANOW, &tty);
}

int main(int argc, char *argv[])
{
    argc--;argv++;
    DEFER(argc < 1, "ERROR: Filepath argument missing\n");
    
    int fd = open(argv[0], O_RDWR | O_NOCTTY | O_NONBLOCK);
    DEFER(fd < 0, "ERROR: Can not open the file: %s\n", argv[0]);

    ttyInit(fd);
    
    char buf[10];
    int n = 0;
    InitWindow(WIDTH, HEIGHT, "Radar");
    SetTargetFPS(FPS);
    while (!WindowShouldClose()) {
	n = read(fd, buf, sizeof(buf));
	// if (n > 0) write(STDOUT_FILENO, buf, n);

	float distance = atof(buf);
	if (distance > MAX_RANGE) distance = MAX_RANGE;

	Draw_radar(distance);
    }
    CloseWindow();
    close(fd);
    
    return 0;
}
