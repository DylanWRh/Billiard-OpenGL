#include "Table.h"
#include "defs.h"
#include "utils.h"
#include "math_utils.h"
#include "render_utils.h"
#include <random>

Table::Table(GLint n_corners_, GLint n_holes_, GLdouble r, GLdouble r_max) :
    n_corners(n_corners_), n_holes(n_holes_) 
{
    int mode = INIT_REGULAR;
    if (r_max > r) {
        mode = INIT_RANDOM;
    }

    const GLdouble PIn = 2 * PI / n_corners;

    for (int i = 0; i < n_corners; ++i) {
        GLdouble r_corner = r;
        if (mode == INIT_RANDOM) {
            r_corner = getRandomDouble(r, r_max);
        }
        corners.emplace_back(r_corner * cos(i * PIn), r_corner * sin(i * PIn));
    }

    for (int i = 0; i < n_holes; ++i) {
        // ���㵱ǰ��hole���ĸ�corner��ӽ�
        int i_corner = int(1.0 * i * n_corners / n_holes);
        GLdouble rr = corners[i_corner].Length2D();
        GLdouble ratio = (rr - HOLE_OFFSET) / rr;
        holes.emplace_back(ratio * corners[i_corner].x, ratio * corners[i_corner].y);
    }
}

int Table::checkInit() const{

    // ����Ƿ�������������
    if (n_corners < 3) {
        ErrorMsg("TABLEINIT_INVALID_CORNERS: ����Ӧ������3����");
        return TABLEINIT_INVALID_CORNERS;
    }

    // �ж��Ƿ��б��ر��
    for (size_t i = 0; i < n_corners; ++i) {
        Vector2 p1 = corners[i];
        Vector2 q1 = corners[(i + 1) % n_corners];
        if (p1.Dist2D(q1) < G_EPS) {
            ErrorMsg("TABLEINIT_INVALID_CORNERS: ����һ��������̫��");
            return TABLEINIT_INVALID_CORNERS;
        }
    }

    // �ж϶���α��Ƿ��ཻ
    if (polygonEdgesIntersect(corners)) {
        ErrorMsg("TABLEINIT_INVALID_CORNERS: �����������ཻ");
        return TABLEINIT_INVALID_CORNERS;
    }

    // �ж�holes�ĺϷ���
    // ������
    if (holes.size() < 1) {
        ErrorMsg("TABLEINIT_INVALID_HOLES: ������Ӧ���ж�");
        return TABLEINIT_INVALID_HOLES;
    }
    // ����Ƿ��б���С�Ķ�
    for (const auto& hole : holes) {
        if (HOLE_RADIUS <= BALL_RADIUS + G_EPS) {
            ErrorMsg("TABLEINIT_INVALID_HOLES: ���ڱ���С�Ķ�");
            return TABLEINIT_INVALID_HOLES;
        }
    }
    // TODO: ���ÿ�����Ƿ��������ڣ����¶����Բ����û���������ڶ��ڣ�
    for (const auto& hole : holes) {
        bool insideTable = false;

        // �򵥵ı߽����
        for (const auto& corner : corners) {
            if (hole.x >= corner.x && hole.x <= corner.x &&
                hole.y >= corner.y && hole.y <= corner.y) {
                insideTable = true;
                break;
            }
        }

        insideTable = pointInPolygon(corners, Vector2(hole.x, hole.y));

        if (!insideTable) {
            ErrorMsg("TABLEINIT_INVALID_HOLES: ���ڲ�����̨�ڵĶ�");
            return TABLEINIT_INVALID_HOLES;
        }
    }
    // �����֮��ľ���
    size_t hole_size = holes.size();
    for (size_t i = 0; i < hole_size; ++i) {
        for (size_t j = i + 1; j < hole_size; ++j) {
            Vector2 holep1 = Vector2(holes[i].x, holes[i].y);
            Vector2 holep2 = Vector2(holes[j].x, holes[j].y);
            double maxdis = 2 * HOLE_RADIUS;
            if ((holep1 - holep2).Length2D() < maxdis) {
                ErrorMsg("TABLEINIT_INVALID_HOLES: �������غϲ��ֵ���");
                return TABLEINIT_INVALID_HOLES;
            }
        }
    }
    return TABLEINIT_OK;
}

void Table::render() {

    // ��̨���ù�����Ⱦ
    glDisable(GL_LIGHTING);

    // ������̨
    // ���±����Ѿ��޸�Ϊdefs�еĲ���
    // float y_plane = 1.0f;       // ̨��߶�
    // float y_cushion = 1.3f;     // ��߸߶�
    // float y_low = 0.0f;         // ����߶�

    // ����̨��
    glColor3f(0, 1.0, 0);       // ��ɫ
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < n_corners; ++i) {
        glVertex3f(
            1.1f * (float)corners[i].x, 
            Y_PLANE, 
            1.1f * (float)corners[i].y
        );
        glVertex3f(
            1.1f * (float)corners[(i + 1) % n_corners].x, 
            Y_PLANE, 
            1.1f * (float)corners[(i + 1) % n_corners].y
        );
        glVertex3f(0.0f, Y_PLANE, 0.0f);
    }
    glEnd();

    // ���ƿ��
    glColor3f(0.4f, 0.2f, 0.0f);    // ��Χ���ɫ
    glBegin(GL_QUADS);
    for (int i = 0; i < n_corners; ++i) {
        glVertex3f(
            1.1f * (float)corners[i].x,
            Y_CUSHION, 
            1.1f * (float)corners[i].y
        );
        glVertex3f(
            1.1f * (float)corners[(i + 1) % n_corners].x,
            Y_CUSHION, 
            1.1f * (float)corners[(i + 1) % n_corners].y
        );
        glVertex3f(
            1.2f * (float)corners[(i + 1) % n_corners].x,
            Y_CUSHION, 
            1.2f * (float)corners[(i + 1) % n_corners].y
        );
        glVertex3f(
            1.2f * (float)corners[i].x,
            Y_CUSHION, 
            1.2f * (float)corners[i].y
        );
    }
    glEnd();
    glColor3f(0, 0.5, 0);       // ��Ȧī��ɫ
    glBegin(GL_QUADS);
    for (int i = 0; i < n_corners; ++i) {
        glVertex3f(
            1.1f * (float)corners[i].x, 
            Y_CUSHION, 
            1.1f * (float)corners[i].y
        );
        glVertex3f(
            1.1f * (float)corners[(i + 1) % n_corners].x,
            Y_CUSHION, 
            1.1f * (float)corners[(i + 1) % n_corners].y
        );
        glVertex3f(
            1.0f * (float)corners[(i + 1) % n_corners].x,
            Y_CUSHION, 
            1.0f * (float)corners[(i + 1) % n_corners].y);
        glVertex3f(
            1.0f * (float)corners[i].x,
            Y_CUSHION, 
            1.0f * (float)corners[i].y
        );
    }
    glEnd();
    glColor3f(0.0f, 0.3f, 0.0f);       // ����̨��������ɫ
    glBegin(GL_QUADS);
    for (int i = 0; i < n_corners; ++i) {
        glVertex3f(
            1.1f * (float)corners[i].x, 
            Y_CUSHION, 
            1.1f * (float)corners[i].y);
        glVertex3f(
            1.1f * (float)corners[(i + 1) % n_corners].x, 
            Y_CUSHION, 
            1.1f * (float)corners[(i + 1) % n_corners].y);
        glVertex3f(
            1.0f * (float)corners[(i + 1) % n_corners].x, 
            Y_PLANE, 
            1.0f * (float)corners[(i + 1) % n_corners].y);
        glVertex3f(
            1.0f * (float)corners[i].x, 
            Y_PLANE, 
            1.0f * (float)corners[i].y);
    }
    glEnd();

    // ���Ƶ���
    glColor3f(0.5, 0.25, 0);    // ��ɫ
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < n_corners; ++i) {
        glVertex3f(
            0.95f * (float)corners[i].x, 
            Y_LOW, 
            0.95f * (float)corners[i].y);
        glVertex3f(
            0.95f * (float)corners[(i + 1) % n_corners].x, 
            Y_LOW, 
            0.95f * (float)corners[(i + 1) % n_corners].y);
        glVertex3f(0.0f, Y_LOW, 0.0f);
    }
    glEnd();

    // ���ƿ�����������
    glColor3f(0.45f, 0.23f, 0.0f);    // ��ɫ
    glBegin(GL_QUADS);
    for (int i = 0; i < n_corners; ++i) {
        glVertex3f(
            0.95f * (float)corners[i].x, 
            Y_LOW, 
            0.95f * (float)corners[i].y);
        glVertex3f(
            0.95f * (float)corners[(i + 1) % n_corners].x, 
            Y_LOW, 
            0.95f * (float)corners[(i + 1) % n_corners].y);
        glVertex3f(
            1.2f * (float)corners[(i + 1) % n_corners].x, 
            Y_CUSHION, 
            1.2f * (float)corners[(i + 1) % n_corners].y);
        glVertex3f(
            1.2f * (float)corners[i].x, 
            Y_CUSHION, 
            1.2f * (float)corners[i].y);
    }
    glEnd();

    // �����򶴣���ɫ
    for (const auto& hole : holes) {
        draw_cylinder(
            Vector2(hole.x, hole.y), 
            HOLE_RADIUS, 
            Y_PLANE + 0.001f, 
            Y_CUSHION + 0.001f, 
            0, 0, 0
        );
    }
    
    // �ָ�����
    glEnable(GL_LIGHTING);

    /* 
    ������ԭ�ȵ�2D�汾
    draw_poly(n_corners, corners.data());
    for (const auto& hole : holes) {
        draw_circle(Vector2(hole.x, hole.y), HOLE_RADIUS, 0.0f, 0.0f, 0.0f);
    } 
    ������ԭ�ȵ�2D�汾 
    */
}