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
    /// ����������������򶴽������ܾ��ȵطֲ��ڸ�������
    /// </summary>
    /// <param name="n_corners">������ζ�����</param>
    /// <param name="n_holes">�򶴸���</param>
    /// <param name="radius">��̨���㵽���ĵľ�����Сֵ��
    /// ��radius��max_r������radiusΪ�뾶�����������</param>
    /// <param name="max_r">��ѡ����̨���㵽���ĵľ������ֵ��
    /// Ĭ��Ϊ0.0����ʾ�����������</param>
    Table(GLint n_corners, GLint n_holes, GLdouble radius, GLdouble max_r = 0.0);

    /// <summary>
    /// ������������
    /// </summary>
    void render();
};