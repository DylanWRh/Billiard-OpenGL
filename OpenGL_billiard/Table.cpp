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
        // 计算当前的hole与哪个corner最接近
        int i_corner = int(1.0 * i * n_corners / n_holes);
        GLdouble rr = corners[i_corner].Length2D();
        GLdouble ratio = (rr - HOLE_OFFSET) / rr;
        holes.emplace_back(ratio * corners[i_corner].x, ratio * corners[i_corner].y);
    }
}

int Table::checkInit() const{

    // 检查是否至少有三个角
    if (n_corners < 3) {
        ErrorMsg("TABLEINIT_INVALID_CORNERS: 球桌应至少有3个角");
        return TABLEINIT_INVALID_CORNERS;
    }

    // 判断是否有边特别短
    for (size_t i = 0; i < n_corners; ++i) {
        Vector2 p1 = corners[i];
        Vector2 q1 = corners[(i + 1) % n_corners];
        if (p1.Dist2D(q1) < G_EPS) {
            ErrorMsg("TABLEINIT_INVALID_CORNERS: 存在一条球桌边太短");
            return TABLEINIT_INVALID_CORNERS;
        }
    }

    // 判断多边形边是否相交
    if (polygonEdgesIntersect(corners)) {
        ErrorMsg("TABLEINIT_INVALID_CORNERS: 存在球桌边相交");
        return TABLEINIT_INVALID_CORNERS;
    }

    // 判断holes的合法性
    // 检查个数
    if (holes.size() < 1) {
        ErrorMsg("TABLEINIT_INVALID_HOLES: 球桌上应当有洞");
        return TABLEINIT_INVALID_HOLES;
    }
    // 检查是否有比球还小的洞
    for (const auto& hole : holes) {
        if (HOLE_RADIUS <= BALL_RADIUS + G_EPS) {
            ErrorMsg("TABLEINIT_INVALID_HOLES: 存在比球还小的洞");
            return TABLEINIT_INVALID_HOLES;
        }
    }
    // TODO: 检查每个球洞是否在球桌内，最多露出半圆（当没有球桌角在洞内）
    for (const auto& hole : holes) {
        bool insideTable = false;

        // 简单的边界框检查
        for (const auto& corner : corners) {
            if (hole.x >= corner.x && hole.x <= corner.x &&
                hole.y >= corner.y && hole.y <= corner.y) {
                insideTable = true;
                break;
            }
        }

        insideTable = pointInPolygon(corners, Vector2(hole.x, hole.y));

        if (!insideTable) {
            ErrorMsg("TABLEINIT_INVALID_HOLES: 存在不在球台内的洞");
            return TABLEINIT_INVALID_HOLES;
        }
    }
    // 检测球洞之间的距离
    size_t hole_size = holes.size();
    for (size_t i = 0; i < hole_size; ++i) {
        for (size_t j = i + 1; j < hole_size; ++j) {
            Vector2 holep1 = Vector2(holes[i].x, holes[i].y);
            Vector2 holep2 = Vector2(holes[j].x, holes[j].y);
            double maxdis = 2 * HOLE_RADIUS;
            if ((holep1 - holep2).Length2D() < maxdis) {
                ErrorMsg("TABLEINIT_INVALID_HOLES: 存在有重合部分的球洞");
                return TABLEINIT_INVALID_HOLES;
            }
        }
    }
    return TABLEINIT_OK;
}

void Table::render() {

    // 球台不用光照渲染
    glDisable(GL_LIGHTING);

    // 绘制球台
    // 以下变量已经修改为defs中的参数
    // float y_plane = 1.0f;       // 台面高度
    // float y_cushion = 1.3f;     // 库边高度
    // float y_low = 0.0f;         // 底面高度

    // 绘制台面
    glColor3f(0, 1.0, 0);       // 绿色
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

    // 绘制库边
    glColor3f(0.4f, 0.2f, 0.0f);    // 外围深褐色
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
    glColor3f(0, 0.5, 0);       // 内圈墨绿色
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
    glColor3f(0.0f, 0.3f, 0.0f);       // 库与台连接深绿色
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

    // 绘制底面
    glColor3f(0.5, 0.25, 0);    // 褐色
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

    // 绘制库与底面的连接
    glColor3f(0.45f, 0.23f, 0.0f);    // 褐色
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

    // 绘制球洞，黑色
    for (const auto& hole : holes) {
        draw_cylinder(
            Vector2(hole.x, hole.y), 
            HOLE_RADIUS, 
            Y_PLANE + 0.001f, 
            Y_CUSHION + 0.001f, 
            0, 0, 0
        );
    }
    
    // 恢复光照
    glEnable(GL_LIGHTING);

    /* 
    以下是原先的2D版本
    draw_poly(n_corners, corners.data());
    for (const auto& hole : holes) {
        draw_circle(Vector2(hole.x, hole.y), HOLE_RADIUS, 0.0f, 0.0f, 0.0f);
    } 
    以上是原先的2D版本 
    */
}