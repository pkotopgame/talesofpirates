#pragma once

#include "TableData.h"
#include "MPRender.h"

#include "cryptlib.h"
#include "filters.h"
#include "files.h"
#include <fstream>

#pragma warning(disable: 4275)

class MINDPOWER_API MPTexInfo : public CRawDataInfo
{
public:
	
	MPTexInfo()
	{
		bAlpha   = 0;
        btMipmap = 5;
        bExist   = TRUE;
	}

public:	

	BYTE    bAlpha;
    BYTE    btMipmap;
	short	sWidth;
	short	sHeight;
};

#pragma pack(push,__Data_Align)
#pragma pack(1)

struct STGAHeader
{
	unsigned char    id_len;
	unsigned char    cm_type;
	unsigned char    type;
	unsigned short   cm_start;
	unsigned short   cm_len;
	unsigned char    cm_bits;
	unsigned short   xorg;
	unsigned short   yorg;
	unsigned short   width;
	unsigned short   height;
	unsigned char    bpp;
	unsigned char    flags;
};

#pragma pack(pop,__Data_Align)


class MINDPOWER_API MPTexSet : public CRawDataSet
{

public:
	
	static MPTexSet* I() { return _Instance; }

    MPTexSet(int nIDStart, int nIDCnt)
	:CRawDataSet(nIDStart, nIDCnt)
	{
	    _Init();
        _Instance = this;
        _nTextureLevel = D3DX_DEFAULT;
		CRawDataSet::SetReleaseInterval(8000);
	}

    
	BOOL IsAlphaTGA(char* pFileBuf)
	{
		STGAHeader header;
		memcpy(&header, pFileBuf, sizeof(STGAHeader));
		return (header.bpp == 32);
	}

	BOOL IsAlphaTexture(int nID)
	{
		MPTexInfo* pInfo = (MPTexInfo*)GetRawDataInfo(nID);
		if(pInfo && pInfo->bAlpha== 1)
		{
			return TRUE;
		}
		return FALSE;
	}

	void    ReloadAllTexture()
    {
        for(int i = 0; i < _nIDCnt; i++)
	    {
		    CRawDataInfo *pInfo = GetRawDataInfo(_nIDStart + i);
		    if(pInfo->pData)
            {
                _DeleteRawData(pInfo);
		        pInfo->pData = NULL;
		        _nLoadedRawDataCnt--;
		        if(_nLoadedRawDataCnt < 0)
		        {
			        LG("error", "LoadedRawDataCnt = %d , < 0 ?\n", _nLoadedRawDataCnt);
		        }
                GetRawData(pInfo->nID);
            }
        }
    }
    
    void    SetTextureLevel(int nLevel, BOOL bUpdateAll = FALSE)
    {
        if(_nTextureLevel!=nLevel)
        {
            _nTextureLevel = nLevel;
            ReloadAllTexture();
        }
    }

    int     GetTextureLevel()  { return _nTextureLevel; }

protected:

	static MPTexSet* _Instance; // �൱�ڵ���, ���Լ���ס
    
    virtual CRawDataInfo* _CreateRawDataArray(int nIDCnt)
	{
		return new MPTexInfo[nIDCnt];
	}
	
	virtual void _DeleteRawDataArray()
	{
		delete[] (MPTexInfo*)_RawDataArray;
	}
	
	virtual int _GetRawDataInfoSize()
	{
		return sizeof(MPTexInfo);
	}
	void CryptImage(bool encrypt, char* filename, char* sinkbuffer) {
		const unsigned char imgTableKey[] = { 0x48, 0x73, 0x29, 0xCA, 0xBB, 0x54, 0xCF, 0xB0, 0xF4, 0xBF, 0x70, 0xA0, 0xAA, 0x4B, 0x12, 0xF5 };
		const unsigned char imgTableIV[] = { 0x43, 0x2a, 0x46, 0x29, 0x4a, 0x40, 0x4e, 0x63, 0x52, 0x66, 0x55, 0x6a, 0x58, 0x6e, 0x32, 0x72 };
		try {
			if (encrypt) {
				std::string encFilename = filename;
				encFilename.replace(encFilename.length() - 3, 3, "wsd");
				CryptoPP::GCM<CryptoPP::AES>::Encryption e;
				e.SetKeyWithIV(imgTableKey, 16, imgTableIV, 16);
				CryptoPP::FileSource(filename, true, new CryptoPP::AuthenticatedEncryptionFilter(e, new CryptoPP::FileSink(encFilename.c_str())));
				return;
			}
			else {
				CryptoPP::GCM<CryptoPP::AES>::Decryption d;
				d.SetKeyWithIV(imgTableKey, 16, imgTableIV, 16);
				std::string buffer;
				CryptoPP::FileSource(filename, true, new CryptoPP::AuthenticatedDecryptionFilter(d, new CryptoPP::StringSink(buffer)));
				memcpy(sinkbuffer, buffer.data(), buffer.size());
				return;
			}
		}
		catch (...) {}
	}


	virtual BOOL _IsFull()
	{
		// ����Դ�ռ�ó���8M,��ʼ���н�����ͼ�͵ر���ͼ�Ķ�̬�ͷ�
		DWORD dwMem = g_Render.GetRegisteredDevMemSize();
		if(rand()%100 < 2)
		{
			LG("vmem", "vmem = %d k\n", dwMem / 1024);
		}
		if(dwMem >= 64 * 1024 * 1024)
		{
			return TRUE;
		}
		return FALSE;
	}

	virtual void* _CreateNewRawData(CRawDataInfo* pInfo) {
#define USE_MANAGED_TEXTURE

#ifdef USE_MANAGED_TEXTURE

			// This is where textures are loaded, either from a file or from memory.
			MPTexInfo* pTexInfo = (MPTexInfo*)pInfo;
			lwITex* tex;
			lwIResourceMgr* res_mgr = g_Render.GetInterfaceMgr()->res_mgr;
			res_mgr->CreateTex(&tex);

			lwTexInfo tex_info;
			lwTexInfo_Construct(&tex_info);

			_tcscpy(tex_info.file_name, pTexInfo->szDataName);
			_tcslwr(tex_info.file_name);
			tex_info.pool = D3DPOOL_MANAGED;
			tex_info.usage = 0;
			tex_info.level = D3DX_DEFAULT;

			size_t len = strlen(pTexInfo->szDataName);
			if (pTexInfo->szDataName[len - 1] == 'd' &&
				pTexInfo->szDataName[len - 2] == 's' &&
				pTexInfo->szDataName[len - 3] == 'w') {
				std::ifstream data(pTexInfo->szDataName, std::ios::binary);
				data.seekg(0, std::ios::end);
				int size = data.tellg();
				// Data is a pointer towards the decrypted image data
				// Inside a try/catch to avoid bad_alloc when size = 0 bytes (nonexistent UI file)
				try {
					tex_info.data = new char[size - 16];
				}
				catch (...) {
					return 0;
				}
				// Decrypt image
				CryptImage(false, pTexInfo->szDataName, (char*)tex_info.data);
				// LoadTexInfo will take care of our data if we specify the type and width (size) correctly
				tex_info.type = TEX_TYPE_DATA;
				tex_info.width = size - 16;
				// According to docs, D3DFMT_UNKNOWN will make sure the DirectX method chooses the correct format according to the file.
				tex_info.format = D3DFMT_UNKNOWN;

			}
			else {
				// Not loading from memory (basically any LoadImage that is not called by UI_LoadImage)
				tex_info.type = TEX_TYPE_FILE;
			}




			// ���dds�ļ���ʽ����Ҫ���ã�ֻ�ж�tga��ʽ
			/*
			BYTE* pbtBuf = LoadRawFileData(pTexInfo);
			if(pbtBuf)
			{
				tex_info.format = tex_fmt[IsAlphaTGA((char*)pbtBuf)];
				delete[] pbtBuf;
			}
			else
			{
				tex_info.format = tex_fmt[0];
			}
			*/
			D3DFORMAT tex_fmt[2];
			tex_fmt[0] = g_Render.GetTexSetFormat(0);
			tex_fmt[1] = g_Render.GetTexSetFormat(1);
			size_t l = _tcslen(tex_info.file_name);

			// Probably useless now, with D3DFMT_UNKNOWN
			if (tex_info.file_name[l - 1] == 'a'
				&& tex_info.file_name[l - 2] == 'g'
				&& tex_info.file_name[l - 3] == 't')
			{
				tex_info.format = tex_fmt[1];
			}
			else
			{
				tex_info.format = tex_fmt[0];
			}
			//////
#if 1
			if (_tcsstr(tex_info.file_name, "ui"))
			{
				tex_info.level = D3DX_DEFAULT;

				size_t str_len = _tcslen(tex_info.file_name);

				if (_tcsicmp(&tex_info.file_name[str_len - 3], "bmp") == 0)
				{
					// This is broken atm - UI files that end with .wsd still have their purple transparency color ingame.
					// New UI is not bmp though, so leaving like this for the moment
					tex_info.colorkey_type = COLORKEY_TYPE_COLOR;
					tex_info.colorkey.color = 0xffff00ff;
					tex_info.format = tex_fmt[1];
				}
			}
#endif

			tex->LoadTexInfo(&tex_info, NULL);
			//LoadVideoMemory calls D3DXCreateTextureFromFileInMemoryEx
			tex->LoadVideoMemory();

			tex->GetTexInfo(&tex_info);

			//return tex->GetTex();
			pTexInfo->sWidth = (short)tex_info.width;
			pTexInfo->sHeight = (short)tex_info.height;
			LG("tex_release", "Load Texture [%s] size = %d %d, id = %d\n", pTexInfo->szDataName, pTexInfo->sWidth, pTexInfo->sHeight, pInfo->nID);
			return tex;
#else
		MPTexInfo* pTexInfo = (MPTexInfo*)pInfo;

		char pszSrcName[64] = "";

		BOOL bReplaceDataName = FALSE;
		if (_nTextureLevel > 0)
		{
			strcpy(pszSrcName, pTexInfo->szDataName);
			char* pszLevelTex = "tex___1";
			memcpy(pTexInfo->szDataName, pszLevelTex, 7);

			if (!Util_IsFileExist(pTexInfo->szDataName))
			{
				strcpy(pTexInfo->szDataName, pszSrcName);
			}
			else
			{
				bReplaceDataName = TRUE;
			}
		}


		BOOL bDDS = FALSE;
		DWORD  dwBufSize;

		// get 'dds' texture file name
		CRawDataInfo ddsInfo;
		int len = (int)(strlen(pInfo->szDataName));
		strcpy(ddsInfo.szDataName, pInfo->szDataName);
		ddsInfo.szDataName[len - 3] = 'd';
		ddsInfo.szDataName[len - 2] = 'd';
		ddsInfo.szDataName[len - 1] = 's';

		// try 'dds' texture at 1st time 
		LPBYTE pbtBuf = NULL;

		/*
		if(pTexInfo->nMipmap != 0xff && g_pDevice->m_bEnableDXT)	// UI texture don't use 'dds'
		//if (0)
		{
			pbtBuf = LoadRawFileData(&ddsInfo);
		}
		*/

		if (!pbtBuf)
		{
			// try 'tga' texture at 2nd time 
			pbtBuf = LoadRawFileData(pTexInfo);
			if (!pbtBuf)
			{
				if (bReplaceDataName)
				{
					strcpy(pTexInfo->szDataName, pszSrcName);
				}
				return NULL;
			}
			else
			{
				dwBufSize = pTexInfo->dwDataSize;
			}
		}
		else
		{
			dwBufSize = ddsInfo.dwDataSize;

			/*DDSURFACEDESC2 ddsc2;
			memcpy(&ddsc2, pbtBuf + 4, sizeof(DDSURFACEDESC2));
			if (ddsc2.dwAlphaBitDepth > 0)
			{
				pTexInfo->bAlpha = 1;
			}
			else
			{
				pTexInfo->bAlpha = 0;
			}*/
			bDDS = TRUE;
		}

		if (bReplaceDataName)
		{
			strcpy(pTexInfo->szDataName, pszSrcName);
		}

		if (!bDDS)
		{
			if (IsAlphaTGA((char*)pbtBuf))
			{
				pTexInfo->bAlpha = 1;
			}
			else
			{
				pTexInfo->bAlpha = 0;
			}
		}

		DWORD   dwUsage = 0;
		D3DPOOL Manage = D3DPOOL_MANAGED;
		DWORD   dwFilter = D3DX_FILTER_NONE;
		DWORD   dwMipFilter = D3DX_FILTER_NONE;
		BYTE btMipMapLv = 1;
		if (pTexInfo->btMipmap != 0xff)
		{
			dwFilter = D3DX_FILTER_LINEAR;
			dwMipFilter = D3DX_DEFAULT;
			btMipMapLv = pTexInfo->btMipmap;
		}

		btMipMapLv = D3DX_DEFAULT;

		// DWORD  dwBufSize = pInfo->dwDataSize;

		IDirect3DTextureX* pTexture = NULL;


		D3DFORMAT fmt = D3DFMT_R5G6B5;
		if (pTexInfo->bAlpha) fmt = D3DFMT_A4R4G4B4;

		if (FAILED(D3DXCreateTextureFromFileInMemoryEx(g_Render.GetDevice(),
			pbtBuf,
			dwBufSize,
			D3DX_DEFAULT,				// default width
			D3DX_DEFAULT,				// default height
			btMipMapLv,					// mipmap level
			dwUsage,					// usage
			// D3DFMT_DXT3,
			fmt,			       		// Texture Format
			Manage, 	        		// Pool
			dwFilter,       			// texture filter
			dwMipFilter,				// MipMap filter
			0x00000000,					// Alpha 
			NULL,						// info
			NULL,						// palette
			&pTexture)))				// texture
		{
			// texture
			LG("error", "Create Texture From Data[%s] Failed!\n", pInfo->szDataName);
			pTexture = NULL;
		}
		else
		{
			LG("debug", "Load Texture Data [%s]\n", pInfo->szDataName);
			//D3DSURFACE_DESC desc;
			//pTexture->GetLevelDesc(0, &desc);
			//pTexInfo->nWidth  = desc.Width;
			//pTexInfo->nHeight = desc.Height;
		}
		delete[] pbtBuf;

		return pTexture;
#endif
	}

	virtual void  _DeleteRawData(CRawDataInfo *pInfo)
	{
		MPTexInfo *pTexInfo = (MPTexInfo*)pInfo;
        LG("tex_release", "Release Texture Data [%s], size = %d %d\n", pTexInfo->szDataName, pTexInfo->sWidth, pTexInfo->sHeight);
        // by lsh
        // __asm
        // {
        //    int 3;
        // }
#if(defined USE_MANAGED_TEXTURE)
        lwITex* pTex = (lwITex*)pInfo->pData;
#else
		IDirect3DTextureX* pTex = (IDirect3DTextureX*)(pInfo->pData);
#endif
		lwTexInfo tex_info;
		pTex->GetTexInfo(&tex_info);

		if (std::string_view(pTexInfo->szDataName).ends_with(".wsd"))
		{
			auto p = reinterpret_cast<char*>(tex_info.data);
			SAFE_DELETE_ARRAY(p);

		}
		SAFE_RELEASE(pTex);
	}
	
	virtual BOOL _ReadRawDataInfo(CRawDataInfo *pRawDataInfo, std::vector<std::string> &ParamList)
	{
        if(ParamList.size()==0) return FALSE;
		
        std::string strParam;
		for(std::vector<std::string>::iterator it = ParamList.begin(); it!=ParamList.end(); it++)
		{
			strParam+=(*it);
		}
		
		MPTexInfo *pTexInfo = (MPTexInfo*)pRawDataInfo;
        return TRUE;
    }

protected:

    int     _nTextureLevel;
};

//--------------
// ���ȫ�ֺ���:
//--------------

inline lwITex* GetTexByID(int nID, BOOL bRequest = FALSE)
{
    lwITex* tex = (lwITex*)MPTexSet::I()->GetRawData(nID, bRequest);
    return tex;
}

inline IDirect3DTextureX* GetTextureByID(int nID, BOOL bRequest = FALSE) // ͨ��IDȡ��Texture����
{
#ifdef USE_MANAGED_TEXTURE
    lwITex* tex = (lwITex*)MPTexSet::I()->GetRawData(nID, bRequest);
    return tex ? tex->GetTex() : 0;
#else
    return (IDirect3DTextureX*)MPTexSet::I()->GetRawData(nID, bRequest);
#endif
}

inline SIZE GetTextureSizeByID(int nID)
{
    SIZE sz = { 0, 0};
#ifdef USE_MANAGED_TEXTURE
    lwITex* tex = (lwITex*)MPTexSet::I()->GetRawData(nID, 0);
    if(tex)
    {
        lwTexInfo info;
        tex->GetTexInfo(&info);
        sz.cx = info.width;
        sz.cy = info.height;
    }
#endif

    return sz;
}

inline int GetTextureID(const char *pszDataName)  // ͨ������ȡ��Texture ID, ����Ƕ�̬ID��ʽ��û�в��ҵ�ID, �ͻ��Զ�����һ��, ����-1��ʾʧ��
{
    if(MPTexSet::I()!=NULL)
	{
		return MPTexSet::I()->GetRawDataID(pszDataName);
	}

	return 0;
}

inline MPTexInfo* GetTextureInfo(int nID) // ͨ��IDȡ��Texture������Ϣ
{
	if(MPTexSet::I()!=NULL)
	{
		return (MPTexInfo*)MPTexSet::I()->GetRawDataInfo(nID);
	}
	return NULL;
}

inline BOOL IsAlphaTexture(int nID)
{
	MPTexInfo *pInfo = GetTextureInfo(nID);
	if(!pInfo) return FALSE;
	return pInfo->bAlpha==1;
}

inline BOOL IsTextureExist(const char *pszDataName)
{
    if(MPTexSet::I()->IsEnablePack()==FALSE) // �Ǵ����ʽ��
    {
        if(Util_IsFileExist((char*)pszDataName))
        {
            return TRUE;
        }
        return FALSE;
    }
    else
    {
        MPTexInfo *pInfo = (MPTexInfo*)MPTexSet::I()->GetRawDataInfo(pszDataName);
        if(pInfo)
        {
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

#pragma warning(default: 4275)