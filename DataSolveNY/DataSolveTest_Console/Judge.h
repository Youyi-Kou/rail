#ifndef JUDGE_H
#define JUDGE_H

#include "DataEntities.h"

void	GetRegions(uint16_t* datas, int length, int limit, std::vector<PointRegion>& regions);

void	SetFileHead(F_HEAD& fhead);

void	FillMarksWounds(VBDB& blocks, VPM& vPMs, VWJ& vWounds, VPM& vPMs2, VLCP& vLCP, F_HEAD& head);


//返回附近螺孔的数量
uint8_t	ParseSewCH(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs);

uint8_t	ParseSewLRH(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs);

int		GetSewXCHMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall);
uint8_t	ParseSewXCH(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs);


int		GetJointMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall, PM& pm, uint8_t iFRow);
uint8_t	ParseJoint(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int& stepF1, int& stepF2, uint8_t& railType, bool& direction, bool& carType, int16_t& iFRow, double& wd, VINT* t_cr, VWJ& vWounds, VPM& vPMs, PM& pm);


void	ParseHorizonalCrack(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, uint32_t step1, uint32_t step2, bool& direction, bool& carType, int16_t& iFRow, VWJ& vWounds, uint8_t isJoint = 0, uint8_t isSew = 0);

void	ParseHorizonalCrack(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& crFG, bool& direction, bool& carType, int16_t& iFRow, VWJ& vWounds, uint8_t isJoint = 0, uint8_t isSew = 0);

//处理核伤
bool	ParseHS(BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, uint8_t& railType, bool& direction, bool& carType, int16_t& iFRow, double& wd, VWJ& vWounds, VPM& vPMs, uint8_t isJoint = 0, uint8_t isSew = 0, uint8_t isScrewHole = 0, uint8_t isGuideHole = 0);

void	ParseD(double angle, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, int16_t& iFRow, int i, int& offset, double& wd, uint8_t& railType, VWJ& vWounds, VPM& vPMs);

void	ParseE(double angle, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, int16_t& iFRow, int i, int& offset, double& wd, uint8_t& railType, VWJ& vWounds, VPM& vPMs);

void	DealWithSpecialMark(BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, PM& pm, VBA& vBA, VWJ& vWounds, VPM& vPMs, int btIndex = -1);

void	AnalyseCR(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VCR* vCRs, CR& cr, int& i, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, int btIndex = -1);

int16_t	AnalyseStep(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VCR* vCRs, int step, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, int stepLimit, int pmIndex = -1, int btIndex = -1);

void	Analyse(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, VPM& vPMsYOLO, int backFlag = 0, int btIndex = -1);

//第二次分析：只分析回退且回退前后判定结果不一致的问题
void	Analyse2(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, VPM& vPMsYOLO);
/*
//重新解析锰钢岔区域
void	Analyse2(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VWJ& vWounds, VPM& vPMs, int iBeginForkBlock, int iEndForkBlock);
*/

//从1中排除2中有的
void	Exclude(VINT& cr1, VINT& cr2);


//获取A超一帧中单通道的峰个数，用以过滤细沙
uint8_t GetStepPeaks(A_Step& step, uint8_t iChA);

//寻找下一个接头
bool	FindNextJoint(int block, int step, VBDB& blocks, int* destBlock, int * destStep);

//寻找下一个焊缝
bool	FindNextWeld(int block, int step, VBDB& blocks, int* destBlock, int * destStep);


std::string GetWoundAccordding(Wound_Judged& wd, VBDB& blocks);


#pragma region 获取伤损矩形区域
void FoldCR(CR& cr, uint8_t iJawRow);

void FoldLCR(LCR& cr, uint8_t iJawRow);

uint8_t	GetWoundRect2(Wound_Judged& wound, uint8_t iJawRow, TRECT* rect, uint8_t isFold  = 0 );

#pragma endregion



void FilterByFork(VWJ& vWounds, VPM& vPMs, VBDB& blocks, FILE* pFileB, uint32_t szB);

void FilterByFork2(VWJ& vWounds, VPM& vPMs, VPM& vPMs2, VBDB& blocks);

void FilterByHole(VWJ& vWounds, VPM& vPM, VBDB& blocks, VBA& vBA);

void FilterByVertical(VWJ& vWounds, VPM& vPM, VBDB& blocks);

void FilterByDoubleHole(VWJ& vWounds, VPM& vPM, VBDB& blocks);
#endif


