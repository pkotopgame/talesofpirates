﻿#include "stdafx.h"
#include "ZRBlock.h"
#include <filesystem>
#include <fstream>

BOOL ZRBlock::Load(const char* pszMapName, BOOL bEdit)
{
	if (fs.is_open()) {
		fs.close();
	}

	if (bEdit) {
		namespace fs = std::filesystem;
		fs::permissions(pszMapName,
			fs::perms::owner_write,
			fs::perm_options::add);
	}

	fs.open(pszMapName, [&] {
		auto flags = std::ios_base::binary | std::ios_base::in;
		if (bEdit) {
			flags |= std::ios_base::out;
		}
		return flags;
		}());

	if (!fs.is_open()) {
		LG("map", "msgLoad Map [%s] Error!\n", pszMapName);
		return false;
	}

	MPMapFileHeader header;
	fs.read(reinterpret_cast<char*>(&header), sizeof(MPMapFileHeader));

	if (header.nMapFlag == MP_MAP_FLAG + 1) {
		fs.close();
		LG("map", "msg该地图文件[%s]版本过期, 请使用MapTool打开它来升级版本!", pszMapName);
		return FALSE;
	}

#ifdef NEW_VERSION
	if (header.nMapFlag != MP_MAP_FLAG + 3)
#else
	if (header.nMapFlag != MP_MAP_FLAG + 2)
#endif
	{
		fs.close();
		LG("map", "msg[%s]不是有效的 MindPower Map File!\n", pszMapName);
		return FALSE;
	}

	m_nWidth = header.nWidth;
	m_nHeight = header.nHeight;
	m_nSectionWidth = header.nSectionWidth;
	m_nSectionHeight = header.nSectionHeight;

	m_nSectionCntX = m_nWidth / m_nSectionWidth;
	m_nSectionCntY = m_nHeight / m_nSectionHeight;
	m_nSectionCnt = m_nSectionCntX * m_nSectionCntY;

	m_bEdit = bEdit;

	// 读取全部索引
	m_pOffsetIdx = std::make_unique<DWORD[]>(m_nSectionCnt);
	fs.read(reinterpret_cast<char*>(m_pOffsetIdx.get()), sizeof(DWORD) * m_nSectionCnt);

	/*NOTE:
	If file is opened without writing permission, read all data to memory?
	Otherwise we will read data as it is needed?
	*/
	if (!m_bEdit) {
		m_dwMapPos = fs.tellg();

		fs.seekg(0, std::ios_base::end);
		const DWORD dwPos = fs.tellg();
		const DWORD dwMapDataSize = dwPos - m_dwMapPos;
		if (dwMapDataSize > m_dwMapDataSize) {
			m_pMapData = std::make_unique<BYTE[]>(dwMapDataSize);
			m_dwMapDataSize = dwMapDataSize;
		}
		fs.seekg(m_dwMapPos);
		fs.read(reinterpret_cast<char*>(m_pMapData.get()), dwMapDataSize);
	}

	ClearSectionArray();

	return TRUE;
}

void ZRBlock::GetBlockByRange(int CenterX, int CenterY, int range)
{
	if (!fs.is_open())
		return;
	
	m_nGridShowWidth  = range * 2;
	m_nGridShowHeight = range * 2;

	m_fShowCenterX = CenterX;
	m_fShowCenterY = CenterY;

    MPTimer t;
    t.Begin();
    int nCurSectionX = (int)(m_fShowCenterX - (float)range / 2.0f)  / m_nSectionWidth;
	int nCurSectionY = (int)(m_fShowCenterY - (float)range / 2.0f)  / m_nSectionHeight;

    int nEndSectionX = (int)(m_fShowCenterX + (float)range / 2.0f)  / m_nSectionWidth;
	int nEndSectionY = (int)(m_fShowCenterY + (float)range / 2.0f)  / m_nSectionHeight;

	int nShowSectionCntX = nEndSectionX - nCurSectionX;
	int nShowSectionCntY = nEndSectionY - nCurSectionY;
	
	if(range  % m_nSectionWidth!=0)   nShowSectionCntX++;
	if(range  % m_nSectionHeight!=0)  nShowSectionCntY++;
	
    for(int y = 0; y < nShowSectionCntY; y++)
	{
		int nSectionY = nCurSectionY + y;

		if(nSectionY < 0 || nSectionY >= m_nSectionCntY) continue;
		for(int x = 0; x < nShowSectionCntX; x++)
		{
			int nSectionX = nCurSectionX + x;

			if(nSectionX < 0 || nSectionX >= m_nSectionCntX) continue;
		
			if (!GetBlockSection(nSectionX, nSectionY))
			{
				LoadBlockData(nSectionX, nSectionY);
			}
		}
	}
}

std::unique_ptr<ZRBlockSection>& ZRBlock::GetBlockSection(int nSectionX, int nSectionY)
{
	return m_BlockSectionArray[nSectionX][nSectionY];
}

std::unique_ptr<ZRBlockSection>& ZRBlock::LoadBlockData(int nSectionX, int nSectionY)
{
	auto block = std::make_unique<ZRBlockSection>();
	block->nX = nSectionX;
	block->nY = nSectionY;
	
    _LoadBlockData(*block);
	m_BlockSectionArray[nSectionX][nSectionY] = std::move(block);

	return m_BlockSectionArray[nSectionX][nSectionY];
}


void ZRBlock::_LoadBlockData(ZRBlockSection& block)
{
	int nSectionX = block.nX;
	int nSectionY = block.nY;

	block.dwDataOffset = _ReadSectionDataOffset(nSectionX, nSectionY);
    
    if(block.dwDataOffset==0) return;

	DWORD dwPos = 0;
	if( m_bEdit )
	{
		fs.seekg(block.dwDataOffset, std::ios_base::beg);
	}
	else
	{
		dwPos = block.dwDataOffset - m_dwMapPos;
	}
	
	block.blockData = std::make_unique<ZRBlockData[]>(m_nSectionWidth * m_nSectionHeight);

#ifdef NEW_VERSION
	SNewFileTile tile;
#else
	SFileTile tile;
#endif
	
	for(int y = 0; y < m_nSectionHeight; y++)
	{
		for(int x = 0; x < m_nSectionWidth; x++)
		{
			ZRBlockData* pB = block.blockData.get() + m_nSectionWidth * y + x;
			if( m_bEdit )
			{
				fs.read(reinterpret_cast<char*>(&tile), sizeof(tile));
			}
			else
			{
				memcpy( &tile, m_pMapData.get() + dwPos, sizeof(tile) );
				dwPos += sizeof(tile);
			}
			pB->sRegion  = tile.sRegion;
            memcpy(&pB->btBlock[0], &tile.btBlock[0], 4); 
        }
	}
}


ZRBlockData*	ZRBlock::GetBlock(int nX, int nY)
{
	if(nX >= m_nWidth || nY >= m_nHeight || nX < 0 || nY < 0) 
	{
		return m_pDefaultBlock.get();
	}

	int nSectionX = nX / m_nSectionWidth;
	int nSectionY = nY / m_nSectionHeight;

	auto& pBlock = GetBlockSection(nSectionX, nSectionY);
	 
	if(pBlock && pBlock->blockData)
	{
		int nOffX = nX % m_nSectionWidth;
		int nOffY = nY % m_nSectionHeight;
		return pBlock->blockData.get() + nOffY * m_nSectionWidth + nOffX;
	}

	return m_pDefaultBlock.get();	
}

DWORD ZRBlock::_ReadSectionDataOffset(int nSectionX, int nSectionY)
{
	DWORD dwLoc = (nSectionY * m_nSectionCntX + nSectionX);
	return m_pOffsetIdx[dwLoc];

	fs.seekg(sizeof(MPMapFileHeader) + 4 * dwLoc, std::ios_base::beg);

	DWORD dwDataOffset;
	fs.read(reinterpret_cast<char*>(&dwDataOffset), sizeof(dwDataOffset));
	return dwDataOffset;
}

void ZRBlock::SetGrid(int GridX, int GridY)
{
	m_nLastGridStartX = GridX;
	m_nLastGridStartY = GridY;
}

void ZRBlock::ClearSectionArray()
{
	for (auto& section : m_BlockSectionArray)
	{
		section = {};
	}
}