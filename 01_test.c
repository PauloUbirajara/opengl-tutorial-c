#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define MAP_AREA MAP_WIDTH * MAP_HEIGHT
#define MAP_PIXEL_SIZE WINDOW_WIDTH / MAP_WIDTH
#define PLAYER_PIXEL_SIZE 8
#define PLAYER_ANGLE_TURN_SPEED 0.2
#define PLAYER_MOVE_SPEED 5
#define PLAYER_DEPTH_OF_FIELD 13
#define PI 3.1415926535897
#define P2 PI / 2 // 90 deg
#define P3 3 * PI / 2 // 270 deg
#define DR 0.0174533 // one degree in radians

typedef struct {
    float x;
    float y;
    float angle;
} Player;

typedef struct {
    float x0;
    float y0;
    float x1;
    float y1;
    float angle;
} Ray;

Player player;

bool isPlayerLookingDown() { return sin(player.angle) > 0; }
bool isPlayerLookingUp() { return sin(player.angle) < 0; }
bool isPlayerLookingLeft() { return cos(player.angle) < 0; }
bool isPlayerLookingRight() { return cos(player.angle) > 0; }
float distance(float x0, float y0, float x1, float y1) {
    float a, b, c;
    a = (x1-x0);
    b = (y1-y0);
    c = sqrt(a*a + b*b);

    return c;
}

// float distance(float ax, float ay, float bx, float by, float rad) {
//     return cos(rad) * (bx-ax) - sin(rad) * (by - ay);
// }


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

bool isPointInsideMap(float x, float y) {
    return (
    x > 0 && 
    y > 0 && 
    x < MAP_WIDTH * MAP_PIXEL_SIZE && 
    y < MAP_HEIGHT * MAP_PIXEL_SIZE
);
}

void clearBackground() {
    glClearColor(0.3, 0.3, 0.3, 0);
    gluOrtho2D(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
}

void init() {
    clearBackground();

    player.x = (int) (MAP_WIDTH * MAP_PIXEL_SIZE / 2);
    player.y = (int) (MAP_HEIGHT * MAP_PIXEL_SIZE / 2);
}

void drawMap() {
    int x, y, x0, y0;
    for (y = 0; y < MAP_HEIGHT; y++) {
        for (x = 0; x < MAP_WIDTH; x++) {
            x0 = x * MAP_PIXEL_SIZE;
            y0 = y * MAP_PIXEL_SIZE;
            int pos = y*MAP_WIDTH+x;

            if (map[pos] == 1) {
                glColor3f(1, 1, 1); 
            } else {
                glColor3f(0, 0, 0); 
            }

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
    glVertex2i(player.x, player.y);
    glEnd();

    // Linha no jogador para referência
    float px, py;
    float PLAYER_VIEW_DISTANCE = 30;

    px = cos(player.angle) * PLAYER_VIEW_DISTANCE;
    py = sin(player.angle) * PLAYER_VIEW_DISTANCE;

    glLineWidth(2);
    glBegin(GL_LINES);
    glVertex2i(player.x, player.y);
    glVertex2i(player.x+px, player.y+py);
    glEnd();
}

float fixAngle(float ang) {
    if (ang > 2 * PI) { return ang - 2 * PI; }
    if (ang < 0) { return ang + 2 * PI; }
    return ang;
}

float degToRad(float deg) {
    return deg * PI / 180;
}

void drawRays3D() {
    int rayCount, depthOfField;
    float rayAngleOffset, rayDeltaX, rayDeltaY;

    rayAngleOffset = 0;

    for (rayCount = 0; rayCount < 1; rayCount++) {
        Ray ray;
        ray.x0 = player.x;
        ray.y0 = player.y;
        ray.x1 = ray.x0;
        ray.y1 = ray.y0;
        ray.angle = fixAngle(player.angle + degToRad(rayAngleOffset));

        rayDeltaX = 0;
        rayDeltaY = 0;
        depthOfField = 0;
        float closestWallX0, closestWallY0, closestWallX1, closestWallY1;

        // Check for horizontal lines
        float distH = 9999999;
        if (isPlayerLookingLeft() || isPlayerLookingRight()) {
            glColor3f(1, 0, 0);
            rayDeltaX = cos(ray.angle) * MAP_PIXEL_SIZE;
            rayDeltaY = sin(ray.angle) * MAP_PIXEL_SIZE;
        } else {
            glColor3f(0, 0, 0);
            depthOfField = 8;
        }

        printf("-----\n");
        printf("A[%.2f %.2f]\n", sin(player.angle), cos(player.angle));
        printf("P[%.2f %.2f > %.2f deg]\n", player.x, player.y, player.angle);

        printf("UP %d\n", isPlayerLookingUp());
        printf("DOWN %d\n", isPlayerLookingDown());
        printf("LEFT %d\n", isPlayerLookingLeft());
        printf("RIGHT %d\n", isPlayerLookingRight());


        while ((depthOfField++) < PLAYER_DEPTH_OF_FIELD) {
            if (!isPointInsideMap(ray.x1, ray.y1)) { break; }
            float rayMapOffsetX, rayMapOffsetY;
            rayMapOffsetX = ((int)ray.x1 >> 6) << 6 / MAP_PIXEL_SIZE;
            rayMapOffsetY = ((int)ray.y1 >> 6) << 6 / MAP_PIXEL_SIZE;

            printf(
                "[%2.0d]%.2f %.2f (%2.2f %2.2f)\n",
                depthOfField,
                ray.x1,
                ray.y1,
                rayMapOffsetX,
                rayMapOffsetY
            );

            int rayMapPos = rayMapOffsetY * MAP_WIDTH + rayMapOffsetX;

            if (rayMapPos > 0 && map[rayMapPos] == 1) { 
                depthOfField = PLAYER_DEPTH_OF_FIELD;

                float wallX0, wallY0, wallX1, wallY1;
                wallX0 = rayMapOffsetX * MAP_PIXEL_SIZE;
                wallY0 = rayMapOffsetY * MAP_PIXEL_SIZE;
                wallX1 = (rayMapOffsetX + 1) * MAP_PIXEL_SIZE;
                wallY1 = (rayMapOffsetY + 1) * MAP_PIXEL_SIZE;

                if (isPlayerLookingDown()) {
                    wallY1 -= MAP_PIXEL_SIZE;
                } 
                if (isPlayerLookingUp()) {
                    wallY0 += MAP_PIXEL_SIZE;
                }

                printf("W1[%.2f %.2f]\n", wallX0, wallY0);
                printf("W2[%.2f %.2f]\n", wallX1, wallY1);

                glColor3f(0, 1, 0);
                glPointSize(PLAYER_PIXEL_SIZE);
                glBegin(GL_POINTS);
                glVertex2i(wallX0, wallY0);
                glVertex2i(wallX1, wallY1);
                glEnd();
                glColor3f(1, 0, 0);

                float distanceH = distance(ray.x1, ray.y1, wallX1, wallY1);
                if (distanceH < distH) {
                    distH = distanceH;
                    closestWallX0 = wallX0;
                    closestWallY0 = wallY0;
                    closestWallX1 = wallX1;
                    closestWallY1 = wallY1;
                }
            }

            ray.x1 += rayDeltaX;
            ray.y1 += rayDeltaY;

            glPointSize(PLAYER_PIXEL_SIZE);
            glBegin(GL_POINTS);
            glVertex2i(ray.x1, ray.y1);
            glEnd();
        }

        ray.x1 = ray.x0;
        ray.y1 = ray.y0;
        rayDeltaX = 0;
        rayDeltaY = 0;
        depthOfField = 0;

        // Check for vertical lines
        float distV = 9999999;
        if (isPlayerLookingUp() || isPlayerLookingDown()) {
            glColor3f(1, 0, 0);
            rayDeltaX = cos(ray.angle) * MAP_PIXEL_SIZE;
            rayDeltaY = sin(ray.angle) * MAP_PIXEL_SIZE;
        } else {
            glColor3f(0, 0, 0);
            depthOfField = 8;
        }

        printf("-----\n");
        printf("A[%.2f %.2f]\n", sin(player.angle), cos(player.angle));
        printf("P[%.2f %.2f > %.2f deg]\n", player.x, player.y, player.angle);

        printf("UP %d\n", isPlayerLookingUp());
        printf("DOWN %d\n", isPlayerLookingDown());
        printf("LEFT %d\n", isPlayerLookingLeft());
        printf("RIGHT %d\n", isPlayerLookingRight());


        while ((depthOfField++) < PLAYER_DEPTH_OF_FIELD) {
            if (!isPointInsideMap(ray.x1, ray.y1)) { break; }
            float rayMapOffsetX, rayMapOffsetY;
            rayMapOffsetX = ((int)ray.x1 >> 6) << 6 / MAP_PIXEL_SIZE;
            rayMapOffsetY = ((int)ray.y1 >> 6) << 6 / MAP_PIXEL_SIZE;

            printf(
                "[%2.0d]%.2f %.2f (%2.2f %2.2f)\n",
                depthOfField,
                ray.x1,
                ray.y1,
                rayMapOffsetX,
                rayMapOffsetY
            );

            int rayMapPos = rayMapOffsetY * MAP_WIDTH + rayMapOffsetX;

            if (rayMapPos > 0 && map[rayMapPos] == 1) { 
                depthOfField = PLAYER_DEPTH_OF_FIELD;

                float wallX0, wallY0, wallX1, wallY1;
                wallX0 = rayMapOffsetX * MAP_PIXEL_SIZE;
                wallY0 = rayMapOffsetY * MAP_PIXEL_SIZE;
                wallX1 = (rayMapOffsetX + 1) * MAP_PIXEL_SIZE;
                wallY1 = (rayMapOffsetY + 1) * MAP_PIXEL_SIZE;

                if (isPlayerLookingRight()) {
                    wallX1 -= MAP_PIXEL_SIZE;
                } 
                if (isPlayerLookingLeft()) {
                    wallX0 += MAP_PIXEL_SIZE;
                }

                printf("W1[%.2f %.2f]\n", wallX0, wallY0);
                printf("W2[%.2f %.2f]\n", wallX1, wallY1);

                glColor3f(0, 1, 0);
                glPointSize(PLAYER_PIXEL_SIZE);
                glBegin(GL_POINTS);
                glVertex2i(wallX0, wallY0);
                glVertex2i(wallX1, wallY1);
                glEnd();
                glColor3f(1, 0, 0);

                float distanceV = distance(ray.x1, ray.y1, wallX1, wallY1);
                if (distanceV < distV) {
                    distV = distanceV;
                    closestWallX0 = wallX0;
                    closestWallY0 = wallY0;
                    closestWallX1 = wallX1;
                    closestWallY1 = wallY1;
                }
            }

            printf("DH[%.2f]\n", distH);
            printf("DV[%.2f]\n", distV);
            ray.x1 += rayDeltaX;
            ray.y1 += rayDeltaY;

            glPointSize(PLAYER_PIXEL_SIZE);
            glBegin(GL_POINTS);
            glVertex2i(ray.x1, ray.y1);
            glEnd();
        }

        // ERRADO - Usando apenas um dos pontos da parede para identificar o mais próximo, ainda não serve para pegar de fato a aresta mais próxima
        float distance = distH < distV ? distH : distV;
        printf("D [%.2f]\n", distance);

        glColor3f(0.7, 0.3, 1);
        glPointSize(PLAYER_PIXEL_SIZE);
        glBegin(GL_POINTS);
        glVertex2i(closestWallX0, closestWallY0);
        glVertex2i(closestWallX1, closestWallY1);
        glEnd();
        glColor3f(1, 0, 0);

        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2i(ray.x0, ray.y0);
        glVertex2i(ray.x1, ray.y1);
        glEnd();

        rayAngleOffset++;
    }
}

void buttons(unsigned char key, int x, int y) {
    if (key=='a') {
        player.angle -= PLAYER_ANGLE_TURN_SPEED;
    }

    if (key=='d') {
        player.angle += PLAYER_ANGLE_TURN_SPEED;
    }

    if (key=='w') {
        player.x += cos(player.angle) * PLAYER_MOVE_SPEED;
        player.y += sin(player.angle) * PLAYER_MOVE_SPEED;
    }

    if (key=='s') {
        player.x -= cos(player.angle) * PLAYER_MOVE_SPEED;
        player.y -= sin(player.angle) * PLAYER_MOVE_SPEED;
    }

    player.angle = fixAngle(player.angle);

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
