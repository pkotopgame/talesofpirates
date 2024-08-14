#include <TableData.h>

BOOL CRawDataSet::_LoadRawDataInfo_Bin(const char* pszFileName)
{

	const unsigned char cluTableKey[] = { 0x32, 0x72, 0x35, 0x75, 0x38, 0x78, 0x2f, 0x41, 0x3f, 0x44, 0x28, 0x47, 0x2b, 0x4b, 0x62, 0x50 };
	const unsigned char cluTableIV[] =	{ 0x43, 0x2a, 0x46, 0x29, 0x4a, 0x40, 0x4e, 0x63, 0x52, 0x66, 0x55, 0x6a, 0x58, 0x6e, 0x32, 0x72 };

	FILE* fp = fopen(pszFileName, "rb");
	char szMsg[MAX_PATH] = { 0 };

	if (fp == NULL)
	{
		LG2("error", "Load Raw Data Info Bin File [%s] Failed!\n", pszFileName);
		//sprintf(szMsg, "�򿪱���ļ�ʧ�ܣ�%s\n���򼴽��˳�!\n", pszFileName);
		//MessageBox(NULL, szMsg, "����", MB_OK | MB_ICONERROR);
		sprintf(szMsg, "Open table file failed:%s\nProgram will exit!\n", pszFileName);
		MessageBox(NULL, szMsg, "Error", MB_OK | MB_ICONERROR);
		return FALSE;
	}

	int nSize = Util_GetFileSize(fp);
	int nInfoSize = _GetRawDataInfoSize();
	

	LPBYTE pbtResInfo = new BYTE[nSize];

	fread(pbtResInfo, sizeof(char), nSize, fp);
	//DWORD dwInfoSize = 0; 
	//fread(&dwInfoSize, 4, 1, fp);
	/*
	if(dwInfoSize!=_GetRawDataInfoSize())
	{
		//sprintf(szMsg, "dwInfoSize: %d\n_GetRawDataInfoSize: %d!\n", dwInfoSize, _GetRawDataInfoSize());
		//MessageBox(NULL, szMsg, "Error2", MB_OK | MB_ICONERROR);
		//LG2("table", "msg��ȡ����ļ�[%s]ʱ, ���ְ汾��һ��!\n", pszFileName);
		LG2("table", "msg read table file [%s], version can't match!\n", pszFileName);
		fclose(fp);
		//sprintf(szMsg, "��ȡ����ļ�����%s\n���򼴽��˳�!\n", pszFileName);
		//MessageBox(NULL, szMsg, "����", MB_OK | MB_ICONERROR);
		sprintf(szMsg, "Open table file failed:%s\nProgram will exit!\n", pszFileName);
		MessageBox(NULL, szMsg, "Error", MB_OK | MB_ICONERROR);
		exit(0);
		return FALSE;
	}
	*/
	
	std::string sink;
	CryptoPP::GCM<CryptoPP::AES>::Decryption d;
	d.SetKeyWithIV(cluTableKey, 16, cluTableIV, 16);

	
	CryptoPP::AuthenticatedDecryptionFilter df(d, new CryptoPP::StringSink(sink), CryptoPP::AuthenticatedDecryptionFilter::DEFAULT_FLAGS, 12);
	CryptoPP::StringSource ss(pbtResInfo, nSize, true, new CryptoPP::Redirector(df));

	//if (sink.size() != nSize - 12) {
	//	sprintf(szMsg, "Size:%d\n, expected:%d\n", sink.size(), (nInfoSize * _nIDCnt) + 12);
	//	MessageBox(NULL, szMsg, "Error", MB_OK | MB_ICONERROR);
	//}
	memset(pbtResInfo, 0, nSize);
	memcpy(pbtResInfo, sink.c_str(), sink.size());

	int nResCnt = sink.size() / nInfoSize;
	for (int i = 0; i < nResCnt; i++)
	{
		CRawDataInfo* pInfo = (CRawDataInfo*)(pbtResInfo + i * _GetRawDataInfoSize());
		// modify by lark.li 20080424 begin
		//strcpy(pInfo->szDataName, ConvertResString(pInfo->szDataName));
		// End

		if (pInfo->bExist != 1) continue;
		if (IsValidID(i) == FALSE) continue;

		CRawDataInfo* pCurInfo = _GetRawDataInfo(pInfo->nID);
		memcpy(pCurInfo, pInfo, nInfoSize); // ���ԭ�е���Ϣ
		_IDIdx[pCurInfo->szDataName] = pCurInfo;
		//vector<string> ParamList; _ReadRawDataInfo(pCurInfo, ParamList);
		_ProcessRawDataInfo(pCurInfo);
		LG2("debug", "Load Bin RawData [%s] = %d\n", pCurInfo->szDataName, pCurInfo->nID);
	}

	delete pbtResInfo;

	fclose(fp);
	return TRUE;
}

void CRawDataSet::_WriteRawDataInfo_Bin(const char* pszFileName)
{

	const unsigned char cluTableKey[] = { 0x32, 0x72, 0x35, 0x75, 0x38, 0x78, 0x2f, 0x41, 0x3f, 0x44, 0x28, 0x47, 0x2b, 0x4b, 0x62, 0x50 };
	const unsigned char cluTableIV[] =	{ 0x43, 0x2a, 0x46, 0x29, 0x4a, 0x40, 0x4e, 0x63, 0x52, 0x66, 0x55, 0x6a, 0x58, 0x6e, 0x32, 0x72 };
	FILE* fp = fopen(pszFileName, "wb");
	if (fp == NULL) return;
	char szMsg[MAX_PATH] = { 0 };
	DWORD dwInfoSize = _GetRawDataInfoSize();

	//fwrite(&dwInfoSize, 4, 1, fp);
	auto buffer = std::make_unique<BYTE[]>(dwInfoSize * _nIDCnt);
	for (int i = 0; i < _nIDCnt; i++) {
		CRawDataInfo* pInfo = (CRawDataInfo*)((LPBYTE)_RawDataArray + i * _GetRawDataInfoSize());
		if (pInfo->bExist)
		{
			memcpy(buffer.get() + (i * dwInfoSize), pInfo, dwInfoSize);
		}

	}

	CryptoPP::GCM<CryptoPP::AES>::Encryption e;
	std::string sink;
	std::string base64;
	e.SetKeyWithIV(cluTableKey, 16, cluTableIV, 16);
	CryptoPP::StringSource ss(buffer.get(), (size_t)(dwInfoSize * _nIDCnt), true, new CryptoPP::AuthenticatedEncryptionFilter(e, new CryptoPP::StringSink(sink), false, 12));
	//CryptoPP::StringSource(sink, true, new CryptoPP::Base64Encoder(new CryptoPP::StringSink(base64), false));
	fwrite(sink.c_str(), sizeof(char), (dwInfoSize*_nIDCnt) + 12, fp);

	//if (sink.size() != (dwInfoSize * _nIDCnt) + 12) {
	//	sprintf(szMsg, "Size:%d\n, expected:%d\n", sink.size(), (dwInfoSize * _nIDCnt) + 12);
	//	MessageBox(NULL, szMsg, "Error", MB_OK | MB_ICONERROR);
	//}
	fclose(fp);
}