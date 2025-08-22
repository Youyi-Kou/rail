#pragma once

#include "DataEntities.h"

/*
blocks: �׿�
t_cr : ��ͨ����������
rowCH1��rowCH2�����λ��
iAStepBig��iAStepSmall����Сͨ������ƽ������
iAStepBig2��iAStepSmall2����Сͨ������ƽ������(/ ��ͨ���е���� >= 5)
iAStepBigNoCc��iAStepSmallNoCc����Сͨ������ƽ������(����Ccͨ��)
sumAaBb, sumCc, sum_ABC, sum_abc��ͨ����������
pointCountlInJaw��AaBb��8ͨ������������
maxSize�������ͨ�����
maxSizeNoCc�������ͨ�����������Cc��
maxCRSize��AaBbCc��10ͨ����ͨ�������ͨ�����
iHeavyChannel�������󣨵���ͨ�����������С��10����ͨ����
iHeavyChannel2��
totalCRCount��totalCRCountNoCc��totalCRCountNoCcInJaw����ͨ����Ŀͳ��
aveCRCount��aveCRCountNoCc��ͨ��ƽ����ͨ����
aveCRSize��aveCRSizeNoCc��ͨ����ͨ��ƽ������
sumEveyChannel��16ͨ����ͨ����������
s������
percent�����ٷֱ�
*/
class PositionFeature
{
public:
	PositionFeature(VBDB& blocks, VCR* vCRs, VINT* t_cr, int step1, int step2, uint8_t rowCH1, uint8_t rowCH2, uint8_t railType);

public:
	int32_t	iAStepBig;
	int32_t iAStepSmall;
	int32_t iAStepBig2;
	int32_t iAStepSmall2;
	int32_t iAStepBigNoCc;
	int32_t iAStepSmallNoCc;
	uint16_t sumAaBb;
	uint16_t sumCc;
	uint16_t sum_ABC;
	uint16_t sum_abc;
	uint32_t iSumBigChannel;
	uint32_t iSumSmallChannel;
	uint32_t pointCountlInJaw[8];
	uint32_t sum10_16;
	uint8_t findEveyChannel[16];
	uint16_t sumEveyChannel[16];
	int32_t	ChannelNum;
	uint16_t maxSize;
	uint16_t maxSizeNoCc;
	int32_t maxCRSize[10];
	uint16_t iHeavyChannel;
	uint16_t iHeavyChannel2;
	int32_t totalCRCountNoCc;
	int32_t totalCRCount;
	int32_t totalCRCountNoCcInJaw;
	double aveCRCountNoCc;
	double aveCRCount;
	double aveCRSize;
	double aveCRSizeNoCc;
	double s;
	double percent;
	int32_t ir1;
	int32_t ir2;
	int iS1;
	int iS2;
	int aveStep;
	int aveRow;

	int32_t	SumAa;

	int32_t	SumBb;

	int32_t	sumAB;

	int32_t	sumab;

	int32_t	validC;

	int32_t validc;
};
void		GetWaveLen(VCR* vcr, VINT* tcr, Section* pSections, int& aveLenNoCc);

void		GetWaveLen2(VCR* vcr, VINT* tcr, Section* pSections, int& aveLenNoCc);

bool		GetStepInBlock2(uint32_t step, VBDB& blocks, int& blockIndex, int& stepIndex);

int32_t		GetNearMark(uint32_t beginStep, uint32_t endstep, VPM& vPMs, PM& pm);

int32_t		GetMarkedPosition(uint32_t beginStep, VPM& vPMs, int32_t* pstep2);

int32_t		GetMarkedPositionIndex(uint32_t beginStep, VPM& vPMs, int32_t* pstep2);

int32_t		GetManualJoint(uint32_t beginStep, VPM& vPMs);

int16_t		GetMarkFromManual(PM& manualPM);

uint32_t	GetLastPos(uint32_t step, std::map<uint32_t, Pos>& vPos, uint32_t& findStep);

int32_t		GetLenWithSlope(uint32_t step, PM& slope);

void		GetLoseStep(int centerStep, VBDB& blocks, int& step1, int& step2);

void		IsFGLose(int step1, int step2, VBDB& blocks, VCR* vCRs, int iFRow, uint8_t& iLoseF, VINT& t_crF, uint8_t& iLoseG, VINT& t_crG, int& iLose_Right, int& iLose_Left);

uint32_t	ParsePosition(F_HEAD& head, VBDB& blocks, VCR* vCRs, CR& cr, int index, int16_t iFDesiredRow, uint8_t railType, VINT* t_cr, PM& pm, int& iAStepBig, int &iAStepSmall, VPM& vPMs, VPM& vPMs2);

uint32_t	ParsePositionPost(F_HEAD& g_fileHead, VBDB& blocks, VCR* vCRs, int step1, int step2, uint8_t row1, uint8_t row2, int16_t iFDesiredRow, uint8_t railType, uint32_t mark, PM& pm, int& iAStepBig, int &iAStepSmall, VPM& vPMs);
