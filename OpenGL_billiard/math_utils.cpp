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
        // TODO: ƽ�л��ߵ������������
        //if (r.Dot2D(s) < 0) {
        //    return abs(crossProduct(q_minus_p, r)) > G_EPS;
        //}
        return false;
    }

    auto t = crossProduct(q_minus_p, s) / cross;
    auto u = crossProduct(q_minus_p, r) / cross;

    return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}

// ������εı��Ƿ��ཻ
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

        // ƽ�л��ߵ������������
        if (abs(cross) < G_EPS) {
            if (r.Dot2D(s) < 0) {
                DebugMsg("������������֮��ļнǹ�С: %d %d", i, (i + 1) % n);
                return true;
            }
        }
        for (size_t j = i + 2; j < n; ++j) {
            if ((j + 1) % n == i) {
                Vector2 q2 = corners[j % n];
                Vector2 r = p1 - q2;
                Vector2 s = q1 - p1;
                auto cross = crossProduct(r, s);

                // ƽ�л��ߵ������������
                if (abs(cross) < G_EPS) {
                    if (r.Dot2D(s) < 0) {
                        DebugMsg("������������֮��ļнǹ�С: %d %d", i, j);
                        return true;
                    }
                }
                continue;
            }
            Vector2 p2 = corners[j];
            Vector2 q2 = corners[(j + 1) % n];
            if (segmentsIntersect(p1, q1, p2, q2)) {
                DebugMsg("���������ཻ: %d %d", i, j);
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

    // ��������ε�ÿһ����
    for (size_t i = 0; i < n; ++i) {
        Vector2 p1 = corners[i];
        Vector2 p2 = corners[(i + 1) % n];

        // ���������ߵ��ཻ���
        if ((p1.y <= point.y && p2.y > point.y) || (p1.y > point.y && p2.y <= point.y)) {
            // ���㽻��� x ����
            double intersectX = (point.y - p1.y) / (p2.y - p1.y) * (p2.x - p1.x) + p1.x;

            // ��������������Ҳ࣬�������һ
            if (intersectX > point.x)
                ++numIntersections;
        }
    }

    // �����������Ϊ����������ڶ�����ڲ�
    return (numIntersections % 2 != 0);
}

double DistancePointToLine(const Vector2& point, const Vector2& line_start, const Vector2& line_vector)
{
    // �����߶εĳ���
    double length_squared = line_vector.Length2D() * line_vector.Length2D();
    // ����߶γ���Ϊ0���򷵻ص㵽�߶����ľ���
    if (length_squared == 0.0) {
        return point.Dist2D(line_start);
    }

    // ����㵽�߶���������
    Vector2 point_to_start = point - line_start;

    // ����㵽�߶ε�ͶӰ����
    double t = point_to_start.Dot2D(line_vector) / length_squared;

    // ����ͶӰ��
    Vector2 projection = line_start + line_vector * t;

    // ���ͶӰ���߶��ϣ���㵽�߶εľ���Ϊ�㵽ͶӰ��ľ���
    if (t >= 0.0 && t <= 1.0) {
        return point.Dist2D(projection);
    }
    // ���򣬵㵽�߶εľ���Ϊ�㵽�߶������߶��յ����С����
    else {
        double distance_to_start = point.Dist2D(line_start);
        double distance_to_end = point.Dist2D(line_start + line_vector);
        return std::min(distance_to_start, distance_to_end);
    }
}