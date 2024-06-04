//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <thread>

#include "defs.h"
#include "billiard_logic.h"
#include "math_utils.h"


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;






void display(void)
{
	int i = 0;
	static bool first_run = true;
	if (first_run) {
		first_run = false;
		std::thread([]() {
			while (true)
				billiard_logic::updateState();
			}).detach();
	}

	glClear(GL_COLOR_BUFFER_BIT);

	billiard_logic::display();

	glutSwapBuffers();
}

void myinit()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-6, 6, -6, 6);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glColor3f(1.0, 0.0, 0.0);

#ifdef _DEBUG

	billiard_logic::test();

#else

	// 初始化正六边形球桌
	constexpr double OFFSET_HOLES = 0.1;
	std::vector<Vector2> corners;
	std::vector<Vector3> holes;
	srand(4);
	for (int i = 0; i < 6; ++i) {
		constexpr double PI3 = 3.14159265358979323846 / 3; // 圆周率π的数值表示
		double r = rand() % 85;
		r = (r + 15) / 15;
		corners.emplace_back(r * cos(i * PI3), r * sin(i * PI3));
		holes.emplace_back((r - OFFSET_HOLES) * cos(i * PI3), (r - OFFSET_HOLES) * sin(i * PI3), 0.25);
	}
	if (billiard_logic::initTable(corners, holes) != TABLEINIT_OK) {
		puts("初始化球桌失败");
		return;
	}

	// 初始化摆球位置
	Vector2 white_position(-2.5, 0.0);
	Vector2 triangle_center(2.5, 0.0);
	if (billiard_logic::initBalls(white_position, triangle_center) != BALLSINIT_OK) {
		puts("初始化球位置失败");
		return;
	}

	// 给白球初速度
	billiard_logic::shot({ 0.1, 100 });

#endif // _DEBUG
}

void idle()
{
	glutPostRedisplay();
}

void myReshape(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(-6, 6, -5, 5);///
	glViewport(-6, -6, 1200, 1000); ///
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}


int
main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1200, 1000);
	glutCreateWindow("Sierpinski Gasket by Recursion");
	glutReshapeFunc(myReshape); //
	glutDisplayFunc(display);
	myinit();
	glutIdleFunc(idle);
	glutMainLoop();
}
































//int main() {
//
//    billiard_logic::test();
//
//
//
//    glfwInit();
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
//    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
//    if (window == NULL)
//    {
//        std::cout << "Failed to create GLFW window" << std::endl;
//        glfwTerminate();
//        return -1;
//    }
//    glfwMakeContextCurrent(window);
//    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
//    {
//        std::cout << "Failed to initialize GLAD" << std::endl;
//        return -1;
//    }
//    while (!glfwWindowShouldClose(window))
//    {
//        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//        glClear(GL_COLOR_BUFFER_BIT);
//
//        // 在画面中心绘制实心圆
//        glMatrixMode(GL_MODELVIEW);
//        glLoadIdentity();
//        glTranslatef(SCR_WIDTH / 2, SCR_HEIGHT / 2, 0.0f);
//        glColor3f(1.0f, 0.5f, 0.2f);  // 设置圆的颜色为橙色
//        renderCircle();
//
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//    }
//    glfwTerminate();
//    return 0;
//}