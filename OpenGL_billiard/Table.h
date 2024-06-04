#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "math_utils.h"
#include <vector>

class Table
{
public:
    enum InitMethod {
        INIT_REGULAR = 0,
        INIT_RANDOM = 1,
    };

    enum InitState {
        TABLEINIT_OK = 0,
        TABLEINIT_INVALID_CORNERS = 1,
        TABLEINIT_INVALID_HOLES = 2,
    };

    GLint n_corners;
    GLint n_holes;


    std::vector<Vector2> corners;
    std::vector<Vector2> holes;

    int checkInit() const;
    
    Table(): n_corners(0), n_holes(0) {};

    /// <summary>
    /// 创建多边形球桌，球洞将尽可能均匀地分布在各个角落
    /// </summary>
    /// <param name="n_corners">正多边形顶点数</param>
    /// <param name="n_holes">球洞个数</param>
    /// <param name="radius">球台顶点到中心的距离最小值，
    /// 若radius比max_r大，则以radius为半径绘制正多边形</param>
    /// <param name="max_r">可选，球台顶点到中心的距离最大值，
    /// 默认为0.0，表示绘制正多边形</param>
    Table(GLint n_corners, GLint n_holes, GLdouble radius, GLdouble max_r = 0.0);

    /// <summary>
    /// 绘制球桌和球洞
    /// </summary>
    void render();
};