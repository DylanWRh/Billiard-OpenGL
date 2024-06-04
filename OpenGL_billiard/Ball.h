#pragma once
#include "math_utils.h"
#include "defs.h"

enum BallType
{
    WHITE = 0,
    BLACK = 1,
    RED = 2
};

struct Ball
{
    bool m_inHole;
    Vector2 m_position;
    Vector2 m_velocity;
    BallType m_type;
    Ball(const Vector2& position, const Vector2& velocity, const BallType& type) : m_position(position), m_velocity(velocity), m_type(type), m_inHole(false) {}
};
