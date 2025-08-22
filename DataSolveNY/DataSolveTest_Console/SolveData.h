#ifndef SOLVEDATA_H
#define SOLVEDATA_H

#include "DataEntities.h"
#include "DataEntitiesForNode.h"
/*******************************************************/

//函数声明

//校验和函数
uint16_t 	Check_Sum(uint16_t *p16Buffer, uint16_t uLength);

//B超解压函数
uint16_t	RLE64_Decompress_Loop2Loop(uint16_t block, uint64_t *pDestBuf, uint32_t width, uint32_t height, uint32_t start, uint32_t L, uint8_t *pSorc, uint8_t *pHead, uint8_t *pTail, uint32_t byteLen, FILE* pFile = NULL);

//A超解压函数
uint16_t 	RLE16_Decompress_Loop2Buf(uint16_t block, uint16_t frameIndex, uint16_t *pDest, uint16_t *pSrc, uint16_t *pSrcHead, uint16_t *pSrcTail, uint32_t uLength, FILE* pFile = NULL);


uint16_t	GetBlockAFrames(FILE* pFileA, int iFileSize, BlockData_B blockB, BlockData_A& vADatas, uint32_t& iBeginFrame);

//B超解压文件函数 68
uint32_t	GetBlockBSteps(FILE* pFileB, int iFileSize, BlockData_B blockB, BlockData_B& block);

//B超解压文件函数 69
uint32_t	GetBlockBStepsPT(FILE* pFileB, int iFileSize, BlockData_B blockB, BlockData_B& block);


uint32_t    SolveTPA_Export(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t filesize, A_Frame4Nodejs* pFrames, uint32_t* frameCount);

uint32_t    SolveTPB_Export(char* strFile, int useL, int blockIndex, int beginStep, int fileSize, BLOCK* blockHead, B_Step4Nodejs* pSteps, uint32_t* stepCount, B_Row4Nodejs* pRow, uint32_t* rowCount);

uint32_t    SolveTPB_ExportPT(char* strFile, int useL, int blockIndex, int beginStep, int fileSize, BLOCK* blockHead, B_Step4NodejsPT* pSteps, uint32_t* stepCount, B_POINT* points, uint32_t* pointCount);

uint32_t    SolveTPB_ExportPT2(char* strFile, int useL, int blockIndex, int beginStep, int fileSize, BLOCK* blockHead, B_Step4NodejsPT* pSteps, uint32_t* stepCount, B_Row4Nodejs* pRow, uint32_t* rowCount);


uint32_t	GetBBlocks(FILE* pFileB, VBDB& vBlockHeads, int iFileSizeB);

uint32_t	GetABlocks(FILE* pFileA, VBDB& vBlockHeads, int iFileSizeA);

//大小门信息转换
uint16_t	GetBigGatePixel(uint8_t railType, uint16_t p);

uint16_t	GetSmallGatePixel(uint8_t railType, uint16_t p, int index);


uint32_t    SolveTPB_C_Internal(std::string& strFileB, int iFileSize, BlockData_B& blockB, BlockData_B& block, bool isPtData = false);

#endif // ifnef
