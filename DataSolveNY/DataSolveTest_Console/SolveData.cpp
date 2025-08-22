#include "stdafx.h"
#include "SolveData.h"
#include <stdlib.h>
#include <string.h>

#include "GlobalDefine.h"
#include "PublicFunc.h"

//uint16_t			g_wave_F0[WAVE_LEN*CH_N];				// 双端RAM中的回波数据
uint16_t			g_wave_F0[7000];				// 双端RAM中的回波数据
uint16_t			g_Cache_A[CASH_LEN];					// A超读取、写入缓存，M空间
uint16_t			Tempbuffer[20000000];					// A超读取、写入缓存，M空间
static uint16_t		*pTail_A = g_Cache_A + CASH_LEN;

B_POINT	g_BchaoBuf2[(ROW + 1)*COLUMN];
int32_t g_ckB[30000];
int32_t	g_ckA[30000];


uint8_t	g_Cache_B[CASH_SIZE];			// B 超读取、写入入	缓存,1M 空间 是一一个介质,作为第一一次读取的数据,未加压的原始数据
uint8_t	*pTail_B = (g_Cache_B + CASH_SIZE);	//g_Cache_B[CASH_SIZE]数组对应的最后一一个地址

//A超解压函数
uint16_t	RLE16_Decompress_Loop2Buf(uint16_t block, uint16_t frameIndex, uint16_t *pDest, uint16_t *pSrc, uint16_t *pSrcHead, uint16_t *pSrcTail, uint32_t uLength, FILE* pFile)
{
	uint32_t		i, cnt;
	uint32_t		u32Sum = 0;

	uint16_t* pStart = pDest;
	int fillCount = 0;
	int fillCountLimit = 12 * 512;
	for (i = 0; i < uLength; i++)
	{
		if (*pSrc & F_ZERO)
		{
			cnt = *pSrc & ZERO_L;
			fillCount += cnt;
			if (fillCount >= 7000)
			{
				break;
			}
			memset(pDest, 0x0, 2 * cnt);
			pDest += cnt;
		}
		else
		{
			*pDest++ = *pSrc;
			++fillCount;
			if (fillCount >= 7000)
			{
				break;
			}
			u32Sum += *pSrc;
		}
		pSrc++;
		if (pSrc >= pSrcTail)
			pSrc = pSrcHead;
	}

	while (u32Sum >> 16)
		u32Sum = (u32Sum & 0xffff) + (u32Sum >> 16);

	if (u32Sum & 0x8000)		//避免最高2位为11，与压缩标志等冲突方便统一压缩。
		u32Sum = (u32Sum & 0xfff) + (u32Sum >> 12);

	if (pFile != NULL)
	{
		fprintf(pFile, "Block = %d, frame = %d, DECOMPRESS %d/%d\n", block, frameIndex, fillCount, fillCountLimit);
	}
	return u32Sum;
}

uint16_t	GetBlockAFrames(FILE* pFileA, int iFileSize, BlockData_B blockB, BlockData_A& vADatas, uint32_t& iBeginFrame)
{
	if (blockB.AStartPos < 0 || blockB.AStartPos >= iFileSize)
	{
		return 0;
	}
	uint32_t			Num = 0;
	uint32_t			i, k, len;
	uint32_t			errF = 0;
	uint16_t			sum;

	//定位到开始读取文件的位置，肯定是某个米块头
	uint32_t	blockIndex = blockB.Index;
	uint32_t    s_FileL_A = (iFileSize - szFileHead) >> 1;
	int iBlockRead = 0;
	BLOCK_A head;

	fseek(pFileA, (blockB.AStartPos << 1) + szFileHead, SEEK_SET);
	size_t readCount = fread(&head, 1, szAHead, pFileA);
	if (readCount < szAHead || head.symbol != 0xFFFFFFFF)
	{
		return 0;
	}

	readCount = fread(Tempbuffer, 1, head.len << 1, pFileA);
	if (readCount < (head.len << 1))
	{
		return 0;
	}
	head.indexL = blockB.IndexL2;

	k = 0;
	errF = 0;
	int iFrameCount = 0;
	A_Step step;
	UINT16 *buff = (UINT16 *)Tempbuffer;
	//FILE* pFileLog = fopen("D:/log.txt", "a");
	for (i = 0; i < head.fNum; ++i)
	{
		// 检查帧头标志字
		while ((*buff & FLAG_COMPRESS) == 0)
		{
			++buff;
			if (++k >= head.len)	break;
		}

		len = (*buff++) & 0xFFF;
		if (++k >= head.len)
			break;

		A_Step step;
		step.Block = blockIndex;
		step.Index = i;
		step.Index2 = (iBeginFrame++);
		step.Step = head.indexL + (*buff++);
		sum = *buff++;

		//if (sum != RLE16_Decompress_Loop2Buf(g_wave_F0, pData, g_Cache_A, pTail_A, frameLen))
		uint16_t retSum = RLE16_Decompress_Loop2Buf(blockIndex, i, g_wave_F0, buff, buff, &buff[2000000], len, NULL);
		if (sum != retSum)
			errF++;

	/*	if (blockIndex == 981)
		{
			fprintf(pFileLog, "step.Step = %d, retSum = %d, sum = %d, len = %d, errF = %d\n", step.Step, retSum, sum, len, errF);
		}*/

		++iFrameCount;
		A_Frame frame;
		for (int p = 0; p < WAVE_LEN; ++p)
		{
			frame.Horizon = p;
			uint32_t t = 0;
			for (int j = 0; j < CH_N; ++j)
			{
				frame.F[j] = g_wave_F0[j * WAVE_LEN + p];
				t += frame.F[j];
			}

			if (t > 0)
			{
				step.Frames.emplace_back(frame);
			}
		}

		if (step.Frames.size() > 0)
		{
			vADatas.vAStepDatas.emplace_back(step);
		}

		k += len;
		buff += len;
	}
	//fclose(pFileLog);

	if (step.Frames.size() > 0)
	{
		vADatas.vAStepDatas.emplace_back(step);
	}

	return iFrameCount;
}


uint16_t Reverse(uint16_t data)
{
	uint8_t low = data & 0xFF;
	uint8_t high = (data >> 8) & 0xFF;
	return high + (low << 8);
}

//B超解压文件函数
uint32_t	GetBlockBSteps(FILE* pFileB, int iFileSize, BlockData_B blockB, BlockData_B& block)
{
	uint8_t			*pData = NULL;
	uint8_t			*pDest = NULL;
	uint16_t		sum;
	BLOCK			head;

	fseek(pFileB, blockB.BStartPos + szFileHead, SEEK_SET);
	if (fread(&head, 1, szBHead, pFileB) < szBHead)
	{
		return 0;
	}

	for (int i = 0; i < 12; ++i)
	{
		head.gain[i] = 160 - head.gain[i];
	}

	if (blockB.Index == 10227)
	{
		int xx = 1;
		xx += 1;
	}

	block = blockB;
	block.IndexL = head.indexL;
	block.BlockHead = head;
	head.indexL = blockB.IndexL2;

	memset(g_Cache_B, 0, CASH_SIZE);
	size_t count = fread(g_Cache_B, 1, head.len, pFileB);
	if (count < head.len)
	{
		return 0;
	}
	pData = g_Cache_B;
	pTail_B = g_Cache_B + CASH_SIZE;
	sum = RLE64_Decompress_Loop2Loop(blockB.Index, (uint64_t*)g_BchaoBuf2, COLUMN, ROW, 0, head.row, pData, g_Cache_B, pTail_B, head.len);
	if(head.checkD == sum)
	{
		uint8_t isFork = 0;
		for (int q = 0; q < head.row; ++q)
		{
			B_Step step;
			step.Step = head.indexL + q;

			for (int qq = 0; qq < VALID_ROW; ++qq)
			{
				B_RowData data;
				data.Row = qq;
				data.Point = g_BchaoBuf2[qq * COLUMN + q];
				if (data.Point.Draw1 > 0 || data.Point.Alarm > 0 || data.Point.Weight > 0 || data.Point.Draw2 > 0 || data.Point.Wound > 0)
				{
					step.vRowDatas.emplace_back(data);
				}
			}
			step.Mark = *(B_MARK*)&g_BchaoBuf2[VALID_ROW * COLUMN + q];
			step.Wound = *(B_WOUND*)&g_BchaoBuf2[(ROW - 1) * COLUMN + q];
			memset(&step.Wound2, 0, sizeof(B_WOUND));

			step.FRow = -1;
			step.GRow = -1;
			if (step.Mark.Mark & FORK)
			{
				isFork = 1;
			}
			block.vBStepDatas.emplace_back(step);
			if (isFork == 0)
			{
				block.BlockHead.swNum = 0;
				blockB.BlockHead.swNum = 0;
			}
		}
		return 1;
	}
	return 0;
}

//B超解压文件函数
uint32_t	GetBlockBStepsPT(FILE* pFileB, int iFileSize, BlockData_B blockB, BlockData_B& block)
{
	uint8_t			*pData = NULL;
	uint8_t			*pDest = NULL;
	uint16_t		sum;
	BLOCK			head;

	fseek(pFileB, blockB.BStartPos + szFileHead, SEEK_SET);
	if (fread(&head, 1, szBHead, pFileB) < szBHead)
	{
		return 0;
	}

	for (int i = 0; i < 12; ++i)
	{
		head.gain[i] = 160 - head.gain[i];
	}

	block = blockB;
	block.IndexL = head.indexL;
	block.BlockHead = head;
	head.indexL = blockB.IndexL2;

	memset(g_Cache_B, 0, CASH_SIZE);
	size_t count = fread(g_Cache_B, 1, head.len, pFileB);
	if (count < head.len)
	{
		return 0;
	}
	pData = g_Cache_B;
	pTail_B = g_Cache_B + CASH_SIZE;
	sum = RLE64_Decompress_Loop2Loop(blockB.Index, (uint64_t*)g_BchaoBuf2, COLUMN, ROW + 1, 0, head.row, pData, g_Cache_B, pTail_B, head.len);
	if (head.checkD == sum)
	{
		uint8_t isFork = 0;
		for (int q = 0; q < head.row; ++q)
		{
			B_Step step;
			step.Step = head.indexL + q;

			for (int qq = 0; qq < VALID_ROW; ++qq)
			{
				B_RowData data;
				data.Row = qq;
				data.Point = g_BchaoBuf2[qq * COLUMN + q];
				if (data.Point.Draw1 > 0 || data.Point.Alarm > 0 || data.Point.Weight > 0 || data.Point.Draw2 > 0 || data.Point.Wound > 0)
				{
					step.vRowDatas.emplace_back(data);
				}
			}
			step.Mark = *(B_MARK*)&g_BchaoBuf2[VALID_ROW * COLUMN + q];
			step.Wound = *(B_WOUND*)&g_BchaoBuf2[(ROW - 1) * COLUMN + q];
			step.Wound2 = *(B_WOUND*)&g_BchaoBuf2[(ROW) * COLUMN + q];			

			step.FRow = -1;
			step.GRow = -1;
			block.vBStepDatas.emplace_back(step);
		}
		return 1;
	}
	return 0;
}

uint16_t	Check_Sum(uint16_t *p16Buffer, uint16_t uLength)
{
	uint8_t* pData = (uint8_t*)p16Buffer;
	uint32_t uSum = 0;

	while (uLength--)
	{
		uSum += *p16Buffer++;
		if (uSum & 0x80000000)
			uSum = (uSum & 0xffff) + (uSum >> 16);
	}

	while (uSum >> 16)
		uSum = (uSum & 0xffff) + (uSum >> 16);

	if (uSum & 0x8000)
		uSum = (uSum & 0xfff) + (uSum >> 12);

	/*uint32_t sumTemp = 0;
	int temp = 2 * uLength;
	while (temp -- )
	{
		sumTemp += *pData++;
		if (sumTemp & 0x80000000)
			sumTemp = (sumTemp & 0xffff) + (sumTemp >> 16);
	}

	while (sumTemp >> 16)
		sumTemp = (sumTemp & 0xffff) + (sumTemp >> 16);

	if (sumTemp & 0x8000)
		sumTemp = (sumTemp & 0xfff) + (sumTemp >> 12);
	uSum = sumTemp;*/
	return (uint16_t)uSum;
}


//static int64_t	bufRLE[1024 * 12];// 12通道
static int64_t	bufRLE[CASH_SIZE];// 12通道
//B超解压函数
uint16_t	RLE64_Decompress_Loop2Loop(uint16_t block, uint64_t* pDestBuf, uint32_t width, uint32_t height, uint32_t start, uint32_t L, uint8_t *pSorc, uint8_t *pHead, uint8_t *pTail, uint32_t byteLen, FILE* pFile /* = NULL*/)
{
	uint64_t	i, k, N;
	uint64_t	*pDest;
	uint64_t	data;
	uint32_t		H = 0, row = 0;
	uint8_t		*pBuf8;
	uint64_t	*pRLE;
	uint32_t		RLE_n = 0;
	uint32_t		u32Sum = 0;

	int myflag1 = 0;
	int myflag2 = 0;
	// xian
	pBuf8 = (uint8_t*)bufRLE;
	int fillCount = 0;
	int totalBufferSize = width * height * sizeof(B_POINT);
	int totalCompressedSize = 1024 * 12 * 8;
	if (byteLen > totalCompressedSize || L > 1000)
	{
		printf("记录米块尺寸过大\n");
		return 0;
	}

	for (i = 0; i < byteLen; i++)//pSorc存储的是米块头的地址还是米块数据的开始地址 size 此米块数据压缩后多长, 采用bit压缩，所以len的单位为Byte
	{
		if (*pSorc)
		{
			++fillCount;
			if (fillCount > totalBufferSize)
			{
				printf("数据异常：%d\n", block);
				return 0;
			}
			u32Sum += *pSorc;
			*pBuf8++ = *pSorc++;
			if (pSorc >= pTail)
				pSorc = pHead;
		}
		else
		{
			*pSorc++;
			if (pSorc >= pTail)
				pSorc = pHead;

			fillCount += *pSorc;
			if (fillCount > totalBufferSize)
			{
				printf("数据异常：%d\n", block);
				return 0;
			}
			for (k = 0; k < *pSorc; k++)
				*pBuf8++ = 0;

			pSorc++;
			if (pSorc >= pTail)
				pSorc = pHead;
			i++;
		}
	}

	while (u32Sum >> 16)
		u32Sum = (u32Sum & 0xffff) + (u32Sum >> 16);

	if (u32Sum & 0x8000)
		u32Sum = (u32Sum & 0xfff) + (u32Sum >> 12);

	pRLE = (uint64_t *)bufRLE;
	pDest = pDestBuf + start;
	data = *pRLE;
	RLE_n = (pBuf8 - (uint8_t*)bufRLE) >> 3;
	//printf("每个米块对应多少个点%d\n",RLE_n);
	for (i = 0; i < RLE_n; i++)
	{
		if (0 == (*pRLE&F_RLE))//0x8000000000000000  读取当前字节的最高位，如果为1 则表示为压缩的数据，如果不是，则表示未压缩
		{
			//对压缩的数据进行存储。
			data = *pRLE;
			*pDest++ = data;
			row++;
			if (row >= L)//L存的是米块有多少列，340列，读完340列的时候，就转到下1000行进行存储数据
			{
				myflag1++;
				row = 0;
				H++;
				if (H >= height)
				{
					//printf("!!!!!!!!!!!!!出现意外！！！！！！！！！！ %d\n RLE_n %d i %I64d",H,RLE_n,i);
					return (uint16_t)u32Sum;
				}
				pDest = pDestBuf + start + H * width;
			}
			if (pDest >= pDestBuf + (H + 1)*width)
			{
				pDest = pDestBuf + H * width;
			}
		}
		else
		{
			//对压缩数据进行解压操作
			N = (*pRLE & (~F_RLE));
			//printf("循环次数%I64d  data%I64d\n",N,data);
			for (k = 0; k < N; k++)
			{
				*pDest++ = data;
				row++;
				if (row >= L)  //L = head.row记录着行数 ---------------
				{
					myflag2++;
					row = 0;
					H++;
					if (H >= height)//height存的是68,说明要进去多少次 解析的数据已经超过68列，有错误
					{
						//printf("!!!!!!!!!!!!!出现意外！！！！！！！！！！ %d\n RLE_n %d i %I64d",H,RLE_n,i);
						return (uint16_t)u32Sum;
					}
					pDest = pDestBuf + start + H * width;//width是1000
				}
				if (pDest >= pDestBuf + (H + 1)*width)
				{
					pDest = pDestBuf + H * width;
				}
			}
		}

		pRLE++;
	}
	return (uint16_t)u32Sum;
}

uint32_t    SolveTPA_Export(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t filesize, A_Frame4Nodejs* pFrames, uint32_t* frameCount)
{
	*frameCount = 0;
	if (useL < 0)
	{
		return 0;
	}
	FILE* pFileA = _fsopen(strFile, "rb", _SH_DENYWR);
	if (pFileA == NULL)
	{
		return 0;
	}

	uint32_t			Num = 0;
	uint16_t			sum;

	uint32_t    s_FileL_A = (filesize - szFileHead) >> 1;
	BLOCK_A head;
	//定位到开始读取文件的位置，肯定是某个米块头
	fseek(pFileA, (useL << 1) + szFileHead, SEEK_SET);
	size_t readCount = fread(&head, 1, szAHead, pFileA);
	if (readCount < szAHead || head.symbol != 0xFFFFFFFF)
	{
		return 0;
	}

	readCount = fread(Tempbuffer, 1, head.len << 1, pFileA);
	if (readCount < (head.len << 1))
	{
		return 0;
	}
	head.indexL = beginStep;

	uint32_t			i, k = 0, len;
	uint32_t			errF = 0;
	int iFrameCount = 0;
	uint32_t fCount = 0;
	A_Step step;
	uint16_t *buff = (uint16_t *)Tempbuffer;
	A_Frame4Nodejs frame;
	for (i = 0; i < head.fNum; ++i)
	{
		int stepFrameCount = 0;

		// 检查帧头标志字
		while ((*buff & FLAG_COMPRESS) == 0)
		{
			++buff;
			if (++k >= head.len)	break;
		}

		len = (*buff++) & 0xFFF;
		if (++k >= head.len)
			break;

		frame.Step = beginStep + (int16_t)(*buff++);
		frame.FrameIndex = i;

		sum = *buff++;
		uint16_t retSum = RLE16_Decompress_Loop2Buf(blockIndex, i, g_wave_F0, buff, buff, &buff[2000000], len, NULL);
		if (sum != retSum)
			errF++;

		for (int kk = 0; kk < WAVE_LEN; ++kk)
		{
			frame.Horizon = kk;
			uint32_t t = 0;
			for (int j = 0; j < CH_N; ++j)
			{
				frame.F[j] = g_wave_F0[j * WAVE_LEN + kk];
				t += frame.F[j];
			}

			if (t > 0)
			{
				pFrames[fCount] = frame;
				++fCount;
				++stepFrameCount;
			}
		}

		if (stepFrameCount == 0)
		{
			pFrames[fCount] = frame;
			++fCount;
		}

		k += len;
		buff += len;
	}

	if (errF)
		g_ckA[Num] = head.indexL;
	*frameCount = fCount;
	fclose(pFileA);
	return head.fNum;
}

uint32_t    SolveTPB_Export(char* strFile, int useL, int blockIndex, int beginStep, int fileSize, BLOCK* blockHead, B_Step4Nodejs* pSteps, uint32_t* stepCount, B_Row4Nodejs* pRow, uint32_t* rowCount)
{
	*stepCount = 0;
	*rowCount = 0;
	FILE* pFileB = _fsopen(strFile, "rb", _SH_DENYWR);
	if (pFileB == NULL)
	{
		return 0;
	}
	uint8_t			*pData = NULL;
	uint8_t			*pDest = NULL;
	uint32_t		Num = 0;
	uint16_t		sum;
	uint32_t		err = 0;
	int             iBlockRead = 0;

	F_HEAD head;
	fread(&head, szFileHead, 1, pFileB);
	bool isPT = (head.DeviceNum[2] == 'P');
	fseek(pFileB, useL + szFileHead, SEEK_SET);
	if (fread(blockHead, 1, szBHead, pFileB) < szBHead)
	{
		fclose(pFileB);
		return 0;
	}

	if (blockHead->symbol != 0xFFFFFFFF)
	{
		fclose(pFileB);
		return 0;
	}

	for (int i = 0; i < 12; ++i)
	{
		blockHead->gain[i] = 160 - blockHead->gain[i];
	}
	blockHead->indexL = beginStep;

	memset(g_Cache_B, 0, CASH_SIZE);
	size_t count = fread(g_Cache_B, 1, blockHead->len, pFileB);
	if (count < blockHead->len)
	{
		return 0;
	}
	pData = g_Cache_B;
	pTail_B = g_Cache_B + CASH_SIZE;
	if (isPT)
	{
		sum = RLE64_Decompress_Loop2Loop(blockIndex, (uint64_t*)g_BchaoBuf2, COLUMN, ROW + 1, 0, blockHead->row, pData, g_Cache_B, pTail_B, blockHead->len);
	}
	else
	{
		sum = RLE64_Decompress_Loop2Loop(blockIndex, (uint64_t*)g_BchaoBuf2, COLUMN, ROW, 0, blockHead->row, pData, g_Cache_B, pTail_B, blockHead->len);
	}

	if (blockHead->checkD != sum)
	{
		err++;
	}
	else
	{
		uint8_t isFork = 0;
		for (int q = 0; q < blockHead->row; ++q)
		{
			B_Step4Nodejs step;
			step.Step = blockHead->indexL + q;
			step.Mark = *(B_MARK*)&g_BchaoBuf2[(ROW - 2) * COLUMN + q];
			step.Wound = *(B_WOUND*)&g_BchaoBuf2[(ROW - 1) * COLUMN + q];
			pSteps[*stepCount] = step;
			*stepCount = (*stepCount) + 1;

			for (int qq = 0; qq < VALID_ROW; ++qq)
			{
				B_RowData data;
				data.Row = qq;
				data.Point = g_BchaoBuf2[qq * COLUMN + q];
				if (data.Point.Draw1 > 0 || data.Point.Alarm > 0 || data.Point.Weight > 0 || data.Point.Draw2 > 0 || data.Point.Wound > 0)
				{
					B_Row4Nodejs row;
					row.Step = step.Step;
					row.Row = qq;
					row.Point = data.Point;
					pRow[*rowCount] = row;
					*rowCount = (*rowCount) + 1;
				}
			}

			if (step.Mark.Mark & FORK)
			{
				isFork = 1;
			}
			if (isFork == 0)
			{
				blockHead->swNum = 0;
			}
		}
	}

	g_ckB[Num] = blockHead->indexL;
	++iBlockRead;
	++Num;
	fclose(pFileB);

	//blockHead->railNum = BCDToINT8(blockHead->railNum);
	//blockHead->swNum = BCDToINT16(blockHead->swNum);
	//blockHead->user = BCDToINT16(blockHead->user);
	uint8_t railType = blockHead->railType & 0x03;
	for (int i = 0; i < 7; ++i)
	{
		uint16_t isOn = 0x8000 & blockHead->door[2 * i];
		if (isPT == false)
		{
			blockHead->door[2 * i] = GetSmallGatePixel(railType, blockHead->door[2 * i] & 0x7FFF, i);
			blockHead->door[2 * i + 1] = GetSmallGatePixel(railType, blockHead->door[2 * i + 1] & 0x7FFF, i);
		}
		else
		{
			blockHead->door[2 * i] = blockHead->door[2 * i] & 0x7FFF;
			blockHead->door[2 * i + 1] = blockHead->door[2 * i + 1] & 0x7FFF;
		}
		if (isOn)
		{
			blockHead->door[2 * i] |= 0x8000;
		}
	}

	return *stepCount;
}

uint32_t    SolveTPB_ExportPT(char* strFile, int useL, int blockIndex, int beginStep, int fileSize, BLOCK* blockHead, B_Step4NodejsPT* pSteps, uint32_t* stepCount, B_POINT* points, uint32_t* pointCount)
{
	*stepCount = 0;
	FILE* pFileB = _fsopen(strFile, "rb", _SH_DENYWR);
	if (pFileB == NULL)
	{
		return 0;
	}
	uint8_t			*pData = NULL;
	uint8_t			*pDest = NULL;
	uint32_t		Num = 0;
	uint16_t		sum;
	uint32_t		err = 0;
	int             iBlockRead = 0;

	F_HEAD head;
	fread(&head, szFileHead, 1, pFileB);
	bool isPT = (head.DeviceNum[2] == 'P');
	fseek(pFileB, useL + szFileHead, SEEK_SET);
	if (fread(blockHead, 1, szBHead, pFileB) < szBHead)
	{
		fclose(pFileB);
		return 0;
	}

	if (blockHead->symbol != 0xFFFFFFFF)
	{
		fclose(pFileB);
		return 0;
	}

	for (int i = 0; i < 12; ++i)
	{
		blockHead->gain[i] = 160 - blockHead->gain[i];
	}
	blockHead->indexL = beginStep;

	memset(g_Cache_B, 0, CASH_SIZE);
	size_t count = fread(g_Cache_B, 1, blockHead->len, pFileB);
	if (count < blockHead->len)
	{
		return 0;
	}
	pData = g_Cache_B;
	pTail_B = g_Cache_B + CASH_SIZE;
	if (isPT)
	{
		sum = RLE64_Decompress_Loop2Loop(blockIndex, (uint64_t*)g_BchaoBuf2, COLUMN, ROW + 1, 0, blockHead->row, pData, g_Cache_B, pTail_B, blockHead->len);
	}
	else
	{
		sum = RLE64_Decompress_Loop2Loop(blockIndex, (uint64_t*)g_BchaoBuf2, COLUMN, ROW, 0, blockHead->row, pData, g_Cache_B, pTail_B, blockHead->len);
	}

	if (blockHead->checkD != sum)
	{
		err++;
	}
	else
	{
		uint8_t isFork = 0;
		int pointIndex = 0;
		for (int q = 0; q < blockHead->row; ++q)
		{
			B_Step4NodejsPT step;
			step.Step = blockHead->indexL + q;
			step.Mark = *(B_MARK*)&g_BchaoBuf2[(ROW - 2) * COLUMN + q];
			step.Wound = *(B_WOUND*)&g_BchaoBuf2[(ROW - 1) * COLUMN + q];
			step.Wound2 = *(B_WOUND*)&g_BchaoBuf2[(ROW) * COLUMN + q];
			pSteps[*stepCount] = step;
			*stepCount = (*stepCount) + 1;

			for (int qq = 0; qq < VALID_ROW; ++qq)
			{
				points[pointIndex++] = g_BchaoBuf2[qq * COLUMN + q];
			}

			if (step.Mark.Mark & FORK)
			{
				isFork = 1;
			}
			if (isFork == 0)
			{
				blockHead->swNum = 0;
			}
		}
		*pointCount = pointIndex;
	}

	g_ckB[Num] = blockHead->indexL;
	++iBlockRead;
	++Num;
	fclose(pFileB);

	//blockHead->railNum = BCDToINT8(blockHead->railNum);
	//blockHead->swNum = BCDToINT16(blockHead->swNum);
	//blockHead->user = BCDToINT16(blockHead->user);
	uint8_t railType = blockHead->railType & 0x03;
	for (int i = 0; i < 7; ++i)
	{
		uint16_t isOn = 0x8000 & blockHead->door[2 * i];
		if (isPT == false)
		{
			blockHead->door[2 * i] = GetSmallGatePixel(railType, blockHead->door[2 * i] & 0x7FFF, i);
			blockHead->door[2 * i + 1] = GetSmallGatePixel(railType, blockHead->door[2 * i + 1] & 0x7FFF, i);
		}
		else
		{
			blockHead->door[2 * i] = blockHead->door[2 * i] & 0x7FFF;
			blockHead->door[2 * i + 1] = blockHead->door[2 * i + 1] & 0x7FFF;
		}
		if (isOn)
		{
			blockHead->door[2 * i] |= 0x8000;
		}
	}

	return *stepCount;
}

uint32_t    SolveTPB_ExportPT2(char* strFile, int useL, int blockIndex, int beginStep, int fileSize, BLOCK* blockHead, B_Step4NodejsPT* pSteps, uint32_t* stepCount, B_Row4Nodejs* pRow, uint32_t* rowCount)
{
	*stepCount = 0;
	*rowCount = 0;
	FILE* pFileB = _fsopen(strFile, "rb", _SH_DENYWR);
	if (pFileB == NULL)
	{
		return 0;
	}
	uint8_t			*pData = NULL;
	uint8_t			*pDest = NULL;
	uint32_t		Num = 0;
	uint16_t		sum;
	uint32_t		err = 0;
	int             iBlockRead = 0;

	F_HEAD head;
	fread(&head, szFileHead, 1, pFileB);
	bool isPT = (head.DeviceNum[2] == 'P' || head.DeviceNum[0] == 'P');
	fseek(pFileB, useL + szFileHead, SEEK_SET);
	if (fread(blockHead, 1, szBHead, pFileB) < szBHead)
	{
		fclose(pFileB);
		return 0;
	}

	if (blockHead->symbol != 0xFFFFFFFF)
	{
		fclose(pFileB);
		return 0;
	}

	for (int i = 0; i < 12; ++i)
	{
		blockHead->gain[i] = 160 - blockHead->gain[i];
	}
	blockHead->indexL = beginStep;

	memset(g_Cache_B, 0, CASH_SIZE);
	size_t count = fread(g_Cache_B, 1, blockHead->len, pFileB);
	if (count < blockHead->len)
	{
		return 0;
	}
	pData = g_Cache_B;
	pTail_B = g_Cache_B + CASH_SIZE;
	if (isPT)
	{
		sum = RLE64_Decompress_Loop2Loop(blockIndex, (uint64_t*)g_BchaoBuf2, COLUMN, ROW + 1, 0, blockHead->row, pData, g_Cache_B, pTail_B, blockHead->len);
	}
	else
	{
		sum = RLE64_Decompress_Loop2Loop(blockIndex, (uint64_t*)g_BchaoBuf2, COLUMN, ROW, 0, blockHead->row, pData, g_Cache_B, pTail_B, blockHead->len);
	}

	if (blockHead->checkD != sum)
	{
		err++;
	}
	else
	{
		uint8_t isFork = 0;
		for (int q = 0; q < blockHead->row; ++q)
		{
			B_Step4NodejsPT step;
			step.Step = blockHead->indexL + q;
			step.Mark = *(B_MARK*)&g_BchaoBuf2[(ROW - 2) * COLUMN + q];
			step.Wound = *(B_WOUND*)&g_BchaoBuf2[(ROW - 1) * COLUMN + q];
			if (isPT)
			{
				step.Wound2 = *(B_WOUND*)&g_BchaoBuf2[(ROW) * COLUMN + q];
			}
			else
			{
				memset(&step.Wound2, 0, sizeof(B_WOUND));
			}
			
			pSteps[*stepCount] = step;
			*stepCount = (*stepCount) + 1;

			for (int qq = 0; qq < VALID_ROW; ++qq)
			{
				B_RowData data;
				data.Row = qq;
				data.Point = g_BchaoBuf2[qq * COLUMN + q];
				if (data.Point.Draw1 > 0 || data.Point.Alarm > 0 || data.Point.Weight > 0 || data.Point.Draw2 > 0 || data.Point.Wound > 0)
				{
					B_Row4Nodejs row;
					row.Step = step.Step;
					row.Row = qq;
					row.Point = data.Point;
					pRow[*rowCount] = row;
					*rowCount = (*rowCount) + 1;
				}
			}

			if (step.Mark.Mark & FORK)
			{
				isFork = 1;
			}
			if (isFork == 0)
			{
				blockHead->swNum = 0;
			}
		}
	}

	g_ckB[Num] = blockHead->indexL;
	++iBlockRead;
	++Num;
	fclose(pFileB);

	//blockHead->railNum = BCDToINT8(blockHead->railNum);
	//blockHead->swNum = BCDToINT16(blockHead->swNum);
	//blockHead->user = BCDToINT16(blockHead->user);
	uint8_t railType = blockHead->railType & 0x03;
	for (int i = 0; i < 7; ++i)
	{
		uint16_t isOn = 0x8000 & blockHead->door[2 * i];
		if (isPT == false)
		{
			blockHead->door[2 * i] = GetSmallGatePixel(railType, blockHead->door[2 * i] & 0x7FFF, i);
			blockHead->door[2 * i + 1] = GetSmallGatePixel(railType, blockHead->door[2 * i + 1] & 0x7FFF, i);
		}
		else
		{
			blockHead->door[2 * i] = blockHead->door[2 * i] & 0x7FFF;
			blockHead->door[2 * i + 1] = blockHead->door[2 * i + 1] & 0x7FFF;
		}
		if (isOn)
		{
			blockHead->door[2 * i] |= 0x8000;
		}
	}

	return *stepCount;
}

int errorcountsss = 0;
//获取全部B超米块头
uint32_t	GetBBlocks(FILE* pFileB, VBDB& vBlockHeads, int iFileSize)
{
	uint8_t			*pData = NULL;
	uint8_t			*pDest = NULL;
	uint32_t		readN;
	uint32_t		isblock = 0, Num = 0;
	uint16_t		sum;
	BLOCK			head;
	uint32_t		ii;
	uint32_t		err = 0;

	int nFileHeadSize = sizeof(F_HEAD);
	fseek(pFileB, 0, SEEK_SET);
	F_HEAD fhead;
	fread(&fhead, szFileHead, 1, pFileB);
	bool isPT = fhead.DeviceNum[2] == 'P'|| fhead.DeviceNum[0] == 'P';

	uint32_t s_File_SizeB = iFileSize - nFileHeadSize;

	uint32_t		iBeginStep = 0;
	uint32_t		iBeginBlock = 0;
	uint32_t		iBlockRead = 0;
	uint32_t		use_Size = 0, read_Size = 0;
	while (true)
	{
		if (use_Size >= s_File_SizeB)
			break;

		isblock = 0;
		if (read_Size < (use_Size + ACCESS_SIZE) && read_Size < s_File_SizeB)
		{
			fseek(pFileB, read_Size + nFileHeadSize, SEEK_SET);
			readN = fread(g_Cache_B + read_Size % CASH_SIZE, 1, ACCESS_SIZE, pFileB);
			if (readN >= 0)
			{
				read_Size += readN;
				if (read_Size >= s_File_SizeB)
				{
					//fclose(pFileB);
					//pFileB = NULL;
					//break;
				}
				else if (ACCESS_SIZE != readN)
				{
					//err++;
				}
			}
			else
			{
				//fclose(pFileB);
				//pFileB = NULL;
				break;
			}
		}

		if (read_Size >= s_File_SizeB || read_Size >= (use_Size + ACCESS_SIZE))
		{
			pData = g_Cache_B + use_Size % CASH_SIZE;
			while (use_Size < s_File_SizeB)
			{
				if (0xff == *pData)
				{
					if (0xff == g_Cache_B[(use_Size + 1) % CASH_SIZE])
					{
						if (0xff == g_Cache_B[(use_Size + 2) % CASH_SIZE])
						{
							if (0xff == g_Cache_B[(use_Size + 3) % CASH_SIZE])
							{
								isblock = 1;
								break;
							}
						}
					}
				}

				pData++;
				if (pData >= pTail_B)
					pData = g_Cache_B;

				use_Size++;
				if (use_Size + szBHead > read_Size)		// Èç¹ûÎÄŒþÃ»ÓÐ¶ÁÍê£¬Sleep
					break;
			}
		}

		if (!isblock)
		{
			continue;
		}

		pDest = (uint8_t*)&head;
		for (ii = 0; ii < sizeof(BLOCK); ii++)
		{
			*pDest++ = *pData++;
			if (pData >= pTail_B)
				pData = g_Cache_B;
		}
		if (use_Size + szBHead + head.len > read_Size)
		{
			errorcountsss++;
			use_Size += 1;
			continue;
		}

		uint16_t checksum = Check_Sum(&head.checkD, (szBHead - 6) >> 1);

		//fprintf(pFileOutB, "[%4d	%4d    %dKm%dm%dmm]\n", block.Index, head.indexL, head.walk.Km, head.walk.m, head.walk.mm);
		BlockData_B block;
		//fprintf(pFile, "%d Begin\n", iBlockRead);
		//fflush(pFile);
		sum = RLE64_Decompress_Loop2Loop(iBlockRead, (uint64_t*)g_BchaoBuf2, COLUMN, ROW, 0, head.row, pData, g_Cache_B, pTail_B, head.len, NULL);
		//fprintf(pFile, "%d End\n", iBlockRead);
		//fflush(pFile);

		if (isPT == false && head.checkSum != checksum)
		{
			errorcountsss++;
			use_Size += 4;
			continue;
		}

		if (head.checkD != sum)
		{
			err++;
			//mikuai--;
			use_Size += 4;
			continue;
		}
		else
		{
			if (head.checkSum != checksum)
			{
				char szlog[100];
				sprintf(szlog, "米块头校验和不一致！米块：%d，校验和：%d，%d\n", iBeginBlock + iBlockRead, checksum, head.checkSum);
				WriteLog(szlog);
			}

			for (int i = 0; i < 12; ++i)
			{
				head.gain[i] = 160 - head.gain[i];
			}
			block.IndexL = head.indexL;
			block.BlockHead = head;
			block.BlockHead.indexL = iBeginStep;
			block.IndexL2 = iBeginStep;
			block.Index = iBeginBlock + (iBlockRead++);
			block.StepCount = head.row;
			block.BStartPos = use_Size;
			block.BlockHead = head;
			iBeginStep += head.row;
			block.sumAaBbCc = 0;
			block.sumDE = 0;
			block.sumFG = 0;
			vBlockHeads.emplace_back(block);
			use_Size += (head.len + szBHead);
		}
	}
	return iBlockRead;
}

int32_t		FindStep(VBDB vBlockHeads, int step, int iBeginBlock)
{
	uint32_t blockCount = vBlockHeads.size();
	for (int i = iBeginBlock; i < blockCount; ++i)
	{
		if (step == vBlockHeads[i].IndexL)
		{
			return i;
		}
	}
	return -1;
}

uint32_t	GetABlocks(FILE* pFileA, VBDB& vBlockHeads, int iFileSizeA)
{
	int         useL = 0;
	BLOCK_A		head;
	uint32_t	iBlockRead = 0;
	uint32_t    s_FileL_A = (iFileSizeA - szFileHead) >> 1;
	uint32_t    iTotalBlock = vBlockHeads.size();
	for (int i = 0; i < iTotalBlock; ++i)
	{
		vBlockHeads[i].AStartPos = -1;
		vBlockHeads[i].FrameCount = 0;
		vBlockHeads[i].FrameCountRead = 0;
	}
	while (useL < s_FileL_A && iBlockRead < iTotalBlock)
	{
		int32_t tempUsedL = useL;
		fseek(pFileA, (tempUsedL << 1) + szFileHead, SEEK_SET);
		fread(&head, szAHead, 1, pFileA);
		printf("####AStartPos: %d, %d\n", iBlockRead, useL);
		if (head.symbol != 0xFFFFFFFF)
		{
			useL++;
			continue;
		}
		else if (head.indexL != vBlockHeads[iBlockRead].IndexL)
		{
			int blockFind = FindStep(vBlockHeads, head.indexL, iBlockRead);
			if (blockFind >= 0)// 数据有效，中间丢了A超数据
			{
				vBlockHeads[blockFind].AStartPos = tempUsedL;
				tempUsedL += (head.len + (szAHead >> 1));
				iBlockRead = blockFind;
				++iBlockRead;
			}
			else// 数据无效，中间多了A超数据
			{
				tempUsedL++;
			}
			useL = tempUsedL;
			continue;
		}
		else
		{
			uint32_t hL = head.len << 1;
			if (fread(Tempbuffer, 1, head.len << 1, pFileA) < hL)
			{
				return iBlockRead;
			}
			tempUsedL += (head.len + (szAHead >> 1));
			useL = tempUsedL;
			vBlockHeads[iBlockRead].AStartPos = useL - (head.len + (szAHead >> 1));
			vBlockHeads[iBlockRead].FrameCount = head.fNum;
			++iBlockRead;
		}
	}
	return iBlockRead;
}

uint16_t	GetBigGatePixel(uint8_t railType, uint16_t p)
{
	int rangle = 250;
	if (railType == 3)
	{
		rangle = 300;           // 75轨的声程为300，其他为250
	}
	uint16_t data = (uint16_t)(1615.0 * p / (30 * rangle));
	return data;
}

uint16_t	GetSmallGatePixel(uint8_t railType, uint16_t p, int index)
{
	int rangle = 250;
	if (railType == 3)
	{
		rangle = 300;           // 75轨的声程为300，其他为250
	}
	uint16_t data = 0;
	if (index == 4 || index == 5)
	{
		data = (uint16_t)(2950.0 * p / (30 * rangle));
	}
	else
	{
		data = (uint16_t)(1615.0 * p / (30 * rangle));
	}
	return data;
}

uint32_t    SolveTPB_C_Internal(std::string& strFileB, int iFileSize, BlockData_B& blockB, BlockData_B& block, bool isPtData)
{
	FILE* pFileB = _fsopen(strFileB.c_str(), "rb", SH_DENYWR);
	uint32_t blockCount = 0;
	if (isPtData)
	{
		blockCount = GetBlockBStepsPT(pFileB, iFileSize, blockB, block);
	}
	else
	{
		blockCount = GetBlockBSteps(pFileB, iFileSize, blockB, block);
	}
	fclose(pFileB);
	return blockCount;
}