// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the DATASOLVELIB_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// DATASOLVELIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#include "DataEntities.h"
#include "DataEntitiesForNode.h"

#ifdef DATASOLVELIB_EXPORTS
#define DATASOLVELIB_API extern __declspec(dllexport) 
#else
#define DATASOLVELIB_API extern __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
	DATASOLVELIB_API void        GetAlgName(char* algName);

	DATASOLVELIB_API void        GetAlgVersion(char* algVersion);

	DATASOLVELIB_API void        GetAlgDescription(char* algDesc);


	DATASOLVELIB_API void        AlgInit(char* strWorkPath, int len);
	
	DATASOLVELIB_API uint8_t     GetFileInfo(char* strTPB, int len, File4Nodejs* pInfo);

	DATASOLVELIB_API int32_t     SetFile(char* strTPB, int32_t len, File4Nodejs* pInfo);

	DATASOLVELIB_API void		 SetStandardGain(double * pGain, int count);

	DATASOLVELIB_API void		 SetChannelOffset(int* offset, int count);

	DATASOLVELIB_API void		 SetChannelOffset2(ChannelOffset* pOffset, int count);

	DATASOLVELIB_API uint32_t    GetBlockCount();

	DATASOLVELIB_API uint8_t     BeginAnalyse(int beginSolveIndex = 0, FileSolveInfo* PrevCycleFiles = nullptr, int PrevCycleCount = 0);

	DATASOLVELIB_API void        GetAnalyseProcess(uint32_t* currentUseL, uint32_t* currentCount, uint32_t* woundCount, uint8_t* isFinish);

	DATASOLVELIB_API void        StopAnalyse();

	DATASOLVELIB_API void        ShutDown();

	DATASOLVELIB_API void        GetWalk(double* walk1, double* walk2);
	
	DATASOLVELIB_API void		 GetSolveInfoItemCount(FileSolveInfo* info);

	DATASOLVELIB_API void		 GetSolveInfo(FileSolveInfo* info);

	DATASOLVELIB_API void		 GetRealTimeAlgSolveDetail(int * woundTypes, int * woundCounts, int * woundTypeCount);


	DATASOLVELIB_API void	     GetResultCount(uint32_t* woundCount, uint32_t* backCount, uint32_t* markCount, uint32_t* blockCount);
	
	DATASOLVELIB_API void		 GetResult2(FileBlock4Nodejs* pFiles, uint32_t fileCount, Position_Mark_RAW* pMarksPrev, uint32_t markCountPrev, Wound4Nodejs* pWoundPrev, uint32_t wCountPrev, BackAction* pBackActionsPrev, uint32_t backCountPrev, Position_Mark_RAW* pMarks, uint32_t*markCount, Wound4Nodejs* pWounds, uint32_t* woundCount, BackAction* pBackActions, uint32_t* backCount, BLOCK_B4Nodejs* pBlocks, uint32_t*blockCount);
	
	DATASOLVELIB_API void		 GetDQCount(uint32_t* manualMarkCount, uint32_t* loseCoupleCount, uint32_t* erCount, uint32_t* ldCount);

	DATASOLVELIB_API void		 GetDQ(Position_Mark_RAW* pMarks, LoseCouple* pLC, ErroeRect* pER, LoseDetect* pLD);

	DATASOLVELIB_API void		 GetFileAttr(uint8_t& isTestEqu, int& sztWoundCount, int& sztAlarmCount);

	DATASOLVELIB_API uint8_t	 GetWoundRect(int index, uint8_t iJawRow, TRECT* rect, uint8_t isFold = 0);

	DATASOLVELIB_API int		 GetBlocks(BLOCK_B4Nodejs* pBlocks, int blockCount, char* tpbLocalPath);

	DATASOLVELIB_API uint32_t    SolveTPA(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t filesize, A_Frame4Nodejs* pFrames, uint32_t* frameCount);

	DATASOLVELIB_API uint32_t    SolveTPB(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t fileSize, BLOCK* blockHead, B_Step4Nodejs* steps, uint32_t* stepCount, B_Row4Nodejs* pRow, uint32_t* rowCount);

	DATASOLVELIB_API uint32_t    SolveTPB2(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t fileSize, BLOCK* blockHead, B_Step4NodejsPT* steps, uint32_t* stepCount, B_POINT* pPoints, uint32_t* pointCount);

	DATASOLVELIB_API uint32_t    SolveTPB3(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t fileSize, BLOCK* blockHead, B_Step4NodejsPT* steps, uint32_t* stepCount, B_Row4Nodejs* pRow, uint32_t* rowCount);

	DATASOLVELIB_API uint32_t    SolveTPA_C(std::string& strFileA, int iFileSize, BlockData_B& blockB, BlockData_A& vADatas, uint32_t& iBeginFrame);

	DATASOLVELIB_API uint32_t    SolveTPB_C(std::string& strFileB, int iFileSize, BlockData_B& blockB, BlockData_B& block);

	DATASOLVELIB_API void		 GetResult_C(VWJ& vWounds, VPM& vPMs, VBA& vBA, VPM& vPMs2, VBDB& vBlocks, VLC& vLC, VER& vER, VLD& vLD);

	DATASOLVELIB_API void		 GetResult_CPP(SW_FileInfo& fileInfo);

	DATASOLVELIB_API int		 SetYoloFiles(char* cfgFile, char* namesFile, char* weightsFile, float threshold);

	DATASOLVELIB_API int		 YOLOTest();


	DATASOLVELIB_API void        SetDeleteTempFile(uint8_t _isDeleteFile);

	DATASOLVELIB_API void        SetSaveRawIamge(uint8_t _isSaveRawImage);





#ifdef __cplusplus
}
#endif // __cplusplus



