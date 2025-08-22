// DataSolveTest_Console.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#define _CRT_SECURE_NO_WARNINGS
#include "DataSolveLib.h"
#include <windows.h>



int main(int argc, TCHAR* argv[])
{
	uint8_t ret = 0;   //设置参数ret 函数返回参数：开始解析成功or失败 
	char pFilePath[] = "D:/00-钢轨探伤/20230111融合判伤paper/数据处理/210511Q0823DLN_0000_PT66666.ptB";   //pFilePath 要解析的tpB文件路径
	FILE *pf = fopen(pFilePath, "rb");

	FILE* pflog = NULL;
	errno_t err = fopen_s(&pflog, "./Log.txt", "r");

	char dataaaaa[5000] = { 0 };
	fread(dataaaaa, 1, 5000, pf);
	//printf("%s\n", pFilePath);

	char workdir[] = "D:/00-钢轨探伤/Temp";
	AlgInit(workdir, (int)strlen(workdir));   //算法初始化（输入：工作路径，路径字节数）

	File4Nodejs node;  // 定义结构体类型 File4Nodejs 的变量 node
	ret = SetFile(pFilePath, (int)strlen(pFilePath), &node);   //设置要解析的文件（输入：待解析的tpB文件的完整路径，tpB文件的完整路径字节数，输出：参数 pInfo文件头信息）
	printf("SetFile Result: %d\n", ret);   //返回值ret 1表示成功，0表示失败


	int channelOffset[CH_N] = { 0 };    //CH_N A超通道数 12（将12个通道拼图参数均初始化为0）
	SetChannelOffset(channelOffset, CH_N);   //设置拼图参数（输入：各通道拼图参数，通道个数）


	ret = BeginAnalyse(0, nullptr, 0);   //开始解析SetFile指定的文件（输入：开始解析米块索引，历史周期tpB信息，历史周期个数）
	printf("BeginAnalyse Result: %d\n", ret);   //返回值ret 1：成功，0：失败

	uint32_t useL = 0, woundCount = 0, currentBlock = 0;
	uint8_t isFinish = 0;
	while (isFinish == 0)
	{
		GetAnalyseProcess(&useL, &currentBlock, &woundCount, &isFinish);   //查询当前解析进度（输出：当前使用了的B超字节数，当前解析的米块数，当前检出伤损数量，整个文件是否解析完成）
		Sleep(1);
	}
	printf("Analyse %s\n", "Finish");


	double walk1, walk2;
	GetWalk(&walk1, &walk2);   //获取当前文件作业里程（输出：作业区间开始里程，作业区间结束里程）
	printf("walk1 = %lf\n", walk1);
	printf("walk2 = %lf\n", walk2);


	FileSolveInfo fileSolveInfo;   //定义结构体类型 FileSolveInfo 的变量 fileSolveInfo 文件解析结果信息
	GetSolveInfoItemCount(&fileSolveInfo);   //获取当前文件解析结果数

	printf("WoundCount：%d\n", fileSolveInfo.WoundCount);   //伤损
	printf("pmCount：%d\n", fileSolveInfo.MarkCount);   //位置标
	printf("BlockCount：%d\n", fileSolveInfo.BlockCount);   //米块
	printf("BacCount：%d\n", fileSolveInfo.BackCount);   //回退

	fileSolveInfo.Backs = new	BackAction[fileSolveInfo.BackCount];   //结构体类型 BackAction 回退数据
	fileSolveInfo.Blocks = new	BLOCK_B4Nodejs[fileSolveInfo.BlockCount];   //结构体类型 BLOCK_B4Nodejs 米块头
	fileSolveInfo.Marks = new	Position_Mark_RAW[fileSolveInfo.MarkCount];   //结构体类型 Position_Mark_RAW 位置标数据   ？？Position_Mark、Position_Mark_RAW区别
	fileSolveInfo.Wounds = new	Wound4Nodejs[fileSolveInfo.WoundCount];   //结构体类型 Wound4Nodejs 伤损数据
	fileSolveInfo.QualityInfo.ErroeRects = new	ErroeRect[fileSolveInfo.QualityInfo.ErroeRectCount];   //QualityInfo数据质量信息  ErroeRectCount异常出波区域数量
	fileSolveInfo.QualityInfo.LoseCouples = new	LoseCouple[fileSolveInfo.QualityInfo.LoseCoupleCount];   //LoseCoupleCount失耦数量
	fileSolveInfo.QualityInfo.ManualMarKs = new	Position_Mark_RAW[fileSolveInfo.QualityInfo.ManualMarkCount];   //ManualMarkCount人工标记数量
	fileSolveInfo.QualityInfo.LoseDetects = new	LoseDetect[fileSolveInfo.QualityInfo.LoseDetectCount];   //LoseDetectCount跳探数量
	GetSolveInfo(&fileSolveInfo);   //获取当前文件解析结果数



	//正常画图
	int blockIndex = 0;
	uint32_t fCount = 0, ret2 = 0;
	A_Frame4Nodejs *pFrames = new A_Frame4Nodejs[200000];   //A_Frame4Nodejs A超每帧结构
	ret2 = SolveTPA(node.NewFileName, fileSolveInfo.Blocks[blockIndex].AStartPos, fileSolveInfo.Blocks[blockIndex].Index, fileSolveInfo.Blocks[blockIndex].IndexL2, node.sizeA, pFrames, &fCount);   
	//获取1米A超数据（输入：文件完整路径，A超该米块在文件中的字节位置，当前米块索引，该米块起始步进，tpA文件字节数，输出：A超帧，帧数）
	for (int i = 0; i < fCount; ++i)
	{
		printf("Step = %d, Horizon = %d\n", pFrames[i].Step, pFrames[i].Horizon);   //输出A_Frame4Nodejs.Step，A_Frame4Nodejs.Horizon
	}
	delete pFrames;

	B_Step4Nodejs* step = new B_Step4Nodejs[1000];  //B_Step4Nodejs B超每步进数据
	B_Row4Nodejs* row = new B_Row4Nodejs[66000];  //B_Row4Nodejs B超每步进每行出波数据
	uint32_t stepCount, rowCount;
	BLOCK blockHead;   //米块头
	printf("*******************%d\n", 2);
	ret2 = SolveTPB(node.NewFileName, fileSolveInfo.Blocks[blockIndex].BStartPos, blockIndex, fileSolveInfo.Blocks[blockIndex].IndexL2, node.sizeB, &blockHead, step, &stepCount, row, &rowCount);
	//获取1米B超数据（输入参数：文件完整路径，B超该米块在文件中的字节位置，当前米块索引，该米块起始步进，tpB文件字节数，输出参数：B超米块头，B超步进数据，步进数，B超行数据，B超行数）
	printf("StepCount = %d, rowCount = %d\n", stepCount, rowCount);  //输出步进数、B超行数

	for (int i = 0; i < rowCount; ++i)
	{
		printf("Step = %d, Row = %d, Draw = %d\n", row[i].Step, row[i].Row, row[i].Point.Draw1);  //步进，行，正常画图
	}

	delete step; step = nullptr;
	delete row; row = nullptr;

	delete fileSolveInfo.Backs; fileSolveInfo.Backs = nullptr;
	delete fileSolveInfo.Blocks; fileSolveInfo.Blocks = nullptr;
	delete fileSolveInfo.Marks; fileSolveInfo.Marks = nullptr;
	delete fileSolveInfo.Wounds; fileSolveInfo.Wounds = nullptr;
	delete fileSolveInfo.Backs;
	delete fileSolveInfo.Backs;
	delete fileSolveInfo.Backs;
	delete fileSolveInfo.Backs;

	return 0;

	fclose(pf);

}