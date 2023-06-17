#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 510
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

    pdx = cos(pa) * PI / 180;
    pdy = sin(pa) * PI / 180;
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
    // return (sqrt((bx - ax)*(bx - ax)) + ((by - ay)*(by - ay)));
    return cos(ang * PI / 180) * (bx-ax) - sin(ang * PI / 180) * (by - ay);
}

float fixAngle(float ang) {
    if (ang > 2 * PI) { return ang - 2 * PI; }
    if (ang < 0) { return ang + 2 * PI; }
    return ang;
}

// QUEBROU!!!
void drawRays3D() {
    int r, mx, my, mp, dof;
    float rx, ry, ra, xo, yo, disT;

    // Partindo do ângulo do jogador, reduzir 30 graus em radianos
    int RAYS_COUNT = 60;
    ra = fixAngle(pa - DR * RAYS_COUNT / 2);

    for (r = 0; r < RAYS_COUNT; r++) {
        // Verificar linhas horizontais
        dof = 0;
        float aTan = 1/tan(ra);
        float distH = 100000, hx=px, hy=py;

        if (ra > PI) {
            // Olhando para baixo
            ry = ((int)py>>6)-0.0001;
            rx = (py-ry)*aTan+px;
            yo=-64;
            xo=-yo*aTan;
        } else if (ra < PI) {
            // Olhando para cima
            ry = ((int)py>>6)+64;
            rx = (py-ry)*aTan+px;
            yo=64;
            xo=-yo*aTan;
        } else {
            // Olhando para esquerda ou direita
            rx=px;
            ry=py;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * MAP_WIDTH + mx;

            // Colidiu com parede horizontal
            if (mp > 0 && mp < MAP_WIDTH*MAP_HEIGHT && map[mp] == 1) {
                hx=rx;
                hy=ry;
                distH = distance(px, py, hx, hy, ra);
                dof = 8;
            } else {
                rx += xo;
                ry += yo;
                dof++;
            }
        }

        // Verificar linhas verticais
        dof = 0;
        float nTan = tan(ra);
        float distV = 100000, vx=px, vy=py;

        // Olhando para esquerda
        if (ra > P2 && ra < P3) {
            rx = (((int)px >> 6) << 6) - 0.0001;
            ry = (px-rx)*nTan+py;
            xo = -64;
            yo = -xo * nTan;
        }

        // Olhando para direita
        if (ra < P2 || ra > P3) {
            rx = (((int)px >> 6) << 6) + 64;
            ry = (px-rx) * nTan + py;
            xo = 64;
            yo = -xo * nTan;
        }

        // Olhando para cima ou para baixo
        if (ra == 0 || ra == PI) {
            rx = px;
            ry = py;
            dof = 8;
        }

        while (dof < 8) {
            mx = (int)(rx) >> 6;
            my = (int)(ry) >> 6;
            mp = my * MAP_WIDTH + mx;

            // Colidiu com parede vertical
            if (mp > 0 && mp < MAP_WIDTH*MAP_HEIGHT && map[mp] == 1) {
                dof = 8;
                vx=rx;
                vy=ry;
                distV = distance(px, py, vx, vy, ra);
            } else {
                rx += xo;
                ry += yo;
                dof++;
            }
        }

        // Desenhar apenas a menor distância do raio que colidiu com uma parede horizontal ou vertical
        if (distV < distH) {
            rx=vx;
            ry=vy;
            disT = distV;
        }

        if (distH < distV) {
            rx=hx;
            ry=hy;
            disT = distH;
        }

        // Raycasting em 2D
        glColor3f(1, 1, 0);
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex2i(px, py);
        glVertex2i(rx, ry);
        glEnd();

        // Ajustar visão de peixe
        float cosAngle = fixAngle(pa - ra);
        disT = disT * cos(cosAngle);

        // Desenhar a parte 3D
        float lineH = (MAP_WIDTH * MAP_HEIGHT * 320) / disT;
        if (lineH > 320) { lineH = 320; }
        float lineO = 255 - lineH / 2;
        glLineWidth(8); glBegin(GL_LINES); glVertex2i(r*8+WINDOW_HEIGHT, lineO); glVertex2i(r*8+WINDOW_HEIGHT, lineH+lineO); glEnd();

        ra = fixAngle(ra + DR);
    }
}

void buttons(unsigned char key, int x, int y) {
    if (key=='a') {
        pa -= PLAYER_ANGLE_TURN_SPEED;
        if (pa < 0) {
            pa += 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    if (key=='d') {
        pa += PLAYER_ANGLE_TURN_SPEED;
        if (pa > 2*PI) {
            pa -= 2*PI;
        }
        pdx = cos(pa) * 5;
        pdy = sin(pa) * 5;
    }
    if (key=='w') {
        px += pdx * 5;
        py += pdy * 5;
    }
    if (key=='s') {
        px -= pdx * 5;
        py -= pdy * 5;
    }

    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
