#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 512
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define MAP_AREA MAP_WIDTH * MAP_HEIGHT
#define MAP_PIXEL_SIZE 64
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
    float x;
    float y;
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

    rayAngleOffset = -30;

    Ray ray;

    for (rayCount = 0; rayCount < 60; rayCount++) {
        ray.angle = fixAngle(player.angle + degToRad(rayAngleOffset));

        // Check horizontal lines
        float aTan = -1/tan(ray.angle);
        float distH = 1000000;
        float minHRayX = player.x, minHRayY = player.y;

        depthOfField = 0;

        // Up
        if (ray.angle > PI) {
            ray.y = (((int)player.y >> 6) << 6) - 0.0001;
            ray.x = (player.y - ray.y) * aTan + player.x;
            rayDeltaY = -MAP_PIXEL_SIZE;
            rayDeltaX = -rayDeltaY * aTan;
        }

        // Down
        if (ray.angle < PI) {
            ray.y = (((int)player.y >> 6) << 6) + MAP_PIXEL_SIZE;
            ray.x = (player.y - ray.y) * aTan + player.x;
            rayDeltaY = MAP_PIXEL_SIZE;
            rayDeltaX = -rayDeltaY * aTan;
        }

        // Left or Right
        if (abs(sin(ray.angle)) <= 0.099) {
            ray.y = player.y;
            ray.x = player.x;
            depthOfField = PLAYER_DEPTH_OF_FIELD;
        }

        while ((depthOfField++) < PLAYER_DEPTH_OF_FIELD) {
            if (!isPointInsideMap(ray.x, ray.y)) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                continue;
            }

            int rayMapPosX = (int)ray.x >> 6;
            int rayMapPosY = (int)ray.y >> 6;
            int rayMapPos = rayMapPosY * MAP_WIDTH + rayMapPosX;


            if (rayMapPos > 0 && map[rayMapPos] == 1) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distH = distance(player.x, player.y, ray.x, ray.y);
                minHRayX = ray.x;
                minHRayY = ray.y;
                continue;
            }

            ray.x += rayDeltaX;
            ray.y += rayDeltaY;
        }

        // Check vertical lines
        float nTan = -tan(ray.angle);
        float distV = 1000000;
        float minVRayX = player.x, minVRayY = player.y;

        depthOfField = 0;


        // Right
        if (ray.angle > P2 && ray.angle < P3) {
            ray.x = (((int)player.x >> 6) << 6) - 0.0001;
            ray.y = (player.x - ray.x) * nTan + player.y;
            rayDeltaX = -MAP_PIXEL_SIZE;
            rayDeltaY = -rayDeltaX * nTan;
        }

        // Left
        if (ray.angle < P2 || ray.angle > P3) {
            ray.x = (((int)player.x >> 6) << 6) + MAP_PIXEL_SIZE;
            ray.y = (player.x - ray.x) * nTan + player.y;
            rayDeltaX = MAP_PIXEL_SIZE;
            rayDeltaY = -rayDeltaX * nTan;
        }

        // Up or Down
        if (abs(cos(ray.angle)) <= 0.099) {
            ray.y = player.y;
            ray.x = player.x;
            depthOfField = PLAYER_DEPTH_OF_FIELD;
        }

        while ((depthOfField++) < PLAYER_DEPTH_OF_FIELD) {
            if (!isPointInsideMap(ray.x, ray.y)) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                continue;
            }

            int rayMapPosX = (int)ray.x >> 6;
            int rayMapPosY = (int)ray.y >> 6;
            int rayMapPos = rayMapPosY * MAP_WIDTH + rayMapPosX;

            if (rayMapPos > 0 && map[rayMapPos] == 1) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distV = distance(player.x, player.y, ray.x, ray.y);
                minVRayX = ray.x;
                minVRayY = ray.y;
                continue;
            }

            ray.x += rayDeltaX;
            ray.y += rayDeltaY;
        }

        float minDist;

        if (distH < distV) {
            ray.x = minHRayX;
            ray.y = minHRayY;
            minDist = distH;
            glColor3f(0.9, 0, 0);
        } else {
            ray.x = minVRayX;
            ray.y = minVRayY;
            minDist = distV;
            glColor3f(0.7, 0, 0);
        }

        // 2D
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2i(player.x, player.y);
        glVertex2i(ray.x, ray.y);
        glEnd();

        // 3D
        minDist = cos(fixAngle(player.angle - ray.angle)) * minDist;
        float lineHeight = MAP_AREA * WINDOW_WIDTH / 2 / minDist;
        float lineOffsetX = WINDOW_WIDTH / 2;
        float lineOffsetY = (WINDOW_HEIGHT - lineHeight) / 2;
        if (lineHeight > lineOffsetX) {
            lineHeight = lineOffsetX;
        }
        glLineWidth(8);
        glBegin(GL_LINES);
        glVertex2i(rayCount * 8 + lineOffsetX, lineOffsetY);
        glVertex2i(rayCount * 8 + lineOffsetX, lineOffsetY+lineHeight);
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
