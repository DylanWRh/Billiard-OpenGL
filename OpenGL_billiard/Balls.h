#pragma once
#include "Ball.h"
#include "Table.h"
#include "math_utils.h"
#include <vector>

class Balls {
public:
	enum InitState {
		BALLSINIT_OK = 0,
		BALLSINIT_INVALID_WHITE = 1,
		BALLSINIT_INVALID_CENTER = 2,
		BALLSINIT_INVALID_DISTENCE = 3,
	};
	std::vector<Ball> balls;
	int cue_pos = -1;
	int black_pos = -1;
	int first_hit_pos = -1;
	std::vector<int> goals{};

	Balls() {};
	Balls(const Vector2& white_position, const Vector2& center_of_triangle);
	// Balls(const Vector2& white_position, const Vector2& center_of_triangle, int triangle_length);
	
	int checkInit(const Table& table);
	void render() const;

	/// <summary>
	/// 更新球的速度和位置
	/// </summary>
	/// <returns>是否所有球都已经停下</returns>
	bool update(const Table& table, double delta_t);
};