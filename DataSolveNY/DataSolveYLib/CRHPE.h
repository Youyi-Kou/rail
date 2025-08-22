#ifndef CRHPE_H
#define CRHPE_H

#include "DataEntities.h"


//������룬���ں���ͨ��ʱʹ��
uint32_t GetDistance(CR& cr1, WaveData& wd);

uint32_t GetDistance(CR& cr1, CR& cr2, int* pH = NULL, int * pV = NULL);

uint32_t GetDistanceH(CR& cr1, CR& cr2, int& iOverlappedCount);

uint32_t GetDistanceV(CR& cr1, CR& cr2, int& iOverlappedCount);

double	 GetDistanceHA(CR& cr1, CR& cr2);

double	 GetDistanceVA(CR& cr1, CR& cr2);


//������ͨ��
int		GetOverlappedCountH(CR& cr1, CR& cr2);

int		GetOverlappedCountV(CR& cr1, CR& cr2);


void	GetCRRowInfo(CR& cr, int& firstRow, int& lastRow);

void	GetCRRowInfo1(CR& cr, std::map<uint32_t, _StepRegion>& vStep);

void	GetCRRowInfo2(CR& cr, int& firstRow, int& lastRow, uint8_t channel);

void	GetCRRowInfo3(CR& cr, int& firstRow, int& lastRow, uint8_t channel, int& firstRow2, int& lastRow2);

void	GetCRRowInfo4(CR& cr, std::map<uint8_t, uint32_t>& vRowPointCount);



void	GetCoreCR(CR& cr, std::map<uint8_t, int>& rowDistribute, CR& crOut, VBDB& blocks, uint8_t jawRow);

bool	IsDoubleCR(CR& cr, VBDB& blocks, int channelDesired, CR& cr1, CR& cr2);

//�Ƿ��۵���CR�����Ļ��������״��0��������1���ģ�2����(DEͨ��)
int		IsAngleCR(CR& cr, VBDB& blocks, CR& cr1, CR& cr2);

int		IsAngleCR2(CR& cr, VBDB& blocks, CR& cr1, CR& cr2);

//�Ƿ���ϵĳ���������������״��0����1����
int		IsHoleCR(CR& cr, VCR* vCRs, VBDB& blocks, uint8_t iJawRow, uint8_t iMaxHeadRow, uint8_t iFRow);

bool	CanCombine(Connected_Region cr1, Connected_Region cr2, uint8_t channel);

void    RemoveRepeatedPoints(CR& cr);

void	CombineABC(VCR &vCR, uint8_t channel);

void	CombineDE(VCR &vCR, uint8_t channel);

void	CombineFG(VCR& vCR);

void	Combine(CR& cr1, CR& cr2);

bool	TryCombineFGInHole(CR& cr1, CR& cr2, int lengthLimit = 25);

bool	TryCombineFG1InHole(CR& cr1, CR& cr2);


void	RemoveInvalidCR(VCR& vCR);

void	FillCR(CR& cr);

//vSmarts���µ�
uint8_t	CreateCR(VBDB& datas, VCR* vCRs, VER& vER, VPM& vSmarts);




//��ȡָ��ͨ����CR
/************************************************************************/
/*             channel��ͨ��                                                  */

/************************************************************************/
uint8_t GetCR(uint8_t channel, int step1, int step2, VBDB& vBDatas, VCR& crToFind, VINT& vCrFound, int32_t iExcept = -1, int iMinimunSize = 1, bool bNeedUnJudged = false);

uint8_t GetCR(uint8_t channel, int step1, uint8_t row1, int step2, uint8_t row2, VBDB& vBDatas, VCR& crToFind, VINT& vCrFound, int32_t iExcept = -1, int iMinimunSize = 1, bool bNeedUnJudged = false);

uint8_t GetCR(uint8_t channel, int step1, uint8_t row1, int step2, uint8_t row2, VBDB& vBDatas, VCR& crToFind, VINT& vCrFound, VINT vExcept, int iMinimunSize = 1, bool bNeedUnJudged = false);

void	GetMaxCR(uint8_t channel1, uint8_t channel2, int step1, int step2, VBDB& vBDatas, VCR* vCRs, CR& cr);

uint8_t GetCRsStep(VCR& vcr, VINT& vIndexes, uint32_t& step1, uint32_t& step2);

int16_t GetFRow(B_Step& step);
int16_t GetGRow(B_Step& step);

int16_t GetFRow2(B_Step& step, int16_t desiredRow);
int16_t GetGRow2(B_Step& step, int16_t desiredRow);


//����AB����Ӧ


int		GetBeginFrameIndexByStep(BlockData_A& DataA, int32_t& step);

int		GetEndFrameIndexByStep(BlockData_A& DataA, int32_t& step);

//��ȡB���в�����Ӧ��A������
/***********************************************************************
channel��	ͨ��,DE,�����ж��ݿס����׵�˫��
cr��		��������
DataB��	B���׿�ͷ
angle��	ͨ��̽ͷ�Ƕ�
offset��	̽ͷƫ����
stepDistance��ÿ�������ȣ�mm��
***********************************************************************/
bool	GetCRASteps(uint8_t channel, Connected_Region& cr, int32_t& step1, int32_t& step2, VBDB& DataB, double angle, int offset, double stepDistance);

bool	GetCRASteps(uint8_t channel, WaveData& wd, int32_t& step1, int32_t& step2, VBDB& DataB, double angle, int offset, double stepDistance);


//��ȡ��Ӧͨ����ĳ������Χ�ڵ�A������
bool	GetAFrames(BlockData_A& DataA, uint32_t& step1, uint32_t& step2, uint8_t ichA, uint16_t h1, uint16_t h2, VASTEPS &vFrames);

int		FindStepInAData(int32_t step, BlockData_A& vAFrames);

uint8_t GetStepPeaks(A_Step& step, uint8_t iChA);

//��ȡB���в�����Ӧ��A������
/***********************************************************************
channel��	ͨ��
cr��		��������
frames��	�ҵ���֡
DataA��	A��ȫ������
DataB��	B���׿�ͷ
angle��	ͨ��̽ͷ�Ƕ�
offset��	̽ͷƫ����
stepDistance��ÿ�������ȣ�mm��
***********************************************************************/
bool	GetCRFrames(Connected_Region& cr, std::vector<A_Step>& vSteps, BlockData_A& DataA, VBDB& DataB, double angle, int offset, double stepDistance, uint8_t isUnique = 0);

bool	GetCRInfo(CR& cr, BlockData_A& DataA, VBDB& DataB, uint8_t isUnique = 0);




bool	RemoveHoleCR(VCR& vcr, VINT& crIndexes);

bool	RemoveHoleCRExcept(VCR& vcr, VINT& crIndexes, int except);

bool	RemoveJointCR(VCR& vcr, VINT& crIndexes);

bool	RemoveCRByLengthLimit(VCR& vcr, VINT& crIndexes, int maxLength);

bool	RemoveUsedCR(VCR& vcr, VINT& crIndexes);

bool	RemoveScrewHoleCR(VCR& vcr, VINT& crIndexes);

bool	RemoveCRByRow1Limit(VCR& vcr, VINT& crIndexes, uint8_t row1Limit);

bool	RemoveCRByWoundFlag(VCR& vcr, VINT& crIndexes);

bool	RemovePMCR(VCR& vcr, VINT& crIndexes);

bool	RemoveCRByLose(VCR& vcr, VINT& crIndexes, int lose);

//�ж�λ�ñ�ʱ�Ĺ���
bool	RemoveDirtyCR(VCR& vcr, VINT&crIndexes, int iJawRow);


bool	IsCRDirty_ABC(VBDB& blocks, VCR* vCRs, CR& cr, uint8_t isDeleteWoundCR = 0);

bool	IsCRDirty_DE(VBDB& blocks, VCR* vCRs, CR& cr, uint8_t isDeleteWoundCR = 0);

void	RemoveWoundCR(Wound_Judged& w, int index);








void	GetFRowRegion(VBDB& blocks, int step1, int step2, int stepChanging, std::map<uint8_t, uint8_t>&mapExistRows, std::vector<uint8_t>& vRows, bool isG = false);

void	SetFRowRegion(VBDB& blocks, int step1, int step2, uint8_t row, std::map<uint32_t, bool>& mapRising, bool isG = false);

#endif