#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define MAP_PIXEL_SIZE 64
#define PLAYER_PIXEL_SIZE 8
#define PLAYER_VIEW_DISTANCE 8
#define PLAYER_SPEED 8
#define PLAYER_ANGLE_TURN_SPEED 0.1
#define PI 3.1415926535897
#define P2 PI / 2 // 90 deg
#define P3 3 * PI / 2 // 270 deg
#define DR 0.0174533 // one degree in radians


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

    px = 300; py = 300;

    pdx = cos(pa) * 5;
    pdy = sin(pa) * 5;
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

    // Linha no jogador para referência
    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2i(px, py);
    glVertex2i(px+pdx*PLAYER_VIEW_DISTANCE, py+pdy*PLAYER_VIEW_DISTANCE);
    glEnd();
}

float distance(float ax, float ay, float bx, float by, float ang) {
    return (sqrt((bx - ax)*(bx - ax)) + ((by - ay)*(by - ay)));
}

void drawRays3D() {
    int r, mx, my, mp, dof;
    float rx, ry, ra, xo, yo;
    ra = pa;


    for (r = 0; r < 1; r++) {
        // Verificar linhas horizontais
        dof = 0;
        float aTan = -1 / tan(ra);
        float distH = 100000, hx=px, hy=py;
        if (ra > PI) { ry = (((int)py>>6)<<6)-0.0001; rx = (py-ry)*aTan+px; yo=-64; xo=-yo*aTan; } // Olhando para baixo
        if (ra < PI) { ry = (((int)py>>6)<<6)+64; rx = (py-ry)*aTan+px; yo=64; xo=-yo*aTan; } // Olhando para cima
        if (ra == 0 || ra == PI) { rx=px; ry=py; dof=8; } // Olhando para esquerda ou direita

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * MAP_WIDTH + mx;

            if (mp > 0 && mp < MAP_WIDTH*MAP_HEIGHT && map[mp] == 1) { dof = 8; hx=rx; hy=ry; distH = distance(px, py, hx, hy, ra); } // Colidiu com parede horizontal
            else { rx += xo; ry += yo; dof++; }
        }

        // Verificar linhas verticais
        dof = 0;
        float nTan = -tan(ra);
        float distV = 100000, vx=px, vy=py;
        if (ra > P2 && ra < P3) { rx = (((int)px>>6)<<6)-0.0001; ry = (px-rx)*nTan+py; xo=-64; yo=-xo*nTan; } // Olhando para esquerda
        if (ra < P2 || ra > P3) { rx = (((int)px>>6)<<6)+64; ry = (px-rx)*nTan+py; xo=64; yo=-xo*nTan; } // Olhando para direita
        if (ra == 0 || ra == PI) { rx=px; ry=py; dof=8; } // Olhando para cima ou para baixo

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * MAP_WIDTH + mx;

            if (mp > 0 && mp < MAP_WIDTH*MAP_HEIGHT && map[mp] == 1) { dof = 8; vx=rx; vy=ry; distV = distance(px, py, vx, vy, ra); } // Colidiu com parede vertical
            else { rx += xo; ry += yo; dof++; }
        }

        // Desenhar apenas a menor distância do raio que colidiu com uma parede horizontal ou vertical
        if (distV < distH) { rx=vx; ry=vy; }
        if (distH < distV) { rx=hx; ry=hy; }

        glColor3f(1, 1, 0); glLineWidth(3); glBegin(GL_LINES); glVertex2i(px, py); glVertex2i(rx, ry); glEnd();
    }
}

void buttons(unsigned char key, int x, int y) {
    if (key=='a') { pa -= PLAYER_ANGLE_TURN_SPEED; if (pa < 0) { pa += 2*PI;} pdx = cos(pa)*5; pdy = sin(pa)*5; }
    if (key=='d') { pa += PLAYER_ANGLE_TURN_SPEED; if (pa > 2*PI) { pa -= 2*PI;} pdx = cos(pa); pdy = sin(pa); }
    if (key=='w') { px += pdx; py += pdy; }
    if (key=='s') { px -= pdx; py -= pdy; }

    printf("%c > %.0f %.0f\n", key, px, py);
    printf("%c > %d %d\n", key, x, y);

    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);
    drawMap();
    drawRays3D();
    drawPlayer();
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
