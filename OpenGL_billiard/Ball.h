#pragma once
#include "math_utils.h"
#include "Sphere.h"
#include "defs.h"

class Ball
{
public:
    enum BallType
    {
        CUE = 0,        // ĸ��
        SINGLE_C = 1,   // ��ɫ��
        DOUBLE_C = 2,   // ˫ɫ��
        BLACK = 3,      // ��ɫ��
    };
    typedef Vector3 BallColor;
    bool m_inHole;
    Vector2 m_position;
    Vector2 m_velocity;
    BallType m_type;

    // ��Ⱦ���
    BallColor m_color;
    Vector3 main_axis;  // ����
    static Sphere sphere;

    Ball(
        const Vector2& position, 
        const Vector2& velocity,
        const Vector3& color,
        const BallType& type
    );

    void render() const;
};
