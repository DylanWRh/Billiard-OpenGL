#include "render_utils.h"
#include <cmath>

void draw_circle(const Vector2& pos, float radius, float r, float g, float b)
{
    glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 90; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(90);
        float x = radius * cosf(theta);
        float y = radius * sinf(theta);
        glVertex2f(
            static_cast<float>(x + pos.x),
            static_cast<float>(y + pos.y)
        );
    }
    glEnd();
}

void draw_circle3D(const Vector2& pos, float radius, float y, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 50; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(50);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        glVertex3f(
            static_cast<float>(x + pos.x),
            y,
            static_cast<float>(z + pos.y)
        );
    }
    glEnd();
}

void draw_cylinder(const Vector2& pos, float radius, float y_low, float y_high, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 50; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(50);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        glVertex3f(
            static_cast<float>(x + pos.x),
            y_low,
            static_cast<float>(z + pos.y)
        );
    }
    glEnd();
    glBegin(GL_POLYGON);
    for (int i = 0; i < 50; i++) {
        float theta = 2.0f * 3.1415926f * float(i) / float(50);
        float x = radius * cosf(theta);
        float z = radius * sinf(theta);
        glVertex3f(
            static_cast<float>(x + pos.x),
            y_high,
            static_cast<float>(z + pos.y)
        );
    }
    glEnd();
    glBegin(GL_QUADS);
    for (int i = 0; i < 50; i++) {
        float theta1 = 2.0f * 3.1415926f * float(i) / float(50);
        float x1 = radius * cosf(theta1);
        float z1 = radius * sinf(theta1);
        float theta2 = 2.0f * 3.1415926f * float((i + 1) % 50) / float(50);
        float x2 = radius * cosf(theta2);
        float z2 = radius * sinf(theta2);
        glVertex3f(
            static_cast<float>(x1 + pos.x),
            y_low,
            static_cast<float>(z1 + pos.y)
        );
        glVertex3f(
            static_cast<float>(x2 + pos.x),
            y_low,
            static_cast<float>(z2 + pos.y)
        );
        glVertex3f(
            static_cast<float>(x2 + pos.x),
            y_high,
            static_cast<float>(z2 + pos.y)
        );
        glVertex3f(
            static_cast<float>(x1 + pos.x),
            y_high,
            static_cast<float>(z1 + pos.y)
        );
    }
    glEnd();
}

void draw_hollow_circle(const Vector2& pos, float radius, float r, float g, float b) {
    glColor3f(r, g, b);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < 90; i++) {
        float angle = 2.0f * 3.14159f * float(i) / float(90);
        float dx = radius * cosf(angle);
        float dy = radius * sinf(angle);
        glVertex2f(
            static_cast<GLfloat>(pos.x + dx),
            static_cast<GLfloat>(pos.y + dy)
        );
    }
    glEnd();
}

void draw_poly(int num, const Vector2* const vex)
{
    glColor3f(0, 1.0, 0);
    // https://blog.csdn.net/m0_52727721/article/details/128263015

    //画凹多边形第一遍
    glColor3f(1.0f, 0.0f, 0.0f);
    glStencilFunc(GL_ALWAYS, 1, 1);//初始模板位为0，由于一定通过测试，所以全部会被置为1，而重复区域由于画了两次模板位又归0
    glStencilMask(0x1);//开启模板缓冲区写入
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//第一遍绘制只是为了构造模板缓冲区，所以设置不显示第一遍的多边形
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < num; ++i) {
        glVertex2f(
            0.0, 0.0
        );
        glVertex2f(
            static_cast<float>(vex[i].x),
            static_cast<float>(vex[i].y)
        );
        glVertex2f(
            static_cast<float>(vex[(i + 1) % num].x),
            static_cast<float>(vex[(i + 1) % num].y)
        );
    }
    glEnd();
    //画凹多边形第二遍
    glStencilFunc(GL_NOTEQUAL, 0, 1);//模板值不为0就通过，即之前的重复区域会被舍弃掉，凹多边形就正确画出了
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glStencilMask(0x1);
    glColor3f(0.0f, 1.0f, 0.0f);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < num; ++i) {
        glVertex2f(
            0.0, 0.0
        );
        glVertex2f(
            static_cast<float>(vex[i].x),
            static_cast<float>(vex[i].y)
        );
        glVertex2f(
            static_cast<float>(vex[(i + 1) % num].x),
            static_cast<float>(vex[(i + 1) % num].y)
        );
    }
    glEnd();

}

void renderBoldStrokeString(float x, float y, float scale, const char* string, float boldness) {
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