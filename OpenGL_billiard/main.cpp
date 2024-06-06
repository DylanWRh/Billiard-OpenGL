//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <iostream>
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <thread>
#include <iostream>
#include "defs.h"
#include "Table.h"
#include "Balls.h"
#include "defs.h"
#include "game.h"
#include "math_utils.h"
#include "render_utils.h"
#include "sound_stuff.h"


static Game g;

// 为gluLookAt旋转视角准备的变量
static int view_mode = 0;   // 0表示自由视角，1表示垂直视角，用鼠标右键转换
static GLfloat distance = 10.0f;
static GLfloat angleH = 0.0f, angleV = 60.0f;
static GLfloat upx = 0.0f, upy = 1.0f, upz = 0.0f;

// 窗口大小
constexpr int WIN_WIDTH = 1200;
constexpr int WIN_HEIGHT = 1000;

// 鼠标是否按下
static bool mouse_down = false;  

// Helper Functions

// 初始化OpenGL设置
void myGLinit();

// 初始化球桌和球
void myinit();

// 显示球桌、球和文字
void display(void);

// 绘制窗口中的2D成分
void render2D(void);

// 判断是否在场景中，输入参数为鼠标坐标
bool isInScene(int x, int y);

// 调整加塞
void setSide(int x, int y);

// 空闲函数
void idle(void);

// 窗口调整
void myReshape(int w, int h);

// 鼠标移动检测
void fnMouseMotion(int x, int y);

// 鼠标点击函数
void fnMouseClick(int button, int state, int x, int y);

// 键盘响应函数
void fnSpecialKeys(int key, int x, int y);

int main(int argc, char** argv)
{
    // 基础设置
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow("Billiard");

    // 初始化声音
    init_sound();

    // OpenGL相关设置
    myGLinit();

    // 球桌和球的初始化
    myinit();

    // 注册回调函数
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(fnMouseMotion);
    glutMotionFunc(fnMouseMotion);
    glutMouseFunc(fnMouseClick);
    glutSpecialFunc(fnSpecialKeys);
    glutMainLoop();

    return 0;
}

void myGLinit() {
    // 光照渲染准备
    glEnable(GL_DEPTH_TEST);    // 启用深度测试
    glEnable(GL_LIGHTING);      // 启用光照计算
    glEnable(GL_LIGHT0);        // 启用第一个光源
    glEnable(GL_NORMALIZE);     // 启用法向量归一化
    glEnable(GL_MULTISAMPLE);   // 启用多重采样

    // 设置环境光照参数
    GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0 };  // 环境光为白光
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };  // 漫反射光为白光
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 }; // 镜面反射光为白光
    GLfloat light_position[] = { 3.0, 3.0, 0.0, 0.0 }; // 光源位置
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void myinit() {
    // 初始化球桌
    Table table(8, 5, 4, 6);

    // 初始化所有球摆球
    Vector2 white_position(-2, 0.0);
    Vector2 triangle_center(2, 0.0);
    Balls balls(white_position, triangle_center);

    // 初始化游戏
    g = Game(table, balls);
    if (!g.initGame()) {
        return;
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (view_mode) {
        gluLookAt(0.0, 10.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -1.0);
    }
    else {
        // 计算摄像机位置
        float radianH = (float)(angleH * PI / 180.0);
        float radianV = (float)(angleV * PI / 180.0);

        float eyex = distance * cos(radianV) * sin(radianH);
        float eyey = distance * sin(radianV);
        float eyez = distance * cos(radianV) * cos(radianH);

        gluLookAt(eyex, eyey, eyez, 0.0, 0.0, 0.0, upx, upy, upz);
    }

    g.updateState();
    glClear(GL_COLOR_BUFFER_BIT);
    g.render();
    
    render2D();

    glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}

void myReshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(90, (GLfloat)w / (GLfloat)h, 1.0, 20.0);
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

void fnMouseMotion(int x, int y) {
    
    if (mouse_down) {
        setSide(x, y);
    }

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];
    GLfloat winX, winY, winZ;
    GLdouble worldX, worldY, worldZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    winX = (float)x;
    winY = (float)viewport[3] - (float)y;
    glReadPixels(x, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);

    gluUnProject(winX, winY, winZ, modelview, projection, viewport, &worldX, &worldY, &worldZ);

    g.mouse_pos = Vector2(worldX, worldZ);

    /*
    以下是原先的2D版本
    double mouse_x = (double)x / 100 - 6.0;
    double mouse_y = 5.0 - (double)y / 100;
    g.mouse_pos = Vector2(mouse_x, mouse_y);
    以上是原先的2D版本
    */
}

void fnMouseClick(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        mouse_down = true;
    }
    else if (state == GLUT_UP) {
        mouse_down = false;
    }

    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (!isInScene(x, y)) {
            g.mouse_click();
        }
    }
    else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        view_mode = 1 - view_mode;
    }
    else if (button == 3) { // 鼠标滚轮向上
        distance -= 0.05f;
        if (distance < 7.0f) distance = 7.0f; 
    }
    else if (button == 4) { // 鼠标滚轮向下
        distance += 0.05f;
        if (distance > 20.0f) distance = 20.0f;
    }
}

void fnSpecialKeys(int key, int x, int y) {
    float angleStep = 5.0f;

    switch (key) {
    case GLUT_KEY_LEFT:
        angleH -= angleStep;
        break;
    case GLUT_KEY_RIGHT:
        angleH += angleStep;
        break;
    case GLUT_KEY_UP:
        angleV += angleStep;
        break;
    case GLUT_KEY_DOWN:
        angleV -= angleStep;
        break;
    }

    // 限制垂直角度范围
    if (angleV > 89.0f) angleV = 89.0f;
    if (angleV < 10.0f) angleV = 10.0f;

    glutPostRedisplay();
}

void render2D(void) {
    // 2D部分初始化
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-6, 6, -5, 5);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();


    // 显示加塞位置
    Vector3 side = g.cueSide;
    float sidex = side.x * 2 / 5 + 5;
    float sidey = side.y * 2/ 5 + 4;
    draw_circle(Vector2(sidex, sidey), 0.1f, 0,0,0);


    // 显示当前玩家以及应该击打的球
    glColor3f(1.0f, 1.0f, 1.0f);
    if (g.cur_player == 0) {
        renderBoldStrokeString(-5.5f, 3.8f, 0.005f, "Player 1", 2.0f);
        draw_circle(Vector2(5, 4), 0.5f, 1.0f, 1.0f, 1.0f);
    }
    else {
        renderBoldStrokeString(-5.5f, 3.8f, 0.005f, "Player 2", 2.0f);
        draw_circle(Vector2(5, 4), 0.35f, 1.0f, 1.0f, 1.0f);
        draw_circle(Vector2(5, 4), 0.5f, 1.0f, 0.0f, 0.0f);
    }

    // 显示出杆力度
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(-2.5, 3.8);
    glVertex2f(3.5, 3.8);
    glVertex2f(3.5, 4.2);
    glVertex2f(-2.5, 4.2);
    glEnd();
    if (g.gameState == Game::GAME_RUN_STATIC) {
        float force = (g.mouse_pos - g.balls.balls[g.balls.cue_pos].m_position).Length2D() * CUE_FORCE_RATE;
        if (force > VEL_MAX) force = VEL_MAX;
        float force_render_x = -2.5 + (force / VEL_MAX) * 6;
        glColor3f(1.0f, 1.0f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(-2.5, 3.8);
        glVertex2f(force_render_x, 3.8);
        glVertex2f(force_render_x, 4.2);
        glVertex2f(-2.5, 4.2);
        glEnd();
    }

    // 显示游戏结束的文字
    if (g.gameState == Game::GAME_OVER) {
        glColor3f(1.0, 0.0, 0.0);
        renderBoldStrokeString(-4.0f, 0.0f, 0.01f, "GAME OVER", 3.0);
        if (g.winner == 1) {
            renderBoldStrokeString(-2.0f, -1.0f, 0.004f, "Player 1 Wins", 3.0);
        }
        else if (g.winner == 2) {
            renderBoldStrokeString(-2.0f, -1.0f, 0.004f, "Player 2 Wins", 3.0);
        }
    }

    // 顶部状态栏
    glColor3f(0.3f, 0.3f, 0.3f);
    glBegin(GL_QUADS);
    glVertex2f(-6, 3);
    glVertex2f(-6, 5);
    glVertex2f(6, 5);
    glVertex2f(6, 3);
    glEnd();

    // 2D部分渲染结束
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

bool isInScene(int x, int y) {
    return (y <= 200);
}

void setSide(int x, int y) {
    if ((x - 1100) * (x - 1100) + (y - 100) * (y - 100) > 47 * 47)
        return;
    float sidex = (x - 1100) * 1.0f / 50;   // in [-1, 1]
    float sidey = -(y - 100) * 1.0f / 50;   // in [-1, 1]

    Vector3 side(sidex, sidey, sqrt(1 - sidex * sidex - sidey * sidey));

    g.cueSide = side;
}