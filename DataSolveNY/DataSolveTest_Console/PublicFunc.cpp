#include "stdafx.h"
#include "PublicFunc.h"
#include "GlobalDefine.h"
#include <time.h>
#include <eh.h>

//log：经度, lat：纬度
bool ParseGPS(unsigned char* strGPS, double& log, double&lat)
{
	if (strGPS[0] != 'A')
	{
		log = lat = 0;
		return false;
	}
	char *pstrGPS = (char*)strGPS;
	char sz[20] = { 0 };
	strncpy(sz, pstrGPS + 4, 2);
	lat = atof(sz);
	strncpy(sz, pstrGPS + 7, 7);
	lat += atof(sz) / 60;

	memset(sz, 0, 20);
	strncpy(sz, pstrGPS + 16, 3);
	log = atof(sz);
	strncpy(sz, pstrGPS + 20, 7);
	log += atof(sz) / 60;

	return true;
}

unsigned char INT8ChangeToBCD(unsigned char srcNum)
{
	unsigned char aimNum = ((srcNum >> 4) & 0x0f) * 10 + (srcNum & 0x0f);
	return aimNum;
}

unsigned short INT16ChangeToBCD(unsigned short srcNum)
{
	unsigned short aimNum = (srcNum >> 12) * 1000 + ((srcNum >> 8) & 0x0f) * 100 + ((srcNum >> 4) & 0x0f) * 10 + (srcNum & 0x0f);
	return aimNum;
}

unsigned int INT32ChangeToBCD(unsigned int srcNum)
{
	unsigned int aimNum = ((srcNum >> 28) & 0x0f) * 10000000
		+ ((srcNum >> 24) & 0x0f) * 1000000
		+ ((srcNum >> 20) & 0x0f) * 100000
		+ ((srcNum >> 16) & 0x0f) * 10000
		+ ((srcNum >> 12) & 0x0f) * 1000
		+ ((srcNum >> 8) & 0x0f) * 100
		+ ((srcNum >> 4) & 0x0f) * 10
		+ (srcNum & 0x0f);
	return aimNum;
}


unsigned char	BCDToINT8(unsigned char bcd)
{
	return  ((0xf0 & bcd) >> 4) * 10 + (0x0f & bcd);
}

unsigned short	BCDToINT16(unsigned short bcd)
{
	return ((0xf000 & bcd) >> 12) * 1000
		+ ((0xf00 & bcd) >> 8) * 100
		+ ((0xf0 & bcd) >> 4) * 10
		+ (0x0f & bcd);
}

unsigned int	BCDToINT32(unsigned int bcd)
{
	unsigned int ret =
		((0xf0000000 & bcd) >> 28) * 10000000
		+ ((0xf000000 & bcd) >> 24) * 1000000
		+ ((0xf00000 & (bcd) >> 20)) * 100000
		+ ((0xf0000 & bcd) >> 16) * 10000
		+ ((0xf000 & bcd) >> 12) * 1000
		+ ((0xf00 & bcd) >> 8) * 100
		+ ((0xf0 & bcd) >> 4) * 10
		+ (0x0f & bcd);
	return ret;
}

/*
void stringToArray(string str, char splitter, uint16_t* data, int count)
{
	int idx =  //str.Find(splitter);
	int index = 0;
	string strTemp;
	while (idx > 0)
	{
		strTemp = str.Left(idx);
		data[index++] = StrToInt(strTemp);
		str = str.Mid(idx + 1);
		idx = str.Find(splitter);
	}
	data[index] = StrToInt(str);
}

void ArrayTostring(uint16_t* data, int count, string str, char splitter)
{
	string strTemp;
	for (int i = 0; i < count; ++i)
	{
		strTemp.Format("%d%c", data[i], splitter);
		str += strTemp;
	}
	str = str.Left(str.GetLength() - 1);
}
*/
std::string	 ToHexstring(int bcdData, int length)
{
    char szFormat[20] = "";
	sprintf(szFormat, "%%0%dX", length);
	char* pData = new char[length + 1];
	sprintf(pData, szFormat, bcdData);
	std::string str = pData;
	delete pData;
	return str;
}

std::string	Tostring(int data, int length)
{
	char szFormat[20] = "";
	sprintf(szFormat, "%%0%dd", length);
	char* pData = new char[length + 1];
	sprintf(pData, szFormat, data);
	std::string str = pData;
	delete pData;
	return str;
}

std::string	GetTimeString(uint32_t date, uint32_t time)
{
	uint16_t year = date >> 16;
	uint8_t	month = ((date & 0xFF00) >> 8);
	uint8_t day = date & 0xFF;

	uint8_t hour = time >> 16;
	uint8_t	minute = ((time & 0xFF00) >> 8);
	uint8_t second = time & 0xFF;

	uint8_t isOK = 1;
	if (year > 9999 || year < 2000 || month > 12 || day > 31 || hour > 23 || minute > 59 || second > 59)
	{
		isOK = 0;
	}

	if (isOK)
	{
		char szTime[20] = "";
		sprintf(szTime, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
		std::string ret = szTime;
		return ret;
	}
	else
	{
		return "0";
	}
}



std::string ToString(char* pData, int len)
{
    std::string str = "";
    for(int i = 0; i < len; ++i)
    str += pData[i];
    return str;
}

std::string GetNewFileName(F_HEAD& head, std::string strFileName, uint8_t leftRight, uint8_t xianbie)
{
	//20190214 11 10209 2839 D 00040005 LS 17S2018_0001_2.26
    uint32_t ft = head.startD;//开始作业时间

	uint32_t section = head.deviceP2.TrackSet.sectionNum;//单位编号5个BCD，工务段

	uint16_t lineNum = head.deviceP2.TrackSet.lineNum;	// 线编号4个BCD

	W_D w1 = head.startKm, w2 = head.endKm;

	uint8_t WalkWay = head.deviceP2.TrackSet.WalkWay;// 行走方向0: 逆里程, 1: 顺里程

	//uint8_t leftRight = head.deviceP2.TrackSet.leftRight;// 左右股0: 右1：左
    char szLeftRight[][2] = { "R", "L" };//左右股

	uint8_t lineWay = head.deviceP2.TrackSet.lineWay;	 // 行别0：单线、1：上，2：下行
    char szLineWay[][2] = { "D", "S", "X" };


	char cShunNi = head.deviceP2.TrackSet.WalkWay == 1 ? 'S' : 'N';
	uint16_t year = (ft & 0xFFFF0000) >> 16;
	uint8_t month = (ft & 0x00FF00) >> 8;
	uint8_t day = ft & 0xFF;

	if (GetTimeString(ft) == "0")
	{
		year = atoi(strFileName.substr(0, 2).c_str());
		month = atoi(strFileName.substr(2, 2).c_str());
		day = atoi(strFileName.substr(4, 2).c_str());

		int index = -1;
		for (int i = strFileName.length(); i >= 0; --i)
		{
			if (strFileName[i] == '_')
			{
				index = i;
				break;
			}
		}
		if (index > 0)
		{
			strncpy(head.DeviceNum, strFileName.substr(index + 1).c_str(), 7);
			head.DeviceNum[7] = '\0';
		}
	}

	uint8_t* pData = (uint8_t*)(&head);

	section = INT32ChangeToBCD(section);
	lineNum = INT16ChangeToBCD(lineNum);
	leftRight = INT8ChangeToBCD(leftRight);
	lineWay = INT8ChangeToBCD(lineWay);
	uint32_t stationNum = INT32ChangeToBCD(head.deviceP2.TrackSet.stationNum);

	char szWD[9] = "";
	sprintf(szWD, "%04d%04d", w1.Km, w2.Km);
	szWD[8] = 0;

	char szFileName[100] = "";
	if (xianbie != 0)
	{
		sprintf(szFileName, "%02d%02d%02d11%05dQ%05d", year % 100, month % 12, day % 30, section, lineNum);
	}
	else
	{
		sprintf(szFileName, "%02d%02d%02d11%05dZ%05d", year, month, day, section, stationNum);
	}
	
	strcat(szFileName, szLineWay[lineWay & 0x03]);

	strcat(szFileName, szWD);

	char sz[60] = { 0 };
	sprintf(sz, "%s%c%s_%04d_%s", szLeftRight[leftRight & 0x01], cShunNi, head.DeviceNum, head.deviceP2.TrackSet.teamNum, g_strAlgVersion);
	strcat(szFileName, sz);
	return szFileName;
}


//寻找某个步进的位置
Pos	FindStepInBlock(uint32_t step, VBDB& blocks, int iBeginBlock)
{
	int pblock = 0, pstep = 0;
	if (iBeginBlock < 0) iBeginBlock = 0;
	Pos pos;
	pos.Block = 0; pos.Step = 0; pos.Step2 = 0; pos.Walk = 0;
	bool bFind = false;
	int nBlockCount = blocks.size() - 1;
	if (step >= blocks[nBlockCount].IndexL2 + blocks[nBlockCount].BlockHead.row)
	{
		pos.Block = blocks[nBlockCount].Index;
		pos.Step = blocks[nBlockCount].BlockHead.row - 1;
		pos.Step2 = blocks[nBlockCount].IndexL2 + blocks[nBlockCount].BlockHead.row - 1;
		pos.Walk = GetWD(blocks[nBlockCount].Walk, pos.Step, g_filehead.step, g_direction);
		return pos;
	}
	if (step < blocks[0].IndexL2)
	{
		pos.Block = blocks[0].Index == 0 ? 0 : (blocks[0].Index - 1);
		pos.Step2 = step;
		pos.Step = (int)blocks[0].IndexL2 - step;
		pos.Walk = GetWD(blocks[0].Walk, pos.Step, g_filehead.step, g_direction);
		return pos;
	}
	int low = iBeginBlock, high = nBlockCount, mid = (low + high) / 2;
	bool isEmptyBlock = false;
	while (low <= high)
	{
		mid = (low + high) / 2;
		if (blocks[mid].IndexL2 <= step && blocks[mid].IndexL2 + blocks[mid].BlockHead.row > step)
		{
			bFind = true;
			pblock = mid;
			pstep = step - blocks[mid].IndexL2;
			break;
		}
		else if (blocks[mid].StepCount == 0 && blocks[mid].IndexL2 <= step && blocks[mid].IndexL2 + blocks[mid].BlockHead.row == step)
		{
			isEmptyBlock = true;
			bFind = true;
			pblock = mid;
			pstep = step - blocks[mid].IndexL2;
			break;
		}
		else if (step < blocks[mid].IndexL2)
		{
			high = mid - 1;
		}
		else if (step > blocks[mid].IndexL2)
		{
			low = mid + 1;
		}
	}
	if (bFind)
	{
		if (isEmptyBlock)
		{
			while (blocks[pblock].StepCount == 0 && pblock < blocks.size())
			{
				pblock++;
				pstep = 0;
			}
		}
		pos.Block = blocks[pblock].Index;
		pos.Step2 = step;
		pos.Step = pstep;
		pos.Walk = GetWD(blocks[pblock].Walk, pos.Step, g_filehead.step, g_direction);
	}
	return pos;
}

Pos	FindRawStepInBlock(int32_t step, VBDB& blocks, int iBeginBlock)
{
	int pblock = 0, pstep = 0;
	if (iBeginBlock < 0) iBeginBlock = 0;
	Pos pos;
	pos.Block = 0; pos.Step = 0; pos.Step2 = 0; pos.Walk = 0;
	bool bFind = false;
	int nBlockCount = blocks.size() - 1;
	for (int i = iBeginBlock; i < nBlockCount; ++i)
	{
		if (blocks[i].StepCount == 0)
		{
			continue;
		}
		if (i != iBeginBlock && blocks[i].IndexL < blocks[i - 1].IndexL)
		{
			pos.Block = i;
			pos.Step = 0;
			pos.Step2 = blocks[i].IndexL2;
			pos.Walk = GetWD(blocks[i].Walk, pos.Step, g_filehead.step, g_direction);
			break;
		}
		if (blocks[i].IndexL <= step && blocks[i].IndexL + blocks[i].StepCount >= step)
		{
			pos.Block = i;
			pos.Step = step - blocks[i].IndexL;
			pos.Step2 = blocks[i].IndexL2 + pos.Step;
			pos.Walk = GetWD(blocks[i].Walk, pos.Step, g_filehead.step, g_direction);
			break;
		}
	}
	return pos;
}

uint8_t	GetAChannelByBChannel(uint8_t iCh)
{
	return ChannelB2A[iCh];
}


double GetWD(W_D pos)
{
	return pos.Km + 0.001 * pos.m + 0.000001 * pos.mm;
}

double GetWD(W_D pos, int nStep, float stepDistance, bool direction)
{
	double wd = pos.Km + 0.001 * pos.m + 0.000001 * pos.mm;
	double r = 0.000001 * nStep * stepDistance;
	return direction ? wd + r : wd - r;
}

double	GetWD(double wd, int nStep, float stepDistance, bool  direction)
{
	double r = 0.000001 * nStep * stepDistance;
	return direction ? wd + r : wd - r;
}


int		GetOverlappedStep(int step11, int step12, int step21, int step22, int& begin, int&end)
{
	int count = 0;
	if (step11 >= step22 || step21 >= step12)
	{
		count = 0;
	}
	else if (step21 <= step11 && step11 && step22 >= step12)
	{
		count = step12 - step11;
		begin = step11;
		end = step12;
	}
	else if (step21 <= step11 && step22 >= step11)
	{
		count = step22 - step11;
		begin = step11;
		end = step22;
	}
	else if (step21 >= step11 && step22 >= step12)
	{
		count = step12 - step21;
		begin = step21;
		end = step12;
	}
	else if (step21 >= step11 && step22 <= step12)
	{
		count = step22 - step21;
		end = step22;
		begin = step21;
	}
	return count;
}

bool RemoveFromVector(std::vector<int>& datas, int data)
{
	for (int i = datas.size() - 1; i >= 0; --i)
	{
		if (datas[i] == data)
		{
			datas.erase(datas.begin() + i);
			break;
		}
	}
	//return datas.size() > 0 ? 1 : 0;
	return datas.size() > 0;
}

void	SetUsedFlag(CR& cr, int flag)
{
	cr.IsUsed = flag;
}

void	SetUsedFlag(VCR& vCR, VINT& idx, int Flag)
{
	for (int i = 0; i < idx.size(); ++i)
	{
		SetUsedFlag(vCR[idx[i]], Flag);
	}	
}


//设置螺孔标志
void	SetScrewHoleFlag(CR& cr, int flag)
{
	cr.IsGuideHole = 0;
	cr.IsScrewHole = flag;
}

void	SetScrewHoleFlag(VCR& vCR, VINT& idx, int flag)
{
	for (int i = 0; i < idx.size(); ++i)
	{
		SetScrewHoleFlag(vCR[idx[i]], flag);
	}
}

//设置导孔标志
void	SetGuideHoleFlag(CR& cr, int flag)
{
	cr.IsScrewHole = 0;
	cr.IsGuideHole = flag;
}

void	SetGuideHoleFlag(VCR& vCR, VINT& idx, int flag)
{
	for (int i = 0; i < idx.size(); ++i)
	{
		SetGuideHoleFlag(vCR[idx[i]], flag);
	}
}

//设置接头标志
void	SetJointFlag(CR& cr, int flag)
{
	cr.IsJoint = flag;
}

void	SetJointFlag(VCR& vCR, VINT& idx, int flag)
{
	for (int i = 0; i < idx.size(); ++i)
	{
		SetJointFlag(vCR[idx[i]], flag);
	}
}

//设置铝热焊标志
void	SetSewLRHFlag(CR& cr, int flag)
{
	cr.IsSew = flag;
}

void	SetSewLRHFlag(VCR& vCR, VINT& idx, int flag)
{
	for (int i = 0; i < idx.size(); ++i)
	{
		SetSewLRHFlag(vCR[idx[i]], flag);
	}
}

//设置厂焊标志
void	SetSewCHFlag(CR& cr, int flag)
{
	cr.IsSew = flag;
}
void	SetSewCHFlag(VCR& vCR, VINT& idx, int flag)
{
	for (int i = 0; i < idx.size(); ++i)
	{
		SetSewCHFlag(vCR[idx[i]], flag);
	}
}

void	SetWoundFlag(Wound_Judged& wound, int flag)
{
	wound.Flag = flag;
}

void	AddWoundData(Wound_Judged& wound, CR& cr, int isTail/* = 0*/)
{
	try
	{
		bool bFind = false;
		for (int i = 0; i < wound.vCRs.size(); ++i)
		{
			if (wound.vCRs[i].Channel == cr.Channel && wound.vCRs[i].Block == cr.Block && wound.vCRs[i].Step == cr.Step && wound.vCRs[i].Region.size() == cr.Region.size())
			{
				bFind = true;
				break;
			}
		}
		if (isTail == 1 && wound.Block == 325)
		{
			printf("%d", isTail);
		}
		if (bFind == false/* && cr.IsWound == 0*/)
		{
			wound.vCRs.emplace_back(cr);
			cr.IsWound++;
		}
	}
	catch (std::bad_alloc& e)
	{
		auto data = e.what();
		printf("%s", data);
	}	
}

void	AddWoundData(Wound_Judged& wound, VCR& vCR, VINT& idx)
{
	for (int i = 0; i < idx.size(); ++i)
	{
		AddWoundData(wound, vCR[idx[i]]);
	}
}

uint8_t isAccordingExist[200] = { 0 };
void	AddToWounds(VWJ& vWounds, Wound_Judged& w)
{
	//if (g_iEndStep - g_iBeginStep >= 3700 && w.Manual == 0 && ( g_iBeginBlock != 0 && w.Step2 < g_iBeginStep + 150 || w.Block != g_filehead.block - 1 && w.Step2 > g_iEndStep - 150) )
	//{
	//	return;
	//}


	uint8_t crCount = w.vCRs.size();
	memset(isAccordingExist, 0, 200);
	VCR vA = w.vCRs;
	for (int i = 0; i < w.vCRs.size(); ++i)
	{
		for (int j = i + 1; j < w.vCRs.size(); ++j)
		{
			if (w.vCRs[i].Channel == w.vCRs[j].Channel && w.vCRs[i].Index == w.vCRs[j].Index)
			{
				isAccordingExist[j] = 1;
			}
		}
	}
	w.vCRs.clear();
	for (int i = 0; i < vA.size(); ++i)
	{
		if (isAccordingExist[i] == 0)
		{
			AddWoundData(w, vA[i]);
		}
	}
	vWounds.emplace_back(w);
}

void	FillWound(Wound_Judged& wd, BLOCK& blockHead, F_HEAD& fhead)
{
	wd.equType = fhead.deviceP2.TrackSet.lineType;
	ParseGPS(blockHead.gpsInfor, wd.gps_log, wd.gps_lat);
}

void	FillWound2(Wound_Judged& wd, VBDB& blocks)
{
	wd.IsTunnel = g_vBlockHeads[wd.Block].BlockHead.gpsInfor[0] == 'V';
	if (wd.Block < blocks[0].Index || wd.Block > blocks[blocks.size() - 1].Index)
	{
		wd.IsCurve = 0;
		wd.IsBridge = 0;
	}
	else
	{
		wd.IsCurve = (blocks[wd.Block - g_iBeginBlock].vBStepDatas[wd.Step].Mark.Mark & CURVE) ? 1 : 0;
		wd.IsBridge = (blocks[wd.Block - g_iBeginBlock].vBStepDatas[wd.Step].Mark.Mark & QIAO) ? 1 : 0;
	}
}

void	CombineWound(Wound_Judged& w1, Wound_Judged& w2)
{
	if (w1.Flag == 0 || w2.Flag == 0)
	{
		w1.Flag = 0;
	}
	w2.Flag = 1;
	for (int i = 0; i < w2.vCRs.size(); ++i)
	{
		AddWoundData(w1, w2.vCRs[i], 1);
	}
	
	w1.IsJoint |= w2.IsJoint;
	w1.IsSew |= w2.IsSew;
}

void	AddToMarks(PM& mark, VPM& vPMs)
{
	vPMs.emplace_back(mark);
}

int		GetTimeSpan(int t1, int t2)
{
	int s1 = t1 & 0xFF;
	int m1 = (t1 >> 8) & 0xFF;
	int h1 = (t1 >> 16) & 0xFF;
	int d1 = (t1 >> 24) & 0xFF;

	int s2 = t2 & 0xFF;
	int m2 = (t2 >> 8) & 0xFF;
	int h2 = (t2 >> 16) & 0xFF;
	int d2 = (t2 >> 24) & 0xFF;
	
	int delt = s2 - s1 + (m2 - m1) * 60 + (h2 - h1) * 3600 + (d2 - d1) * 3600 * 24;
	return delt >= 0 ? delt : -delt;
}



int		Sum(int* data, int begin, int end, int totalLength)
{
	int sum = 0;
	int s = begin >= 0 ? begin : 0;
	int e = end <= (totalLength - 1) ? end : (totalLength - 1);
	for (int i = s; i <= e; ++i)
	{
		sum += data[i];
	}
	return sum;
}

uint32_t Abs(int x)
{
	return x >= 0 ? x : -x;
}

void	GetNearestHole(uint32_t step, HolePara& hp)
{
	int dist = 0x7FFFFFFF;
	HolePara htp;
	for (auto itr = g_vHoleParas.begin(); itr != g_vHoleParas.end(); ++itr)
	{
		int td = itr->first - step;
		if (Abs(td) < dist)
		{
			dist = Abs(td);
			htp = itr->second;
		}
		if (itr->first > step)
		{
			break;
		}
	}
	hp = htp;
}

void	GetNearestHole(uint32_t step, uint16_t mark, HolePara& hp)
{
	int dist = 0x7FFFFFFF;
	HolePara htp;
	for (auto itr = g_vHoleParas.begin(); itr != g_vHoleParas.end(); ++itr)
	{
		if (itr->second.mark == mark)
		{
			int td = itr->first - step;
			if (Abs(td) < dist)
			{
				dist = Abs(td);
				htp = itr->second;
			}
		}
		if (itr->first > step)
		{
			break;
		}
	}
	hp = htp;
}


uint8_t		GetJawRow(int iBeginFR, int step, int& iJawRow, int& iFRow, int& railType)
{
	bool bFind = false;
	if (g_vFR.size() == 0)
	{
		return 0;
	}
	if (step < g_vFR[0].Step2)
	{
		iFRow = g_vBlockHeads[0].BlockHead.railH / 3;
		bFind = true;
	}
	if (step > g_vFR[g_vFR.size() - 1].Step2)
	{
		iFRow = (g_vFR[g_vFR.size() - 1].FRow < g_vFR[g_vFR.size() - 1].GRow ? g_vFR[g_vFR.size() - 1].FRow : g_vFR[g_vFR.size() - 1].GRow);
		bFind = true;
	}
	for (int i = iBeginFR; i < g_vFR.size(); ++i)
	{
		if (g_vFR[i].Step2 < step && (i == g_vFR.size() - 1 || g_vFR[i+1].Step2 >= step) )
		{
			iFRow = (g_vFR[i].FRow < g_vFR[i].GRow ? g_vFR[i].FRow : g_vFR[i].GRow);			
			bFind = true;
			break;
		}
	}
	if (bFind)
	{
		if (iFRow <= g_iBottomRow[0] + 1)
		{
			railType = 0;
		}
		else if (iFRow <= g_iBottomRow[1] + 1)
		{
			railType = 1;
		}
		else if (iFRow <= g_iBottomRow[2] + 1)
		{
			railType = 2;
		}
		else
		{
			railType = 3;
		}
		iJawRow = g_iJawRow[railType] - (g_iBottomRow[railType] - iFRow);
		return 1;
	}
	return 0;
}

void	WriteLog(char* strLog)
{
	if (g_pFileLog != NULL)
	{
		time_t t1 = time(NULL);
		tm* lt = localtime(&t1);
		fprintf(g_pFileLog, "%02d:%02d:%02d %s", lt->tm_hour, lt->tm_min, lt->tm_sec, strLog);
		fflush(g_pFileLog);
	}
}

void	WriteLog2(char* strFormat, ...)
{
	if (g_pFileLog != NULL)
	{
		time_t t1 = time(NULL);
		tm* lt = localtime(&t1);

		char szBuf[1024] = "";
		va_list args;
		va_start(args, strFormat);
		snprintf(szBuf, 1024, strFormat, args);
		va_end(args);


		fprintf(g_pFileLog, "%02d:%02d:%02d %s", lt->tm_hour, lt->tm_min, lt->tm_sec, szBuf);
		fflush(g_pFileLog);
	}
}


int			GetNearestHeavyStepLen(int step)
{
	int len = 0x7FFFFFFF;
	for (auto itr = g_vHeavyPos.begin(); itr != g_vHeavyPos.end(); ++itr)
	{
		int dLen = itr->first - step;
		if (dLen < 0)
		{
			dLen = 0 - dLen;
		}
		if (dLen < len)
		{
			len = dLen;
		}
		if (itr->first > step)
		{
			break;
		}
	}
	return len;
}


uint8_t		GetRailTypeByFRow(uint8_t fRow, uint8_t* fRows, int railTypeCount)
{
	uint8_t railType = 0;
	for (int i = railTypeCount; i >= 1; --i)
	{
		if (fRow >= fRows[i - 1] - 2)
		{
			railType = i - 1;
			break;
		}
	}
	return railType;
}

uint8_t		GetMaxValueAndIndex(uint8_t* data, int count, uint8_t& maxValue, int& maxIndex)
{
	maxIndex = 0;
	maxValue = data[0];
	for (int i = 1; i < count; ++i)
	{
		if (data[i] > maxValue)
		{
			maxValue = data[i];
			maxIndex = i;
		}
	}
	return count > 0;
}

uint8_t		GetMaxValueAndIndex(std::vector<uint8_t>& data, uint8_t& maxValue, int& maxIndex)
{
	std::map<uint8_t, int> counts;
	for (int i = 0; i < data.size(); ++i)
	{
		if (counts.find(data[i]) == counts.end())
		{
			counts[data[i]] = 1;
		}
		else
		{
			counts[data[i]] = counts[data[i]] + 1;
		}
	}
	if (counts.size() > 0)
	{
		maxIndex = counts.begin()->first;
		maxValue = counts.begin()->second;
		for (auto itr = counts.begin(); itr != counts.end(); ++itr)
		{
			if (itr->second > maxValue)
			{
				maxValue = itr->second;
				maxIndex = itr->first;
			}
		}
	}

	return data.size() > 0;
}

void	Exclude(VINT& cr1, VINT& cr2)
{
	uint8_t* bEexist = new uint8_t[cr1.size()];
	for (int i = 0; i < cr1.size(); ++i)
	{
		bEexist[i] = 0;
		for (int j = 0; j < cr2.size(); ++j)
		{
			if (cr1[i] == cr2[j])
			{
				bEexist[i] = 1;
				break;
			}
		}
	}

	VINT ret;
	for (int i = 0; i < cr1.size(); ++i)
	{
		if (bEexist[i] == 0)
		{
			ret.emplace_back(cr1[i]);
		}
	}

	cr1 = ret;
	delete bEexist;
	bEexist = NULL;
}




std::string GetValue(std::map<uint8_t, std::string>& map, uint8_t& key)
{
	if (map.find(key) != map.end())
	{
		return map[key];
	}
	else
	{
		return "";
	}
}


bool IsJoint(uint16_t mark)
{
	return mark == PM_JOINT || mark == PM_JOINT2 || mark == PM_JOINT4 || mark == PM_JOINT6 || mark == PM_JOINT_RIGHT || mark == PM_JOINT_LEFT;
}

bool IsSew(uint16_t mark)
{
	return mark == PM_SEW_CH || mark == PM_SEW_LIVE || mark == PM_SEW_LRH;
}
bool		IsHole(uint16_t mark)
{
	return mark == PM_SCREWHOLE || mark == PM_GUIDEHOLE;
}

bool IsBacked(uint32_t step1, uint32_t step2, VINT& vBackSteps)
{
	if (step1 > step2)
	{
		uint32_t t = step1;
		step1 = step2;
		step2 = t;
	}
	for (int i = 0; i < vBackSteps.size(); ++i)
	{
		if (vBackSteps[i] >= step1 && vBackSteps[i] <= step2)
		{
			return true;
		}
	}
	return false;
}

void PrintWound(VWJ& vWounds, std::string filePath, int beginIndex /* = 0 */ )
{
	FILE* pFile = fopen(filePath.c_str(), "w");
	fprintf(pFile, "%d\n", (int)vWounds.size());
	for (int i = beginIndex; i < vWounds.size(); ++i)
	{
		fprintf(pFile, "%d, %d, %d\n", vWounds[i].Block, vWounds[i].Step, vWounds[i].Type);
	}
	fclose(pFile);
}

void PrintWound2(VWJ& vWounds, std::string filePath, int beginIndex /* = 0 */)
{
	FILE* pFile = fopen(filePath.c_str(), "w");
	fprintf(pFile, "%d\n", (int)vWounds.size());
	for (int i = beginIndex; i < vWounds.size(); ++i)
	{
		fprintf(pFile, "%d, %d, %d, %d\n", vWounds[i].Block, vWounds[i].Step, vWounds[i].Type, vWounds[i].Flag);
	}
	fclose(pFile);
}



bool	PMCompare(PM& pm1, PM& pm2)
{
	return pm1.Step2 < pm2.Step2;
}

bool	WoundCompare(Wound_Judged& w1, Wound_Judged& w2)
{
	return w1.Step2 < w2.Step2;
}

void	FillManualMark(int step, int stepWave, VPM& vPMs2)
{
	int low = 0, high = vPMs2.size() - 1, mid = (low + high) / 2;
	while (low <= high)
	{
		mid = (low + high) / 2;
		if (vPMs2[mid].Step2 == step)
		{
			vPMs2[mid].IsFindWave = stepWave;
			break;
		}
		else if (step < vPMs2[mid].Step2)
		{
			high = mid - 1;
		}
		else if (step > vPMs2[mid].Step2)
		{
			low = mid + 1;
		}
	}
}

void		RemoveRepeatedPM(VPM& vPM, int distance)
{
	for (int i = vPM.size() - 1; i >= 0; --i)
	{
		for (int j = i - 1; j >= 0; --j)
		{
			if (vPM[i].Mark == vPM[j].Mark && vPM[i].Step2 - vPM[j].Step2 < distance)
			{
				vPM.erase(vPM.begin() + i);
				break;
			}
			if (vPM[i].Step2 - vPM[j].Step2 > distance)
			{
				break;
			}
		}	
	}
}


void		PMToPMRaw(PM& pm, Position_Mark_RAW& rawpm)
{
	//memcpy(&pm, &rawpm, sizeof(Position_Mark_RAW));
	rawpm.ARow = pm.ARow;
	rawpm.AStep = pm.AStep;
	rawpm.BeginStep = pm.BeginStep;
	rawpm.Block = pm.Block;
	rawpm.ChannelNum = pm.ChannelNum;
	rawpm.Data = pm.Data;
	rawpm.Fangcha = pm.Fangcha;
	rawpm.Flag = pm.Flag;
	rawpm.GuideHoleCount = pm.GuideHoleCount;
	rawpm.Height = pm.Height;
	rawpm.Length = pm.Length;
	rawpm.Manual = pm.Manual;
	rawpm.Mark = pm.Mark;
	memcpy(rawpm.Num, pm.Num, sizeof(pm.Num));
	rawpm.Percent = pm.Percent;
	rawpm.ScrewHoleCount = pm.ScrewHoleCount;
	rawpm.Size = pm.Size;
	rawpm.Step = pm.Step;
	rawpm.Step2 = pm.Step2;
	rawpm.Walk = pm.Walk;
	rawpm.Walk2 = pm.Walk2;
}


bool		IsSpell(BlockData_B& block1, BlockData_B& block2)
{
	bool bNotSpell =
		(block1.BlockHead.probOff[ACH_A] == block2.BlockHead.probOff[ACH_A]) &&
		(block1.BlockHead.probOff[ACH_a] == block2.BlockHead.probOff[ACH_a]) &&
		(block1.BlockHead.probOff[ACH_B] == block2.BlockHead.probOff[ACH_B]) &&
		(block1.BlockHead.probOff[ACH_b] == block2.BlockHead.probOff[ACH_b]) &&
		(block1.BlockHead.probOff[ACH_C] == block2.BlockHead.probOff[ACH_C]) &&
		(block1.BlockHead.probOff[ACH_c] == block2.BlockHead.probOff[ACH_c]) &&
		(block1.BlockHead.probOff[ACH_D] == block2.BlockHead.probOff[ACH_D]) &&
		(block1.BlockHead.probOff[ACH_E] == block2.BlockHead.probOff[ACH_E]) &&
		(block1.BlockHead.probOff[ACH_G] == block2.BlockHead.probOff[ACH_G]) ;
	return !bNotSpell;
}

bool		IsExistJointSew(int step1, int step2, VPM& vPMs, int& markIndex, int beginIndex, int endIndex)
{
	markIndex = -1;
	for (int i = endIndex; i >= 0; --i)
	{		
		if (vPMs[i].Step2 >= step1 && vPMs[i].Step2 <= step2 && (IsSew(vPMs[i].Mark) || IsJoint(vPMs[i].Mark)) && vPMs[i].Manual == 0)
		{
			markIndex = i;
			return true;
		}
		if (vPMs[i].Step2 >= g_iEndStep || vPMs[i].Step2 < g_iBeginStep)
		{
			break;
		}
	}
	return false;
}

bool	IsExistHeavyPoint(int step1, int step2, VINT& vHeavyStep)
{
	vHeavyStep.clear();
	for (auto itr = g_vHeavyPos.begin(); itr != g_vHeavyPos.end(); ++itr)
	{
		if (itr->first > step2)
		{
			break;
		}
		if (itr->first >= step1 && itr->first <= step2)
		{
			vHeavyStep.push_back(itr->first);
		}
	}
	return vHeavyStep.size() > 0;
}

bool	IsExistHeavyPoint2(int step1, int step2, VINT& vHeavyStep)
{
	vHeavyStep.clear();
	for (auto itr = g_vHeavyPos2.begin(); itr != g_vHeavyPos2.end(); ++itr)
	{
		if (itr->first > step2)
		{
			break;
		}
		if (itr->first >= step1 && itr->first <= step2)
		{
			vHeavyStep.push_back(itr->first);
		}
	}
	return  vHeavyStep.size() > 0;
}

bool		IsExistHeavyPoint3(int step1, int step2, VINT& vHeavyStep)
{
	vHeavyStep.clear();
	for (auto itr = g_vHeavyPos3.begin(); itr != g_vHeavyPos3.end(); ++itr)
	{
		if (itr->first > step2)
		{
			break;
		}
		if (itr->first >= step1 && itr->first <= step2)
		{
			vHeavyStep.push_back(itr->first);
		}
	}
	return  vHeavyStep.size() > 0;
}

int32_t	GetMarkedPositionInArea(uint32_t beginStep, uint32_t endStep, VPM& vPMs, int32_t* pstep2, int* pIndex)
{
	for (int i = 0; i < vPMs.size(); ++i)
	{
		if ((IsJoint(vPMs[i].Mark) || IsSew(vPMs[i].Mark) || vPMs[i].Mark == PM_SELFDEFINE && vPMs[i].Data >= 1 && vPMs[i].Data <= 3) && vPMs[i].Manual == 1 && vPMs[i].IsOverlapped == 0)
		{
			if (vPMs[i].Step2 >= beginStep && vPMs[i].Step2 <= endStep)
			{
				if (pstep2 != nullptr)
				{
					*pstep2 = vPMs[i].Step2;
				}
				if (pIndex != nullptr)
				{
					*pIndex = i;
				}
				return vPMs[i].Mark;
			}
		}
		if (vPMs[i].Step2 > endStep)
		{
			break;
		}
	}
	return -1;
}


void	GetBlockInfo(VBDB& blocks, int blockIndex, bool& cartype, uint8_t& railType, int16_t& iFRow)
{
	int bIndex = blockIndex;
	if (bIndex >= blocks.size())
	{
		bIndex = blocks.size() - 1;
	}
	else if (blockIndex < 0)
	{
		bIndex = 0;
	}
	cartype = blocks[bIndex].BlockHead.detectSet.Identify & BIT2;// 车型，1-右手车，0-左手车
	railType = blocks[bIndex].BlockHead.railType & 0x03;
	iFRow = blocks[bIndex].BlockHead.railH / 3;
}