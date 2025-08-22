// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <DbgHelp.h>
#pragma comment(lib, "dbghelp.lib")

#include <vector>

//#define OUTPUT_JOINT

//#define OUTPUT_FORK

#define OUTPUT_EX

#define OPENCV

#define GPU

//#define YOLOV4

//#define NET_SUPPORT

#define DQTESTOUTPUT

//#define EX_OUTPUT

//#define DQ_COMBINE


// TODO: reference additional headers your program requires here
extern std::string g_strTPB;//ԭtpB����·��
extern std::string g_strTPA;//ԭtpA����·��
extern std::string g_strFileName;//ԭ�����ļ���������׺
extern std::string g_strFileNameWithoutExt;//ԭ�����ļ�����������׺
extern std::string g_strFolder;//ԭ�����ļ���
extern char        g_splitter;
extern FILE*       g_pFileA;
extern FILE*       g_pFileB;
extern uint32_t    g_szFileA;
extern uint32_t    g_szFileB;
extern uint8_t     g_isFileOpened;

extern std::string g_strNewFileName;
 

extern uint32_t    g_iCurrentBlock;
extern uint8_t     g_isRunning;//
extern uint8_t     g_isFinish;

extern std::string	g_strWavePointFile;

extern std::string g_strDataPath;
extern std::string	g_strWorkPath;
extern std::string g_strLogPath;

extern uint8_t     g_isInitialized;//encrypt


std::string GetCurrentTimeString();
