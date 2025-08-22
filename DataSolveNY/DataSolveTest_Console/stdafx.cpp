
// stdafx.cpp : source file that includes just the standard includes
// DataSolveLib.pch will be the pre-compiled header
// stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"
#include <time.h>
#include "StackWalker.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file

std::string g_strTPB;//原tpB完整路径
std::string g_strTPA;//原tpA完整路径
std::string g_strFileName;//原数据文件名，带后缀
std::string g_strFileNameWithoutExt;//原数据文件名，不带后缀
std::string g_strFolder;//原数据文件夹
char        g_splitter;
FILE*       g_pFileA = NULL;
FILE*       g_pFileB = NULL;
uint32_t    g_szFileA;
uint32_t    g_szFileB;
uint8_t     g_isFileOpened = 0; 
std::string g_strNewFileName;


uint32_t    g_iCurrentBlock = 0;
uint8_t     g_isRunning = 0;//
uint8_t     g_isFinish = 0;

std::string	g_strWavePointFile;

std::string g_strDataPath;
std::string	g_strWorkPath;
std::string g_strLogPath;

uint8_t     g_isInitialized = 0;//encrypt


std::string GetCurrentTimeString()
{
	time_t currentTime;
	currentTime = time(NULL);
	tm* tm = localtime(&currentTime);
	char szTime[20] = "";
	sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d",
		tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
	return szTime;
}
