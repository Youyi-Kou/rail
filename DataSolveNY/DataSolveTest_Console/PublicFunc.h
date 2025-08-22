#ifndef PUBLICFUNC_H
#define PUBLICFUNC_H
#include "DataEntities.h"

bool		ParseGPS(unsigned char* strGPS, double& log, double&lat);

uint8_t		INT8ChangeToBCD(unsigned char srcNum);

uint16_t	INT16ChangeToBCD(unsigned short srcNum);

uint32_t	INT32ChangeToBCD(unsigned int srcNum);

uint8_t		BCDToINT8(unsigned char bcd);

uint16_t	BCDToINT16(unsigned short bcd);

uint32_t	BCDToINT32(unsigned int bcd);


std::string	GetTimeString(uint32_t date, uint32_t time = 0);

std::string GetCurrentTimeString();

std::string	ToHexstring(int bcdData, int length);

std::string	Tostring(int data, int length);

//leftRight;// 左右股0: 右1：左
std::string GetNewFileName(F_HEAD& head, std::string strFileName, uint8_t leftRight, uint8_t xianbie);

//寻找某个步进的位置
Pos			FindStepInBlock(uint32_t step, VBDB& blocks, int ibeginBlock);

Pos			FindRawStepInBlock(int32_t step, VBDB& blocks, int ibeginBlock);

//根据B中的通道，寻找A中通道
uint8_t		GetAChannelByBChannel(uint8_t bChannel);

//计算里程
double		GetWD(W_D walk);

//direction: true:顺里程，false:逆里程
double		GetWD(W_D pos, int nStep, float stepDistance, bool  direction = true);

//direction: true:顺里程，false:逆里程
double		GetWD(double wd, int nStep, float stepDistance, bool  direction = true);

int			GetOverlappedStep(int step11, int step12, int step21, int step22, int& begin, int&end);

bool		RemoveFromVector(std::vector<int>& datas, int data);
template<class T> void RemoveFromVector(std::vector<int>& datas, int data);

//设置判断标志
void		SetUsedFlag(CR& cr, int flag);
void		SetUsedFlag(VCR& vCR, VINT& idx, int flag);

//设置螺孔标志
void		SetScrewHoleFlag(CR& cr, int flag);
void		SetScrewHoleFlag(VCR& vCR, VINT& idx, int flag);

//设置导孔标志
void		SetGuideHoleFlag(CR& cr, int flag);
void		SetGuideHoleFlag(VCR& vCR, VINT& idx, int flag);

//设置接头标志
void		SetJointFlag(CR& cr, int flag);
void		SetJointFlag(VCR& vCR, VINT& idx, int flag);

//设置铝热焊标志
void		SetSewLRHFlag(CR& cr, int flag);
void		SetSewLRHFlag(VCR& vCR, VINT& idx, int flag);

//设置厂焊标志
void		SetSewCHFlag(CR& cr, int flag);
void		SetSewCHFlag(VCR& vCR, VINT& idx, int flag);

void		SetWoundFlag(Wound_Judged& wound, int flag);

//增加伤损/位置判据
void		AddWoundData(Wound_Judged& wound, CR& cr, int isTail = 0);
void		AddWoundData(Wound_Judged& wound, VCR& vCR, VINT& idx);

void		AddToWounds(VWJ& vWounds, Wound_Judged& w);
	
void		FillWound(Wound_Judged& wd, BLOCK& blockHead, F_HEAD& fhead);

void		FillWound2(Wound_Judged& wd, VBDB& blocks);

void		CombineWound(Wound_Judged& w1, Wound_Judged& w2);

void		AddToMarks(PM& mark, VPM& vPMs);

int			GetTimeSpan(int t1, int t2);

int			Sum(int* data, int begin, int end, int totalLength);


uint32_t	Abs(int x);

//获取最近的孔的信息
void		GetNearestHole(uint32_t step, HolePara& hp);

void		GetNearestHole(uint32_t step, uint16_t mark, HolePara& hp);

uint8_t		GetJawRow(int iBeginFR, int step, int& iJawRow, int& iFRow, int& railType);

void		WriteLog(char* strLog);

void		WriteLog2(char* strFormat, ...);

int			GetNearestHeavyStepLen(int step);


uint8_t		GetRailTypeByFRow(uint8_t fRow, uint8_t* fRows, int railTypeCount);


uint8_t		GetMaxValueAndIndex(uint8_t* data, int count, uint8_t& maxValue, int& maxIndex);


uint8_t		GetMaxValueAndIndex(std::vector<uint8_t>& data, uint8_t& maxValue, int& maxIndex);

void		Exclude(VINT& cr1, VINT& cr2);

std::string GetValue(std::map<uint8_t, std::string>& map, uint8_t& key);

bool		IsJoint(uint16_t mark);

bool		IsSew(uint16_t mark);

bool		IsHole(uint16_t mark);

bool		IsBacked(uint32_t step1, uint32_t step2, VINT& vBackSteps);

void		PrintWound(VWJ& vWounds, std::string filePath, int beginIndex = 0);

void		PrintWound2(VWJ& vWounds, std::string filePath, int beginIndex = 0);

bool		PMCompare(PM& pm1, PM& pm2);

bool		WoundCompare(Wound_Judged& w1, Wound_Judged& w2);

void		FillManualMark(int step, int stepWave, VPM& vPMs2);

void		RemoveRepeatedPM(VPM& vPM, int distance);

void		PMToPMRaw(PM& pm, Position_Mark_RAW& rawpm);



bool		IsSpell(BlockData_B& block1, BlockData_B& block2);

bool		IsExistJointSew(int step1, int step2, VPM& vPMs, int& markIndex, int beginIndex, int endIndex);

bool		IsExistHeavyPoint(int step1, int step2, VINT& vHeavyStep);

bool		IsExistHeavyPoint2(int step1, int step2, VINT& vHeavyStep);

bool		IsExistHeavyPoint3(int step1, int step2, VINT& vHeavyStep);

int			GetSewCHMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall);

int			GetSewLRHMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall, PM& pm);

int32_t		GetMarkedPositionInArea(uint32_t beginStep, uint32_t endStep, VPM& vPMs, int32_t* pstep2, int* pIndex = nullptr);

void		GetBlockInfo(VBDB& blocks, int blockIndex, bool& cartype, uint8_t& railType, int16_t& iFRow);
#endif