//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <thread>

#include "defs.h"
#include "Table.h"
#include "Balls.h"
#include "defs.h"
#include "game.h"
#include "math_utils.h"
#include "render_utils.h"


static Game g;
static int view_option = 0;
constexpr int WIN_WIDTH = 1200;
constexpr int WIN_HEIGHT = 1000;

// Helper Functions

// ��ʼ��OpenGL����
void myGLinit();

// ��ʼ����������
void myinit();

// ��ʾ�������������
void display(void);

// ���к���
void idle(void);

// ���ڵ���
void myReshape(int w, int h);

// ����ƶ����
void mouseMotion(int x, int y);

// ���������
void mouseClick(int button, int state, int x, int y);


int main(int argc, char** argv)
{
    // ��������
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
    glutCreateWindow("Billiard");

    // OpenGL�������
    myGLinit();

    // ��������ĳ�ʼ��
    myinit();

    // ������
    glutReshapeFunc(myReshape);
    glutDisplayFunc(display);
    glutIdleFunc(idle);
    glutPassiveMotionFunc(mouseMotion);
    glutMouseFunc(mouseClick);
    glutMainLoop();
}

void myGLinit() {
    // ������Ⱦ׼��
    glEnable(GL_DEPTH_TEST);    // ������Ȳ���
    glEnable(GL_LIGHTING);      // ���ù��ռ���
    glEnable(GL_LIGHT0);        // ���õ�һ����Դ
    glEnable(GL_NORMALIZE);     // ���÷�������һ��

    // ���û������ղ���
    GLfloat light_ambient[] = { 1.0, 1.0, 1.0, 1.0 };  // ������Ϊ�׹�
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };  // �������Ϊ�׹�
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 }; // ���淴���Ϊ�׹�
    GLfloat light_position[] = { 0.0, 10.0, 0.0, 0.0 }; // ��Դλ��
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
}

void myinit() {
    // ��ʼ������
    Table table(8, 5, 4, 6);

    // ��ʼ�����������
    Vector2 white_position(-2, 0.0);
    Vector2 triangle_center(2, 0.0);
    Balls balls(white_position, triangle_center);

    // ��ʼ����Ϸ
    g = Game(table, balls);
    if (!g.initGame()) {
        return;
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    if (view_option) {
        gluLookAt(0, 10, 0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    }
    else {
        gluLookAt(0, 5, -7, 0.0, 0.0, 0.0, 0.0, 0.8, 0.2);
    }

    // ��������

    g.updateState();
    glClear(GL_COLOR_BUFFER_BIT);
    g.render();

    //// ��ʾ��ǰ����Լ�Ӧ�û������
    //glColor3f(0.0, 0.0, 0.0);
    //if (g.cur_player == 0) {
    //    renderBoldStrokeString(-5.5f, 3.9f, 0.005f, "Player 1", 3.0);
    //    draw_circle(Vector2(5, 4), 0.5, 1.0, 0.0, 0.0);
    //}
    //else {
    //    renderBoldStrokeString(-5.5f, 3.9f, 0.005f, "Player 2", 3.0);
    //    draw_circle(Vector2(5, 4), 0.5, 1.0, 0.0, 0.0);
    //    draw_circle(Vector2(5, 4), 0.25, 1.0, 1.0, 1.0);
    //}

    //// ��ʾ��Ϸ����������
    //if (g.gameState == Game::GAME_OVER) {
    //    glColor3f(1.0, 0.0, 0.0);
    //    renderBoldStrokeString(-4.0f, 0.0f, 0.01f, "GAME OVER", 3.0);
    //    if (g.winner == 1) {
    //        renderBoldStrokeString(-2.0f, -1.0f, 0.004f, "Player 1 Wins", 3.0);
    //    }
    //    else if (g.winner == 2) {
    //        renderBoldStrokeString(-2.0f, -1.0f, 0.004f, "Player 2 Wins", 3.0);
    //    }
    //}

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

void mouseMotion(int x, int y) {
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
    ������ԭ�ȵ�2D�汾
    double mouse_x = (double)x / 100 - 6.0;
    double mouse_y = 5.0 - (double)y / 100;
    g.mouse_pos = Vector2(mouse_x, mouse_y);
    ������ԭ�ȵ�2D�汾
    */
}

void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        g.mouse_click();
    }
    if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
        view_option = 1 - view_option;
    }
}