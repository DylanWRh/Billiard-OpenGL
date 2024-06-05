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
	// ���ѡ�����᷽��
	main_axis = Vector3(
		getRandomDouble(0, 1),
		getRandomDouble(0, 1),
		getRandomDouble(0, 1)
	);
	main_axis.Normalize();
};

Sphere Ball::sphere(SUBDIVIDE_TETRA);

void Ball::render() const{
	if (m_inHole) {
		return;
	}

	// ��������
	glEnable(GL_LIGHTING);
	
	for (const auto& face : sphere.faces) {
		// ������������ɫ
		bool iswhite = false;
		// �����˫ɫ���淨��������н����Ҵ���һ����ֵ����Ϊ��ɫ
		if (m_type == DOUBLE_C) {
			const Vector3& p1 = sphere.vertices[face[0]];
			const Vector3& p2 = sphere.vertices[face[1]];
			const Vector3& p3 = sphere.vertices[face[2]];

			Vector3 faceNormal = sphere.computeFaceNormal(p1, p2, p3);
			if (abs(faceNormal.Dot3D(main_axis)) > 0.6) {
				iswhite = true;
			}
		}

		// ���������ɫ
		if (iswhite) {
			GLfloat mat_ambient[] = { 0.7, 0.7, 0.7, 1.0 };
			GLfloat mat_diffuse[] = { 0.7, 0.7, 0.7, 1.0 };
			GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
			GLfloat high_shininess[] = { 35.0 };

			glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
		}
		else {
			GLfloat mat_ambient[] = { m_color.x * 0.7, m_color.y * 0.7, m_color.z * 0.7, 0.8 };
			GLfloat mat_diffuse[] = { m_color.x * 0.7, m_color.y * 0.7, m_color.z * 0.7, 0.8 };
			GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 0.8 };
			GLfloat high_shininess[] = { 50.0 };

			glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
			glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
			glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
			glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
		}

		// ���������
		glBegin(GL_TRIANGLES);
		for (int idx : face) {
			glNormal3f(
				sphere.vertexNormals[idx].x, 
				sphere.vertexNormals[idx].y, 
				sphere.vertexNormals[idx].z);
			glVertex3f(
				sphere.vertices[idx].x * BALL_RADIUS + m_position.x,
				sphere.vertices[idx].y * BALL_RADIUS + Y_BALL,
				sphere.vertices[idx].z * BALL_RADIUS + m_position.y);
		}
		glEnd();
	}

	// �رչ���
	glDisable(GL_LIGHTING);

	/* ������ԭ�ȵ�2D�汾
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
	������ԭ�ȵ�2D�汾 */
	return;
}