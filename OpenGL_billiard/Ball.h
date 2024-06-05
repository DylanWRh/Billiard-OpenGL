#pragma once
#include "math_utils.h"
#include "Sphere.h"
#include "defs.h"

class Ball
{
public:
    enum BallType
    {
        CUE = 0,        // Ä¸Çò
        SINGLE_C = 1,   // ´¿É«Çò
        DOUBLE_C = 2,   // Ë«É«Çò
        BLACK = 3,      // ºÚÉ«Çò
    };
    typedef Vector3 BallColor;
    bool m_inHole;
    Vector2 m_position;
    Vector2 m_velocity;
    BallType m_type;

    // äÖÈ¾Ïà¹Ø
    BallColor m_color;
    Vector3 main_axis;  // Ö÷Öá
    static Sphere sphere;

    Ball(
        const Vector2& position, 
        const Vector2& velocity,
        const Vector3& color,
        const BallType& type
    );

    void render() const;
};
