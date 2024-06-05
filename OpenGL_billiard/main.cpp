//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <thread>

#include "Table.h"
#include "Balls.h"
#include "defs.h"
#include "game.h"
#include "math_utils.h"
#include "render_utils.h"


static Game g;

// Helper Functions

// 初始化OpenGL设置，初始化球桌和球
void myinit();

// 显示球桌、球和文字
void display(void);

// 空闲函数
void idle(void);

// 窗口调整
void myReshape(int w, int h);

// 鼠标移动检测
void mouseMotion(int x, int y);

// 鼠标点击函数
void mouseClick(int button, int state, int x, int y);


int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 1000);
    glutCreateWindow("Billiard");
    myinit();
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);
    glutMainLoop();
}

void myinit() {
    // 初始化OpenGL设置
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-6, 6, -6, 6);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glColor3f(1.0, 0.0, 0.0);

    // 初始化球桌
    Table table(8, 5, 3, 5);

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
    // 显示球桌和球
    g.updateState();
    glClear(GL_COLOR_BUFFER_BIT);
    g.render();

    // 显示当前玩家以及应该击打的球
    glColor3f(0.0, 0.0, 0.0);
    if (g.cur_player == 0) {
        renderBoldStrokeString(-5.5f, 3.9f, 0.005f, "Player 1", 3.0);
        draw_circle(Vector2(5, 4), 0.5, 1.0, 0.0, 0.0);
    }
    else {
        renderBoldStrokeString(-5.5f, 3.9f, 0.005f, "Player 2", 3.0);
        draw_circle(Vector2(5, 4), 0.5, 1.0, 0.0, 0.0);
        draw_circle(Vector2(5, 4), 0.25, 1.0, 1.0, 1.0);
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

    glutSwapBuffers();
}

void idle()
{
    glutPostRedisplay();
}

void myReshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-6, 6, -5, 5);
    glViewport(-6, -5, 1200, 1000); 
    glMatrixMode(GL_MODELVIEW);
    glutPostRedisplay();
}

void mouseMotion(int x, int y) {
    double mouse_x = (double)x / 100 - 6.0;
    double mouse_y = 5.0 - (double)y / 100;
    g.mouse_pos = Vector2(mouse_x, mouse_y);
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        g.mouse_click();
    }
}