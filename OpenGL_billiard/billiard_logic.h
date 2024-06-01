#pragma once
#include <vector>
#include "math_utils.h"

namespace billiard_logic {

	/// <summary>
	/// �������Ǽ򵥽�����Ĳ�������Ϊ(����,����)������Ϊ0ʱ��x��������ͬ��Ϊpi/2ʱ��y��������ͬ��
	/// </summary>
	/// <remarks>
	/// ����������չ���������
	/// </remarks>
	typedef Vector2 ShotParam;

	/// <summary>
	/// ��ʼ�������̨���������趨��̨������������Ϸ��ʼǰ���á�
	/// </summary>
	/// <param name="corners">����������ǵ�λ��</param>
	/// <param name="holes">��������λ�ã�����Ϊ(x, y, r)��rΪ�뾶</param>
	/// <returns>
	///		TABLEINIT_OK: �ɹ���
	///		TABLEINIT_INVALID_CORNERS: ������������㲻�Ϸ���
	///		TABLEINIT_INVALID_HOLES: ����λ�ò��Ϸ���
	/// </returns>
	int initTable(const std::vector<Vector2>& corners, const std::vector<Vector3>& holes);

	/// <summary>
	/// ���������η����ɰ���λ�õ�����������λ�ù��ɵ�����������
	/// </summary>
	/// <param name="white_position">����λ��</param>
	/// <param name="center_of_triangle">������ڵ�������������λ��</param>
	/// <returns>
	///		BALLSINIT_OK: �Ϸ���ʼ�����ɹ���
	///		BALLSINIT_INVALID_WHITE: ��������̨�ڣ�
	///		BALLSINIT_INVALID_CENTER: �ںõ���������������̨�ڣ�
	///		BALLSINIT_INVALID_DISTENCE: ������ںõ������ξ��������
	/// </returns>
	int initBalls(const Vector2& white_position, const Vector2& center_of_triangle);

	/// <summary>
	/// ������������״̬����֡��������á�
	/// </summary>
	void updateState();

	/// <summary>
	/// ��������ϵ����Ƿ����˶�
	/// </summary>
	/// <returns>true: �����˶������ɻ���false: �Ѿ�ȫ����ֹ�����Ի���</returns>
	bool isMoving();

	/// <summary>
	/// ����������Ϊ�����ı���������״̬��
	/// </summary>
	/// <param name="param">����Ļ��������</param>
	void shot(const ShotParam& param);

	/// <summary>
	/// ������̨�Ľӿڡ�
	/// </summary>
	void display();

#ifdef _DEBUG
	/// <summary>
	/// ����ģʽ�¶���Ĳ��Ժ��������ڼ�麯��������ȷ�ԡ�����billiard_logic.cpp�ж���
	/// </summary>
	void test();
#endif
}
