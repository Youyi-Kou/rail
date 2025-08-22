#include "stdafx.h"
#include "SolveData.h"

#include "Judge.h"
#include <math.h>
#include <stdlib.h>
#include "PublicFunc.h"
#include "hole.h"
#include "CRHPE.h"
#include "PositionHPE.h"
#include "GlobalDefine.h"
#include <algorithm>
#include <thread>

void    SetFileHead(F_HEAD& head)
{
	memcpy(&g_filehead, &head, szFileHead);
	g_vFR.clear();
	g_vHeavyPos.clear();
	g_vHeavyPos2.clear();

	vExistedPMs.clear();
	g_vJointPos.clear();
	g_vSewPos.clear();

	g_vReturnSteps.clear();
	g_vFLoseBig.clear();
	g_vJWRD.clear();
	g_vJWRE.clear();

	//memset(g_StepPoints, 0, sizeof(g_StepPoints));

	g_vStepsInBackArea.clear();
	g_vStepsInJoint.clear();
	g_vStepsInSew.clear();
	g_vStepsInK.clear();
	g_vStepsInCouple.clear();
	g_vStepsInWound.clear();
	g_vHoleParas.clear();
	g_vSlopes.clear();


	iLastScrewHoleRow = 0;
	iLastScrewHoleRow2 = 0;
	iLastScrewHoleFRow = 58;
	iLastScrewHoleIndex = 0;
	iLastScrewHoleStep = 0;
	iLastScrewHoleRailType = 2;

	memset(&fork, 0, sizeof(fork));
}


template<typename T>
void GetMaxValue(T* data, int count, T& value, int& index)
{
	T maxData = data[0];
	index = 0;
	for (int i = 1; i < count; ++i)
	{
		if (maxData < data[i])
		{
			maxData = data[i];
			index = i;
		}
	}
}

uint8_t GetSlopeRow(std::vector<int16_t>& vFRow, int index, int& index1, int& row1, int& index2, int& row2)
{
	row1 = vFRow[index];
	row2 = vFRow[index];
	int cLen = 0;
	for (int i = index - 1; i >= 0; --i)
	{
		if (vFRow[i] - row1 >= -1 && vFRow[i] - row1 <= 1)
		{
			cLen++;
		}
		else
		{
			cLen = 1;
			row1 = vFRow[i];
		}

		if (cLen >= 20)
		{
			index1 = index - i;
			break;
		}
	}

	uint8_t fRows[VALID_ROW] = { 0 };
	for (int i = index - index1; i < index; ++i)
	{
		if (vFRow[i] > 0)
		{
			fRows[vFRow[i]] += 1;
		}
	}
	uint8_t maxValue;
	GetMaxValue(fRows, (int)VALID_ROW, maxValue, row1);

	cLen = 0;
	for (int i = index + 1; i < vFRow.size(); ++i)
	{
		if (vFRow[i] - row2 >= -1 && vFRow[i] - row2 <= 1)
		{
			cLen++;
		}
		else
		{
			cLen = 1;
			row2 = vFRow[i];
		}

		if (cLen >= 20)
		{
			index2 = i - index;
			break;
		}
	}
	memset(fRows, 0, VALID_ROW);
	for (int i = index + index2; i > index; --i)
	{
		if (vFRow[i] > 0)
		{
			fRows[vFRow[i]] += 1;
		}
	}
	GetMaxValue(fRows, (int)VALID_ROW, maxValue, row2);

	std::map<uint8_t, bool> mapRows;
	for (int i = index - index1; i < index + index2; ++i)
	{
		if (vFRow[i] > 0)
		{
			mapRows[vFRow[i]] = true;
		}
	}
	return mapRows.size() >= 4 ? 1 : 0;
}

void	FillMarksWounds(VBDB& blocks, VPM& vPMs, VWJ& vWounds, VPM& vPMs2, VLCP& vLCP, F_HEAD& g_fileHead)
{
	int markCount = vPMs.size();
	int mark2Count = vPMs2.size();

	PM pm;
	pm.Data = 0;

	for (size_t i = 0; i < blocks.size(); i++)
	{
		for (size_t j = 0; j < blocks[i].vBStepDatas.size(); j++)
		{
			uint16_t& wound = blocks[i].vBStepDatas[j].Wound.W_Mark;
			if (wound & W_MAN)
			{
				if (g_vStepsInWound.find(blocks[i].IndexL2 + j) == g_vStepsInWound.end())
				{
					Wound_Judged wd;
					wd.Block = blocks[i].Index;
					wd.Step = j;
					wd.Step2 = blocks[i].vBStepDatas[j].Step;
					wd.Walk = GetWD(blocks[i].BlockHead.walk, j, g_fileHead.step, g_direction);
					wd.Manual = 1;
					wd.Type = W_MANUAL;
					strcpy(wd.Result, "人工标记伤损");
					uint8_t degree = (wound & 0xF00) >> 8;
					if (degree == 4)
					{
						wd.Degree = WD_BREAK;
						memcpy(wd.Result, "人工标记折断", strlen("人工标记折断"));
					}
					else if (degree == 3)
					{
						wd.Degree = WD_SERIOUS;
						memcpy(wd.Result, "人工标记重伤", strlen("人工标记重伤"));
					}
					else if (degree == 2)
					{
						wd.Degree = WD_MEDIUM;
						memcpy(wd.Result, "人工标记轻发", strlen("人工标记轻发"));
					}
					else //if (degree == 1)
					{
						wd.Degree = WD_SMALL;
						memcpy(wd.Result, "人工标记轻伤", strlen("人工标记轻伤"));
					}
					wd.According.emplace_back(wd.Result);

					FillWound2(wd, blocks);

					uint32_t code = blocks[i].vBStepDatas[j].Wound.W_Code;
					uint8_t walongPlace = BCDToINT8(code & 0x0F);
					uint8_t vercPlace = BCDToINT8((code >> 4) & 0x0F);
					uint8_t state = BCDToINT8((code >> 8) & 0x0F);
					uint8_t thin = BCDToINT8((code >> 12) & 0x0F);
					uint8_t deal = BCDToINT8(blocks[i].vBStepDatas[j].Wound.Other & 0xFF);
					std::string strAccording = "长度位置： ";
					strAccording += GetValue(g_strWoundAlongPlace, walongPlace);
					strAccording += "\r\n截面位置： ";
					strAccording += GetValue(g_strWoundVercPlace, vercPlace);
					strAccording += "\r\n伤损状态： ";
					strAccording += GetValue(g_strWoundState, state);
					strAccording += "\r\n状态细化： ";
					strAccording += GetValue(g_strWoundThinning, thin);
					strAccording += "\r\n处理情况： ";
					strAccording += GetValue(g_strWoundDeal, deal);
					strAccording += "\r\n伤损程度： ";
					strAccording += g_strWoundDegree[degree];

					wd.According.emplace_back(strAccording);

					//根据标记，校正部分信息
					if (walongPlace == 2)
					{
						wd.Type = W_JOINT;
					}
					else if (walongPlace >= 3 && walongPlace <= 7 || walongPlace == 9)
					{
						wd.Type = W_SEWLR;
					}

					if (vercPlace == 0 || vercPlace == 1)
					{
						wd.Place = WP_TM;
					}
					else if (vercPlace == 2)
					{
						wd.Place = WP_HEAD_IN;
					}
					else if (vercPlace == 3)
					{
						wd.Place = WP_JAW_IN;
					}
					else if (vercPlace == 4)
					{
						wd.Place = WP_WAIST;
					}
					else if (vercPlace == 5)
					{
						wd.Place = WP_WAIST;
						wd.IsScrewHole = 1;
					}
					else if (vercPlace == 6)
					{
						wd.Place = WP_BOTTOM;
					}
					AddToWounds(vWounds, wd);
					g_vStepsInWound[blocks[i].IndexL2 + j] = 1;
				}
			}
		}
	}

	pm.Data = 0;

	std::vector<int16_t> vFRows;
	for (int i = 0; i < blocks.size(); ++i)
	{
		blocks[i].FRow = -1;
		for (int j = 0; j < blocks[i].vBStepDatas.size(); ++j)
		{
			int16_t frow = GetFRow(blocks[i].vBStepDatas[j]);
			int16_t grow = GetGRow(blocks[i].vBStepDatas[j]);

			blocks[i].vBStepDatas[j].FRow = frow;
			blocks[i].vBStepDatas[j].GRow = grow;
			vFRows.push_back(frow);
			if (blocks[i].FRow < 0 && frow > 0)
			{
				blocks[i].FRow = frow;
				blocks[i].FRow2 = frow;
			}
		}
		if (blocks[i].FRow > 0)
		{
			for (int j = blocks[i].vBStepDatas.size() - 1; j >= 0; --j)
			{
				if (blocks[i].vBStepDatas[j].FRow > 0)
				{
					blocks[i].FRow2 = blocks[i].vBStepDatas[j].FRow;
					break;
				}
			}
		}
		if (blocks[i].FRow < 0)
		{
			blocks[i].FRow = blocks[i].BlockHead.railH / 3;
			blocks[i].FRow2 = blocks[i].FRow;
		}
	}

	for (int i = 1; i < blocks.size(); ++i)
	{
		if (Abs(blocks[i].FRow2 - blocks[i - 1].FRow) >= 3)
		{
			memset(&pm, 0, sizeof(pm));
			pm.Mark = PM_SMART1;
			std::vector<int16_t>		vFFindRow;
			for (int j = blocks[i - 1].IndexL2 - blocks[0].IndexL2; j < blocks[i].IndexL2 + blocks[i].StepCount - blocks[0].IndexL2; ++j)
			{
				vFFindRow.emplace_back(vFRows[j]);
			}

			if (vFFindRow.size() >= 20)
			{
				bool bFind = false;
				for (int j = 30; j < vFFindRow.size() - 30; j += 30)
				{
					bool state1 = vFFindRow[j] > vFFindRow[j - 30] && vFFindRow[j] < vFFindRow[j + 30] && vFFindRow[j + 30] - vFFindRow[j - 30] >= 4 && vFFindRow[j + 7] - vFFindRow[j - 7] < 20;
					bool state2 = vFFindRow[j] < vFFindRow[j - 30] && vFFindRow[j] > vFFindRow[j + 30] && vFFindRow[j + 30] - vFFindRow[j - 30] > -20 && vFFindRow[j + 7] - vFFindRow[j - 7] <= -4;
					if (state1 || state2)
					{
						if (j < blocks[i - 1].BlockHead.row)
						{
							pm.Walk = GetWD(blocks[i - 1].BlockHead.walk, j, g_fileHead.step, g_direction);
							pm.Block = blocks[i - 1].Index;
							pm.Step = j;
							pm.Step2 = blocks[i - 1].IndexL2 + pm.Step;
							bFind = true;
						}
						else
						{
							pm.Walk = GetWD(blocks[i].BlockHead.walk, j - blocks[i - 1].BlockHead.row, g_fileHead.step, g_direction);
							pm.Block = blocks[i].Index;
							pm.Step = j - blocks[i - 1].BlockHead.row;
							pm.Step2 = blocks[i].IndexL2 + pm.Step;
							bFind = true;
						}

						int i1 = 0, i2 = 0, r1 = 0, r2 = 0;
						if (GetSlopeRow(vFFindRow, j, i1, r1, i2, r2) > 0)
						{
							pm.ScrewHoleCount = r1;//变坡之前的行高
							pm.GuideHoleCount = r2;//变坡之后的行高
							pm.Num[CH_d] = i1;	   //初始变坡点
							pm.Num[CH_e] = i2;	   //结束变坡点
							uint32_t s1 = pm.Step2 - i1;
							Pos pos = FindStepInBlock(s1, blocks, 0);
							pm.Block = pos.Block;
							pm.Step = pos.Step;
							pm.Step2 = s1;
							pm.Length = i2 + i1 + 1;
							AddToMarks(pm, vPMs);
							break;
						}
						else
						{
							pm.Block = blocks[i].Index;
							pm.Step = 0;
							pm.Step2 = blocks[i].IndexL2;
							AddToMarks(pm, vPMs);
						}
					}
				}

				if (!bFind)//未找到
				{
					pm.Walk = blocks[i].Walk;
					pm.Block = blocks[i].Index;
					pm.Step = 0;
					pm.Step2 = blocks[i].IndexL2;
					AddToMarks(pm, vPMs);
				}
			}
		}
	}

	for (int i = vPMs2.size() - 1; i >= mark2Count; --i)
	{
		for (int j = i - 1; j >= mark2Count; --j)
		{
			if (vPMs2[i].Step2 - vPMs2[j].Step2 > 500)
			{
				break;
			}

			if (vPMs2[j].Mark == vPMs2[i].Mark && vPMs2[i].Mark == PM_SELFDEFINE && vPMs2[i].Data >= 1 && vPMs2[i].Data <= 3 && vPMs2[j].Data >= 1 && vPMs2[j].Data <= 3)
			{
				for (int k = vPMs.size() - 1; k >= markCount; --k)
				{
					if (vPMs[k].Step2 == vPMs2[j].Step2 && vPMs[k].Manual == 1)
					{
						vPMs.erase(vPMs.begin() + k);
					}
				}

				vPMs2[j].IsOverlapped = 1;
			}
		}
	}
}

bool	IsInBackArea(uint32_t step, VBA& pBA, int backCount, int& backIndex)
{
	bool result = false;
	for (int i = 0; i < backCount; ++i)
	{
		if (step >= pBA[i].Pos0.Step2 && step <= pBA[i].Pos1.Step2)
		{
			result = true;
			backIndex = i;
			break;
		}
		else if (pBA[i].Pos1.Step2 > step)
		{
			break;
		}
	}
	return result;
}

bool	IsHaveBackPoint(uint32_t step1, uint32_t step2, VBA& pBA, int backCount, int& backIndex)
{
	bool result = false;
	for (int i = 0; i < backCount; ++i)
	{
		if (step2 >= pBA[i].Pos1.Step2 && step1 <= pBA[i].Pos1.Step2)
		{
			result = true;
			backIndex = i;
			break;
		}
		else if (pBA[i].Pos1.Step2 > step2)
		{
			break;
		}
	}
	return result;
}

void	GetRegions(uint16_t* datas, int length, int limit, std::vector<PointRegion>& regions)
{
	int lengthLimit = 2;

	VINT vFind;
	for (int i = 0; i < length; ++i)
	{
		if (datas[i] >= limit)
		{
			vFind.emplace_back(i);
		}
	}

	int count = vFind.size();
	if (count == 0)
		return;

	PointRegion pr;
	pr.Index = vFind[0];
	pr.Length = 1;
	int lastIndex = pr.Index;
	for (int i = 1; i < count; ++i)
	{
		if (vFind[i] - lastIndex <= 2)
		{
			pr.Length++;
			if (i == count - 1)
			{
				if (pr.Length >= lengthLimit)
				{
					regions.emplace_back(pr);
				}
			}
		}
		else
		{
			if (pr.Length >= lengthLimit)
			{
				regions.emplace_back(pr);
			}
			pr.Index = vFind[i];
			pr.Length = 1;
		}
		lastIndex = vFind[i];
	}
}

uint16_t	tempCountJoint[256] = { 0 };
uint8_t		jointLengthCH = 0;
int GetJointMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall, PM& pm, uint8_t iFRow)
{
	memset(tempCountJoint, 0, sizeof(uint16_t) * 256);

	VINT t_crF, t_crG;
	//略微考虑拼图不好的情况
	uint8_t iLoseF = GetCR(CH_F, iAStepSmall, iFRow - 3, iAStepBig, iFRow + 3, blocks, vCRs[CH_F], t_crF, -1, 2);
	uint8_t iLoseG = GetCR(CH_G, iAStepSmall, iFRow - 3, iAStepBig, iFRow + 3, blocks, vCRs[CH_G], t_crG, -1, 2);
	int iLose_Left = -1;
	int iLose_Right = -1;
	if (iLoseF)
	{
		for (int i = 0; i < t_crF.size() && iLose_Right <= 0; ++i)
		{
			uint32_t x1 = vCRs[CH_F][t_crF[i]].Step1, x2 = vCRs[CH_F][t_crF[i]].Step2;
			if (x2 - x1 <= 1)
			{
				continue;
			}
			VINT t_crG2;
			uint8_t bLoseG = GetCR(CH_G, x1, iFRow - 5, x2, iFRow + 5, blocks, vCRs[CH_G], t_crG2, -1, 2);
			for (int j = 0; j < t_crG2.size(); ++j)
			{
				if (vCRs[CH_G][t_crG2[j]].Step1 > iAStepBig || vCRs[CH_G][t_crG2[j]].Step2 < iAStepSmall)
				{
					continue;
				}
				int tb = 0, te = 0;
				GetOverlappedStep(x1, x2, vCRs[CH_G][t_crG2[j]].Step1, vCRs[CH_G][t_crG2[j]].Step2, tb, te);
				if (tb > 0)
				{
					iLose_Right = tb;
					iLose_Left = te;
					break;
				}
			}
		}
	}

	return 0;
}


//寻找厂焊大小通道中间分开的步进
uint16_t	tempCountCH[256] = { 0 };
uint8_t		chLengthCH = 0;
int		GetSewCHMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall)
{
	if (iAStepBig <= 5 || t_cr[CH_A1].size() + t_cr[CH_B1].size() + t_cr[CH_C].size() == 0)
	{
		iAStepBig = iAStepSmall + 10;
		return iAStepSmall + 5;
	}
	else if (iAStepSmall <= 5 || t_cr[CH_a1].size() + t_cr[CH_b1].size() + t_cr[CH_c].size() == 0)
	{
		iAStepSmall = iAStepBig - 10;
		return iAStepBig - 5;
	}

	Section sections[16];
	int aveLenNoCc;
	GetWaveLen2(vCRs, t_cr, sections, aveLenNoCc);

	for (int i = 0; i < CH_c; ++i)
	{
		if (sections[i].Flag > 0 && sections[i].Flag < 10)
		{
			iAStepSmall = min(iAStepSmall, sections[i].Start);
			iAStepBig = max(iAStepBig, sections[i].End);
		}
	}


	memset(tempCountCH, 0, sizeof(uint16_t) * 256);
	int iMaxStep = iAStepBig;
	int iMinStepFind = iAStepSmall;
	for (int i = 0; i < CH_D; ++i)
	{
		for (int j = 0; j < t_cr[i].size(); ++j)
		{
			CR& t = vCRs[i][t_cr[i][j]];
			for (int k = 0; k < t.Region.size(); ++k)
			{
				if (t.Region[k].step >= iAStepSmall && t.Region[k].row >= 11 && t.Region[k].row <= 15)
				{
					tempCountCH[t.Region[k].step - iAStepSmall]++;
					if (t.Region[k].step <= iMinStepFind && t.Channel % 4 != 0)
					{
						iMinStepFind = t.Region[k].step;
					}
				}
				if (t.Region[k].step >= iAStepBig && t.Region[k].row >= 11 && t.Region[k].row <= 15)
				{
					if (t.Region[k].step > iMaxStep && t.Channel % 4 == 0)
					{
						iMaxStep = t.Region[k].step;
					}
				}
			}
		}
	}
	chLengthCH = iMaxStep - iAStepSmall + 5;

	int minCount = 100, iminIndex = 0;
	int delt = iMinStepFind - iAStepSmall;
	for (int i = delt; i < iMaxStep - iAStepSmall; ++i)
	{
		if (tempCountCH[i] < minCount)
		{
			minCount = tempCountCH[i];
			iminIndex = i;
		}
	}

	int count = minCount;
	std::vector<Section> vv;
	Section v; v.Start = 0; v.End = 0; v.Flag = 0;
	int sz1 = 0;
	for (int i = iminIndex + 1; i < chLengthCH; ++i)
	{
		if (tempCountCH[i] == count)
		{
			if (i - v.End == 1)
			{
				v.End = i;
				if (i == chLengthCH - 1)
				{
					vv.push_back(v);
					v.Start = 0;
					v.End = 0;
				}
			}
			else
			{
				if (v.End - v.Start > 0)
				{
					vv.push_back(v);
					v.Start = 0;
					v.End = 0;
				}
				v.Start = i;
				v.End = i;
			}
			sz1++;
		}
		else
		{
			if (v.End - v.Start > 0)
			{
				vv.push_back(v);
				v.Start = 0;
				v.End = 0;
			}
		}
	}

	if (vv.size() > 0)
	{
		if (vv[vv.size() - 1].End == chLengthCH - 1)
		{
			vv.erase(vv.begin() + vv.size() - 1);
		}
	}

	if (vv.size() > 0)
	{
		std::sort(vv.begin(), vv.end(), [&](Section& s1, Section& s2) {return s1.End - s1.Start > s2.End - s2.Start; });
		return iAStepSmall + (vv[0].Start + vv[0].End + 1) / 2;
	}
	else
	{
		return iAStepSmall + iminIndex + (sz1 + 1) / 2;
	}
}


uint8_t	ParseSewCH(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs)
{
	int iStepSew2 = iStepSew;
	iStepSew = GetSewCHMiddleStep(blocks, vCRs, cr, t_cr, iAStepBig, iAStepSmall);
	if (iStepSew < 10)
	{
		iStepSew = iStepSew2;
	}

	//if (Abs(iStepSew2 - iStepSew) > 10)
	//{
	//	iStepSew = iStepSew2;
	//}
	Pos	 pos = FindStepInBlock(iStepSew, g_vBlockHeads, cr.Block - g_iBeginBlock);
	BLOCK blockHead = blocks[cr.Block - g_iBeginBlock].BlockHead;
	uint8_t isWound = 0;

	Wound_Judged wound;
	wound.Type = W_HS;
	wound.IsSew = 1;
	wound.Block = cr.Block;
	wound.Step = cr.Step;
	wound.Step2 = cr.Step1;
	memcpy(wound.Result, "厂焊焊缝核伤", 30);
	FillWound(wound, blockHead, head);
#pragma region 核伤
	for (int j = 0; j < 10; ++j)
	{
		int t_A = GetAChannelByBChannel(j);
		double t_angle = 0.1 * head.deviceP2.Angle[t_A].Refrac;
		int t_offset = head.deviceP2.Place[t_A] + blockHead.probOff[t_A];
		for (int k = 0; k < t_cr[j].size(); ++k)
		{
			CR& t = vCRs[j][t_cr[j][k]];
			SetSewLRHFlag(t, 1);
			if (t.IsUsed == 1)
			{
				continue;
			}
			if (t.Step1 < iStepSew - 10 || t.Step2 > iStepSew + 10)
			{
				continue;
			}
			if (j % 4 == 0)//大通道
			{
				GetCRInfo(t, DataA, blocks, t_cr[j].size() == 1);
				//正常位置出波
				if (t.Step1 >= iStepSew)
				{
					//A, B
					if (t.Channel < CH_C && (t.Row1 < g_iJawRow[railType] - 4 || t.Row2 > g_iJawRow[railType] + 4)/* && t.Info.MaxV >= g_iAmpl && t.Info.Shift >= 50*/)
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = head.step * (t.Step2 - t.Step1);
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "厂焊焊缝处%s出波，位移：%.1f大格，幅值%d，低于8行出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
					else if (t.Channel == CH_C && t.Row1 <= g_iJawRow[railType] - 3 && t.Region.size() >= 2/* && t.Info.Shift >= 25*/)
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = head.step * (t.Step2 - t.Step1);
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "厂焊焊缝处%s正常位置低于10行出波，位移：%.1f大格，幅值%d", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
				}
				//异常位置出波
				else
				{
					wound.Block = t.Block;
					wound.Step = t.Step;
					wound.Step2 = t.Step1;
					wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
					wound.SizeX = 3.0 * (t.Row2 - t.Row1);
					wound.SizeY = head.step * (t.Step2 - t.Step1);
					if (j < CH_C && t.Info.MaxH >= 250)
					{
						if ((carType == true && j == CH_A1) || (carType == false && j == CH_B1))
						{
							wound.Place = WP_HEAD_IN;
						}
						else
						{
							wound.Place = WP_HEAD_OUT;
						}
						wound.Degree = WD_SERIOUS;
						wound.SizeX = 1.5 * (t.Row2 - t.Row1);
						wound.SizeY = 0.5 * head.step * (t.Step2 - t.Step1);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "厂焊焊缝处%s小通道位置出波，位移：%.1f大格，幅值%d，大于5大格出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
					else if (j >= CH_C && t.Region.size() >= 2)
					{
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = head.step * (t.Step2 - t.Step1);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "厂焊焊缝处%s小通道位置出波，位移：%.1f大格，幅值%d", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
				}
			}
			else//小通道
			{
				//正常未知出波
				if (t.Step1 <= iStepSew)//正常位置出波
				{
					if (GetCRInfo(t, DataA, blocks, t_cr[j].size() == 1) && t.Info.MaxV >= g_iAmpl && t.Info.Shift >= 25 || t.Row1 <= g_iJawRow[railType] - 3)
					{
						if (j < CH_C && (t.Row1 < g_iJawRow[railType] - 4 || t.Row2 > g_iJawRow[railType] + 4))
						{
							wound.Block = t.Block;
							wound.Step = t.Step;
							wound.Step2 = t.Step1;
							wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
							if ((carType == true && j == CH_A1) || (carType == false && j == CH_B1))
							{
								wound.Place = WP_HEAD_IN;
							}
							else
							{
								wound.Place = WP_HEAD_OUT;
							}
							wound.Degree = WD_SERIOUS;
							wound.SizeX = 1.5 * (t.Row2 - t.Row1);
							wound.SizeY = 0.5 * head.step * (t.Step2 - t.Step1);
							if (bShowWoundDetail)
							{
								sprintf(tempAccording, "厂焊焊缝处%s出波，位移：%.1f大格，幅值%d，大于5大格出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
								wound.According.emplace_back(tempAccording);
							}
							AddWoundData(wound, t);
						}
						else if (j >= CH_C && t.Row1 <= g_iJawRow[railType] - 3 && t.Region.size() >= 2)
						{
							wound.Block = t.Block;
							wound.Step = t.Step;
							wound.Step2 = t.Step1;
							wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
							wound.Place = WP_HEAD_MID;
							wound.Degree = WD_SERIOUS;
							wound.SizeX = 3.0 * (t.Row2 - t.Row1);
							wound.SizeY = head.step * (t.Step2 - t.Step1);
							if (bShowWoundDetail)
							{
								sprintf(tempAccording, "厂焊焊缝处%s出波，位移：%.1f大格，幅值%d，低于10行出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
								wound.According.emplace_back(tempAccording);
							}
							AddWoundData(wound, t);
						}
					}
				}
				else//异常位置出波
				{
					GetCRInfo(t, DataA, blocks, t_cr[j].size() == 1);
					if (j == CH_c && t.Region.size() >= 2/* && t.Info.MaxV >= g_iAmpl && t.Info.Shift >= 25*/)
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
						wound.IsSew = 1;
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = head.step * (t.Step2 - t.Step1);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "厂焊焊缝处%s大通道位置出波，位移：%.1f大格，幅值%d，大通道位置出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
					else if (j < CH_c /* && t.Info.MaxV >= g_iAmpl && t.Info.Shift >= 50 */)
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
						wound.IsSew = 1;
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = head.step * (t.Step2 - t.Step1);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "厂焊焊缝处%s大通道位置出波，位移：%.1f大格，幅值%d，大通道位置出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
				}
			}
		}
	}
#pragma endregion

	VINT crD, crE, crF, crG;
#pragma region 螺孔
	int screwIndexes[] = { -3, -2, -1, 1, 2, 3 };
	bool isExistScrewHoles[6] = { false, false , false ,false ,false ,false };
	uint8_t count = 0;
	for (int i = 0; i < 3; ++i)
	{
		VINT crF, crG, crF2, crG2;
		int tr = iAStepSmall + dScrewDistance[railType][i] / head.step - g_screwholeRange;
		int tl = iAStepSmall + dScrewDistance[railType][i] / head.step + g_screwholeRange;
		bool bFind = GetCR(CH_F, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF, -1, 2) ||
			GetCR(CH_G, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG, -1, 2);

		bool bLose = GetCR(CH_F, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_F], crF2, -1, 2) ||
			GetCR(CH_G, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_G], crG2, -1, 2);
		crD.clear(); crE.clear();
		bool bFindD = GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 2);
		bool bFindE = GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 2);
		if (bFind && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHole(head, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else if ((bFindD || bFindE) && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHoleNoFG(head, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else
		{
			isExistScrewHoles[i] = false;
		}
	}
	for (int i = 3; i < 6; ++i)
	{
		VINT crF, crG, crF2, crG2;
		int tr = iAStepBig + dScrewDistance[railType][i] / head.step - g_screwholeRange;
		int tl = iAStepBig + dScrewDistance[railType][i] / head.step + g_screwholeRange;
		bool bFind = GetCR(CH_F, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF, -1, 2) ||
			GetCR(CH_G, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG, -1, 2);

		bool bLose = GetCR(CH_F, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_F], crF2, -1, 2) ||
			GetCR(CH_G, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_G], crG2, -1, 2);
		crD.clear(); crE.clear();
		bool bFindD = GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 2);
		bool bFindE = GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 2);
		if (bFind && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHole(head, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else if ((bFindD || bFindE) && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHoleNoFG(head, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else
		{
			isExistScrewHoles[i] = false;
		}
	}
#pragma endregion

	crD.clear();
	crE.clear();
	bool bFindD = GetCR(CH_D, iStepSew - 20, 0, iStepSew + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_D], crD);
	bool bFindE = GetCR(CH_E, iStepSew - 20, 0, iStepSew + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_E], crE);
	bool bFindF = GetCR(CH_F, iStepSew - 20, 0, iStepSew + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_F], crF);
	bool bFindG = GetCR(CH_G, iStepSew - 20, 0, iStepSew + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_G], crG);

	bFindD = RemoveScrewHoleCR(vCRs[CH_D], crD);
	bFindE = RemoveScrewHoleCR(vCRs[CH_E], crE);
	bFindF = RemoveScrewHoleCR(vCRs[CH_F], crF);
	bFindG = RemoveScrewHoleCR(vCRs[CH_G], crG);

	VINT crF2, crG2;
	bool bLoseF = GetCR(CH_F, iStepSew - 10, iFRow - 5, iStepSew + 10, iFRow + 5, blocks, vCRs[CH_F], crF2);
	bool bLoseG = GetCR(CH_G, iStepSew - 10, iFRow - 5, iStepSew + 10, iFRow + 5, blocks, vCRs[CH_G], crG2);

	if (bFindD || bFindE || bFindF || bFindG || wound.vCRs.size() > 0/* || bLoseF || bLoseG*/)
	{
		wound.Place = WP_HEAD_MID;
		AddWoundData(wound, vCRs[CH_D], crD);
		AddWoundData(wound, vCRs[CH_E], crE);
		AddWoundData(wound, vCRs[CH_F], crF);
		AddWoundData(wound, vCRs[CH_G], crG);
		//AddWoundData(wound, vCRs[CH_F], crF2);
		//AddWoundData(wound, vCRs[CH_G], crG2);
		SetUsedFlag(vCRs[CH_D], crD, 1);
		SetUsedFlag(vCRs[CH_E], crE, 1);
		SetUsedFlag(vCRs[CH_F], crF, 1);
		SetUsedFlag(vCRs[CH_G], crG, 1);
		//SetUsedFlag(vCRs[CH_F], crF2, 1);
		//SetUsedFlag(vCRs[CH_G], crG2, 1);
		AddToWounds(vWounds, wound);
		wound.SetEmpty();
		wound.Block = cr.Block;
		wound.Step = cr.Step;
		wound.Step2 = cr.Step1;
	}

	if (bLoseF || bLoseG)
	{
		for (int i = 0; i < crF2.size(); ++i)
		{
			ParseHorizonalCrack(head, DataA, blocks, vCRs, vCRs[CH_F][crF2[i]], g_direction, carType, iFRow, vWounds, 0, 1);
		}
		for (int i = 0; i < crG2.size(); ++i)
		{
			ParseHorizonalCrack(head, DataA, blocks, vCRs, vCRs[CH_G][crG2[i]], g_direction, carType, iFRow, vWounds, 0, 1);
		}
	}

	crD.clear();	crE.clear();	crF.clear();	crG.clear();
	bFindD = GetCR(CH_D, iStepSew2 - 20, 2 * g_iJawRow[railType] + 1, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_D], crD);
	bFindE = GetCR(CH_E, iStepSew2 - 20, 2 * g_iJawRow[railType] + 1, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_E], crE);
	bFindF = GetCR(CH_F, iStepSew2 - 20, 2 * g_iJawRow[railType] + 1, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_F], crF);
	bFindG = GetCR(CH_G, iStepSew2 - 20, 2 * g_iJawRow[railType] + 1, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_G], crG);
	bFindD = RemoveScrewHoleCR(vCRs[CH_D], crD);
	bFindE = RemoveScrewHoleCR(vCRs[CH_E], crE);
	bFindF = RemoveScrewHoleCR(vCRs[CH_F], crF);
	bFindG = RemoveScrewHoleCR(vCRs[CH_G], crG);

	bFindD = RemoveCRByRow1Limit(vCRs[CH_D], crD, 2 * g_iJawRow[railType]);
	bFindE = RemoveCRByRow1Limit(vCRs[CH_E], crE, 2 * g_iJawRow[railType]);
	bFindF = RemoveCRByRow1Limit(vCRs[CH_F], crF, 2 * g_iJawRow[railType]);
	bFindG = RemoveCRByRow1Limit(vCRs[CH_G], crG, 2 * g_iJawRow[railType]);
	if (bFindD || bFindE || bFindF || bFindG)
	{
		Wound_Judged wound2;
		wound2.Block = pos.Block;
		wound2.Step = pos.Step;
		wound2.Step2 = pos.Step2;
		wound2.Type = W_SEWCH;
		wound2.Place = WP_WAIST;
		wound2.IsSew = 1;

		AddWoundData(wound2, vCRs[CH_D], crD);
		AddWoundData(wound2, vCRs[CH_E], crE);
		AddWoundData(wound2, vCRs[CH_F], crF);
		AddWoundData(wound2, vCRs[CH_G], crG);

		AddToWounds(vWounds, wound2);
	}



#pragma region 现场焊轨底端角反射
	crD.clear(); crE.clear();
	bFindD = GetCR(CH_D, iStepSew2 - 10, iFRow - 2, iStepSew2 + 10, iFRow + 10, blocks, vCRs[CH_D], crD);
	bFindE = GetCR(CH_E, iStepSew2 - 10, iFRow - 2, iStepSew2 + 10, iFRow + 10, blocks, vCRs[CH_E], crE);

	SetSewLRHFlag(vCRs[CH_D], crD, 1);
	SetSewLRHFlag(vCRs[CH_E], crE, 1);

	Wound_Judged wound2;
	wound2.Block = pos.Block;
	wound2.Step = pos.Step;
	wound2.Step2 = pos.Step2;
	wound2.Type = W_BOTTOM_EX;
	wound2.Place = WP_WAIST;
	wound2.IsSew = 1;

	for (int i = 0; i < crD.size(); ++i)
	{
		if (vCRs[CH_D][crD[i]].Step2 >= iStepSew2 - 2)
		{
			SetUsedFlag(vCRs[CH_D][crD[i]], 1);
		}
		else
		{
			AddWoundData(wound2, vCRs[CH_D][crD[i]]);
		}
	}

	for (int i = 0; i < crE.size(); ++i)
	{
		if (vCRs[CH_E][crE[i]].Step1 <= iStepSew2 + 2)
		{
			SetUsedFlag(vCRs[CH_E][crE[i]], 1);
		}
		else
		{
			AddWoundData(wound2, vCRs[CH_E][crE[i]]);
		}
	}
	if (wound2.vCRs.size() > 0)
	{
		AddToWounds(vWounds, wound2);
	}

#pragma endregion

	for (int j = 0; j < 10; ++j)
	{
		for (int k = 0; k < t_cr[j].size(); ++k)
		{
			if (vCRs[j][t_cr[j][k]].Step1 < iStepSew - 15 || vCRs[j][t_cr[j][k]].Step2 > iStepSew + 15)
			{
				continue;
			}
			SetUsedFlag(vCRs[j][t_cr[j][k]], 1);
		}
	}

	return count;
}

//寻找铝热大小通道中间分开的步进

uint16_t	tempCountLRHS[256] = { 0 };//小通道
uint16_t	tempCountLRHB[256] = { 0 };//大通道
uint16_t	tempCountLRH[256] = { 0 };//大小通道之和
uint16_t	tempCountLRH2[256] = { 0 };//大小通道之和
uint8_t		chLengthLRH = 0;
//寻找铝热大小通道中间分开的步进
int		GetSewLRHMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall, PM& pm)
{
	if (pm.Size >= 150)
	{
		return (iAStepSmall + iAStepBig) / 2;
	}

	if (iAStepBig <= 5)
	{
		iAStepBig = iAStepSmall + 20;
		return iAStepSmall + 10;
	}
	else if (iAStepSmall == -5)
	{
		iAStepSmall = iAStepBig - 20;
		return iAStepBig - 10;
	}

	chLengthLRH = iAStepBig - iAStepSmall + 1;
	memset(tempCountLRHS, 0, sizeof(uint16_t) * 256);
	memset(tempCountLRHB, 0, sizeof(uint16_t) * 256);
	memset(tempCountLRH, 0, sizeof(uint16_t) * 256);
	memset(tempCountLRH2, 0, sizeof(uint16_t) * 256);
	int iMaxStep = iAStepBig;
	int iMinStepFind = iAStepBig;
	for (int i = 0; i < CH_D; ++i)
	{
		for (int j = 0; j < t_cr[i].size(); ++j)
		{
			CR& t = vCRs[i][t_cr[i][j]];
			if (t.Region.size() < 3)
			{
				continue;
			}
			for (int k = 0; k < t.Region.size(); ++k)
			{
				if (t.Region[k].step >= iAStepSmall && t.Region[k].row >= 11 && t.Region[k].row <= 15)
				{
					if (i == CH_A1 || i == CH_A2 || i == CH_B1 || i == CH_B2 || i == CH_C)
					{
						if (t.Region[k].step - iAStepSmall < 0 || t.Region[k].step - iAStepSmall > 256)
						{
							int x = 0;
							++x;
						}
						tempCountLRHB[t.Region[k].step - iAStepSmall]++;
						if (t.Region[k].step >= iAStepBig)
						{
							iMaxStep = t.Region[k].step;
						}
					}
					else
					{
						tempCountLRHS[t.Region[k].step - iAStepSmall]++;
						if (t.Region[k].step <= iMinStepFind)
						{
							iMinStepFind = t.Region[k].step;
						}
					}
				}

				if (t.Region[k].step >= iAStepSmall)
				{
					tempCountLRH2[t.Region[k].step - iAStepSmall] ++;
				}
			}
		}
	}

	int minCount = 100, iminIndex = 0;
	int delt = iMinStepFind - iAStepSmall;
	for (int i = 0; i < 256; ++i)
	{
		tempCountLRH[i] = tempCountLRHB[i] + tempCountLRHS[i];
		if (tempCountLRH[i] < minCount)
		{
			minCount = tempCountLRH[i];
			iminIndex = i;
		}
	}

	/*
	std::vector<PointRegion> v1, v2, v3;
	GetRegions(tempCountLRHB, 256, 2, v1);
	GetRegions(tempCountLRHS, 256, 2, v2);
	*/

	//小通道出波点
	bool bFindS1 = false;
	int s1 = delt;
	int imaxIndex = delt;
	int iMaxCount = minCount;
	for (int i = delt; i < chLengthLRH; ++i)
	{
		if (tempCountLRHS[i] >= iMaxCount && tempCountLRHS[i] >= 3)
		{
			s1 = i;
			iMaxCount = tempCountLRHS[i];
			bFindS1 = true;
		}
		if (bFindS1 == true && (tempCountLRHS[i + 1] <= tempCountLRHS[i] - 3) || tempCountLRHS[i + 1] == minCount)
		{
			break;
		}
	}

	if (!bFindS1)
	{
		return (iAStepSmall + iAStepBig) / 2;
	}

	//大通道出波区域
	bool bFindS2 = false;
	int s2 = delt;
	iMaxCount = minCount;
	for (int i = iminIndex + 15; i < s1 + 30; ++i)
	{
		if (tempCountLRHB[i] >= iMaxCount && tempCountLRHB[i] >= 2)
		{
			s2 = i;
			iMaxCount = tempCountLRHB[i];
			bFindS2 = true;
		}
		if (tempCountLRHB[i + 1] <= minCount && tempCountLRHB[i] > minCount && iMaxCount >= minCount + 2 && s2 - s1 >= 5)
		{
			break;
		}
	}

	if (bFindS1 && bFindS2)
	{
		int offset = (s2 + s1) >> 1;
		int iBegin = offset - 3 > 0 ? offset - 3 : 0;
		minCount = tempCountLRH2[iBegin];
		iminIndex = iBegin;
		for (int i = iBegin; i < offset + 3; ++i)
		{
			if (tempCountLRH2[i] < minCount)
			{
				minCount = tempCountLRH2[i];
				iminIndex = i;
			}
		}
		int mid = iAStepSmall + iminIndex;
		int hold = 0;
		for (int i = mid + 1; i < offset + 3; ++i)
		{
			if (tempCountLRH2[i] == tempCountLRH2[mid])
			{
				hold++;
			}
			else
			{
				break;
			}
		}
		mid += hold;
#ifdef _DEBUG
		Pos pos = FindStepInBlock(mid, blocks, 0);
#endif
		iAStepBig = iAStepSmall + s2;
		iAStepSmall = iAStepSmall + s1;
		return mid;
	}
	else
	{
		return (iAStepSmall + iAStepBig) / 2;
	}
}


void	GetSewLRHMainCR(uint8_t channel, VCR* vCRs, VINT* t_cr, int& stepSew, uint8_t& iJawRow, int& mainCRIndex)
{
	mainCRIndex = -1;
	int crSize = 0;
	bool bFront = (channel % 4 == 0);
	if (bFront)//A,B,C
	{
		for (int i = 0; i < t_cr[channel].size(); ++i)
		{
			if (channel < CH_C && vCRs[channel][t_cr[channel][i]].Region.size() < 3)
			{
				continue;
			}
			if (channel < CH_C && vCRs[channel][t_cr[channel][i]].Row1 > iJawRow + 3 || vCRs[channel][t_cr[channel][i]].Row2 < iJawRow - 3 ||
				channel >= CH_C && vCRs[channel][t_cr[channel][i]].Row2 <= iJawRow
				)
			{
				continue;
			}

			if (vCRs[channel][t_cr[channel][i]].Step2 > stepSew)
			{
				mainCRIndex = t_cr[channel][i];
				crSize = vCRs[channel][t_cr[channel][i]].Region.size();
				break;
			}
		}
	}
	else//a, b, c
	{
		for (int i = t_cr[channel].size() - 1; i >= 0; --i)
		{
			if (channel < CH_C && vCRs[channel][t_cr[channel][i]].Region.size() < 3)
			{
				continue;
			}
			if (channel < CH_C && vCRs[channel][t_cr[channel][i]].Row1 > iJawRow + 3 || vCRs[channel][t_cr[channel][i]].Row2 < iJawRow - 3 ||
				channel >= CH_C && vCRs[channel][t_cr[channel][i]].Row2 <= iJawRow
				)
			{
				continue;
			}
			if (vCRs[channel][t_cr[channel][i]].Step1 < stepSew)
			{
				mainCRIndex = t_cr[channel][i];
				crSize = vCRs[channel][t_cr[channel][i]].Region.size();
				break;
			}
		}
	}
}


int rowLRHL[10];
int rowLRHH[10];
int countLRH[10];
uint8_t	ParseSewLRH(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs)
{
	for (int i = 0; i < 10; ++i)
	{
		rowLRHH[i] = 0;
		rowLRHL[i] = VALID_ROW + 1;
		countLRH[i] = 0;
	}
	for (int i = 0; i < CH_D; ++i)
	{
		uint8_t chA = GetAChannelByBChannel(i);
		for (int j = 0; j < t_cr[i].size(); ++j)
		{
			CR &t = vCRs[i][t_cr[i][j]];
			t.IsSew = 1;
			if (t.Step1 > iStepSew + 16 || t.Step2 < iStepSew - 16)
				continue;

			countLRH[chA] += t.Region.size();
			if (t.Row1 < rowLRHL[chA])	rowLRHL[chA] = t.Row1;
			if (t.Row2 > rowLRHH[chA])	rowLRHH[chA] = t.Row2;
		}
	}

	int iStepSew2 = iStepSew;
	//iStepSew = GetSewMiddleStep(blocks, vCRs, cr, t_cr, iAStepBig, iAStepSmall);
	Pos	 pos = FindStepInBlock(iStepSew, g_vBlockHeads, cr.Block - g_iBeginBlock);
	uint8_t isWound = 0;
	Wound_Judged wound;

#pragma region 核伤
	uint8_t isSew = 2;
	wound.IsSew = isSew;
	wound.Type = W_HS;
	wound.Block = cr.Block;
	wound.Step = cr.Step;
	wound.Step2 = cr.Step1;
	memcpy(wound.Result, "铝热焊缝核伤", 30);

	uint8_t iJawRow = g_iJawRow[railType];

	//最小判伤点数
	int iSmallestSizeAaBb = 3;
	int iSmallestSizeCc = 2;

	const int order[] = { CH_A1, CH_A2, CH_a1, CH_a2, CH_B1, CH_B2, CH_b1, CH_b2, CH_c, CH_C };
	int  mainCRIndexes[10] = { 0 };
	for (int i = 0; i < 10; ++i)
	{
		int j = order[i];
		mainCRIndexes[j] = -1;
		int t_A = GetAChannelByBChannel(j);
		double t_angle = 0.1 * head.deviceP2.Angle[t_A].Refrac;
		BLOCK blockHead = blocks[cr.Block - g_iBeginBlock].BlockHead;
		int t_offset = head.deviceP2.Place[t_A] + blockHead.probOff[t_A];

		int mainCRIndex = -1;
		GetSewLRHMainCR(j, vCRs, t_cr, iStepSew2, g_iJawRow[railType & 0x03], mainCRIndex);
		mainCRIndexes[j] = mainCRIndex;
		if (mainCRIndex >= 0)
		{
			SetSewLRHFlag(vCRs[j][mainCRIndex], isSew);
			SetUsedFlag(vCRs[j][mainCRIndex], 1);
			int mains1 = vCRs[j][mainCRIndex].Step1 - 3, mains2 = vCRs[j][mainCRIndex].Step2 + 3;
			//有主出波
			if (j % 4 == 0)//A, B, C 0 , 4, 8，大通道
			{
				for (int k = 0; k < t_cr[j].size(); ++k)
				{
					if (t_cr[j][k] == mainCRIndex)
					{
						continue;
					}

					CR &t = vCRs[j][t_cr[j][k]];
					SetSewLRHFlag(t, isSew);
					if (t.Step1 > iStepSew + SewLRHLegalStep || t.Step2 < iStepSew2 - SewLRHLegalStep)
					{
						SetSewLRHFlag(t, 0);
					}

					SetSewLRHFlag(t, isSew);
					if (t.IsUsed == 1)
					{
						continue;
					}
					SetUsedFlag(t, 1);
					GetCRInfo(t, DataA, blocks, t_cr[j].size() == 1);
					bool iswound = false;

					if (t.Step1 > mains2 || t.Step2 < mains1 /* && t.Info.MaxV >= g_iAmpl*/)//异常位置出波
					{
						if (true)//if (t.Info.MaxV >= g_iAmpl)
						{
							FillWound(wound, blockHead, head);
							wound.Block = t.Block;
							wound.Step = t.Step;
							wound.Step2 = t.Step1;
							wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
							wound.Degree = WD_SERIOUS;
							if (bShowWoundDetail)
							{
								sprintf(tempAccording, "铝热焊缝处%s出波，位移：%.1f大格，幅值%d", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
								wound.According.emplace_back(tempAccording);
							}
							if (j == CH_C && t.Region.size() >= iSmallestSizeCc/* && t.Info.Shift >= 25*/)
							{
								wound.Place = WP_HEAD_MID;
								wound.SizeX = 3.0 * (t.Row2 - t.Row1);
								wound.SizeY = head.step * (t.Step2 - t.Step1);
								AddWoundData(wound, t);
							}
							else if (j < CH_C/* && t.Info.Shift >= 30*/ && t.Region.size() >= iSmallestSizeAaBb)
							{
								if ((carType == true && j == CH_A1) || (carType == false && j == CH_B1))
								{
									wound.Place = WP_HEAD_IN;
								}
								else
								{
									wound.Place = WP_HEAD_OUT;
								}
								wound.SizeX = 1.5 * (t.Row2 - t.Row1);
								wound.SizeY = 0.5 * head.step * (t.Step2 - t.Step1);
								AddWoundData(wound, t);
							}
						}
					}
					else if (j == CH_C && t.Region.size() >= iSmallestSizeCc/*&& t.Info.Shift >= 25 */ && (vCRs[j][t_cr[j][k]].Row1 < g_iJawRow[railType] - 3 || vCRs[j][t_cr[j][k]].Row2 < g_iJawRow[railType] - 1))
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "铝热焊缝处%s出波，位移：%.1f大格，幅值%d", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						//wound.Type = W_HEAD_HS_MID;
						wound.Place = WP_HEAD_MID;
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = head.step * (t.Step2 - t.Step1);
						AddWoundData(wound, t);
					}
				}
			}
			else //小通道
			{
				for (int k = 0; k < t_cr[j].size(); ++k)
				{
					if (t_cr[j][k] == mainCRIndex)
					{
						continue;
					}
					CR &t = vCRs[j][t_cr[j][k]];
					SetSewLRHFlag(t, isSew);
					if (t.IsUsed == 1 || t.Step1 > iStepSew + SewLRHLegalStep || t.Step2 < iStepSew2 - SewLRHLegalStep)
					{
						continue;
					}
					SetUsedFlag(t, 1);
					GetCRInfo(t, DataA, blocks, t_cr[j].size() == 1);
					if (t.Step1 > mains2 || t.Step2 < mains1 /* && t.Info.MaxV >= g_iAmpl*/)//非正常位置出波
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
						wound.Degree = WD_SERIOUS;
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "铝热焊缝处%s出波，位移：%.1f大格，幅值%d", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						if (j == CH_c && t.Region.size() >= iSmallestSizeCc /* && t.Info.Shift >= 25 */)
						{
							wound.Place = WP_HEAD_MID;
							wound.SizeX = 3.0 * (t.Row2 - t.Row1);
							wound.SizeY = head.step * (t.Step2 - t.Step1);
							AddWoundData(wound, t);
						}
						else if (j < CH_c && t.Region.size() >= iSmallestSizeAaBb/*&& t.Info.Shift >= 30*/)
						{
							if ((carType == true && j == CH_A1) || (carType == false && j == CH_B1))
							{
								wound.Place = WP_HEAD_IN;
							}
							else
							{
								wound.Place = WP_HEAD_OUT;
							}
							wound.SizeX = 1.5 * (t.Row2 - t.Row1);
							wound.SizeY = 0.5 * head.step * (t.Step2 - t.Step1);
							AddWoundData(wound, t);
						}
					}
					else if (j == CH_c && t.Region.size() >= iSmallestSizeCc && t.Row1 < g_iJawRow[railType] - 3/* && t.Info.Shift >= 25*/)
					{
						FillWound(wound, blockHead, head);
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, head.step, g_direction);
						wound.Degree = WD_SERIOUS;
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "铝热焊缝处%s出波，位移：%.1f大格，幅值%d", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						wound.Place = WP_HEAD_MID;
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = head.step * (t.Step2 - t.Step1);
						AddWoundData(wound, t);
					}
				}
			}
		}
	}

	for (int i = 0; i < CH_D; ++i)
	{
		if (i < CH_C)
		{
			if (i % 2 == 0 && mainCRIndexes[i] < 0)
			{
				if (i == CH_A1 || i == CH_B1)
				{
					LCR lcr(i, iStepSew, iStepSew + 8, iJawRow - 3, iJawRow + 3, isSew, 0, 0, 0);
					wound.vLCRs.emplace_back(lcr);
				}
				else
				{
					LCR lcr(i, iStepSew - 8, iStepSew, iJawRow - 3, iJawRow + 3, isSew, 0, 0, 0);
					wound.vLCRs.emplace_back(lcr);
				}
				sprintf(tempAccording, "焊缝处通道%c未出现明显回波", ChannelNames[GetAChannelByBChannel(i)]);
				wound.According.push_back(tempAccording);
				if (t_cr[i].size() > 0)
				{
					SetUsedFlag(vCRs[i], t_cr[i], 1);
					SetSewLRHFlag(vCRs[i], t_cr[i], isSew);
					AddWoundData(wound, vCRs[i], t_cr[i]);
				}
			}
		}
		else if (i >= CH_C)
		{
			if (t_cr[i].size() > 0 && mainCRIndexes[i] < 0)
			{
				SetSewLRHFlag(vCRs[i], t_cr[i], isSew);
				AddWoundData(wound, vCRs[i], t_cr[i]);
			}
		}
	}

	//if (wound.vCRs.size() > 0)
	//{
	//	AddToWounds(vWounds, wound);
	//	wound.SetEmpty();
	//}
#pragma endregion

	VINT crD, crE, crF, crG;
#pragma region 螺孔
	int screwIndexes[] = { -3, -2, -1, 1, 2, 3 };
	bool isExistScrewHoles[6] = { false, false , false ,false ,false ,false };
	uint8_t count = 0;
	for (int i = 0; i < 3; ++i)
	{
		VINT crF, crG, crF2, crG2;
		int tr = iAStepSmall + dScrewDistance[railType][i] / head.step - g_screwholeRange;
		int tl = iAStepSmall + dScrewDistance[railType][i] / head.step + g_screwholeRange;
		bool bFind = GetCR(CH_F, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF, -1, 2) ||
			GetCR(CH_G, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG, -1, 2);

		bool bLose = GetCR(CH_F, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_F], crF2, -1, 2) ||
			GetCR(CH_G, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_G], crG2, -1, 2);
		crD.clear(); crE.clear();
		bool bFindD = GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 2);
		bool bFindE = GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 2);
		if (bFind && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHole(head, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else if ((bFindD || bFindE) && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHoleNoFG(head, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else
		{
			isExistScrewHoles[i] = false;
		}
	}
	for (int i = 3; i < 6; ++i)
	{
		VINT crF, crG, crF2, crG2;
		int tr = iAStepBig + dScrewDistance[railType][i] / head.step - g_screwholeRange;
		int tl = iAStepBig + dScrewDistance[railType][i] / head.step + g_screwholeRange;
		bool bFind = GetCR(CH_F, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF, -1, 2) ||
			GetCR(CH_G, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG, -1, 2);

		bool bLose = GetCR(CH_F, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_F], crF2, -1, 2) ||
			GetCR(CH_G, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_G], crG2, -1, 2);
		crD.clear(); crE.clear();
		bool bFindD = GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 2);
		bool bFindE = GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 2);
		if (bFind && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHole(head, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else if ((bFindD || bFindE) && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHoleNoFG(head, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else
		{
			isExistScrewHoles[i] = false;
		}
	}
#pragma endregion

	crD.clear();
	crE.clear();

	if (pos.Block - g_iBeginBlock < 0 || pos.Block - g_iBeginBlock >= blocks.size())
	{

	}
	else
	{
		iFRow = (std::min)((int16_t)(blocks[pos.Block - g_iBeginBlock].vBStepDatas[pos.Step].FRow), (int16_t)(blocks[pos.Block - g_iBeginBlock].vBStepDatas[pos.Step].GRow));
	}

	bool bFindD = GetCR(CH_D, iStepSew2 - 20, 0, iStepSew2 + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_D], crD);
	bool bFindE = GetCR(CH_E, iStepSew2 - 20, 0, iStepSew2 + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_E], crE);
	bool bFindF = GetCR(CH_F, iStepSew2 - 20, 0, iStepSew2 + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_F], crF);
	bool bFindG = GetCR(CH_G, iStepSew2 - 20, 0, iStepSew2 + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_G], crG);
	bFindD = RemoveScrewHoleCR(vCRs[CH_D], crD);
	bFindE = RemoveScrewHoleCR(vCRs[CH_E], crE);
	bFindF = RemoveScrewHoleCR(vCRs[CH_F], crF);
	bFindG = RemoveScrewHoleCR(vCRs[CH_G], crG);

	VINT crF2, crG2;
	bool bLoseF = GetCR(CH_F, iStepSew2 - 10, iFRow - 5, iStepSew2 + 10, iFRow + 5, blocks, vCRs[CH_F], crF2);
	bool bLoseG = GetCR(CH_G, iStepSew2 - 10, iFRow - 5, iStepSew2 + 10, iFRow + 5, blocks, vCRs[CH_G], crG2);

	if (bFindD || bFindE || bFindF || bFindG || wound.vCRs.size() > 0 || wound.According.size() > 0/* || bLoseF || bLoseG*/)
	{
		wound.IsSew = isSew;
		wound.Place = WP_HEAD_MID;
		AddWoundData(wound, vCRs[CH_D], crD);
		AddWoundData(wound, vCRs[CH_E], crE);
		AddWoundData(wound, vCRs[CH_F], crF);
		AddWoundData(wound, vCRs[CH_G], crG);
		//AddWoundData(wound, vCRs[CH_F], crF2);
		//AddWoundData(wound, vCRs[CH_G], crG2);

		if (bFindF || bFindG)
		{
			bool bFindWound = false;
			for (int i = 0; i < crF.size(); ++i)
			{
				int c1 = vWounds.size();
				ParseHorizonalCrack(head, DataA, blocks, vCRs, vCRs[CH_F][crF[i]], g_direction, carType, iFRow, vWounds, 0, 2);
				int c2 = vWounds.size();
				if (c2 > c1)
				{
					bFindWound = true;
					memcpy(wound.Result, vWounds[vWounds.size() - 1].Result, 60);
					wound.Type = vWounds[vWounds.size() - 1].Type;
					vWounds.pop_back();
					break;
				}
			}
			if (bFindWound == false)
			{
				for (int i = 0; i < crG.size(); ++i)
				{
					int c1 = vWounds.size();
					ParseHorizonalCrack(head, DataA, blocks, vCRs, vCRs[CH_G][crG[i]], g_direction, carType, iFRow, vWounds, 0, 2);
					int c2 = vWounds.size();
					if (c2 > c1)
					{
						bFindWound = true;
						memcpy(wound.Result, vWounds[vWounds.size() - 1].Result, 60);
						wound.Type = vWounds[vWounds.size() - 1].Type;
						vWounds.pop_back();
						break;
					}
				}
			}
		}

		SetUsedFlag(vCRs[CH_D], crD, 1);
		SetUsedFlag(vCRs[CH_E], crE, 1);
		SetUsedFlag(vCRs[CH_F], crF, 1);
		SetUsedFlag(vCRs[CH_G], crG, 1);
		//SetUsedFlag(vCRs[CH_F], crF2, 1);
		//SetUsedFlag(vCRs[CH_G], crG2, 1);

		AddToWounds(vWounds, wound);
		wound.SetEmpty();
		wound.Block = cr.Block;
		wound.Step = cr.Step;
		wound.Step2 = cr.Step1;
	}

	if (bLoseF && bLoseG)
	{
		for (int i = 0; i < crF2.size(); ++i)
		{
			ParseHorizonalCrack(head, DataA, blocks, vCRs, vCRs[CH_F][crF2[i]], g_direction, carType, iFRow, vWounds, 0, 2);
		}
		for (int i = 0; i < crG2.size(); ++i)
		{
			ParseHorizonalCrack(head, DataA, blocks, vCRs, vCRs[CH_G][crG2[i]], g_direction, carType, iFRow, vWounds, 0, 2);
		}
	}

	crD.clear();	crE.clear();	crF.clear();	crG.clear();
	bFindD = GetCR(CH_D, iStepSew2 - 20, 2 * g_iJawRow[railType] - 4, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_D], crD);
	bFindE = GetCR(CH_E, iStepSew2 - 20, 2 * g_iJawRow[railType] - 4, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_E], crE);
	bFindF = GetCR(CH_F, iStepSew2 - 20, 2 * g_iJawRow[railType] - 4, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_F], crF);
	bFindG = GetCR(CH_G, iStepSew2 - 20, 2 * g_iJawRow[railType] - 4, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_G], crG);
	bFindD = RemoveScrewHoleCR(vCRs[CH_D], crD);
	bFindE = RemoveScrewHoleCR(vCRs[CH_E], crE);
	bFindF = RemoveScrewHoleCR(vCRs[CH_F], crF);
	bFindG = RemoveScrewHoleCR(vCRs[CH_G], crG);

	bFindD = RemoveCRByRow1Limit(vCRs[CH_D], crD, 2 * g_iJawRow[railType]);
	bFindE = RemoveCRByRow1Limit(vCRs[CH_E], crE, 2 * g_iJawRow[railType]);
	bFindF = RemoveCRByRow1Limit(vCRs[CH_F], crF, 2 * g_iJawRow[railType]);
	bFindG = RemoveCRByRow1Limit(vCRs[CH_G], crG, 2 * g_iJawRow[railType]);

	if (bFindD || bFindE || bFindF || bFindG)
	{
		Wound_Judged wound2;
		wound2.Block = pos.Block;
		wound2.Step = pos.Step;
		wound2.Step2 = pos.Step2;
		wound2.Type = W_SEWLR;
		wound2.Place = WP_WAIST;
		wound2.IsSew = 2;

		AddWoundData(wound2, vCRs[CH_D], crD);
		AddWoundData(wound2, vCRs[CH_E], crE);
		AddWoundData(wound2, vCRs[CH_F], crF);
		AddWoundData(wound2, vCRs[CH_G], crG);

		if (bFindF || bFindG)
		{
			bool bFindWound = false;
			for (int i = 0; i < crF.size(); ++i)
			{
				int c1 = vWounds.size();
				ParseHorizonalCrack(head, DataA, blocks, vCRs, vCRs[CH_F][crF[i]], g_direction, carType, iFRow, vWounds, 0, 2);
				int c2 = vWounds.size();
				if (c2 > c1)
				{
					bFindWound = true;
					memcpy(wound2.Result, vWounds[vWounds.size() - 1].Result, 60);
					wound2.Type = vWounds[vWounds.size() - 1].Type;
					vWounds.pop_back();
					break;
				}
			}
			if (bFindWound == false)
			{
				for (int i = 0; i < crG.size(); ++i)
				{
					int c1 = vWounds.size();
					ParseHorizonalCrack(head, DataA, blocks, vCRs, vCRs[CH_G][crG[i]], g_direction, carType, iFRow, vWounds, 0, 2);
					int c2 = vWounds.size();
					if (c2 > c1)
					{
						bFindWound = true;
						memcpy(wound2.Result, vWounds[vWounds.size() - 1].Result, 60);
						wound2.Type = vWounds[vWounds.size() - 1].Type;
						vWounds.pop_back();
						break;
					}
				}
			}
		}
		AddToWounds(vWounds, wound2);
	}


#pragma region 铝热焊轨底端角反射
	crD.clear(); crE.clear();
	bFindD = GetCR(CH_D, iStepSew2 -10, iFRow - 2, iStepSew2 + 10, iFRow + 10, blocks, vCRs[CH_D], crD);
	bFindE = GetCR(CH_E, iStepSew2 - 10, iFRow - 2, iStepSew2 +10, iFRow + 10, blocks, vCRs[CH_E], crE);

	SetSewLRHFlag(vCRs[CH_D], crD, isSew);
	SetSewLRHFlag(vCRs[CH_E], crE, isSew);

	Wound_Judged wound2;
	wound2.Block = pos.Block;
	wound2.Step = pos.Step;
	wound2.Step2 = pos.Step2;
	wound2.Type = W_BOTTOM_EX;
	wound2.Place = WP_WAIST;
	wound2.IsSew = 2;

	for (int i = 0; i < crD.size(); ++i)
	{
		if (vCRs[CH_D][crD[i]].Step2 >= iStepSew2 - 2)
		{
			SetUsedFlag(vCRs[CH_D][crD[i]], 1);
		}
		else
		{
			AddWoundData(wound2, vCRs[CH_D][crD[i]]);
		}
	}

	for (int i = 0; i < crE.size(); ++i)
	{
		if (vCRs[CH_E][crE[i]].Step1 <= iStepSew2 + 2)
		{
			SetUsedFlag(vCRs[CH_E][crE[i]], 1);
		}
		else
		{
			AddWoundData(wound2, vCRs[CH_E][crE[i]]);
		}
	}
	if (wound2.vCRs.size() > 0)
	{
		AddToWounds(vWounds, wound2);
	}

#pragma endregion

	for (int j = 0; j < 10; ++j)
	{
		for (int k = 0; k < t_cr[j].size(); ++k)
		{
			if (vCRs[j][t_cr[j][k]].Step2 < iStepSew - 15 || vCRs[j][t_cr[j][k]].Step1 > iStepSew + 15)
			{
				continue;
			}
			SetUsedFlag(vCRs[j][t_cr[j][k]], 1);
		}
	}

	return count;
}

//寻找厂焊大小通道中间分开的步进
uint16_t	tempCountXCH[256] = { 0 };
uint8_t		chLengthXCH = 0;
int		GetSewXCHMiddleStep(VBDB& blocks, VCR* vCRs, CR& cr, VINT* t_cr, int& iAStepBig, int &iAStepSmall)
{
	if (iAStepBig <= 5 || t_cr[CH_A1].size() + t_cr[CH_B1].size() + t_cr[CH_C].size() == 0)
	{
		iAStepBig = iAStepSmall + 14;
		return iAStepSmall + 7;
	}
	else if (iAStepSmall <= 5 || t_cr[CH_a1].size() + t_cr[CH_b1].size() + t_cr[CH_c].size() == 0)
	{
		iAStepSmall = iAStepBig - 14;
		return iAStepBig - 7;
	}

	chLengthXCH = iAStepBig - iAStepSmall + 1;
	memset(tempCountXCH, 0, sizeof(uint16_t) * 256);
	int iMaxStep = iAStepBig;
	int iMinStepFind = iAStepBig;
	for (int i = 0; i < CH_D; ++i)
	{
		for (int j = 0; j < t_cr[i].size(); ++j)
		{
			CR& t = vCRs[i][t_cr[i][j]];
			for (int k = 0; k < t.Region.size(); ++k)
			{
				if (t.Region[k].step >= iAStepSmall && t.Region[k].row >= 11 && t.Region[k].row <= 15)
				{
					tempCountXCH[t.Region[k].step - iAStepSmall]++;
					if (t.Region[k].step <= iMinStepFind)
					{
						iMinStepFind = t.Region[k].step;
					}
				}
				if (t.Region[k].step >= iAStepBig)
				{
					iMaxStep = t.Region[k].step;
				}
			}
		}
	}

	int minCount = 100, iminIndex = 0;
	int delt = iMinStepFind - iAStepSmall;
	for (int i = delt; i < iMaxStep - iAStepSmall; ++i)
	{
		if (tempCountXCH[i] < minCount)
		{
			minCount = tempCountXCH[i];
			iminIndex = i;
		}
	}

	int count = tempCountXCH[delt];
	int sz1 = delt;
	for (int i = delt + 1; i < iminIndex - 1; ++i)
	{
		if (tempCountXCH[i] > count)
		{
			count = tempCountXCH[i];
			sz1 = i;
		}
	}

	count = minCount;
	int sz2 = iminIndex;
	for (int i = iminIndex + 1; i < iMaxStep - iAStepSmall; ++i)
	{
		if (tempCountXCH[i] > count)
		{
			count = tempCountXCH[i];
			sz2 = i;
		}
	}
	return iAStepSmall + (sz1 + sz2) / 2;
}

uint8_t	ParseSewXCH(F_HEAD& g_fileHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int iCrIndex, int16_t iFRow, bool carType, uint8_t railType, VINT* t_cr, int& iAStepBig, int &iAStepSmall, int & iStepSew, VWJ& vWounds, VPM& vPMs)
{
	int iStepSew2 = iStepSew;
	iStepSew = GetSewXCHMiddleStep(blocks, vCRs, cr, t_cr, iAStepBig, iAStepSmall);
	if (Abs(iStepSew2 - iStepSew) > 10)
	{
		iStepSew = iStepSew2;
	}
	Pos	 pos = FindStepInBlock(iStepSew, g_vBlockHeads, cr.Block - g_iBeginBlock);
	BLOCK blockHead = blocks[cr.Block - g_iBeginBlock].BlockHead;
	uint8_t isWound = 0;

	Wound_Judged wound;
	wound.Type = W_HS;
	wound.IsSew = 3;
	wound.Block = cr.Block;
	wound.Step = cr.Step;
	wound.Step2 = cr.Step1;
	memcpy(wound.Result, "现场焊焊缝核伤", 30);
	FillWound(wound, blockHead, g_fileHead);
#pragma region 核伤
	for (int j = 0; j < 10; ++j)
	{
		int t_A = GetAChannelByBChannel(j);
		double t_angle = 0.1 * g_fileHead.deviceP2.Angle[t_A].Refrac;
		int t_offset = g_fileHead.deviceP2.Place[t_A] + blockHead.probOff[t_A];
		bool isFindMain = false;
		for (int k = 0; k < t_cr[j].size(); ++k)
		{
			CR& t = vCRs[j][t_cr[j][k]];
			SetSewLRHFlag(t, 2);
			if (t.IsUsed == 1)
			{
				continue;
			}
			if (t.Step1 < iStepSew - 10 || t.Step2 > iStepSew + 10)
			{
				continue;
			}
			if (j % 4 == 0)//大通道
			{
				GetCRInfo(t, DataA, blocks, t_cr[j].size() == 1);
				//正常位置出波
				if (t.Step1 >= iStepSew)
				{
					if (t.Channel == CH_C && t.Row1 <= g_iJawRow[railType] - 3/* && t.Info.Shift >= 25*/)
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, g_fileHead.step, g_direction);
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = g_fileHead.step * (t.Step2 - t.Step1);
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "现场焊焊缝处%s正常位置低于10行出波，位移：%.1f大格，幅值%d", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
				}
				else //if (t.Info.MaxV >= g_iAmpl  && t.Info.Shift >= 25)//异常位置
				{
					wound.Block = t.Block;
					wound.Step = t.Step;
					wound.Step2 = t.Step1;
					wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, g_fileHead.step, g_direction);
					wound.SizeX = 3.0 * (t.Row2 - t.Row1);
					wound.SizeY = g_fileHead.step * (t.Step2 - t.Step1);
					if (j < CH_C && t.Info.MaxH >= 250)
					{
						if ((carType == true && j == CH_A1) || (carType == false && j == CH_B1))
						{
							wound.Place = WP_HEAD_IN;
						}
						else
						{
							wound.Place = WP_HEAD_OUT;
						}
						wound.Degree = WD_SERIOUS;
						wound.SizeX = 1.5 * (t.Row2 - t.Row1);
						wound.SizeY = 0.5 * g_fileHead.step * (t.Step2 - t.Step1);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "现场焊焊缝处%s小通道位置出波，位移：%.1f大格，幅值%d，大于5大格出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
					else if (j >= CH_C)
					{
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = g_fileHead.step * (t.Step2 - t.Step1);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "现场焊焊缝处%s小通道位置出波，位移：%.1f大格，幅值%d", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
				}
			}
			else//小通道
			{
				if (t.Step1 > iStepSew)//异常位置出波
				{
					GetCRInfo(t, DataA, blocks, t_cr[j].size() == 1);
					if (j == CH_c/* && t.Info.MaxV >= g_iAmpl && t.Info.Shift >= 25*/)
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, g_fileHead.step, g_direction);
						wound.IsSew = 3;
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = g_fileHead.step * (t.Step2 - t.Step1);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "现场焊焊缝处%s大通道位置出波，位移：%.1f大格，幅值%d，大通道位置出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
					else if (j < CH_c /* && t.Info.MaxV >= g_iAmpl && t.Info.Shift >= 50 */)
					{
						wound.Block = t.Block;
						wound.Step = t.Step;
						wound.Step2 = t.Step1;
						wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, g_fileHead.step, g_direction);
						wound.IsSew = 3;
						wound.Place = WP_HEAD_MID;
						wound.Degree = WD_SERIOUS;
						wound.SizeX = 3.0 * (t.Row2 - t.Row1);
						wound.SizeY = g_fileHead.step * (t.Step2 - t.Step1);
						if (bShowWoundDetail)
						{
							sprintf(tempAccording, "现场焊焊缝处%s大通道位置出波，位移：%.1f大格，幅值%d，大通道位置出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
							wound.According.emplace_back(tempAccording);
						}
						AddWoundData(wound, t);
					}
				}
				else//正常位置出波
				{
					if (GetCRInfo(t, DataA, blocks, t_cr[j].size() == 1) && t.Info.MaxV >= g_iAmpl && t.Info.Shift >= 25 || t.Row1 <= g_iJawRow[railType] - 3)
					{
						if (j >= CH_C && t.Row1 <= g_iJawRow[railType] - 3)
						{
							wound.Block = t.Block;
							wound.Step = t.Step;
							wound.Step2 = t.Step1;
							wound.Walk = GetWD(blocks[wound.Block - g_iBeginBlock].BlockHead.walk, wound.Step, g_fileHead.step, g_direction);
							wound.Place = WP_HEAD_MID;
							wound.Degree = WD_SERIOUS;
							wound.SizeX = 3.0 * (t.Row2 - t.Row1);
							wound.SizeY = g_fileHead.step * (t.Step2 - t.Step1);
							if (bShowWoundDetail)
							{
								sprintf(tempAccording, "现场焊焊缝处%s出波，位移：%.1f大格，幅值%d，低于10行出波", ChannelNamesB[j].c_str(), 0.02f * t.Info.Shift, t.Info.MaxV);
								wound.According.emplace_back(tempAccording);
							}
							AddWoundData(wound, t);
						}
					}
				}
			}
		}

		if (t_cr[j].size() > 1)
		{
			int maxStep = 0, minStep = 0x7FFFFFFF;
			if (j % 4 == 0)//大通道
			{
				if (vCRs[j][t_cr[j][0]].Region.size() >= 3 && j < CH_C || vCRs[j][t_cr[j][0]].Region.size() >= 2 && j >= CH_C)
				{
					AddWoundData(wound, vCRs[j][t_cr[j][0]]);
				}
			}
			else
			{
				if (vCRs[j][t_cr[j][t_cr[j].size() - 1]].Region.size() >= 3 && j < CH_C || vCRs[j][t_cr[j][t_cr[j].size() - 1]].Region.size() >= 2 && j >= CH_C)
				{
					AddWoundData(wound, vCRs[j][t_cr[j][t_cr[j].size() - 1]]);
				}
			}
		}
	}
#pragma endregion

	VINT crD, crE, crF, crG;
#pragma region 螺孔
	int screwIndexes[] = { -3, -2, -1, 1, 2, 3 };
	bool isExistScrewHoles[6] = { false, false , false ,false ,false ,false };
	uint8_t count = 0;
	for (int i = 0; i < 3; ++i)
	{
		VINT crF, crG, crF2, crG2;
		int tr = iAStepSmall + dScrewDistance[railType][i] / g_fileHead.step - g_screwholeRange;
		int tl = iAStepSmall + dScrewDistance[railType][i] / g_fileHead.step + g_screwholeRange;
		bool bFind = GetCR(CH_F, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF, -1, 2) ||
			GetCR(CH_G, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG, -1, 2);

		bool bLose = GetCR(CH_F, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_F], crF2, -1, 2) ||
			GetCR(CH_G, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_G], crG2, -1, 2);
		crD.clear(); crE.clear();
		bool bFindD = GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 2);
		bool bFindE = GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 2);
		if (bFind && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHole(g_fileHead, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else if ((bFindD || bFindE) && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHoleNoFG(g_fileHead, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else
		{
			isExistScrewHoles[i] = false;
		}
	}
	for (int i = 3; i < 6; ++i)
	{
		VINT crF, crG, crF2, crG2;
		int tr = iAStepBig + dScrewDistance[railType][i] / g_fileHead.step - g_screwholeRange;
		int tl = iAStepBig + dScrewDistance[railType][i] / g_fileHead.step + g_screwholeRange;
		bool bFind = GetCR(CH_F, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF, -1, 2) ||
			GetCR(CH_G, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG, -1, 2);

		bool bLose = GetCR(CH_F, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_F], crF2, -1, 2) ||
			GetCR(CH_G, tr, iFRow - 3, tl, iFRow + 3, blocks, vCRs[CH_G], crG2, -1, 2);
		crD.clear(); crE.clear();
		bool bFindD = GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 2);
		bool bFindE = GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 2);
		if (bFind && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHole(g_fileHead, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else if ((bFindD || bFindE) && bLose)
		{
			isExistScrewHoles[i] = ParseScrewHoleNoFG(g_fileHead, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, false, false, true, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else
		{
			isExistScrewHoles[i] = false;
		}
	}
#pragma endregion

	crD.clear();
	crE.clear();
	uint8_t bFindD = GetCR(CH_D, iStepSew2 - 20, 0, iStepSew2 + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_D], crD);
	uint8_t bFindE = GetCR(CH_E, iStepSew2 - 20, 0, iStepSew2 + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_E], crE);
	uint8_t bFindF = GetCR(CH_F, iStepSew2 - 20, 0, iStepSew2 + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_F], crF);
	uint8_t bFindG = GetCR(CH_G, iStepSew2 - 20, 0, iStepSew2 + 20, 2 * g_iJawRow[railType], blocks, vCRs[CH_G], crG);
	bFindD = RemoveScrewHoleCR(vCRs[CH_D], crD);
	bFindE = RemoveScrewHoleCR(vCRs[CH_E], crE);
	bFindF = RemoveScrewHoleCR(vCRs[CH_F], crF);
	bFindG = RemoveScrewHoleCR(vCRs[CH_G], crG);

	VINT crF2, crG2;
	uint8_t bLoseF = GetCR(CH_F, iStepSew2 - 10, iFRow - 5, iStepSew2 + 10, iFRow + 5, blocks, vCRs[CH_F], crF2);
	uint8_t bLoseG = GetCR(CH_G, iStepSew2 - 10, iFRow - 5, iStepSew2 + 10, iFRow + 5, blocks, vCRs[CH_G], crG2);

	if (bFindD || bFindE || bFindF || bFindG || wound.vCRs.size() > 0/* || bLoseF || bLoseG*/)
	{
		wound.Place = WP_HEAD_MID;
		if (bFindD + bFindE == 1)
		{
			for (int i = crD.size() - 1; i >= 0; --i)
			{
				CR& tpD = vCRs[CH_D][crD[i]];
				if (tpD.Step1 > iStepSew2 && tpD.Row1 > g_iJawRow[railType])
				{
					crD.erase(crD.begin() + i);
				}
			}

			for (int i = crE.size() - 1; i >= 0; --i)
			{
				CR& tpE = vCRs[CH_E][crE[i]];
				if (tpE.Step2 < iStepSew2 && tpE.Row1 > g_iJawRow[railType])
				{
					crE.erase(crE.begin() + i);
				}
			}
		}

		AddWoundData(wound, vCRs[CH_D], crD);
		AddWoundData(wound, vCRs[CH_E], crE);
		AddWoundData(wound, vCRs[CH_F], crF);
		AddWoundData(wound, vCRs[CH_G], crG);
		//AddWoundData(wound, vCRs[CH_F], crF2);
		//AddWoundData(wound, vCRs[CH_G], crG2);
		SetUsedFlag(vCRs[CH_D], crD, 1);
		SetUsedFlag(vCRs[CH_E], crE, 1);
		SetUsedFlag(vCRs[CH_F], crF, 1);
		SetUsedFlag(vCRs[CH_G], crG, 1);
		//SetUsedFlag(vCRs[CH_F], crF2, 1);
		//SetUsedFlag(vCRs[CH_G], crG2, 1);
		AddToWounds(vWounds, wound);
		wound.SetEmpty();
		wound.Block = cr.Block;
		wound.Step = cr.Step;
		wound.Step2 = cr.Step1;
	}

	if (bLoseF || bLoseG)
	{
		for (int i = 0; i < crF2.size(); ++i)
		{
			ParseHorizonalCrack(g_fileHead, DataA, blocks, vCRs, vCRs[CH_F][crF2[i]], g_direction, carType, iFRow, vWounds, 0, 1);
		}
		for (int i = 0; i < crG2.size(); ++i)
		{
			ParseHorizonalCrack(g_fileHead, DataA, blocks, vCRs, vCRs[CH_G][crG2[i]], g_direction, carType, iFRow, vWounds, 0, 1);
		}
	}

	crD.clear();	crE.clear();	crF.clear();	crG.clear();
	bFindD = GetCR(CH_D, iStepSew2 - 20, 2 * g_iJawRow[railType] + 1, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_D], crD);
	bFindE = GetCR(CH_E, iStepSew2 - 20, 2 * g_iJawRow[railType] + 1, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_E], crE);
	bFindF = GetCR(CH_F, iStepSew2 - 20, 2 * g_iJawRow[railType] + 1, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_F], crF);
	bFindG = GetCR(CH_G, iStepSew2 - 20, 2 * g_iJawRow[railType] + 1, iStepSew2 + 20, iFRow - 10, blocks, vCRs[CH_G], crG);
	bFindD = RemoveScrewHoleCR(vCRs[CH_D], crD);
	bFindE = RemoveScrewHoleCR(vCRs[CH_E], crE);
	bFindF = RemoveScrewHoleCR(vCRs[CH_F], crF);
	bFindG = RemoveScrewHoleCR(vCRs[CH_G], crG);

	bFindD = RemoveCRByRow1Limit(vCRs[CH_D], crD, 2 * g_iJawRow[railType]);
	bFindE = RemoveCRByRow1Limit(vCRs[CH_E], crE, 2 * g_iJawRow[railType]);
	bFindF = RemoveCRByRow1Limit(vCRs[CH_F], crF, 2 * g_iJawRow[railType]);
	bFindG = RemoveCRByRow1Limit(vCRs[CH_G], crG, 2 * g_iJawRow[railType]);
	if (bFindD || bFindE || bFindF || bFindG)
	{
		Wound_Judged wound2;
		wound2.Block = pos.Block;
		wound2.Step = pos.Step;
		wound2.Step2 = pos.Step2;
		//wound2.Type = W_SEWXCH;
		wound2.Type = W_SEWXCH;
		wound2.Place = WP_WAIST;
		wound2.IsSew = 3;

		AddWoundData(wound2, vCRs[CH_D], crD);
		AddWoundData(wound2, vCRs[CH_E], crE);
		AddWoundData(wound2, vCRs[CH_F], crF);
		AddWoundData(wound2, vCRs[CH_G], crG);

		AddToWounds(vWounds, wound2);
	}



#pragma region 现场焊轨底端角反射
	crD.clear(); crE.clear();
	bFindD = GetCR(CH_D, iStepSew2 - 10, iFRow - 2, iStepSew2 + 10, iFRow + 10, blocks, vCRs[CH_D], crD);
	bFindE = GetCR(CH_E, iStepSew2 - 10, iFRow - 2, iStepSew2 + 10, iFRow + 10, blocks, vCRs[CH_E], crE);

	SetSewLRHFlag(vCRs[CH_D], crD, 3);
	SetSewLRHFlag(vCRs[CH_E], crE, 3);

	Wound_Judged wound2;
	wound2.Block = pos.Block;
	wound2.Step = pos.Step;
	wound2.Step2 = pos.Step2;
	wound2.Type = W_BOTTOM_EX;
	wound2.Place = WP_WAIST;
	wound2.IsSew = 3;

	for (int i = 0; i < crD.size(); ++i)
	{
		if (vCRs[CH_D][crD[i]].Step2 >= iStepSew2 - 2)
		{
			SetUsedFlag(vCRs[CH_D][crD[i]], 1);
		}
		else
		{
			AddWoundData(wound2, vCRs[CH_D][crD[i]]);
		}
	}

	for (int i = 0; i < crE.size(); ++i)
	{
		if (vCRs[CH_E][crE[i]].Step1 <= iStepSew2 + 2)
		{
			SetUsedFlag(vCRs[CH_E][crE[i]], 1);
		}
		else
		{
			AddWoundData(wound2, vCRs[CH_E][crE[i]]);
		}
	}
	if (wound2.vCRs.size() > 0)
	{
		AddToWounds(vWounds, wound2);
	}

#pragma endregion

	for (int j = 0; j < 10; ++j)
	{
		for (int k = 0; k < t_cr[j].size(); ++k)
		{
			if (vCRs[j][t_cr[j][k]].Step1 < iStepSew - 15 || vCRs[j][t_cr[j][k]].Step2 > iStepSew + 15)
			{
				continue;
			}
			SetUsedFlag(vCRs[j][t_cr[j][k]], 1);
		}
	}

	return count;
}

void GetJointMainCR(uint8_t channel, VCR* vCRs, VINT* t_cr, int&stepFRight, int& stepFLeft, uint8_t& iJawRow, int& mainCRIndex)
{
	mainCRIndex = -1;
	int crSize = 0;
	for (int i = 0; i < t_cr[channel].size(); ++i)
	{
		if (vCRs[channel][t_cr[channel][i]].Row1 > iJawRow + 3 || vCRs[channel][t_cr[channel][i]].Row2 < iJawRow - 3)
		{

		}
		else
		{
			if (channel % 4 == 0 && vCRs[channel][t_cr[channel][i]].Step1 <= stepFRight && vCRs[channel][t_cr[channel][i]].Step1 >= stepFRight - 10 ||
				channel % 4 != 0 && vCRs[channel][t_cr[channel][i]].Step2 >= stepFLeft && vCRs[channel][t_cr[channel][i]].Step1 <= stepFLeft + 10)
			{

				mainCRIndex = t_cr[channel][i];
				crSize = vCRs[channel][t_cr[channel][i]].Region.size();
				break;
			}
		}
	}
}

uint8_t	ParseJoint(F_HEAD& g_fileHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int& stepFRight, int& stepFLeft, uint8_t& railType, bool& direction, bool& carType, int16_t& iFRow, double& wd, VINT* t_cr, VWJ& vWounds, VPM& vPMs, PM& pm)
{
#pragma region 螺孔伤损判定
	int screwIndexes[] = { -3, -2, -1, 1, 2, 3 };

	bool bAutoJoint = false;
	int pIndex = GetManualJoint(stepFRight - 100, vPMs);
	if (pIndex >= 0)
	{
		bAutoJoint = true;
	}

	Pos posRight = FindStepInBlock(stepFRight, blocks, 0);
	Pos posLeft = FindStepInBlock(stepFLeft, blocks, 0);;

	VINT crFLose, crGLose;
	GetCR(CH_F, stepFRight - 1, iFRow - 3, stepFLeft + 1, iFRow + 3, blocks, vCRs[CH_F], crFLose, -1, 1, false);
	GetCR(CH_G, stepFRight - 1, iFRow - 3, stepFLeft + 1, iFRow + 3, blocks, vCRs[CH_G], crGLose, -1, 1, false);
	SetJointFlag(vCRs[CH_F], crFLose, 1);
	SetJointFlag(vCRs[CH_G], crGLose, 1);
	SetUsedFlag(vCRs[CH_F], crFLose, 1);
	SetUsedFlag(vCRs[CH_G], crGLose, 1);
	if (crFLose.size() == 1 && crGLose.size() == 1)
	{
		int sl = vCRs[CH_F][crFLose[0]].Step2;
		int sr = vCRs[CH_F][crFLose[0]].Step1;
		if (sl - sr < 30)
		{
			stepFRight = sr;
			stepFLeft = sl;
#ifdef _DEBUG
			posRight = FindStepInBlock(stepFRight, blocks, 0);
			posLeft = FindStepInBlock(stepFLeft, blocks, 0);
#endif // _DEBUG
		}
	}

	uint8_t iJawRow = g_iJawRow[railType & 0x03];
	for (int i = 0; i < t_cr[CH_c].size(); ++i)
	{
		CR& temp = vCRs[CH_c][t_cr[CH_c][i]];
		if (temp.Step2 < stepFRight && temp.Step2 > stepFRight - 30 && temp.Row1 <= iJawRow && temp.Row2 >= iJawRow + 3)
		{
			SetJointFlag(temp, 1);
			SetUsedFlag(temp, 1);
		}
	}

	for (int i = t_cr[CH_C].size() - 1; i >= 0; --i)
	{
		CR& temp = vCRs[CH_C][t_cr[CH_C][i]];
		if (temp.Step1 > stepFLeft && temp.Step1 < stepFLeft + 30 && temp.Row1 <= iJawRow && temp.Row2 >= iJawRow + 3)
		{
			SetJointFlag(temp, 1);
			SetUsedFlag(temp, 1);
		}
	}


	bool isExistScrewHoles[6];
	uint8_t count = 0;

	int oldRT = railType;
	posRight = FindStepInBlock(stepFRight - 20, blocks, 0);
	uint8_t fRow = iFRow;
	if (posRight.Block - g_iBeginBlock >= 0 && posRight.Block - g_iBeginBlock < blocks.size())
	{
		fRow = blocks[posRight.Block - g_iBeginBlock].vBStepDatas[posRight.Step].FRow;
	}
	railType = GetRailTypeByFRow(fRow, g_iBottomRow, R_N);
	for (int i = 0; i < 3; ++i)
	{
		VINT crF, crG, crF2, crG2, crD, crE;
		int tr = stepFRight + 1.0 * dScrewDistance[railType][i] / g_fileHead.step - g_screwholeRange;
		int tl = stepFRight + 1.0 * dScrewDistance[railType][i] / g_fileHead.step + g_screwholeRange;
		if (tr < 0) tr = 0;
		if (tl < 0) tl = 0;
		Pos pr = FindStepInBlock(tr, g_vBlockHeads, 0);
		Pos pl = FindStepInBlock(tl, g_vBlockHeads, 0);
		if (pr.Step2 < blocks[0].IndexL2)
		{
			bool bFindStep = false;
			for (int j = 0; j < blocks.size() && bFindStep == false; ++j)
			{
				for (int k = 0; k < blocks[j].vBStepDatas.size(); ++k)
				{
					iFRow = (std::min)(blocks[j].vBStepDatas[k].FRow, blocks[j].vBStepDatas[k].GRow);
					bFindStep = true;
					break;
				}
			}
		}
		else
		{
			if (pl.Block - g_iBeginBlock < 0 || pl.Block - g_iBeginBlock >= blocks.size())
			{

			}
			else
			{
				iFRow = (std::min)(blocks[pl.Block - g_iBeginBlock].vBStepDatas[pl.Step].FRow, blocks[pl.Block - g_iBeginBlock].vBStepDatas[pl.Step].GRow);
			}

		}
		bool bFind = GetCR(CH_F, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF, -1, 2) ||
			GetCR(CH_G, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG, -1, 2);

		bool bLose = GetCR(CH_F, tr, iFRow - 3, tl, iFRow + 20, blocks, vCRs[CH_F], crF2, -1, 2) ||
			GetCR(CH_G, tr, iFRow - 3, tl, iFRow + 20, blocks, vCRs[CH_G], crG2, -1, 2);
		bool bFindD = GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 2);
		bool bFindE = GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 2);
		if (bLose)
		{
			isExistScrewHoles[i] = ParseScrewHole(g_fileHead, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, true, bAutoJoint, false, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else
		{
			isExistScrewHoles[i] = false;
		}
	}

	posLeft = FindStepInBlock(stepFLeft + 20, blocks, 0);
	fRow = iFRow;
	if (posLeft.Block - g_iBeginBlock >= 0 && posLeft.Block - g_iBeginBlock < blocks.size())
	{
		fRow = blocks[posLeft.Block - g_iBeginBlock].vBStepDatas[posLeft.Step].FRow;
	}
	railType = GetRailTypeByFRow(fRow, g_iBottomRow, R_N);
	for (int i = 3; i < 6; ++i)
	{
		VINT crF, crG, crF2, crG2, crD, crE;
		int tr = stepFLeft + 1.0 * dScrewDistance[railType][i] / g_fileHead.step - g_screwholeRange;
		int tl = stepFLeft + 1.0 * dScrewDistance[railType][i] / g_fileHead.step + g_screwholeRange;
		if (tr < 0) tr = 0;
		if (tl < 0) tl = 0;
		Pos pr = FindStepInBlock(tr, g_vBlockHeads, 0);
		Pos pl = FindStepInBlock(tl, g_vBlockHeads, 0);
		if (pl.Step2 >= blocks[blocks.size() - 1].IndexL2 + blocks[blocks.size() - 1].BlockHead.row)
		{
			bool bFindStep = false;
			for (int j = blocks.size() - 1; j >= 0 && bFindStep == false; --j)
			{
				for (int k = blocks[j].vBStepDatas.size() - 1; k >= 0; --k)
				{
					iFRow = (std::min)(blocks[j].vBStepDatas[k].FRow, blocks[j].vBStepDatas[k].GRow);
					bFindStep = true;
					break;
				}

		}
	}
		else
		{
			if (pl.Block - g_iBeginBlock < 0 && pl.Block - g_iBeginBlock >= blocks.size())
			{

			}
			else
			{
				iFRow = (std::min)(blocks[pl.Block - g_iBeginBlock].vBStepDatas[pl.Step].FRow, blocks[pl.Block - g_iBeginBlock].vBStepDatas[pl.Step].GRow);
			}
		}
		bool bFind = GetCR(CH_F, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF, -1, 2) ||
			GetCR(CH_G, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG, -1, 2);
		bool bLose = GetCR(CH_F, tr, iFRow - 3, tl, iFRow + 20, blocks, vCRs[CH_F], crF2, -1, 2) ||
			GetCR(CH_G, tr, iFRow - 3, tl, iFRow + 20, blocks, vCRs[CH_G], crG2, -1, 2);
		bool bFindD = GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 2);
		bool bFindE = GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 2);
		if (bLose)
		{
			isExistScrewHoles[i] = ParseScrewHole(g_fileHead, DataA, blocks, vCRs, tr, tl, iFRow, railType, vWounds, vPMs, true, bAutoJoint, false, screwIndexes[i]);
			if (isExistScrewHoles[i])	++count;
		}
		else
		{
			isExistScrewHoles[i] = false;
		}
}	railType = oldRT;
#pragma endregion

#pragma region 先检测是否有核伤
	for (int i = 0; i < CH_C; ++i)
	{
		for (int j = 0; j < t_cr[i].size(); ++j)
		{
			CR& cr = vCRs[i][t_cr[i][j]];
			if ((cr.Step1 >= stepFRight - 15 && cr.Step1 <= stepFLeft + 15) || (cr.Step2 >= stepFRight - 15 && cr.Step2 <= stepFLeft + 15))
			{
				SetJointFlag(cr, 1);
			}
		}
	}

	Section sections[16];
	int aveLenNoCc;
	GetWaveLen(vCRs, t_cr, sections, aveLenNoCc);

	VWJ vw;
	//临时伤损
	Wound_Judged wound;
	bool hasWound = false;

	for (int i = 0; i < CH_C; ++i)
	{
		if (sections[i].Flag > 0 && sections[i].Flag < aveLenNoCc + 5)
		{
			SetJointFlag(vCRs[i], t_cr[i], 1);
			SetUsedFlag(vCRs[i], t_cr[i], 1);
			continue;
		}

		int iChA = GetAChannelByBChannel(i);
		int iMainIndex;
		GetJointMainCR(i, vCRs, t_cr, stepFRight, stepFLeft, g_iJawRow[railType], iMainIndex);
		if (iMainIndex >= 0)
		{
			int s1 = vCRs[i][iMainIndex].Step1, s2 = vCRs[i][iMainIndex].Step2;
			for (int j = 0; j < t_cr[i].size(); ++j)
			{
				CR& cr = vCRs[i][t_cr[i][j]];
				BLOCK& blockHead = blocks[cr.Block - g_iBeginBlock].BlockHead;
				if (i % 4 == 0 && cr.Region.size() >= 5 && cr.IsUsed == 0 && ((cr.Step1 > s2 && cr.Step2 > stepFLeft) || (cr.Step2 < s1 && cr.Step2 < stepFRight - 10)))
				{
					if (cr.Region.size() <= 5 || Abs(cr.Step1 - stepFRight) <= 8 && Abs(cr.Step2 - stepFLeft) <= 8 && cr.Region.size() < 0.3 * vCRs[i][iMainIndex].Region.size())
					{
						SetUsedFlag(cr, 1);
						continue;
					}
					if (cr.Step1 >= s1 - 3 && cr.Step2 <= s2 + 3)
					{
						SetUsedFlag(cr, 1);
						continue;
					}

					GetCRInfo(cr, DataA, blocks);
					bool bWound = ParseHS(DataA, blocks, vCRs, cr, cr.Info, railType, direction, carType, iFRow, wd, vw, vPMs, 1, 0);
					if (bWound)
					{
						if (hasWound)
						{
							AddWoundData(wound, cr);
						}
						else
						{
							if (vw.size() > 0)
							{
								wound = vw[vw.size() - 1];
								vw.pop_back();
							}
							hasWound = true;
						}
					}
				}
				else if (i % 4 != 0 && cr.Region.size() >= 5 && cr.IsUsed == 0 && ((cr.Step1 < s1 && cr.Step1 < stepFRight) || (cr.Step2 > s2 && cr.Step2 > stepFLeft + 10)))
				{
					if (cr.Step1 >= s1 - 8 && cr.Step2 <= s2 + 8)
					{
						if (cr.Region.size() <= 5 || Abs(cr.Step1 - stepFRight) <= 8 && Abs(cr.Step2 - stepFLeft) <= 8 && cr.Region.size() < 0.3 * vCRs[i][iMainIndex].Region.size())
						{
							SetUsedFlag(cr, 1);
							continue;
						}
						if (cr.Step1 >= s1 - 3 && cr.Step2 <= s2 + 3)
						{
							SetUsedFlag(cr, 1);
							continue;
						}
					}

					GetCRInfo(cr, DataA, blocks);
					bool bWound = ParseHS(DataA, blocks, vCRs, cr, cr.Info, railType, direction, carType, iFRow, wd, vw, vPMs, 1, 0);
					if (bWound)
					{
						if (hasWound)
						{
							AddWoundData(wound, cr);
						}
						else
						{
							if (vw.size() > 0)
							{
								wound = vw[vw.size() - 1];
								vw.pop_back();
							}
							hasWound = true;
						}
					}
				}
			}
		}
		else
		{
			for (int j = 0; j < t_cr[i].size(); ++j)
			{
				CR& cr = vCRs[i][t_cr[i][j]];
				BLOCK& blockHead = blocks[cr.Block - g_iBeginBlock].BlockHead;
				if (cr.Region.size() >= 3 && cr.IsUsed == 0)
				{
					if (cr.Step2 <= stepFRight - 10 || cr.Step1 >= stepFLeft + 10)
					{
						GetCRInfo(cr, DataA, blocks);
						bool bWound = ParseHS(DataA, blocks, vCRs, cr, cr.Info, railType, direction, carType, iFRow, wd, vw, vPMs, 1, 0);
						if (bWound)
						{
							if (hasWound)
							{
								AddWoundData(wound, cr);
							}
							else
							{
								if (vw.size() > 0)
								{
									wound = vw[vw.size() - 1];
									vw.pop_back();
								}
								hasWound = true;
							}
						}
					}
					else if (cr.Step2 <= stepFRight - 4 || cr.Step1 >= stepFLeft + 4)
					{
						GetCRInfo(cr, DataA, blocks);
						bool bWound = ParseHS(DataA, blocks, vCRs, cr, cr.Info, railType, direction, carType, iFRow, wd, vw, vPMs, 1, 0);
						if (bWound)
						{
							if (hasWound)
							{
								AddWoundData(wound, cr);
							}
							else
							{
								if (vw.size() > 0)
								{
									wound = vw[vw.size() - 1];
									vw.pop_back();
								}
								hasWound = true;
							}
						}
					}
				}
			}
		}
	}
	for (int i = CH_C; i <= CH_c; ++i)
	{
		int iChA = GetAChannelByBChannel(i);
		for (int j = 0; j < t_cr[i].size(); ++j)
		{
			CR& cr = vCRs[i][t_cr[i][j]];
			BLOCK blockHead = blocks[cr.Block - g_iBeginBlock].BlockHead;
			if (cr.Region.size() >= 3 && cr.IsUsed == 0)
			{
				if (
					cr.Channel == CH_C && (cr.Step2 <= stepFRight - 10 || cr.Step1 >= stepFLeft + 20) ||
					cr.Channel == CH_c && (cr.Step1 <= stepFRight - 20 || cr.Step1 >= stepFLeft + 10)
					)
				{
					GetCRInfo(cr, DataA, blocks);
					bool bWound = ParseHS(DataA, blocks, vCRs, cr, cr.Info, railType, direction, carType, iFRow, wd, vw, vPMs, 1, 0);
					if (bWound)
					{
						if (hasWound)
						{
							AddWoundData(wound, cr);
						}
						else
						{
							if (vw.size() > 0)
							{
								wound = vw[vw.size() - 1];
								vw.pop_back();
							}
							hasWound = true;
						}
					}
				}
			}
		}
	}
	if (hasWound)
	{
		AddToWounds(vWounds, wound);
	}
	for (int i = 0; i < CH_C; ++i)
	{
		SetJointFlag(vCRs[i], t_cr[i], 1);
		SetUsedFlag(vCRs[i], t_cr[i], 1);
	}
#pragma endregion

#pragma region 端面反射DE
	//轨底部
	VINT crD, crE;
	GetCR(CH_D, stepFRight - 10, iFRow - 3, stepFRight + 2, iFRow + 10, blocks, vCRs[CH_D], crD);
	GetCR(CH_E, stepFLeft - 2, iFRow - 3, stepFLeft + 10, iFRow + 10, blocks, vCRs[CH_E], crE);
	SetJointFlag(vCRs[CH_D], crD, 1);
	SetJointFlag(vCRs[CH_E], crE, 1);
	for (int k = 0; k < crD.size(); ++k)
	{
		if (vCRs[CH_D][crD[k]].Row2 >= iFRow)
		{
			SetUsedFlag(vCRs[CH_D][crD[k]], 1);
		}
	}
	for (int k = 0; k < crE.size(); ++k)
	{
		if (vCRs[CH_E][crE[k]].Row2 >= iFRow)
		{
			SetUsedFlag(vCRs[CH_E][crE[k]], 1);
		}
	}

	//轨颚处
	int stepMiddle = (stepFLeft + stepFRight) / 2;
	crD.clear();
	crE.clear();
	uint8_t iFindD = GetCR(CH_D, stepFRight - 6, g_iCornerReflectionRowL[railType], stepMiddle + 3, g_iCornerReflectionRowH[railType], blocks, vCRs[CH_D], crD);
	uint8_t iFindE = GetCR(CH_E, stepMiddle - 3, g_iCornerReflectionRowL[railType], stepFLeft + 6, g_iCornerReflectionRowH[railType], blocks, vCRs[CH_E], crE);
	for (int k = 0; k < crD.size(); ++k)
	{
		if (vCRs[CH_D][crD[k]].Region.size() > 3)
		{
			JointReflectWave jrf;
			jrf.cr = vCRs[CH_D][crD[k]];
			jrf.Row1 = jrf.cr.Row1;
			jrf.Row2 = jrf.cr.Row2;
			jrf.RailType = railType;
			jrf.FRow = fRow;
			g_vJWRD.push_back(jrf);
		}
		if (vCRs[CH_D][crD[k]].Region.size() < 15 && vCRs[CH_D][crD[k]].Step2 - vCRs[CH_D][crD[k]].Step1 <= 4 && vCRs[CH_D][crD[k]].Row2 - vCRs[CH_D][crD[k]].Row1 <= 4)
		{
			SetJointFlag(vCRs[CH_D][crD[k]], 1);
		}
		if (vCRs[CH_D][crD[k]].Row1 >= g_iCornerReflectionRowL[railType] && vCRs[CH_D][crD[k]].Row2 <= g_iCornerReflectionRowH[railType])
		{
			SetUsedFlag(vCRs[CH_D][crD[k]], 1);
		}
	}
	for (int k = 0; k < crE.size(); ++k)
	{
		if (vCRs[CH_E][crE[k]].Region.size() > 3)
		{
			JointReflectWave jrf;
			jrf.cr = vCRs[CH_E][crE[k]];
			jrf.Row1 = jrf.cr.Row1;
			jrf.Row2 = jrf.cr.Row2;
			jrf.RailType = railType;
			jrf.FRow = fRow;
			g_vJWRE.push_back(jrf);
		}
		if (vCRs[CH_E][crE[k]].Region.size() < 15 && vCRs[CH_E][crE[k]].Step2 - vCRs[CH_E][crE[k]].Step1 <= 4 && vCRs[CH_E][crE[k]].Row2 - vCRs[CH_E][crE[k]].Row1 <= 4)
		{
			SetJointFlag(vCRs[CH_E][crE[k]], 1);
		}
		if (vCRs[CH_E][crE[k]].Row1 >= g_iCornerReflectionRowL[railType] && vCRs[CH_E][crE[k]].Row2 <= g_iCornerReflectionRowH[railType])
		{
			SetUsedFlag(vCRs[CH_E][crE[k]], 1);
		}
	}
#pragma endregion 

#pragma region 左侧倒打下裂纹
	if (isExistScrewHoles[3] == false && count > 0)
	{
		int tr = stepFLeft;
		int tl = stepFLeft + 1.0 * dScrewDistance[railType][3] / g_fileHead.step;
		VINT crD;
		GetCR(CH_D, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType] + 10, blocks, vCRs[CH_D], crD, -1, 3, true);

		HolePara hp;
		GetNearestHole((tr + tl) / 2, PM_SCREWHOLE, hp);

		VCR vReversed;
		for (int i = 0; i < crD.size(); ++i)
		{
			CR& td = vCRs[CH_D][crD[i]];
			int lastRow = 0, firstRow = 0, lastRow2 = 0, firstRow2 = 0;
			GetCRRowInfo3(td, firstRow, lastRow, CH_E, firstRow2, lastRow2);
			if (firstRow > lastRow)
			{
				vReversed.emplace_back(td);
			}

			if (td.Row2 <= hp.tempD.Row2 + 2 && Abs(hp.step2 - td.Step2) < 150)
			{
				SetScrewHoleFlag(td, 1);
				SetUsedFlag(td, 1);
			}
		}


		for (int i = vReversed.size() - 1; i >= 0; --i)
		{
			if (hp.tempD.Row2 >= vReversed[i].Row1 && hp.tempD.Row1 <= vReversed[i].Row1)
			{
				SetUsedFlag(vCRs[CH_D][vReversed[i].Index], 1);
				SetScrewHoleFlag(vCRs[CH_D][vReversed[i].Index], 1);
				vReversed.erase(vReversed.begin() + i);
				break;
			}
		}

		if (vReversed.size() > 0)
		{
			int step3d = 0;
			if (isExistScrewHoles[2])
			{
				for (int i = vPMs.size() - 1; i >= 0; --i)
				{
					if (vPMs[i].Mark == PM_SCREWHOLE && (int)vPMs[i].Data == 9)
					{
						step3d = vPMs[i].Step2;
						break;
					}
					if (tr - vPMs[i].Step2 >= 10000)
					{
						break;
					}
				}
			}


			int iMaxRow = vReversed[0].Row1;
			int iMaxIndex = 0;
			for (int i = 1; i < vReversed.size(); ++i)
			{
				if (vReversed[i].Step1 < stepFLeft || vReversed[i].Row2 > iLuokong_D_Row1_H[railType] + 10)
				{
					continue;
				}
				else if (vReversed[i].Row1 > iMaxRow)
				{
					iMaxRow = vReversed[i].Row1;
					iMaxIndex = i;
				}
			}

			if (iMaxRow > iLastScrewHoleRow2 + 2)
			{
				Wound_Judged w;
				w.IsScrewHole = -1 + 10;
				FillWound(w, g_vBlockHeads[vReversed[iMaxIndex].Block].BlockHead, g_fileHead);
				w.Block = vReversed[iMaxIndex].Block;
				w.Step = vReversed[iMaxIndex].Step;
				w.Step2 = vReversed[iMaxIndex].Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, g_fileHead.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔倒打三象限斜裂纹", -1);
				w.Type = W_SCREW_CRACK3;
				w.Place = WP_WAIST;
				w.SizeX = (vReversed[iMaxIndex].Row2 - vReversed[iMaxIndex].Row1) / 0.8;
				w.SizeY = 1;
				AddWoundData(w, vReversed[iMaxIndex]);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				SetUsedFlag(vCRs[CH_D][vReversed[iMaxIndex].Index], 1);
			}
		}
	}
#pragma endregion

#pragma region 右侧倒打下裂纹
	if (isExistScrewHoles[2] == false && isExistScrewHoles[3])
	{
		int tr = stepFLeft - 12;
		int tl = stepFLeft - 5;
		VINT crE;
		GetCR(CH_E, tr, iLuokong_D_Row1_L[railType], tl, iLuokong_D_Row1_H[railType] + 10, blocks, vCRs[CH_E], crE, -1, 3, true);

		HolePara hp;
		GetNearestHole((tr + tl) / 2, PM_SCREWHOLE, hp);

		VCR vReversed;
		for (int i = 0; i < crE.size(); ++i)
		{
			CR& te = vCRs[CH_E][crE[i]];
			int lastRow = 0, firstRow = 0, lastRow2 = 0, firstRow2 = 0;
			GetCRRowInfo3(te, firstRow, lastRow, CH_E, firstRow2, lastRow2);
			if (firstRow < lastRow)
			{
				vReversed.emplace_back(te);
			}

			if (te.Row2 <= hp.tempE.Row2 + 2 && Abs(hp.step2 - te.Step2) < 150)
			{
				SetScrewHoleFlag(te, 1);
				SetUsedFlag(te, 1);
			}
		}

		for (int i = vReversed.size() - 1; i >= 0; --i)
		{
			if (hp.tempE.Row2 >= vReversed[i].Row1 && hp.tempE.Row1 <= vReversed[i].Row1 || hp.tempE.Step2 == 0 && vReversed[i].Row2 <= (std::max)(hp.tempF.Row2, hp.tempD.Row2))
			{
				SetUsedFlag(vCRs[CH_E][vReversed[i].Index], 1);
				SetScrewHoleFlag(vCRs[CH_E][vReversed[i].Index], 1);
				vReversed.erase(vReversed.begin() + i);
				break;
			}
		}

		if (vReversed.size() > 0)
		{

			int step3d = 0;
			if (isExistScrewHoles[2])
			{
				for (int i = vPMs.size() - 1; i >= 0; --i)
				{
					if (vPMs[i].Mark == PM_SCREWHOLE && (int)vPMs[i].Data == 11)
					{
						step3d = vPMs[i].Step2;
						break;
					}
					if (tr - vPMs[i].Step2 >= 10000)
					{
						break;
					}
				}
			}

			int iMaxRow = vReversed[0].Row1;
			int iMaxIndex = 0;
			for (int i = 1; i < vReversed.size(); ++i)
			{
				if (vReversed[i].Step1 > stepFRight || vReversed[i].Row2 > iLuokong_D_Row1_H[railType] + 10)
				{
					continue;
				}
				else if (vReversed[i].Row1 > iMaxRow)
				{
					iMaxRow = vReversed[i].Row1;
					iMaxIndex = i;
				}
			}

			Wound_Judged w;
			w.IsScrewHole = -1 + 10;
			FillWound(w, g_vBlockHeads[vReversed[iMaxIndex].Block].BlockHead, g_fileHead);
			w.Block = vReversed[iMaxIndex].Block;
			w.Step = vReversed[iMaxIndex].Step;
			w.Step2 = vReversed[iMaxIndex].Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, g_fileHead.step, g_direction);
			w.Degree = WD_SERIOUS;
			sprintf(w.Result, "%d孔倒打四象限斜裂纹", 1);
			w.Type = W_SCREW_CRACK4;
			w.Place = WP_WAIST;
			w.SizeX = (vReversed[iMaxIndex].Row2 - vReversed[iMaxIndex].Row1) / 0.8;
			w.SizeY = 1;
			AddWoundData(w, vReversed[iMaxIndex]);
			w.IsCurve = (blocks[w.Block - g_iBeginBlock].vBStepDatas[w.Step].Mark.Mark & CURVE) ? 1 : 0;
			w.IsBridge = (blocks[w.Block - g_iBeginBlock].vBStepDatas[w.Step].Mark.Mark & QIAO) ? 1 : 0;
			w.IsTunnel = blocks[w.Block - g_iBeginBlock].BlockHead.gpsInfor[0] == 'V';
			AddToWounds(vWounds, w);
			SetUsedFlag(vCRs[CH_E][vReversed[iMaxIndex].Index], 1);
		}
	}
#pragma endregion

#pragma region FG出波
	//int tr = stepFRight + 1.0 * dScrewDistance[railType][1] / g_fileHead.step - g_screwholeRange;
	//int tl = stepFLeft + 1.0 * dScrewDistance[railType][4] / g_fileHead.step + g_screwholeRange;
	VINT crF, crG;
	GetCR(CH_F, stepFRight - 15, 0, stepFLeft + 15, iFRow - 3, blocks, vCRs[CH_F], crF);
	GetCR(CH_G, stepFRight - 15, 0, stepFLeft + 15, iFRow - 3, blocks, vCRs[CH_G], crG);
	RemoveHoleCR(vCRs[CH_F], crF);
	RemoveHoleCR(vCRs[CH_G], crG);
	for (int i = 0; i < crF.size(); ++i)
	{
		GetCRInfo(vCRs[CH_F][crF[i]], DataA, blocks, 0);
		ParseHorizonalCrack(g_fileHead, DataA, blocks, vCRs, vCRs[CH_F][crF[i]], direction, carType, iFRow, vWounds, true, false);
	}

	for (int i = 0; i < crG.size(); ++i)
	{
		GetCRInfo(vCRs[CH_G][crG[i]], DataA, blocks, 0);
		ParseHorizonalCrack(g_fileHead, DataA, blocks, vCRs, vCRs[CH_G][crG[i]], direction, carType, iFRow, vWounds, true, false);
	}
#pragma endregion

#pragma region 判断接头处水平裂纹
	wound.SetEmpty();
	hasWound = false;
	VCR vFG;
	for (int i = CH_F; i <= CH_G; i++)
	{
		VINT vcr;
		GetCR(i, stepFRight - 4, 0, stepFLeft + 4, iFRow - 3, blocks, vCRs[i], vcr, -1, 1, false);
		for (int j = 0; j < vcr.size(); ++j)
		{
			vFG.emplace_back(vCRs[i][vcr[j]]);
			GetCRInfo(vCRs[i][vcr[j]], DataA, blocks, 0);
			ParseHorizonalCrack(g_fileHead, DataA, blocks, vCRs, vCRs[i][vcr[j]], direction, carType, iFRow, vWounds, true, false);
		}
	}

	int stepFind1 = 0x7FFFFFFF, stepFind2 = 0;
	for (int i = 0; i < vFG.size(); ++i)
	{
		if (vFG[i].Step1 < stepFind1)
		{
			stepFind1 = vFG[i].Step1;
		}
		if (vFG[i].Step2 > stepFind2)
		{
			stepFind2 = vFG[i].Step2;
		}
	}
	if (vFG.size() > 0)
	{
		stepFLeft = (std::min)(stepFind2, stepFLeft);
		stepFRight = (std::max)(stepFind1, stepFRight);
	}

	int dStepLose = stepFLeft - stepFRight;
	if (crFLose.size() > 0 && crGLose.size() > 0 && pm.IsHalf == 0)
	{
		int stepL = 0, stepR = 0;
		GetOverlappedStep(vCRs[CH_F][crFLose[0]].Step1, vCRs[CH_F][crFLose[0]].Step2, vCRs[CH_G][crGLose[0]].Step1, vCRs[CH_G][crGLose[0]].Step2, stepR, stepL);
		dStepLose = stepL - stepR;

		if (dStepLose >= 18)
		{
			Pos pos = FindStepInBlock(stepR, g_vBlockHeads, 0);
			CR cLoseF, cLoseG;
			cLoseF.Channel = CH_F; cLoseF.Step1 = stepR;
			cLoseG.Channel = CH_G; cLoseG.Step1 = stepR;
			for (int i = 0; i < dStepLose; ++i)
			{
				WaveData wd;
				wd.block = pos.Block;
				wd.step = stepR + i;
				wd.find = 0;
				wd.row = iFRow;
				cLoseF.Region.emplace_back(wd);
				cLoseG.Region.emplace_back(wd);
			}
			cLoseF.IsLose = 1;
			cLoseG.IsLose = 1;
			FillCR(cLoseF);
			FillCR(cLoseG);
			if (!hasWound)
			{
				Pos pos = FindStepInBlock(stepFRight, g_vBlockHeads, 0);
				wound.IsJoint = 1;
				wound.IsSew = 0;
				wound.Walk = wd;
				wound.Block = pos.Block;
				wound.Step = pos.Step;
				wound.Step2 = stepFRight;
				wound.SizeX = g_fileHead.step * dStepLose;
				wound.SizeY = 1;
				FillWound(wound, g_vBlockHeads[pos.Block].BlockHead, g_fileHead);
				memcpy(wound.Result, "接头伤损", 30);
				wound.Type = W_VERTICAL_CRACK;
				wound.Place = WP_HEAD_MID;
				wound.Degree = WD_SERIOUS;
				wound.According.emplace_back("FG底部失波宽度过宽，可能有轨面缺陷/水平裂纹/斜裂纹/纵向裂纹");
				FillWound2(wound, blocks);
				AddWoundData(wound, cLoseF);
				AddWoundData(wound, cLoseG);
				AddToWounds(vWounds, wound);
			}
			else
			{
				AddWoundData(vWounds[vWounds.size() - 1], cLoseF);
				AddWoundData(vWounds[vWounds.size() - 1], cLoseG);
				vWounds[vWounds.size() - 1].According.emplace_back("FG底部失波宽度过宽，可能有轨面缺陷/水平裂纹/斜裂纹/纵向裂纹");
				vWounds[vWounds.size() - 1].Place = WP_HEAD_MID;
			}
		}
	}

#pragma endregion

	return count;
}


void	ParseHorizonalCrack(F_HEAD& g_fileHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, uint32_t step1, uint32_t step2, bool& direction, bool& carType, int16_t& iFRow, VWJ& vWounds, uint8_t isJoint/* = 0*/, uint8_t isSew /* = 0 */)
{
	VINT crF, crG;
	bool bFindF = GetCR(CH_F, step1 - 10, 0, step2 + 10, iFRow - 3, blocks, vCRs[CH_F], crF);
	bool bFindG = GetCR(CH_G, step1 - 10, 0, step2 + 10, iFRow - 3, blocks, vCRs[CH_G], crG);
	if (bFindF || bFindG)
	{
		int rowF = iFRow, rowG = iFRow;
		int idxF = -1, idxG = -1;
		for (int i = 0; i < crF.size(); ++i)
		{
			if (vCRs[CH_F][crF[i]].Row1 < rowF)
			{
				rowF = vCRs[CH_F][crF[i]].Row1;
				idxF = crF[i];
			}
		}

		for (int i = 0; i < crG.size(); ++i)
		{
			if (vCRs[CH_G][crG[i]].Row1 < rowG)
			{
				rowG = vCRs[CH_G][crG[i]].Row1;
				idxG = crG[i];
			}
		}

		int iLoseCh = -1;
		CR cLose;
		if (rowF <= rowG && rowF < iFRow)
		{
			iLoseCh = CH_F;
			cLose = vCRs[CH_F][idxF];
		}
		else if (rowG < rowF && rowG < iFRow)
		{
			iLoseCh = CH_G;
			cLose = vCRs[CH_G][idxG];
		}
		if (iLoseCh > 0)
		{
			uint8_t iChA = GetAChannelByBChannel(cLose.Channel);
			double angle = 0.1 * g_fileHead.deviceP2.Angle[iChA].Refrac;
			double offset = g_fileHead.deviceP2.Place[iChA] + blocks[cLose.Block - g_iBeginBlock].BlockHead.probOff[iChA];
			GetCRInfo(cLose, DataA, blocks);
			ParseHorizonalCrack(g_fileHead, DataA, blocks, vCRs, cLose, direction, carType, iFRow, vWounds, isJoint, isSew);
		}
		SetUsedFlag(vCRs[CH_F], crF, 1);
		SetUsedFlag(vCRs[CH_G], crG, 1);
	}
}


void	ParseHorizonalCrack(F_HEAD& g_fileHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, bool& direction, bool& carType, int16_t& iFRow, VWJ& vWounds, uint8_t isJoint/* = 0*/, uint8_t isSew /* = 0 */)
{
	if (cr.IsUsed == 1 || (cr.Step2 - cr.Step1 < 2 && cr.Region.size() <= 3 && isJoint == false && isSew == false))
	{
		return;
	}

	SetUsedFlag(cr, 1);

	BLOCK blockHead = blocks[cr.Block - g_iBeginBlock].BlockHead;
	Wound_Judged wound;
	FillWound(wound, blockHead, g_fileHead);
	wound.Block = cr.Block;
	wound.Step = cr.Step;
	wound.Step2 = cr.Step1;
	wound.IsJoint = isJoint;
	wound.IsSew = isSew;
	wound.Walk = GetWD(blockHead.walk, wound.Step, g_fileHead.step, direction);

	WaveData t_wd = cr.Region[0];
	uint8_t m = cr.Channel;
	uint8_t iChA = GetAChannelByBChannel(m);


	CR_INFO_A& crInfo = cr.Info;
	std::vector<A_Step>& vASteps = cr.vASteps;

	uint8_t iChR = rcs[cr.Channel];
	VINT crR;
	int maxVR = 0;
	bool bFindR = GetCR(iChR, cr.Step1 - 2, cr.Row1 - 1, cr.Step2 + 1, cr.Row2 + 1, blocks, vCRs[iChR], crR);
	bFindR = RemoveHoleCR(vCRs[iChR], crR);
	for (int i = 0; i < crR.size(); ++i)
	{
		GetCRInfo(vCRs[iChR][crR[i]], DataA, blocks, 0);
		int tp = (std::max)((int)vCRs[iChR][crR[i]].Info.MaxV, (int)vCRs[iChR][crR[i]].Info.MaxV2);
		maxVR = (std::max)(tp, maxVR);
	}

	int iJawRow = g_iJawRow[blocks[cr.Block - g_iBeginBlock].BlockHead.railType & 0x03];
	//轨颚裂纹（最多） 在1.2­~1.5之间有半幅以上回波，底波波幅减弱或消失，在接头处有D或E做辅助校验；判重伤；报告：轨颚水平裂纹。。mm
	if (t_wd.row >= iJawRow - 3 && t_wd.row <= iJawRow + 3)
	{
		bool bHalfWave = (cr.Info.MaxV >= g_iAmpl / 2 || cr.Info.MaxV2 >= g_iAmpl / 2 || cr.IsContainA == 0);
		if (bHalfWave || isJoint || isSew || maxVR >= 75)
		{
			memcpy(wound.Result, "轨颚水平裂纹", 30);
			wound.Type = W_HORIZONAL_CRACK;
			wound.Place = WP_JAW_IN;
			wound.Degree = WD_SERIOUS;
			wound.SizeX = g_fileHead.step * (cr.Step2 - cr.Step1);
			wound.SizeY = 1;
			AddWoundData(wound, cr);
			AddWoundData(wound, vCRs[iChR], crR);
		}
	}
	//轨头水平裂纹（其次）
	else if (t_wd.row < iJawRow - 3)
	{
		bool bHalfWave = (cr.Info.MaxV >= g_iAmpl / 2 || cr.Info.MaxV2 >= g_iAmpl / 2 || cr.IsContainA == 0);
		VINT cr1, cr2;
		if (bHalfWave || isJoint || isSew || maxVR >= g_iAmpl / 2)
		{
			memcpy(wound.Result, "轨头水平裂纹", 30);
			wound.Type = W_HORIZONAL_CRACK;
			wound.Place = WP_HEAD_MID;
			wound.SizeX = g_fileHead.step * (cr.Step2 - cr.Step1);
			wound.SizeY = 1;
			wound.Degree = wound.SizeX >= 30 ? WD_SERIOUS : WD_SMALL;
			AddWoundData(wound, cr);
			AddWoundData(wound, vCRs[iChR], crR);
		}
	}
	//轨腰水平裂纹
	else if (t_wd.row < iFRow - 3 && t_wd.find)
	{
		bool bHalfWave = (cr.Info.MaxV >= g_iAmpl / 2 || cr.Info.MaxV2 >= g_iAmpl / 2 || cr.IsContainA == 0);
		if (bHalfWave || isJoint || isSew || maxVR >= 75)
		{
			memcpy(wound.Result, "轨腰水平裂纹", 30);
			wound.Type = W_HORIZONAL_CRACK;
			wound.Place = WP_WAIST;
			wound.SizeX = g_fileHead.step * (cr.Step2 - cr.Step1);
			wound.SizeY = 1;
			wound.Degree = wound.SizeX >= 30 ? WD_SERIOUS : WD_SMALL;
			AddWoundData(wound, cr);
			AddWoundData(wound, vCRs[iChR], crR);
		}
	}
	else if (t_wd.row >= iFRow - 3 && cr.Step2 - cr.Step1 >= 11 && cr.Step2 - cr.Step1 <= 1000)
	{
		if (!bFindR)
		{
			wound.Type = 0;
		}
		else
		{
			int indexR = crR[0];
			int gap = vCRs[iChR][indexR].Step2 - vCRs[iChR][indexR].Step1;
			for (int i = 1; i < crR.size(); ++i)
			{
				if (vCRs[iChR][crR[i]].Step2 - vCRs[iChR][crR[i]].Step1 > gap)
				{
					indexR = crR[i];
					gap = vCRs[iChR][crR[i]].Step2 - vCRs[iChR][crR[i]].Step1;
				}
			}

			CR& tpR = vCRs[iChR][indexR];
			int begin = 0, end = 0;
			int count = GetOverlappedStep(cr.Step1, cr.Step2, tpR.Step1, tpR.Step2, begin, end);

			VINT crFind[14];
			uint8_t findChannel = 0;
			for (int i = 0; i < CH_F; ++i)
			{
				if (i == CH_d || i == CH_e)
				{
					continue;
				}
				findChannel += GetCR(i, begin - 20, 0, end + 20, iFRow - 4, blocks, vCRs[i], crFind[i], -1, 1, false);
			}

			if (count >= 11 && count <= 1000 /*&& findChannel > 0*/)
			{
				Pos pos = FindStepInBlock(begin, g_vBlockHeads, cr.Block - g_iBeginBlock - 1);
				wound.Block = pos.Block;
				wound.Step = pos.Step;
				wound.Step2 = pos.Step2;
				memcpy(wound.Result, "纵向裂纹", 30);
				wound.Type = W_VERTICAL_CRACK;
				wound.Place = WP_HEAD_MID;
				wound.SizeX = g_fileHead.step * count;
				wound.SizeY = 1;
				wound.Degree = wound.SizeX >= 30 ? WD_SERIOUS : WD_SMALL;
				wound.Row1 = cr.Row1;

				CR crFA, crGA;
				for (int k = begin; k < end; ++k)
				{
					WaveData wd;
					wd.block = pos.Block;
					wd.find = 0;
					wd.row = iFRow;
					wd.step = k;
					crFA.Region.emplace_back(wd);
					crGA.Region.emplace_back(wd);
				}
				crFA.Channel = CH_F;
				crGA.Channel = CH_G;

				if (cr.Channel == CH_F)
				{
					crFA.Index = cr.Index;
					for (int j = 0; j < crR.size(); ++j)
					{
						int ss = 0, ee = 0;
						if (GetOverlappedStep(vCRs[CH_G][crR[j]].Step1, vCRs[CH_G][crR[j]].Step2, begin, end, ss, ee) > 0)
						{
							crGA.Index = vCRs[CH_G][crR[j]].Index;
							break;
						}
					}
				}
				else
				{
					crGA.Index = cr.Index;
					for (int j = 0; j < crR.size(); ++j)
					{
						int ss = 0, ee = 0;
						if (GetOverlappedStep(vCRs[CH_F][crR[j]].Step1, vCRs[CH_F][crR[j]].Step2, begin, end, ss, ee) > 0)
						{
							crFA.Index = vCRs[CH_F][crR[j]].Index;
							break;
						}
					}
				}

				crFA.Block = pos.Block; crFA.Step = pos.Step;
				crGA.Block = pos.Block; crGA.Step = pos.Step;
				FillCR(crFA);
				FillCR(crGA);
				crFA.IsLose = 1;
				crGA.IsLose = 1;

				for (int i = 0; i < CH_F; ++i)
				{
					RemoveHoleCR(vCRs[i], crFind[i]);
					RemovePMCR(vCRs[i], crFind[i]);
					AddWoundData(wound, vCRs[i], crFind[i]);
				}

				AddWoundData(wound, crFA);
				AddWoundData(wound, crGA);
				wound.According.emplace_back("此处FG失波，可能有轨面缺陷/水平裂纹/斜裂纹/纵向裂纹");
			}
		}
	}

	if (wound.Type > 0)
	{
		//if (isJoint)
		//{
		VINT crD, crE;
		bool bFindD = GetCR(CH_D, cr.Step1, cr.Row1, cr.Step2, cr.Row2, blocks, vCRs[CH_D], crD);
		bool bFindE = GetCR(CH_E, cr.Step1, cr.Row1, cr.Step2, cr.Row2, blocks, vCRs[CH_E], crE);
		RemoveHoleCR(vCRs[CH_D], crD);
		RemoveHoleCR(vCRs[CH_E], crE);
		AddWoundData(wound, vCRs[CH_D], crD);
		AddWoundData(wound, vCRs[CH_E], crE);
		SetUsedFlag(vCRs[CH_D], crD, 1);
		SetUsedFlag(vCRs[CH_E], crE, 1);
		//if (bFindD)	wound.According.emplace_back("D端角反射");
		//if (bFindE)	wound.According.emplace_back("E端角反射");
	//}
		FillWound2(wound, blocks);
		AddToWounds(vWounds, wound);
		SetUsedFlag(vCRs[iChR], crR, 1);

		VINT cLowerF, cLowerG;
		GetCR(CH_F, cr.Step1, cr.Row2 + 2, cr.Step2, iFRow - 3, blocks, vCRs[CH_F], cLowerF);
		for (int i = 0; i < cLowerF.size(); ++i)
		{
			if (vCRs[CH_F][cLowerF[i]].IsLose == 0)
			{
				SetUsedFlag(vCRs[CH_F][cLowerF[i]], 1);
			}
		}

		GetCR(CH_G, cr.Step1, cr.Row2 + 2, cr.Step2, iFRow - 3, blocks, vCRs[CH_G], cLowerG);
		for (int i = 0; i < cLowerG.size(); ++i)
		{
			if (vCRs[CH_G][cLowerG[i]].IsLose == 0)
			{
				SetUsedFlag(vCRs[CH_G][cLowerG[i]], 1);
			}
		}
	}
}

bool ParseHS(BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, uint8_t& railType, bool& direction, bool& carType, int16_t& iFRow, double& wd, VWJ& vWounds, VPM& vPMs, uint8_t isJoint, uint8_t isSew, uint8_t isScrewHole, uint8_t isGuideHole)
{
	int m = cr.Channel;
	if (cr.IsUsed)
	{
		return false;
	}

	bool isAngle = false;
	CR oldCR = cr;
	if (cr.Step2 - cr.Step1 >= 5 && cr.Row2 - cr.Row1 < 0.8 * (cr.Step2 - cr.Step1))
	{
		CR temp = cr;
		for (int i = temp.Region.size() - 1; i >= 0; --i)
		{
			if (temp.Region[i].row == temp.Row2)
			{
				temp.Region.erase(temp.Region.begin() + i);
			}
		}
		FillCR(temp);
		if (temp.Step2 - temp.Step1 >= 5 && temp.Row2 - temp.Row1 >= 3 && temp.Row2 - temp.Row1 < 0.8 * (temp.Step2 - temp.Step1))
		{
			bool isMightDirty = false;
			CR cr1, cr2;
			if (IsAngleCR2(temp, blocks, cr1, cr2))
			{
				isAngle = true;
				if (cr.Channel == CH_A1 || cr.Channel == CH_B1)
				{
					if (cr1.Step2 - cr1.Step1 >= 5 && cr1.Row2 - cr1.Row1 >= 3 && cr1.Row2 - cr1.Row1 < 0.8 * (cr1.Step2 - cr1.Step1))
					{
						isMightDirty = true;
					}
				}
				else if (cr.Channel == CH_a1 || cr.Channel == CH_b1)
				{
					if (cr2.Step2 - cr2.Step1 >= 5 && cr2.Row2 - cr2.Row1 >= 3 && cr2.Row2 - cr2.Row1 < 0.8 * (cr2.Step2 - cr2.Step1))
					{
						isMightDirty = true;
						if (cr2.Row2 - cr2.Row1 < 0.5 * (cr2.Step2 - cr2.Step1))
						{
							SetUsedFlag(cr, 1);
							return false;
						}
					}
				}
			}
			else
			{
				isMightDirty = true;
			}

			if (isMightDirty)
			{
				int totalCount = (cr.Row2 - cr.Row1 + 1) * (cr.Step2 - cr.Step1 + 1);
				std::sort(cr.Region.begin(), cr.Region.end());
				int ptCount = 1;
				for (int i = 1; i < cr.Region.size(); ++i)
				{
					if (false == (cr.Region[i] == cr.Region[i - 1]))
					{
						ptCount++;
					}
				}
				if (1.0 * ptCount / totalCount < 0.3)
				{
					return false;
				}
			}
		}
	}

	if (cr.IsReversed == 1 && cr.Channel < CH_C)
	{
		bool isInFork = (blocks[cr.Block - g_iBeginBlock].vBStepDatas[cr.Step].Mark.Mark & FORK) > 0;

		int iF1 = cr.Step1;
		int iF2 = cr.Step2 + 15;
		if (m == CH_A1 || m == CH_B1)
		{
			iF1 = cr.Step1 - 15;
			iF2 = cr.Step2;
		}
		if (isInFork && IsExistScrewHole(g_filehead, DataA, blocks, vCRs, iF1, iF2, iFRow, railType, vWounds, vPMs, isJoint, false, isSew, 0))
		{
			SetUsedFlag(cr, 1);
			return false;
		}
	}

	int iBlock = cr.Region[0].block - g_iBeginBlock;
	uint8_t iChA = GetAChannelByBChannel(m);//该通道对应的A通道
	BLOCK& blockHead = blocks[iBlock].BlockHead;
	int offset = g_filehead.deviceP2.Place[iChA] + blockHead.probOff[iChA];
	double angle = 0.1 * g_filehead.deviceP2.Angle[iChA].Refrac;

	bool bFindFrames = cr.vASteps.size() > 0;


	Wound_Judged wound;
	wound.IsJoint = isJoint;
	wound.IsSew = isSew;
	wound.Walk = wd;
	wound.Block = cr.Block;
	wound.Step = cr.Step;
	wound.Step2 = cr.Step1;

	bool isWound = false;
	//判定核伤的幅值下限
	int hsAmpl = 120;


	// BIT0~1: 轨型(0为43轨，1-50，2-60，3-75), BIT4:0逆里程、1顺里程，BIT5:0右股、1左股，BIT6~7：单线、上行、下行，其他预留
	//int railType = blockHead.railType & 0x03;
	if (m == CH_A1 || m == CH_a1 || m == CH_B1 || m == CH_b1)
	{
		if (cr.Row1 > g_iJawRow[railType] && 2 * g_iJawRow[railType] - cr.Row2 <= 1)
		{
			if (IsHoleCR(cr, vCRs, blocks, g_iJawRow[railType], 2 * g_iJawRow[railType], iFRow))
			{
				SetUsedFlag(cr, 1);
				return 0;
			}
		}

		FillWound(wound, blockHead, g_filehead);
		AddWoundData(wound, cr);

		bool bFront = (m == CH_A1 || m == CH_B1 || m == CH_C);
		uint8_t iCh_R = rcs[m];
		VINT t_cr, t_cr2, t_cr3;;
		uint8_t bFindR = false, bFindR2 = false, bFindR3 = false;
		if (isJoint == 0 && isSew == 0)
		{
			bFindR = GetCR(iCh_R, cr.Step1 - 5, (std::max)(cr.Row1 - 2, 0), cr.Step2 + 5, cr.Row2 + 2, blocks, vCRs[iCh_R], t_cr, -1, 2);
			bFindR2 = GetCR(iCh_R, cr.Step1 - 5, (std::max)(2 * g_iJawRow[railType] - cr.Row2 - 1, 0), cr.Step2 + 5, 2 * g_iJawRow[railType] - cr.Row1 + 1, blocks, vCRs[iCh_R], t_cr2, t_cr, 2);
			if (cr.Row1 > g_iJawRow[railType])//一次波
			{
				bFindR3 = GetCR(cr.Channel, cr.Step1 - 5, (std::min)(cr.Row1 - 5, 0), cr.Step2 + 5, 2 * g_iJawRow[railType] - cr.Row1 + 1, blocks, vCRs[iCh_R], t_cr3, t_cr, 2);
			}
		}

		if (cr.Row1 <= g_iJawRow[railType] + 2 && cr.Row2 >= g_iJawRow[railType] - 2)//轨颚连贯伤
		{
			if ((crInfo.MaxV >= g_iAmpl || cr.IsContainA == 0) && crInfo.Shift >= 25 || cr.Row2 - cr.Row1 >= 7 || bFindR || bFindR2)
			{
				//对照 对向二次波 A2 <=> a2
				for (int i = 0; i < t_cr.size(); ++i)
				{
					GetCRInfo(vCRs[iCh_R][t_cr[i]], DataA, blocks);
				}
				wound.Degree = WD_SMALL;
				if (crInfo.Shift >= 50)
				{
					wound.Degree = WD_SERIOUS;
				}
				if ((carType == true && (m == CH_A1 || m == CH_a1)) || (carType == false && (m == CH_B1 || m == CH_b1)))
				{
					memcpy(wound.Result, "轨头内侧连贯核伤", 30);
					wound.Type = W_HS; wound.Place = WP_HEAD_IN;
				}
				else
				{
					memcpy(wound.Result, "轨头外侧连贯核伤", 30);
					wound.Type = W_HS; wound.Place = WP_HEAD_OUT;
				}
				if (bFindR || bFindR2)	strcat(wound.Result, ",竖直横核伤");

				wound.SizeX = 1.5 * (cr.Row2 - cr.Row1);
				wound.SizeY = 1.33 * (cr.Step2 - cr.Step1);
				//sprintf(tempAccording, "%s出波，位移：%.1f大格，幅值%d;", ChannelNamesB[m].c_str(), 0.02f * crInfo.Shift, crInfo.MaxV);
				//wound.According.emplace_back(tempAccording);
				//if (bFindR)sprintf(wound.According, _T("%s出波，位移：%.1f大格，幅值%d; %s出波"), ChannelNamesB[m], 0.02f * crInfo.Shift, crInfo.MaxV, ChannelNamesB[iCh_R]);
				AddWoundData(wound, vCRs[iCh_R], t_cr);
				FillWound2(wound, blocks);
				AddToWounds(vWounds, wound);
				SetUsedFlag(vCRs[iCh_R], t_cr, 1);
				isWound = true;
			}
		}
		else if (cr.Row1 >= g_iJawRow[railType])//纯二次波
		{
#pragma region 2020年3月14 
			/*
			if (cr.Row2 >= 2 * g_iJawRow[railType] - 4)
			{
				wound.Type = W_HS;
				VINT crF, crG;
				int iF1 = cr.Step1;
				int iF2 = cr.Step2 + 15;
				if (m == CH_A1 || m == CH_B1)
				{
					iF1 = cr.Step1 - 15;
					iF2 = cr.Step2;
				}

				bool bScrew = false;
				if (isScrewHole == 0)
				{
					bScrew = ParseScrewHole(g_fileHead, DataA, blocks, vCRs, iF1, iF2, iFRow, railType, vWounds, vPMs, isJoint, false, isSew);
				}
				if (isScrewHole || bScrew)//螺孔回波
				{
					bool bOK = true;
					if (isScrewHole || bScrew)
					{
						if (cr.Row1 >= iLastScrewHoleRow - 5 && cr.Row2 <= iLastScrewHoleRow2 + 3)
						{

						}
						else
						{
							bOK = false;
						}
					}
					if (bOK == true)
					{
						SetUsedFlag(cr, 1);
					}
					if (cr.IsUsed == 1)
					{
						return false;
					}
				}
			}
			*/
#pragma endregion				

			if (bFindFrames || cr.IsContainA == 0)//在A超中能找到对应的帧数据
			{
				//对照 对向二次波 A2 <=> a2, 本通道一次波
				uint8_t iCh_R = rcs[m];
				VINT t_cr, t_crFirst;
				uint8_t bFindR = false;
				if (isJoint || isSew)
				{
					bFindR = GetCR(iCh_R, cr.Step1 - 2, cr.Row1, cr.Step2 + 2, cr.Row2, blocks, vCRs[iCh_R], t_cr, -1, -1, true);
				}
				else
				{
					bFindR = GetCR(iCh_R, cr.Step1 - 2, cr.Row1, cr.Step2 + 2, cr.Row2, blocks, vCRs[iCh_R], t_cr);
				}

				for (int i = 0; i < t_cr.size(); ++i)
					GetCRInfo(vCRs[iCh_R][t_cr[i]], DataA, blocks);

				uint8_t	bFindFirst = false;
				if (isJoint || isSew)
				{
					bFindFirst = GetCR(m, cr.Step1 - 2, 25 - cr.Row2, cr.Step2 + 2, 27 - cr.Row1, blocks, vCRs[m], t_crFirst, -1, -1, true);
				}
				else
				{
					bFindFirst = GetCR(m, cr.Step1 - 2, 25 - cr.Row2, cr.Step2 + 2, 27 - cr.Row1, blocks, vCRs[m], t_crFirst);
				}

				for (int i = 0; i < t_crFirst.size(); ++i)
					GetCRInfo(vCRs[m][t_crFirst[i]], DataA, blocks);


				//二次回波位移2大格以上判定重伤，1.5大格是轻伤
				if ((crInfo.MaxV >= g_iAmpl || cr.IsContainA == 0) && (crInfo.d1_4 >= 25 && crInfo.iSection == 0 || crInfo.d4_7 >= 45 && crInfo.iSection == 1 || crInfo.Shift >= 90 && crInfo.iSection == 2) || cr.Row2 - cr.Row1 >= 6)
				{
					memcpy(wound.Result, "轨头外侧轻伤", 30);
					wound.Type = W_HS;
					wound.Place = WP_HEAD_OUT;
					if ((carType == true && (m == CH_A1 || m == CH_a1)) || (carType == false && (m == CH_B1 || m == CH_b1)))
					{
						memcpy(wound.Result, "轨头内侧轻伤", 30);
						wound.Type = W_HS; wound.Place = WP_HEAD_IN;
					}

					wound.Degree = WD_SMALL;
					if (crInfo.Shift >= 90)
					{
						wound.Degree = WD_SERIOUS;
						memcpy(wound.Result, "轨头外侧重伤", 30); wound.Type = W_HS; wound.Place = WP_HEAD_OUT;
						if ((carType == true && (m == CH_A1 || m == CH_a1)) || (carType == false && (m == CH_B1 || m == CH_b1)))
						{
							memcpy(wound.Result, "轨头内侧重伤", 30); wound.Type = W_HS; wound.Place = WP_HEAD_IN;
						}
					}

					if (bFindR)	strcat(wound.Result, ",竖直横核伤");
					wound.SizeX = 1.5 * (cr.Row2 - cr.Row1);
					wound.SizeY = 1.33 * (cr.Step2 - cr.Step1);
					//sprintf(tempAccording, "%s二次出波，位移：%.1f大格，幅值%d", ChannelNamesB[m].c_str(), 0.02f * crInfo.Shift, crInfo.MaxV);
					//wound.According.emplace_back(tempAccording);
					AddWoundData(wound, vCRs[iCh_R], t_cr);
					AddWoundData(wound, vCRs[cr.Channel], t_crFirst);
					FillWound2(wound, blocks);
					AddToWounds(vWounds, wound);
					SetUsedFlag(vCRs[iCh_R], t_cr, 1);
					SetUsedFlag(vCRs[m], t_crFirst, 1);
					isWound = true;
				}
				else if ((crInfo.MaxV >= g_iAmpl || cr.IsContainA == 0) && crInfo.Shift >= 45)//二次波回波不到1.5大格
				{
					//对应的对向通道
					//一次波的话任意位移1大格加上对向（例如A对应a）二次波验证判重伤, 找对向一次波
					int iCh_R = rcs[m];
					VINT t_crFirst;
					uint8_t bFindFirst = GetCR(iCh_R, cr.Step1 - 5, 2 * g_iJawRow[railType] - cr.Row2 - 2, cr.Step2 + 5, 2 * g_iJawRow[railType] - cr.Row1 + 2, blocks, vCRs[iCh_R], t_crFirst);

					if (bFindFirst)
					{
						for (int i = 0; i < t_crFirst.size(); ++i)
						{
							CR& tpFirst = vCRs[iCh_R][t_crFirst[i]];
							GetCRInfo(tpFirst, DataA, blocks);
							if ((tpFirst.Info.MaxV >= g_iAmpl || tpFirst.IsContainA == 0) && tpFirst.Info.Shift >= 25)
							{
								wound.Type = W_HS;
								wound.Place = WP_HEAD_OUT;
								memcpy(wound.Result, "轨头外侧核伤", 30);
								if ((carType == true && (m == CH_A1 || m == CH_a1)) || (carType == false && (m == CH_B1 || m == CH_b1)))
								{
									wound.Type = W_HS;
									wound.Place = WP_HEAD_IN;
									memcpy(wound.Result, "轨头内侧核伤", 30);
								}

								wound.Degree = WD_SERIOUS;
								wound.SizeX = (cr.Row2 - cr.Row1) * 3;
								wound.SizeY = (cr.Step2 - cr.Step1) * 2.67;
								AddWoundData(wound, tpFirst);
								//sprintf(tempAccording, "%s一二次出波，位移：%.1f大格，幅值%d", ChannelNamesB[m].c_str(), 0.02f * crInfo.Shift, crInfo.MaxV);
								//wound.According.emplace_back(tempAccording);
								AddWoundData(wound, vCRs[iCh_R], t_crFirst);
								FillWound2(wound, blocks);
								AddToWounds(vWounds, wound);
								isWound = true;
								break;
							}
							SetUsedFlag(vCRs[iCh_R], t_crFirst, 1);
						}
					}
					else
					{
						//wound.Type = W_HS;
						//wound.Place = WP_HEAD_OUT;
						//memcpy(wound.Result, "轨头外侧核伤", 30);
						//if ((carType == true && (m == CH_A1 || m == CH_a1)) || (carType == false && (m == CH_B1 || m == CH_b1)))
						//{
						//	wound.Type = W_HS;
						//	wound.Place = WP_HEAD_IN;
						//	memcpy(wound.Result, "轨头内侧核伤", 30);
						//}

						//wound.Degree = WD_SMALL;
						//wound.SizeX = (cr.Row2 - cr.Row1) * 3;
						//wound.SizeY = (cr.Step2 - cr.Step1) * 2.67;
						//AddWoundData(wound, cr);
						//FillWound2(wound, blocks);
						//AddToWounds(vWounds, wound);
						//isWound = true;
					}
				}
			}
		}
		else if (cr.Row2 <= g_iJawRow[railType])////纯一次波
		{
			int shift = 25;
			if ((crInfo.MaxV >= g_iAmpl || cr.IsContainA == 0) && crInfo.Shift >= shift || cr.Row2 - cr.Row1 >= 7)
			{
				VINT t_crR, t_crSecond;
				int iCh_R = rcs[m];
				bool bFind = GetCR(rcs[iCh_R], cr.Step1 - 2, 25 - cr.Row2, cr.Step2 + 2, 27 - cr.Row1, blocks, vCRs[iCh_R], t_crR);//寻找对应通道二次波 A1 <=> a2
				bool bFindSecond = GetCR(rcs[iCh_R], cr.Step1 - 2, 25 - cr.Row2, cr.Step2 + 2, 27 - cr.Row1, blocks, vCRs[m], t_crSecond);//寻找本通道二次波 A1 <=> A2

				memcpy(wound.Result, "轨头外侧重伤", 30);
				wound.Type = W_HS;
				wound.Place = WP_HEAD_OUT;
				if ((carType == true && (m == CH_A1 || m == CH_a1)) || (carType == false && (m == CH_B1 || m == CH_b1)))
				{
					memcpy(wound.Result, "轨头内侧重伤", 30);
					wound.Type = W_HS;
					wound.Place = WP_HEAD_IN;
				}
				wound.SizeX = (cr.Row2 - cr.Row1) * 3;
				wound.SizeY = (cr.Step2 - cr.Step1) * 2.67;
				wound.Degree = WD_SERIOUS;
				AddWoundData(wound, cr);
				AddWoundData(wound, vCRs[iCh_R], t_crR);
				//sprintf(tempAccording, "%s一次出波，位移：%.1f大格，幅值%d", ChannelNamesB[m].c_str(), 0.02f * crInfo.Shift, crInfo.MaxV);
				//wound.According.emplace_back(tempAccording);
				FillWound2(wound, blocks);
				AddToWounds(vWounds, wound);

				SetUsedFlag(vCRs[iCh_R], t_crR, 1);
				SetUsedFlag(vCRs[m], t_crSecond, 1);
				isWound = true;
			}
		}

		/************************2020-05-22 增加核伤小出波检测*********************/
		if (isWound == false)
		{
			VINT tcr, tcrR;
			uint8_t b1 = GetCR(cr.Channel, cr.Step1 - 5, cr.Step2 + 5, blocks, vCRs[cr.Channel], tcr, cr.Index, 1, false);
			uint8_t b2 = GetCR(iCh_R, cr.Step1 - 5, cr.Step2 + 5, blocks, vCRs[iCh_R], tcrR, -1, 1, false);

			VINT tcr2, tcrR2;
			uint8_t b3 = GetCR(cr.Channel, cr.Step1 - 150, 0, cr.Step2 + 150, g_iJawRow[railType] * 2, blocks, vCRs[cr.Channel], tcr2, tcr, 1, false);
			RemoveFromVector(tcr2, cr.Index);
			b3 = tcr2.size() > 0;
			uint8_t b4 = GetCR(iCh_R, cr.Step1 - 5, 0, cr.Step2 + 5, g_iJawRow[railType] * 2, blocks, vCRs[iCh_R], tcrR2, tcrR, 1, false);

			if ((b1 || b2) && (b3 + b4) == 0)
			{
				memcpy(wound.Result, "轨头外侧重伤", 30);
				wound.Type = W_HS;
				wound.Place = WP_HEAD_OUT;
				if ((carType == true && (m == CH_A1 || m == CH_a1)) || (carType == false && (m == CH_B1 || m == CH_b1)))
				{
					memcpy(wound.Result, "轨头内侧重伤", 30);
					wound.Type = W_HS;
					wound.Place = WP_HEAD_IN;
				}
				wound.SizeX = (cr.Row2 - cr.Row1) * 3;
				wound.SizeY = (cr.Step2 - cr.Step1) * 2.67;
				wound.Degree = WD_SERIOUS;
				AddWoundData(wound, cr);
				AddWoundData(wound, vCRs[cr.Channel], tcr);
				AddWoundData(wound, vCRs[iCh_R], tcrR);
				if (bShowWoundDetail)
				{
					sprintf(tempAccording, "%s一次出波，位移：%.1f大格，幅值%d", ChannelNamesB[m].c_str(), 0.02f * crInfo.Shift, crInfo.MaxV);
					wound.According.emplace_back(tempAccording);
				}
				FillWound2(wound, blocks);
				AddToWounds(vWounds, wound);

				SetUsedFlag(vCRs[m], tcr, 1);
				SetUsedFlag(vCRs[iCh_R], tcrR, 1);
				isWound = true;
			}
		}

		/************************2020-06-09 增加核伤小出波检测*********************/
		/*
		200608Z47813_0002_18S1005, 96单通道出波达不到判伤标准，但前后很干净
		*/
		if (isWound == false && cr.Info.MaxV >= hsAmpl && cr.Info.Shift >= 40 && cr.Info.MinH <= 300 ||
			isWound == false && cr.Info.MaxV >= hsAmpl && cr.Info.Shift >= 50 && cr.Info.MinH <= 310)//6大格
		{
			bool isSingle = true;
			VINT crThisChannel;
			int s1 = cr.Step1 - 500, s2 = cr.Step2 + 500;
			GetCR(cr.Channel, s1, s2, blocks, vCRs[cr.Channel], crThisChannel, cr.Index);
			RemovePMCR(vCRs[cr.Channel], crThisChannel);
			isSingle = (crThisChannel.size() == 0);

			if (isSingle)
			{
				memcpy(wound.Result, "轨头外侧轻伤", 30);
				wound.Type = W_HS;
				wound.Place = WP_HEAD_OUT;
				if ((carType == true && (m == CH_A1 || m == CH_a1)) || (carType == false && (m == CH_B1 || m == CH_b1)))
				{
					memcpy(wound.Result, "轨头内侧轻伤", 30);
					wound.Type = W_HS;
					wound.Place = WP_HEAD_IN;
				}
				wound.SizeX = (cr.Row2 - cr.Row1) * 3;
				wound.SizeY = (cr.Step2 - cr.Step1) * 2.67;
				wound.Degree = WD_LESS;
				AddWoundData(wound, cr);
				if (bShowWoundDetail)
				{
					sprintf(tempAccording, "%s二次出波，位移：%.1f大格，幅值%d", ChannelNamesB[m].c_str(), 0.02f * crInfo.Shift, crInfo.MaxV);
					wound.According.emplace_back(tempAccording);
				}
				FillWound2(wound, blocks);
				AddToWounds(vWounds, wound);

				isWound = true;
			}
		}

		if (isWound && isJoint == 0 && isSew == 0)
		{
			if (vWounds.size() > 0)
			{
				wound = vWounds[vWounds.size() - 1];
				vWounds.pop_back();
			}
			TRECT rect;
			GetWoundRect2(wound, g_iJawRow[railType], &rect);

			VINT crC, crc, crD, crE, crF, crG;
			if (cr.Channel >= CH_C)
			{
				if (cr.Channel % 4 == 0)
				{
					GetCR(CH_C, rect.step1 - 5, rect.row1, rect.step2, cr.Row2, blocks, vCRs[CH_C], crC);
					GetCR(CH_c, rect.step1, rect.row1, rect.step2 + 10, cr.Row2, blocks, vCRs[CH_c], crc);
					GetCR(CH_D, rect.step1 - 5, rect.row1, rect.step2, cr.Row2, blocks, vCRs[CH_D], crD);
					GetCR(CH_E, rect.step1, rect.row1, rect.step2 + 10, cr.Row2, blocks, vCRs[CH_E], crE);
					GetCR(CH_F, rect.step1 - 5, rect.row1, rect.step2 + 10, cr.Row2, blocks, vCRs[CH_F], crF);
					GetCR(CH_G, rect.step1, rect.row1, rect.step2 + 10, cr.Row2, blocks, vCRs[CH_G], crG);
				}
				else
				{
					GetCR(CH_C, rect.step1, rect.row1, rect.step2, cr.Row2, blocks, vCRs[CH_C], crC);
					GetCR(CH_c, rect.step1, rect.row1, rect.step2, cr.Row2, blocks, vCRs[CH_c], crc);
					GetCR(CH_D, rect.step1 - 10, rect.row1, rect.step2, cr.Row2, blocks, vCRs[CH_D], crD);
					GetCR(CH_E, rect.step1, rect.row1, rect.step2, cr.Row2, blocks, vCRs[CH_E], crE);
					GetCR(CH_F, rect.step1 - 10, rect.row1, rect.step2, cr.Row2, blocks, vCRs[CH_F], crF);
					GetCR(CH_G, rect.step1 - 10, rect.row1, rect.step2, cr.Row2, blocks, vCRs[CH_G], crG);
				}
			}
			else
			{
				/*	2020-06-21 注释 190313Q3048XLS_0003_17S2025，米块：129
				uint8_t row1 = cr.Row1, row2 = cr.Row2;
				uint8_t iJawRow = g_iJawRow[railType];
				if (row1 >= g_iJawRow[railType])
				{
					uint8_t t = row1;
					row1 = 2 * g_iJawRow[railType] - row2;
					row2 = 2 * g_iJawRow[railType] - t;
				}
				else if (row2 > g_iJawRow[railType])
				{
					uint8_t t = row1;
					row1 = (std::min)(2 * g_iJawRow[railType] - row2, row1);
					row2 = g_iJawRow[railType];
				}
				*/
				if (cr.Channel % 4 == 0)
				{
					GetCR(CH_C, rect.step1 - 5, rect.row1, rect.step2, rect.row2, blocks, vCRs[CH_C], crC);
					GetCR(CH_c, rect.step1, rect.row1, rect.step2 + 10, rect.row2, blocks, vCRs[CH_c], crc);
					GetCR(CH_D, rect.step1 - 5, rect.row1, rect.step2, rect.row2, blocks, vCRs[CH_D], crD);
					GetCR(CH_E, rect.step1, rect.row1, rect.step2 + 10, rect.row2, blocks, vCRs[CH_E], crE);
					GetCR(CH_F, rect.step1 - 5, rect.row1, rect.step2 + 10, rect.row2, blocks, vCRs[CH_F], crF);
					GetCR(CH_G, rect.step1, rect.row1, rect.step2 + 10, rect.row2, blocks, vCRs[CH_G], crG);
				}
				else
				{
					GetCR(CH_C, rect.step1, rect.row1, rect.step2, rect.row2, blocks, vCRs[CH_C], crC);
					GetCR(CH_c, rect.step1, rect.row1, rect.step2, rect.row2, blocks, vCRs[CH_c], crc);
					GetCR(CH_D, rect.step1 - 10, rect.row1, rect.step2, rect.row2, blocks, vCRs[CH_D], crD);
					GetCR(CH_E, rect.step1, rect.row1, rect.step2, rect.row2, blocks, vCRs[CH_E], crE);
					GetCR(CH_F, rect.step1 - 10, rect.row1, rect.step2, rect.row2, blocks, vCRs[CH_F], crF);
					GetCR(CH_G, rect.step1 - 10, rect.row1, rect.step2, rect.row2, blocks, vCRs[CH_G], crG);
				}
			}



			if (crD.size() > 0)
			{
				uint8_t aD = ACH_D;
				int offset = g_filehead.deviceP2.Place[aD] + blockHead.probOff[aD];
				double angle = 0.1 * g_filehead.deviceP2.Angle[iChA].Refrac;
				for (int i = 0; i < crD.size(); ++i)
				{
					GetCRInfo(vCRs[CH_D][crD[i]], DataA, blocks, 1);
					ParseD(angle, blockHead, DataA, blocks, vCRs, vCRs[CH_D][crD[i]], vCRs[CH_D][crD[i]].Info, iFRow, crD[i], offset, wd, railType, vWounds, vPMs);
				}
			}

			if (crE.size() > 0)
			{
				uint8_t aE = ACH_E;
				int offset = g_filehead.deviceP2.Place[aE] + blockHead.probOff[aE];
				double angle = 0.1 * g_filehead.deviceP2.Angle[iChA].Refrac;
				for (int i = 0; i < crE.size(); ++i)
				{
					GetCRInfo(vCRs[CH_E][crE[i]], DataA, blocks, 1);
					ParseE(angle, blockHead, DataA, blocks, vCRs, vCRs[CH_E][crE[i]], vCRs[CH_E][crE[i]].Info, iFRow, crE[i], offset, wd, railType, vWounds, vPMs);
				}
			}

			if (crF.size() > 0)
			{
				for (int i = 0; i < crF.size(); ++i)
				{
					GetCRInfo(vCRs[CH_F][crF[i]], DataA, blocks, 1);
					ParseHorizonalCrack(g_filehead, DataA, blocks, vCRs, vCRs[CH_F][crF[i]], direction, carType, iFRow, vWounds, isJoint, isSew);
				}
			}

			if (crG.size() > 0)
			{
				for (int i = 0; i < crG.size(); ++i)
				{
					GetCRInfo(vCRs[CH_G][crG[i]], DataA, blocks, 1);
					ParseHorizonalCrack(g_filehead, DataA, blocks, vCRs, vCRs[CH_G][crG[i]], direction, carType, iFRow, vWounds, isJoint, isSew);
				}
			}


			RemovePMCR(vCRs[CH_C], crC);
			RemovePMCR(vCRs[CH_c], crc);
			RemovePMCR(vCRs[CH_D], crD);
			RemovePMCR(vCRs[CH_E], crE);
			RemovePMCR(vCRs[CH_F], crF);
			RemovePMCR(vCRs[CH_G], crG);

			RemoveHoleCR(vCRs[CH_D], crD);
			RemoveHoleCR(vCRs[CH_E], crE);
			RemoveHoleCR(vCRs[CH_F], crF);
			RemoveHoleCR(vCRs[CH_G], crG);

			SetUsedFlag(vCRs[CH_C], crC, 1);
			SetUsedFlag(vCRs[CH_c], crc, 1);
			SetUsedFlag(vCRs[CH_D], crD, 1);
			SetUsedFlag(vCRs[CH_E], crE, 1);
			SetUsedFlag(vCRs[CH_F], crF, 1);
			SetUsedFlag(vCRs[CH_G], crG, 1);

			AddWoundData(wound, vCRs[CH_C], crC);
			AddWoundData(wound, vCRs[CH_c], crc);
			AddWoundData(wound, vCRs[CH_D], crD);
			AddWoundData(wound, vCRs[CH_E], crE);
			AddWoundData(wound, vCRs[CH_F], crF);
			AddWoundData(wound, vCRs[CH_G], crG);
			AddToWounds(vWounds, wound);
		}

		return wound.Type > 0;
	}
	else if (m == CH_C || m == CH_c)//C, c 通道
	{
		if (cr.Region.size() <= 2 && (cr.Step1 == cr.Step2 || cr.Row1 == cr.Row2) && isJoint == 0 && isSew == 0)
		{
			SetUsedFlag(cr, 1);
			return false;
		}
		FillWound(wound, blockHead, g_filehead);
		AddWoundData(wound, cr);
		wound.Place = WP_HEAD_MID;
		wound.Degree = WD_SERIOUS;
		wound.SizeX = (cr.Row2 - cr.Row1) * 3;
		wound.SizeY = (cr.Step2 - cr.Step1) * 2.67;
		memcpy(wound.Result, "轨头中部核伤", 22);

		if ((crInfo.Shift >= 15 || cr.Region.size() >= 3) && (crInfo.MaxV >= hsAmpl || cr.IsContainA == 0) && cr.Row1 < g_iJawRow[railType] * 2 || cr.Row2 - cr.Row1 >= 7 || crInfo.Shift >= 50 && cr.Region.size() >= 5)
		{
			wound.Type = W_HS;
			VINT crF, crG;
			int iF1 = cr.Step1 - 5;
			int iF2 = cr.Step2 + 15;
			if (m == CH_C)
			{
				iF1 = cr.Step1 - 15;
				iF2 = cr.Step2 + 5;
			}

			bool bScrew = false, bGuide = false;
			if (isScrewHole == 0 && isGuideHole == 0)
			{
				bScrew = ParseScrewHole(g_filehead, DataA, blocks, vCRs, iF1, iF2, iFRow, railType, vWounds, vPMs, isJoint, false, isSew);
				if (bScrew == false)
				{
					bGuide = ParseGuideHole(g_filehead, DataA, blocks, vCRs, cr, cr.Index, iFRow, railType, vWounds, vPMs);
				}
			}
			if (isScrewHole || isGuideHole || bScrew || bGuide)//螺孔回波
			{
				//bool direction = blockHead.railType & BIT4;//direction: true:顺里程，false:逆里程
				//bool carType = blockHead.detectSet.Identify & BIT2;// 车型，1-右手车，0-左手车
				bool bOK = true;
				if (isScrewHole || bScrew)
				{
					if (cr.Row1 >= iLastScrewHoleRow - 8 && cr.Row2 <= iLastScrewHoleRow2 + 3)
					{

					}
					else
					{
						bOK = false;
					}
				}
				else if (isGuideHole || bGuide)
				{
					if (cr.Row1 >= iLastGuideHoleRow - 5 && cr.Row2 <= iLastGuideHoleRow2 + 3)
					{

					}
					else
					{
						bOK = false;
					}
				}/*
				for (int k = 0; k < cr.vASteps.size(); ++k)
				{
					for (int p = 0; p < cr.vASteps[k].Frames.size(); ++p)
					{
						if (cr.vASteps[k].Frames[p].Horizon < 240 && cr.vASteps[k].Frames[p].F[iChA] >= g_iAmpl)
						{
							bOK = false;
						}
					}
				}*/
				if (bOK == true)
				{
					SetUsedFlag(cr, 1);
				}
				if (cr.IsUsed == 1)
				{
					return false;
				}
			}
			else//非孔的回波
			{
				wound.Type = W_HS;
				VINT crF, crG, crF2, crG2, crD, crE;
				bool bFindF = GetCR(CH_F, iF1, 0, iF2, cr.Row2 + 5, blocks, vCRs[CH_F], crF, -1, 2, true);
				bool bFindG = GetCR(CH_G, iF1, 0, iF2, cr.Row2 + 5, blocks, vCRs[CH_G], crG, -1, 2, true);
				bool bLoseF = GetCR(CH_F, iF1, iFRow - 3, iF2, iFRow + 3, blocks, vCRs[CH_F], crF2, -1, 2, true);
				bool bLoseG = GetCR(CH_G, iF1, iFRow - 3, iF2, iFRow + 3, blocks, vCRs[CH_G], crG2, -1, 2, true);

				bFindF = RemoveHoleCR(vCRs[CH_F], crF);
				bFindG = RemoveHoleCR(vCRs[CH_G], crG);
				bLoseF = RemoveHoleCR(vCRs[CH_F], crF2);
				bLoseG = RemoveHoleCR(vCRs[CH_G], crF2);

				bFindF = RemoveCRByLengthLimit(vCRs[CH_F], crF, 30);
				bFindG = RemoveCRByLengthLimit(vCRs[CH_G], crG, 30);
				bLoseF = RemoveCRByLengthLimit(vCRs[CH_F], crF2, 30);
				bLoseG = RemoveCRByLengthLimit(vCRs[CH_G], crF2, 30);

				GetCR(CH_D, iF1, cr.Row1 - 2, iF2, cr.Row2 + 5, blocks, vCRs[CH_D], crD, -1, 2, true);
				GetCR(CH_E, iF1, cr.Row1 - 2, iF2, cr.Row2 + 5, blocks, vCRs[CH_E], crE, -1, 2, true);

				AddWoundData(wound, vCRs[CH_D], crD);
				AddWoundData(wound, vCRs[CH_E], crE);
				AddWoundData(wound, vCRs[CH_F], crF);
				AddWoundData(wound, vCRs[CH_G], crG);
				AddWoundData(wound, vCRs[CH_F], crF2);
				AddWoundData(wound, vCRs[CH_G], crG2);

				SetUsedFlag(vCRs[CH_D], crD, 1);
				SetUsedFlag(vCRs[CH_E], crE, 1);
				SetUsedFlag(vCRs[CH_F], crF, 1);
				SetUsedFlag(vCRs[CH_F], crF2, 1);
				SetUsedFlag(vCRs[CH_G], crG, 1);
				SetUsedFlag(vCRs[CH_G], crG2, 1);
			}
			if (isJoint)
			{
				wound.IsJoint = 1;
			}
			FillWound2(wound, blocks);
			AddToWounds(vWounds, wound);
			return wound.Type > 0;
		}
	}
	return wound.Type > 0;
}

void CreateSection(VINT& rawdata, int distance, std::vector<Section>& vsec)
{
	Section sec;
	sec.Flag = 0;
	for (int i = 0; i < rawdata.size(); ++i)
	{
		if (i == 0)
		{
			sec.Start = rawdata[i];
			sec.End = rawdata[i];
		}

		if (i == rawdata.size() - 1)
		{
			sec.End = rawdata[i];
			vsec.emplace_back(sec);
			return;
		}

		if (rawdata[i] - sec.End < distance)
		{
			sec.End = rawdata[i];
		}
		else
		{
			vsec.emplace_back(sec);
			sec.Start = rawdata[i];
			sec.End = rawdata[i];
		}
	}
}


int SolvePosition(uint16_t parsedMark, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int i, double wd, int16_t iFRow, uint8_t railType, bool carType, bool direction, VINT* t_cr, PM& pm, int iAStepBig, int iAStepSmall, VPM& vPMs, VPM& vPMs2, VWJ& vWounds, uint8_t isPreAnalyse)
{
	bool isManualJoint = false;
	int pIndex = GetMarkedPositionIndex(cr.Step1 - 50, vPMs2, nullptr);
	if (parsedMark == 0 && (pm.Percent >= 0.3 || pm.Size >= 300))
	{
		if (pIndex >= 0)
		{
			VINT t_cr2[10];
			uint8_t chCount = 0;
			int pointCount = 0;
			int s = 0;
			for (int i = g_vReturnSteps.size() - 1; i >= 0; --i)
			{
				if (g_vReturnSteps[i] < cr.Step1)
				{
					s = g_vReturnSteps[i];
					break;
				}
			}

			int iBeginStep = vPMs2[pIndex].Step2 - 500;
			if (s > 0)
			{
				iBeginStep = (std::max)(iBeginStep, s);
			}
			for (int j = 0; j < 10; ++j)
			{
				chCount += GetCR(j, iBeginStep, 10, vPMs2[pIndex].Step2 + 20, 16, blocks, vCRs[j], t_cr2[j], t_cr[j]);
				for (int k = 0; k < t_cr2[j].size(); ++k)
				{
					pointCount += vCRs[j][t_cr2[j][k]].Region.size();
				}
			}
			if (pointCount < 0.5 * pm.Size)
			{
				if ((vPMs2[pIndex].Mark == PM_SEW_LRH || vPMs2[pIndex].Mark == PM_SEW_LIVE || vPMs2[pIndex].Mark == PM_SELFDEFINE && vPMs2[pIndex].Data >= 2 && vPMs2[pIndex].Data <= 3))
				{
					if (vPMs2[pIndex].Mark == PM_SEW_LRH && pm.ChannelNum >= 5 || vPMs2[pIndex].Mark == PM_SEW_LIVE && pm.ChannelNum >= 2)
					{
						pm.Mark = GetMarkFromManual(vPMs2[pIndex]);
					}
				}
				else
				{
					pm.Mark = GetMarkFromManual(vPMs2[pIndex]);
					int dStep = pm.Step2 - vPMs2[pIndex].Step2;
					if (IsJoint(pm.Mark) && (fabs(dStep) <= 20 || pm.Manual == 1 || pm.Manual == 2))
					{
						isManualJoint = true;
						pm.Step2 = vPMs2[pIndex].Step2;
						iAStepBig = vPMs2[pIndex].Step2;
						iAStepSmall = vPMs2[pIndex].Step2 - 5;
					}
				}
			}
			else if (pm.Size >= 200 && pm.ChannelNum >= 8)
			{
				bool isHeavy = true;
				for (int j = 0; j < CH_C; ++j)
				{
					if (pm.Num[j] < 20)
					{
						isHeavy = false;
						break;
					}
				}
				if (isHeavy)
				{
					pm.Mark = GetMarkFromManual(vPMs2[pIndex]);
				}
			}
		}
		else if (pm.Size >= 50 && pm.Length <= 35)
		{
			uint32_t s1 = 0, s2 = 0;
			uint32_t dt1 = GetLastPos(cr.Step1, g_vSewPos, s1);
			uint32_t dt2 = GetLastPos(cr.Step1, g_vJointPos, s2);
			if (dt1 < 100 || dt2 < 400)
			{	//设置判断标志
				//	cr.Flag = 1;
				//	continue;
			}
			else if (pm.ChannelNum >= 5)
			{
				int chA = pm.Num[0] + pm.Num[1];
				int cha = pm.Num[2] + pm.Num[3];
				int chB = pm.Num[4] + pm.Num[5];
				int chb = pm.Num[6] + pm.Num[7];
				if (chA >= 0.5 * pm.Size || cha >= 0.5 * pm.Size || chB >= 0.5 * pm.Size || chb >= 0.5 * pm.Size)
				{
				}
				else
				{
					pm.Mark = PM_SEW_LRH;
				}
			}
		}
	}
	else if (parsedMark > 0)
	{
		if (pIndex >= 0)
		{
			if (IsJoint(vPMs2[pIndex].Mark) && Abs(cr.Step1 - vPMs2[pIndex].Step2) > 100)
			{

			}
			else
			{
				pm.Mark = GetMarkFromManual(vPMs2[pIndex]);
				parsedMark = pm.Mark;
				int dStep = pm.Step2 - vPMs2[pIndex].Step2;
				if (IsJoint(pm.Mark) && fabs(dStep) <= 20)
				{
					isManualJoint = true;
					pm.Step2 = vPMs2[pIndex].Step2;
					iAStepBig = vPMs2[pIndex].Step2;
					iAStepSmall = vPMs2[pIndex].Step2 - 5;
				}
				else if (IsSew(pm.Mark) && fabs(dStep) < 400)
				{
					//pm.Step2 = vPMs[pIndex].Step2;
					//iAStepBig = vPMs[pIndex].Step2;
					//iAStepSmall = vPMs[pIndex].Step2 - 5;
				}
			}
		}
	}

	if (pm.Mark == 0)
	{
		return 0;
	}

	uint32_t dt1 = 0, dt2 = 0, s1 = 0, s2 = 0;
	dt1 = GetLastPos(cr.Step1, g_vSewPos, s1);
	dt2 = GetLastPos(cr.Step1, g_vJointPos, s2);

	bool hasReturn1 = IsBacked(cr.Step1, s1, g_vReturnSteps);
	bool hasReturn2 = IsBacked(cr.Step1, s2, g_vReturnSteps);
	if (pm.Mark == PM_SEW_LRH || pm.Mark == PM_SEW_CH || pm.Mark == PM_SEW_LIVE)
	{
		if ((dt1 <= 20 || dt2 <= 20) && g_iBeginBlock == cr.Block && g_iBeginBlock != 0)
		{
			Pos pos;
			pos.Block = cr.Block; pos.Step = cr.Step; pos.Step2 = pm.Step2;
			g_vSewPos[pos.Step2] = pos;
		}
		else if ((dt1 < 1200 && hasReturn1 || dt1 >= 1200) && (dt2 < 1200 && hasReturn2 || dt2 >= 1200))
		{
			Pos pos;
			pos.Block = cr.Block; pos.Step = cr.Step; pos.Step2 = pm.Step2;
			g_vSewPos[pos.Step2] = pos;
		}
		else if (dt2 < 10)
		{
			//解决210416Z47693_0003_17S5029 1773 同时判定为接头和焊缝的问题
			return 0;
		}
		else
		{
			return 0;
		}

		if (pIndex >= 0)
		{
			pm.Mark = GetMarkFromManual(vPMs2[pIndex]);
		}
	}
	else if (IsJoint(pm.Mark))
	{
		if ((dt2 < 25 || dt1 < 25) && cr.Block == g_iBeginBlock && g_iBeginBlock != 0)
		{
			Pos pos;
			pos.Block = cr.Block; pos.Step = cr.Step; pos.Step2 = iAStepSmall;
			g_vJointPos[pos.Step2] = pos;
		}
		else  if ((dt1 < 600 && hasReturn1 || dt1 >= 1200) && (dt2 < 600 && hasReturn2 || dt2 >= 600))
		{
			Pos pos;
			pos.Block = cr.Block; pos.Step = cr.Step; pos.Step2 = iAStepSmall;
			g_vJointPos[pos.Step2] = pos;
		}
		else if (isManualJoint || pm.Manual == 1 || pm.Manual == 2)
		{
			Pos pos;
			pos.Block = cr.Block; pos.Step = cr.Step; pos.Step2 = iAStepSmall;
			g_vJointPos[pos.Step2] = pos;
		}
		else if (dt1 >= 400 && dt2 >= 400 && pm.ChannelNum >= 8 && pm.Length < 50 && pm.Num[CH_C] + pm.Num[CH_c] >= 30)
		{
			Pos pos;
			pos.Block = cr.Block; pos.Step = cr.Step; pos.Step2 = iAStepSmall;
			g_vJointPos[pos.Step2] = pos;
		}
		else if (isManualJoint == false)
		{
			return 0;
		}
	}

	if (pm.Mark == PM_SEW_CH)
	{
		int istepSew = pm.BeginStep + pm.Length / 2;
		pm.ScrewHoleCount = ParseSewCH(g_filehead, DataA, blocks, vCRs, cr, i, iFRow, carType, railType, t_cr, iAStepBig, iAStepSmall, istepSew, vWounds, vPMs);
	}
	else if (pm.Mark == PM_SEW_LRH)
	{
		iAStepBig += 5;
		iAStepSmall -= 5;
		int istepSew = GetSewLRHMiddleStep(blocks, vCRs, cr, t_cr, iAStepBig, iAStepSmall, pm);
		if (iAStepBig < 20)
		{
			istepSew = cr.Step2;
		}
		pm.MiddleStep = istepSew;
		Pos	 pos = FindStepInBlock(istepSew, g_vBlockHeads, 0);
		pm.Block = pos.Block;
		pm.Step = pos.Step;
		pm.Step2 = pos.Step2;
		pm.ScrewHoleCount = ParseSewLRH(g_filehead, DataA, blocks, vCRs, cr, i, iFRow, carType, railType, t_cr, iAStepBig, iAStepSmall, istepSew, vWounds, vPMs);
	}
	else if (pm.Mark == PM_SEW_LIVE)
	{
		iAStepBig += 5;
		iAStepSmall -= 5;
		int istepSew = GetSewXCHMiddleStep(blocks, vCRs, cr, t_cr, iAStepBig, iAStepSmall);
		if (iAStepBig < 20)
		{
			istepSew = cr.Step2;
		}
		pm.MiddleStep = istepSew;
		Pos	 pos = FindStepInBlock(istepSew, g_vBlockHeads, 0);
		pm.Block = pos.Block;
		pm.Step = pos.Step;
		pm.Step2 = pos.Step2;
		pm.ScrewHoleCount = ParseSewXCH(g_filehead, DataA, blocks, vCRs, cr, i, iFRow, carType, railType, t_cr, iAStepBig, iAStepSmall, istepSew, vWounds, vPMs);
	}
	else if (IsJoint(pm.Mark))
	{
		if (iAStepSmall <= 0 || iAStepBig <= 0)
		{
			return 0;
		}

		for (int j = 0; j < CH_D; ++j)
		{
			GetCR(j, iAStepSmall - 25, 0, iAStepBig + 25, 26, blocks, vCRs[j], t_cr[j], t_cr[j]);
			for (int k = 0; k < t_cr[j].size(); ++k)
			{
				if (
					(j % 4 != 0 && vCRs[j][t_cr[j][k]].Step1 >= iAStepSmall - 25 && vCRs[j][t_cr[j][k]].Step1 <= iAStepBig + 5) ||
					(j % 4 == 0 && vCRs[j][t_cr[j][k]].Step2 <= iAStepBig + 25 && vCRs[j][t_cr[j][k]].Step1 >= iAStepSmall - 5)
					)
				{
					//SetUsedFlag(vCRs[j][t_cr[j][k]], 1);
					SetJointFlag(vCRs[j][t_cr[j][k]], 1);
				}
			}
		}
		VINT crD, crE;
		if (iAStepSmall > iAStepBig)
		{
			int t = iAStepSmall;
			iAStepSmall = iAStepBig;
			iAStepBig = t;
		}
		pm.MiddleStep = (iAStepSmall + iAStepBig) >> 1;
		pm.ScrewHoleCount = ParseJoint(g_filehead, DataA, blocks, vCRs, iAStepSmall, iAStepBig, railType, direction, carType, iFRow, wd, t_cr, vWounds, vPMs, pm);
		if (pm.ScrewHoleCount >= 5)
		{
			pm.Mark = PM_JOINT6;
		}
		else if (pm.ScrewHoleCount >= 3)
		{
			pm.Mark = PM_JOINT4;
		}
		else
		{
			pm.Mark == PM_JOINT2;
		}
		//uint8_t iFindD = GetCR(CH_D, iAStepSmall - 3, g_iJawRow[railType] - 3, iAStepBig + 3, iFRow - 5, blocks, vCRs[CH_D], crD);
		//uint8_t iFindE = GetCR(CH_E, iAStepSmall - 3, g_iJawRow[railType] - 3, iAStepBig + 3, iFRow - 5, blocks, vCRs[CH_E], crE);
		//SetUsedFlag(vCRs[CH_D], crD, 1);
		//SetUsedFlag(vCRs[CH_E], crE, 1);
	}

	if (pm.Mark == PM_SEW_CH || pm.Mark == PM_SEW_LRH || pm.Mark == PM_SEW_LIVE || pm.Mark == PM_JOINT2 || pm.Mark == PM_JOINT4 || pm.Mark == PM_JOINT6)
	{
		if (pm.ScrewHoleCount >= 5)
		{
			pm.ScrewHoleCount = 6;
		}
		else if (pm.ScrewHoleCount >= 3)
		{
			pm.ScrewHoleCount = 4;
		}
		else if (pm.ScrewHoleCount >= 1)
		{
			pm.ScrewHoleCount = 2;
		}
		else
		{
			pm.ScrewHoleCount = 0;
		}
	}

	AddToMarks(pm, vPMs);
	if (pm.Mark == PM_SEW_CH || pm.Mark == PM_SEW_LRH)
	{
		SetUsedFlag(vCRs[CH_C], t_cr[8], 1);
		SetUsedFlag(vCRs[CH_c], t_cr[9], 1);
	}
	return pm.Mark;
}

void IsHole(BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, int i, double wd, int& offset, double angle, uint8_t& railType, int16_t& iFRow, VPM& vPMs, bool& bGuideHole, bool& bScrewHole, int &iFStep_1, int &iFStep_2, VINT& crD, VINT& crF, VINT& crG, VINT& crE, VINT& crF2, VINT& crG2, VWJ& vWounds)
{
	bGuideHole = false;//导孔
	bScrewHole = false;//螺孔
	if (cr.Region.size() < 5)
	{
		VINT vcr;
		bool bMany = GetCR(cr.Channel, cr.Step1 - 300, cr.Row1 - 3, cr.Step2 + 300, cr.Row2 + 3, blocks, vCRs[cr.Channel], vcr, cr.Index, 1, false);
		if (vcr.size() >= 20)
		{
			return;
		}
	}

	int firstrow = VALID_ROW - 1, lastrow = 0;
	GetCRRowInfo(cr, firstrow, lastrow);
	if (cr.Channel == CH_D && firstrow < lastrow)
	{
		return;
	}

	if (cr.Channel == CH_E && firstrow > lastrow)
	{
		return;
	}

	VINT exceptD = crD;
	VINT exceptE = crE;
	uint8_t bFindD = GetCR(CH_D, cr.Step1 + 1, g_iJawRow[railType], cr.Step2 + 15, cr.Row2 + 4, blocks, vCRs[CH_D], crD, exceptD, 2, true);
	uint8_t bFindE = GetCR(CH_E, cr.Step2 - 1, cr.Row1 - 2, cr.Step2 + 25, cr.Row2 + 4, blocks, vCRs[CH_E], crE, exceptE, 2, true);
	bFindD = RemoveHoleCR(vCRs[CH_D], crD);
	bFindE = RemoveHoleCR(vCRs[CH_E], crE);
	uint8_t bFindF = false, bFindG = false;
	if (bFindE)
	{
		int idxE = 0;
		int rowE1 = vCRs[CH_E][crE[0]].Row1;
		for (int k = 1; k < crE.size(); ++k)
		{
			if (vCRs[CH_E][crE[k]].Row1 < rowE1)
			{
				rowE1 = vCRs[CH_E][crE[k]].Row1;
				idxE = k;
			}
		}
		bFindF = GetCR(CH_F, cr.Step1 - 5, vCRs[CH_E][crE[idxE]].Row1 - 5, cr.Step2 + 15, vCRs[CH_E][crE[idxE]].Row2 + 2, blocks, vCRs[CH_F], crF2, -1, 2, false);//F螺孔高度出波
		bFindG = GetCR(CH_G, cr.Step1 - 5, vCRs[CH_E][crE[idxE]].Row1 - 5, cr.Step2 + 15, vCRs[CH_E][crE[idxE]].Row2 + 2, blocks, vCRs[CH_G], crG2, -1, 2, false);//G螺孔高度出波
	}
	else
	{
		bFindF = GetCR(CH_F, cr.Step1 - 5, cr.Row1 - 5, cr.Step2 + 15, cr.Row2 + 2, blocks, vCRs[CH_F], crF2, -1, 2, false);//F螺孔高度出波
		bFindG = GetCR(CH_G, cr.Step1 - 5, cr.Row1 - 5, cr.Step2 + 15, cr.Row2 + 2, blocks, vCRs[CH_G], crG2, -1, 2, false);//G螺孔高度出波
	}
	uint8_t bLoseF = GetCR(CH_F, cr.Step2 - 3, iFRow - 3, cr.Step2 + 15, iFRow + 3, blocks, vCRs[CH_F], crF, -1, 2, false);
	uint8_t bLoseG = GetCR(CH_G, cr.Step2 - 3, iFRow - 3, cr.Step2 + 15, iFRow + 3, blocks, vCRs[CH_G], crG, -1, 2, false);

	bFindF = RemoveHoleCR(vCRs[CH_F], crF2);
	bFindG = RemoveHoleCR(vCRs[CH_G], crG2);
	//bLoseF = RemoveHoleCR(vCRs[CH_F], crF);
	//bLoseG = RemoveHoleCR(vCRs[CH_G], crG);

	iFStep_1 = 0;
	iFStep_2 = 0;
	int iFStep2 = 0;
	if (bFindF)//F螺孔出波
	{
		int idxF = 0;
		uint8_t rowF = vCRs[CH_F][crF2[0]].Row1;
		for (int k = 1; k < crF2.size(); ++k)
		{
			if (vCRs[CH_F][crF2[k]].Row1 < rowF)
			{
				rowF = vCRs[CH_F][crF2[k]].Row1;
				idxF = k;
			}
		}
		CR tempF = vCRs[CH_F][crF2[idxF]];
		if (tempF.Step2 - tempF.Step1 < 8)
		{
			for (int k = 0; k < crF2.size(); ++k)
			{
				if (vCRs[CH_F][crF2[k]].Row1 >= tempF.Row1 - 1 && vCRs[CH_F][crF2[k]].Row2 <= tempF.Row2 + 1 && k != idxF && crF2[k] != crF2[idxF])
				{
					TryCombineFGInHole(tempF, vCRs[CH_F][crF2[k]]);
				}
			}
		}
		iFStep_1 = tempF.Step1;
		iFStep_2 = tempF.Step2;
		iFStep2 = (tempF.Step2 + tempF.Step1) >> 1;
	}
	else if (bFindG)
	{
		int idxG = 0;
		uint8_t rowG = vCRs[CH_G][crG2[0]].Row1;
		for (int k = 1; k < crG2.size(); ++k)
		{
			if (vCRs[CH_G][crG2[k]].Row1 < rowG)
			{
				rowG = vCRs[CH_G][crG2[k]].Row1;
				idxG = k;
			}
		}
		CR tempG = vCRs[CH_G][crG2[idxG]];
		if (tempG.Step2 - tempG.Step1 < 8)
		{
			for (int k = 0; k < crG2.size(); ++k)
			{
				if (vCRs[CH_G][crG2[k]].Row1 >= tempG.Row1 - 1 && vCRs[CH_G][crG2[k]].Row2 <= tempG.Row2 + 1 && k != idxG && crG2[k] != crG2[idxG])
				{
					TryCombineFGInHole(tempG, vCRs[CH_G][crG2[k]]);
				}
			}
		}
		iFStep_1 = tempG.Step1;
		iFStep_2 = tempG.Step2;
		iFStep2 = (tempG.Step2 + tempG.Step1) >> 1;
	}
	else if (bLoseF)
	{
		iFStep2 = (vCRs[CH_F][crF[0]].Step2 + vCRs[CH_F][crF[0]].Step1) >> 1;
		iFStep_1 = vCRs[CH_F][crF[0]].Step1;
		iFStep_2 = vCRs[CH_F][crF[0]].Step2;
	}
	else
	{
		iFStep2 = -999;
	}

	int iFLoseStep1 = 0, iFLoseStep2 = 0;//底部失波宽度
	int iLoseLen = 0;
	if (bLoseF)
	{
		iFLoseStep1 = vCRs[CH_F][crF[0]].Step1;
		iFLoseStep2 = vCRs[CH_F][crF[0]].Step2;
		iLoseLen = iFLoseStep2 - iFLoseStep1;
		for (int k = 1; k < crF.size(); ++k)
		{
			if (vCRs[CH_F][crF[k]].Step2 - vCRs[CH_F][crF[k]].Step1 > iLoseLen && vCRs[CH_F][crF[k]].Step2 - vCRs[CH_F][crF[k]].Step1 < 30)
			{
				iFLoseStep1 = vCRs[CH_F][crF[k]].Step1;
				iFLoseStep2 = vCRs[CH_F][crF[k]].Step2;
				iLoseLen = iFLoseStep2 - iFLoseStep1;
			}
		}
	}
	if (bLoseG)
	{
		for (int k = 0; k < crG.size(); ++k)
		{
			if (vCRs[CH_G][crG[k]].Step2 - vCRs[CH_G][crG[k]].Step1 > iLoseLen && vCRs[CH_G][crG[k]].Step2 - vCRs[CH_G][crG[k]].Step1 < 30)
			{
				iFLoseStep1 = vCRs[CH_G][crG[k]].Step1;
				iFLoseStep2 = vCRs[CH_G][crG[k]].Step2;
				iLoseLen = iFLoseStep2 - iFLoseStep1;
			}
		}
	}

	int iJawRow = -1;
	int iFRow2 = -1;
	int railType2 = -1;
	GetJawRow(0, cr.Step1, iJawRow, iFRow2, railType2);
	if (railType2 < 0)
	{
		railType2 = railType;
		iJawRow = g_iJawRow[railType];
		iFRow2 = iFRow;
	}
	int wear = g_iJawRow[railType2] - iJawRow;

	if (cr.Row1 >= iJawRow + 5 && cr.Row1 <= iLuokong_D_Row1_H[railType] - wear && cr.Row1 >= iLuokong_D_Row1_L[railType] - wear && (bFindE || bFindF || bFindG) && (bLoseF && bLoseG))//螺孔高度出D波
	{
		PM pm;
		memset(&pm, 0, sizeof(pm));
		pm.Walk = wd;
		pm.Block = cr.Block;
		pm.Step = cr.Step;
		pm.Step2 = cr.Step2;
		//F失波长度 >= 3个步进，为螺孔而不是导孔
		if ((iFLoseStep2 - iFLoseStep1 >= 3 && bFindF && bFindG) || (iFLoseStep2 - iFLoseStep1 >= 8 && bLoseF && bLoseG && (bFindF || bFindG)))//F底部失波步进数
		{
			pm.Mark = PM_SCREWHOLE;
			//AddToMarks(pm, vPMs);
			bScrewHole = true;
		}
		else
		{
			pm.Mark = PM_GUIDEHOLE;
			//AddToMarks(pm, vPMs);
			bGuideHole = true;
		}
	}
	else if (cr.Row1 <= iFRow - 4)//导孔、杂波、斜裂纹
	{
		//D是否满峰
		if (cr.Row1 >= iJawRow + 6 && cr.Row2 < iFRow - 10 && (bFindE || bFindF || bFindG))
		{
			PM pm;
			memset(&pm, 0, sizeof(pm));
			pm.Walk = wd;
			pm.Mark = PM_GUIDEHOLE;
			pm.Block = cr.Block;
			pm.Step = cr.Step;
			pm.Step2 = cr.Step2;
			AddToMarks(pm, vPMs);
			bGuideHole = true;
		}
	}

	if (cr.Row1 >= iJawRow + 6 && cr.Row2 <= iFRow - 10 && crD.size() > 0)
	{
		int stepD1 = 0, stepD2 = 0;
		GetCRASteps(CH_D, cr, stepD1, stepD2, blocks, angle, offset, g_filehead.step);
		for (int k = 0; k < crD.size(); ++k)
		{
			CR& tpD = vCRs[CH_D][crD[k]];
			if (tpD.Index == cr.Index && tpD.Region.size() == cr.Region.size())
			{
				continue;
			}
			if (tpD.Row2 - cr.Row2 >= 10)
			{
				continue;
			}

			int firstrow = VALID_ROW - 1, lastrow = 0;
			GetCRRowInfo(tpD, firstrow, lastrow);
			if (cr.Channel == CH_D && firstrow <= lastrow)
			{
				continue;
			}

			if (cr.Channel == CH_E && firstrow >= lastrow)
			{
				continue;
			}

			int stepDA1 = 0, stepDA2 = 0;
			int iCountA = 0, iCountB = 0;
			for (int p = 0; p < tpD.Region.size(); ++p)
			{
				GetCRASteps(CH_D, tpD.Region[p], stepDA1, stepDA2, blocks, angle, offset, g_filehead.step);
				if (stepDA1 >= stepD1 - 1 && stepDA2 <= stepD2 + 1)
				{
					iCountA++;
				}
				if (tpD.Region[p].step >= cr.Step1 && tpD.Region[p].step <= cr.Step2)
				{
					iCountB++;
				}
			}
			if (iCountB >= 0.5 * tpD.Region.size())
			{
				if (iCountA >= 0.7 * tpD.Region.size())
				{
					PM pm;
					memset(&pm, 0, sizeof(pm));
					pm.Walk = wd;
					pm.Mark = PM_GUIDEHOLE;
					pm.Block = cr.Block;
					pm.Step = cr.Step;
					pm.Step2 = cr.Step2;
					AddToMarks(pm, vPMs);
					bGuideHole = true;
					//SetUsedFlag(cr, 1);
					//SetUsedFlag(tpD, 1);
				}
				else if (bFindE == false && bFindF == false && bFindG == false)
				{
					double dv = GetDistanceVA(cr, tpD);
					if (dv >= 4 && dv <= 10)
					{
						CR crtemp = cr;
						if (cr.Row1 < tpD.Row1)
						{
							crtemp = tpD;
						}

						PM pm;
						memset(&pm, 0, sizeof(pm));
						pm.Walk = wd;
						pm.Mark = PM_SCREWHOLE;
						pm.Block = crtemp.Block;
						pm.Step = crtemp.Step;
						pm.Step2 = crtemp.Step2;
						AddToMarks(pm, vPMs);
						bScrewHole = true;

						Wound_Judged w;
						w.IsScrewHole = 0 + 10;
						FillWound(w, blocks[crtemp.Block - g_iBeginBlock].BlockHead, g_filehead);
						w.Block = crtemp.Block;
						w.Step = crtemp.Step;
						w.Step2 = crtemp.Step1;
						w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, g_filehead.step, g_direction);
						w.Degree = WD_SERIOUS;
						sprintf(w.Result, "%d孔四象限斜裂纹", 0);
						w.Type = W_SCREW_CRACK4;
						w.Place = WP_WAIST;
						w.SizeX = (crtemp.Row2 - crtemp.Row1) / 0.6;
						w.SizeY = 1;

						SetUsedFlag(vCRs[CH_D][crtemp.Index], 1);
						AddWoundData(w, crtemp);
						FillWound2(w, blocks);
						AddToWounds(vWounds, w);
					}
					else if (dv < 4)
					{
						CR crtemp = cr;
						if (cr.Row1 < tpD.Row1)
						{
							crtemp = tpD;
						}

						PM pm;
						memset(&pm, 0, sizeof(pm));
						pm.Walk = wd;
						pm.Mark = PM_GUIDEHOLE;
						pm.Block = crtemp.Block;
						pm.Step = crtemp.Step;
						pm.Step2 = crtemp.Step2;
						AddToMarks(pm, vPMs);
						bGuideHole = true;

						Wound_Judged w;
						w.IsGuideHole = 1;
						FillWound(w, blocks[crtemp.Block - g_iBeginBlock].BlockHead, g_filehead);
						w.Block = crtemp.Block;
						w.Step = crtemp.Step;
						w.Step2 = crtemp.Step1;
						w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, g_filehead.step, g_direction);
						w.Degree = WD_SERIOUS;
						sprintf(w.Result, "导孔四象限斜裂纹");
						w.Type = W_GUIDE_CRACK4;
						w.Place = WP_WAIST;
						w.SizeX = (crtemp.Row2 - crtemp.Row1) / 0.6;
						w.SizeY = 1;

						SetUsedFlag(vCRs[CH_D][crtemp.Index], 1);
						AddWoundData(w, crtemp);
						FillWound2(w, blocks);
						AddToWounds(vWounds, w);
					}
					//SetUsedFlag(tpD, 1);
					//SetUsedFlag(cr, 1);
				}
			}
			else if (iCountA >= 0.5 * tpD.Region.size() && bFindE == false && bFindF == false && bFindG == false)
			{
				PM pm;
				memset(&pm, 0, sizeof(pm));
				pm.Walk = wd;
				pm.Mark = PM_GUIDEHOLE;
				pm.Block = cr.Block;
				pm.Step = cr.Step;
				pm.Step2 = cr.Step2;
				AddToMarks(pm, vPMs);
				bGuideHole = true;
				//SetUsedFlag(tpD, 1);
				if (cr.Row1 > tpD.Row1)
				{
					SetUsedFlag(cr, 1);
				}
				else
				{
					SetUsedFlag(tpD, 1);
				}
			}
		}
	}

	uint32_t sew, joint;
	int dt1 = GetLastPos(cr.Step1, g_vSewPos, sew);
	int dt2 = GetLastPos(cr.Step1, g_vJointPos, joint);

	if (dt1 <= 30 || dt2 <= 100)
	{
		bGuideHole = false;
		return;
	}
}

void ParseD(double angle, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, int16_t& iFRow, int i, int& offset, double& wd, uint8_t& railType, VWJ& vWounds, VPM& vPMs)
{
	if (cr.IsUsed)
	{
		return;
	}
	Wound_Judged wound;
	FillWound(wound, blockHead, g_filehead);
	wound.Place = WP_WAIST;
	wound.Walk = wd;
	wound.Block = cr.Block;
	wound.Step = cr.Step;
	wound.Step2 = cr.Step1;

	int shift = g_isTestEqu ? 10 : 15;
	bool bGuideHole = false;//导孔
	bool bScrewHole = false;//螺孔
	if (cr.Row2 >= iFRow - 5)//轨底出波
	{
		if ((crInfo.MaxV >= g_iAmpl || crInfo.MaxV2 >= g_iAmpl || cr.IsContainA == 0) /*&& crInfo.Shift >= shift*/)
		{
			VINT crE;
			uint8_t bFindE = GetCR(CH_E, cr.Step1, iFRow - 3, cr.Step2 + 15, iFRow + 3, blocks, vCRs[CH_E], crE, -1, 2);
			for (int p = 0; p < crE.size(); ++p)
			{
				CR& tpE = vCRs[CH_E][crE[p]];
				int stepD = cr.Step1 + cr.Step2, stepDM = cr.Region[cr.Region.size() / 2].step;
				int stepE = tpE.Step1 + tpE.Step2, stepEM = tpE.Region[tpE.Region.size() / 2].step;
				if (stepD >= stepE + 5 && stepDM >= stepEM + 3)
				{
					continue;
				}
				GetCRInfo(tpE, DataA, blocks);
				if ((tpE.Info.MaxV >= g_iAmpl || tpE.Info.MaxV2 >= g_iAmpl || tpE.IsContainA == 0) /*&& tpE.Info.Shift >= shift*/)
				{
					AddWoundData(wound, cr);
					wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
					wound.Place = WP_BOTTOM;
					wound.Degree = WD_SERIOUS;
					wound.SizeX = (cr.Row2 - cr.Row1) * 3;
					wound.SizeY = (cr.Step2 - cr.Step1) * 2.67;
					memcpy(wound.Result, "轨底横向裂纹", 30);
					AddWoundData(wound, vCRs[CH_E], crE);
					//sprintf(tempAccording, "D出波，位移：%.1f大格，幅值:%d;E出波，位移：%.1f大格，幅值:%d", 0.02f * crInfo.Shift, crInfo.MaxV, 0.02f * tpE.Info.Shift, tpE.Info.MaxV);
					//wound.According.emplace_back(tempAccording);
					FillWound2(wound, blocks);
					AddToWounds(vWounds, wound);
					break;
				}
			}
			SetUsedFlag(vCRs[CH_E], crE, 1);

			if (crE.size() == 0 && cr.Row2 - cr.Row1 >= 5)
			{
				FillWound(wound, blockHead, g_filehead);
				wound.Block = cr.Block;
				wound.Step = cr.Step;
				wound.Step2 = cr.Step1;
				wound.Type = W_BOTTOM_EX;
				wound.Place = WP_BOTTOM;
				wound.Degree = WD_SERIOUS;
				memcpy(wound.Result, "轨底异常", 30);
				wound.SizeX = g_filehead.step * (cr.Row2 - cr.Row1) / 0.6;
				wound.SizeY = 1;
				AddWoundData(wound, cr);
				AddToWounds(vWounds, wound);
			}
		}
	}
	else
	{
		VINT crD, crF, crG, crE, crF2, crG2;
		int iFStep_1 = 0, iFStep_2 = 0;

		GetCR(CH_D, cr.Step1 + 1, g_iJawRow[railType] + 1, cr.Step2 + 10, iFRow - 3, blocks, vCRs[CH_D], crD, i, 2);
		RemoveHoleCR(vCRs[CH_D], crD);

		int iFirstRow = 0, iLastRow = 0;
		std::map<uint32_t, _StepRegion> vStep;
		GetCRRowInfo1(cr, vStep);
		iFirstRow = vStep[cr.Step1].row1;
		iLastRow = vStep[cr.Step2].row2;
		if (iFirstRow <= iLastRow && crD.size() > 0)
		{
			if (cr.Row1 < g_iJawRow[railType] + 5)
			{
				SetUsedFlag(cr, 1);
				return;
			}
			VINT crF, crG;
			GetCR(CH_F, cr.Step1 - 20, g_iJawRow[railType], cr.Step2 + 20, cr.Row1, blocks, vCRs[CH_F], crF);
			GetCR(CH_G, cr.Step1 - 50, g_iJawRow[railType], cr.Step2 + 50, cr.Row1, blocks, vCRs[CH_G], crG);
			if (crF.size() + crG.size() == 0)
			{
				SetUsedFlag(cr, 1);
				return;
			}
		}

		for (int k = 0; k < crD.size(); ++k)
		{
			CR& tempCR = vCRs[CH_D][crD[k]];
			if (tempCR.Row2 >= cr.Row2 || tempCR.Region.size() < cr.Region.size() / 2)
			{
				continue;
			}
			if (tempCR.Step2 - cr.Step2 >= 8)
			{
				continue;
			}
			IsHole(DataA, blocks, vCRs, tempCR, tempCR.Index, wd, offset, angle, railType, iFRow, vPMs, bGuideHole, bScrewHole, iFStep_1, iFStep_2, crD, crF, crG, crE, crF2, crG2, vWounds);
			if (bScrewHole) //螺孔
			{
				VPM vtemp;
				ParseScrewHole(g_filehead, DataA, blocks, vCRs, iFStep_1, iFStep_2, iFRow, railType, vWounds, vPMs);
			}
			else if (bGuideHole)//导孔
			{
				//导孔F经常正常，没有区别，故采用DE来进行步进控制
				int iStepGuide1 = cr.Step1, iStepGuide2 = cr.Step1 + 15;
				bGuideHole = ParseGuideHole(g_filehead, DataA, blocks, vCRs, tempCR, tempCR.Index, iFRow, railType, vWounds, vPMs);
			}
		}

		if (cr.IsUsed)
		{
			return;
		}
		IsHole(DataA, blocks, vCRs, cr, i, wd, offset, angle, railType, iFRow, vPMs, bGuideHole, bScrewHole, iFStep_1, iFStep_2, crD, crF, crG, crE, crF2, crG2, vWounds);
		if (bScrewHole) //螺孔
		{
			VPM vtemp;
			if (ParseScrewHole(g_filehead, DataA, blocks, vCRs, iFStep_1, iFStep_2, iFRow, railType, vWounds, vPMs) == false)
			{
				bGuideHole = true;
			}
		}
		if (bGuideHole)//导孔
		{
			//导孔F经常正常，没有区别，故采用DE来进行步进控制
			int iStepGuide1 = cr.Step1, iStepGuide2 = cr.Step1 + 15;
			bGuideHole = ParseGuideHole(g_filehead, DataA, blocks, vCRs, cr, i, iFRow, railType, vWounds, vPMs);
		}
		if (cr.IsUsed == 0 && (crInfo.MaxV >= g_iAmpl || crInfo.MaxV2 >= g_iAmpl || cr.IsContainA == 0) && cr.Region.size() >= 3 && cr.Step2 - cr.Step1 >= 2)
		{
			if (cr.Row1 < iLastGuideHoleRow && railType == iLastGuideHoleRailType || cr.Row1 < iLastScrewHoleRow && railType == iLastScrewHoleRailType)
			{
				if (iFirstRow < iLastRow)
				{
					SetUsedFlag(cr, 1);
					return;
				}
			}
			else if (cr.Row2 - cr.Row1 == 0 || iFirstRow < iLastRow)
			{
				SetUsedFlag(cr, 1);
				return;
			}
			wound.Type = W_SKEW_CRACK;
			wound.Place = WP_WAIST;
			wound.Degree = WD_SERIOUS;
			wound.Block = cr.Block;
			wound.Step = cr.Step;
			wound.Step2 = cr.Step1;
			memcpy(wound.Result, "斜裂纹", 30);
			wound.SizeX = g_filehead.step * (cr.Step2 - cr.Step1) / 0.8;
			wound.SizeY = 1;

			VINT crHead[10];
			int channelFind = 0;
			for (int k = 0; k < 10; ++k)
			{
				channelFind += GetCR(k, cr.Step1 - 2, cr.Row1 - 2, cr.Step2 + 2, cr.Row2 + 2, blocks, vCRs[k], crHead[k]);
			}

			//sprintf(tempAccording, "D出波，位移：%.1f大格，幅值：%d", 0.02f * crInfo.Shift, crInfo.MaxV);
			//wound.According.emplace_back(tempAccording);
			AddWoundData(wound, cr);

			/*
			VINT crF3, crG3;
			bool bLoseF2 = GetCR(CH_F, cr.Step1 - 1, iFRow - 3, cr.Step2 + 1, iFRow + 3, blocks, vCRs[CH_F], crF3, -1, 2, false);
			bool bLoseG2 = GetCR(CH_G, cr.Step1 - 1, iFRow - 3, cr.Step2 + 1, iFRow + 3, blocks, vCRs[CH_G], crG3, -1, 2, false);
			bLoseF2 = RemoveHoleCR(vCRs[CH_F], crF3);
			bLoseG2 = RemoveHoleCR(vCRs[CH_G], crG3);

			bool bFindF = crF2.size() > 0;
			bool bFindG = crG2.size() > 0;
			if (!bFindF && !bFindG && !bLoseF2 && !bLoseG2)
			{
				//uint32_t beginstep = cr.Step1 - 10, endstep = cr.Step2 + 10;
				//std::vector<A_Step> vFramesF, vFramesG;
				//int standardH = 350 * blocks[cr.Block - g_iBeginBlock].BlockHead.railH / 3 / 58;
				//GetAFrames(DataA, beginstep, endstep, ACH_F, standardH - 10, standardH + 10, vFramesF);
				//GetAFrames(DataA, beginstep, endstep, ACH_G, standardH - 10, standardH + 10, vFramesG);
				//uint16_t minF = 65535, maxF = 0, minG = 65535, maxG = 0;
				//for (int t = 0; t < vFramesF.size(); ++t)
				//{
				//	int temp = 0;
				//	for (int s = 0; s < vFramesF[t].Frames.size(); ++s)
				//	{
				//		if (vFramesF[t].Frames[s].F[ACH_F] > temp)
				//		{
				//			temp = vFramesF[t].Frames[s].F[ACH_F];
				//		}
				//	}
				//	if (temp > maxF)
				//	{
				//		maxF = temp;
				//	}
				//	if (temp < minF && vFramesF[t].Step >= beginstep && vFramesF[t].Step <= endstep)
				//	{
				//		minF = temp;
				//	}
				//}

				//for (int t = 0; t < vFramesG.size(); ++t)
				//{
				//	int temp = 0;
				//	for (int s = 0; s < vFramesG[t].Frames.size(); ++s)
				//	{
				//		if (vFramesG[t].Frames[s].F[ACH_G] > temp)
				//		{
				//			temp = vFramesG[t].Frames[s].F[ACH_G];
				//		}
				//	}
				//	if (temp > maxG)
				//	{
				//		maxG = temp;
				//	}
				//	if (temp < minG && vFramesG[t].Step >= beginstep && vFramesG[t].Step <= endstep)
				//	{
				//		minG = temp;
				//	}
				//}

				//int iJointIndex = -1;
				//for (int i = vJointPos.size() - 1; i >= 0; --i)
				//{
				//	if (vJointPos[i].Step2 - cr.Step1 >= -30 && vJointPos[i].Step2 - cr.Step1 <= 30)
				//	{
				//		iJointIndex = i;
				//		break;
				//	}
				//	if (vJointPos[i].Block < g_iBeginBlock)
				//	{
				//		break;
				//	}
				//}

				//if (iJointIndex < 0)
				//{
				//	if (minF > maxF / 2 || minG > maxG / 2)
				//	{
				//		//	wound.Type = 0;
				//	}
				//}
				//else
				//{
				//	int iScrewIndex = -1;
				//	if (cr.Step1 > vJointPos[iJointIndex].Step2)
				//	{
				//		iScrewIndex = 1;
				//	}
				//}
			}
			else
			{
				if (bFindF || bFindG)
				{
					if (bFindF && bFindG)
					{
						sprintf(tempAccording, "F, G出波");
					}
					else if (bFindF && !bFindG)
					{
						sprintf(tempAccording, "F出波");
					}
					else if (!bFindF && bFindG)
					{
						sprintf(tempAccording, "G出波");
					}
					wound.According.emplace_back(tempAccording);
				}

				if (bLoseF2 || bLoseG2)
				{
					if (bLoseF2 && bLoseG2)
					{
						sprintf(tempAccording, "F, G底部失波");
					}
					else if (bLoseF2 && !bLoseG2)
					{
						sprintf(tempAccording, "F底部失波");
					}
					else if (!bLoseF2 && bLoseG2)
					{
						sprintf(tempAccording, "G底部失波");
					}
					wound.According.emplace_back(tempAccording);
				}
			}
			*/
		}

		if (wound.Type != 0)
		{
			for (int t = 0; t < crE.size(); ++t)
			{
				GetCRInfo(vCRs[CH_E][crE[t]], DataA, blocks, 1);
			}
			for (int t = 0; t < crF2.size(); ++t)
			{
				GetCRInfo(vCRs[CH_F][crF2[t]], DataA, blocks, 1);
			}
			for (int t = 0; t < crG2.size(); ++t)
			{
				GetCRInfo(vCRs[CH_G][crG2[t]], DataA, blocks, 1);
			}
			if (wound.Type == W_SKEW_CRACK)
			{
				SetUsedFlag(vCRs[CH_E], crE, 1);
				SetUsedFlag(vCRs[CH_F], crF, 1);
				SetUsedFlag(vCRs[CH_F], crF2, 1);
				SetUsedFlag(vCRs[CH_G], crG, 1);
				SetUsedFlag(vCRs[CH_G], crG2, 1);

				RemoveHoleCR(vCRs[CH_E], crE);
				RemoveHoleCR(vCRs[CH_F], crF);
				RemoveHoleCR(vCRs[CH_F], crF2);
				RemoveHoleCR(vCRs[CH_G], crG);
				RemoveHoleCR(vCRs[CH_G], crG2);

				if (cr.IsJoint == 1)
				{
					RemoveJointCR(vCRs[CH_F], crF);
					RemoveJointCR(vCRs[CH_F], crF2);
					RemoveJointCR(vCRs[CH_G], crG);
					RemoveJointCR(vCRs[CH_G], crG2);
				}

				AddWoundData(wound, vCRs[CH_E], crE);
				AddWoundData(wound, vCRs[CH_F], crF);
				AddWoundData(wound, vCRs[CH_F], crF2);
				AddWoundData(wound, vCRs[CH_G], crG);
				AddWoundData(wound, vCRs[CH_G], crG2);

				FillWound2(wound, blocks);
				AddToWounds(vWounds, wound);
			}
			else if (wound.Type != 0)
			{
				AddWoundData(wound, vCRs[CH_E], crE);
				AddWoundData(wound, vCRs[CH_F], crF);
				AddWoundData(wound, vCRs[CH_F], crF2);
				AddWoundData(wound, vCRs[CH_G], crG);
				AddWoundData(wound, vCRs[CH_G], crG2);
				FillWound2(wound, blocks);
				AddToWounds(vWounds, wound);
			}
		}

		int s1 = 0, s2 = 0;
		GetCRASteps(CH_D, cr, s1, s2, blocks, angle, offset, g_filehead.step);
		VINT crDD;
		GetCR(CH_D, cr.Step1 - 2, cr.Row1, cr.Step2 + 40, iFRow - 4, blocks, vCRs[CH_D], crDD);
		for (int k = 0; k < crDD.size(); ++k)
		{
			int stepD11 = 0, stepD22 = 0;
			GetCRASteps(CH_D, vCRs[CH_D][crDD[k]], stepD11, stepD22, blocks, angle, offset, g_filehead.step);
			if (stepD11 >= s1 - 1 && stepD22 <= s2 + 1)
			{
				SetUsedFlag(vCRs[CH_D][crDD[k]], 1);
			}
		}
	}
}

void ParseE(double angle, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR& cr, CR_INFO_A& crInfo, int16_t& iFRow, int i, int& offset, double& wd, uint8_t& railType, VWJ& vWounds, VPM& vPMs)
{
	if (cr.IsUsed)
	{
		return;
	}

	int iBlock = cr.Block - g_iBeginBlock;
	Wound_Judged wound;
	wound.Walk = wd;
	wound.Block = cr.Block;
	wound.Step = cr.Step;
	wound.Step2 = cr.Step1;
	FillWound(wound, blockHead, g_filehead);
	AddWoundData(wound, cr);

	int iBottomRow = blocks[iBlock].vBStepDatas[cr.Step].FRow;
	if (cr.Row2 >= iFRow - 5)//轨底出波,不用再考虑单独的E出波
	{
		if (cr.Row2 - cr.Row1 >= 5 && cr.Region.size() >= 5 || cr.Region.size() >= 7)
		{
			FillWound(wound, blockHead, g_filehead);
			wound.Block = cr.Block;
			wound.Step = cr.Step;
			wound.Step2 = cr.Step1;
			wound.Type = W_BOTTOM_EX;
			wound.Place = WP_BOTTOM;
			wound.Degree = WD_SERIOUS;
			memcpy(wound.Result, "轨底异常", 30);
			wound.SizeX = g_filehead.step * (cr.Row2 - cr.Row1) / 0.6;
			wound.SizeY = 1;
			AddWoundData(wound, cr);
			AddToWounds(vWounds, wound);
		}
	}
	else
	{
		bool bGuideHole = false;//导孔
		bool bScrewHole = false;//螺孔

		bool bWhole = crInfo.MaxV >= g_iAmpl; //D是否满峰
		VINT crF, crG, crE, crF2, crG2;
		uint8_t bFindF = GetCR(CH_F, cr.Step1 - 15, cr.Row1 - 3, cr.Step1 - 1, cr.Row2, blocks, vCRs[CH_F], crF2, -1, 2, true);//F螺孔高度出波
		uint8_t bFindG = GetCR(CH_G, cr.Step1 - 15, cr.Row1 - 3, cr.Step1 - 1, cr.Row2, blocks, vCRs[CH_G], crG2, -1, 2, true);//G螺孔高度出波
		uint8_t bLoseF = GetCR(CH_F, cr.Step1 - 15, iFRow - 3, cr.Step1 - 1, iFRow + 3, blocks, vCRs[CH_F], crF, -1, 2, true);//G螺孔高度出波
		uint8_t bLoseG = GetCR(CH_G, cr.Step1 - 15, iFRow - 3, cr.Step1 - 1, iFRow + 3, blocks, vCRs[CH_G], crG, -1, 2, true);//G螺孔高度出波

		for (int k = crF.size() - 1; k >= 0; --k)
		{
			if (vCRs[CH_F][crF[k]].IsLose == 0)
			{
				crF.erase(crF.begin() + k);
			}
		}
		bLoseF = crF.size() > 0;
		for (int k = crG.size() - 1; k >= 0; --k)
		{
			if (vCRs[CH_G][crG[k]].IsLose == 0)
			{
				crG.erase(crG.begin() + k);
			}
		}
		bLoseG = crG.size() > 0;
		int iFStep2 = 0;
		if (bFindF)//F螺孔出波
		{
			iFStep2 = (vCRs[CH_F][crF2[0]].Step2 + vCRs[CH_F][crF2[0]].Step1) >> 1;
		}
		else if (bLoseF)
		{
			iFStep2 = (vCRs[CH_F][crF[0]].Step2 + vCRs[CH_F][crF[0]].Step1) >> 1;
		}
		else
		{
			iFStep2 = -999;
		}

		if (cr.Row1 >= g_iJawRow[railType] + 8 && cr.Row1 <= iLuokong_D_Row1_H[railType] && cr.Row1 >= iLuokong_D_Row1_L[railType])//螺孔高度出E波
		{
			bool bLose = bLoseF || bLoseG;
			if (bWhole && (bFindF || bFindG) && (bLoseF && bLoseG))
			{
				int step1 = 0, step2 = 0;
				GetCRASteps(cr.Channel, cr, step1, step2, blocks, angle, offset, g_filehead.step);

				//F失波长度 >= 8个步进，为螺孔而不是导孔
				int iLoseLenF = 0, iLoseLenG = 0;
				if (bLoseF)
				{
					iLoseLenF = vCRs[CH_F][crF[0]].Step2 - vCRs[CH_F][crF[0]].Step1;
				}
				if (bLoseG)
				{
					iLoseLenG = vCRs[CH_G][crG[0]].Step2 - vCRs[CH_G][crG[0]].Step1;
				}
				int ir1 = vCRs[CH_F][crF[0]].Row1;
				int ir2 = vCRs[CH_F][crF[0]].Row2;

				PM pm;
				memset(&pm, 0, sizeof(pm));
				pm.Walk = wd;
				pm.Block = cr.Block;
				pm.Step = cr.Step;
				pm.Step2 = cr.Step2;
				if (iLoseLenF >= 7 && iLoseLenG >= 7)//F底部失波步进数
				{
					pm.Mark = PM_SCREWHOLE;
					AddToMarks(pm, vPMs);
					bScrewHole = true;
				}
				else if (iLoseLenF < 7 || iLoseLenG < 7)
				{
					pm.Mark = PM_GUIDEHOLE;
					//AddToMarks(pm, vPMs);
					bGuideHole = true;
				}
			}
			else if (bFindF || bFindG)
			{
				PM pm;
				memset(&pm, 0, sizeof(pm));
				pm.Walk = wd;
				pm.Mark = PM_GUIDEHOLE;
				pm.Block = cr.Block;
				pm.Step = cr.Step;
				pm.Step2 = cr.Step2;
				//AddToMarks(pm, vPMs);
				bGuideHole = true;
			}
		}
		else if (cr.Row1 <= iFRow - 3 && cr.Step2 - cr.Step1 >= 2)//导孔、杂波、斜裂纹
		{
			bool bWhole = crInfo.MaxV >= g_iAmpl; //D是否满峰
			VINT crF, crG, crF2, crG2;
			uint8_t bFindF = GetCR(CH_F, cr.Step1 - 5, cr.Row1 - 5, cr.Step2 + 4, cr.Row2, blocks, vCRs[CH_F], crF2, -1, 2);//F螺孔高度出波
			uint8_t bFindG = GetCR(CH_G, cr.Step1 - 5, cr.Row1 - 5, cr.Step2 + 4, cr.Row2, blocks, vCRs[CH_G], crG2, -1, 2);//G螺孔高度出波
			uint8_t bLoseF = GetCR(CH_F, cr.Step2 - 1, iFRow - 3, cr.Step2 + 4, iFRow, blocks, vCRs[CH_F], crF);
			uint8_t bLoseG = GetCR(CH_G, cr.Step2 - 1, iFRow - 3, cr.Step2 + 4, iFRow, blocks, vCRs[CH_G], crG);

			bFindF = RemoveScrewHoleCR(vCRs[CH_F], crF2);
			bFindG = RemoveScrewHoleCR(vCRs[CH_G], crG2);
			bLoseF = RemoveScrewHoleCR(vCRs[CH_F], crF);
			bLoseG = RemoveScrewHoleCR(vCRs[CH_G], crG);

			bool bLose = bLoseF || bLoseG;

			if (cr.Row1 >= g_iJawRow[railType] + 5 && (((crInfo.Shift >= 6 && bWhole) || cr.IsContainA == 0 && cr.Region.size() >= 3) && (bFindF || bFindG)) && cr.Row2 <= 40)
			{
				memcpy(wound.Result, "导孔", 20);
				PM pm;
				memset(&pm, 0, sizeof(pm));
				pm.Walk = wd;
				pm.Mark = PM_GUIDEHOLE;
				pm.Block = cr.Block;
				pm.Step = cr.Step;
				pm.Step2 = cr.Step2;
				//AddToMarks(pm, vPMs);
				bGuideHole = true;
			}
		}

		if (bGuideHole)
		{
			uint32_t sew, joint;
			int dt1 = GetLastPos(cr.Step1, g_vSewPos, sew);
			int dt2 = GetLastPos(cr.Step1, g_vJointPos, joint);
			if (dt1 <= 140 || dt2 <= 140)
			{
				bGuideHole = false;
				vPMs.pop_back();
			}
		}

		if (bScrewHole) //螺孔
		{
			if (false == ParseScrewHole(g_filehead, DataA, blocks, vCRs, cr.Step1 - 10, cr.Step2 + 5, iFRow, railType, vWounds, vPMs))
			{
				bGuideHole = true;
			}
		}
		if (bGuideHole)//导孔
		{
			ParseGuideHole(g_filehead, DataA, blocks, vCRs, cr, i, iFRow, railType, vWounds, vPMs);
		}
		else if (crInfo.MaxV >= g_iAmpl && cr.Region.size() >= 5)
		{
			FillWound(wound, blockHead, g_filehead);
			wound.Block = cr.Block;
			wound.Step = cr.Step;
			wound.Step2 = cr.Step1;
			wound.Type = W_SKEW_CRACK;
			wound.Place = WP_WAIST;
			wound.Degree = WD_SERIOUS;
			memcpy(wound.Result, "斜裂纹", 30);
			wound.SizeX = g_filehead.step * (cr.Row2 - cr.Row1) / 0.6;
			wound.SizeY = 1;

			VINT crHead[10];
			int channelFind = 0;
			for (int k = 0; k < 10; ++k)
			{
				channelFind += GetCR(k, cr.Step1 - 2, cr.Row1 - 2, cr.Step2 + 2, cr.Row2 + 2, blocks, vCRs[k], crHead[k]);
			}

			//sprintf(tempAccording, "E出波，位移：%.1f大格，幅值：%d", 0.02f * crInfo.Shift, crInfo.MaxV);
			//wound.According.emplace_back(tempAccording);
			FillWound2(wound, blocks);

			VINT crF3, crG3;
			bool bLoseF2 = GetCR(CH_F, cr.Step1 - 1, iFRow - 3, cr.Step2 + 1, iFRow + 3, blocks, vCRs[CH_F], crF3, -1, 2, false);
			bool bLoseG2 = GetCR(CH_G, cr.Step1 - 1, iFRow - 3, cr.Step2 + 1, iFRow + 3, blocks, vCRs[CH_G], crG3, -1, 2, false);

			if (!bFindF && !bFindG && !bLoseF2 && !bLoseG2)
			{
				uint32_t beginstep = cr.Step1 - 10, endstep = cr.Step2 + 10;
				std::vector<A_Step> vFramesF, vFramesG;
				int standardH = 350 * blocks[cr.Block - g_iBeginBlock].BlockHead.railH / 3 / 58;
				GetAFrames(DataA, beginstep, endstep, ACH_F, standardH - 10, standardH + 10, vFramesF);
				GetAFrames(DataA, beginstep, endstep, ACH_G, standardH - 10, standardH + 10, vFramesG);
				uint16_t minF = 65535, maxF = 0, minG = 65535, maxG = 0;
				for (int t = 0; t < vFramesF.size(); ++t)
				{
					int temp = 0;
					for (int s = 0; s < vFramesF[t].Frames.size(); ++s)
					{
						if (vFramesF[t].Frames[s].F[ACH_F] > temp)
						{
							temp = vFramesF[t].Frames[s].F[ACH_F];
						}
					}
					if (temp > maxF)
					{
						maxF = temp;
					}
					if (temp < minF && vFramesF[t].Step >= beginstep && vFramesF[t].Step <= endstep)
					{
						minF = temp;
					}
				}

				for (int t = 0; t < vFramesG.size(); ++t)
				{
					int temp = 0;
					for (int s = 0; s < vFramesG[t].Frames.size(); ++s)
					{
						if (vFramesG[t].Frames[s].F[ACH_G] > temp)
						{
							temp = vFramesG[t].Frames[s].F[ACH_G];
						}
					}
					if (temp > maxG)
					{
						maxG = temp;
					}
					if (temp < minG && vFramesG[t].Step >= beginstep && vFramesG[t].Step <= endstep)
					{
						minG = temp;
					}
				}

			}
			else
			{

			}
		}

		if (wound.Type == W_SKEW_CRACK)
		{
			if (cr.IsJoint == 1)
			{
				RemoveJointCR(vCRs[CH_F], crF);
				RemoveJointCR(vCRs[CH_F], crF2);
				RemoveJointCR(vCRs[CH_G], crG);
				RemoveJointCR(vCRs[CH_G], crG2);
			}

			AddWoundData(wound, vCRs[CH_F], crF);
			AddWoundData(wound, vCRs[CH_F], crF2);
			AddWoundData(wound, vCRs[CH_G], crG);
			AddWoundData(wound, vCRs[CH_G], crG2);
			AddWoundData(wound, vCRs[CH_E], crE);

			AddToWounds(vWounds, wound);

			SetUsedFlag(vCRs[CH_F], crF, 1);
			SetUsedFlag(vCRs[CH_F], crF2, 1);
			SetUsedFlag(vCRs[CH_G], crG2, 1);
			SetUsedFlag(vCRs[CH_E], crE, 1);
		}

		int s1 = 0, s2 = 0;
		GetCRASteps(CH_E, cr, s1, s2, blocks, angle, offset, g_filehead.step);

		VINT crEE;
		GetCR(CH_E, cr.Step1 - 2, cr.Row1, cr.Step2 + 40, iFRow - 4, blocks, vCRs[CH_E], crEE);
		for (int k = 0; k < crEE.size(); ++k)
		{
			int stepE11 = 0, stepE22 = 0;
			GetCRASteps(CH_E, vCRs[CH_E][crEE[k]], stepE11, stepE22, blocks, angle, offset, g_filehead.step);
			if (stepE11 >= s1 - 1 && stepE22 <= s2 + 1)
			{
				SetUsedFlag(vCRs[CH_E][crEE[k]], 1);
			}
		}
	}
}

bool isFindReverse = false;
CR crReversed;
void AnalyseCR(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VCR* vCRs, CR& cr, int& i, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, int btIndex)
{
	int iBlock = cr.Region[0].block - g_iBeginBlock;
	int m = cr.Channel;
	uint8_t iChA = GetAChannelByBChannel(m);//该通道对应的A通道
	BLOCK& blockHead = blocks[iBlock].BlockHead;
	hour = (blockHead.time & 0x00FF0000) >> 16;
	minute = (blockHead.time & 0x0000FF00) >> 8;
	second = (blockHead.time & 0x000000FF);
	int iEndBlock = blocks[blocks.size() - 1].Index;
	// BIT0~1: 轨型(0为43轨，1-50，2-60，3-75), BIT4:0逆里程、1顺里程，BIT5:0右股、1左股，BIT6~7：单线、上行、下行，其他预留
	uint8_t railType = blockHead.railType & 0x03;

	isFindReverse = false;

	if (cr.Region.size() == 1 || cr.IsUsed > 0 || cr.Region[0].block < g_iBeginBlock || cr.Region[0].block > iEndBlock)
	{
		SetUsedFlag(cr, 1);
		return;
	}
	//if (cr.Block == blocks[blocks.size() - 1].Index && cr.Step > blocks[blocks.size() - 1].BlockHead.row - 150 && g_filehead.block - g_iBeginBlock >= N_BLOCKREAD && btIndex < 0)
	//{
	//	SetUsedFlag(cr, 1);
	//	return;
	//}

	//if (cr.Block == blocks[0].Index && blocks[0].Index != 0 && cr.Step1 < blocks[0].IndexL2 + 140 && btIndex < 0)
	//{
	//	SetUsedFlag(cr, 1);
	//	return;
	//}

	int offset = g_filehead.deviceP2.Place[iChA] + blockHead.probOff[iChA];
	double angle = 0.1 * g_filehead.deviceP2.Angle[iChA].Refrac;

	uint8_t isUnique = 0;
	if (vCRs[m].size() == 1)
	{
		isUnique = 1;
	}
	else
	{
		VINT vu;
		GetCR(cr.Channel, cr.Step1 - 100, cr.Row1, cr.Step2 + 100, cr.Row2, blocks, vCRs[cr.Channel], vu, i, 1);
		isUnique = vu.size() < 4 ? 1 : 0;
		if (vu.size() == 0)
		{
			isUnique = 2;
		}
	}

	bool bFindFrames = GetCRInfo(cr, DataA, blocks, isUnique);
	CR_INFO_A& crInfo = cr.Info;
	std::vector<A_Step>& vASteps = cr.vASteps;

	int16_t iFRow = (std::min)(blocks[cr.Block - g_iBeginBlock].vBStepDatas[cr.Step].FRow, blocks[cr.Block - g_iBeginBlock].vBStepDatas[cr.Step].GRow);
	if (iFRow < g_iBottomRow[0] - 5)
	{
		if (railType != 0)
		{
			if (iFRow * 3 <= rail_uDC[railType - 1] + 3)
			{
				railType = railType - 1;
			}
		}
		//iFRow = g_iBottomRow[blocks[cr.Block - g_iBeginBlock].BlockHead.railType & 0x03];
	}
	else if (iFRow > g_iBottomRow[railType + 2])
	{
		if (railType != 3)
		{
			if (iFRow * 3 >= rail_uDC[railType + 1] - 3)
			{
				railType = railType + 1;
			}
		}
	}

	bool direction = railType & BIT4;//direction: true:顺里程，false:逆里程

	bool carType = blockHead.detectSet.Identify & BIT2;// 车型，1-右手车，0-左手车
	double wd = GetWD(blockHead.walk, cr.Step, g_filehead.step, direction);

	if (m < 14)
	{
		if (m >= CH_A1 && m < CH_D)
		{
			VINT t_cr[10];
			int iAStepSmall, iAStepBig;
			PM pm;
			memset(&pm, 0, sizeof(pm));
			uint16_t parsedMark = ParsePosition(g_filehead, blocks, vCRs, cr, i, iFRow, railType, t_cr, pm, iAStepBig, iAStepSmall, vPMs, vPMs2);
#ifdef _DEBUG
			Pos te1 = FindStepInBlock(iAStepSmall, blocks, 0);
			Pos te2 = FindStepInBlock(iAStepBig, blocks, 0);
#endif // _DEBUG					  
			int mark = SolvePosition(parsedMark, DataA, blocks, vCRs, cr, cr.Index, wd, iFRow, railType, carType, direction, t_cr, pm, iAStepBig, iAStepSmall, vPMs, vPMs2, vWounds, 0);
			if (parsedMark == 0 && mark != 0)
			{
				AddToMarks(pm, vPMs);
			}
			if (cr.IsUsed == 1)
			{
				return;
			}
		}

		if (m == CH_A1 || m == CH_a1 || m == CH_B1 || m == CH_b1 || m == CH_C || m == CH_c)
		{
			ParseHS(DataA, blocks, vCRs, cr, crInfo, railType, direction, carType, iFRow, wd, vWounds, vPMs);
		}
		else if (m == CH_D)//D通道
		{
			ParseD(angle, blockHead, DataA, blocks, vCRs, cr, crInfo, iFRow, i, offset, wd, railType, vWounds, vPMs);
		}
		else if (m == CH_E)//E 通道
		{
			ParseE(angle, blockHead, DataA, blocks, vCRs, cr, crInfo, iFRow, i, offset, wd, railType, vWounds, vPMs);
		}
	}
	else if (m == CH_F || m == CH_G)//F 出波失波，不考虑G，除非F失耦而G不失耦
	{
		ParseHorizonalCrack(g_filehead, DataA, blocks, vCRs, cr, direction, carType, iFRow, vWounds, false, false);
	}

	//设置判断标志
	SetUsedFlag(cr, 1);


	if (isFindReverse)
	{
		VCR& vcr = vCRs[m];
		int start = (std::min)(crReversed.Index, i);
		for (int k = start; k < vcr.size(); ++k)
		{
			if (vcr[k].Index == crReversed.Index)
			{
				crReversed.IsUsed = vcr[k].IsUsed;
				break;
			}
		}
		vCRs[m][i] = cr;
		vCRs[m][crReversed.Index] = crReversed;
		i = crReversed.Index - 1;
	}
}


void DealWithSpecialMark(BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, PM& pm, VBA& vBA, VWJ& vWounds, VPM& vPMs, int btIndex)
{
	g_isYoloPost = true;

	for (int i = vWounds.size() - 1; i >= 0; --i)
	{
		if (vWounds[i].Manual != 0)
		{
			continue;
		}

		if (vWounds[i].Step2 >= step1 && vWounds[i].Step2 <= step2)
		{
			vWounds[i].Flag = 1;
		}
		if (vWounds[i].Step2 < blocks[0].IndexL2 || vWounds[i].Step2 >= g_iEndStep)
		{
			break;
		}
	}

	for (int i = vPMs.size() - 1; i >= 0; --i)
	{
		if (vPMs[i].Manual != 0)
		{
			continue;
		}
		if (vPMs[i].Step2 >= step1 && vPMs[i].Step2 <= step2)
		{
			vPMs[i].Flag = 1;
		}
		if (vPMs[i].Step2 < blocks[0].IndexL2 || vPMs[i].Step2 >= g_iEndStep)
		{
			break;
		}
	}

	VCR vCRs2[16];
	VINT tcr[16];
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < vCRs[i].size(); ++j)
		{
			if (vCRs[i][j].Step1 > step2 || vCRs[i][j].Step2 < step1)
			{
				continue;
			}
			vCRs[i][j].IsUsed = 0;
			vCRs[i][j].IsJoint = 0;
			vCRs[i][j].IsSew = 0;
			vCRs[i][j].IsScrewHole = 0;
			vCRs[i][j].IsGuideHole = 0;
			vCRs[i][j].IsWound = 0;
			CR cr = vCRs[i][j];
			cr.Index = vCRs2[i].size();
			vCRs2[i].emplace_back(cr);
		}
		}

	for (int i = 0; i < CH_D; ++i)
	{
		GetCR(i, step1, step2, blocks, vCRs2[i], tcr[i]);
	}

	int centerstep = (step1 + step2) >> 1;
	if (IsJoint(pm.Mark))
	{
		int stepLeft = centerstep + 3;
		int stepRight = centerstep - 3;
		uint8_t railType;
		bool cartype;
		int16_t iFRow;
		GetBlockInfo(blocks, pm.Block - g_iBeginBlock, cartype, railType, iFRow);
		double wd = 0;
		pm.ScrewHoleCount = ParseJoint(g_filehead, DataA, blocks, vCRs2, stepRight, stepLeft, railType, g_direction, cartype, iFRow, wd, tcr, vWounds, vPMs, pm);
		pm.Num[CH_d] = 1;
		AddToMarks(pm, vPMs);
	}
	else if (IsSew(pm.Mark))
	{
		int stepLeft = centerstep + 3;
		int stepRight = centerstep - 3;
		uint8_t railType;
		bool cartype;
		int16_t iFRow;
		GetBlockInfo(blocks, pm.Block - g_iBeginBlock, cartype, railType, iFRow);
		double wd = 0;

		CR cr;
		int index;
		for (int i = CH_c; i >= 0; --i)
		{
			for (int j = 0; j < vCRs2[i].size(); ++j)
			{
				if (cr.Region.size() < vCRs2[i][j].Region.size())
				{
					cr = vCRs2[i][j];
					index = j;
				}
			}
		}

		if (cr.Region.size() == 0)
		{
			return;
		}

		if (pm.Mark == PM_SEW_LRH)
		{
			pm.ScrewHoleCount = ParseSewLRH(g_filehead, DataA, blocks, vCRs2, cr, index, iFRow, cartype, railType, tcr, stepLeft, stepRight, centerstep, vWounds, vPMs);
		}
		else if (pm.Mark == PM_SEW_LIVE)
		{
			pm.ScrewHoleCount = ParseSewXCH(g_filehead, DataA, blocks, vCRs2, cr, index, iFRow, cartype, railType, tcr, stepLeft, stepRight, centerstep, vWounds, vPMs);
		}
		else if (pm.Mark == PM_SEW_CH)
		{
			pm.ScrewHoleCount = ParseSewCH(g_filehead, DataA, blocks, vCRs2, cr, index, iFRow, cartype, railType, tcr, stepLeft, stepRight, centerstep, vWounds, vPMs);
		}

		pm.Num[CH_d] = 1;
		AddToMarks(pm, vPMs);
	}
	VER vER;
	VLCP vLCP;
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < vCRs2[i].size(); ++j)
		{
			AnalyseCR(DataA, blocks, vBA, vCRs2, vCRs2[i][j], j, vWounds, vPMs, vER, vPMs, vLCP);
		}
	}

	g_isYoloPost = false;
	}


int16_t AnalyseStep(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VCR* vCRs, int step, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, int stepLimit, int pmIndex/* = -1 */, int btIndex/* = -1 */)
{
	Pos pos = FindStepInBlock(step, g_vBlockHeads, 0);
	uint8_t railType = blocks[pos.Block - g_iBeginBlock].BlockHead.railType & 0x03;
	int iAStepSmall, iAStepBig;
	VINT t_cr[10];
	PM pm;
	CR cr;
	for (int j = CH_c; j >= 0; --j)
	{
		if (GetCR(j, step - stepLimit, g_iJawRow[railType] - 2, step + stepLimit, g_iJawRow[railType] + 2, blocks, vCRs[j], t_cr[j]))
		{
			int iMaxIndex = 0;
			int iMaxSize = vCRs[j][t_cr[j][iMaxIndex]].Region.size();
			for (int k = 1; k < t_cr[j].size(); ++k)
			{
				if (vCRs[j][t_cr[j][k]].Region.size() > iMaxSize)
				{
					iMaxIndex = k;
					iMaxSize = vCRs[j][t_cr[j][k]].Region.size();
				}
			}
			cr = vCRs[j][t_cr[j][iMaxIndex]];
			t_cr[j].clear();
			break;
		}
	}
	if (cr.Region.size() == 0)
	{
		return 0;
	}
	int16_t iFRow = blocks[cr.Block - g_iBeginBlock].vBStepDatas[cr.Step].FRow;
	uint16_t parsedMark = ParsePosition(g_filehead, blocks, vCRs, cr, cr.Index, iFRow, railType, t_cr, pm, iAStepBig, iAStepSmall, vPMs, vPMs2);
	bool direction = blocks[pos.Block - g_iBeginBlock].BlockHead.railType & BIT4;//direction: true:顺里程，false:逆里程
	if (railType != 0)
	{
		if (iFRow * 3 <= rail_uDC[railType - 1] + 3)
		{
			railType = railType - 1;
		}
	}
	else if (railType != 3)
	{
		if (iFRow * 3 >= rail_uDC[railType + 1] - 3)
		{
			railType = railType + 1;
		}
	}
	bool carType = blocks[pos.Block - g_iBeginBlock].BlockHead.detectSet.Identify & BIT2;// 车型，1-右手车，0-左手车
	int16_t parsedMark2 = SolvePosition(parsedMark, DataA, blocks, vCRs, cr, cr.Index, pos.Walk, iFRow, railType, carType, direction, t_cr, pm, iAStepBig, iAStepSmall, vPMs, vPMs2, vWounds, 1);
	if (pmIndex >= 0)
	{
		vPMs[pmIndex].Mark = parsedMark2;
	}
	else if (parsedMark == 0 && parsedMark2 != 0)
	{
		AddToMarks(pm, vPMs);
	}
	return pm.Mark;
}

//第一次分析
void Analyse(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, VPM& vPMsYOLO, int backFlag/* = 0 */, int btIndex/* = -1 */)
{
	if (blocks.size() == 0)
	{
		return;
	}

	g_vHeavyPos.clear();
	g_iBeginBlock = blocks[0].Index;
	int iEndBlock = blocks[blocks.size() - 1].Index;
	//vReturnSteps.clear();

	VWJ hDatas;	//获取历史数据
	VPM hPMs;		//位置标记，如接头，桥梁，曲线等
	uint32_t nCount = 1000;

	int markCount = vPMs.size();
	int woundCount = vWounds.size();
	//S2 获取尖轨的位置标记
	FillMarksWounds(blocks, vPMs, vWounds, vPMs2, vLCP, g_filehead);

	//S3 本期数据判伤，求连通域
	//A a B b C c D d E e F G
	//需记录 A,a, B, b,C,c, D, E, F, G的出波
	VPM vSmarts;
	for (int i = markCount; i < vPMs.size(); ++i)
	{
		if (vPMs[i].Mark == PM_SMART1 || vPMs[i].Mark == PM_SMART2)
		{
			vSmarts.emplace_back(vPMs[i]);
			if (vPMs[i].Length > 0)
			{
				g_vSlopes[vPMs[i].Step2] = vPMs[i];
			}
		}
	}
	g_iBeginStep = blocks[0].IndexL2;

#ifdef YOLOV4
	g_isYoloRuning = true;
	std::map<int, std::vector<bbox_t>> yoloResults;
	std::thread th(DrawImage, std::ref(blocks), blocks[0].IndexL2, blocks[0].IndexL2 + 1000, std::ref(yoloResults));
	th.detach();
#endif

	VCR vCRs[16];
	CreateCR(blocks, vCRs, vER, vSmarts);

#pragma region 获取大块FG失波，提取锰钢岔
	int iLastLFCount = g_vFLoseBig.size();

	VCR vFLose;
	for (int i = 0; i < vCRs[CH_F].size(); ++i)
	{
		if (vCRs[CH_F][i].IsLose == 0)
		{
			continue;
		}


		CR& cF = vCRs[CH_F][i];
		cF.IsStart = (cF.Step1 <= g_iBeginStep + 100);
		cF.IsEnd = (cF.Step2 >= g_iEndStep - 100);

		if (cF.Step2 - cF.Step1 >= 100 || cF.IsStart || cF.IsEnd)
		{
			int sF1 = cF.Step1, sF2 = cF.Step2;

			VINT vExcept;
			vExcept.push_back(i);
			while (true)
			{
				VINT crF;
				GetCR(CH_F, sF2 - 5, cF.Row1 - 5, sF2 + 10, cF.Row2 + 5, blocks, vCRs[CH_F], crF, vExcept, 5);
				RemoveCRByLose(vCRs[CH_F], crF, 1);
				if (crF.size() > 0 && vCRs[CH_F][crF[0]].Step1 - sF2 <= 1)
				{
					sF2 = vCRs[CH_F][crF[0]].Step2;
					vExcept.push_back(crF[0]);
				}
				else
				{
					break;
				}
			}

			while (true)
			{
				VINT crF;
				GetCR(CH_F, sF1 - 50, cF.Row1 - 5, sF1 + 10, cF.Row2 + 5, blocks, vCRs[CH_F], crF, vExcept, 5);
				RemoveCRByLose(vCRs[CH_F], crF, 1);
				if (crF.size() > 0 && sF1 - vCRs[CH_F][crF[0]].Step2 <= 1)
				{
					sF1 = vCRs[CH_F][crF[0]].Step1;
					vExcept.push_back(crF[0]);
				}
				else
				{
					break;
				}
			}

			VINT crG;
			int loseLenLimit = 50;
			if (cF.IsStart || cF.IsEnd)
			{
				loseLenLimit = max(cF.Step2 - cF.Step1, 5);
			}
			if (loseLenLimit <= 11)
			{
				loseLenLimit = 11;
			}
			GetCR(CH_G, sF1, cF.Row1 - 5, sF2, cF.Row2 + 5, blocks, vCRs[CH_G], crG, -1, 1);
			RemoveCRByLose(vCRs[CH_G], crG, 1);

			for (int j = 0; j < crG.size(); ++j)
			{
				CR& cG = vCRs[CH_G][crG[j]];
				int sG1 = cG.Step1, sG2 = cG.Step2;
				if (j < crG.size() - 1 && vCRs[CH_G][crG[j + 1]].Step1 - sG2 <= 1)
				{
					sG2 = vCRs[CH_G][crG[j + 1]].Step2;
				}
				int bs, es;
				int os = GetOverlappedStep(sF1, sF2, sG1, sG2, bs, es);
				if (os >= loseLenLimit - 10)
				{
					CR cLose;
					cLose.Channel = CH_F;
					cLose.Step1 = bs;
					cLose.Step2 = es;
					cLose.IsStart = vCRs[CH_F][i].IsStart;
					cLose.IsEnd = vCRs[CH_F][i].IsEnd;
					Pos pos = FindStepInBlock(bs, blocks, 0);
					cLose.Block = pos.Block;
					cLose.Step = pos.Step;
					for (int k = bs; k <= es; ++k)
					{
						WaveData wd;
						wd.block = pos.Block;
						wd.step = k;
						wd.row = cF.Row1;
						wd.find = 0;
						cLose.Region.emplace_back(wd);
					}
					FillCR(cLose);
					vFLose.emplace_back(cLose);
				}
			}
		}
	}

	if (vFLose.size() > 0)
	{
		std::sort(vFLose.begin(), vFLose.end(), [&](CR& cr1, CR& cr2) {return  cr1.Step2 - cr1.Step1 > cr2.Step2 - cr2.Step1; });
		int flags[100] = { 0 };
		for (int i = 0; i < vFLose.size(); ++i)
		{
			if (flags[i] != 0)
			{
				continue;
			}
			for (int j = i + 1; j < vFLose.size(); ++j)
			{
				if (vFLose[i].Step2 - vFLose[i].Step1 >= 300)
				{
					if (vFLose[j].Step2 >= vFLose[i].Step1 &&vFLose[j].Step1 <= vFLose[i].Step1)
					{
						Combine(vFLose[i], vFLose[j]);
						flags[j] = 1;
						continue;
					}

					else if (vFLose[j].Step1 <= vFLose[i].Step2 &&vFLose[j].Step2 >= vFLose[i].Step1)
					{
						Combine(vFLose[i], vFLose[j]);
						flags[j] = 1;
						continue;
					}

					else if (vFLose[i].Step1 > vFLose[j].Step2 && vFLose[i].Step1 <= vFLose[j].Step2 + 100)
					{
						Combine(vFLose[i], vFLose[j]);
						flags[j] = 1;
						continue;
					}

					else if (vFLose[i].Step2 < vFLose[j].Step1 && vFLose[i].Step2 >= vFLose[j].Step1 - 100)
					{
						Combine(vFLose[i], vFLose[j]);
						flags[j] = 1;
						continue;
					}
				}
			}
		}

		for (int i = vFLose.size() - 1; i >= 0; --i)
		{
			if (flags[i] == 1)
			{
				vFLose.erase(vFLose.begin() + i);
			}
		}
	}

	if (vFLose.size() > 0)
	{
		for (int i = 0; i < vFLose.size(); ++i)
		{
			g_vFLoseBig.emplace_back(vFLose[i]);
		}
		std::sort(g_vFLoseBig.begin(), g_vFLoseBig.end());
		for (int i = g_vFLoseBig.size() - 1; i >= 1; --i)
		{
			if (g_vFLoseBig[i].Step1 - g_vFLoseBig[i - 1].Step2 <= 100 && (g_vFLoseBig[i].Step2 - g_vFLoseBig[i].Step1 >= 300 || g_vFLoseBig[i - 1].Step2 - g_vFLoseBig[i - 1].Step1 >= 300) ||
				g_vFLoseBig[i].Step1 - g_vFLoseBig[i - 1].Step2 < 0)
			{
				Combine(g_vFLoseBig[i - 1], g_vFLoseBig[i]);
				g_vFLoseBig.erase(g_vFLoseBig.begin() + i);
			}
			else if (g_vFLoseBig[i].IsStart == false && g_vFLoseBig[i].IsEnd == false && g_vFLoseBig[i].Step2 - g_vFLoseBig[i].Step1 < 300)
			{
				g_vFLoseBig.erase(g_vFLoseBig.begin() + i);
			}
			else if (g_vFLoseBig[i].IsStart && g_vFLoseBig[i].Block != 0 && g_vBlockSolvedFlags[g_vFLoseBig[i].Block - 1] != 0 && g_vFLoseBig[i].Step2 - g_vFLoseBig[i].Step1 < 300)
			{
				g_vFLoseBig.erase(g_vFLoseBig.begin() + i);
			}
			else if (g_vFLoseBig[i].IsEnd && g_vFLoseBig[i].Block < g_vBlockHeads.size() - 1 && g_vBlockSolvedFlags[g_vFLoseBig[i].Block + 1] != 0 && g_vFLoseBig[i].Step2 - g_vFLoseBig[i].Step1 < 300)
			{
				g_vFLoseBig.erase(g_vFLoseBig.begin() + i);
			}
		}
	}
#pragma endregion

#pragma region 获取FG底波变化点
	FRowRecord lastFR;
	if (g_vFR.size() > 0)
	{
		lastFR = g_vFR[g_vFR.size() - 1];
	}
	for (int i = 0; i < blocks.size(); ++i)
	{
		for (int j = 0; j < blocks[i].vBStepDatas.size(); ++j)
		{
			if (g_vFR.size() == 0 || (blocks[i].vBStepDatas[j].FRow != lastFR.FRow || blocks[i].vBStepDatas[j].GRow != lastFR.GRow))
			{
				FRowRecord fr;
				fr.Step2 = blocks[i].vBStepDatas[j].Step;
				fr.FRow = blocks[i].vBStepDatas[j].FRow;
				fr.GRow = blocks[i].vBStepDatas[j].GRow;
				g_vFR.emplace_back(fr);
				lastFR = fr;
			}
		}
	}
	std::sort(g_vFR.begin(), g_vFR.end(), [&](FRowRecord& a, FRowRecord& b) {return a.Step2 < b.Step2; });
#pragma endregion

#pragma region 处理多出波点区域
	VINT vWeight, vWeight2, vWeight3;
	std::map<int, int> vStepWithMark;
	for (int i = 0; i < g_stepCount; ++i)
	{
		if (g_StepPoints[i].SumHead15 >= 40)
		{
			vWeight.emplace_back(g_StepPoints[i].Step);
			vStepWithMark[g_StepPoints[i].Step] = 1;
		}
		if (g_StepPoints[i].SumHead15 > 20)
		{
			vWeight2.emplace_back(g_StepPoints[i].Step);
		}
		else if (g_StepPoints[i].SumHead15 < 20 && g_StepPoints[i].SumHead15 > 0)
		{
			vWeight3.emplace_back(g_StepPoints[i].Step);
		}
	}

	std::vector<Section> vsec, vsec2, vsec3;
	CreateSection(vWeight, 100, vsec);
	CreateSection(vWeight2, 50, vsec2);
	CreateSection(vWeight3, 50, vsec3);

	std::vector<Pos> vPosMarks;
	std::vector<Section> vHeavySteps;
	Section sec;
	for (int i = 0; i < vsec.size(); ++i)
	{
#ifdef _DEBUG
		Pos s1 = FindStepInBlock(vsec[i].Start, blocks, 0);
		Pos s2 = FindStepInBlock(vsec[i].End, blocks, 0);
#endif // _DEBUG

		int step = (vsec[i].Start + vsec[i].End) / 2;
		int maxIndex = vsec[i].Start - g_iBeginStep;
		int maxValue = g_StepPoints[maxIndex].TotalHead;
		for (int j = vsec[i].Start + 1 - g_iBeginStep; j <= vsec[i].End - g_iBeginStep; ++j)
		{
			if (g_StepPoints[j].TotalHead >= maxValue)
			{
				maxIndex = j;
				maxValue = g_StepPoints[j].TotalHead;
			}
		}
		step = maxIndex + g_iBeginStep;
		sec.Start = step;
		sec.End = g_StepPoints[step - g_iBeginStep].SumHead15;
		if (sec.End >= 40)
		{
			sec.Flag = 0;
			vHeavySteps.emplace_back(sec);
		}
	}

	std::vector<Section> vHeavySteps2;
	for (int i = 0; i < vsec2.size(); ++i)
	{
#ifdef _DEBUG
		Pos s1 = FindStepInBlock(vsec2[i].Start, blocks, 0);
		Pos s2 = FindStepInBlock(vsec2[i].End, blocks, 0);
#endif // _DEBUG

		int step = (vsec2[i].Start + vsec2[i].End) / 2;
		int maxIndex = vsec2[i].Start - g_iBeginStep;
		int maxValue = g_StepPoints[maxIndex].TotalHead;
		for (int j = vsec2[i].Start + 1 - g_iBeginStep; j <= vsec2[i].End - g_iBeginStep; ++j)
		{
			if (g_StepPoints[j].TotalHead >= maxValue)
			{
				maxIndex = j;
				maxValue = g_StepPoints[j].TotalHead;
			}
		}
		step = maxIndex + g_iBeginStep;
		sec.Start = step;
		sec.End = g_StepPoints[step - g_iBeginStep].SumHead15;
		if (sec.End >= 20)
		{
			sec.Flag = 0;
			vHeavySteps2.emplace_back(sec);
		}
	}

	std::vector<Section> vHeavySteps3;
	for (int i = 0; i < vsec3.size(); ++i)
	{
#ifdef _DEBUG
		Pos s1 = FindStepInBlock(vsec3[i].Start, blocks, 0);
		Pos s2 = FindStepInBlock(vsec3[i].End, blocks, 0);
#endif // _DEBUG

		int step = (vsec3[i].Start + vsec3[i].End) / 2;
		int maxIndex = vsec3[i].Start - g_iBeginStep;
		int maxValue = g_StepPoints[maxIndex].TotalHead;
		for (int j = vsec3[i].Start + 1 - g_iBeginStep; j <= vsec3[i].End - g_iBeginStep; ++j)
		{
			if (g_StepPoints[j].TotalHead >= maxValue)
			{
				maxIndex = j;
				maxValue = g_StepPoints[j].TotalHead;
			}
		}
		step = maxIndex + g_iBeginStep;
		sec.Start = step;
		sec.End = g_StepPoints[step - g_iBeginStep].SumHead15;
		if (sec.End > 0)
		{
			sec.Flag = 0;
			vHeavySteps3.emplace_back(sec);
		}
	}


#pragma region 处理人工标记的接头
	int pmCount = vPMs.size();
	int heaveIndex = vHeavySteps.size() - 1;
	for (int i = markCount; i < pmCount; ++i)
	{
		if (IsJoint(vPMs[i].Mark) == false)
		{
			continue;
		}
		VINT vHeavyIndexes;
		for (int j = heaveIndex; j >= 0; --j)
		{
			int dstep = vPMs[i].Step2 - vHeavySteps[j].Start;
			if (dstep >= -100 && dstep <= 100)
			{
				vHeavyIndexes.push_back(j);
			}
			if (dstep > 100)
			{
				heaveIndex = (j == vHeavySteps.size() - 1 ? j : j + 1);
				break;
			}
		}
		if (vHeavyIndexes.size() == 1 && vHeavySteps[vHeavyIndexes[0]].Flag == 0)
		{
			int16_t mark = AnalyseStep(DataA, blocks, vBA, vCRs, vPMs[i].Step2, vWounds, vPMs, vER, vPMs2, vLCP, 30, i, btIndex);
			vHeavySteps[vHeavyIndexes[0]].Flag = 1;
			if (mark == 0)
			{
				vPMs[i].IsOverlapped = 1;
				for (int j = vPMs2.size() - 1; j >= 0; --j)
				{
					if (vPMs2[j].Step2 == vPMs[i].Step2)
					{
						vPMs2[j].IsOverlapped = 1;
						break;
					}
				}
			}
		}
		else if (vHeavyIndexes.size() == 0)
		{
			vPMs[i].IsOverlapped = 1;
			for (int j = vPMs2.size() - 1; j >= 0; --j)
			{
				if (vPMs2[j].Step2 == vPMs[i].Step2)
				{
					vPMs2[j].IsOverlapped = 1;
					break;
				}
			}
		}
	}

	//for (int i = vPMs.size() - 1; i >= pmCount; --i)
	//{
	//	if (IsJoint(vPMs[i].Mark))
	//	{
	//		vPMs.erase(vPMs.begin() + i);
	//	}
	//}
#pragma endregion

#pragma region 处理人工标记的铝热焊/现场焊

	pmCount = vPMs.size();

	heaveIndex = vHeavySteps.size() - 1;
	for (int i = pmCount - 1; i >= markCount; --i)
	{
		if (vPMs[i].Mark != PM_SEW_LIVE && vPMs[i].Mark != PM_SEW_LRH)
		{
			continue;
		}
		VINT vHeavyIndexes;
		for (int j = heaveIndex; j >= 0; --j)
		{
			int dstep = vPMs[i].Step2 - vHeavySteps[j].Start;
			if (dstep >= -300 && dstep <= 600)
			{
				vHeavyIndexes.push_back(j);
			}
			if (dstep > 600)
			{
				heaveIndex = (j == vHeavySteps.size() - 1 ? j : j + 1);
				break;
		}
	}

		if (vHeavyIndexes.size() == 1 && vHeavySteps[vHeavyIndexes[0]].Flag == 0)
		{
			//vHeavySteps[vHeavyIndexes[0]].Flag = 1;
			AnalyseStep(DataA, blocks, vBA, vCRs, vHeavySteps[vHeavyIndexes[0]].Start, vWounds, vPMs, vER, vPMs2, vLCP, 30, i, btIndex);
			vHeavySteps[vHeavyIndexes[0]].Flag = 1;
		}
			}
#pragma endregion

#pragma region 处理其它多出波点
	for (int i = vHeavySteps.size() - 1; i >= 0; --i)
	{
		if (vHeavySteps[i].Flag == 1)
		{
			g_vHeavyPos[vHeavySteps[i].Start] = vHeavySteps[i].End;
			g_vHeavyPos2[vHeavySteps[i].Start] = vHeavySteps[i].End;
			vHeavySteps.erase(vHeavySteps.begin() + i);
		}
	}

	for (int i = 0; i < vHeavySteps.size(); ++i)
	{
		for (int j = 0; j < vHeavySteps.size() - i - 1; ++j)
		{
			if (vHeavySteps[i].End > vHeavySteps[j].End)
			{
				Section sec = vHeavySteps[i];
				vHeavySteps[i] = vHeavySteps[j];
				vHeavySteps[j] = sec;
			}
		}
	}
	for (int i = 0; i < vHeavySteps.size(); ++i)
	{
		g_vHeavyPos[vHeavySteps[i].Start] = vHeavySteps[i].End;
	}

	for (int i = 0; i < vHeavySteps2.size(); ++i)
	{
		g_vHeavyPos2[vHeavySteps2[i].Start] = vHeavySteps2[i].End;
	}

	for (int i = 0; i < vHeavySteps3.size(); ++i)
	{
		g_vHeavyPos3[vHeavySteps3[i].Start] = vHeavySteps3[i].End;
	}

	for (int i = 0; i < vHeavySteps.size(); ++i)
	{
		AnalyseStep(DataA, blocks, vBA, vCRs, vHeavySteps[i].Start, vWounds, vPMs, vER, vPMs2, vLCP, 50, -1, btIndex);
	}
#pragma endregion

#pragma endregion

#pragma region 处理人工标记的厂焊
	for (int i = vPMs2.size() - 1; i >= 0; --i)
	{
		if (vPMs2[i].Step2 < g_iBeginStep)
		{
			break;
		}

		if (vPMs2[i].Mark != PM_SELFDEFINE || vPMs2[i].Data != 1 || vPMs2[i].IsOverlapped == 1)
		{
			continue;
		}

		//if (vPMs2[i].Manual != 0)
		//{
		//	continue;
		//}

		int s1 = vPMs2[i].Step2 - 500, s2 = vPMs2[i].Step2 + 100;
		VINT vStepIndexes;
		int sIndex = (std::max)(s1 - g_iBeginStep, 0);
		int eIndex = (std::min)(s2 - g_iBeginStep, g_stepCount - 1);
		for (int i = sIndex; i < eIndex; ++i)
		{
			if (g_StepPoints[i].NumJaw > 0)
			{
				vStepIndexes.emplace_back(i);
			}
		}
		std::vector<Section> vsec;
		CreateSection(vStepIndexes, 15, vsec);
		//只有一个
		if (vsec.size() == 1 && vsec[0].End - vsec[0].Start <= 20)
		{
			int sstep = g_iBeginStep + vsec[0].Start;
			int estep = g_iBeginStep + vsec[0].End;

			Pos pos = FindStepInBlock(sstep, blocks, 0);
			uint8_t railType = blocks[pos.Block - g_iBeginBlock].BlockHead.railType & 0x03;
			uint8_t row1 = g_iJawRow[railType] - 3, row2 = g_iJawRow[railType] + 3;
			uint8_t iFRow = blocks[pos.Block - g_iBeginBlock].BlockHead.railH / 3;
			if (blocks[pos.Block - g_iBeginBlock].vBStepDatas.size() > 0)
			{
				iFRow = (std::min)(blocks[pos.Block - g_iBeginBlock].vBStepDatas[0].FRow, blocks[pos.Block - g_iBeginBlock].vBStepDatas[0].GRow);
			}
			bool carType = false;
			VINT tcr[16];
			CR cr;
			for (int j = 0; j < 16; ++j)
			{
				GetCR(j, sstep, estep, blocks, vCRs[j], tcr[j]);
				if (tcr[j].size() > 0 && j < CH_D)
				{
					cr = vCRs[j][tcr[j][0]];
					tcr[j].clear();
					break;
				}
				tcr[j].clear();
			}
			if (cr.Step2 != 0)
			{
				PM pm;
				int iAStepBig, iAStepSmall;
				pm.Mark = ParsePosition(g_filehead, blocks, vCRs, cr, cr.Index, iFRow, railType, tcr, pm, iAStepBig, iAStepSmall, vPMs, vPMs2);
				pm.Mark = PM_SEW_CH;
				SolvePosition(pm.Mark, DataA, blocks, vCRs, cr, cr.Index, 0, iFRow, railType, carType, g_direction, tcr, pm, iAStepBig, iAStepSmall, vPMs, vPMs2, vWounds, true);

			}
		}
	}
#pragma endregion

#pragma region 伤损判定

#pragma region S4 填充人工标记伤损的信息
	for (int i = woundCount; i < vWounds.size(); ++i)
	{
		if (vWounds[i].Manual == 1)
		{
			VINT tcr[16];
			uint16_t count[16] = { 0 };
			for (int j = 0; j < 16; ++j)
			{
				uint8_t bFind = GetCR(j, vWounds[i].Step2 - 1, vWounds[i].Step2 + 1, blocks, vCRs[j], tcr[j]);
				for (int k = 0; k < tcr[j].size(); ++k)
				{
					count[j] += vCRs[j][tcr[j][k]].Region.size();
					GetCRInfo(vCRs[j][tcr[j][k]], DataA, blocks);
					AddWoundData(vWounds[i], vCRs[j][tcr[j][k]]);
				}
			}

			bool carType = blocks[vWounds[i].Block - g_iBeginBlock].BlockHead.detectSet.Identify & BIT2;
			int channelAa = count[CH_A1] + count[CH_A2] + count[CH_a1] + count[CH_a2];
			int channelBb = count[CH_B1] + count[CH_B2] + count[CH_b1] + count[CH_b2];
			if (vWounds[i].Place == WP_HEAD_IN || vWounds[i].Type == W_JOINT || vWounds[i].Type == W_SEWLR || vWounds[i].Type == W_SEWCH)
			{
				if (count[CH_C] + count[CH_c] >= 5)
				{
					vWounds[i].Place = WP_HEAD_MID;
				}
				else if (carType && channelAa > channelBb || !carType && channelAa < channelBb)
				{
					vWounds[i].Place = WP_HEAD_IN;
				}
				else
				{
					vWounds[i].Place = WP_HEAD_OUT;
				}
			}

			else if (vWounds[i].Place == WP_JAW_IN)
			{
				if (carType && channelAa > channelBb || !carType && channelAa < channelBb)
				{
					vWounds[i].Place = WP_JAW_IN;
				}
				else
				{
					vWounds[i].Place = WP_JAW_OUT;
				}
			}


			else if (vWounds[i].Place == 0)
			{
				if (count[CH_C] + count[CH_c] >= 5)
				{
					vWounds[i].Place = WP_HEAD_MID;
				}
				else if (channelAa + channelBb >= 10)
				{
					if (carType && channelAa > channelBb || !carType && channelAa < channelBb)
					{
						vWounds[i].Place = WP_HEAD_IN;
					}
					else
					{
						vWounds[i].Place = WP_HEAD_OUT;
					}
				}
				else
				{
					vWounds[i].Place = WP_WAIST;
				}
			}
		}
	}
#pragma endregion


	Position_Mark pm;
	year = g_filehead.startD >> 16;
	month = (g_filehead.startD & 0xFF00) >> 8;
	day = g_filehead.startD & 0xFF;

	//58行对应7大格
	float pixel = 58.0 / 7;

	//各通道抑制值
	float restr[CH_N] = { 0 };
	for (int i = 0; i < CH_N; ++i)
	{
		restr[i] = 0.005 * g_filehead.deviceP2.Restr[i];
	}

	//先寻找二次波，再去找一次波对照	
	bool isFindReverse = false;
	CR crReversed;
	VINT vUpper;
	for (int order = 0; order < 10; ++order)
	{
		int m = chParseOrder[order];
		VCR& vcr = vCRs[m];
		for (int i = 0; i < vCRs[m].size(); ++i)
		{
			AnalyseCR(DataA, blocks, vBA, vCRs, vcr[i], i, vWounds, vPMs, vER, vPMs2, vLCP);
		}
	}

#pragma endregion

#pragma region 回退校验
	if (backFlag == 0xFFFF)
	{
		std::vector<BackInfo> vBI;
		if (g_vBT[btIndex].BackCount == 1)
		{
			BackInfo bi;
			bi.backIndex = g_vBT[btIndex].BeginBackIndex;
			bi.step1 = g_vBAs[g_vBT[btIndex].BeginBackIndex].Pos0.Step2;
			bi.step2 = g_vBAs[g_vBT[btIndex].BeginBackIndex].Pos1.Step2;
			bi.JointSewMark = -1;
			bi.JointSewMarkIndex = -1;
			vBI.emplace_back(bi);
			bi.step1 = g_vBAs[g_vBT[btIndex].BeginBackIndex].Pos1.Step2;
			bi.step2 = g_vBAs[g_vBT[btIndex].BeginBackIndex].Pos2.Step2;
			vBI.emplace_back(bi);
		}
		else
		{
			BackInfo bi;
			for (int j = g_vBT[btIndex].BeginBackIndex; j < g_vBT[btIndex].BeginBackIndex + g_vBT[btIndex].BackCount; ++j)
			{
				BackAction& ba = g_vBAs[j];
				if (j == g_vBT[btIndex].BeginBackIndex)
				{
					BackInfo bi;
					bi.backIndex = j;
					bi.step1 = g_vBAs[j].Pos0.Step2;
					bi.step2 = g_vBAs[j].Pos1.Step2;
					bi.JointSewMark = -1;
					bi.JointSewMarkIndex = -1;
					vBI.emplace_back(bi);
					bi.step1 = g_vBAs[j].Pos1.Step2;
					bi.step2 = g_vBAs[j].Pos2.Step2;
					vBI.emplace_back(bi);
				}
				else
				{
					BackInfo bi;
					bi.backIndex = j;
					bi.step1 = g_vBAs[j].Pos1.Step2;
					bi.step2 = g_vBAs[j].Pos2.Step2;
					bi.JointSewMark = -1;
					bi.JointSewMarkIndex = -1;
					vBI.emplace_back(bi);
				}
			}
		}

		std::vector<int16_t> vMarksPerBack;
		std::vector<int16_t> vMarkIndexesPerBack;
		for (int j = 0; j < vBI.size(); ++j)
		{
			bool bJointSew = IsExistJointSew(vBI[j].step1, vBI[j].step2, vPMs, vBI[j].JointSewMarkIndex, markCount, vPMs.size() - 1);
			if (bJointSew)
			{
				vMarksPerBack.push_back(vPMs[vBI[j].JointSewMarkIndex].Mark);
				vMarkIndexesPerBack.push_back(vBI[j].JointSewMarkIndex);
			}
			else
			{
				vMarksPerBack.push_back(-1);
				vMarkIndexesPerBack.push_back(-1);
			}
		}

		std::map<int16_t, int> mark2Count;
		bool isSameValue = true;
		for (auto itr : vMarksPerBack)
		{
			if (mark2Count.find(itr) == mark2Count.end())
			{
				mark2Count[itr] = 1;
			}
			else
			{
				mark2Count[itr] = mark2Count[itr] + 1;
			}

			if (itr != vMarksPerBack[0])
			{
				isSameValue = false;
			}
		}

		if (isSameValue == false)
		{
			std::string strLogInfo = "回退前后位置标判定不一致";
			strLogInfo.append(std::to_string(g_vBT[btIndex].Block1) + " " + std::to_string(g_vBT[btIndex].Step1) + "\t"
				+ std::to_string(g_vBT[btIndex].Block2) + " " + std::to_string(g_vBT[btIndex].Step2));
			strLogInfo.append("\n");
			WriteLog((char*)strLogInfo.c_str());

			int manualStep = 0, manualStepLast = 0;
			int manualMark = GetMarkedPositionInArea(g_vBT[btIndex].Step1, g_vBT[btIndex].Step2, vPMs2, &manualStep);
			BackAction& lastBA = g_vBAs[g_vBT[btIndex].BeginBackIndex + g_vBT[btIndex].BackCount - 1];
			int manualIndex = -1;
			int manualMarkLast = GetMarkedPositionInArea(lastBA.Pos1.Step2, lastBA.Pos2.Step2, vPMs2, &manualStepLast, &manualIndex);
			bool isExistMunaulMark = (manualMark >= 0);
			bool isExistMunaulMarkLast = (manualMarkLast >= 0);

			int maxMark = 0;
			int maxValue = 0;
			//最后一次有人工标记
			if (isExistMunaulMarkLast)
			{
				maxMark = GetMarkFromManual(vPMs2[manualIndex]);

			}
			else//最后一次没有人工标记
			{
				for (auto& itr : mark2Count)
				{
					if (itr.second > maxValue)
					{
						maxValue = itr.second;
						maxMark = itr.first;
					}
				}
			}

			//bool isCredible = maxValue > 0.5 * vMarksPerBack.size();
			bool isCredible = maxValue > 0.5 * vMarksPerBack.size();
			if (isCredible || manualMarkLast > 0 || manualMarkLast == maxMark && maxMark > 0)
			{
				//对每次回退进行再次判定（默认有该位置标）
				for (int i = 0; i < vMarksPerBack.size(); ++i)
				{
					if (vMarksPerBack[i] == maxMark || IsJoint(vMarksPerBack[i]) == 1 || IsJoint(maxMark) == 1)
					{
						continue;
					}
					VINT vHeavySteps;
					bool isIsExistHeavyPoint = false;
					if (maxMark == PM_SEW_LRH || maxMark == PM_SEW_LIVE || IsJoint(maxMark))
					{
						isIsExistHeavyPoint = IsExistHeavyPoint(vBI[i].step1, vBI[i].step2, vHeavySteps);
						if (isIsExistHeavyPoint == false)
						{
							isIsExistHeavyPoint = IsExistHeavyPoint2(vBI[i].step1, vBI[i].step2, vHeavySteps);
							if (isIsExistHeavyPoint == false)
							{
								isIsExistHeavyPoint = IsExistHeavyPoint3(vBI[i].step1, vBI[i].step2, vHeavySteps);
							}
						}
					}
					else if (maxMark == PM_SEW_CH)
					{
						isIsExistHeavyPoint = IsExistHeavyPoint2(vBI[i].step1, vBI[i].step2, vHeavySteps);
						if (isIsExistHeavyPoint == false)
						{
							isIsExistHeavyPoint = IsExistHeavyPoint3(vBI[i].step1, vBI[i].step2, vHeavySteps);
						}
					}
					if (manualMark > 0 && manualMark == maxMark)
					{
						if (isIsExistHeavyPoint)
						{

						}
						else
						{

						}
					}
					else
					{
						if (isIsExistHeavyPoint)
						{
							if (vHeavySteps.size() == 1)
							{
								PM pm;
								VINT tcr[16];
								CR cr;
								for (int i = CH_c; i >= 0; --i)
								{
									VINT ttt;
									if (GetCR(i, vHeavySteps[0] - 20, vHeavySteps[0] + 20, blocks, vCRs[i], ttt))
									{
										cr = vCRs[i][ttt[0]];
										break;
									}
								}
								if (cr.Region.size() > 0)
								{
									int iAStepBig, iAStepSmall;
									ParsePosition(g_filehead, blocks, vCRs, cr, cr.Index, blocks[cr.Block - g_iBeginBlock].BlockHead.railH / 3, blocks[cr.Block - g_iBeginBlock].BlockHead.railType & 0x03,
										tcr, pm, iAStepBig, iAStepSmall, vPMs, vPMs2);
									pm.Mark = maxMark;

									int stepRight = pm.Step2 - 250;
									int stepLeft = pm.Step2 + pm.Length / 2 + 250;
									stepRight = max(stepRight, vBI[i].step1);
									stepLeft = min(stepLeft, vBI[i].step2);

									DealWithSpecialMark(DataA, blocks, vCRs, stepRight, stepLeft, pm, vBA, vWounds, vPMs, btIndex);
								}
							}
						}
						else
						{
							if (vMarksPerBack[i] > 0)
							{
								PM& pm = vPMs[vMarkIndexesPerBack[i]];
								pm.Mark = maxMark;

								int stepRight = pm.Step2 - 250;
								int stepLeft = pm.Step2 + pm.Length / 2 + 250;
								stepRight = max(stepRight, vBI[i].step1);
								stepLeft = min(stepLeft, vBI[i].step2);
								DealWithSpecialMark(DataA, blocks, vCRs, stepRight, stepLeft, pm, vBA, vWounds, vPMs, btIndex);
							}
						}
					}
				}
			}
		}
		WriteLog("Filter By BackTotal Finish\n");
	}
	else if (backFlag == 0xFFFF)
	{

	}
	else
	{

	}
#pragma endregion 


#ifdef YOLOV4

	while (g_isYoloRuning)
	{
		Sleep(1);
	}
	g_isYoloRuning = false;


	VPM vPMsYolo, vPMsYoloJointSew, vPMsYoloHole;
	std::vector<bbox_t> boxs;
	std::vector<Pos> vYoloPos, vSmartPos;
	for (auto& yoloresult : yoloResults)
	{
		for (auto& bbox : yoloresult.second)
		{
			if (bbox.prob < 0.5)
			{
				continue;
			}

			bbox_t box = bbox;
			int right = yoloresult.first + 1000 - 1000.0 * (bbox.x + bbox.w) / g_ImgWidth;
			int top = 1.0 * bbox.y * VALID_ROW / g_ImgHeight;

			int left = yoloresult.first + 1000 - 1.0 * bbox.x * 1000 / g_ImgWidth;
			int bottom = top + 1.0 * bbox.h * VALID_ROW / g_ImgHeight;
			std::string objname = g_obj_names[bbox.obj_id];
			printf("%lf\n", objname.c_str());

			box.x = right;
			box.w = left;
			box.y = top;
			box.h = bottom;
			boxs.emplace_back(box);

			if (yoloTypesToExpert.find(bbox.obj_id) != yoloTypesToExpert.end())
			{
				PM pm;
				memset(&pm, 0, sizeof(PM));
				pm.Mark = yoloTypesToExpert[bbox.obj_id];
				Pos pos = FindStepInBlock((left + right) / 2, blocks, 0);
				pm.Block = pos.Block;
				pm.Step = pos.Step;
				pm.Step2 = pos.Step2;
				pm.Num[0] = top;
				pm.Num[1] = bottom;
				pm.ARow = (top + bottom) / 2;
				pm.Length = left - right;
				pm.Data = bbox.obj_id;
				pm.Fangcha = box.prob;
				pm.Num[CH_d] = 1;
				vPMsYolo.push_back(pm);
				if (IsJoint(pm.Mark) || IsSew(pm.Mark))
				{
					vPMsYoloJointSew.push_back(pm);
					vPMsYOLO.push_back(pm);
					vYoloPos.push_back(pos);
				}
				else if (IsHole(pm.Mark))
				{
					vPMsYoloHole.emplace_back(pm);
				}
				else if (pm.Mark == PM_SMART1 || pm.Mark == PM_SMART2)
				{
					vSmartPos.push_back(pos);
				}
			}
		}
	}

	g_isYoloRuning = false;

	VPM vPMsCurrent, vPMsHole;
	for (int i = markCount; i < vPMs.size(); ++i)
	{
		if (IsJoint(vPMs[i].Mark) || (IsSew(vPMs[i].Mark) && vPMs[i].Mark != PM_SEW_CH))
		{
			vPMsCurrent.push_back(vPMs[i]);
		}
		else if (IsHole(vPMs[i].Mark))
		{
			vPMsHole.emplace_back(vPMs[i]);
		}
	}
	std::sort(vPMsCurrent.begin(), vPMsCurrent.end(), PMCompare);
	std::sort(vPMsYoloJointSew.begin(), vPMsYoloJointSew.end(), PMCompare);

	RemoveRepeatedPM(vPMsCurrent, 20);
	RemoveRepeatedPM(vPMsYoloJointSew, 20);

	int cIndex = 0;

#pragma region 算法检出而YOLO未检出
	int *isDetected = new int[vPMsCurrent.size()]();
	for (int i = 0; i < vPMsCurrent.size(); ++i)
	{
		for (int j = cIndex; j < vPMsYoloJointSew.size(); ++j)
		{
			if (Abs(vPMsYoloJointSew[j].Step2 - vPMsCurrent[i].Step2) <= 20 &&
				(IsJoint(vPMsYoloJointSew[j].Mark) && IsJoint(vPMsCurrent[i].Mark) || IsSew(vPMsYoloJointSew[j].Mark) && IsSew(vPMsCurrent[i].Mark)))
			{
				isDetected[i] = 1;
				break;
			}
			else if ((int)(vPMsYoloJointSew[j].Step2 - vPMsCurrent[i].Step2) > 20)
			{
				if (cIndex > 0)
				{
					cIndex--;
				}
				break;
			}
		}
	}

	for (int i = 0; i < vPMsCurrent.size(); ++i)
	{
		if (isDetected[i] == 0)
		{
			char szInfo[1000] = "";
			if (vPMsCurrent[i].Size == 0 && IsSew(vPMsCurrent[i].Mark))
			{
			}
			else
			{
				sprintf(szInfo, "yolo lose : Block = %d, Step = %d, Type = %d Missed!\n", vPMsCurrent[i].Block, vPMsCurrent[i].Step, vPMsCurrent[i].Mark);
				WriteLog(szInfo);
				SaveImage(blocks, vPMsCurrent[i].Block, vPMsCurrent[i].Step2, -1, -1, -1, -1, -1, 0, true);
			}
		}
	}
	delete isDetected;
#pragma endregion 算法检出而YOLO未检出

#pragma region YOLO检出而算法未检出
	isDetected = new int[vPMsYoloJointSew.size()]();
	for (int i = 0; i < vPMsYoloJointSew.size(); ++i)
	{
		for (int j = cIndex; j < vPMsCurrent.size(); ++j)
		{
			if (Abs(vPMsYoloJointSew[i].Step2 - vPMsCurrent[j].Step2) <= 20 &&
				(IsJoint(vPMsYoloJointSew[i].Mark) && IsJoint(vPMsCurrent[j].Mark) || IsSew(vPMsYoloJointSew[i].Mark) && IsSew(vPMsCurrent[j].Mark)))
			{
				isDetected[i] = 1;
				break;
			}
			else if ((int)(vPMsCurrent[j].Step2 - vPMsYoloJointSew[i].Step2) > 20)
			{
				if (cIndex > 0)
				{
					cIndex--;
				}
				break;
			}
		}
	}

	VPM vExpertLose;
	for (int i = 0; i < vPMsYoloJointSew.size(); ++i)
	{
		if (isDetected[i] == 0)
		{
			char szInfo[1000] = "";
			sprintf(szInfo, "expert lose: Block = %d, Step = %d, Type = %d Missed!\n", vPMsYoloJointSew[i].Block, vPMsYoloJointSew[i].Step, vPMsYoloJointSew[i].Mark);
			WriteLog(szInfo);
			SaveImage(blocks, vPMsYoloJointSew[i].Block, vPMsYoloJointSew[i].Step2,
				vPMsYoloJointSew[i].Step2, vPMsYoloJointSew[i].Length, vPMsYoloJointSew[i].Num[0], vPMsYoloJointSew[i].Num[1] - vPMsYoloJointSew[i].Num[0],
				vPMsYoloJointSew[i].Data, vPMsYoloJointSew[i].Fangcha,
				false);
			vExpertLose.push_back(vPMsYoloJointSew[i]);
		}
	}
	delete isDetected;
#pragma endregion YOLO检出而算法未检出

	for (int i = 0; i < vExpertLose.size(); ++i)
	{
		PM& tmpmark = vExpertLose[i];
		PM markPost;
		memset(&markPost, 0, sizeof(PM));
		int iStepBig, iStepSmall;
		int iFrom = (std::min)(blocks[tmpmark.Block - g_iBeginBlock].vBStepDatas[tmpmark.Step].FRow, blocks[tmpmark.Block - g_iBeginBlock].vBStepDatas[tmpmark.Step].GRow);
		iFrom /= 3;
		uint32_t mark = ParsePositionPost(g_filehead, blocks, vCRs,
			vExpertLose[i].Step2, vExpertLose[i].Step2 + vExpertLose[i].Length, vExpertLose[i].Num[0], vExpertLose[i].Num[1],
			iFrom, blocks[tmpmark.Block - g_iBeginBlock].BlockHead.railType & 0x03, vExpertLose[i].Mark, markPost, iStepBig, iStepSmall, vPMs);

		if (mark > 0)
		{
			PM pmExisted;
			memset(&pmExisted, 0, sizeof(PM));
			int pIndex = GetNearMark(tmpmark.Step2 - 50, tmpmark.Step2 + tmpmark.Length + 50, vPMs, pmExisted);
			if (pIndex < 0)
			{
				if (vExpertLose[i].Mark == mark)
				{
					int step1 = vExpertLose[i].Step2 - 160, step2 = vExpertLose[i].Step2 + 160;
					DealWithSpecialMark(DataA, blocks, vCRs, step1, step2, vExpertLose[i], vBA, vWounds, vPMs);
					std::string logContent = "DealWithSpecialMark " + std::to_string(vExpertLose[i].Block) + " " + std::to_string(vExpertLose[i].Mark) + "\n";
					WriteLog((char*)logContent.c_str());
				}
				else
				{

				}
			}
			else
			{
				if (vPMs[pIndex].Mark != mark)
				{
					vPMs[pIndex].IsYoloChecked = 2;
				}
				else
				{
					vPMs[pIndex].IsYoloChecked = 1;
				}
			}
		}
		else
		{
			// YOLO误判
		}
	}

	/*
	HeapSort(vPMsYoloHole, vPMsYoloHole.size());
	HeapSort(vPMsHole, vPMsHole.size());

	RemoveRepeatedPM(vPMsYoloHole, 20);
	RemoveRepeatedPM(vPMsHole, 20);
	isDetected = new	int[vPMsYoloHole.size()]();
	for (int i = 0; i < vPMsYoloHole.size(); ++i)
	{
		for (int j = cIndex; j < vPMsHole.size(); ++j)
		{
			if (Abs(vPMsYoloHole[i].Step2 - vPMsHole[j].Step2) <= 20)
			{
				isDetected[i] = 1;
				break;
			}
			else if ((int)(vPMsHole[j].Step2 - vPMsYoloHole[i].Step2) > 20)
			{
				if (cIndex > 0)
				{
					cIndex--;
				}
				break;
			}
		}
	}

	for (int i = 0; i < vPMsYoloHole.size(); ++i)
	{
		if (isDetected[i] == 0)
		{
			char szInfo[1000] = "";
			sprintf(szInfo, ">>>File: %s, Block = %d, Step = %d, Type = %d Missed!\n", g_strFileName.c_str(), vPMsYoloHole[i].Block, vPMsYoloHole[i].Step, vPMsYoloHole[i].Mark);
			WriteLog(szInfo);
		}
	}

	delete isDetected;
	*/
#endif

#pragma region 伤损过滤

#ifdef _DEBUG
	//std::sort(vWounds.begin(), vWounds.end());
#endif
	for (int i = woundCount; i < vWounds.size(); ++i)
	{
		//if (vWounds[i].Block - g_iBeginBlock < 0 || vWounds[i].Block - g_iBeginBlock >= blocks.size())
		//{
		//	FILE* pFFF = fopen("D:/Files/AlgLog/aaaa.txt", "w");
		//	fprintf(pFFF, "File = %s\n", g_strFileName.c_str());
		//	fprintf(pFFF, "g_iBeginBlock = %d\n", g_iBeginBlock);
		//	fprintf(pFFF, "Wound = %d, %d, %d\n", vWounds[i].Block, vWounds[i].Step, vWounds[i].Type);
		//	fprintf(pFFF, "Step2 = %d\n", vWounds[i].Step2);
		//	fprintf(pFFF, "B1 = %d, B2 = %d\n", blocks[0].Index, blocks[blocks.size() - 1].Index);
		//	fprintf(pFFF, "%d\n", vWounds[i].Block - g_iBeginBlock);
		//	fclose(pFFF);
		//}

		if (vWounds[i].Manual != 0)
		{
			continue;
		}

		int iJawRow = g_iJawRow[blocks[vWounds[i].Block - g_iBeginBlock].BlockHead.railType & 0x03];
		int iFRow = blocks[vWounds[i].Block - g_iBeginBlock].BlockHead.railH / 3;
		int iMaxHeadRow = iJawRow << 1;
		for (int j = vWounds[i].vCRs.size() - 1; j >= 0; --j)
		{
			if (vWounds[i].vCRs[j].Channel == CH_D || vWounds[i].vCRs[j].Channel == CH_E)
			{
				if (vCRs[vWounds[i].vCRs[j].Channel][vWounds[i].vCRs[j].Index].IsDoubleWave == 1 && vCRs[vWounds[i].vCRs[j].Channel][vWounds[i].vCRs[j].Index].Region.size() == vWounds[i].vCRs[j].Region.size())
				{
					vWounds[i].vCRs[j].IsDoubleWave = 1;
				}
			}

			else if (vWounds[i].vCRs[j].Channel < CH_C && (vWounds[i].vCRs[j].Step1 == vWounds[i].vCRs[j].Step2 || vWounds[i].vCRs[j].Row1 == vWounds[i].vCRs[j].Row2))
			{
				RemoveWoundCR(vWounds[i], j);
				//vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
			}

			else if (vWounds[i].vCRs[j].Channel < CH_C && vWounds[i].vCRs[j].Row1 > iJawRow && iMaxHeadRow - vWounds[i].vCRs[j].Row2 <= 1)
			{
				if (IsHoleCR(vWounds[i].vCRs[j], vCRs, blocks, iJawRow, iMaxHeadRow, iFRow))
				{
					RemoveWoundCR(vWounds[i], j);
					//vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
				}
			}
		}

		if (vWounds[i].Type == W_SKEW_CRACK)
		{
			for (int j = vWounds[i].vCRs.size() - 1; j >= 0; --j)
			{
				if (vCRs[vWounds[i].vCRs[j].Channel][vWounds[i].vCRs[j].Index].IsScrewHole || vCRs[vWounds[i].vCRs[j].Channel][vWounds[i].vCRs[j].Index].IsGuideHole)
				{
					RemoveWoundCR(vWounds[i], j);
					//vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
				}
			}
		}

		if (vWounds[i].Type == W_SKEW_CRACK || vWounds[i].Type == W_SCREW_CRACK1 || vWounds[i].Type == W_SCREW_CRACK2 || vWounds[i].Type == W_SCREW_CRACK3 || vWounds[i].Type == W_SCREW_CRACK4
			|| vWounds[i].Type == W_GUIDE_CRACK1 || vWounds[i].Type == W_GUIDE_CRACK2 || vWounds[i].Type == W_GUIDE_CRACK3 || vWounds[i].Type == W_GUIDE_CRACK4
			)
		{
			for (int j = vWounds[i].vCRs.size() - 1; j >= 0; --j)
			{
				if (vWounds[i].vCRs[j].Row1 == vWounds[i].vCRs[j].Row2 && (vWounds[i].vCRs[j].Channel == CH_D || vWounds[i].vCRs[j].Channel == CH_E))
				{
					RemoveWoundCR(vWounds[i], j);
					//vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
				}
			}
		}

		if (vWounds[i].Manual == 0 && vWounds[i].vCRs.size() == 0 && vWounds[i].According.size() == 0)
		{
			SetWoundFlag(vWounds[i], 1);
		}

		if (vWounds[i].Block != g_iTotalBlock - 1 && vWounds[i].Step2 >= g_iEndStep - 20 && vWounds[i].Manual == 0)
		{
			if ( (vWounds[i].Type >= W_SCREW_CRACK1 && vWounds[i].Type <= W_GUIDE_HORIZON_CRACK_LEFT) || vWounds[i].Type == W_BOTTOM_TRANSVERSE_CRACK)
			{

			}
			else 
			{
				SetWoundFlag(vWounds[i], 1);
			}
		}

		if (vWounds[i].Block != 0 && vWounds[i].Step2 - g_iBeginStep <= 20 && vWounds[i].Manual == 1)
		{
			SetWoundFlag(vWounds[i], 1);
		}
	}

	std::map<uint8_t, bool> isChannelExist;
	std::map<uint8_t, bool> isChannelOK;
	for (int i = woundCount; i < vWounds.size(); ++i)
	{
		if (vWounds[i].Manual != 0 || vWounds[i].Flag != 0)
		{
			continue;
		}

		if (vWounds[i].Type == W_HS && vWounds[i].Manual == 0 && vWounds[i].IsMatched == 0 /* && vWounds[i].IsJoint == 0 && vWounds[i].IsSew == 0*/)
		{
			if (vWounds[i].vCRs.size() == 0)
			{
				continue;
				}

			for (int j = vWounds[i].vCRs.size() - 1; j >= 0; --j)
			{
				if (vWounds[i].vCRs[j].Channel < CH_D && IsCRDirty_ABC(blocks, vCRs, vWounds[i].vCRs[j], 1))
				{
					RemoveWoundCR(vWounds[i], j);
					//vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
				}
			}

			bool isOK = false;
			for (int j = 0; j < vWounds[i].vCRs.size(); ++j)
			{
				if (vWounds[i].vCRs[j].Channel < CH_D)
				{
					isOK = true;
					break;
				}
			}
			if (isOK == false)
			{
				//vWounds[i].Flag = 1;
				//SetWoundFlag(vWounds[i], 1);
			}
		}
		else if ((vWounds[i].Type == W_SKEW_CRACK || vWounds[i].Type == W_BOTTOM_EX) && vWounds[i].Manual == 0)
		{
			if (vWounds[i].vCRs.size() == 0)
			{
				continue;
			}
			isChannelExist.clear();
			for (int ic = 0; ic < vWounds[i].vCRs.size(); ++ic)
			{
				isChannelExist[vWounds[i].vCRs[ic].Channel] = true;
			}

			bool isOK = true;
			for (auto itr = isChannelExist.begin(); itr != isChannelExist.end(); ++itr)
			{
				if (itr->first != CH_D && itr->first != CH_E)
				{
					continue;
				}
				VINT crA;
				CR& temp = vWounds[i].vCRs[0];
				uint8_t row = temp.Row1 >= 2 ? temp.Row1 - 2 : 0;
				GetCR(temp.Channel, temp.Step1 - 300, row, temp.Step2 + 300, temp.Row2 + 2, blocks, vCRs[temp.Channel], crA, temp.Index, 1);
				int n = crA.size();
				if (temp.Region.size() >= 7 && n >= 5 || temp.Region.size() < 7 && temp.Region.size() >= 4 && n >= 3 || temp.Region.size() < 4 && n >= 2)
				{
					int n2 = n;
					for (int ia = n2 - 1; ia >= 0; --ia)
					{
						if (vCRs[temp.Channel][crA[ia]].IsScrewHole == 1 || vCRs[temp.Channel][crA[ia]].IsGuideHole == 1)
						{
							crA.erase(crA.begin() + ia);
							n--;
						}
					}
					int dRow = temp.Row2 - temp.Row1;
					if (dRow >= 5 && n < 5 || dRow < 5 && dRow > 3 && n < 2 || dRow <= 3 && n == 0)
					{
						isOK = false;
						break;
					}

					double ra1 = 0, ra2 = 0;
					for (int ia = 0; ia < n; ++ia)
					{
						ra1 += vCRs[temp.Channel][crA[ia]].Row1;
						ra2 += vCRs[temp.Channel][crA[ia]].Row2;
					}

					ra1 = ra1 / n;
					ra2 = ra2 / n;
					if (fabs(ra1 - temp.Row1) > 2 || fabs(ra2 - temp.Row2) > 2)
					{
						isOK = false;
						break;
					}
				}
				else
				{
					isOK = false;
				}
			}

			if (isOK)
			{
				SetWoundFlag(vWounds[i], 1);
			}
		}
		else if (vWounds[i].Type == W_HORIZONAL_CRACK && vWounds[i].Manual == 0 &&
			vWounds[i].IsJoint == 0 && vWounds[i].IsSew == 0 &&
			vWounds[i].IsScrewHole == 0 && vWounds[i].IsGuideHole == 0
			)
		{
			for (int j = vWounds[i].vCRs.size() - 1; j >= 0; --j)
			{
				if (vWounds[i].vCRs[j].Channel >= CH_F && vWounds[i].vCRs[j].IsLose == 0)
				{
					CR& cr = vWounds[i].vCRs[j];
					VINT tcrL, tcrR, tcrM;

					GetCR(cr.Channel, cr.Step1 - 300, cr.Row1 - 1, cr.Step2, cr.Row2 + 1, blocks, vCRs[cr.Channel], tcrR, cr.Index);
					GetCR(cr.Channel, cr.Step1 - 150, cr.Row1 - 1, cr.Step2 + 150, cr.Row2 + 1, blocks, vCRs[cr.Channel], tcrM, cr.Index);
					GetCR(cr.Channel, cr.Step1, cr.Row1 - 1, cr.Step2 + 300, cr.Row2 + 1, blocks, vCRs[cr.Channel], tcrL, cr.Index);
					RemoveHoleCR(vCRs[cr.Channel], tcrR);
					RemoveHoleCR(vCRs[cr.Channel], tcrM);
					RemoveHoleCR(vCRs[cr.Channel], tcrL);

					if (tcrR.size() >= 5 || tcrM.size() >= 5 || tcrL.size() >= 5)
					{
						vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
						if (vWounds[i].vCRs.size() == 0)
						{
							SetWoundFlag(vWounds[i], 1);
						}
					}
					else
					{
						uint8_t ch = CH_F + CH_G - cr.Channel;
						VINT tcrL2, tcrR2, tcrM2;

						GetCR(ch, cr.Step1 - 300, cr.Row1 - 1, cr.Step2, cr.Row2 + 1, blocks, vCRs[ch], tcrR2, -1);
						GetCR(ch, cr.Step1 - 150, cr.Row1 - 1, cr.Step2 + 150, cr.Row2 + 1, blocks, vCRs[ch], tcrM2, -1);
						GetCR(ch, cr.Step1, cr.Row1 - 1, cr.Step2 + 300, cr.Row2 + 1, blocks, vCRs[ch], tcrL2, -1);
						RemoveHoleCR(vCRs[ch], tcrR2);
						RemoveHoleCR(vCRs[ch], tcrM2);
						RemoveHoleCR(vCRs[ch], tcrL2);

						if (tcrR2.size() >= 5 || tcrM2.size() >= 5 || tcrL2.size() >= 5)
						{
							vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
							if (vWounds[i].vCRs.size() == 0)
							{
								SetWoundFlag(vWounds[i], 1);
							}
						}
					}
				}
			}
		}
	}

#pragma region 轨底横向裂纹
	if (g_isTestEqu == 0/* || g_isTestEqu != 0*/)
	{
		for (int i = woundCount; i < vWounds.size(); ++i)
		{
			if (vWounds[i].Type == W_BOTTOM_TRANSVERSE_CRACK && vWounds[i].Manual == 0 && vWounds[i].IsMatched == 0 && vWounds[i].IsJoint == 0 && vWounds[i].IsSew == 0)
			{
				if (vWounds[i].vCRs.size() == 0)
				{
					continue;
				}
				bool isDirtyD = true, isDirtyE = true;
				bool bFindD = false, bFindE = false;
				uint8_t row1 = VALID_ROW, row2 = 0;
				for (int j = vWounds[i].vCRs.size() - 1; j >= 0; --j)
				{
					if (IsCRDirty_DE(blocks, vCRs, vWounds[i].vCRs[j], 1))
					{
						RemoveWoundCR(vWounds[i], j);
						//vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
					}
					else
					{
						if (vWounds[i].vCRs[j].Channel == CH_D)
						{
							isDirtyD = false;
							bFindD = true;
						}
						else if (vWounds[i].vCRs[j].Channel == CH_E)
						{
							isDirtyE = false;
							bFindE = true;
						}
						row1 = (std::min)(row1, vWounds[i].vCRs[j].Row1);
						row2 = (std::max)(row2, vWounds[i].vCRs[j].Row2);
					}
				}

				if (bFindD == false || bFindE == false || row2 - row1 <= 2 && (isDirtyD || isDirtyE))
				{
					//vWounds[i].Flag = 1;
					SetWoundFlag(vWounds[i], 1);
				}
			}
		}


		std::map<uint8_t, bool> isChannelExist;
		for (int i = woundCount; i < vWounds.size(); ++i)
		{
			if (vWounds[i].Type == W_BOTTOM_TRANSVERSE_CRACK && vWounds[i].Manual == 0)
			{
				bool bFindD = false, bFindE = false;
				double wrd1 = 0, wrd2 = 0, wre1 = 0, wre2 = 0, dCount = 0, ecount = 0, minD2 = VALID_ROW, maxD2 = 0, minE2 = VALID_ROW, maxE2 = 0;
				for (int id = 0; id < vWounds[i].vCRs.size(); ++id)
				{
					if (vWounds[i].vCRs[id].Channel == CH_D)
					{
						wrd1 += vWounds[i].vCRs[id].Row1;
						wrd2 += vWounds[i].vCRs[id].Row2;
						minD2 = (std::min)(minD2, (double)vWounds[i].vCRs[id].Row1);
						maxD2 = (std::max)(maxD2, (double)vWounds[i].vCRs[id].Row2);
						dCount++;
						bFindD = true;
					}
					else if (vWounds[i].vCRs[id].Channel == CH_E)
					{
						wre1 += vWounds[i].vCRs[id].Row1;
						wre2 += vWounds[i].vCRs[id].Row2;
						minE2 = (std::min)(minE2, (double)vWounds[i].vCRs[id].Row1);
						maxE2 = (std::max)(maxE2, (double)vWounds[i].vCRs[id].Row2);
						ecount++;
						bFindE = true;
					}
				}

				if (bFindD == false || bFindE == false)
				{
					SetWoundFlag(vWounds[i], 1);
					continue;
				}

				wrd1 /= dCount;
				wrd2 /= dCount;
				wre1 /= ecount;
				wre2 /= ecount;

				VINT crD, crE, crDExcept, crEExcept;
				for (int id = 0; id < vWounds[i].vCRs.size(); ++id)
				{
					if (vWounds[i].vCRs[id].Channel == CH_D)
					{
						crDExcept.emplace_back(vWounds[i].vCRs[id].Index);
					}
					else if (vWounds[i].vCRs[id].Channel == CH_E)
					{
						crEExcept.emplace_back(vWounds[i].vCRs[id].Index);
					}
				}

				VINT dlow, dhigh, elow, ehigh;
				double avd1 = 0, avd2 = 0, ave1 = 0, ave2 = 0, minD = VALID_ROW, maxD = 0, minE = VALID_ROW, maxE = 0;
				GetCR(CH_D, vWounds[i].Step2 - 500, wrd1 - 3, vWounds[i].Step2 - 50, wrd2 + 3, blocks, vCRs[CH_D], crD, crDExcept);
				GetCR(CH_D, vWounds[i].Step2 + 50, wrd1 - 3, vWounds[i].Step2 + 500, wrd2 + 3, blocks, vCRs[CH_D], crD, crDExcept);
				GetCR(CH_E, vWounds[i].Step2 - 500, wre1 - 3, vWounds[i].Step2 - 50, wre2 + 3, blocks, vCRs[CH_E], crE, crEExcept);
				GetCR(CH_E, vWounds[i].Step2 + 50, wre1 - 3, vWounds[i].Step2 + 500, wre2 + 3, blocks, vCRs[CH_E], crE, crEExcept);
				int n1 = crD.size(), n2 = crE.size();

				for (int id = 0; id < n1; ++id)
				{
					dlow.emplace_back(vCRs[CH_D][crD[id]].Row1);
					dhigh.emplace_back(vCRs[CH_D][crD[id]].Row2);
					avd1 += vCRs[CH_D][crD[id]].Row1;
					avd2 += vCRs[CH_D][crD[id]].Row2;
					minD = (std::min)(minD, (double)vCRs[CH_D][crD[id]].Row1);
					maxD = (std::max)(maxD, (double)vCRs[CH_D][crD[id]].Row2);
				}
				avd1 = avd1 / dlow.size();
				avd2 = avd2 / dlow.size();

				for (int ie = 0; ie < n2; ++ie)
				{
					elow.emplace_back(vCRs[CH_E][crE[ie]].Row1);
					ehigh.emplace_back(vCRs[CH_E][crE[ie]].Row2);
					ave1 += vCRs[CH_E][crE[ie]].Row1;
					ave2 += vCRs[CH_E][crE[ie]].Row2;
					minE = (std::min)(minE, (double)vCRs[CH_E][crE[ie]].Row1);
					maxE = (std::max)(maxE, (double)vCRs[CH_E][crE[ie]].Row2);
				}
				ave1 = ave1 / elow.size();
				ave2 = ave2 / elow.size();

				if (n1 >= 15 && n2 >= 15 && wrd2 - wrd1 <= 3 && wre2 - wre1 <= 3)
				{
					SetWoundFlag(vWounds[i], 1);
				}
				else if (n1 >= 15 && wrd2 - wrd1 <= 3 || n2 >= 15)
				{
					if (
						((wrd1 - avd1 > -1.0 && wrd2 - avd2 <= 1.0) || (wre1 - ave1 >= -1.0 &&  wre2 - ave2 <= 1.0))
						&& (n1 != 0 && maxD2 <= maxD + 2 && minD2 >= maxD - 2 || n1 == 0)
						&& (n2 != 0 && maxE2 <= maxE + 2 && minE2 >= maxE - 2 || n2 == 0)
						)
						SetWoundFlag(vWounds[i], 1);
				}
				/*
				else if (n1 >= 5 && wrd2 - wrd1 <= 2 && dCount >= 2)//D 是杂波
				{
					SetWoundFlag(vWounds[i], 1);
				}
				else if (n2 >= 5 && wre2 - wre1 <= 2 && ecount >= 2)//E 是杂波
				{
					SetWoundFlag(vWounds[i], 1);
				}
				*/
				else if (n1 >= 5 && wrd2 - wrd1 <= 2 && dCount >= 2)//D 是杂波
				{
					SetWoundFlag(vWounds[i], 1);
				}
				else if (n2 >= 5 && wre2 - wre1 <= 2 && ecount >= 2)//E 是杂波
				{
					SetWoundFlag(vWounds[i], 1);
				}
			}
		}
	}
#pragma endregion

#pragma endregion

	VPM vvv;
	for (int i = markCount; i < vPMs.size(); ++i)
	{
		vvv.emplace_back(vPMs[i]);
	}
	std::sort(vvv.begin(), vvv.end(), PMCompare);
	for (int i = 0; i < vvv.size(); ++i)
	{
		if (vvv[i].Mark == PM_SEW_CH || vvv[i].Mark == PM_SEW_LRH && vvv[i].Manual == 0)
		{
			VINT crD, crE, crF, crG;
			int row1 = g_iJawRow[g_vBlockHeads[vvv[i].Block].BlockHead.railType & 0x03] + 5;
			bool bFindD = GetCR(CH_D, vvv[i].Step2 - 20, row1, vvv[i].Step2 + 20, g_vBlockHeads[vvv[i].Block].BlockHead.railH / 3 - 10, blocks, vCRs[CH_D], crD);
			bool bFindE = GetCR(CH_E, vvv[i].Step2 - 20, row1, vvv[i].Step2 + 20, g_vBlockHeads[vvv[i].Block].BlockHead.railH / 3 - 10, blocks, vCRs[CH_E], crE);
			bool bFindF = GetCR(CH_F, vvv[i].Step2 - 20, 0, vvv[i].Step2 + 20, g_vBlockHeads[vvv[i].Block].BlockHead.railH / 3 - 10, blocks, vCRs[CH_F], crF);
			bool bFindG = GetCR(CH_G, vvv[i].Step2 - 20, 0, vvv[i].Step2 + 20, g_vBlockHeads[vvv[i].Block].BlockHead.railH / 3 - 10, blocks, vCRs[CH_G], crG);
			bFindD = RemoveScrewHoleCR(vCRs[CH_D], crD);
			bFindE = RemoveScrewHoleCR(vCRs[CH_E], crE);
			bFindF = RemoveScrewHoleCR(vCRs[CH_F], crF);
			bFindG = RemoveScrewHoleCR(vCRs[CH_G], crG);

			bFindD = RemoveCRByWoundFlag(vCRs[CH_D], crD);
			bFindE = RemoveCRByWoundFlag(vCRs[CH_E], crE);
			bFindF = RemoveCRByWoundFlag(vCRs[CH_F], crF);
			bFindG = RemoveCRByWoundFlag(vCRs[CH_G], crG);

			if (bFindD || bFindE || bFindF || bFindG)
			{
				Wound_Judged wound;
				wound.Block = vvv[i].Block;
				wound.Step = vvv[i].Step;
				wound.Step2 = vvv[i].Step2;
				if (vvv[i].Mark == PM_SEW_CH)
				{
					wound.Type = W_SEWCH;
					wound.IsSew = 1;
				}
				else
				{
					wound.Type = W_SEWLR;
					wound.IsSew = 2;
				}
				wound.Place = WP_HEAD_MID;
				AddWoundData(wound, vCRs[CH_D], crD);
				AddWoundData(wound, vCRs[CH_E], crE);
				AddWoundData(wound, vCRs[CH_F], crF);
				AddWoundData(wound, vCRs[CH_G], crG);

				AddToWounds(vWounds, wound);
			}
		}
	}

	std::map<int32_t, B_Step> vSteps;
	for (int i = 0; i < blocks.size(); ++i)
	{
		for (int j = 0; j < blocks[i].vBStepDatas.size(); ++j)
		{
			vSteps[blocks[i].vBStepDatas[j].Step] = blocks[i].vBStepDatas[j];
		}
	}

	for (int i = woundCount; i < vWounds.size(); ++i)
	{
		if (vWounds[i].Type == W_VERTICAL_CRACK)
		{
			TRECT rect;
			GetWoundRect2(vWounds[i], 13, &rect);
#pragma region 回退点附近的失波不再判定			
			for (int j = g_vReturnSteps.size() - 1; j >= 0; --j)
			{
				if (rect.step1 <= g_vReturnSteps[j] + 2 && rect.step2 >= g_vReturnSteps[j] - 2)
				{
					SetWoundFlag(vWounds[i], 1);
					break;
				}

				else if (rect.step2 > g_vReturnSteps[j] + 2)
				{
					break;
				}
			}
			if (vWounds[i].Flag == 1)
			{
				continue;
			}
#pragma endregion

#pragma region FG 共同失波区域不在[10-1000]，不再判定		
			int bs = 0, es = 0;
			std::map<uint8_t, bool> existedChannels;
			VCR vcr[16];
			for (int j = 0; j < vWounds[i].vCRs.size(); ++j)
			{
				existedChannels[vWounds[i].vCRs[j].Channel] = true;
				vcr[vWounds[i].vCRs[j].Channel].emplace_back(vWounds[i].vCRs[j]);
			}

			int sF1 = 0x7FFFFFFF, sG1 = 0x7FFFFFFF, sF2 = 0, sG2 = 0;
			int iFRow = vWounds[i].Row1;
			for (int j = 0; j < vcr[CH_F].size(); ++j)
			{
				if (sF1 > vcr[CH_F][j].Step1)	sF1 = vcr[CH_F][j].Step1;
				if (sF2 < vcr[CH_F][j].Step2)	sF2 = vcr[CH_F][j].Step2;
			}
			for (int j = 0; j < vcr[CH_G].size(); ++j)
			{
				if (sG1 > vcr[CH_G][j].Step1)	sG1 = vcr[CH_G][j].Step1;
				if (sG2 < vcr[CH_G][j].Step2)	sG2 = vcr[CH_G][j].Step2;
			}

			int stepcount = GetOverlappedStep(sF1, sF2, sG1, sG2, bs, es);
			if (stepcount < 10 || stepcount > 1000)
			{
				SetWoundFlag(vWounds[i], 1);
				continue;
			}
#pragma endregion

			int iMaxLoseF = es - bs, iMaxLoseG = es - bs;
			int esF = es, bsF = bs, esG = es, bsG = bs;
			for (auto itr = vSteps.find(bs); itr->first < es; ++itr)
			{
				if (GetFRow2(itr->second, iFRow) > 0)
				{
					iMaxLoseF = (std::max)(esF - itr->first, itr->first - bsF);
					bsF = itr->first;
					if (iMaxLoseF < 10)
					{
						SetWoundFlag(vWounds[i], 1);
						break;
					}
				}
				if (GetGRow2(itr->second, iFRow) > 0)
				{
					iMaxLoseG = (std::max)(esG - itr->first, itr->first - bsG);
					bsG = itr->first;
					if (iMaxLoseG < 10)
					{
						SetWoundFlag(vWounds[i], 1);
						break;
					}
				}
			}

			if (vWounds[i].Flag == 1)
			{
				continue;
			}

#pragma region 只选择最大的失波		
			VINT crF, crG;

			GetCR(CH_F, sF1 - 500, iFRow - 1, sF1, iFRow + 1, blocks, vCRs[CH_F], crF, vcr[CH_F][0].Index, 2);
			GetCR(CH_F, sF2, iFRow - 1, sF2 + 500, iFRow + 1, blocks, vCRs[CH_F], crF, vcr[CH_F][0].Index, 2);

			GetCR(CH_G, sG1 - 500, iFRow - 1, sG1, iFRow + 1, blocks, vCRs[CH_G], crG, vcr[CH_G][0].Index, 2);
			GetCR(CH_G, sG2, iFRow - 1, sG2 + 500, iFRow + 1, blocks, vCRs[CH_G], crG, vcr[CH_G][0].Index, 2);

			int maxSizeF = iMaxLoseF, maxSizeG = iMaxLoseG;
			RemoveHoleCR(vCRs[CH_F], crF);
			RemoveHoleCR(vCRs[CH_G], crG);

			if (crF.size() > 0)
			{
				for (int j = 0; j < crF.size(); ++j)
				{
					if (vCRs[CH_F][crF[j]].Step2 - vCRs[CH_F][crF[j]].Step1 > maxSizeF)
					{
						SetWoundFlag(vWounds[i], 1);
						break;
					}
				}
			}

			if (vWounds[i].Flag == 0 && crG.size() > 0)
			{
				for (int j = 0; j < crG.size(); ++j)
				{
					if (vCRs[CH_G][crG[j]].Step2 - vCRs[CH_G][crG[j]].Step1 > maxSizeG)
					{
						SetWoundFlag(vWounds[i], 1);
						break;
					}
				}
			}
#pragma endregion 
		}
		else if (vWounds[i].Type == W_HORIZONAL_CRACK)
		{
			for (int j = g_vReturnSteps.size() - 1; j >= 0; --j)
			{
				if (vWounds[i].Step2 - g_vReturnSteps[j] <= 5 && vWounds[i].Step2 - g_vReturnSteps[j] >= 0)
				{
					SetWoundFlag(vWounds[i], 1);
					break;
				}
				else if (vWounds[i].Step2 > g_vReturnSteps[j] + 2)
				{
					break;
				}
			}
		}
		else if (vWounds[i].Type == W_GUIDE_CRACK1 || vWounds[i].Type == W_GUIDE_CRACK2 || vWounds[i].Type == W_GUIDE_CRACK3 || vWounds[i].Type == W_GUIDE_CRACK4 || vWounds[i].Type == W_SCREW_CRACK1 || vWounds[i].Type == W_SCREW_CRACK2 || vWounds[i].Type == W_SCREW_CRACK3 || vWounds[i].Type == W_SCREW_CRACK4)
		{
			for (int j = 0; j < vWounds[i].vCRs.size(); ++j)
			{
				if (vWounds[i].vCRs[j].Channel == CH_D || vWounds[i].vCRs[j].Channel == CH_E)
				{
					if (vWounds[i].vCRs[j].Step2 - vWounds[i].vCRs[j].Step1 < 2)
					{
						RemoveWoundCR(vWounds[i], j);
						//vWounds[i].vCRs.erase(vWounds[i].vCRs.begin() + j);
					}
				}
			}
		}
	}

	for (int i = woundCount; i < vWounds.size(); ++i)
	{
		if (vWounds[i].Manual == 1)
		{
			continue;
		}

		for (int j = 0; j < vWounds[i].vCRs.size(); ++j)
		{
			if (vWounds[i].vCRs[j].Info.MaxV == 0 && vWounds[i].vCRs[j].Region.size() > 0)
			{
				GetCRInfo(vWounds[i].vCRs[j], DataA, blocks, 0);
			}
		}
	}

	if (g_vHoleParas.size() > 0)
	{
		for (auto itrhole = (--g_vHoleParas.end()); ; --itrhole)
		{
			if (itrhole->first < blocks[0].IndexL2)
			{
				break;
			}

			HolePara& hp = itrhole->second;
			memset(hp.channels, 0, 10);
			VINT tcr[10];
			for (int j = 0; j < 10; ++j)
			{
				int stepMiddle = itrhole->first + itrhole->second.FLength / 2;
				if (j >= CH_C)
				{
					if (j % 4 == 0)
					{
						hp.channels[j] = GetCR(j, stepMiddle, 10, itrhole->first + 20, 26, blocks, vCRs[j], tcr[j]);
					}
					else
					{
						hp.channels[j] = GetCR(j, itrhole->first - 5, 10, stepMiddle, 26, blocks, vCRs[j], tcr[j]);
					}
				}
				else
				{
					if (j % 4 == 0)
					{
						hp.channels[j] = GetCR(j, itrhole->first - 10, 10, stepMiddle, 26, blocks, vCRs[j], tcr[j]);
					}
					else
					{
						hp.channels[j] = GetCR(j, stepMiddle, 10, itrhole->first + 20, 26, blocks, vCRs[j], tcr[j]);
					}
				}

				hp.channels[j] = RemovePMCR(vCRs[j], tcr[j]);
				for (int k = tcr[j].size() - 1; k >= 0; --k)
				{
					if (j < CH_C && vCRs[j][tcr[j][k]].Row1 <= 13)
					{
						tcr[j].erase(tcr[j].begin() + k);
					}
				}
				if (hp.channels[j] > 0)
				{
					hp.row1[j] = 26;
					hp.row2[j] = 0;
					for (int k = 0; k < tcr[j].size(); ++k)
					{
						CR& cr = vCRs[j][tcr[j][k]];
						if (cr.Row1 < hp.row1[j])
						{
							hp.row1[j] = cr.Row1;
						}
						if (cr.Row2 > hp.row2[j])
						{
							hp.row2[j] = cr.Row2;
						}
					}
				}
			}

			if (itrhole == g_vHoleParas.begin())
			{
				break;
			}
		}
	}

	for (int i = vWounds.size() - 1; i >= woundCount; --i)
	{
		if (vWounds[i].vCRs.size() == 0 && vWounds[i].According.size() == 0 && vWounds[i].Manual == 0)
		{
			vWounds.erase(vWounds.begin() + i);
			continue;
		}

		//if (g_iBeginBlock != 0 && vWounds[i].Step2 < g_iBeginStep + 10)
		//{
		//	vWounds.erase(vWounds.begin() + i);
		//	continue;
		//}

		if (vWounds[i].Flag == 1)
		{
			vWounds.erase(vWounds.begin() + i);
		}
	}

	for (int i = vPMs.size() - 1; i >= markCount; --i)
	{
		if (vPMs[i].Flag == 1)
		{
			vPMs.erase(vPMs.begin() + i);
		}
	}

	if (g_StepPoints != nullptr)
	{
		delete[]g_StepPoints;
		g_StepPoints = nullptr;
	}
}

void Analyse2(BlockData_A& DataA, VBDB& blocks, VBA& vBA, VWJ& vWounds, VPM& vPMs, VER& vER, VPM& vPMs2, VLCP& vLCP, VPM& vPMsYOLO)
{

}

bool FindNextJoint(int block, int step, VBDB& blocks, int* destBlock, int * destStep)
{
	int istep = step;
	bool bFind = false;
	for (int j = 0; j < blocks[block].vBStepDatas.size(); ++j)
	{
		if (blocks[block].vBStepDatas[j].Step >= step && blocks[block].vBStepDatas[j].Mark.Mark & BIT4)
		{
			bFind = true;
			*destBlock = block;
			*destStep = j;
			return true;
		}

		if (j == blocks[block].vBStepDatas.size() - 1)
		{
			++block;
			j = 0;
		}
	}
	return false;
}

bool FindNextWeld(int block, int step, VBDB& blocks, int* destBlock, int * destStep)
{
	int istep = step;
	bool bFind = false;
	for (int j = 0; j < blocks[block].vBStepDatas.size(); ++j)
	{
		if (blocks[block].vBStepDatas[j].Step >= step && blocks[block].vBStepDatas[j].Mark.Mark & BIT4)
		{
			bFind = true;
			*destBlock = block;
			*destStep = j;
			return true;
		}

		if (j == blocks[block].vBStepDatas.size() - 1)
		{
			++block;
			j = 0;
		}
	}
	return false;
}

int	GetMaxIndex(int* data1, int* data2, int count)
{
	int index = 0, maxValue = data1[0] + data2[0];
	for (int i = 1; i < count; ++i)
	{
		if (data1[i] + data2[i] > maxValue)
		{
			index = i;
			maxValue = data1[i] + data2[i];
		}
	}
	return index;
}

std::string GetWoundAccordding(Wound_Judged& wound, VBDB& blocks)
{
	int railType = blocks[wound.Block].BlockHead.railType & 0x03;
	std::string str = "";
	std::map<std::string, uint8_t> mapStr;
	for (int i = 0; i < wound.According.size(); ++i)
	{
		str.append(wound.According[i]).append("\r\n");
	}

	//处理各通道出波的判据
	std::string strAccording = str;
	char strTemp[400] = { 0 };
	mapStr.clear();

	VCR vCRs[18];
	int wpcount[18] = { 0 };
	std::vector<WaveData> vPoints[18];
	std::vector<A_Step> vASteps[18];

	uint8_t isJoint = 0, isSew = 0;

	for (int i = 0; i < wound.vCRs.size(); ++i)
	{
		int channel = wound.vCRs[i].Channel;
		if (channel >= CH_F && wound.vCRs[i].IsLose == 1)
		{
			channel += 2;
		}
		else
		{
			if (wound.vCRs[i].IsJoint != 0)
			{
				isJoint = wound.vCRs[i].IsJoint;
			}
			if (wound.vCRs[i].IsSew != 0)
			{
				isSew = wound.vCRs[i].IsSew;
			}
		}
		wpcount[channel] += wound.vCRs[i].Region.size();
		for (int j = 0; j < wound.vCRs[i].Region.size(); ++j)
		{
			if (wound.vCRs[i].Region[j].find & BIT0)
			{
				vPoints[channel].emplace_back(wound.vCRs[i].Region[j]);
			}
		}
		vCRs[channel].emplace_back(wound.vCRs[i]);
		for (int j = 0; j < wound.vCRs[i].vASteps.size(); ++j)
		{
			vASteps[channel].emplace_back(wound.vCRs[i].vASteps[j]);
		}
	}

	isJoint = wound.IsJoint;
	isSew = wound.IsSew;
	for (int i = 0; i < wound.vLCRs.size(); ++i)
	{
		if (wound.vLCRs[i].IsJoint != 0)
		{
			isJoint = wound.vLCRs[i].IsJoint;
		}
		if (wound.vLCRs[i].IsSew != 0)
		{
			isSew = wound.vLCRs[i].IsSew;
		}
	}
	wound.IsJoint = isJoint;
	wound.IsSew = isSew;

	int totalSize = 0;
	for (int i = 0; i < 18; ++i)
	{
		if (vPoints[i].size() > 0)
		{
			std::sort(vPoints[i].begin(), vPoints[i].end());
			totalSize += vPoints[i].size();
		}
	}

	int sssssss = 0;
	sssssss = 1;
	printf("%d", sssssss);
	for (int i = 0; i < 18; ++i)
	{
		if (vPoints[i].size() > 0)
			for (int j = vPoints[i].size() - 1; j >= 1; --j)
			{
				if (vPoints[i][j] == vPoints[i][j - 1])
				{
					vPoints[i].erase(vPoints[i].begin() + j);
				}
			}
		wpcount[i] = vPoints[i].size();
	}

	if (wound.Manual == 0)
	{
		int bAa = (vPoints[CH_A1].size() > 0 ? 1 : 0) + (vPoints[CH_A2].size() > 0 ? 1 : 0) + (vPoints[CH_a1].size() > 0 ? 1 : 0) + (vPoints[CH_a2].size() > 0 ? 1 : 0);
		int bBb = (vPoints[CH_B1].size() > 0 ? 1 : 0) + (vPoints[CH_B2].size() > 0 ? 1 : 0) + (vPoints[CH_b1].size() > 0 ? 1 : 0) + (vPoints[CH_b2].size() > 0 ? 1 : 0);
		int bCc = (vPoints[CH_C].size() > 0 ? 1 : 0) + (vPoints[CH_c].size() > 0 ? 1 : 0);
		int bDE = (vPoints[CH_D].size() > 0 ? 1 : 0) + (vPoints[CH_E].size() > 0 ? 1 : 0);
		int bFindFG = wpcount[CH_F] + wpcount[CH_G];
		int bLoseFG = wpcount[CH_FL] + wpcount[CH_GL];

		if (wound.IsScrewHole == 0 && wound.IsGuideHole == 0)
		{
			bool isVertical = (wound.Type == W_VERTICAL_CRACK);
			int iJawRow = g_iJawRow[railType];
			int iFrow = g_vBlockHeads[wound.Block].BlockHead.railH / 3;
			GetJawRow(0, wound.Step2, iJawRow, iFrow, railType);
			int distribute[18][4] = { 0 };
			for (int i = 0; i < 18; ++i)
			{
				for (int j = 0; j < vPoints[i].size(); ++j)
				{
					if ((vPoints[i][j].find & BIT0) == 0)
					{
						continue;
					}
					if (vPoints[i][j].row <= iJawRow - 3)
					{
						distribute[i][0] += 1;
					}
					else if (vPoints[i][j].row <= iJawRow + 6)
					{
						distribute[i][1] += 1;
					}
					else if (vPoints[i][j].row <= iFrow - 6)
					{
						distribute[i][2] += 1;
					}
					else
					{
						distribute[i][3] += 1;
					}
				}
			}

			bool carType = g_vBlockHeads[wound.Block].BlockHead.detectSet.Identify & BIT2;// 车型，1-右手车，0-左手车
			if (bFindFG > 0)
			{
				int mIndex = GetMaxIndex(distribute[CH_F], distribute[CH_G], 4);
				if (mIndex == 0)
				{
					if (distribute[CH_F][mIndex] + distribute[CH_G][mIndex] >= 0.3 * totalSize)
					{
						wound.Type = W_HORIZONAL_CRACK;
						wound.Place = WP_HEAD;
					}
					else if (
						bCc + bDE <= 1 && bAa == 0 && bBb == 0
						)
					{
						wound.Type = W_HORIZONAL_CRACK;
						wound.Place = WP_HEAD;
						if (bCc == 1)
						{
							wound.Place = WP_HEAD_MID;
						}
					}
					else if (bCc > 0 && bDE > 0)
					{
						wound.Type = W_HEAD_EX;
						wound.Place = WP_HEAD;
					}
				}
				else if (mIndex == 1)
				{
					wound.Place = WP_JAW_IN;
					if (distribute[CH_F][mIndex] + distribute[CH_G][mIndex] >= 0.3 * totalSize)
					{
						wound.Type = W_HORIZONAL_CRACK;
					}
					else if (
						bCc + bDE <= 1 && bAa == 0 && bBb == 0
						)
					{
						wound.Type = W_HORIZONAL_CRACK;
						if (bCc == 1)
						{
							wound.Type = W_JAW_EX;
						}
					}
					else if (bCc > 0 && bDE > 0)
					{
						wound.Type = W_JAW_EX;
					}
				}
				else if (mIndex == 2)
				{
					wound.Place = WP_WAIST;
					wound.Type = W_HORIZONAL_CRACK;
					if (bCc > 0)
					{
						wound.Type = W_WAIST_EX;
					}
				}
				else if (mIndex == 3)
				{
					wound.Place = WP_BOTTOM;
					wound.Type = W_BOTTOM_EX;
				}
			}
			else if (bDE > 0)
			{
				int mIndex = GetMaxIndex(distribute[CH_D], distribute[CH_E], 4);
				if (mIndex == 0)
				{
					wound.Type = W_SKEW_CRACK;
					wound.Place = WP_HEAD_MID;
					if (bCc > 0)
					{
						wound.Type = W_HS;
					}
					else if (bAa + bBb > 0)
					{
						wound.Type = W_HEAD_EX;
						wound.Place = WP_HEAD;
					}
				}
				else if (mIndex == 1)
				{
					wound.Place = WP_JAW_IN;
					wound.Type = W_SKEW_CRACK;
					if (bCc + bAa + bBb > 0)
					{
						wound.Type = W_JAW_EX;
					}
				}
				else if (mIndex == 2)
				{
					wound.Place = WP_WAIST;
					wound.Type = W_SKEW_CRACK;
					if (bCc > 0 && wpcount[CH_C] + wpcount[CH_c] > (wpcount[CH_D] + wpcount[CH_E]) / 2)
					{
						wound.Type = W_HS;
						wound.Place = WP_HEAD_MID;
					}
				}
				else if (mIndex == 3)
				{
					wound.Place = WP_BOTTOM;
					if (vPoints[CH_D].size() == 0 || vPoints[CH_E].size() == 0)
					{
						wound.Type = W_BOTTOM_EX;
					}
					else
					{
						wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
					}
				}

			}
			else if (bCc > 0)
			{
				int mIndex = GetMaxIndex(distribute[CH_C], distribute[CH_c], 4);
				if (mIndex == 0)
				{
					wound.Type = W_HS;
					wound.Place = WP_HEAD_MID;
					if (bAa + bBb > 0)
					{
						int c1 = 2 * (vPoints[CH_C].size() + vPoints[CH_c].size());
						int c2 = vPoints[CH_A1].size() + vPoints[CH_a1].size();
						int c3 = vPoints[CH_b1].size() + vPoints[CH_b1].size();
						if (c2 > c1 && c2 > c3 && c1 < 5)//Aa
						{
							wound.Place = carType ? WP_HEAD_OUT : WP_HEAD_IN;
						}
						else if (c3 > c1 && c3 > c2 && c1 < 5)//Bb
						{
							wound.Place = carType ? WP_HEAD_IN : WP_HEAD_OUT;
						}
					}
				}
				else if (mIndex == 2)
				{
					wound.Type = W_HS;
					wound.Place = WP_JAW_IN;
				}
			}
			else if (bAa + bBb > 0)
			{
				wound.Type = W_HS;
				int c2 = vPoints[CH_A1].size() + vPoints[CH_a1].size();
				int c3 = vPoints[CH_B1].size() + vPoints[CH_b1].size();
				if (c2 > c3)//Aa
				{
					wound.Place = carType ? WP_HEAD_IN : WP_HEAD_OUT;
				}
				else
				{
					wound.Place = carType ? WP_HEAD_OUT : WP_HEAD_IN;
				}
			}

			if (isVertical)
			{
				wound.Type = W_VERTICAL_CRACK;
			}
		}

	}

#pragma region  判据
	int order[18] = { CH_F, CH_G, CH_C, CH_c, CH_D, CH_E, CH_A1, CH_B1, CH_a1, CH_b1, CH_A2, CH_a2, CH_B2, CH_b2, CH_d, CH_e, CH_FL, CH_GL };
	bool bFindF = false, bFindG = false, bLoseF = false, bLoseG = false;
	for (int od = 0; od < 18; ++od)
	{
		int m = order[od];
		if (vCRs[m].size() == 0)	continue;

		int iChA = GetAChannelByBChannel(m);
		if (m < CH_F)
		{
			uint16_t minH = 512, maxH = 0, maxV = 0, maxV2 = 0, shift = 0, shift2 = 0, minRow = VALID_ROW - 1, maxRow = 0;
			uint16_t h1 = 512, h2 = 0;
			uint8_t isLose = false;
			for (int i = 0; i < vCRs[m].size(); ++i)
			{
				if (minH > vCRs[m][i].Info.MinH)	minH = vCRs[m][i].Info.MinH;
				if (maxH < vCRs[m][i].Info.MaxH)	maxH = vCRs[m][i].Info.MaxH;
				if (maxV < vCRs[m][i].Info.MaxV)	maxV = vCRs[m][i].Info.MaxV;
				if (maxV2 < vCRs[m][i].Info.MaxV2)	maxV2 = vCRs[m][i].Info.MaxV2;
				if (shift < vCRs[m][i].Info.Shift)	shift = vCRs[m][i].Info.Shift;
				if (shift2 < vCRs[m][i].Info.Shift2)	shift2 = vCRs[m][i].Info.Shift2;
				if (h1 > vCRs[m][i].H1)	h1 = vCRs[m][i].H1;
				if (h2 < vCRs[m][i].H2)	h2 = vCRs[m][i].H2;
				if (minRow > vCRs[m][i].Row1)	minRow = vCRs[m][i].Row1;
				if (maxRow < vCRs[m][i].Row2)	maxRow = vCRs[m][i].Row2;
			}

			if (maxV > 0)
			{
				if (m >= CH_C)
				{
					//sprintf(strTemp, "通道%c(%.1fdB)：原幅值：%d，原位移：%.1f，新幅值：%d，新位移：%.1f (%d-%d) (%d-%d)",
					//	ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], maxV2, 0.02f*shift2, maxV, 0.02f*shift, minH, maxH, h1, h2);

					sprintf(strTemp, "通道%c(%.1fdB)：幅值：%d，位移：%.1f大格(%.1f-%.1f)，",
						ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], maxV2, 0.02f*shift, 0.02 * minH, 0.02 * maxH);
				}
				else
				{
					if (minRow >= g_iJawRow[railType])
					{
						sprintf(strTemp, "通道%c(%.1fdB)二次出波：幅值：%d，位移：%.1f大格(%.1f-%.1f)，",
							ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], maxV2, 0.02f*shift, 0.02 * minH, 0.02 * maxH);
					}
					else if (maxRow <= g_iJawRow[railType])
					{
						sprintf(strTemp, "通道%c(%.1fdB)一次出波：幅值：%d，位移：%.1f大格(%.1f-%.1f)，",
							ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], maxV2, 0.02f*shift, 0.02 * minH, 0.02 * maxH);
					}
					else
					{
						sprintf(strTemp, "通道%c(%.1fdB)一、二次出波：幅值：%d，位移：%.1f大格(%.1f-%.1f)，",
							ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], maxV2, 0.02f*shift, 0.02 * minH, 0.02 * maxH);
					}
				}
			}
			else
			{
				sprintf(strTemp, "通道%c(%.1fdB)出波，", ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA]);
			}
			strAccording.append(strTemp);
			sprintf(strTemp, "点数：%d", wpcount[m]);
			strAccording.append(strTemp);
			strAccording.append("\r\n");
		}
		else
		{
			for (int i = 0; i < vCRs[m].size(); ++i)
			{
				if (m == CH_F || m == CH_FL)
				{
					if (vCRs[m][i].IsLose == 1)
					{
						if (!bLoseF)
						{
							sprintf(strTemp, "通道%c(%.1fdB)失波，失波距离：%.1fmm", ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], 2.66 * (vCRs[m][i].Step2 - vCRs[m][i].Step1 + 1));
							strAccording.append(strTemp).append("\r\n");
							bLoseF = true;
						}
					}
					else
					{
						if (!bFindF)
						{
							sprintf(strTemp, "通道%c(%.1fdB)出波：幅值：%d，出波距离：%.1fmm",
								ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], vCRs[m][i].Info.MaxV2, 2.66 * (vCRs[m][i].Step2 - vCRs[m][i].Step1 + 1));
							strAccording.append(strTemp).append("\r\n");
							bFindF = true;
						}
					}
				}
				if (m == CH_G || m == CH_GL)
				{
					if (vCRs[m][i].IsLose == 1)
					{
						if (!bLoseG)
						{
							sprintf(strTemp, "通道%c(%.1fdB)失波，失波距离：%.1fmm", ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], 2.66f * (vCRs[m][i].Step2 - vCRs[m][i].Step1 + 1));
							strAccording.append(strTemp).append("\r\n");
							bLoseG = true;
						}
					}
					else
					{
						if (!bFindG)
						{
							sprintf(strTemp, "通道%c(%.1fdB)出波：幅值：%d，出波距离：%.1fmm",
								ChannelNames[iChA], 0.5 * blocks[wound.Block].BlockHead.gain[iChA], vCRs[m][i].Info.MaxV, 2.66 * (vCRs[m][i].Step2 - vCRs[m][i].Step1 + 1));
							strAccording.append(strTemp).append("\r\n");
							bFindG = true;
						}
					}
				}
			}
		}
	}
#pragma endregion
	return strAccording;
}

void FoldCR(CR& cr, uint8_t iJawRow)
{
	if (cr.Channel >= CH_C || cr.Row2 <= iJawRow)
	{
		return;
	}
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (cr.Region[i].row > iJawRow)
		{
			while (2 * iJawRow - cr.Region[i].row < 0)
			{
				iJawRow++;
			}
			cr.Region[i].row = 2 * iJawRow - cr.Region[i].row;
		}
	}
	FillCR(cr);
}

void FoldLCR(LCR& cr, uint8_t iJawRow)
{
	if (cr.Row1 >= iJawRow)
	{
		int t = cr.Row1;
		cr.Row1 = (iJawRow << 1) - cr.Row2;
		cr.Row2 = (iJawRow << 1) - t;
	}
	if (cr.Row2 > iJawRow)
	{
		cr.Row2 = iJawRow;
		cr.Row1 = (std::min)((int)cr.Row1, (iJawRow << 1) - cr.Row1);
	}
}


uint8_t	GetWoundRect2(Wound_Judged& wound, uint8_t iJawRow, TRECT* rect, uint8_t isFold /* = 0 */)
{
	try
	{
		if (wound.vCRs.size() == 0 && wound.vLCRs.size() == 0 && wound.IsMatched == 0)
		{
			return 0;
		}

		if (wound.vCRs.size() == 0 && wound.vLCRs.size() == 0 && wound.IsMatched == 1)
		{
			rect->step1 = wound.Step2;
			rect->step2 = wound.Step2 + wound.StepLen - 1;
			rect->row1 = wound.Row1;
			rect->row2 = wound.Row1 + wound.RowLen - 1;
			return 1;
		}

		rect->step1 = 0x7FFFFFFF;
		rect->step2 = 0;
		rect->row1 = 0xFF;
		rect->row2 = 0;

		bool isOnlyLose = true;
		for (int i = 0; i < wound.vCRs.size(); ++i)
		{
			if (wound.vCRs[i].IsLose == 0)
			{
				isOnlyLose = false;
				break;
			}
		}

		if (isOnlyLose)
		{
			wound.Place = WP_UNKNOWN;
		}

		if (wound.Type == W_VERTICAL_CRACK || isOnlyLose)
		{
			for (int i = 0; i < wound.vCRs.size(); ++i)
			{
				if (wound.vCRs[i].IsLose == 0)
				{
					continue;
				}

				CR cr = wound.vCRs[i];
				if (cr.Channel < CH_C && cr.Row2 > iJawRow)
				{
					FoldCR(cr, iJawRow);
				}
				if (cr.Step2 > rect->step2)	rect->step2 = cr.Step2;
				if (cr.Step1 < rect->step1)	rect->step1 = cr.Step1;
				if (cr.Row2 > rect->row2)	rect->row2 = cr.Row2;
				if (cr.Row1 < rect->row1)	rect->row1 = cr.Row1;
			}
		}
		else
		{
			for (int i = 0; i < wound.vCRs.size(); ++i)
			{
				if (wound.vCRs[i].Region.size() == 0)
				{
					continue;
				}
				if (wound.vCRs[i].IsLose == 1 && isOnlyLose == false)
				{
					continue;
				}

				CR cr = wound.vCRs[i];
				if (cr.Channel < CH_C && cr.Row2 > iJawRow)
				{
					FoldCR(cr, iJawRow);
				}
				if (cr.Step2 > rect->step2)	rect->step2 = cr.Step2;
				if (cr.Step1 < rect->step1)	rect->step1 = cr.Step1;
				if (cr.Row2 > rect->row2)	rect->row2 = cr.Row2;
				if (cr.Row1 < rect->row1)	rect->row1 = cr.Row1;
			}
		}

		if (wound.vLCRs.size() > 0)
		{
			for (int i = 0; i < wound.vLCRs.size(); ++i)
			{
				LCR cr = wound.vLCRs[i];
				if (cr.Channel < CH_C && cr.Row2 > iJawRow)
				{
					FoldLCR(cr, iJawRow);
				}
				if (cr.Step2 > rect->step2)	rect->step2 = cr.Step2;
				if (cr.Step1 < rect->step1)	rect->step1 = cr.Step1;
				if (cr.Row2 > rect->row2)	rect->row2 = cr.Row2;
				if (cr.Row1 < rect->row1)	rect->row1 = cr.Row1;
			}
		}

		return 1;
	}
	catch (std::bad_alloc* e)
	{
		return 0;
	}
	catch (...)
	{
		return 0;
	}

}


PM   FindPM(int stepBegin, int stepEnd, VPM& vPMs, int iBeginIndex, int *pIndex)
{
	PM pm;
	memset(&pm, 0, sizeof(PM));
	*pIndex = -1;
	for (int i = iBeginIndex; i < vPMs.size(); i++)
	{
		if (vPMs[i].Step2 > stepEnd)
		{
			break;
		}
		if (vPMs[i].Step2 < stepBegin)
		{
			continue;
		}
		if (IsJoint(vPMs[i].Mark))
		{
			*pIndex = i;
			return vPMs[i];
		}
	}
	return pm;
}

void GetOverlappedBigLose(int step1, int step2, CR& bigLose)
{
	VCR vLose;
	for (int i = 0; i < g_vFLoseBig.size(); ++i)
	{
		if (g_vFLoseBig[i].Step1 > step2)
		{
			break;
		}
		if (g_vFLoseBig[i].Step2 < step1)
		{
			continue;
		}
		vLose.push_back(g_vFLoseBig[i]);
	}
	if (vLose.size() > 0)
	{
		std::sort(vLose.begin(), vLose.end(), [&](CR& a, CR& b) {return a.Step2 - a.Step1 > b.Step2 - b.Step1; });
		bigLose = vLose[0];
	}
}


int IsInSection(int data, std::vector<Section>& vSec, int& flag)
{
	for (int i = 0; i < vSec.size(); ++i)
	{
		if (data > vSec[i].Start && data < vSec[i].End)
		{
			flag = vSec[i].Flag;
			return i;
		}
	}
	return -1;
}


void FilterByFork(VWJ& vWounds, VPM& vPMs, VBDB& blocks, FILE* pFileB, uint32_t szB)
{
	std::map<int32_t, bool> vStepInForks;
	int beginStep = 0, mark = 0;
	for (int i = 0; i < vPMs.size(); ++i)
	{
		if (vPMs[i].Mark == PM_HJFORK_BEGIN || vPMs[i].Mark == PM_MGFORK_BEGIN)
		{
			beginStep = vPMs[i].Mark;
			mark = vPMs[i].Mark;
			for (int j = i + 1; j < vPMs.size(); ++j)
			{
				if (vPMs[j].Mark == mark + 1)
				{
					for (int k = beginStep; k < vPMs[j].Step2; ++k)
					{
						vStepInForks[k] = 1;
					}
					i = j;
					break;
				}
			}
		}
	}

	int countPrev = vWounds.size();
	int iIndex = 0, iwoundIndex = 0;
	for (int i = 0; i < g_vFLoseBig.size(); ++i)
	{
		int s = g_vFLoseBig[i].Step1, e = g_vFLoseBig[i].Step2;
		VBDB blockst;
		int iEndBlock = FindStepInBlock(e, blocks, 0).Block;
		if (iEndBlock - g_vFLoseBig[i].Block > 30)
		{
			continue;
		}
		for (int j = g_vFLoseBig[i].Block; j <= iEndBlock; ++j)
		{
			BlockData_B block;
			if (g_isPTData)
			{
				GetBlockBStepsPT(pFileB, szB, blocks[j], block);
			}
			else
			{
				GetBlockBSteps(pFileB, szB, blocks[j], block);
			}
			blockst.emplace_back(block);
		}

		VCR vCRs[16];
		VER vER;
		VPM vSmarts;
		g_iBeginBlock = g_vFLoseBig[i].Block;
		CreateCR(blockst, vCRs, vER, vSmarts);

		int iFind = iIndex;
		PM pm = FindPM(s - 30, e + 30, vPMs, 0, &iFind);
		if (iFind < 0)
		{
			continue;
		}

		iIndex = iFind;
		for (int j = iwoundIndex; j < vWounds.size(); ++j)
		{
			if (vStepInForks.find(vWounds[j].Step2) == vStepInForks.end())
			{
				continue;
			}
			if (vWounds[j].Step2 > e + 5)
			{
				iwoundIndex = j;
				break;
			}
			if (vWounds[j].Step2 < s - 5)
			{
				continue;
			}
			for (int k = vWounds[j].vCRs.size() - 1; k >= 0; --k)
			{
				bool isOK = true;
				VINT crA;
				CR tempMax = vWounds[j].vCRs[k];
				bool isFirst = true;
				CR temp;
				std::map<uint8_t, int> rowDistribute;
				GetCoreCR(tempMax, rowDistribute, temp, blocks, g_iJawRow[blocks[vWounds[j].Block].BlockHead.railType & 0x03]);

				uint8_t row = temp.Row1 >= 2 ? temp.Row1 - 2 : 0;
				GetCR(temp.Channel, temp.Step1 - 300, row, temp.Step2 + 300, temp.Row2 + 2, blocks, vCRs[temp.Channel], crA, tempMax.Index, 1);

#ifdef _DEBUG
				VCR vTempCR;
				for (int j = 0; j < crA.size(); ++j)
				{
					vTempCR.emplace_back(vCRs[temp.Channel][crA[j]]);
				}
#endif // _DEBUG

				if ((tempMax.Channel == CH_C || tempMax.Channel == CH_c) && tempMax.Row2 - tempMax.Row1 <= 3 && crA.size() >= 5)
				{
					isOK = true;
				}
				else
				{
					int n = crA.size();
					if (temp.Region.size() >= 7 && n >= 5 || temp.Region.size() < 7 && temp.Region.size() >= 4 && n >= 3 || temp.Region.size() < 4 && n >= 2)
					{
						int n2 = n, nBig = 0;
						for (int ia = n2 - 1; ia >= 0; --ia)
						{
							if (vCRs[temp.Channel][crA[ia]].IsSew != 0 || vCRs[temp.Channel][crA[ia]].IsJoint != 0)
							{
								n--;
								crA.erase(crA.begin() + ia);
							}
							else
							{
								if (vCRs[temp.Channel][crA[ia]].Region.size() > 0.5 * temp.Region.size())
								{
									nBig++;
								}
							}
						}
						int dRow = temp.Row2 - temp.Row1;
						if (dRow >= 5 && n < 5 || dRow < 5 && dRow > 3 && n < 2 || dRow <= 3 && n == 0)
						{
							isOK = false;
							break;
						}
						if (nBig >= 5 && temp.Row2 >= 20)
						{
							vWounds[i].Type = W_YLS;
						}

						double ra1 = 0, ra2 = 0;
						for (int ia = 0; ia < n; ++ia)
						{
							ra1 += vCRs[temp.Channel][crA[ia]].Row1;
							ra2 += vCRs[temp.Channel][crA[ia]].Row2;
						}

						ra1 = ra1 / n;
						ra2 = ra2 / n;
						if (temp.Row1 < ra1 && fabs(ra1 - temp.Row1) > 2 || temp.Row2 >= ra2 && fabs(ra2 - temp.Row2) > 2)
						{
							isOK = false;
							break;
						}
					}
					else
					{
						isOK = false;
					}
				}

				if (isOK)
				{
					RemoveWoundCR(vWounds[j], k);
					//vWounds[j].vCRs.erase(vWounds[j].vCRs.begin() + k);
				}
			}

			if (vWounds[j].vCRs.size() == 0 || vWounds[j].Type == W_VERTICAL_CRACK)
			{
				SetWoundFlag(vWounds[j], 1);
			}
		}

		countPrev = vWounds.size();
		for (int i = countPrev - 1; i >= 0; --i)
		{
			if (vWounds[i].Flag == 1)
			{
				vWounds.erase(vWounds.begin() + i);
			}
		}
	}
}

void FilterByFork2(VWJ& vWounds, VPM& vPMs, VPM& vPMs2, VBDB& blocks)
{
	for (int i = g_vFLoseBig.size() - 1; i >= 0; --i)
	{
		if (g_vFLoseBig[i].Step2 - g_vFLoseBig[i].Step1 <= 100)
		{
			g_vFLoseBig.erase(g_vFLoseBig.begin() + i);
			continue;
		}

		Pos pos1 = FindStepInBlock(g_vFLoseBig[i].Step1, blocks, 0);
		g_vFLoseBig[i].Block = pos1.Block;
		g_vFLoseBig[i].Step = pos1.Step;
	}
	std::sort(g_vFLoseBig.begin(), g_vFLoseBig.end());

	std::vector<Section> vSections;
	/************************************************************************/
	/* 有的道岔会没有人工标记                                                                     */
	/************************************************************************/
	/*
	int beginStep = 0, mark = 0;
	for (int i = 0; i < vPMs2.size(); ++i)
	{

		if (vPMs2[i].Mark == PM_HJFORK_BEGIN || vPMs2[i].Mark == PM_MGFORK_BEGIN)
		{
			Section sec;
			sec.Start = vPMs2[i].Step2;
			beginStep = vPMs2[i].Mark;
			mark = vPMs2[i].Mark;
			for (int j = i + 1; j < vPMs.size(); ++j)
			{
				if (vPMs2[j].Mark == mark + 1)
				{
					sec.End = vPMs2[j].Step2;
					sec.Flag = 1;
					CR bigLose;
					GetOverlappedBigLose(beginStep, vPMs2[j].Step2, bigLose);
					if (bigLose.Step2 - bigLose.Step1 >= 500)
					{
						sec.Flag = 2;
					}
					vSections.push_back(sec);
					i = j;
					break;
				}
			}
		}
	}
	*/
	int iIndex = 0;
	int iFind = iIndex;
	for (int i = 0; i < g_vFLoseBig.size(); ++i)
	{
		int s = g_vFLoseBig[i].Step1, e = g_vFLoseBig[i].Step2;
		PM pm = FindPM(s - 30, e + 30, vPMs, 0, &iFind);
		if (iFind < 0)
		{
			continue;
		}

		Section sec;
		sec.Flag = 2;
		sec.Start = s;
		sec.End = e;
		vSections.emplace_back(sec);
	}

	int countPrev = vWounds.size();
	int iwoundIndex = 0;
	for (int i = 0; i < vSections.size(); ++i)
	{
		int s = vSections[i].Start, e = vSections[i].End;
		int iEndBlock = FindStepInBlock(e, blocks, 0).Block;

		for (int j = iwoundIndex; j < vWounds.size(); ++j)
		{
			if (vWounds[j].Step2 > e + 5)
			{
				iwoundIndex = j;
				break;
			}
			if (vWounds[j].Step2 < s - 5)
			{
				continue;
			}

			int flag = vSections[i].Flag;
			for (int k = vWounds[j].vCRs.size() - 1; k >= 0; --k)
			{
				/************************************************************************/
				/*	2020-11-13 锰钢岔失波区域上面FG出波	*/
				/************************************************************************/
				if (flag == 2 && vWounds[j].vCRs[k].Row2 <= g_iJawRow[2] && vWounds[j].vCRs[k].Step1 > s && vWounds[j].vCRs[k].Step1 < e)
				{
					if ((vWounds[j].vCRs[k].Channel == CH_F || vWounds[j].vCRs[k].Channel == CH_G) /*&& vWounds[j].vCRs[k].Step2 - vWounds[j].vCRs[k].Step1 <= 20*/)
					{
						vWounds[j].vCRs.erase(vWounds[j].vCRs.begin() + k);
					}
				}
			}

			if (vWounds[j].vCRs.size() == 0 || vWounds[j].Type == W_VERTICAL_CRACK)
			{
				SetWoundFlag(vWounds[j], 1);
			}
		}
	}

	countPrev = vWounds.size();
	for (int i = countPrev - 1; i >= 0; --i)
	{
		if (vWounds[i].Flag == 1)
		{
			vWounds.erase(vWounds.begin() + i);
		}
	}
}


void RemoveDuplicateHole(std::vector<HolePara>& vHoles)
{
	vHoles.clear();
	//FILE* pFile = fopen("D:/screwHole.txt", "w");
	for (auto itr = g_vHoleParas.begin(); itr != g_vHoleParas.end(); ++itr)
	{
		vHoles.emplace_back(itr->second);
		//fprintf(pFile, "Block = %d, Step = %d, Step2 = %d, Mark = %d\n", itr->second.Block, itr->second.Step, itr->second.step2, itr->second.mark);
	}
	//fclose(pFile);
	for (int i = vHoles.size() - 1; i >= 1; --i)
	{
		int d = vHoles[i].step2 - vHoles[i - 1].step2;
		if (Abs(d) <= 20)
		{
			vHoles.erase(vHoles.begin() + i);
		}
	}
}

void FilterByHole(VWJ& vWounds, VPM& vPM, VBDB& blocks, VBA& vBA)
{
	if (g_isTestEqu == 1)
	{
		return;
	}

	std::vector<HolePara> vHoles;
	RemoveDuplicateHole(vHoles);

	VINT vFlag;
	for (int i = 0; i < vHoles.size(); ++i)
	{
		vFlag.emplace_back(0);
	}
	if (vPM.size() % 2 != 0)
	{
		PM pm = vPM[vPM.size() - 1];
		pm.Mark += 1;
		pm.Step2 = blocks[blocks.size() - 1].IndexL2 + blocks[blocks.size() - 1].BlockHead.row - 1;
		pm.Block = blocks.size() - 1;
		pm.Step = blocks[blocks.size() - 1].BlockHead.row - 1;
		AddToMarks(pm, vPM);
	}

	int forkIndex = 0;
	//判断每个孔是否在道岔上
	for (int i = 0; i < vHoles.size(); ++i)
	{
		vHoles[i].isInFork = -1;
		vHoles[i].flag = 0;
		for (; forkIndex < vPM.size(); forkIndex += 2)
		{
			if (vHoles[i].step2 >= vPM[forkIndex].Step2 && vHoles[i].step2 <= vPM[forkIndex + 1].Step2)
			{
				vHoles[i].isInFork = forkIndex;
				break;
			}
			if (vPM[forkIndex].Step2 > vHoles[i].step2)
			{
				break;
			}
		}
	}


	int idx = 0;
	int woundIndex = 0;
	while (idx < vHoles.size())
	{
		if (vFlag[idx] == 1)
		{
			idx++;
			continue;
		}
		int lastIndex = idx;
		VINT vIndexes;
		for (int j = idx - 1; j >= 0; --j)
		{
			if (Abs(vHoles[lastIndex].FRow - vHoles[j].FRow) <= 1 &&
				(vHoles[lastIndex].isInFork >= 0 && vHoles[j].isInFork == vHoles[lastIndex].isInFork && vHoles[lastIndex].step2 - vHoles[j].step2 <= 1000 || vHoles[lastIndex].isInFork < 0 && vHoles[lastIndex].step2 - vHoles[j].step2 <= 600)
				)
			{
				int backIndex = -1;
				if (IsHaveBackPoint(vHoles[j].step2, vHoles[idx].step2, vBA, vBA.size(), backIndex))
				{
					break;
				}
				vIndexes.insert(vIndexes.begin(), j);
				lastIndex = j;
			}
			else
			{
				break;
			}
		}

		vIndexes.emplace_back(idx);
		lastIndex = idx;
		for (int j = idx + 1; j < vHoles.size(); ++j)
		{
			if (Abs(vHoles[lastIndex].FRow - vHoles[j].FRow) <= 1 &&
				(vHoles[lastIndex].isInFork >= 0 && vHoles[j].isInFork == vHoles[lastIndex].isInFork  && vHoles[j].step2 - vHoles[lastIndex].step2 <= 1000 || vHoles[lastIndex].isInFork < 0 && vHoles[j].step2 - vHoles[lastIndex].step2 <= 600))
			{
				int backIndex = -1;
				if (IsHaveBackPoint(vHoles[idx].step2, vHoles[j].step2, vBA, vBA.size(), backIndex))
				{
					break;
				}
				vIndexes.emplace_back(j);
				lastIndex = j;
			}
			else
			{
				break;
			}
		}

#ifdef _DEBUG
		HolePara hpFirst = vHoles[vIndexes[0]];
		HolePara hpLast = vHoles[vIndexes[vIndexes.size() - 1]];
#endif

		if (vIndexes.size() >= 2)
		{
			int row1[10] = { 0 };
			int row2[10] = { 0 };
			int count[10] = { 0 };
			int minR1 = VALID_ROW, maxR2 = 0;

			int s1 = vHoles[vIndexes[0]].step2;
			HolePara& endlistHP = vHoles[vIndexes[vIndexes.size() - 1]];
			int s2 = (std::max)((int)endlistHP.tempF.Step2, (int)(endlistHP.mark == PM_SCREWHOLE ? endlistHP.step2 + 10 : endlistHP.step2 + 5));

			std::vector<HolePara> vvvHP;

			for (int i = 0; i < vIndexes.size(); ++i)
			{
				for (int j = 0; j < 10; ++j)
				{
					if (vHoles[vIndexes[i]].channels[j] > 0)
					{
						count[j] += 1;
						row1[j] += vHoles[vIndexes[i]].row1[j];
						row2[j] += vHoles[vIndexes[i]].row2[j];
						if (j == 2)
						{
							vvvHP.emplace_back(vHoles[vIndexes[i]]);
						}
					}
				}
			}

			for (int j = 0; j < 10; ++j)
			{
				if (count[j] > 0)
				{
					row1[j] = row1[j] / count[j];
					row2[j] = row2[j] / count[j];
				}
			}
			//if (vHoles[vIndexes[0]].step2 <= 267610 && vHoles[vIndexes[vIndexes.size() - 1]].step2 >= 267610)
			//{
			//	int x = 0;
			//	x++;
			//}
			int wdIndex = woundIndex;
			for (int i = 0; i < vIndexes.size(); ++i)
			{
				vFlag[vIndexes[i]] = 1;
				for (; woundIndex < vWounds.size(); ++woundIndex)
				{
					Wound_Judged & wound = vWounds[woundIndex];
					TRECT rect;
					GetWoundRect2(vWounds[woundIndex], 13, &rect, 1);
					if (rect.step2 < vHoles[vIndexes[i]].step2 - 10)
					{
						continue;
					}
					if (rect.step1 > vHoles[vIndexes[i]].step2 + vHoles[vIndexes[i]].FLength + 10)
					{
						if (woundIndex != 0)
						{
							woundIndex -= 1;
						}
						break;
					}
					if (vWounds[woundIndex].Type == W_HS || vWounds[woundIndex].Type == W_YLS)
					{
						if (Abs(vWounds[woundIndex].Step2 - vHoles[vIndexes[i]].step2) <= 40)
						{
							for (int k = vWounds[woundIndex].vCRs.size() - 1; k >= 0; --k)
							{
								uint8_t channel = vWounds[woundIndex].vCRs[k].Channel;
								if (channel > CH_c)
								{
									continue;
								}
								if (
									channel >= CH_C && count[channel] >= 2 && vWounds[woundIndex].vCRs[k].Row1 >= row1[channel] - 1 && vWounds[woundIndex].vCRs[k].Row2 <= row2[channel] + 1
									||
									channel < CH_C && count[channel] >= 2 && vWounds[woundIndex].vCRs[k].Row1 >= row1[channel] - 2 && vWounds[woundIndex].vCRs[k].Row2 <= row2[channel] + 2
									)
								{
									RemoveWoundCR(vWounds[woundIndex], k);
									//vWounds[woundIndex].vCRs.erase(vWounds[woundIndex].vCRs.begin() + k);
								}
							}
						}

						if (vWounds[woundIndex].Type != W_VERTICAL_CRACK)
						{
							bool isOnlyLose = true;
							for (int k = vWounds[woundIndex].vCRs.size() - 1; k >= 0; --k)
							{
								if (vWounds[woundIndex].vCRs[k].Channel < CH_F || vWounds[woundIndex].vCRs[k].Channel >= CH_F && vWounds[woundIndex].vCRs[k].IsLose == 0)
								{
									isOnlyLose = false;
								}
							}
							if (isOnlyLose && vWounds[woundIndex].IsJoint == 0 && vWounds[woundIndex].IsSew == 0 && vWounds[woundIndex].IsScrewHole == 0)
							{
								SetWoundFlag(vWounds[woundIndex], 1);
							}
						}
						else if (vWounds[woundIndex].Type == W_VERTICAL_CRACK)
						{
							if (vWounds[woundIndex].vCRs.size() < 2)
							{
								SetWoundFlag(vWounds[woundIndex], 1);
							}
						}
					}
				}
			}
		}
		idx++;
	}
}

void FilterByVertical(VWJ& vWounds, VPM& vPM, VBDB& blocks)
{
	int iJumpIndex = 0;
	for (int i = 0; i < vWounds.size(); ++i)
	{
		if (vWounds[i].Type != W_VERTICAL_CRACK || vWounds[i].IsJoint != 0 || vWounds[i].IsSew != 0)
		{
			continue;
		}
		TRECT rect;
		GetWoundRect2(vWounds[i], 13, &rect, 1);

		int jawRow = 0, frow = 0, railType = 0;
		GetJawRow(0, rect.step1 - 10, jawRow, frow, railType);

		int jawRow2 = 0, frow2 = 0, railType2 = 0;
		GetJawRow(0, rect.step2 + 10, jawRow2, frow2, railType2);

		if (frow2 != frow)
		{
			SetWoundFlag(vWounds[i], 1);
		}

		if (vWounds[i].Flag == 1)
		{
			continue;
		}

		for (int j = iJumpIndex; j < g_vFR.size(); ++j)
		{
			if (Abs((int)(g_vFR[j].Step2 - rect.step1)) <= 10 || Abs((int)(g_vFR[j].Step2 - rect.step2)) <= 10)
			{
				SetWoundFlag(vWounds[i], 1);
				iJumpIndex = j;
				break;
			}
			if (g_vFR[j].Step2 > rect.step2)
			{
				break;
			}
		}
	}

	for (int i = vWounds.size() - 1; i >= 0; --i)
	{
		if (vWounds[i].Flag == 0 || vWounds[i].Manual != 0 || vWounds[i].IsMatched == 1)
		{
			continue;
		}
		vWounds.erase(vWounds.begin() + i);
	}
}

void FilterByDoubleHole(VWJ& vWounds, VPM& vPM, VBDB& blocks)
{
	for (int i = 0; i < vWounds.size(); ++i)
	{
		if (vWounds[i].Type != W_DOUBLEHOULE_SCREW)
		{
			continue;
		}
		uint8_t isNotLose = 0;
		for (int j = 0; j < vWounds[i].vCRs.size(); ++j)
		{
			if (vWounds[i].vCRs[j].IsLose == 0)
			{
				isNotLose = 1;
				break;
			}
		}
		if (isNotLose == 0)
		{
			SetWoundFlag(vWounds[i], 1);
		}
	}

	for (int i = vWounds.size() - 1; i >= 0; --i)
	{
		if (vWounds[i].Flag == 0 || vWounds[i].Manual != 0 || vWounds[i].IsMatched == 1)
		{
			continue;
		}
		vWounds.erase(vWounds.begin() + i);
	}
}