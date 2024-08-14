#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include "DBCCommon.h"

_DBC_BEGIN
#pragma pack(push)
#pragma pack(4)

//------------------------------------------------------------------------------------------------------------------
//common exception define
class excp :public std::runtime_error			//���쳣��
{
public:
	excp(const char* desc) : std::runtime_error(desc) {}
};
//------------------------------------------------------------------------------------------------------------------
class excpMem:public excp				//�ڴ������ͷ��쳣
{
public:
	excpMem(cChar * desc):excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpArr:public excp				//����Խ�������������ش����쳣
{
public:
	excpArr(cChar * desc):excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpSync:public excp				//����ϵͳͬ����������쳣
{
public:
	excpSync(cChar * desc):excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpThrd:public excp				//����ϵͳ�̲߳����쳣
{
public:
	excpThrd(cChar * desc):excp(desc){};
};
class excpSock:public excp				//����ϵͳ�̲߳����쳣
{
public:
	excpSock(cChar * desc):excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpCOM:public excp				//COM�����쳣
{
public:
	excpCOM(cChar * desc):excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpDB:public excp				//���ݿ�����쳣
{
public:
	excpDB(cChar * desc):excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpIniF:public excp				//�ļ������쳣
{
public:
	excpIniF(cChar * desc):excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpFile:public excp				//�ļ������쳣
{
public:
	excpFile(cChar * desc):excp(desc){};
};
//------------------------------------------------------------------------------------------------------------------
class excpXML:public excp				//�ļ������쳣
{
public:
	excpXML(cChar * desc):excp(desc){};
};

#pragma pack(pop)
_DBC_END

//===================================================================================================================================

//From: https://stackoverflow.com/questions/2670816/how-can-i-use-the-compile-time-constant-line-in-a-string
#define STRINGIZE(x) STRINGIZE2(x)
#define STRINGIZE2(x) #x
#define LINE_STRING STRINGIZE(__LINE__)

#define THROW_EXCP(EXCP, DESC) throw EXCP((std::string("File:") + __FILE__ + " Line:" + LINE_STRING + " desc:" + DESC).c_str());