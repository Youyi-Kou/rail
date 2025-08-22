#include "stdafx.h"
#include "DataEntities.h"

Wound_Judged::Wound_Judged()
{
	SetEmpty();
}

void Wound_Judged::SetEmpty()
{
	Flag = 0;
	memset(this->Result, 0, sizeof(char) * 60);
	//memset(this->WoundNo, 0, sizeof(char) * 64);
	Walk = 0.0;
	Place = 0;
	Type = 0;
	Degree = 0;
	SizeX = SizeY = 0;
	Checked = 0;
	Walk2 = 0.0;

	StepLen = -1;
	Row1 = 0;
	RowLen = -1;

	gps_log = 0;
	gps_lat = 0;

	IsBridge = 0;
	IsTunnel = 0;
	IsCurve = 0;
	IsJoint = 0;
	IsSew = 0;
	IsScrewHole = 0;
	IsGuideHole = 0;

	this->Cycle = 1;
	LastCycleID = 0;

	Manual = 0;

	Block = 0;
	Step = 0;
	Step2 = 0;
	FileID = 0;
	this->vCRs.clear();
	this->vLCRs.clear();

	ChannelNum = 0;
	ChannelMaxNum = 0;
	memset(this->Num, 0, sizeof(uint16_t) * 16);

	IsReversed = 0;
	IsMatched = 0;

	Rect.step1 = 0;
	Rect.step2 = 0;
	Rect.row1 = 0;
	Rect.row2 = 0;
}

bool Wound_Judged::operator<(Wound_Judged& w)const
{
	//return this->Rect.step1 < w.Rect.step1;
	return this->Step2 < w.Step2;
}

bool _WaveData::operator < (_WaveData& wd) const
{
	if (this->block < wd.block)
	{
		return true;
	}
	else if (this->block > wd.block)
	{
		return false;
	}

	if(this->step < wd.step)
	{
		return true;
	}
	else if (this->step > wd.step)
	{
		return false;
	}

	if (this->row < wd.row)
	{
		return true;
	}
	else if (this->row > wd.row)
	{
		return false;
	}

	return this->find < wd.find;
}

bool _WaveData::operator == (_WaveData& wd) const
{
	return (this->block == wd.block && this->step == wd.step && this->row == wd.row	&& this->find == wd.find);
}

Connected_Region::Connected_Region()
{
	Block = 0;
	Step = 0;
	Row1 = 0;
	Step1 = 0;
	Row2 = 0;
	Step2 = 0;
	Channel = 0;
	IsUsed = 0;
	memset(&Info, 0, szCRA);
	H1 = 0;
	H2 = 0;

	MinH = 0;
	MaxH = 0;

	IsDirty = 0;
	IsLose = 0;//Ä¬ÈÏ²»ÊÇÊ§²¨

	IsJoint = 0;
	IsSew = 0;
	IsWound = 0;
	IsScrewHole = 0;
	IsGuideHole = 0;
	IsContainA = 0;

	IsDoubleCR = 0;
	IsDoubleWave = 0;
	IsEnsureNotDoubleWave = 0;
	IsReversed = 0;
	CombinedCount = 0;
	IsIllgeal = 0;

	Index = -1;
}


bool Connected_Region::operator < (Connected_Region& cr) const
{
	return this->Step1 < cr.Step1;
}


LoseConnected_Region::LoseConnected_Region(uint8_t channel, int step1, int step2, uint8_t row1, uint8_t row2, uint8_t isSew, uint8_t isjoint, uint8_t isScrewHole, uint8_t isGuideHole)
{
	this->Channel = channel;
	this->Step1 = step1;
	this->Step2 = step2;
	this->Row1 = row1;
	this->Row2 = row2;
	this->IsSew = isSew;
	this->IsJoint = isjoint;
	this->IsScrewHole = isScrewHole;
	this->IsGuideHole = isGuideHole;
}