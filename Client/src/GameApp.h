#pragma once

#define USE_DSOUND

#include "MPGameApp.h"
#include "script.h"
#include "CRCursorObj.h"
#include "RenderStateMgr.h"
#include "steadyframe.h"
#include "CameraCtrl.h"

#ifdef USE_DSOUND
#include "DSoundManager.h"
#include "DSoundInstance.h"
#endif

#include "AudioSDL.h"
#include "chastate.h"
#include "UIPicture.h"

extern DWORD g_dwCurMusicID;

const DWORD MAX_ANI_CLOCK = 10;             // 时间沙漏个数

namespace Ninja
{
	class Camera;
	struct SphereCoord;
	template < class T >
	class Controller;
}

class CSceneObj;

#define	  SHOWRSIZE		    40					// Original: 40
#define   TIP_TEXT_NUM      128
#define   SHOW_TEXT_TIME	3000
#define   MAX_CQUEUE		7
#define   TIME_CQUEUE		3000

//#define TESTDEMO

namespace GUI
{
	class CTextHint;
	class CTextScrollHint;//Add by sunny.sun20080804
};

enum eSceneType
{
	enumLoginScene     = 0, // ��¼����
	enumWorldScene     = 1,	// ��Ϸ����
	enumSelectChaScene = 2,	// ѡ������
	enumCreateChaScene = 3, // ��������
	enumSceneEnd,
};

struct STipText
{
    char  szText[TIP_TEXT_NUM];
    DWORD dwBeginTime;
    BYTE  btAlpha;
};

class CSteadyFrame;
class CGameScene;
class CMPShadeMap;
class CPointTrack;
class CEffectObj;
class CActor;
class CDrawPointList;
class CCameraCtrl;
class CAniClock;

struct stSceneInitParam;

struct SAddSceneObj
{
    int nTypeID;
    int nPosX;
    int nPosY;
    int nHeightOff;
    int nAngle;
};



class CGameApp : public MPGameApp
{
public:
	CGameApp();
	~CGameApp();

    void    End();

    virtual void		MouseButtonDown(int nButton);
	virtual void		MouseButtonUp(int nButton);
	virtual void		MouseButtonDB(int nButton);
	virtual void		MouseMove(int nOffsetX, int nOffsetY);
	virtual void		MouseScroll(int nScroll);
	virtual void		MouseContinue(int nButton);
	virtual void		HandleKeyDown(DWORD dwKey);
    virtual void		LG_Config(const LGInfo& info);

	bool				LoadRes4();

    void				SetIsRun( bool v ) { if(v != _isRun) { _isRun = v; if(v == 0) { ::SendMessage(_hWnd, WM_DESTROY, 0, 0); } } }
    BOOL                IsRun() { return _isRun; }
	void				SetIsRenderTipText( bool v ){ _IsRenderTipText = v;		}
	bool				GetIsRenderTipText()		{ return _IsRenderTipText;	}
	
	void				InitAllTable();			// �������еı����ı�
	void				ReleaseAllTable();		// �ͷ����еı����ı�
	

	void	            HandleKeyContinue();
	bool	            HandleWindowMsg(DWORD dwMsg, DWORD dwParam1, DWORD dwParam2);

	void                ChangeVideoStyle( int width , int height ,D3DFORMAT format , bool bWindowed  );//by billy

	void	            EnableCameraFollow(BOOL bEnable)	{ _bCameraFollow = bEnable;		} 
	BOOL	            IsCameraFollow()					{ return _bCameraFollow;		}
    void                ResetGameCamera(int type = 0);
	void                ResetCamera();

	CPointTrack*		GetCameraTrack()					{ return _pCamTrack;			}

    void                HandleSuperKey();
    void                HandleContinueSuperKey();
    void                EnableSuperKey(BOOL bEnable)		{ _bEnableSuperKey = bEnable;	}
	BOOL                IsEnableSuperKey();              
    void                PlayMusic(int nMusicNo);
	void				PlaySound(int nSoundNo);

	void				SendMessage( DWORD dwTypeID, DWORD dwParam1=0, DWORD dwParam2=0 );

	bool				IsInit()							{ return _IsInit;				}

	bool				HasLogFile( const char* log_file, bool isOpen=true );

	void				Loading( int nFrame=40 );				// ����Ϊ����֡
    static void         Waiting( bool isWaiting=true );         // ��ʾ��һ���ȴ��Ի���,����䲻�ܲ���UI
	static bool			IsMouseContinue(int nButton)		{ return _dwMouseDownTime[nButton]>12;	}

    void                AddTipText(const char *pszFormat, ...);
	void				SysInfo( const char *pszFormat, ... );
	void				ShowNotify( const char *szStr, DWORD dwColor );
	void				ShowNotify1( const char *szStr,int setnum, DWORD dwColor );//Add by sunny.sun20080804
	void				ShowHint( int x, int y, const char *szStr, DWORD dwColor );
	void				ShowStateHint( int x, int y, CChaStateMgr::stChaState stateData);
	
    static void         SetMusicSize( float fVol );         // 0~1,0������,1�������
    static float        GetMusicSize()                      { return (float)_nMusicSize / 128.0f;   }

    static void         MsgBox( const char *pszFormat, ... );

    void                ShowBigText( const char *pszFormat, ... );
    void                ShowMidText( const char *pszFormat, ... );
	void                ShowBottomText( unsigned int rgb, const char *pszFormat, ... );

	// �л�����begin
	CGameScene*	CreateScene( stSceneInitParam* param );
	void		GotoScene( CGameScene* scene, bool isDelCurScene=true, bool IsShowLoading=true );		// ֱ���л�����һ������
	int			Run();
	static CGameScene*	GetCurScene()					{ return _pCurScene;	}	// ��õ�ǰ����
	// �л�����end

	void		CreateCharImg();

	void		RefreshLoadingProgress();
	
public:	// �ű���������
	void		LoadScriptScene( eSceneType eType );
	void		LoadScriptScene( const char* script_file );

	bool		btest;

	int			ihei;
public:
	static DWORD		GetCurTick() { return _dwCurTick; }
	void				SetTickCount(DWORD dwTick)		{ _dwCurTick = dwTick;  }
	void				SetFPSInterval( DWORD v );
	static DWORD        GetFrameFPS();

	static CSteadyFrame* GetFrame()						{ return _pSteady;		}
	static void			 SetFrame(bool v)				{ _pSteady->SetFramerate60(v);}
	// �����ļ�����ʼ����ǰ������xuedong 2004.09.06 ���ڡ������ϰ���Ϣ�ļ���
	BOOL				CreateCurrentScene(char *szMapName);

	CursorMgr*			GetCursor()						{ return &_stCursorMgr;	}

	CDrawPointList*		GetDrawPoints()					{ return _pDrawPoints;	}

	void				OnLostDevice();
	void				OnResetDevice();

	CCameraCtrl*		GetMainCam()					{ return _pMainCam;		} 
    static bool         IsMusicSystemValid()            { return _IsMusicSystemValid;       }

	CMPFont*			GetFont()						{return &g_CFont;		}
    RenderStateMgr*     GetRenderStateMgr()             { return _rsm;			}

	void				SetCameraPos(D3DXVECTOR3& pos,bool bRestoreCustom=true);	// bRestoreCustom����Ϊtrue����ʾ����û��Ծ�ͷ�ĸı�

	void				SetStartMinimap(int  ix, int iy, int destx,int desty);
	CScriptMgr*			GetScriptMgr()					{ return &_stScriptMgr;				}
	void				ResetCaption();

	void				AutoTest();			// �Զ�������
	void				AutoTestInfo( const char *pszFormat, ... );		// �����Զ�������ʱ��ʾ��������
	void				AutoTestUpdate();

	static bool			IsMouseInScene()				{ return _MouseInScene;				}

	CAniClock*			AddAniClock();

	std::map<int, DWORD> m_mapSkillClock;
	DWORD				GetSkillClock(int skill_id);
	void				SetSkillClock(int skill_id, DWORD dwSkillTime);
	void				DeleteSkillClock(int skill_id);
	void				ClearAllSkillClocks();


	//���ڼ�¼/��ȡ��½��Ϸ��Tickʱ��
	static void SetLoginTime(DWORD _dwLoginTime)		{ m_dwLoginTime=_dwLoginTime;		}
	static DWORD GetLoginTime()							{ return m_dwLoginTime;				}

public:    
    std::list<SAddSceneObj*>  m_AddSceneObjList;
    DWORD                m_dwRenderUITime;
    DWORD                m_dwRenderSceneTime;
    DWORD                m_dwRenderScneObjTime;
    DWORD                m_dwRenderChaTime;
    DWORD                m_dwRenderEffectTime;
    DWORD                m_dwLoadingObjTime;
    DWORD                m_dwTranspObjTime;
    DWORD                m_dwRenderMMap;
    DWORD                m_dwPathFinding;
	
	BOOL			m_bRenderFlash;

#ifdef USE_DSOUND
	SoundManager*	mSoundManager;
	
	void			PlaySample( std::string SoundName );

#endif
	
protected:
	virtual BOOL        _Init();
	virtual void		_PreMouseRun( DWORD dwMouseKey );
    virtual void	    _FrameMove(DWORD dwTimeParam, bool camMove);		//ViM
	virtual void	    _Render();
    virtual void        _End();

	BOOL				_PrintScreen();
	BOOL				_CreateAviScreen();
	bool				_IsSceneOk() { return _pCurScene!=NULL; }
	BOOL				_CreateSmMap( MPTerrain* pTerr );
    void                _RenderTipText();
    void                _ShowLoading(int percent);

protected:
	
    BOOL			    _bCameraFollow;
    BOOL                _bEnableSuperKey;
	BOOL			    _bConnected;
    CPointTrack*		_pCamTrack;
	CDrawPointList*		_pDrawPoints;

    std::list<std::unique_ptr<STipText>> _TipText;

	CCameraCtrl*		_pMainCam;

public:
    RenderStateMgr*     _rsm;
#if(defined USE_TIMERPERIOD)
    MPITimerPeriod*     _TimerPeriod;
#endif
	float	xp,xp1;
	float	yp,yp1;

	float	destxp;
	float	destyp;

protected:
	void	_SceneError( const char* info, CGameScene * p );
	void	_HandleMsg(DWORD dwTypeID, DWORD dwParam1, DWORD dwParam2);

	CGameScene*	_pStartScene;		// ��һ������
	static CGameScene*	_pCurScene;	// Ŀǰ�������еĳ���
	static DWORD		_dwCurTick;

	int			_nSwitchScene;		// ������,�����л�����

	bool		_isRun;		

	CScriptMgr	_stScriptMgr;
	CursorMgr	_stCursorMgr;

	bool		_IsRenderTipText;
	bool 		_IsRenderColourTest;

    static CAniClock*	_AniClock;
	
private:		// ���ڽű�
	bool				_IsInit;
    DWORD               _dwGameThreadID;

	static DWORD		_dwMouseDownTime[2];

    static int          _nMusicSize;        // ��������
    bool                _IsUserEnabled;     // �Ƿ�����û�����

    static char         _szOutBuf[256];

	CMPFont				g_CFont;
    CMPFont             _MidFont;         // ����Ļ����ƫ�ϵ�һ���е�����
    STipText            _stMidFont;
	CMPFont             _BottomFont;

	//int					_nCQueueIndex;
	//STipText*			_sCQueue[MAX_CQUEUE];
	//unsigned long 		_iCQueueColour[MAX_CQUEUE];
	std::queue< std::pair<STipText*, unsigned int> >	_qCQueueStrColour;

	//void UpdateColourQueue();
	void RenderColourQueue();

    static bool         _IsMusicSystemValid; 

private:        // �������ֵ��л�,���������׶�:1.��ǰ����������С,2.Ϊ��ʱ�л���������,�������ֱ��,3.��������������
    enum eBkgMusic { enumNoMusic, enumOldMusic, enumNewMusic, enumMusicPlay };
    eBkgMusic           _eSwitchMusic;
    int                 _nCurMusicSize;     // �л�����ʱ��������С
    char                _szBkgMusic[256];   // ������
    
	static CSteadyFrame*	_pSteady;
	static	bool			_MouseInScene;
	
	CTextHint			*_pNotify;
	CTextScrollHint		*_pNotify1;//Add by sunny.sun20080804
	DWORD				_dwNotifyTime;
	DWORD				_dwNotifyTime1;//Add by sunny.sun20080804
	
	int					SetNum;
	int					_total;

	static DWORD		m_dwLoginTime;		//��¼��½��Ϸ��Tickʱ��

	DWORD				_dwLoadingTick;		// ��ʼ Loading ��ʱ��


public:
	//static struct 		Application	*app;


	Ninja::Camera*		GetNinjaCamera()	{ return _pNinjaCamera; }
//	Ninja::Camera*		_pNinjaCamera;
//	Ninja::Controller < D3DXVECTOR3 > *_ctrl;

	// Added by CLP
	private:
	Ninja::Camera*		_pNinjaCamera;
	Ninja::Controller < D3DXVECTOR3 > *_camera_target_ctrl;
	Ninja::Controller < Ninja::SphereCoord > *_camera_eye_ctrl;
};

class CAniClock
{
public:
	CAniClock();
	~CAniClock();

	bool			Create(int width,DWORD	dwColor = 0x80ffffff);
	void			MoveTo(int x, int y);

	bool			IsEnd()		{return !_bUpdate;}
	float			RemainingTime() const;
	void			Play(DWORD dwPlayTime);

	int				GetID()			{return _iID;}
	void			SetID(int iID)	{_iID = iID;}

	void			Resume(DWORD dwStartTime, DWORD dwPlayTime);
	
	struct ClockVer
	{
		D3DXVECTOR4 vPos;
		DWORD dwColor;
	};

	virtual	void	FrameMove(DWORD dwDailTime);

	void			Update();
	void			Render(int x,int y);
	
protected:
	void	ResetTime(DWORD	dwTime);

	int		_iID;
	
private:
	MPIMesh*				_pVBWnd;

	RECT					_rcWnd;
	bool					_bUpdate;
	float					_fPlayTime;
	float					_fCurTime;
	float					_fCurAngle;

	ClockVer				_vVertex[10];
	ClockVer				_vTempVer[10];

	D3DXVECTOR4				_vSave[10];
};

inline void CGameApp::SendMessage( DWORD dwTypeID, DWORD dwParam1, DWORD dwParam2 )
{	
	int i=0;
	while( !::PostMessage( GetHWND(), dwTypeID, dwParam1, dwParam2 ) && i<=10 )
	{
		Sleep(50);
		i++;
	}
}

inline void	CGameApp::SetStartMinimap(int  ix, int iy, int destx,int desty)
{ 
	xp = (float)((ix / SHOWRSIZE) * SHOWRSIZE) + (SHOWRSIZE/2);
	yp = (float)((iy / SHOWRSIZE) * SHOWRSIZE) + (SHOWRSIZE/2);

	destxp = (float)xp + destx* SHOWRSIZE;
	destyp = (float)yp + desty* SHOWRSIZE;

	xp1 = xp;
	yp1 = yp;
}

inline DWORD CGameApp::GetFrameFPS()                   
{ 
	return CSteadyFrame::GetFPS();	
}

inline DWORD CGameApp::GetSkillClock(int skill_id)
{
	std::map<int,DWORD>::iterator it = m_mapSkillClock.find(skill_id);
	if (it != m_mapSkillClock.end()) {
		return m_mapSkillClock[skill_id];
	}
	return NULL;
}

inline void CGameApp::SetSkillClock(int skill_id, DWORD dwSkillTime)
{
	if (!GetSkillClock(skill_id)) {
		m_mapSkillClock[skill_id] = dwSkillTime;
	}
}

inline void CGameApp::DeleteSkillClock(int skill_id)
{
	std::map<int,DWORD>::iterator it = m_mapSkillClock.find(skill_id);
	if (it != m_mapSkillClock.end()) {
		m_mapSkillClock.erase(skill_id);
	}
}

inline void CGameApp::ClearAllSkillClocks()
{
	m_mapSkillClock.clear();
}

#define TipI(con, t1, t2) { if(con) {g_pGameApp->AddTipText(t1);} else { g_pGameApp->AddTipText(t2); } }
#define Tip(t)            { g_pGameApp->AddTipText(t);}

extern CGameApp* g_pGameApp;
extern bool	volatile	g_bLoadRes;
BOOL RenderHintFrame(const RECT* rc, DWORD color);