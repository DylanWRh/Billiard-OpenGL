#include "Ball.h"
#include "math_utils.h"
#include "render_utils.h"

Ball::Ball(
	const Vector2& position,
	const Vector2& velocity,
	const Vector3& color,
	const BallType& type
) :
	m_position(position),
	m_velocity(velocity),
	m_color(color),
	m_type(type),
	m_inHole(false)
{};

void Ball::render() const{
	if (m_inHole) {
		return;
	}
	if (m_type == SINGLE_C || m_type == BLACK || m_type == CUE) {
		draw_circle(
			m_position, (float)BALL_RADIUS,
			(float)m_color.x, (float)m_color.y, (float)m_color.z);
	}
	else if (m_type == DOUBLE_C) {
		draw_circle(
			m_position, (float)BALL_RADIUS,
			(float)m_color.x, (float)m_color.y, (float)m_color.z);
		draw_circle(
			m_position, (float)BALL_RADIUS / 3,
			1.0f, 1.0f, 1.0f);
	}
	return;
}