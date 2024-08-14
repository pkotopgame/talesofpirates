#include "Stdafx.h"

#include "GameApp.h"

#include "SceneObjSet.h"
#include "EffectSet.h"
#include "MusicSet.h"
#include "CharacterPoseSet.h"
#include "MapSet.h"
#include "ChaCreateSet.h"
#include "EventSoundSet.h"
#include "AreaRecord.h"
#include "ServerSet.h"

#include "MPEditor.h"
#include "FindPath.h"
#include "MPFont.h"

#include "EffectObj.h"
#include "notifyset.h"
#include "ChatIconSet.h"
#include "ItemTypeSet.h"
#include "InputBox.h"
#include "ItemPreSet.h"
#include "ItemRefineSet.h"
#include "ItemRefineEffectSet.h"
#include "StoneSet.h"
#include "ElfSkillSet.h"
#include "GameWG.h"
#include "GameMovie.h"
#include "MonsterSet.h"
#include "helpinfoset.h"
#include "ResourceBundleManage.h"
#include "pi_Alloc.h"
#include "MountRecord.h"
#include "LootFilter.h"

#ifndef USE_DSOUND
#include "AudioThread.h"
CAudioThread	g_AudioThread;
#endif

// �����Կ����ȳ�ʼ��
CLanguageRecord g_oLangRec("./scripts/table/StringSet.bin", "./scripts/table/StringSet.txt");
CResourceBundleManage g_ResourceBundleManage("Game.loc");			// These objects are just being called here to avoid linker errors,
pi_LeakReporter pi_leakReporter("gameleak.log");		// since client uses StringSet instead of .res files.


bool	volatile	g_bLoadRes				  = FALSE;
CGameApp*	        g_pGameApp	              = NULL;

CEffectSet*	        CEffectSet::_Instance     = NULL;
CShadeSet*          CShadeSet::_Instance      = NULL;
CMusicSet*          CMusicSet::_Instance      = NULL;
CPoseSet*           CPoseSet::_Instance       = NULL;
CMapSet*            CMapSet::_Instance        = NULL;
CChaCreateSet*      CChaCreateSet::_Instance  = NULL;
CEventSoundSet*     CEventSoundSet::_Instance = NULL;
CAreaSet*           CAreaSet::_Instance       = NULL;
CServerSet*         CServerSet::_Instance     = NULL;
CNotifySet*         CNotifySet::_Instance     = NULL;
CChatIconSet*		CChatIconSet::_Instance   = NULL;
CItemTypeSet*		CItemTypeSet::_Instance   = NULL;
CItemPreSet*		CItemPreSet::_Instance	  = NULL;
CItemRefineSet*		CItemRefineSet::_Instance = NULL;
CItemRefineEffectSet* CItemRefineEffectSet::_Instance	= NULL;
CStoneSet*			CStoneSet::_Instance				= NULL;
CElfSkillSet*		CElfSkillSet::_Instance				= NULL;

CMonsterSet*        CMonsterSet::_Instance    = NULL;//Add by sunny.sun 20080903
CHelpInfoSet*		CHelpInfoSet::_Instance     = NULL;
// Add by Mdr
CMountSet*          CMountSet::_Instance      = NULL;


MPEditor	        g_Editor;
CFindPath			g_cFindPath(128,38);
CFindPathEx         g_cFindPathEx;          //Add by mdr
CInputBox			g_InputBox;

CGameWG             g_oGameWG;
CGameMovie			g_GameMovie;

LootFilter*         g_lootFilter            = NULL;

// �ͻ��˰汾��, ��GateServer����֤
short g_sClientVer = 78519;
short g_sKeyData = short(g_sClientVer * g_sClientVer * 0x93828311); // 0x1232222
char g_szSendKey[4];
char g_szRecvKey[4];

DWORD g_dwCurMusicID = 0;

// CLightEff plight;
