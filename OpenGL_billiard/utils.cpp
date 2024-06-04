#include "utils.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <random>
#include <chrono>

GLdouble getRandomDouble(GLdouble a, GLdouble b) {
    // ʹ�õ�ǰʱ����Ϊ����������������ӣ��Ի�ø��õ������
    unsigned seed = unsigned(std::chrono::system_clock::now().time_since_epoch().count());
    std::default_random_engine generator(seed);

    // ����һ����[a, b)��Χ�ڵľ��ȷֲ�
    std::uniform_real_distribution<GLdouble> distribution(a, b);

    // ���ɲ����������
    return distribution(generator);
}

void ErrorMsg(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    putchar('\n');
}