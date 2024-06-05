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

void renderStrokeString(float x, float y, float scale, const char* string);

void renderBoldStrokeString(float x, float y, float scale, const char* string, float boldness = 1.0f) {
    const char* c;
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, scale);

    for (float dx = -boldness; dx <= boldness; dx += boldness / 10) {
        for (float dy = -boldness; dy <= boldness; dy += boldness / 10) {
            if (dx != 0.0f || dy != 0.0f) {
                glPushMatrix();
                glTranslatef(dx, dy, 0);
                for (c = string; *c != '\0'; c++) {
                    glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
                }
                glPopMatrix();
            }
        }
    }

    for (c = string; *c != '\0'; c++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    }

    glPopMatrix();
}

void display(void)
{
    int i = 0;
    static bool first_run = true;
    //if (first_run) {
    //    first_run = false;
    //    std::thread([]() {
    //        while (true)
    //        }).detach();
    //}
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

    // 游戏结束
    
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

void renderStrokeString(float x, float y, float scale, const char* string) {
    const char* c;
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, scale);
    for (c = string; *c != '\0'; c++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    }
    glPopMatrix();
}

void myinit()
{
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


void idle()
{
    glutPostRedisplay();
}

void myReshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-6, 6, -5, 5);///
    glViewport(-6, -5, 1200, 1000); ///
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

int
main(int argc, char** argv)
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

