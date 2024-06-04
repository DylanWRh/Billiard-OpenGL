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
	//glBegin(GL_TRIANGLES);
	//// TODO:
	//// https://blog.csdn.net/m0_52727721/article/details/128263015
	//for (int i = 0; i < num; ++i) {
	//	glVertex2f(
	//		0.0, 0.0
	//	);
	//	glVertex2f(
	//		static_cast<float>(vex[i].x),
	//		static_cast<float>(vex[i].y)
	//	);
	//	glVertex2f(
	//		static_cast<float>(vex[(i + 1) % num].x),
	//		static_cast<float>(vex[(i + 1) % num].y)
	//	);
	//}

	//glEnd();


	//画凹多边形第一遍
	glColor3f(1.0f, 0.0f, 0.0f);
	glStencilFunc(GL_ALWAYS, 1, 1);//初始模板位为0，由于一定通过测试，所以全部会被置为1，而重复区域由于画了两次模板位又归0
	glStencilMask(0x1);//开启模板缓冲区写入
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//第一遍绘制只是为了构造模板缓冲区，所以设置不显示第一遍的多边形
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < num; ++i) {
		glVertex2f(
			0.0, 0.0
		);
		glVertex2f(
			static_cast<float>(vex[i].x),
			static_cast<float>(vex[i].y)
		);
		glVertex2f(
			static_cast<float>(vex[(i + 1) % num].x),
			static_cast<float>(vex[(i + 1) % num].y)
		);
	}
	glEnd();
	//画凹多边形第二遍
	glStencilFunc(GL_NOTEQUAL, 0, 1);//模板值不为0就通过，即之前的重复区域会被舍弃掉，凹多边形就正确画出了
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glStencilMask(0x1);
	glColor3f(0.0f, 1.0f, 0.0f);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBegin(GL_TRIANGLE_FAN);
	for (int i = 0; i < num; ++i) {
		glVertex2f(
			0.0, 0.0
		);
		glVertex2f(
			static_cast<float>(vex[i].x),
			static_cast<float>(vex[i].y)
		);
		glVertex2f(
			static_cast<float>(vex[(i + 1) % num].x),
			static_cast<float>(vex[(i + 1) % num].y)
		);
	}
	glEnd();

}

