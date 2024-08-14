
#pragma once

#include <unordered_set>
#include <string>


// ��Ϸ�����  add by Philip.Wu  2006-07-06
class CGameWG
{
public:
	CGameWG(void);
	~CGameWG(void);

	// ˢ�µ�ǰ�������ģ��
	bool RefreshModule(void);

	// �Ƿ�ʹ���ˡ�������ʹ�����
	bool IsUseHdts(void);

	// �����߳�
	void BeginThread(void);

	// ��ȫ��ֹ�߳�
	void SafeTerminateThread();

private:

	// ���ģ���б�
	std::unordered_set<std::string> m_lstModule;

	// �߳̾��
	HANDLE m_hThread;

	// �̻߳ص�
	static UINT CALLBACK Run(void* param);

};

