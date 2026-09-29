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
#define MAX_ANGLE 2250
#define GAP_ANGLE 25

#define DEFER(x, ...)			\
do {					\
    if (x) {				\
        fprintf(stderr, __VA_ARGS__);	\
	return 0;			\
    }					\
} while (0)

void Draw_radar(int pos, float *dis)
{
    const char ErrAngle[] = "ERROR ANGLE OUT OF RANGE";
    char DrawAngle[] = "XXXXXXXXXXXXXX";
    char DrawRange[] = "XXXXXXXXXXXXXX";
    int err = 0;
    if (pos < 0 || pos > 90) {
	err = 1;
    }
    else {
	// sprintf(DrawAngle, "ANGLE: %.2f%c", ((float)pos/(float)MAX_ANGLE) * 90.f, '°');
	snprintf(DrawAngle, sizeof(DrawAngle), "ANGLE: %d°", pos);
	snprintf(DrawRange, sizeof(DrawRange), "RANGE: %.3f", dis[pos]);
    }

    static const float midH = HEIGHT/2.f;
    static const float midW = WIDTH/2.f;
    
    ClearBackground(BLACK);

    if (err == 1) DrawText(ErrAngle, midW-150, midH+225, 20, GRAY);
    DrawText(DrawAngle, 13, 100, 20, GRAY);
    DrawText(DrawRange, 13, 125, 20, GRAY);

    for (int i = 0; i < 8; i++) {
	DrawCircleSectorLines((Vector2){midW, midH+200}, i*50, -180, 0, 1, GREEN);
	DrawLine(midW, midH+200, (50*7)*cosf(i*M_PI*(1.f/7.f) + M_PI)+midW, (50*7)*sinf(i*M_PI*(1.f/7.f) + M_PI)+midH+200, GREEN);
    }


    for (int i = 0; i < 90; i++) {
	if (i == pos) continue;
	DrawCircle(midW + (50.f*7.f)*(dis[i]/MAX_RANGE)*(cosf((float)i/90.f*M_PI)), midH + (50.f*7.f)*(dis[i]/MAX_RANGE)*(-sinf((float)i/90.f*M_PI))+200, 4, DARKBLUE);
    }
    DrawCircle(midW + (50.f*7.f)*(dis[pos]/MAX_RANGE)*(cosf((float)pos/90.f*M_PI)), midH + (50.f*7.f)*(dis[pos]/MAX_RANGE)*(-sinf((float)pos/90.f*M_PI))+200, 5, RED);
    
    DrawFPS(20, 20);
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

    struct termios tty;
    tcgetattr(fd, &tty);

    ttyInit(fd);

    char raw_data[13];
    char str_pos[] = "0000,";
    char str_distance[] = "0.000\n\r";
    float Mdistance[90] = {0.f};
    float distance = 0;
    unsigned int bufpos = 0;
    unsigned int position = 0;
    
    InitWindow(WIDTH, HEIGHT, "Radar");
    SetTargetFPS(FPS);
    while (!WindowShouldClose()) {
	int n = read(fd, raw_data, sizeof(raw_data));
	if (n == 13) {
	    sprintf(str_pos, "%.*s", 4, raw_data);
	    sprintf(str_distance, "%.*s", 5, raw_data+sizeof(str_pos)-1);

	    distance = atof(str_distance);
	    if (distance > MAX_RANGE) distance = MAX_RANGE;

	    position = atoi(str_pos) / 25;
	    if (position > MAX_ANGLE) position = bufpos;
	}

	Mdistance[position] = distance;
	
	BeginDrawing();
	Draw_radar(position, Mdistance);
	EndDrawing();
	bufpos = position;
    }
    CloseWindow();
    tcsetattr(fd, TCSANOW, &tty);
    close(fd);

    return 0;
}
