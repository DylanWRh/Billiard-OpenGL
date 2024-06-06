#include "Ball.h"
#include "defs.h"
#include "utils.h"
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
{
	// 随机选定主轴方向
	main_axis = Vector3(
		getRandomDouble(0, 1),
		getRandomDouble(0, 1),
		getRandomDouble(0, 1)
	);
	main_axis.Normalize();
}

void Ball::ApplyRotation(double dt)
{
}

Sphere Ball::sphere(SUBDIVIDE_TETRA);

void Ball::render() const{
	if (m_inHole) {
		return;
	}

	// 开启光照
	glEnable(GL_LIGHTING);
	
	for (const auto& face : sphere.faces) {
		// 决定这个面的颜色
		bool iswhite = false;
		// 如果是双色球，面法向与主轴夹角余弦大于一个阈值，设为白色
		if (m_type == DOUBLE_C) {
			const Vector3& p1 = sphere.vertices[face[0]];
			const Vector3& p2 = sphere.vertices[face[1]];
			const Vector3& p3 = sphere.vertices[face[2]];

			Vector3 faceNormal = sphere.computeFaceNormal(p1, p2, p3);
			if (abs(faceNormal.Dot3D(main_axis)) > 0.6) {
				iswhite = true;
			}
		}

		// 设置面的颜色
		if (iswhite) {
			GLfloat mat_ambient[] = { 0.9f, 0.9f, 0.9f, 0.8f };
			GLfloat mat_diffuse[] = { 0.9f, 0.9f, 0.9f, 0.8f };
			GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 0.8f };
			GLfloat high_shininess[] = { 35.0f };

			glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
		}
		else {
			GLfloat mat_ambient[] = { (float)m_color.x * 0.9f, (float)m_color.y * 0.9f, (float)m_color.z * 0.9f, 0.8f };
			GLfloat mat_diffuse[] = { (float)m_color.x * 0.9f, (float)m_color.y * 0.9f, (float)m_color.z * 0.9f, 0.8f };
			GLfloat mat_specular[] = { 1.0f, 1.0f, 1.0f, 0.8f };
			GLfloat high_shininess[] = { 35.0f };

			glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
		}

		// 绘制这个面
		glBegin(GL_TRIANGLES);
		for (int idx : face) {
			glNormal3f(
				(float)sphere.vertexNormals[idx].x,
				(float)sphere.vertexNormals[idx].y,
				(float)sphere.vertexNormals[idx].z);
			glVertex3f(
				(float)(sphere.vertices[idx].x * BALL_RADIUS + m_position.x),
				(float)(sphere.vertices[idx].y * BALL_RADIUS + Y_BALL),
				(float)(sphere.vertices[idx].z * BALL_RADIUS + m_position.y));
		}
		glEnd();
	}

	// 关闭光照
	glDisable(GL_LIGHTING);

	/* 以下是原先的2D版本
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
	以上是原先的2D版本 */
	return;
}