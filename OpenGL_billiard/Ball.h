#pragma once
#include "math_utils.h"
#define BALL_RADIUS 0.15

enum BallType
{
	IN_HOLE = -1,
	WHITE	= 0,
	BLACK	= 1,
	RED		= 2
};

struct Ball
{
	Vector2 m_position;
	Vector2 m_velocity;
	BallType m_type;
	Ball(const Vector2& position, const Vector2& velocity, const BallType& type) : m_position(position), m_velocity(velocity), m_type(type) {}
};
