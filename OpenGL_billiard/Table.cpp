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
    draw_poly(n_corners, corners.data());
    for (const auto& hole : holes) {
        draw_circle(Vector2(hole.x, hole.y), HOLE_RADIUS, 0.0f, 0.0f, 0.0f);
    }
}