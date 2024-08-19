#include "StdAfx.h"
#include "procirculate.h"
#include "GameApp.h"
#include "PacketCmd.h"
#include "Character.h"
#include "ChaAttr.h"
#include "GameConfig.h"
#include "util2.h"
#include "LoginScene.h"
#include "netprotocol.h"
#include "AreaRecord.h"

using namespace std;
#include "blake2.h"
#include "hex.h"
#ifdef _TEST_CLIENT
#include "..\..\TestClient\testclient.h"
#endif

_DBC_USING

// Э��C->S : �����ж���Ϣ
void CProCirculateCS::BeginAction( CCharacter* pCha, DWORD type, void* param, CActionState* pState )
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_BEGINACTION);
#ifdef _TEST_CLIENT
	CTestClient* pClient = reinterpret_cast<CTestClient*>( g_NetIF->m_connect.GetDatasock()->GetPointer() );
	DWORD dwCharID=(pCha)?pCha->getAttachID():pClient->GetCharID();
	pk.WriteLong(dwCharID);
#else
	pk.WriteLong(pCha->getAttachID());
#endif

    char	szLogName[1024] = { "BeginAction" };

    if( pCha )
    {
        strcpy(szLogName, pCha->getLogName() );            
    }

	try
	{
		LG(szLogName, "$$$PacketID:\t%d\n", pCNetIf->m_ulPacketCount);

#ifdef defPROTOCOL_HAVE_PACKETID
		pk.WriteLong(pCNetIf->m_ulPacketCount++);
#endif
		pk.WriteChar((dbc::uChar)type);
		switch (type)
		{
		case	enumACTION_MOVE:
			{
				
				stNetMoveInfo *pMove = (stNetMoveInfo *)param;
				pk.WriteSequence((cChar *)pMove->pos_buf ,uShort(sizeof(Point) * pMove->pos_num));
				pCNetIf->SendPacketMessage(pk);


				char buffer[64] = {0};
				char buf[64] = {0};
				CCharacter	*pCha = CGameScene::GetMainCha();
				CGameScene	*pScene = g_pGameApp->GetCurScene();
				if(!pCha->IsBoat()) {
				int nArea = pScene->_pTerrain->GetTile(pCha->GetCurX()/100, pCha->GetCurY()/100 )->getIsland();
				CAreaInfo* pArea = GetAreaInfo( nArea );


				sprintf(buf, "In %s",pArea->szDataName);

				sprintf(buffer, "%s Lv%d %s", pCha->getHumanName(), pCha->getLv(), g_GetJobName((short)pCha->getGameAttr()->get(ATTR_JOB)));
				if(pCha->GetTeamLeaderID() > 0) {
				}else{

				updateDiscordPresence(buffer, buf );

				}
				}else {
					sprintf(buffer, "%s Lv%d %s", pCha->getHumanName(), pCha->getLv(), g_GetJobName((short)pCha->getGameAttr()->get(ATTR_JOB)));
					sprintf(buf, "Sailing");
					updateDiscordPresence(buffer, buf );
				}
				
				




				LG(szLogName, "###Send(Move):\tTick:[%d]\n", GetTickCount());
				LG(szLogName, "Ping:\t%3d\n", pMove->dwAveragePing);
				LG(szLogName, "Point:\t%3d\n", pMove->pos_num);
				for (DWORD i = 0; i < pMove->pos_num; i++)
				{
					LG(szLogName, "\t%d, \t%d\n", pMove->pos_buf[i].x, pMove->pos_buf[i].y);
				}
				LG(szLogName, "\n");
				break;
			}
		case	enumACTION_SKILL:
			{
				stNetSkillInfo *pSkill = (stNetSkillInfo *)param;
				pk.WriteChar(pSkill->chMove);
				pk.WriteChar(pSkill->byFightID);
				if (pSkill->chMove == 2)
				{
					pk.WriteSequence((cChar *)pSkill->SMove.pos_buf, uShort(sizeof(POINT) * pSkill->SMove.pos_num));
				}
				pk.WriteLong(pSkill->lSkillID);
				pk.WriteLong(pSkill->lTarInfo1);
				pk.WriteLong(pSkill->lTarInfo2);

				//for (int n = 0; n < 200; ++n)
				//{
					pCNetIf->SendPacketMessage(pk);
				//}

				LG(szLogName, "###Send(Skill):\tTick:[%d]\n", GetTickCount());
				if (pSkill->chMove == 2)
				{
					LG(szLogName, "Ping:\t%3d\n", pSkill->SMove.dwAveragePing);
					LG(szLogName, "Point:\t%3d\n", pSkill->SMove.pos_num);
					for (DWORD i = 0; i < pSkill->SMove.pos_num; i++)
					{
						LG(szLogName, "\t%d, \t%d\n", pSkill->SMove.pos_buf[i].x, pSkill->SMove.pos_buf[i].y);
					}
				}
				LG(szLogName, "Skill:\t%3d, FightID:%d\n", pSkill->lSkillID, pSkill->byFightID);
				LG(szLogName, "Target:\t%u, \t%u\n", pSkill->lTarInfo1, pSkill->lTarInfo2);
				LG(szLogName, "\n");
				break;
			}
		case	enumACTION_STOP_STATE:
			{
				pk.WriteShort(*((short *)param));
				pCNetIf->SendPacketMessage(pk);

				// log
				LG(szLogName, "###Send(Stop Skill State %d):\tTick:[%d]\n", *((short *)param), GetTickCount());
				LG(szLogName, "\n");
				//
				break;
			}
		case	enumACTION_LEAN: // �п�
			{
				stNetLeanInfo *pSLean = (stNetLeanInfo *)param;
				pk.WriteLong(pSLean->lPose);
				pk.WriteLong(pSLean->lAngle);
				pk.WriteLong(pSLean->lPosX);
				pk.WriteLong(pSLean->lPosY);
				pk.WriteLong(pSLean->lHeight);
				pCNetIf->SendPacketMessage(pk);

				// log
				LG(szLogName, "###Send(Lean):\tTick:[%d]\n", GetTickCount());
				LG(szLogName, "\n");
				//
				break;
			}
		case	enumACTION_ITEM_PICK: // �����
			{
				stNetItemPick *pPick = (stNetItemPick *)param;
				pk.WriteLong(pPick->lWorldID);
				pk.WriteLong(pPick->lHandle);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Pick):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_ITEM_THROW: // ������
			{
				stNetItemThrow *pThrow = (stNetItemThrow *)param;
				pk.WriteShort(pThrow->sGridID);
				pk.WriteShort((short)pThrow->lNum);
				pk.WriteLong(pThrow->lPosX);
				pk.WriteLong(pThrow->lPosY);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Throw):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_ITEM_USE:
			{
				stNetUseItem *pUseItem = (stNetUseItem *)param;
				pk.WriteShort(pUseItem->sGridID);
				pk.WriteShort(pUseItem->sTarGridID);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Use Item):\tTick:[%d]\n", GetTickCount());
				LG(szLogName, g_oLangRec.GetString(322), pUseItem->sGridID, pUseItem->sTarGridID);
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_ITEM_UNFIX: // жװ����
			{
				stNetItemUnfix *pUnfix = (stNetItemUnfix *)param;
				pk.WriteChar(pUnfix->chLinkID);
				pk.WriteShort(pUnfix->sGridID);
				if (pUnfix->sGridID < 0) // ��������
				{
					pk.WriteLong(pUnfix->lPosX);
					pk.WriteLong(pUnfix->lPosY);
				}
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Unfix):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_ITEM_POS:
			{
				stNetItemPos *pChangePos = (stNetItemPos *)param;
				pk.WriteShort(pChangePos->sSrcGridID);
				pk.WriteShort(pChangePos->sSrcNum);
				pk.WriteShort(pChangePos->sTarGridID);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Item pos):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_ITEM_DELETE:
			{
				stNetDelItem *pDelItem = (stNetDelItem *)param;
				pk.WriteShort(pDelItem->sGridID);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Del Item):\tTick:[%d]\n", GetTickCount());
				LG(szLogName, g_oLangRec.GetString(323), pDelItem->sGridID);
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_ITEM_INFO:
			{
				stNetItemInfo *pItemInfo = (stNetItemInfo *)param;
				pk.WriteChar(pItemInfo->chType);
				pk.WriteShort(pItemInfo->sGridID);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Item Info):\tTick:[%d]\n", GetTickCount());
				LG(szLogName, g_oLangRec.GetString(324), pItemInfo->chType, pItemInfo->sGridID);
                LG(szLogName, "\n");
				break;
				break;
			}
		case	enumACTION_SHORTCUT: // ���¿����
			{
				stNetShortCutChange *pShortcutChange = (stNetShortCutChange *)param;
				pk.WriteChar(pShortcutChange->chIndex);
				pk.WriteChar(pShortcutChange->chType);
				pk.WriteShort(pShortcutChange->shyGrid);
				//pk.WriteByte(pShortcutChange->shyGrid2==-1?0:1);
				//pk.WriteShort(pShortcutChange->shyGrid2);
				pCNetIf->SendPacketMessage(pk);
				break;
			}
		case	enumACTION_LOOK: // ������ۣ��紬�Ļ�װ��
			{
				stNetChangeChaPart *pSChaPart = (stNetChangeChaPart *)param;
				pk.WriteShort(pSChaPart->sTypeID);
				for (int i = 0; i < enumEQUIP_NUM; i++)
				{
					pk.WriteShort(pSChaPart->SLink[i].sID);
				}
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Look):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_TEMP: //��ʱ��װЭ��
			{
				stTempChangeChaPart *pSTempChaPart = (stTempChangeChaPart *)param;
				pk.WriteLong(pSTempChaPart->dwItemID);
				pk.WriteLong(pSTempChaPart->dwPartID);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Temp):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_EVENT: // �����¼�
			{
				stNetActivateEvent *pEvent = (stNetActivateEvent *)param;
				pk.WriteLong(pEvent->lTargetID);
				pk.WriteLong(pEvent->lHandle);
				pk.WriteShort(pEvent->sEventID);
                pCNetIf->SendPacketMessage(pk);

				// log
				LG(szLogName, "###Send(Event):\tTick:[%d]\n", GetTickCount());
				LG(szLogName, "\n");
				//
				break;
			}
		case	enumACTION_FACE:
			{
				stNetFace	*pNetFace = (stNetFace *)param;
				pk.WriteShort(pNetFace->sAngle);
				pk.WriteShort(pNetFace->sPose);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Face):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case enumACTION_ChangepName: {
			const auto name = static_cast<const char*>(param); // (const char*)param;
			pk.WriteString(name);
			pCNetIf->SendPacketMessage(pk);
			break;
		}
		case	enumACTION_SKILL_POSE:
			{
				stNetFace	*pNetFace = (stNetFace *)param;
				pk.WriteShort(pNetFace->sAngle);
				pk.WriteShort(pNetFace->sPose);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Skill Pos):\tTick:[%d]\n", GetTickCount());
				LG(szLogName, "Angle:%d, Pose:%d\n", pNetFace->sAngle, pNetFace->sPose );
				break;
			}
		case enumACTION_GUILDBANK:
		case	enumACTION_BANK:
			{
				stNetBank	*pNetBank = (stNetBank *)param;
				pk.WriteChar(pNetBank->chSrcType);
				pk.WriteShort(pNetBank->sSrcID);
				pk.WriteShort(pNetBank->sSrcNum);
				pk.WriteChar(pNetBank->chTarType);
				pk.WriteShort(pNetBank->sTarID);
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Bank Req):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_CLOSE_BANK:
			{
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(Bank Close):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_REQUESTGUILDLOGS:
			{
				uShort* curSize = reinterpret_cast<uShort*> (param);
				pk.WriteShort(*curSize);

				pCNetIf->SendPacketMessage(pk);
				break;
			}	
		case	enumACTION_UPDATEGUILDLOGS:
			{
				pCNetIf->SendPacketMessage(pk);
				break;
			}

		
		case	enumACTION_REQUESTGUILDBANK:
			{
				pCNetIf->SendPacketMessage(pk);

                LG(szLogName, "###Send(request guild):\tTick:[%d]\n", GetTickCount());
                LG(szLogName, "\n");
				break;
			}
		case	enumACTION_KITBAGTMP_DRAG:	// �϶���ʱ����
			{
				stNetTempKitbag* pNetTempKitbag = (stNetTempKitbag*)param;

				pk.WriteShort(pNetTempKitbag->sSrcGridID);
				pk.WriteShort(pNetTempKitbag->sSrcNum);
				pk.WriteShort(pNetTempKitbag->sTarGridID);

				pCNetIf->SendPacketMessage(pk);
				break;
			}
		case enumACTION_GOLDSTORE: {
			int gold = (int)param;
			pk.WriteLong(gold);
			pCNetIf->SendPacketMessage(pk);
			break;
		}
		default:
			break;
		}
	}
	catch (...)
	{
		MessageBox(0, "!!!!!!!!!!!!!!!!!!!!exception: Begin Action", "error", 0);
	}
}

// Э��C->S : ����ֹͣ�ж���Ϣ
void CProCirculateCS::EndAction( CActionState* pState )
{
	WPacket pk	=pCNetIf->GetWPacket();

	pk.WriteCmd(CMD_CM_ENDACTION);	//�����ж�
	pCNetIf->SendPacketMessage(pk);

	// log
    CCharacter *pMainCha = CGameApp::GetCurScene()->GetMainCha();
	if (pMainCha)
	{
		LG(pMainCha->getLogName(), "###Send(EndAction):\tTick:[%d]\n", GetTickCount());
	}
	//
}

bool CProCirculate::Connect(const char *hostname,unsigned short port,unsigned long timeout)
{
	return pCNetIf->m_connect.Connect(hostname,port,timeout);
}
void CProCirculate::Disconnect(int reason)
{
	pCNetIf->m_connect.Disconnect(reason);
}

bool CProCirculate::SendPrivateKey()
{
	CryptoPP::RSAES_OAEP_SHA_Encryptor e(g_NetIF->srvPublicKey);
	// Get AES Private Key -> Encrypt w/ srvPublicKey -> Base64Encode
	string spki_private;
	string cipher_private;
	string base64encoded_private;
	CryptoPP::StringSource ss(g_NetIF->cliPrivateKey.data(), g_NetIF->cliPrivateKey.size(), true, new CryptoPP::PK_EncryptorFilter(g_NetIF->rng, e, new CryptoPP::StringSink(cipher_private)));
	CryptoPP::StringSource ss2(cipher_private, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(base64encoded_private)));


	WPacket pk = pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_SEND_PRIVATE_KEY);
	// Send cipher to GateServer
	pk.WriteString(base64encoded_private.c_str());
	
	pCNetIf->SendPacketMessage(pk);
	g_NetIF->handshakeDone = true;
	g_NetIF->_comm_enc = true;
	g_NetIF->m_connect.CHAPSTR(false);
	return true;

}


void CProCirculate::Login(const char *accounts,const char *password, const char* passport)
{
	if (!g_NetIF->handshakeDone) 
	{
		// Do not continue with login if the key exchange failed.
		return;
	}

	extern short g_sClientVer;
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_LOGIN);
	pk.WriteString(accounts);

	string digest;
	string hexencoded;
	CryptoPP::BLAKE2s d;
	CryptoPP::StringSource ss(password, true, new CryptoPP::HashFilter(d, new CryptoPP::StringSink(digest)));
	CryptoPP::StringSource ss2(digest, true, new CryptoPP::HexEncoder(new CryptoPP::StringSink(hexencoded)));
	pk.WriteString(hexencoded.c_str());
	string strMac=GetMacString();
	if (strMac.empty()) strMac="Unknown";
	pk.WriteString(strMac.c_str());
	pk.WriteShort(911);
	pk.WriteShort(g_sClientVer);

	pCNetIf->SendPacketMessage(pk);
}

void CProCirculate::Logout()
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_LOGOUT);
	pCNetIf->SyncSendPacketMessage(pk,3000);
}

void CProCirculate::BeginPlay(char cha_index)
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_BGNPLAY);
	pk.WriteChar(cha_index);

	pCNetIf->SendPacketMessage(pk);
}

void CProCirculate::EndPlay()
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_ENDPLAY);
	pCNetIf->SendPacketMessage(pk);
}

void CProCirculate::NewCha(const char* chaname, const char* birth, int type, int hair, int face)
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_NEWCHA);
	pk.WriteString(chaname);
	pk.WriteString(birth);
	pk.WriteLong(type);
	pk.WriteLong(hair);
	pk.WriteLong(face);
	pCNetIf->SendPacketMessage(pk);
}

void CProCirculate::DelCha(uint8_t cha_index, const char szPassword2[])
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_DELCHA);
	pk.WriteChar(cha_index);
	pk.WriteString(szPassword2);

	pCNetIf->SendPacketMessage(pk);
}

//------------------------
// Э��C->S : ����������Ϣ
//------------------------
void CProCirculate::OpenRankings()
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_RANK);
	pCNetIf->SendPacketMessage(pk);
}

void CProCirculate::Say(const char *content)
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_SAY);			//����
	pk.WriteSequence(content, uShort(strlen(content))+1);
	pCNetIf->SendPacketMessage(pk);
}

//----------------------------
// Э��C->S : ���ͷ������Ե�����
//----------------------------
void CProCirculate::SynBaseAttribute(CChaAttr *pCAttr)
{
	char	chAttrNum = 0;
	for (int i = ATTR_STR; i <= ATTR_LUK; i++)
		if (pCAttr->GetChangeBitFlag(i))
			chAttrNum++;

	if (chAttrNum == 0)
		return;

	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_SYNATTR);	//ͬ����������
	pk.WriteChar(chAttrNum);
	for (int i = ATTR_STR; i <= ATTR_LUK; i++)
	{
		if (pCAttr->GetChangeBitFlag(i))
		{
			pk.WriteShort(i);
			pk.WriteLong((uLong)pCAttr->GetAttr(i));
		}
	}

	// log
	char szReqChangeAttr[256] = {0};
	strcpy(szReqChangeAttr, g_oLangRec.GetString(325));

	for (int i = ATTR_STR; i <= ATTR_LUK; i++)
	{
		if (pCAttr->GetChangeBitFlag(i))
			LG(szReqChangeAttr, g_oLangRec.GetString(326), i, pCAttr->GetAttr(i));
	}
	LG(szReqChangeAttr, "\n");

	pCNetIf->SendPacketMessage(pk);
}

//----------------------------
// Э��C->S : ����ˢ�½�ɫ���Ե�����
//----------------------------
void CProCirculate::RefreshChaData(long lWorldID, long lHandle)
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_REFRESH_DATA);	//ͬ����������
	pk.WriteLong(lWorldID);
	pk.WriteLong(lHandle);

	pCNetIf->SendPacketMessage(pk);
}

void CProCirculate::SkillUpgrade(short sSkillID, char chAddLv)
{
	WPacket pk	=pCNetIf->GetWPacket();
	pk.WriteCmd(CMD_CM_SKILLUPGRADE);	//ͬ����������
	pk.WriteShort(sSkillID);
	pk.WriteChar(chAddLv);

	// log
	char szReqChangeAttr[256] = {0};
	strcpy(szReqChangeAttr, g_oLangRec.GetString(327));

	LG(szReqChangeAttr, g_oLangRec.GetString(328), sSkillID, chAddLv);

	pCNetIf->SendPacketMessage(pk);
}

