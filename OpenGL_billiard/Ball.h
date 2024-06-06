#pragma once
#include "math_utils.h"
#include "Sphere.h"
#include "defs.h"

class Ball
{
public:
    enum BallType
    {
        CUE = 0,        // 母球
        SINGLE_C = 1,   // 纯色球
        DOUBLE_C = 2,   // 双色球
        BLACK = 3,      // 黑色球
    };
    typedef Vector3 BallColor;
    bool m_inHole;
    Vector2 m_position;
    Vector2 m_velocity;
    Vector3 m_angular_velocity;
    BallType m_type;

    // 渲染相关
    BallColor m_color;
    Vector3 main_axis;  // 主轴
    static Sphere sphere;

    Ball(
        const Vector2& position, 
        const Vector2& velocity,
        const Vector3& color,
        const BallType& type
    );

    /// <summary>
    /// 计算与球台接触点的线速度（球心指向球台的向量，n = Vector3(0.0, -BALL_RADIUS, 0.0) ）
    /// </summary>
    /// <returns>相对于球心的线速度</returns>
    Vector3 PerimeterSpeed() const noexcept
    {
        return m_angular_velocity.Cross(Vector3{ 0.0, -BALL_RADIUS, 0.0 });
    }

    /// <summary>
    /// 计算与某个接触点的线速度
    /// </summary>
    /// <param name="normal">球心指向与本球接触点的方向单位向量</param>
    /// <returns>相对于球心的线速度</returns>
    Vector3 PerimeterSpeed(const Vector3& normal) const noexcept
    {
        return m_angular_velocity.Cross(normal * BALL_RADIUS);
    }

    void ApplyRotation(double dt);

    void render() const;
};
