#pragma once
#include <vector>
#include "math_utils.h"
#include "Table.h"
#include "Balls.h"
#include <chrono>

using GameClock = std::chrono::high_resolution_clock;
using GameTimePoint = std::chrono::time_point<GameClock>;
typedef double GameWorldTime;

class Game {
public:
    enum GameLevel {
        UNINIT = 0, // ��ȫû�г�ʼ����������û�С�
        TABLE_INITED = 1,// �Ѿ��������ˣ���û����Ҳ������������Լ�����״̬��δ��ʼ����
        GAME_RUN_STATIC = 2,// �ں������Ϸ�����У��������ʱ��ֹ
        GAME_RUN_MOVING = 3,// �ں������Ϸ�����У��������˶�
        GAME_RUN_SETTING = 4, // ��Ϸ�����У�����������
        GAME_OVER = 5, // ��Ϸ�ж�Ϊ����������δ�ͷš�
        GAME_JUDGING = 6, // �ж���
    };

    enum PlayerSide {
        UNDEF = 0,          // δ��ɫ
        PLAY_SINGLE_C = 1,  // ����ɫ��
        PLAY_DOUBLE_C = 2,  // ����˫ɫ��
    };


    // ����Ϸ״̬��صı���
    int gameState;
    // Note. ��ҵҪ�����Ѿ��򻯶�ɫ����
    // ����ǰֱ��Ϊ˫��������ɫ
    PlayerSide player_side[2] = { PLAY_SINGLE_C, PLAY_DOUBLE_C };
    int cur_player = 0; // ע��cur_playerȡֵΪ0��1��
    int winner = 0;     // ��winnerΪ0��ʾû��Ӯ�ң�Ϊ1��2��ʾʤ��

    // ��Ϸ����
    Table table;
    Balls balls;
    Vector3 cueSide;    // ĸ�����

    // ��ʱ����صı���
    GameWorldTime worldTime;
    GameTimePoint program_start_time;
    GameWorldTime last_time;

    // �뽻������Ⱦ��صı���
    bool mouse_clicked = false;

    /// <summary>
    /// ��������������е�λ��
    /// </summary>
    Vector2 mouse_pos;

    // ���캯��
    Game() :
        gameState(UNINIT), worldTime(0.0),
        program_start_time(GameClock::now()),
        last_time(-1.0) {}
    Game(const Table& table_, const Balls& balls_);

    // ��Ϸ����

    /// <summary>
    /// ��ʼ�����������Լ���Ա״̬
    /// </summary>
    /// <returns>trueΪ��ʼ���ɹ�</returns>
    bool initGame();

    /// <summary>
    /// �������λ�ã����ж϶Ծ�״̬
    /// </summary>
    void updateState();

    /// <summary>
    /// �������������ʱ���߼����������ĸ�������ĸ��
    /// </summary>
    void mouse_click();

    /// <summary>
    /// �ж���Ϸ�Ƿ����
    /// </summary>
    /// <returns>
    /// ����ֵΪ0��ʾδ������
    /// ����ֵΪ1��ʾ 1 ����һ�ʤ
    /// ����ֵΪ2��ʾ 2 ����һ�ʤ
    /// </returns>
    int judge();

    // ��Ⱦ���
    void render();
    void renderMouse();
};
