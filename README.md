# Billiard-OpenGL

Term Project for the course **Computer Graphics**, 24 Spring, Peking University.

## Homework requirements

- [ ] 基本要求
  - [x] 相关球体运动方向是否和用力方向一致
  - [x] 台球进洞评判
  - [ ] 击球音效
  - [ ] 计分是否准确
- [ ] 模拟效果逼真度
- [ ] 用户界面友好程度
- [ ] 项目演示和报告质量
- [ ] 室内光照效果
- [ ] 3D 台球（附加分）

## Installation

### Environment
- Windows
- Visual Studio 2022
- OpenGL <link>[GLUT](https://www.opengl.org/resources/libraries/glut/glutdlls37beta.zip)

Pull it, then run it!

## Source files

### billiard_logic.cpp

Implementations of the core functions defined in `billiard_logic.h`. To implement the feature, there are some helper functions and some crucial global variables defined in it.
- `int billiard_logic::initTable(const std::vector<Vector2>& corners, const std::vector<Vector3>& holes)`

This function should be called first in order to create the polygonal billard table.

`corners`: A list holding all positions $(x, y)$ of the corners of the table.

`holes`: A list holding all positions $(x, y)$ and the radius of the holes of the table. `Vector3(x, y, radius)`

- `int billiard_logic::initBalls(const Vector2& white_position, const Vector2& center_of_triangle)`

This function should be called after `initTable` in order to place the balls in the initial positions on the polygonal billard table.

- `void billiard_logic::updateState()`

This function should be called in the display loop first in order to update the state of the game.

- `bool billiard_logic::isMoving()`

This function is called in order to check whether the balls are moving.

The players can't shot until the return is `false`.

- `void billiard_logic::shot(const ShotParam& param)`

Hit the white ball.

- `void billiard_logic::display()`

Render the table and the balls.

