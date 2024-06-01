#include "render_utils.h"
#include <cmath>

void draw_circle(const Vector2& pos, float radius, float r, float g, float b)
{
	glColor3f(r, g, b);
	glBegin(GL_POLYGON);
	for (int i = 0; i < 90; i++) {
		float theta = 2.0f * 3.1415926f * float(i) / float(90);
		float x = radius * cosf(theta);
		float y = radius * sinf(theta);
		glVertex2f(
			static_cast<float>(x + pos.x), 
			static_cast<float>(y + pos.y)
		);
	}
	glEnd();
}

void draw_poly(int num, const Vector2* const vex)
{
	glColor3f(0, 1.0, 0);
	glBegin(GL_POLYGON);
	for (int i = 0; i < num; ++i) {
		glVertex2f(
			static_cast<float>(vex[i].x), 
			static_cast<float>(vex[i].y)
		);
	}
	glEnd();
}

