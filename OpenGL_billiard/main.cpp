//#include <glad/glad.h>
//#include <GLFW/glfw3.h>
#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "billiard_logic.h"


const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;






void display(void)
{
	int i = 0;
	billiard_logic::updateState();

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
	gluOrtho2D(-6, 6, -6, 6);///
	glViewport(-6, -6, 1200, 1200); ///
	glMatrixMode(GL_MODELVIEW);
	glutPostRedisplay();
}


void
main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(1200, 1200);
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