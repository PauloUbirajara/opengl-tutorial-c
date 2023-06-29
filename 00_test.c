#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#define WINDOW_WIDTH 512
#define WINDOW_HEIGHT 512
#define MAP_WIDTH 8
#define MAP_HEIGHT 8
#define MAP_PIXEL_SIZE 64
#define PLAYER_PIXEL_SIZE 8
#define PLAYER_ANGLE_TURN_SPEED 0.2
#define PLAYER_MOVE_SPEED 5
#define PLAYER_DEPTH_OF_FIELD 13
#define PI 3.1415926535897
#define MODE_2D 0
#define MODE_3D 1

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
int mode = MODE_2D;

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
//
bool isRayLookingDown(Ray ray) { return sin(ray.angle) > 0; }
bool isRayLookingUp(Ray ray) { return sin(ray.angle) < 0; }
bool isRayLookingLeft(Ray ray) { return cos(ray.angle) < 0; }
bool isRayLookingRight(Ray ray) { return cos(ray.angle) > 0; }


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

void drawMap2D() {
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

void drawRays2D() {
    int rayCount, depthOfField;
    float rayAngleOffset, rayDeltaX, rayDeltaY;

    rayAngleOffset = -30;

    Ray ray;

    /*
    * Setting the ray.x and ray.y to offset the player position and always
    * have the lines following the grid lines
    *
    * Setting ray delta Y to skip every X pixels between the grid lines
    * Setting ray delta X to calculate the position where it will hit the
    * horizontal line
    *
    * tan(angle) = oposite side / adjacent side
    * oposite side = tan(angle) * adjacent side
    *
    * the opposite side being the X position
    */

    glColor3f(1, 0, 0);

    float distH = 999999, distHRayX, distHRayY;
    for (rayCount = 0; rayCount < 60; rayCount++) {
        ray.angle = fixAngle(player.angle + degToRad(rayAngleOffset));

        // Horizontal check
        depthOfField=0;

        ray.x = player.x;  
        ray.y = player.y;

        float aTan = -1 / tan(ray.angle);

        if (isRayLookingUp(ray)) {
            ray.y = (((int)player.y >> 6) << 6) - 0.0001;
            ray.x = (player.y - ray.y) * aTan + player.x;
            rayDeltaY = -MAP_PIXEL_SIZE;
            rayDeltaX = -rayDeltaY * aTan;
        } else if (isRayLookingDown(ray)) {
            ray.y = (((int)player.y >> 6) << 6) + MAP_PIXEL_SIZE;
            ray.x = (player.y - ray.y) * aTan + player.x;
            rayDeltaY = MAP_PIXEL_SIZE;
            rayDeltaX = -rayDeltaY * aTan;
        } else {
            // left or right;
            ray.x = player.x;
            ray.y = player.y;
            depthOfField = PLAYER_DEPTH_OF_FIELD;
        }

        while ((depthOfField++) < PLAYER_DEPTH_OF_FIELD) {
            if (!isPointInsideMap(ray.x, ray.y)) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distH = distance(player.x, player.y, ray.x, ray.y);
                distHRayX = ray.x;
                distHRayY = ray.y;
                continue;
            }

            int rayPosY = ray.y / MAP_PIXEL_SIZE;
            int rayPosX = ray.x / MAP_PIXEL_SIZE;

            int rayPos = rayPosY * MAP_HEIGHT + rayPosX;

            if (rayPos > 0 && map[rayPos] == 1) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distH = distance(player.x, player.y, ray.x, ray.y);
                distHRayX = ray.x;
                distHRayY = ray.y;
                continue;
            }

            ray.x += rayDeltaX;
            ray.y += rayDeltaY;
        }

        // Vertical check
        depthOfField=0;

        ray.x = player.x;  
        ray.y = player.y;

        float nTan = -tan(ray.angle);

        float distV = 999999, distVRayX, distVRayY;
        if (isRayLookingLeft(ray)) {
            ray.x = (((int)player.x >> 6) << 6) - 0.0001;
            ray.y = (player.x - ray.x) * nTan + player.y;
            rayDeltaX = -MAP_PIXEL_SIZE;
            rayDeltaY = -rayDeltaX * nTan;

        } else if (isRayLookingRight(ray)) {
            ray.x = (((int)player.x >> 6) << 6) + MAP_PIXEL_SIZE;
            ray.y = (player.x - ray.x) * nTan + player.y;
            rayDeltaX = MAP_PIXEL_SIZE;
            rayDeltaY = -rayDeltaX * nTan;

        } else {
            ray.x = player.x;
            ray.y = player.y;
            depthOfField = PLAYER_DEPTH_OF_FIELD;
        }

        while ((depthOfField++) < PLAYER_DEPTH_OF_FIELD) {
            if (!isPointInsideMap(ray.x, ray.y)) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distV = distance(player.x, player.y, ray.x, ray.y);
                distVRayX = ray.x;
                distVRayY = ray.y;
                continue;
            }

            int rayPosY = ray.y / MAP_PIXEL_SIZE;
            int rayPosX = ray.x / MAP_PIXEL_SIZE;

            int rayPos = rayPosY * MAP_HEIGHT + rayPosX;

            if (rayPos > 0 && map[rayPos] == 1) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distV = distance(player.x, player.y, ray.x, ray.y);
                distVRayX = ray.x;
                distVRayY = ray.y;
                continue;
            }

            ray.x += rayDeltaX;
            ray.y += rayDeltaY;
        }

        float disT;
        if (distH < distV) {
            disT = distH;
            ray.x = distHRayX;
            ray.y = distHRayY;
        } else {
            disT = distV;
            ray.x = distVRayX;
            ray.y = distVRayY;
        }

        // 2D
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2i(player.x, player.y);
        glVertex2i(ray.x, ray.y);
        glEnd();

        rayAngleOffset++;
    }
}

void drawRays3D() {
    int rayCount, depthOfField;
    float rayAngleOffset, rayDeltaX, rayDeltaY;

    rayAngleOffset = -30;

    Ray ray;

    /*
    * Setting the ray.x and ray.y to offset the player position and always
    * have the lines following the grid lines
    *
    * Setting ray delta Y to skip every X pixels between the grid lines
    * Setting ray delta X to calculate the position where it will hit the
    * horizontal line
    *
    * tan(angle) = oposite side / adjacent side
    * oposite side = tan(angle) * adjacent side
    *
    * the opposite side being the X position
    */

    glColor3f(1, 0, 0);

    float distH = 999999, distHRayX, distHRayY;
    int maxRayCount = 60;
    for (rayCount = 0; rayCount < maxRayCount; rayCount++) {
        ray.angle = fixAngle(player.angle + degToRad(rayAngleOffset));

        // Horizontal check
        depthOfField = 0;

        ray.x = player.x;  
        ray.y = player.y;

        float aTan = -1 / tan(ray.angle);

        if (isRayLookingUp(ray)) {
            ray.y = (((int)player.y >> 6) << 6) - 0.0001;
            ray.x = (player.y - ray.y) * aTan + player.x;
            rayDeltaY = -MAP_PIXEL_SIZE;
            rayDeltaX = -rayDeltaY * aTan;
        } else if (isRayLookingDown(ray)) {
            ray.y = (((int)player.y >> 6) << 6) + MAP_PIXEL_SIZE;
            ray.x = (player.y - ray.y) * aTan + player.x;
            rayDeltaY = MAP_PIXEL_SIZE;
            rayDeltaX = -rayDeltaY * aTan;
        } else {
            // left or right;
            ray.x = player.x;
            ray.y = player.y;
            depthOfField = PLAYER_DEPTH_OF_FIELD;
        }

        while ((depthOfField++) < PLAYER_DEPTH_OF_FIELD) {
            if (!isPointInsideMap(ray.x, ray.y)) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distH = distance(player.x, player.y, ray.x, ray.y);
                distHRayX = ray.x;
                distHRayY = ray.y;
                continue;
            }

            int rayPosY = ray.y / MAP_PIXEL_SIZE;
            int rayPosX = ray.x / MAP_PIXEL_SIZE;

            int rayPos = rayPosY * MAP_HEIGHT + rayPosX;

            if (rayPos > 0 && map[rayPos] == 1) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distH = distance(player.x, player.y, ray.x, ray.y);
                distHRayX = ray.x;
                distHRayY = ray.y;
                continue;
            }

            ray.x += rayDeltaX;
            ray.y += rayDeltaY;
        }

        // Vertical check
        depthOfField=0;

        ray.x = player.x;  
        ray.y = player.y;

        float nTan = -tan(ray.angle);

        float distV = 999999, distVRayX, distVRayY;
        if (isRayLookingLeft(ray)) {
            ray.x = (((int)player.x >> 6) << 6) - 0.0001;
            ray.y = (player.x - ray.x) * nTan + player.y;
            rayDeltaX = -MAP_PIXEL_SIZE;
            rayDeltaY = -rayDeltaX * nTan;

        } else if (isRayLookingRight(ray)) {
            ray.x = (((int)player.x >> 6) << 6) + MAP_PIXEL_SIZE;
            ray.y = (player.x - ray.x) * nTan + player.y;
            rayDeltaX = MAP_PIXEL_SIZE;
            rayDeltaY = -rayDeltaX * nTan;

        } else {
            ray.x = player.x;
            ray.y = player.y;
            depthOfField = PLAYER_DEPTH_OF_FIELD;
        }

        while ((depthOfField++) < PLAYER_DEPTH_OF_FIELD) {
            if (!isPointInsideMap(ray.x, ray.y)) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distV = distance(player.x, player.y, ray.x, ray.y);
                distVRayX = ray.x;
                distVRayY = ray.y;
                continue;
            }

            int rayPosY = ray.y / MAP_PIXEL_SIZE;
            int rayPosX = ray.x / MAP_PIXEL_SIZE;

            int rayPos = rayPosY * MAP_HEIGHT + rayPosX;

            if (rayPos > 0 && map[rayPos] == 1) {
                depthOfField = PLAYER_DEPTH_OF_FIELD;
                distV = distance(player.x, player.y, ray.x, ray.y);
                distVRayX = ray.x;
                distVRayY = ray.y;
                continue;
            }

            ray.x += rayDeltaX;
            ray.y += rayDeltaY;
        }

        float disT;
        if (distH < distV) {
            disT = distH;
            ray.x = distHRayX;
            ray.y = distHRayY;
        } else {
            disT = distV;
            ray.x = distVRayX;
            ray.y = distVRayY;
        }

        // 3D
        // TODO Ajustar altura do raio, está invertido
        disT = disT * cos(fixAngle(player.angle - ray.angle));
        float rayOffsetX = WINDOW_WIDTH / maxRayCount;
        float lineHeight = disT;
        glLineWidth(8);
        glBegin(GL_LINES);
        glVertex2i(rayCount * rayOffsetX, lineHeight - WINDOW_HEIGHT / 2);
        glVertex2i(rayCount * rayOffsetX, lineHeight + WINDOW_HEIGHT / 2);
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

    // Barra de espaço
    if (key=='\040') {
        mode = mode == MODE_2D ? MODE_3D : MODE_2D;
        printf("Swapping mode: %d\n", mode);
    }

    player.angle = fixAngle(player.angle);

    glutPostRedisplay();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (mode == MODE_2D) {
        drawMap2D();
        drawRays2D();
        drawPlayer();
    }

    if (mode == MODE_3D) {
        drawRays3D();
    }

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
