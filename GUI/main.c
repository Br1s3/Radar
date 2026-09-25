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
#define MAX_ANGLE 17

#define DEFER(x, ...)			\
do {					\
    if (x) {				\
        fprintf(stderr, __VA_ARGS__);	\
	return 0;			\
    }					\
} while (0)

void Draw_radar(int pos, float *dis)
{
    char Todraw[] = "XXXXXXXXXX";
    int err = 0;
    if (pos < 0 || pos > 17) {
	err = 1;
	sprintf(Todraw, "ERR: Pos ");
    }
    // sprintf(Todraw, "%.3f m", dis);
    static const float midH = HEIGHT/2.f;
    static const float midW = WIDTH/2.f;
    
    BeginDrawing();
    if (err == 1)
        DrawText(Todraw, midW-50, midH+200+25, 20, GRAY);
    ClearBackground(BLACK);

    for (int i = 0; i < 8; i++) {
	DrawCircleSectorLines((Vector2){midW, midH+200}, i*50, -180, 0, 1, GREEN);
	DrawLine(midW, midH+200, (50*7)*cosf(i*M_PI*(1.f/7.f) + M_PI)+midW, (50*7)*sinf(i*M_PI*(1.f/7.f) + M_PI)+midH+200, GREEN);
    }

    for (int i = 0; i < 18; i++) {
	DrawCircle(midW + (50.f*7.f)*(dis[i]/MAX_RANGE)*(cosf((float)i/17.f*M_PI)), midH + (50.f*7.f)*(dis[i]/MAX_RANGE)*(-sinf((float)i/17.f*M_PI))+200, 5, RED);
    }
    
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

    char raw_data[11];
    char str_pos[] = "00,";
    char str_distance[] = "0.000\n\r\0";
    int n = 0;
    float Mdistance[17] = {0.f};
    float distance = 0;
    unsigned int bufpos = 0;
    unsigned int rep = 1;
    
    InitWindow(WIDTH, HEIGHT, "Radar");
    SetTargetFPS(FPS);
    while (!WindowShouldClose()) {
	n = read(fd, raw_data, sizeof(raw_data));
	if (n > 0) {
	    sprintf(str_pos, "%.*s", 2, raw_data);
	    sprintf(str_distance, "%.*s", 8, raw_data+3);
	}

	distance = atof(str_distance);
	if (distance > MAX_RANGE) distance = MAX_RANGE;

	unsigned int position = atoi(str_pos);
	if (position > MAX_ANGLE)
	    distance = 0;

	Mdistance[position] = (Mdistance[position] * rep + distance)/(rep+1);
	// Make the average without knowing the number of repetition

	if (bufpos == position) rep++;
	else rep = 1;
	
	Draw_radar(position, Mdistance);
	bufpos = position;
    }
    CloseWindow();
    close(fd);

    return 0;
}
