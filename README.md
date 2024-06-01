# Billiard-OpenGL

Term Project for the course **Computer Graphics**, 24 Spring, Peking University.

## Installation

Pull it, then run it!

## Source files

### billiard_logic.cpp

Implementations of the core functions defined in `billiard_logic.h`. To implement the feature, there are some helper functions and some crucial global variables defined in it.
- `int billiard_logic::initTable(const std::vector<Vector2>& corners, const std::vector<Vector3>& holes)`

This function should be called first in order to create the polygonal billard table.

`corners`: A list holding all positions $(x, y)$ of the corners of the table.

`holes`: A list holding all positions $(x, y)$ and the radius of the holes of the table. `Vector3(x, y, radius)`

- `int billiard_logic::initBalls(const Vector2& white_position, const Vector2& center_of_triangle)`
- `void billiard_logic::updateState()`
- `bool billiard_logic::isMoving()`
- `void billiard_logic::shot(const ShotParam& param)`
- `void billiard_logic::display()`
