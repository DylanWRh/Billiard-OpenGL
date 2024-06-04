#include "math_utils.h"
#include "utils.h"
#include "defs.h"
#include <math.h>
#include <vector>

double crossProduct(Vector2 a, Vector2 b) {
    return a.x * b.y - a.y * b.x;
}

bool segmentsIntersect(Vector2 p1, Vector2 q1, Vector2 p2, Vector2 q2)
{
    Vector2 r = q1 - p1;
    Vector2 s = q2 - p2;

    auto cross = crossProduct(r, s);
    Vector2 q_minus_p = p2 - p1;

    if (abs(cross) < G_EPS) {
        // TODO: 平行或共线的情况，计算点乘
        //if (r.Dot2D(s) < 0) {
        //    return abs(crossProduct(q_minus_p, r)) > G_EPS;
        //}
        return false;
    }

    auto t = crossProduct(q_minus_p, s) / cross;
    auto u = crossProduct(q_minus_p, r) / cross;

    return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

// 检查多边形的边是否相交
bool polygonEdgesIntersect(const std::vector<Vector2>& corners)
{
    size_t n = corners.size();
    for (size_t i = 0; i < n; ++i) {
        Vector2 p1 = corners[i];
        Vector2 q1 = corners[(i + 1) % n];
        Vector2 q2 = corners[(i + 2) % n];
        Vector2 r = q1 - p1;
        Vector2 s = q2 - q1;
        auto cross = crossProduct(r, s);

        // 平行或共线的情况，计算点乘
        if (abs(cross) < G_EPS) {
            if (r.Dot2D(s) < 0) {
                DebugMsg("存在相邻两边之间的夹角过小: %d %d", i, (i + 1) % n);
                return true;
            }
        }
        for (size_t j = i + 2; j < n; ++j) {
            if ((j + 1) % n == i) {
                Vector2 q2 = corners[j % n];
                Vector2 r = p1 - q2;
                Vector2 s = q1 - p1;
                auto cross = crossProduct(r, s);

                // 平行或共线的情况，计算点乘
                if (abs(cross) < G_EPS) {
                    if (r.Dot2D(s) < 0) {
                        DebugMsg("存在相邻两边之间的夹角过小: %d %d", i, j);
                        return true;
                    }
                }
                continue;
            }
            Vector2 p2 = corners[j];
            Vector2 q2 = corners[(j + 1) % n];
            if (segmentsIntersect(p1, q1, p2, q2)) {
                DebugMsg("存在两边相交: %d %d", i, j);
                return true;
            }
        }
    }
    return false;
}

bool pointInPolygon(const std::vector<Vector2>& corners, const Vector2& point)
{
    int numIntersections = 0;
    size_t n = corners.size();

    // 遍历多边形的每一条边
    for (size_t i = 0; i < n; ++i) {
        Vector2 p1 = corners[i];
        Vector2 p2 = corners[(i + 1) % n];

        // 检查边与射线的相交情况
        if ((p1.y <= point.y && p2.y > point.y) || (p1.y > point.y && p2.y <= point.y)) {
            // 计算交点的 x 坐标
            double intersectX = (point.y - p1.y) / (p2.y - p1.y) * (p2.x - p1.x) + p1.x;

            // 如果交点在射线右侧，则计数加一
            if (intersectX > point.x)
                ++numIntersections;
        }
    }

    // 如果交点数量为奇数，则点在多边形内部
    return (numIntersections % 2 != 0);
}

double DistancePointToLine(const Vector2& point, const Vector2& line_start, const Vector2& line_vector)
{
    // 计算线段的长度
    double length_squared = line_vector.Length2D() * line_vector.Length2D();
    // 如果线段长度为0，则返回点到线段起点的距离
    if (length_squared == 0.0) {
        return point.Dist2D(line_start);
    }

    // 计算点到线段起点的向量
    Vector2 point_to_start = point - line_start;

    // 计算点到线段的投影长度
    double t = point_to_start.Dot2D(line_vector) / length_squared;

    // 计算投影点
    Vector2 projection = line_start + line_vector * t;

    // 如果投影在线段上，则点到线段的距离为点到投影点的距离
    if (t >= 0.0 && t <= 1.0) {
        return point.Dist2D(projection);
    }
    // 否则，点到线段的距离为点到线段起点和线段终点的最小距离
    else {
        double distance_to_start = point.Dist2D(line_start);
        double distance_to_end = point.Dist2D(line_start + line_vector);
        return std::min(distance_to_start, distance_to_end);
    }
}