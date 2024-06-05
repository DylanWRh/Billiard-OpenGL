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
			m_position, BALL_RADIUS,
			m_color.x, m_color.y, m_color.z);
	}
	else if (m_type == DOUBLE_C) {
		draw_circle(
			m_position, BALL_RADIUS,
			m_color.x, m_color.y, m_color.z);
		draw_circle(
			m_position, BALL_RADIUS / 3,
			1.0, 1.0, 1.0);
	}
	return;
}