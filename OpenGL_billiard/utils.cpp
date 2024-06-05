#include "utils.h"
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <random>
#include <chrono>

GLdouble getRandomDouble(GLdouble a, GLdouble b) {
    // 使用当前时间作为随机数生成器的种子，以获得更好的随机性
    unsigned seed = unsigned(std::chrono::system_clock::now().time_since_epoch().count());
    std::default_random_engine generator(seed);

    // 定义一个在[a, b)范围内的均匀分布
    std::uniform_real_distribution<GLdouble> distribution(a, b);

    // 生成并返回随机数
    return distribution(generator);
}

int getRandomInt(int a, int b) {
    // 使用当前时间作为随机数生成器的种子，以获得更好的随机性
    unsigned seed = unsigned(std::chrono::system_clock::now().time_since_epoch().count());
    std::default_random_engine generator(seed);

    // 定义一个在[a, b]范围内的均匀分布
    std::uniform_int_distribution<> dis(a, b);

    return dis(generator);
}

int getRandomIntWithoutC(int a, int b, int c) {
    // 使用当前时间作为随机数生成器的种子，以获得更好的随机性
    unsigned seed = unsigned(std::chrono::system_clock::now().time_since_epoch().count());
    std::default_random_engine generator(seed);

    // 定义一个在[a, b]范围内的均匀分布
    std::uniform_int_distribution<> dis(a, b);

    int res = 0;
    do {
        res = dis(generator);
    } while (res == c);
    return res;
}

void ErrorMsg(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    putchar('\n');
}