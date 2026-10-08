#include <sys/types.h>  // Used for: open()
#include <sys/stat.h>   // Used for: open()
#include <fcntl.h>      // Used for: open()
#include <unistd.h>     // Used for: tcgetattr(), tcsetattr(), cfmakeraw(), cfsetospeed(), close()
#include <stdio.h>      // Used for: fprintf(), snprintf()
#include <stdlib.h>     // Used for: atof(), exit()
#include <math.h>       // Used for: M_PI, sinf(), cosf()
#include <termios.h>    // Used for: tcgetattr(), tcsetattr(), cfmakeraw(), cfsetospeed()

#include <raylib.h>     // Used for: DARKBLUE, RED, GREEN, BLUE, GRAY, BLACK, DrawText(), DrawCircle(), ClearBackground(), DrawFPS(), BeginDrawing(), EndDrawing(), CloseWindow(), DrawCircleSectorLines()

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

void Draw_radar_BG(int pos, double *dis, const double midW, const double midH, const int BGSR, const int BGL, const double BGS, const double BGDL)
{
    const char ErrAngle[] = "ERROR ANGLE OUT OF RANGE";
    char DrawAngle[] = "XXXXXXXXXXXXXX";
    char DrawRange[] = "XXXXXXXXXXXXXX";
    int err = 0;
    if (pos < 0 || pos > 90) {
	err = 1;
    }
    else {
	snprintf(DrawAngle, sizeof(DrawAngle), "ANGLE: %d°", 90-pos);
	snprintf(DrawRange, sizeof(DrawRange), "RANGE: %.3f", dis[pos]);
    }

    // static const double midRdctr = midH+200;

    if (err == 1) DrawText(ErrAngle, midW-150, midH+225, 20, GRAY);
    DrawText(DrawRange, midW - BGS, midH+200-BGS, BGSR*2.f/5.f, GREEN);
    DrawText(DrawAngle, midW - BGS, midH+200-BGS+25, BGSR*2.f/5.f, GREEN);

    for (int i = 0; i < BGL+1; i++) {
	DrawLine(midW, midH+200, BGS*cosf((double)i*M_PI*BGDL + M_PI)+midW, BGS*sinf(i*M_PI*BGDL + M_PI)+midH+200, GREEN);
	DrawCircleSectorLines((Vector2){midW, midH+200}, BGSR*(i), -180, 0, 1, GREEN);
	char BG_distance[5] = {0};
	sprintf(BG_distance, "%1.1fm", (3.5f/(double)BGL)*i);
	DrawText(BG_distance, midW, midH+200 - BGSR*i+7, ceil((double)BGSR*1/4.f), GREEN);
	
    }
    DrawLine(midW, midH+200, BGS*cosf(((double)pos/90.f)*M_PI)+midW, BGS*-sinf(((double)pos/90.f)*M_PI)+midH+200, GREEN);

    for (int i = 0; i < 90; i++) {
	DrawLine(BGS*cosf((double)i/90.f*M_PI)+midW, BGS*sinf((double)i/90.f*M_PI + M_PI)+midH+200, (BGS-10.f)*cosf((double)i/90.f*M_PI)+midW, (BGS-10.f)*sinf((double)i/90.f*M_PI + M_PI)+midH+200, GREEN);
    }
}

void Draw_radar(int pos, double *dis)
{
    static const double midH = HEIGHT/2.f;
    static const double midW = WIDTH/2.f;

    static const int BG_size_ratio = 60;
    static const int BG_layer = 5;
    static const double BG_size = BG_size_ratio*BG_layer;
    static const double BG_density_layer = 1.f/(double)BG_layer;

    BeginDrawing();
    ClearBackground(BLACK);
    Draw_radar_BG(pos, dis, midW, midH, BG_size_ratio, BG_layer, BG_size, BG_density_layer);

    for (int i = 0; i < 91; i++) {
	if (i == pos) DrawCircle(midW + BG_size*(dis[pos]/MAX_RANGE)*(cosf((double)pos/90.f*M_PI)), midH + BG_size*(dis[pos]/MAX_RANGE)*(-sinf((double)pos/90.f*M_PI))+200, 5, RED);
	else DrawCircle(midW + BG_size*(dis[i]/MAX_RANGE)*(cosf((double)i/90.f*M_PI)), midH + BG_size*(dis[i]/MAX_RANGE)*(-sinf((double)i/90.f*M_PI))+200, 4, DARKBLUE);
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

    struct termios tty;
    tcgetattr(fd, &tty);

    ttyInit(fd);

    char raw_data[13];
    char str_pos[] = "0000,";
    char str_distance[] = "0.000\n\r";
    double Mdistance[90] = {0.f};
    double distance = 0;
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
	
	// Position must be between 0-90°
	Draw_radar(position, Mdistance);
	bufpos = position;
    }
    CloseWindow();
    tcsetattr(fd, TCSANOW, &tty);
    close(fd);

    return 0;
}
