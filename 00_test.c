#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define MAP_PIXEL_SIZE 64
#define PLAYER_PIXEL_SIZE 8
#define PLAYER_SPEED 8
#define PLAYER_VIEW_DISTANCE 128
#define PLAYER_ANGLE_TURN_SPEED 0.1
#define PI 3.1415926535897


float px, py;
float pdx, pdy, pa;

int map[] = {
    1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,1,
    1,0,0,0,0,1,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,0,0,0,1,
    1,0,0,1,0,0,0,1,
    1,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,
};

void clearBackground() {
    glClearColor(0.3, 0.3, 0.3, 0);
    gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
}

void init() {
    clearBackground();

    px = (WINDOW_WIDTH / 2);
    px += (int)px % 8;
    py = (WINDOW_HEIGHT / 2);
    py += (int)py % 8;

    pdx = cos(pa);
    pdy = sin(pa); 
}

void drawMap() {
    int x, y, x0, y0;
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            x0 = x * MAP_PIXEL_SIZE;
            y0 = y * MAP_PIXEL_SIZE;

            if (map[y*MAP_WIDTH+x] == 1) { glColor3f(1, 1, 1); }
            else { glColor3f(0, 0, 0); }

            glBegin(GL_QUADS);
            glVertex2i(x0+1, y0+1);
            glVertex2i(x0+1, y0+MAP_PIXEL_SIZE-1);
            glVertex2i(x0+MAP_PIXEL_SIZE-1, y0+MAP_PIXEL_SIZE-1);
            glVertex2i(x0+MAP_PIXEL_SIZE-1, y0+1);
            glEnd();
        }
    }
}

void drawPlayer() {
    glColor3f(0, 0, 1);
    glPointSize(PLAYER_PIXEL_SIZE);

    glBegin(GL_POINTS);
    glVertex2i(px, py);
    glEnd();
}

void drawRays3D() {
    glPointSize(2);
    glBegin(GL_LINES);
    glVertex2i(px, py);
    glVertex2i(px+pdx*PLAYER_VIEW_DISTANCE, py+pdy*PLAYER_VIEW_DISTANCE);
    glEnd();
}

void buttons(unsigned char key, int x, int y) {
    if (key=='a') { pa -= PLAYER_ANGLE_TURN_SPEED; if (pa<0) {pa += 2*PI;} pdx = cos(pa); pdy = sin(pa); }
    if (key=='d') { pa += PLAYER_ANGLE_TURN_SPEED; if (pa>2*PI) {pa -= 2*PI;} pdx = cos(pa); pdy = sin(pa); }
    if (key=='w') { px += pdx * PLAYER_SPEED; py += pdy * PLAYER_SPEED;}
    if (key=='s') { px -= pdx * PLAYER_SPEED; py -= pdy * PLAYER_SPEED;}

    printf("%c > %.0f %.0f\n", key, px, py);
    printf("%c > %d %d\n", key, x, y);

    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawMap();
    drawPlayer();
    drawRays3D();
    glutSwapBuffers();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("OpenGL");

    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(buttons);

    glutMainLoop();
    return 0;
}
