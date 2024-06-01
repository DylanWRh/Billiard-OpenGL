#pragma once
#include <vector>
#include "math_utils.h"

namespace billiard_logic {

	/// <summary>
	/// 这里我们简单将击球的参数定义为(方向,力度)。方向为0时与x正方向相同。为pi/2时与y正方向相同。
	/// </summary>
	/// <remarks>
	/// 后续可以拓展击球参数。
	/// </remarks>
	typedef Vector2 ShotParam;

	/// <summary>
	/// 初始化多边形台球桌。在设定好台球桌参数后，游戏开始前调用。
	/// </summary>
	/// <param name="corners">多边形球桌角的位置</param>
	/// <param name="holes">球桌洞的位置，依次为(x, y, r)，r为半径</param>
	/// <returns>
	///		TABLEINIT_OK: 成功，
	///		TABLEINIT_INVALID_CORNERS: 多边形球桌顶点不合法，
	///		TABLEINIT_INVALID_HOLES: 洞的位置不合法。
	/// </returns>
	int initTable(const std::vector<Vector2>& corners, const std::vector<Vector3>& holes);

	/// <summary>
	/// 摆球。三角形方向由白球位置到三角形中心位置构成的向量决定。
	/// </summary>
	/// <param name="white_position">白球位置</param>
	/// <param name="center_of_triangle">其他球摆的正三角形中心位置</param>
	/// <returns>
	///		BALLSINIT_OK: 合法初始化，成功，
	///		BALLSINIT_INVALID_WHITE: 白球不在球台内，
	///		BALLSINIT_INVALID_CENTER: 摆好的三角形有球不在球台内，
	///		BALLSINIT_INVALID_DISTENCE: 白球与摆好的三角形距离过近。
	/// </returns>
	int initBalls(const Vector2& white_position, const Vector2& center_of_triangle);

	/// <summary>
	/// 更新球桌物理状态，在帧结束后调用。
	/// </summary>
	void updateState();

	/// <summary>
	/// 检测球桌上的球是否还在运动
	/// </summary>
	/// <returns>true: 还在运动，不可击球；false: 已经全部静止，可以击球。</returns>
	bool isMoving();

	/// <summary>
	/// 击打白球的行为，将改变白球的物理状态。
	/// </summary>
	/// <param name="param">传入的击球参数。</param>
	void shot(const ShotParam& param);

	/// <summary>
	/// 绘制球台的接口。
	/// </summary>
	void display();

#ifdef _DEBUG
	/// <summary>
	/// 调试模式下定义的测试函数，用于检查函数功能正确性。请在billiard_logic.cpp中定义
	/// </summary>
	void test();
#endif
}
