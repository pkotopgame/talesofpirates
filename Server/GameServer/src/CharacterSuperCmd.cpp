#include "stdafx.h"
#include "GameApp.h"
#include "Character.h"
#include "SubMap.h"
#include "NPC.h"
#include "Item.h"
#include "Script.h"
#include "CommFunc.h"
#include "Player.h"
#include "ItemAttr.h"
#include "JobInitEquip.h"
#include "GameAppNet.h"
#include "SkillStateRecord.h"
#include "lua_gamectrl.h"
#include <fstream>
#include <iostream>

using namespace std;
#pragma warning(disable: 4355)


void CCharacter::DoCommand(cChar *cszCommand, uLong ulLen)
{T_B
	Char	szComHead[256], szComParam[2048];
	std::string	strList[10];
	std::string strPrint = cszCommand;

	Char	*szCom = (Char *)cszCommand;
	size_t	tStart = strspn(cszCommand, " ");
	if (tStart >= strlen(cszCommand))
		return;
	szCom += tStart;
	Char	*szParam = strstr(szCom, " ");
	if (szParam)
	{
		*szParam = '\0';
		strncpy(szComHead, szCom, 256 - 1);
		if (szParam[1] != '\0')
			strncpy(szComParam, szParam + 1, 256 - 1);
		else
			szComParam[0] = '\0';
	}
	else
	{
		strncpy(szComHead, szCom, 256 - 1);
		szComParam[0] = '\0';
	}

	
	// ���ִ��GMָ��
	if(DoGMCommand(szComHead, szComParam))
		//LG("DoCommand", "[ִ�гɹ�]%s��%s\n", GetLogName(), strPrint.c_str());
		LG("DoCommand", "[operator succeed]%s��%s\n", GetLogName(), strPrint.c_str());
	else
		//LG("DoCommand", "[ִ��ʧ��]%s��%s\n", GetLogName(), strPrint.c_str());
		LG("DoCommand", "[operator succeed]%s��%s\n", GetLogName(), strPrint.c_str());
	
T_E}


//--------------------------------------------------------------------------------
// GM ָ����, ��ȱ���ʺ�Ȩ���ж�
//--------------------------------------------------------------------------------
//TODO(Ogge): Extract method for each GM-level
BOOL CCharacter::DoGMCommand(const char *pszCmd, const char *pszParam)
{T_B
	CPlayer *pPlayer = GetPlayer(); 
	if(!pPlayer) return FALSE;
	
	uChar uchGMLv = pPlayer->GetGMLev();
	if (uchGMLv == 0)
	{
		//SystemNotice("Ȩ�޲���!");
		SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00001));
		return FALSE;
	}

	std::string	strList[10];
	string strCmd = pszCmd;

	C_PRINT("%s: %s %s\n", GetName(), strCmd.c_str(), pszParam);
	//-----------------------
	// ����GM������ִ�е�ָ��
	//-----------------------
	if (strCmd==g_Command.m_cMove) // ��ͼ��ת����ʽ��move x,y,��ͼ��
	{
		int n = Util_ResolveTextLine(pszParam, strList, 10, ',');
		Point l_aim;
		l_aim.x = Str2Int(strList[0]) * 100;
		l_aim.y = Str2Int(strList[1]) * 100;
		const char	*szMapName = 0;
		short	sMapCpyNO = 0;
		if (n == 3)
			szMapName = strList[2].c_str();
		else
			szMapName = GetSubMap()->GetName();
		if (n == 4)
			sMapCpyNO = Str2Int(strList[1]);

		SwitchMap(GetSubMap(), szMapName, l_aim.x, l_aim.y, true, enumSWITCHMAP_CARRY, sMapCpyNO);
		// Delete by lark.li 20080814 begin
		//LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		// End
		return TRUE;
	}
	else if(strCmd==g_Command.m_cNotice) // ϵͳͨ��
	{
		g_pGameApp->WorldNotice(pszParam);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if(strCmd==g_Command.m_cHide) // ����
	{
		AddSkillState(m_uchFightID, GetID(), GetHandle(), enumSKILL_TYPE_SELF, enumSKILL_TAR_LORS, enumSKILL_EFF_HELPFUL, SSTATE_HIDE, 1, -1, enumSSTATE_ADD);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if(strCmd==g_Command.m_cUnhide) // ����
	{
		DelSkillState(SSTATE_HIDE);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if(strCmd==g_Command.m_cGoto) // ���Լ�����ĳ��ɫ���
	{
		int n = Util_ResolveTextLine(pszParam, strList, 10, ',');
		WPACKET WtPk	=GETWPACKET();
		WRITE_CMD(WtPk, CMD_MM_GOTO_CHA);
		WRITE_LONG(WtPk, GetID());
		WRITE_STRING(WtPk, strList[0].c_str());
		WRITE_CHAR(WtPk, 1);
		WRITE_STRING(WtPk, GetName());
		ReflectINFof(this, WtPk);//ͨ��
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}

	if(uchGMLv <= 1)
	{
		//SystemNotice("Ȩ�޲���!");
		SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00001));
		return FALSE;
	}

	//-----------------------
	// 1������GM������ִ�е�ָ��
	//-----------------------
	
	if (strCmd == g_Command.m_cMute)
	{
		const auto n = Util_ResolveTextLine(pszParam, strList, 10, ',');
		if (n < 2)
		{
			return false;
		}

		WPACKET wpk = GETWPACKET();
		WRITE_CMD(wpk, CMD_MP_MUTE_PLAYER);
		WRITE_STRING(wpk, strList[0].c_str());
		WRITE_LONG(wpk, Str2Int(strList[1]));
		ReflectINFof(this, wpk);
	}

	if (strCmd==g_Command.m_cKick) // �����������
	{
		int n = Util_ResolveTextLine(pszParam, strList, 10, ',');
		if (n < 1)
		{
			SystemNotice("You did not input a player name!");
			return FALSE;
		}
		WPACKET WtPk = GETWPACKET();
		WRITE_CMD(WtPk, CMD_MM_KICK_CHA);
		WRITE_LONG(WtPk, GetID());
		WRITE_STRING(WtPk, strList[0].c_str());
		if (n == 2)
			WRITE_LONG(WtPk, Str2Int(strList[1]));
		else
			WRITE_LONG(WtPk, 0);

		ReflectINFof(this, WtPk);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}

	// Modify by lark.li 20080731 begin
	time_t t = time(0);
    tm* TM = localtime(&t);

	bool gmOK = false;

	for(vector<int>::iterator it = g_Config.m_vGMCmd.begin(); it != g_Config.m_vGMCmd.end(); it++)
	{
		if(TM->tm_wday == *it)
		{
			gmOK = true;
			break;
		}
	}

	if(!gmOK)
	{
		SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00047));
		return FALSE;
	}
	// End

	if(uchGMLv != 99)
	{
		//SystemNotice("Ȩ�޲���!");
		SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00001));
		return FALSE;
	}

    LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);

	cChar	*szComHead = pszCmd;
	cChar	*szComParam = pszParam;
	//-----------------------
	// 99��GM������ִ�е�ָ��
	//-----------------------
	if (!strcmp(szComHead, g_Command.m_cReload)) // ���¶���
	{
		cChar *pszChaInfo = "characterinfo";
		cChar *pszSkillInfo = "skillinfo";
		cChar *pszItemInfo = "iteminfo";
		if (!strcmp(szComParam, pszChaInfo))
			g_pGameApp->LoadCharacterInfo();
		else if (!strcmp(szComParam, pszSkillInfo))
			g_pGameApp->LoadSkillInfo();
		else if (!strcmp(szComParam, pszItemInfo))
			g_pGameApp->LoadItemInfo();
		else
		{
			SystemNotice("Available argument: iteminfo, skillinfo, and characterinfo", szComParam);
			return TRUE;
		}
		SystemNotice("Reloading %s.txt success!", szComParam);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
    else if(!strcmp(szComHead, g_Command.m_cRelive)) // ԭ�ظ���
	{	
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if(!strcmp(szComHead, g_Command.m_cQcha)) // ��ѯ��ɫ��Ϣ(���ڵ�ͼ,����,ΨһID)
	{
		int n = Util_ResolveTextLine(pszParam, strList, 10, ',');
		WPACKET WtPk	=GETWPACKET();
		WRITE_CMD(WtPk, CMD_MM_QUERY_CHA);
		WRITE_LONG(WtPk, GetID());
		WRITE_STRING(WtPk, strList[0].c_str());
		ReflectINFof(this, WtPk);//ͨ��
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if(!strcmp(szComHead, g_Command.m_cQitem)) // ��ѯ��ɫ����
	{
		int n = Util_ResolveTextLine(pszParam, strList, 10, ',');
		WPACKET WtPk	=GETWPACKET();
		WRITE_CMD(WtPk, CMD_MM_QUERY_CHAITEM);
		WRITE_LONG(WtPk, GetID());
		WRITE_STRING(WtPk, strList[0].c_str());
		ReflectINFof(this, WtPk);//ͨ��
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
    if(!strcmp(szComHead, g_Command.m_cCall)) // ����һ��ɫ�������
	{
		int n = Util_ResolveTextLine(pszParam, strList, 10, ',');
		WPACKET WtPk	=GETWPACKET();
		WRITE_CMD(WtPk, CMD_MM_CALL_CHA);
		WRITE_LONG(WtPk, GetID());
		WRITE_STRING(WtPk, strList[0].c_str());
		if (IsBoat())
			WRITE_CHAR(WtPk, 1);
		else
			WRITE_CHAR(WtPk, 0);
		WRITE_STRING(WtPk, GetSubMap()->GetName());
		WRITE_LONG(WtPk, GetPos().x);
		WRITE_LONG(WtPk, GetPos().y);
		WRITE_LONG(WtPk, GetSubMap()->GetCopyNO());
		ReflectINFof(this, WtPk);//ͨ��
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, g_Command.m_cGamesvrstop)) // ������Ϸ����������
	{
		g_pGameApp->m_CTimerReset.Begin(1000);
		g_pGameApp->m_ulLeftSec = atol(szComParam);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp(szComHead, g_Command.m_cUpdateall) ) // �ű�lua����
	{
		LoadScript();
		if ( g_pGameApp->ReloadNpcInfo( *this ) )
		{
			//SystemNotice( "NPC�Ի�������lua�ű����³ɹ�!" );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00002) );
		}
		else
		{
			return FALSE;
		}
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp(szComHead, "reloadcfg"))
	{
		if (!g_Config.Reload((char*)pszParam))
		{
			SystemNotice("Reloading %s failed!", pszParam);
		}
		else
		{
			SystemNotice("Reloading %s success!", pszParam);
		}
		return TRUE;
	}
	else if( !strcmp(szComHead, "harmlog=1") ) // �˺��ۼƼ���Log����
	{
		g_bLogHarmRec = TRUE;
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp(szComHead, "harmlog=0") ) // �˺��ۼƼ���Log����
	{
		g_bLogHarmRec = FALSE;
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp(szComHead, g_Command.m_cMisreload) ) // ����ű�����
	{
		LoadScript();
		if( g_pGameApp->ReloadNpcInfo( *this ) )
		{
			//SystemNotice( "NPC�Ի�������lua�ű����³ɹ�!" );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00002) );
		}
		else
		{
			return FALSE;
		}
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp(szComHead, "reload_ai") )
	{
		ReloadAISdk();
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp(szComHead, "setrecord" ) ) // ���ý�ɫ������ʷ���
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');		
		USHORT sID   = Str2Int(strList[0]);
		if( GetPlayer()->MisSetRecord( sID ) )
		{
			//SystemNotice( "����������ʷ��ǳɹ�!ID[%d]", sID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00003), sID );
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		else
		{
			//SystemNotice( "����������ʷ���ʧ��!ID[%d]", sID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00003), sID );
			return FALSE;
		}
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp(szComHead, "clearrecord" ) ) // ���ý�ɫ������ʷ���
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');		
		USHORT sID   = Str2Int(strList[0]);
		if( GetPlayer()->MisClearRecord( sID ) )
		{
			//SystemNotice( "���������ʷ��ǳɹ�!ID[%d]", sID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00004), sID );
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		else
		{
			//SystemNotice( "���������ʷ���ʧ��!ID[%d]", sID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00005), sID );
			return FALSE;
		}
		return TRUE;
	}
	else if( !strcmp(szComHead, "setflag" ) ) // ����������
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');		
		USHORT sID   = Str2Int(strList[0]);
		USHORT sFlag = Str2Int(strList[1]);
		if( GetPlayer()->MisSetFlag( sID, sFlag ) )
		{
			//SystemNotice( "���������ǳɹ�!ID[%d], FLAG[%d]", sID, sFlag );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00006), sID, sFlag );
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		else
		{
			//SystemNotice( "����������ʧ��!ID[%d], FLAG[%d]", sID, sFlag );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00007), sID, sFlag );
			return FALSE;
		}
		return TRUE;
	}
	else if( !strcmp(szComHead, "clearflag" ) ) // ���������
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');		
		USHORT sID   = Str2Int(strList[0]);
		USHORT sFlag = Str2Int(strList[1]);
		if( GetPlayer()->MisClearFlag( sID, sFlag ) )
		{
			//SystemNotice( "��������ǳɹ�!ID[%d], FLAG[%d]", sID, sFlag );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00008), sID, sFlag );
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		else
		{
			//SystemNotice( "���������ʧ��!ID[%d], FLAG[%d]", sID, sFlag );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00009), sID, sFlag );
			return FALSE;
		}
		return TRUE;
	}
	else if( !strcmp(szComHead, "addmission" ) ) // ����������
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');		
		USHORT sMID   = Str2Int(strList[0]);
		USHORT sSID   = Str2Int(strList[1]);
		if( GetPlayer()->MisAddRole( sMID, sSID ) )
		{
			//SystemNotice( "�������ɹ�!MID[%d], SID[%d]", sMID, sSID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00010), sMID, sSID );
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		else
		{
			//SystemNotice( "�������ʧ��!MID[%d], SID[%d]", sMID, sSID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00011), sMID, sSID );
			return FALSE;
		}
		return TRUE;
	}
	else if( !strcmp(szComHead, "clearmission" ) ) // ���������
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		USHORT sID   = Str2Int(strList[0]);
		if( GetPlayer()->MisCancelRole( sID ) )
		{
			//SystemNotice( "�������ɹ�!MID[%d]", sID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00012), sID );
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		else
		{
			//SystemNotice( "�������ʧ��!MID[%d]", sID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00013), sID );
			return FALSE;
		}
		return TRUE;
	}
	else if( !strcmp(szComHead, "delmission" ) ) // ���������
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		USHORT sID   = Str2Int(strList[0]);
		if( GetPlayer()->MisClearRole( sID ) )
		{
			//SystemNotice( "ɾ������ɹ�!MID[%d]", sID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00014), sID );
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		else
		{
			//SystemNotice( "ɾ������ʧ��!MID[%d]", sID );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00015), sID );
			return FALSE;
		}
		return TRUE;
	}
	else if( !strcmp(szComHead, "missdk" ) )	 // ����ű�����
	{
		ReloadLuaSdk();
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp( szComHead, "misclear") ) // �����ɫ�����ǩ��Ϣ�ʹ�������Ϣ
	{
		GetPlayer()->MisClear();
		//SystemNotice( "�����ɫ�����ǩ��Ϣ�ʹ�������Ϣ�ɹ�!" );
		SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00016) );
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, "isblock")) // �Ƿ����ϰ���
	{
		if (m_submap->IsBlock(GetPos().x / m_submap->GetBlockCellWidth(), GetPos().y / m_submap->GetBlockCellHeight()))
			//SystemNotice("�ϰ�");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00017));
		else
			//SystemNotice("���ϰ�");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00018));
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, "pet")) // �ٻ�����
	{
		Long	lChaInfoID = Str2Int(strList[0]);
		Point	Pos = GetPos();
		Pos.move(rand() % 360, 3 * 100);
		CCharacter *pCha = m_submap->ChaSpawn(lChaInfoID, enumCHACTRL_PLAYER_PET, rand()%360, &Pos);
		if (pCha)
		{
			pCha->m_HostCha = this;
			pCha->SetPlayer(GetPlayer());
			pCha->m_AIType = 5;
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		else
		{
			//SystemNotice( "�ٻ�����ʧ��" );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00019) );
			return FALSE;
		}
		return TRUE;
	}
	else if (!strcmp(szComHead, g_Command.m_cSummon)) // �ٻ�����
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n >= 1)
		{
			Long	lChaInfoID = Str2Int(strList[0]);
			Point	Pos = GetPos();
			Pos.move(rand() % 360, 3 * 100);
			CCharacter *pCha = m_submap->ChaSpawn(lChaInfoID, enumCHACTRL_NONE, rand()%360, &Pos);
			if (pCha)
			{
				if(n>=2)
				{
					DWORD dwLifeTime = Str2Int(strList[1]);
					pCha->ResetLifeTime(dwLifeTime);	
				}
				if( n==3 )
				{
					int nAIType = Str2Int(strList[2]);
					pCha->m_AIType  = (BYTE)nAIType; // ���ù����AI����
				}
			}
			else
			{
				//SystemNotice( "��������ʧ��!" );
				SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00020) );
				return FALSE;
			}
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		return FALSE;
	}
	else if (!strcmp(szComHead, g_Command.m_cSummonex)) // �ٻ�������չָ���ʽ����ɫ��ţ��������Ƿ񼤻���Ұ����������
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n >= 1)
		{
			Long	lChaInfoID = Str2Int(strList[0]);
			Point	Pos;
			long	lChaNum = 1;
			if (n > 1)
				lChaNum = Str2Int(strList[1]);
			bool	bActEyeshot = false;
			if( n > 2 )
				bActEyeshot = Str2Int(strList[2]) ? true : false;
			int nAIType = 0;
			if( n > 3 )
				nAIType = Str2Int(strList[3]);
			CCharacter *pCha;
			for (long i = 0; i < lChaNum; i++)
			{
				Pos = GetPos();
				Pos.move(rand() % 360, rand() % 20 * 100);
				pCha = m_submap->ChaSpawn(lChaInfoID, enumCHACTRL_NONE, rand()%360, &Pos, bActEyeshot);
				if (pCha)
				{
					if( n > 3 )
						pCha->m_AIType = (BYTE)nAIType; // ���ù����AI����
				}
				else
				{
					//SystemNotice( "������ɫʧ��!" );
					SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00021 ));
				}
			}
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		return FALSE;
	}
	else if (!strcmp(szComHead, g_Command.m_cKill)) // ɱ���ٻ����ָ���ʽ���������ƣ���Χ���ף�Ĭ��ֵ8�ף���������Ĭ��Ϊ��Χ�ڵ����й��
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n >= 1)
		{
			const char	*szMonsName = strList[0].c_str();
			long	lRange = 8 * 100;
			long	lNum = 0;
			if (n >= 2)
				lRange = Str2Int(strList[1]) * 100;
			if (n >= 3)
				lNum = Str2Int(strList[2]);

			long	lBParam[defSKILL_RANGE_BASEP_NUM] = {GetPos().x, GetPos().y, 0};
			long	lEParam[defSKILL_RANGE_EXTEP_NUM] = {enumRANGE_TYPE_CIRCLE, lRange};
			CCharacter	*pCCha = 0, *pCFreeCha = 0;
			long	lFindNum = 0, lKillNum = 0;
			GetSubMap()->BeginSearchInRange(lBParam, lEParam);
			while (pCCha = GetSubMap()->GetNextCharacterInRange())
			{
				if (pCFreeCha)
					pCFreeCha->Free();
				pCFreeCha = 0;

				lFindNum++;
				if (pCCha->IsPlayerCha())
					continue;
				if (!strcmp(pCCha->GetName(), szMonsName))
				{
					if (lNum == 0 || lKillNum <= lNum)
					{
						pCFreeCha = pCCha;
						lKillNum++;
					}
				}
			}
			if (pCFreeCha)
				pCFreeCha->Free();

			//SystemNotice( "ɾ��������Ŀ��%u.!", lKillNum );
			SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00022), lKillNum );
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		return FALSE;
	}
	
	if(g_Config.m_bSuperCmd==FALSE)
	{
		//SystemNotice("Ȩ�޲���!");
		SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00001));
		return FALSE;
	}
	
	//-----------------------------------
	// ��������ָ��, ��GMָ��Ҫ�ϸ����ֿ�
	//-----------------------------------
	if( !strcmp( szComHead, g_Command.m_cAddmoney ) )
	{
		//AddMoney( "ϵͳ", atol(szComParam) );
		AddMoney( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00023), atol(szComParam) );
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	if (!strcmp(szComHead, g_Command.m_cAddImp))
	{
		const char* szItemScript = "GiveIMP";
		lua_getglobal(g_pLuaState, szItemScript);
		if (!lua_isfunction(g_pLuaState, -1))
		{
			lua_pop(g_pLuaState, 1);
			return FALSE;
		}
		lua_pushlightuserdata(g_pLuaState, (void*)this);
		lua_pushnumber(g_pLuaState, atol(szComParam));
		int ret = lua_pcall(g_pLuaState, 2, 0, 0);
		lua_settop(g_pLuaState, 0);
		return TRUE;
	}
	else if( !strcmp( szComHead, g_Command.m_cAddexp ) )
	{
		AddExpAndNotic( atol(szComParam) );
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp( szComHead, "addlifeexp" ) )
	{
		AddAttr( ATTR_CLIFEEXP, atol(szComParam) );
		//SystemNotice( "ϵͳ������%ld�����!", atol(szComParam) );
		SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00024), atol(szComParam) );
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp( szComHead, "addsailexp" ) )
	{
		AddAttr( ATTR_CSAILEXP, atol(szComParam) );
		//SystemNotice( "ϵͳ������%ldת������!", atol(szComParam) );
		SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00025), atol(szComParam) );
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp( szComHead, "addcess" ) )
	{
		AdjustTradeItemCess( 60000, (USHORT)atol(szComParam) );
		//SystemNotice( "���%ldó��˰��!", atol(szComParam) );
		SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00026), atol(szComParam) );
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if( !strcmp( szComHead, "setcesslevel" ) )
	{
		SetTradeItemLevel( (BYTE)atol(szComParam) );
		//SystemNotice( "����ó�׵ȼ�%ld!", atol(szComParam) );
		SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00027), atol(szComParam) );
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, g_Command.m_cMake)) // �ٻ����ߣ���ʽ�����߱�ţ�����[��ʵ��������][������1��������.����Ϊ���棩]
	{
		LG("GMmakeLog", "begin make\n");
		int n = Util_ResolveTextLine(szComParam, strList, 20, ',');
		if(n >= 2)
		{
			short	sID = Str2Int(strList[0]);
			short	sNum = Str2Int(strList[1]);
			short	sTo = 1; // ���������
			char	chSpawnType = enumITEM_INST_MONS;
			if (sNum < 0 || sNum > 100)
				sNum = 10;
			if (n == 3)
				chSpawnType = Str2Int(strList[2]);
			if (n == 4)
				sTo = Str2Int(strList[3]);
			
			
			LG("GMmakeLog", "atorNome = %s,sID = %d,sNum = %d,sTo = %d,chSpawnType = %c\n",
				m_name,sID,sNum,sTo,chSpawnType);

			if (sTo == 1)
			{
				if (AddItem( sID, sNum, this->GetName(), chSpawnType))
				{
					LG("GMmakeLog", "add to kitbag successful!\n");
					return TRUE;
				}
			}
			else
			{
				SItemGrid GridContent(sID, sNum);
				ItemInstance(chSpawnType, &GridContent);
				Long	lPosX, lPosY;
				CCharacter	*pCCtrlCha = GetPlyCtrlCha();
				pCCtrlCha->GetTrowItemPos(&lPosX, &lPosY);
				if (pCCtrlCha->GetSubMap()->ItemSpawn(&GridContent, lPosX, lPosY, enumITEM_APPE_THROW, pCCtrlCha->GetID()))
				{
					LG("GMmakeLog", "add to ground successful!\n");
					return TRUE;
				}
			}
			LG("GMmakeLog", "make failed!\n");
			return FALSE;
		}
		LG("GMmakeLog", "make failed! because the param is less than 2!\n");
		return FALSE;
	}
	else if (!strcmp(szComHead, g_Command.m_cAttr)) // ��ɫ���ԣ���ʽ�����Ա�ţ�����ֵ.
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n < 2)
			return FALSE;
		CCharacter	*pCCha = this;

		long	lAttrID = Str2Int(strList[0]);


		long	lAttrVal = Str2Int(strList[1]);
		//LONG64 lAttrVal = _atoi64(strList[1].c_str());

		if (n == 3)
			pCCha = g_pGameApp->FindChaByID(Str2Int(strList[2]));
		if (!pCCha)
		{
			//SystemNotice("û��������Ŀ���Ŀ�겻�ǽ�ɫ�����!");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00028));
			return FALSE;
		}
		pCCha->m_CChaAttr.ResetChangeFlag();
		pCCha->SetBoatAttrChangeFlag(false);
		pCCha->setAttr(lAttrID, lAttrVal);
		if (pCCha->IsPlayerOwnCha())
		{
			if (pCCha->IsBoat())
				g_CParser.DoString("ShipAttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCCha, DOSTRING_PARAM_END);
			else
				g_CParser.DoString("AttrRecheck", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCCha, DOSTRING_PARAM_END);
		}
		else
		{
			g_CParser.DoString("ALLExAttrSet", enumSCRIPT_RETURN_NONE, 0, enumSCRIPT_PARAM_LIGHTUSERDATA, 1, pCCha, DOSTRING_PARAM_END);
		}
		if (pCCha->GetPlayer())
		{
			pCCha->GetPlayer()->RefreshBoatAttr();
			pCCha->SyncBoatAttr(enumATTRSYN_TASK);
		}
		pCCha->SynAttr(enumATTRSYN_TASK);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, g_Command.m_cItemattr)) // ��ɫ�������ԣ���ʽ��λ�����ͣ�1��װ����.2������������λ�ñ�ţ����Ա�ţ�����ֵ.
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n != 4)
			return FALSE;

		Char	chPosType = Str2Int(strList[0]);
		Long	lPosID = Str2Int(strList[1]);
		SItemGrid	*pSItem = GetItem2(chPosType, lPosID);
		if (!pSItem)
			return FALSE;
		Long	lAttrID = Str2Int(strList[2]);
		Short	sAttr = Str2Int(strList[3]);
		pSItem->SetAttr(lAttrID, sAttr);
		pSItem->SetInstAttr(lAttrID, sAttr);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	
	else if (!strcmp(szComHead, "setexpiration"))
	{
	int n = Util_ResolveTextLine(szComParam, strList, 30, ',');
	if (n != 3) return FALSE;
	Char	chPosType = Str2Int(strList[0]);
	Long	lPosID = Str2Int(strList[1]);
	SItemGrid* pSItem = GetItem2(chPosType, lPosID);
	if (!pSItem)
		return FALSE;

	int minutes = Str2Int(strList[2]);
	time_t expirationt;
	if (minutes == 0) expirationt = 0;
	else expirationt = std::time(0) + minutes;
	pSItem->expiration = expirationt;
	pSItem->SetChange(true);
	char buf[32];

	return TRUE;

	}

	else if (!strcmp(szComHead, "light")) // ��ɫ���ͼ�ƹⷶΧ����ʽ����Χ���ף�.
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n != 1)
			return FALSE;

		long	lLight = Str2Int(strList[0]);
		if (lLight < 0)
			return FALSE;
		GetPlayer()->SetMMaskLightSize(lLight);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, "seeattr")) // �鿴��ɫ���ԣ���ʽ��WorldID�����Ա��.
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n != 2)
			return FALSE;

		uLong	ulWorldID = Str2Int(strList[0]);
		short	sAttrID = Str2Int(strList[1]);
		CCharacter	*pCCha = g_pGameApp->FindChaByID(ulWorldID);
		if (!pCCha)
		{
			//SystemNotice("û��������Ŀ���Ŀ�겻�ǽ�ɫ�����!");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00028));
			return FALSE;
		}
		//SystemNotice("Ŀ��[%s]������Ϊ%d��ֵ��%d!", pCCha->m_CLog.GetLogName(), sAttrID, pCCha->getAttr(sAttrID));
		SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00029), pCCha->m_CLog.GetLogName(), sAttrID, pCCha->getAttr(sAttrID));
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, "forge")) // ���߾�������ʽ�������ȼ�����ֵ��������λ��
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		char	chAddLv = Str2Int(strList[0]);
		short	sGridID = 0;
		if (n == 2)
			sGridID = Str2Int(strList[1]);

		SItemGrid *pItemCont = m_CKitbag.GetGridContByID(sGridID);
		if (pItemCont)
		{
			CItemRecord	*pCItemRec = GetItemRecordInfo(pItemCont->sID);
			if (pCItemRec && pCItemRec->chForgeLv > 0)
			{
				m_CKitbag.SetChangeFlag(false);
				pItemCont->AddForgeLv(chAddLv);
				ItemForge(pItemCont, chAddLv);
				SynKitbagNew(enumSYN_KITBAG_FORGES);
				LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
				return TRUE;
			}
		}
		return FALSE;
	}
	else if (!strcmp(szComHead, g_Command.m_cSkill)) // ���Ӽ��ܣ���ʽ����ţ��ȼ�
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		short	sID = Str2Int(strList[0]);
		char	chLv = Str2Int(strList[1]);
		bool	bLimit = true;
		if (n == 3 && Str2Int(strList[2]) == 0)
			bLimit = false;

		if (!GetPlayer()->GetMainCha()->LearnSkill(sID, chLv, true, false, bLimit))
		{
			//SystemNotice("ѧϰ����ʧ�ܣ�����ѧϰ�ȼ���ְҵ���ƣ�ǰ�ü������Ƶȣ������ %d���ȼ� %d.!", sID, chLv);
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00030), sID, chLv);
			return FALSE;
		}
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, g_Command.m_cDelitem))
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n == 6)
		{
			long	lItemID = Str2Int(strList[0]);
			long	lItemNum = Str2Int(strList[1]);
			char	chFromType = Str2Int(strList[2]);
			short	sFromID = Str2Int(strList[3]);
			char	chToType = Str2Int(strList[4]);
			char	chForcible = Str2Int(strList[5]);
			if (Cmd_RemoveItem(lItemID, lItemNum, chFromType, sFromID, chToType, 0, true, chForcible) != enumITEMOPT_SUCCESS)
			{
				//SystemNotice("ɾ������ʧ��!");
				SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00031));
				return FALSE;
			}
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
		return FALSE;
	}
	else if (!strcmp(szComHead, g_Command.m_cLua)) // ��GameServerִ�нű�
	{
		luaL_dostring(g_pLuaState, szComParam);
		//C_PRINT("%s: lua %s\n", GetName(), pszParam);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, g_Command.m_cLuaall)) // ����GameServerִ�нű�
	{
		WPACKET WtPk	=GETWPACKET();
		WRITE_CMD(WtPk, CMD_MM_DO_STRING);
		WRITE_LONG(WtPk, GetID());
		WRITE_STRING(WtPk, szComParam);
		ReflectINFof(this, WtPk);//ͨ��
		C_PRINT("%s: lua_all %s\n", GetName(), pszParam);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, "setping"))
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n != 1)
		{
			//SystemNotice("��������");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00032));
			return FALSE;
		}
		Long	lPing = Str2Int(strList[0]);
		m_lSetPing = lPing;
		SendPreMoveTime();
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, "getping"))
	{
		//SystemNotice("��ǰping��%d", m_SMoveInit.usPing);
		SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00033), m_SMoveInit.usPing);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, "senddata"))
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n != 2)
		{
			//SystemNotice("��������");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00032));
			return FALSE;
		}

		m_timerNetSendFreq.SetInterval((DWORD)Str2Int(strList[0]));
		m_ulNetSendLen = (uLong)Str2Int(strList[1]);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, "setpinginfo"))
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n != 2)
		{
			//SystemNotice("��������");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00032));
			return FALSE;
		}

		m_timerPing.SetInterval((DWORD)Str2Int(strList[0]));
		m_ulPingDataLen = (uLong)Str2Int(strList[1]);
		LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
		return TRUE;
	}
	else if (!strcmp(szComHead, g_Command.m_cAddkb)) // ���ӱ�����������ʽ������������[,WorldID]
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		short	sAddCap = Str2Int(strList[0]);
		CCharacter	*pCCha = this;
		if (n == 2)
			pCCha = g_pGameApp->FindChaByID(Str2Int(strList[1]));
		if (!pCCha)
		{
			//SystemNotice("���ӱ�������ʧ�ܣ��Ҳ����ý�ɫ��.!");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00034));
			return FALSE;
		}

		if (!pCCha->AddKitbagCapacity(sAddCap))
		{
			//SystemNotice("���� %s �ı�������ʧ�ܣ������Ƿ���.!", pCCha->GetName());
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00035), pCCha->GetName());
			return FALSE;
		}
		else
		{
			//pCCha->SystemNotice("���ӱ��������ɹ�����ǰ���� %d.!", pCCha->m_CKitbag.GetCapacity());
			pCCha->SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00036), pCCha->m_CKitbag.GetCapacity());
			if (pCCha != this)
				//SystemNotice("���� %s �ı��������ɹ�����ǰ���� %d.!", pCCha->GetName(), pCCha->m_CKitbag.GetCapacity());
				SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00037), pCCha->GetName(), pCCha->m_CKitbag.GetCapacity());
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
		}
	}
	else if (!strcmp(szComHead, "itemvalid")) // ���ñ���������Ч��
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		if (n != 2)
		{
			//SystemNotice("��ʽ����!");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00038));
			return FALSE;
		}
		short	sPosID = Str2Int(strList[0]);
		bool	bValid = Str2Int(strList[1]) != 0 ? true : false;

		if (!SetKitbagItemValid(sPosID, bValid))
		{
			//SystemNotice("���ñ���������Ч��ʧ��!");
			SystemNotice(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00039));
			return FALSE;
		}
		else
			LG("ServerRunLog", "ChaID: %i, ChaName: %s, CMD: %s, Param: %s\n", GetPlayer()->GetID(), GetName(), pszCmd, pszParam);
			return TRUE;
	}
	else if (!strcmp(szComHead, "scroll")) {
	g_pGameApp->ScrollNotice(szComParam, 2);
		return TRUE;
	}
	else if (!strcmp(szComHead, "generatecharbag")) {					
	const char *charname = szComParam;
	CCharacter *player = g_pGameApp->FindChaByName(charname);
	if (!player) return FALSE;
	CKitbag inventory[2];
	inventory[0] = player->m_CKitbag;
	inventory[1] = *player->GetPlayer()->GetBank();
	char buf2[50];
	char buf3[1024];
	char buf4[300];
	sprintf(buf2, "%s.txt", charname);
	ofstream myfile;
	myfile.open(buf2, ios::out | ios::app);
	if (myfile.is_open())
	{
		for (int p = 0; p < 2; p++) {
			if (p == 0) {
				myfile << "INVENTORY ITEMS: \n";
			}
			else {
				myfile << "BANK ITEMS: \n";
			}
			for (int i = 0; i < inventory[p].GetCapacity(); i++) {
				if (inventory[p].GetGridContByID(i)) {
					
					sprintf(buf3, "Item Name: %s; Item ID: %d; Position ID: %d\n;", GetItemRecordInfo(inventory[p].GetGridContByID(i)->sID)->szName, inventory[p].GetGridContByID(i)->sID, i);
					myfile << buf3;
					sprintf(buf4, "STR (raw): %d. STR (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(26), inventory[p].GetGridContByID(i)->GetInstAttr(1));
					myfile << buf4;
					sprintf(buf4, "AGI (raw): %d. AGI (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(27), inventory[p].GetGridContByID(i)->GetInstAttr(2));
					myfile << buf4;
					sprintf(buf4, "DEX (raw): %d. DEX (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(28), inventory[p].GetGridContByID(i)->GetInstAttr(3));
					myfile << buf4;
					sprintf(buf4, "CON (raw): %d. CON (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(29), inventory[p].GetGridContByID(i)->GetInstAttr(4));
					myfile << buf4;
					sprintf(buf4, "STA (raw): %d. STA (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(30), inventory[p].GetGridContByID(i)->GetInstAttr(5));
					myfile << buf4;
					sprintf(buf4, "LUCK (raw): %d. LUCK (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(31), inventory[p].GetGridContByID(i)->GetInstAttr(6));
					myfile << buf4;
					sprintf(buf4, "ASPD (raw): %d. ASPD(%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(32), inventory[p].GetGridContByID(i)->GetInstAttr(7));
					myfile << buf4;
					sprintf(buf4, "ADIS (raw): %d. ADIS (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(33), inventory[p].GetGridContByID(i)->GetInstAttr(8));
					myfile << buf4;
					sprintf(buf4, "MNATK (raw): %d. MNATK (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(34), inventory[p].GetGridContByID(i)->GetInstAttr(9));
					myfile << buf4;
					sprintf(buf4, "MXATK (raw): %d. MXATK (%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(35), inventory[p].GetGridContByID(i)->GetInstAttr(10));
					myfile << buf4;
					sprintf(buf4, "DEF (raw): %d. DEF(%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(36), inventory[p].GetGridContByID(i)->GetInstAttr(11));
					myfile << buf4;
					sprintf(buf4, "MXHP (raw): %d. MXHP(%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(37), inventory[p].GetGridContByID(i)->GetInstAttr(12));
					myfile << buf4;
					sprintf(buf4, "MXSP (raw): %d. MXSP(%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(38), inventory[p].GetGridContByID(i)->GetInstAttr(13));
					myfile << buf4;
					sprintf(buf4, "FLEE (raw): %d. FLEE(%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(39), inventory[p].GetGridContByID(i)->GetInstAttr(14));
					myfile << buf4;
					sprintf(buf4, "HIT (raw): %d. HIT(%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(40), inventory[p].GetGridContByID(i)->GetInstAttr(15));
					myfile << buf4;
					sprintf(buf4, "CRT (raw): %d. CRT(%%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(41), inventory[p].GetGridContByID(i)->GetInstAttr(16));
					myfile << buf4;
					sprintf(buf4, "MF (raw): %d. MF(%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(42), inventory[p].GetGridContByID(i)->GetInstAttr(17));
					myfile << buf4;
					sprintf(buf4, "HREC (raw): %d. HREC(%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(43), inventory[p].GetGridContByID(i)->GetInstAttr(18));
					myfile << buf4;
					sprintf(buf4, "SREC (raw): %d. SREC(%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(44), inventory[p].GetGridContByID(i)->GetInstAttr(19));
					myfile << buf4;
					sprintf(buf4, "MSPD (raw): %d. MSPD(%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(45), inventory[p].GetGridContByID(i)->GetInstAttr(20));
					myfile << buf4;
					sprintf(buf4, "COL (raw): %d. COL(%): %2.2f\n", inventory[p].GetGridContByID(i)->GetInstAttr(46), inventory[p].GetGridContByID(i)->GetInstAttr(21));
					myfile << buf4;
					sprintf(buf4, "PDEF (raw): %d. PDEF(%): %2.2f\n\n", inventory[p].GetGridContByID(i)->GetInstAttr(47), inventory[p].GetGridContByID(i)->GetInstAttr(22));
					myfile << buf4;

				}
			}
			myfile << "=========================================\n";
		}
		myfile << "EQUIPMENTS: \n";
		for (int i = 0; i < 34; i++) {
			if (player->m_SChaPart.SLink[i].sID) {
				sprintf(buf3, "Item ID: %d; SLink (position) ID: %d\n;", player->m_SChaPart.SLink[i].sID, i);
				myfile << buf3;
				sprintf(buf4, "STR (raw): %d. STR (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(26), player->m_SChaPart.SLink[i].GetInstAttr(1));
				myfile << buf4;
				sprintf(buf4, "AGI (raw): %d. AGI (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(27), player->m_SChaPart.SLink[i].GetInstAttr(2));
				myfile << buf4;
				sprintf(buf4, "DEX (raw): %d. DEX (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(28), player->m_SChaPart.SLink[i].GetInstAttr(3));
				myfile << buf4;
				sprintf(buf4, "CON (raw): %d. CON (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(29), player->m_SChaPart.SLink[i].GetInstAttr(4));
				myfile << buf4;
				sprintf(buf4, "STA (raw): %d. STA (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(30), player->m_SChaPart.SLink[i].GetInstAttr(5));
				myfile << buf4;
				sprintf(buf4, "LUCK (raw): %d. LUCK (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(31), player->m_SChaPart.SLink[i].GetInstAttr(6));
				myfile << buf4;
				sprintf(buf4, "ASPD (raw): %d. ASPD(%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(32), player->m_SChaPart.SLink[i].GetInstAttr(7));
				myfile << buf4;
				sprintf(buf4, "ADIS (raw): %d. ADIS (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(33), player->m_SChaPart.SLink[i].GetInstAttr(8));
				myfile << buf4;
				sprintf(buf4, "MNATK (raw): %d. MNATK (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(34), player->m_SChaPart.SLink[i].GetInstAttr(9));
				myfile << buf4;
				sprintf(buf4, "MXATK (raw): %d. MXATK (%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(35), player->m_SChaPart.SLink[i].GetInstAttr(10));
				myfile << buf4;
				sprintf(buf4, "DEF (raw): %d. DEF(%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(36), player->m_SChaPart.SLink[i].GetInstAttr(11));
				myfile << buf4;
				sprintf(buf4, "MXHP (raw): %d. MXHP(%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(37), player->m_SChaPart.SLink[i].GetInstAttr(12));
				myfile << buf4;
				sprintf(buf4, "MXSP (raw): %d. MXSP(%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(38), player->m_SChaPart.SLink[i].GetInstAttr(13));
				myfile << buf4;
				sprintf(buf4, "FLEE (raw): %d. FLEE(%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(39), player->m_SChaPart.SLink[i].GetInstAttr(14));
				myfile << buf4;
				sprintf(buf4, "HIT (raw): %d. HIT(%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(40), player->m_SChaPart.SLink[i].GetInstAttr(15));
				myfile << buf4;
				sprintf(buf4, "CRT (raw): %d. CRT(%%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(41), player->m_SChaPart.SLink[i].GetInstAttr(16));
				myfile << buf4;
				sprintf(buf4, "MF (raw): %d. MF(%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(42), player->m_SChaPart.SLink[i].GetInstAttr(17));
				myfile << buf4;
				sprintf(buf4, "HREC (raw): %d. HREC(%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(43), player->m_SChaPart.SLink[i].GetInstAttr(18));
				myfile << buf4;
				sprintf(buf4, "SREC (raw): %d. SREC(%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(44), player->m_SChaPart.SLink[i].GetInstAttr(19));
				myfile << buf4;
				sprintf(buf4, "MSPD (raw): %d. MSPD(%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(45), player->m_SChaPart.SLink[i].GetInstAttr(20));
				myfile << buf4;
				sprintf(buf4, "COL (raw): %d. COL(%): %2.2f\n", player->m_SChaPart.SLink[i].GetInstAttr(46), player->m_SChaPart.SLink[i].GetInstAttr(21));
				myfile << buf4;
				sprintf(buf4, "PDEF (raw): %d. PDEF(%): %2.2f\n\n", player->m_SChaPart.SLink[i].GetInstAttr(47), player->m_SChaPart.SLink[i].GetInstAttr(22));
				myfile << buf4;
			}
	
		}
		myfile.close();
		}
	SystemNotice(".txt created!");
		return TRUE;
	}
	else if (!strcmp(szComHead, "editcharbag")) {
		int n = Util_ResolveTextLine(szComParam, strList, 16, ',');
		const char* charname = strList[0].c_str();
		int positionID = Str2Int(strList[1]);
		int ATTR_TYPE = Str2Int(strList[2]);
		int ATTR_VALUE = Str2Int(strList[3]);
		int TYPE = Str2Int(strList[4]);
		CCharacter* player = g_pGameApp->FindChaByName(charname);
		if (!player) return FALSE;
		char buffer[32];
		bool p;
		if (TYPE == 1) {
			p = player->m_CKitbag.GetGridContByID(positionID)->SetInstAttr(ATTR_TYPE, ATTR_VALUE);
		}
		else if (TYPE == 2) {
		 p = player->m_SChaPart.SLink[positionID].SetInstAttr(ATTR_TYPE, ATTR_VALUE);
		}
		else if (TYPE == 3) {
			p = player->GetPlayer()->GetBank()->GetGridContByID(positionID)->SetInstAttr(ATTR_TYPE, ATTR_VALUE);
		}
		
		sprintf(buffer, "Attribute updated (1 is ok, 0 is not ok) =  %d", p);
		SystemNotice(buffer);
		player->SynKitbagNew(enumSYN_KITBAG_PICK);
		return TRUE;
	}

	


	SystemNotice("Invalid command!");
	return FALSE;
T_E}

// ��ѯ������״̬
void CCharacter::DoCommand_CheckStatus(cChar *pszCommand, uLong ulLen)
{T_B
	Char szComHead[256], szComParam[256];
	std::string	strList[10];

	int n = Util_ResolveTextLine(pszCommand, strList, 10, ' ');
	strncpy(szComHead, strlwr((char *)strList[0].c_str()), 256 - 1);
	strncpy(szComParam, strList[1].c_str(), 256 - 1);

	string strCmd   = szComHead;
	string strParam = szComParam;

	if(strCmd=="game_status")	 // ����gameserver��״̬
	{
		char szInfo[255];
		sprintf(szInfo, "fps:%d tick:%d player:%d mgr:%d\n", g_pGameApp->m_dwFPS, 
			                     g_pGameApp->m_dwRunCnt, g_pGameApp->m_dwPlayerCnt,
								 g_pGameApp->m_dwActiveMgrUnit);
		SystemNotice(szInfo);
	}
	else if (strCmd=="ping_game") // ��ѯ��ɫ��GameServer�߼����pingֵ
	{
		int n = Util_ResolveTextLine(szComParam, strList, 10, ',');
		WPACKET WtPk  = GETWPACKET();
		WRITE_CMD(WtPk, CMD_MM_QUERY_CHAPING);
		WRITE_LONG(WtPk, GetID());
		WRITE_STRING(WtPk, strList[0].c_str());
		ReflectINFof(this, WtPk);//ͨ��
	}
T_E}

	
// NPC������Լ�˽�˵�˵���� �����޷�������
void NPC_PrivateTalk(CCharacter *pCha, CCharacter *pNPC, const char *pszText)
{
	WPACKET wpk	= GETWPACKET();
	WRITE_CMD(wpk, CMD_MC_SAY);
	WRITE_LONG(wpk, pNPC->m_ID);
	WRITE_SEQ(wpk, pszText, uShort(strlen(pszText) + 1));
	pCha->ReflectINFof(pCha, wpk);
}		

// ������������ѯ
void CCharacter::HandleHelp(cChar *pszCommand, uLong ulLen)
{T_B
	if(!pszCommand)           return;
	
	if(ulLen==0 || strlen(pszCommand)==0) 
	{
		//SystemNotice( "�������ʲô?" );
		SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00040) );
		return;
	}

	// �����Ҿ���npc̫Զ, Ҳ������
	if(GetSubMap()==NULL) return;
	if(strcmp(GetSubMap()->GetName(), "garner")!=0) return;

	int x = this->GetPos().x / 100;
	int y = this->GetPos().y / 100;

	if(g_HelpNPCList.size()==0) return;

	CCharacter *pNPC1 = g_HelpNPCList.front();
	if(pNPC1==NULL)
	{
		//LG("error", "��ѯNPCΪ��\n");
		LG("error", "inquire NPC is empty\n");
		return;
	}

	if(! ( abs(x - 2222) < 4 && abs(y - 2888) < 4) )
	{
		//SystemNotice( "����û�п��Իش��������!" );
		SystemNotice( RES_STRING(GM_CHARACTERSUPERCMD_CPP_00041) );
		return;
	}
	
	std::string	strList[3];
	int n = Util_ResolveTextLine(pszCommand, strList, 3, ' ');
	
	const char *pszHelp = FindHelpInfo(strList[0].c_str());
	
	
	char szTip[128]; 
	//sprintf(szTip, "�����й�'%s':\n", strList[0].c_str()); 
	sprintf(szTip, RES_STRING(GM_CHARACTERSUPERCMD_CPP_00042), strList[0].c_str()); 
	if(strList[0]=="time")	// ��ǰʱ���ѯ
	{
		//SystemNotice( szTip );
		//GetCurrentTime()
		//SystemNotice( "force is strong with this one!");
	}
	//else if(strList[0]=="ryan" || strList[0]=="��һ����ͷר��")
	else if(strList[0]=="ryan" || strList[0]==RES_STRING(GM_CHARACTERSUPERCMD_CPP_00043))
	{
		SystemNotice( szTip );
		NPC_PrivateTalk(this, pNPC1, "force is strong with this one!");
		return;
	}
	
	if(pszHelp==NULL)
	{
		SystemNotice( szTip );
		//NPC_PrivateTalk(this, pNPC1, "�����,��������ʹ���,���ʵ�����!");
		NPC_PrivateTalk(this, pNPC1, RES_STRING(GM_CHARACTERSUPERCMD_CPP_00044));
	}
	else
	{
		//if(strcmp(GetName(), "��һ����ͷר��")==0) // �˽�ɫ������Ϣ����Ǯ,��Ӱ����Ϸƽ��
		if(strcmp(GetName(), RES_STRING(GM_CHARACTERSUPERCMD_CPP_00043))==0) // �˽�ɫ������Ϣ����Ǯ,��Ӱ����Ϸƽ��
		{
			SystemNotice( szTip );
			NPC_PrivateTalk(this, pNPC1, pszHelp);
		}
		//else if(TakeMoney("����ͨ", 100))
		else if(TakeMoney(RES_STRING(GM_CHARACTERSUPERCMD_CPP_00045), 100))
		{
			SystemNotice( szTip );
			NPC_PrivateTalk(this, pNPC1, pszHelp);
		}
		else
		{
			SystemNotice( szTip );
			//NPC_PrivateTalk(this, pNPC1, "�Բ���, Money Talk!" );
			NPC_PrivateTalk(this, pNPC1, RES_STRING(GM_CHARACTERSUPERCMD_CPP_00046) );
		}
	}
T_E}