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


	//��������ε�һ��
	glColor3f(1.0f, 0.0f, 0.0f);
	glStencilFunc(GL_ALWAYS, 1, 1);//��ʼģ��λΪ0������һ��ͨ�����ԣ�����ȫ���ᱻ��Ϊ1�����ظ��������ڻ�������ģ��λ�ֹ�0
	glStencilMask(0x1);//����ģ�建����д��
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);//��һ�����ֻ��Ϊ�˹���ģ�建�������������ò���ʾ��һ��Ķ����
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
	//��������εڶ���
	glStencilFunc(GL_NOTEQUAL, 0, 1);//ģ��ֵ��Ϊ0��ͨ������֮ǰ���ظ�����ᱻ��������������ξ���ȷ������
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

