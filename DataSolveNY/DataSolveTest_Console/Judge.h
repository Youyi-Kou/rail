#ifndef JUDGE_H
#define JUDGE_H

#include "DataEntities.h"

void	GetRegions(uint16_t* datas, int length, int limit, std::vector<PointRegion>& regions);

void	SetFileHead(F_HEAD& fhead);

void	FillMarksWounds(VBDB& blocks, VPM& vPMs, VWJ& vWounds, VPM& vPMs2, VLCP& vLCP, F_HEAD& head);


//���ظ����ݿ׵�����
uint8_t	ParseSewCH(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs);

uint8_t	ParseSewLRH(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs);

int		GetSewXCHMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall);
uint8_t	ParseSewXCH(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs);


int		GetJointMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall, PM& pm, uint8_t iFRow);
uint8_t	ParseJoint(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int& stepF1, int& stepF2, uint8_t& railType, bool& direction, bool& carType, int16_t& iFRow, double& wd, VINT* t_cr, VWJ& vWounds, VPM& vPMs, PM& pm);


void	ParseHorizonalCrack(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, uint32_t step1, uint32_t step2, bool& direction, bool& carType, int16_t& iFRow, VWJ& vWounds, uint8_t isJoint = 0, uint8_t isSew = 0);

void	ParseHorizonalCrack(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& crFG, bool& direction, bool& carType, int16_t& iFRow, VWJ& vWounds, uint8_t isJoint = 0, uint8_t isSew = 0);

//�������
bool	ParseHS(BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, uint8_t& railType, bool& direction, bool& carType, int16_t& iFRow, double& wd, VWJ& vWounds, VPM& vPMs, uint8_t isJoint = 0, uint8_t isSew = 0, uint8_t isScrewHole = 0, uint8_t isGuideHole = 0);

void	ParseD(double angle, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, int16_t& iFRow, int i, int& offset, double& wd, uint8_t& railType, VWJ& vWounds, VPM& vPMs);

void	ParseE(double angle, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, int16_t& iFRow, int i, int& offset, double& wd, uint8_t& railType, VWJ& vWounds, VPM& vPMs);

void	DealWithSpecialMark(BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, PM& pm, VBA& vBA, VWJ& vWounds, VPM& vPMs, int btIndex = -1);

void	AnalyseCR(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VCR* vCRs, CR& cr, int& i, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, int btIndex = -1);

int16_t	AnalyseStep(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VCR* vCRs, int step, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, int stepLimit, int pmIndex = -1, int btIndex = -1);

void	Analyse(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, VPM& vPMsYOLO, int backFlag = 0, int btIndex = -1);

//�ڶ��η�����ֻ���������һ���ǰ���ж������һ�µ�����
void	Analyse2(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, VPM& vPMsYOLO);
/*
//���½����̸ֲ�����
void	Analyse2(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VWJ& vWounds, VPM& vPMs, int iBeginForkBlock, int iEndForkBlock);
*/

//��1���ų�2���е�
void	Exclude(VINT& cr1, VINT& cr2);


//��ȡA��һ֡�е�ͨ���ķ���������Թ���ϸɳ
uint8_t GetStepPeaks(A_Step& step, uint8_t iChA);

//Ѱ����һ����ͷ
bool	FindNextJoint(int block, int step, VBDB& blocks, int* destBlock, int * destStep);

//Ѱ����һ������
bool	FindNextWeld(int block, int step, VBDB& blocks, int* destBlock, int * destStep);


std::string GetWoundAccordding(Wound_Judged& wd, VBDB& blocks);


#pragma region ��ȡ�����������
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


