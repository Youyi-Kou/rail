#include "stdafx.h"
#include "CRHPE.h"
#include "PublicFunc.h"
#include "GlobalDefine.h"
#include <algorithm>

int g_iAFrameGapLimit = 60;

//是否折叠的CR，检测水平裂纹用，螺孔导孔处的DE通道用
bool	IsDoubleCR(CR& cr, VBDB& blocks, int channelDesired, CR& cr1, CR& cr2)
{
	std::map<uint8_t, _RowRegion> vR;
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (vR.find(cr.Region[i].row) != vR.end())
		{
			_RowRegion& rr = vR[cr.Region[i].row];
			if (rr.step1 > cr.Region[i].step)
			{
				rr.step1 = cr.Region[i].step;
			}
			if (rr.step2 < cr.Region[i].step)
			{
				rr.step2 = cr.Region[i].step;
			}
		}
		else
		{
			_RowRegion rr;
			rr.row = cr.Region[i].row;
			rr.step1 = cr.Region[i].step;
			rr.step2 = cr.Region[i].step;
			vR[rr.row] = rr;
		}
	}

	bool isFirst = true, isDouble = false;
	int iSplitRow = 0;
	if (channelDesired == CH_D)
	{
		int step1 = 0, step2 = 0, row = 0;
		int index = 0;
		for (auto itr = vR.begin(); itr != vR.end(); ++itr)
		{
			if (isFirst)
			{
				isFirst = false;
				step1 = itr->second.step1;
				step2 = itr->second.step2;
				row = itr->second.row;
			}
			else if ((itr->second.step1 > step1 || itr->second.step2 > step2) && index > 1)
			{
				iSplitRow = itr->second.row;
				isDouble = true;
				break;
			}
			else if (itr->first - row > 1 && (itr->second.step2 >= step2 || itr->second.step1 >= step1))
			{
				iSplitRow = itr->second.row;
				isDouble = true;
				break;
			}
			else
			{
				step1 = itr->second.step1;
				step2 = itr->second.step2;
			}
			row = itr->second.row;
			++index;
		}
	}
	else if (channelDesired == CH_E)
	{
		int step1 = 0, step2 = 0, row = 0;
		int index = 0;
		for (auto itr = vR.begin(); itr != vR.end(); ++itr)
		{
			if (isFirst)
			{
				isFirst = false;
				step1 = itr->second.step1;
				step2 = itr->second.step2;
				row = itr->second.row;
			}
			else if ((itr->second.step2 < step2 || itr->second.step1 < step1) && index > 1)
			{
				iSplitRow = itr->second.row;
				isDouble = true;
				break;
			}
			else if (itr->first - row > 1 && (itr->second.step2 <= step2 || itr->second.step1 <= step1))
			{
				iSplitRow = itr->second.row;
				isDouble = true;
				break;
			}
			else
			{
				step1 = itr->second.step1;
				step2 = itr->second.step2;
			}
			row = itr->second.row;
			++index;
		}
	}
	else
	{
		return false;
	}

	if (isDouble)
	{
		for (int i = 0; i < cr.Region.size(); ++i)
		{
			if (cr.Region[i].row < iSplitRow)
			{
				cr1.Region.emplace_back(cr.Region[i]);
			}
			else
			{
				cr2.Region.emplace_back(cr.Region[i]);
			}
		}
		cr1.Channel = cr.Channel;
		cr2.Channel = cr.Channel;
		cr1.Index = cr.Index;
		cr2.Index = cr.Index;
		FillCR(cr1);
		FillCR(cr2);
		cr1.Step = FindStepInBlock(cr1.Step1, g_vBlockHeads, 0).Step;
		cr2.Step = FindStepInBlock(cr2.Step1, g_vBlockHeads, 0).Step;
		return (cr2.Step2 - cr2.Step1 > 1);
	}
	return false;
}


//是否折叠的CR，检测∧或∨这种形状。0：单调，1：∧，2：∨
int		IsAngleCR(CR& cr, VBDB& blocks, CR& cr1, CR& cr2)
{
	std::map<uint32_t, _StepRegion> vStep;
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (vStep.find(cr.Region[i].step) != vStep.end())
		{
			_StepRegion& rr = vStep[cr.Region[i].step];
			if (rr.row1 > cr.Region[i].row)
			{
				rr.row1 = cr.Region[i].row;
			}
			if (rr.row2 < cr.Region[i].row)
			{
				rr.row2 = cr.Region[i].row;
			}
		}
		else
		{
			_StepRegion sr;
			sr.step = cr.Region[i].step;
			sr.row1 = cr.Region[i].row;
			sr.row2 = cr.Region[i].row;
			vStep[sr.step] = sr;
		}
	}

	if (vStep.size() <= 2)
	{
		return 0;
	}
	int stepEnd = (--vStep.end())->first;

	bool isUpper = true;//∧
	int middleStep = cr.Step1 + 1;
	int lastRow1 = -1;
	while (middleStep != stepEnd)
	{
		if (vStep.find(middleStep) == vStep.end())
		{
			middleStep++;
			continue;
		}
		if (vStep[middleStep].row1 != vStep[cr.Step1].row1 && middleStep - cr.Step1 != 1 && vStep[middleStep].row1 != lastRow1)
		{
			isUpper = vStep[middleStep].row1 < vStep[cr.Step1].row1;
			break;
		}
		lastRow1 = vStep[middleStep].row1;
		middleStep++;
	}

	bool isFirst = true, isDouble = false;
	int iSplitStep = 0;

	int row1 = 0, row2 = 0;
	int index = 0;
	for (auto itr = vStep.begin(); itr != vStep.end(); ++itr)
	{
		if (isFirst)
		{
			isFirst = false;
		}
		else if ((isUpper &&  itr->second.row2 > row2 || !isUpper && itr->second.row1 < row1))
		{
			if (itr->first < cr.Step2 - 2 && itr->first > cr.Step1 + 2)
			{
				iSplitStep = itr->second.step;
				isDouble = true;
				break;
			}
		}
		row1 = itr->second.row1;
		row2 = itr->second.row2;
		++index;
	}

	if (isDouble)
	{
		int tr1 = vStep[iSplitStep].row1, tr2 = vStep[iSplitStep].row2;
		int tr11 = tr1, tr12 = tr2, tr21 = tr1, tr22 = tr2;
		for (auto& s : vStep)
		{
			if (s.first < iSplitStep)
			{
				tr11 = min(s.second.row1, tr11);
				tr12 = max(s.second.row2, tr12);
			}
			else
			{
				tr21 = min(s.second.row1, tr21);
				tr22 = max(s.second.row2, tr22);
			}
		}

		if (tr12 - tr11 <= 2 || tr22 - tr21 <= 2)
		{
			return 0;
		}

		for (int i = 0; i < cr.Region.size(); ++i)
		{
			if (cr.Region[i].step < iSplitStep)
			{
				cr1.Region.emplace_back(cr.Region[i]);
			}
			else
			{
				cr2.Region.emplace_back(cr.Region[i]);
			}
		}
		cr1.Channel = cr.Channel;
		cr2.Channel = cr.Channel;
		cr1.Index = cr.Index;
		cr2.Index = cr.Index;
		FillCR(cr1);
		FillCR(cr2);
		cr1.Step = FindStepInBlock(cr1.Step1, g_vBlockHeads, 0).Step;
		cr2.Step = FindStepInBlock(cr2.Step1, g_vBlockHeads, 0).Step;
		return isUpper ? 2 : 1;
	}
	return 0;
}

int		IsAngleCR2(CR& cr, VBDB& blocks, CR& cr1, CR& cr2)
{
	std::map<uint32_t, _StepRegion> vStep;
	VINT vBottomSteps;
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (vStep.find(cr.Region[i].step) != vStep.end())
		{
			_StepRegion& rr = vStep[cr.Region[i].step];
			if (rr.row1 > cr.Region[i].row)
			{
				rr.row1 = cr.Region[i].row;
			}
			if (rr.row2 < cr.Region[i].row)
			{
				rr.row2 = cr.Region[i].row;
			}
		}
		else
		{
			_StepRegion sr;
			sr.step = cr.Region[i].step;
			sr.row1 = cr.Region[i].row;
			sr.row2 = cr.Region[i].row;
			vStep[sr.step] = sr;
		}
		if (cr.Region[i].row == cr.Row1)
		{
			vBottomSteps.emplace_back(cr.Region[i].step);
		}
	}

	int iSplitStep = 0;
	for (int i = 0; i < vBottomSteps.size(); ++i)
	{
		iSplitStep += vBottomSteps[i];
	}
	iSplitStep /= vBottomSteps.size();

	bool isDouble = vStep[cr.Step1].row1 > cr.Row1 && vStep[cr.Step2].row1 > cr.Row1 && iSplitStep > cr.Step1 + 1 && iSplitStep < cr.Step2 - 1;
	if (isDouble)
	{
		for (int i = 0; i < cr.Region.size(); ++i)
		{
			if (cr.Region[i].step < iSplitStep)
			{
				cr1.Region.emplace_back(cr.Region[i]);
			}
			else
			{
				cr2.Region.emplace_back(cr.Region[i]);
			}
		}
		cr1.Channel = cr.Channel;
		cr2.Channel = cr.Channel;
		cr1.Index = cr.Index;
		cr2.Index = cr.Index;
		FillCR(cr1);
		FillCR(cr2);
		cr1.Step = FindStepInBlock(cr1.Step1, g_vBlockHeads, 0).Step;
		cr2.Step = FindStepInBlock(cr2.Step1, g_vBlockHeads, 0).Step;
		return 1;
	}
	return 0;
}

int		IsHoleCR(CR& cr, VCR* vCRs, VBDB& blocks, uint8_t iJawRow, uint8_t iMaxHeadRow, uint8_t iFRow)
{
	return 0;
	if (cr.IsScrewHole == 1 || cr.IsGuideHole == 1)
	{
		return 1;
	}

	if (iMaxHeadRow - cr.Row2 > 1 || cr.Step2 - cr.Step1 <= 1)
	{
		return 0;
	}

	bool isLike = false;

#pragma region /--
	CR temp;
	std::map<uint8_t, int> rowDistribute;
	GetCoreCR(cr, rowDistribute, temp, blocks, iJawRow);
	int maxRowPointCount = (--rowDistribute.end())->second;
	int imaxRowIndex = rowDistribute.begin()->first;
	int imaxRowCount = rowDistribute.begin()->second;
	for (auto itr2 = rowDistribute.begin(); itr2 != rowDistribute.end(); ++itr2)
	{
		if (itr2->second >= imaxRowCount)
		{
			imaxRowIndex = itr2->first;
			imaxRowCount = itr2->second;
		}
	}
	if (imaxRowIndex >= iMaxHeadRow - 1)
	{
		isLike = true;
		if (imaxRowIndex == cr.Row2 && rowDistribute[imaxRowIndex] >= 5)
		{
			if (cr.Row1 == cr.Row2)
			{
				return 1;
			}
			if (rowDistribute.find(imaxRowIndex - 1) != rowDistribute.end() && rowDistribute[imaxRowIndex - 1] < rowDistribute[imaxRowIndex] - 2)
			{
				/*
				std::map<uint8_t, RowRegion> vRows;
				for (int i = 0; i < cr.Region.size(); ++i)
				{
					if (vRows.find(cr.Region[i].row) != vRows.end())
					{
						RowRegion& rr = vRows[cr.Region[i].row];
						if (rr.step1 > cr.Region[i].step)
						{
							rr.step1 = cr.Region[i].row;
						}
						if (rr.step2 < cr.Region[i].row)
						{
							rr.step2 = cr.Region[i].row;
						}
					}
					else
					{
						RowRegion rs;
						rs.row = cr.Region[i].row;
						rs.step1 = cr.Region[i].step;
						rs.step2 = cr.Region[i].step;
						vRows[cr.Region[i].row] = rs;
					}
				}
				return 1;
				*/

				if (cr.Row2 - cr.Row1 < 5)
				{
					return 1;
				}
			}
		}
	}

#pragma endregion

	if (isLike == false)
	{
		std::map<uint32_t, _StepRegion> vStep;
		int iTopStep = 0;
		for (int i = 0; i < cr.Region.size(); ++i)
		{
			if (vStep.find(cr.Region[i].step) != vStep.end())
			{
				_StepRegion& rr = vStep[cr.Region[i].step];
				if (rr.row1 > cr.Region[i].row)
				{
					rr.row1 = cr.Region[i].row;
				}
				if (rr.row2 < cr.Region[i].row)
				{
					rr.row2 = cr.Region[i].row;
				}
			}
			else
			{
				_StepRegion sr;
				sr.step = cr.Region[i].step;
				sr.row1 = cr.Region[i].row;
				sr.row2 = cr.Region[i].row;
				vStep[sr.step] = sr;
			}

			if (cr.Region[i].row == cr.Row2)
			{
				iTopStep = cr.Region[i].step;
			}
		}

		if (vStep[cr.Step1].row2 < cr.Row2 && vStep[cr.Step2].row2 < cr.Row2)
		{
			isLike = true;
		}
	}


	if (isLike)
	{
		VINT crD, crE, crF, crG, crF2, crG2;
		int s1 = cr.Step1 - 5, s2 = cr.Step2 + 5;
		uint8_t bFindD = GetCR(CH_D, s1, iJawRow + 1, s2, iFRow - 10, blocks, vCRs[CH_D], crD, -1, 3);
		uint8_t bFindE = GetCR(CH_E, s1, iJawRow + 1, s2, iFRow - 10, blocks, vCRs[CH_E], crE, -1, 3);
		uint8_t bFindF = GetCR(CH_F, s1, iJawRow + 1, s2, iFRow - 10, blocks, vCRs[CH_F], crF2, -1, 3);
		uint8_t bFindG = GetCR(CH_G, s1, iJawRow + 1, s2, iFRow - 10, blocks, vCRs[CH_G], crG2, -1, 3);
		uint8_t bLoseF = GetCR(CH_F, s1, iFRow - 3, s2, iFRow + 10, blocks, vCRs[CH_F], crF, -1, 3);
		uint8_t bLoseG = GetCR(CH_G, s1, iFRow - 3, s2, iFRow + 10, blocks, vCRs[CH_G], crG, -1, 3);

		if (bFindD || bFindE || bFindF || bFindG)
		{
			uint32_t step1 = 0x7FFFFFFF, step2 = 0;
			if (cr.Channel == CH_A1 || cr.Channel == CH_A2 || cr.Channel == CH_B1 || cr.Channel == CH_B2)
			{
				GetCRsStep(vCRs[CH_D], crD, step1, step2);
			}
			else
			{
				GetCRsStep(vCRs[CH_E], crE, step1, step2);
			}
			GetCRsStep(vCRs[CH_F], crF2, step1, step2);
			GetCRsStep(vCRs[CH_G], crG2, step1, step2);
			if (!(cr.Step1 > step2 - 5 || cr.Step2 < step1 + 5))
			{
				return 1;
			}
		}
		else if (bLoseF && bLoseG)
		{
			return 0;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

void    FillCR(CR& cr)
{
	if (cr.Region.size() == 0)
	{
		return;
	}
	int nsize = cr.Region.size();
	cr.Block = cr.Region[0].block;
	cr.Step1 = cr.Region[0].step;
	cr.Row1 = cr.Region[0].row;
	cr.Step2 = cr.Region[nsize - 1].step;
	cr.Row2 = cr.Region[nsize - 1].row;

	for (int j = 0; j < nsize; ++j)
	{
		if (cr.Region[j].step > cr.Step2)
		{
			cr.Step2 = cr.Region[j].step;
		}

		if (cr.Region[j].step < cr.Step1)
		{
			cr.Step1 = cr.Region[j].step;
		}

		if (cr.Region[j].row < cr.Row1)
		{
			cr.Row1 = cr.Region[j].row;
		}

		if (cr.Region[j].row > cr.Row2)
		{
			cr.Row2 = cr.Region[j].row;
		}
	}
}

uint32_t GetDistance(CR& cr, WaveData& wd)
{
	uint32_t dist = 0x7FFFFFFF;
	int n = cr.Region.size();
	for (int i = n - 1; i >= 0; --i)
	{
		int r = Abs(wd.row - cr.Region[i].row);
		int s = Abs(wd.step - cr.Region[i].step);
		uint32_t t = s + r;
		dist = (t > dist) ? dist : t;
		if (dist <= 1)
		{
			return 1;
		}
	}
	return dist;
}

uint32_t GetDistance(CR& cr1, CR& cr2, int* pH/* = NULL */, int * pV/* = NULL */)
{
	uint32_t dist = 0x7FFFFFFF;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();
	for (int i = 0; i < n1; ++i)
	{
		for (int j = 0; j < n2; ++j)
		{
			if (Abs(cr2.Region[j].block - cr1.Region[i].block) > 1)
			{
				continue;
			}

			int r = Abs(cr2.Region[j].row - cr1.Region[i].row);
			int s = Abs(cr2.Region[j].step - cr1.Region[i].step);
			uint32_t t = s + r;
			if (t < dist)
			{
				dist = t;
				if (pH != NULL)
				{
					*pH = s;
					*pV = r;
				}
			}
			if (dist <= 1)
			{
				return 1;
			}
		}
	}
	return dist;
}


uint32_t GetDistanceH(CR& cr1, CR& cr2, int& iOverlappedCount)
{
	int hDist = 999;
	iOverlappedCount = 0;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();
	for (int i = 0; i < n1; ++i)
	{
		for (int j = 0; j < n2; ++j)
		{
			if (cr1.Region[i].row == cr2.Region[j].row)
			{
				iOverlappedCount++;
				int t = Abs(cr1.Region[i].step - cr2.Region[j].step);
				hDist = t < hDist ? t : hDist;
				break;
			}
		}
	}
	return hDist;
}

uint32_t GetDistanceV(CR& cr1, CR& cr2, int& iOverlappedCount)
{
	int vDist = 999;
	iOverlappedCount = 0;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();
	for (int i = 0; i < n1; ++i)
	{
		for (int j = 0; j < n2; ++j)
		{
			if (cr1.Region[i].step == cr2.Region[j].step)
			{
				iOverlappedCount++;
				int t = Abs(cr1.Region[i].row - cr2.Region[j].row);
				vDist = t < vDist ? t : vDist;
				break;
			}
		}
	}
	return vDist;
}

//平均步进差
double	 GetDistanceHA(CR& cr1, CR& cr2)
{
	double r1 = 0.0, r2 = 0.0;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();

	int minRow = -100, maxRow = -100;
	GetOverlappedStep(cr1.Row1, cr1.Row2, cr2.Row1, cr2.Row2, minRow, maxRow);
	if (minRow >= 0)
	{
		int count1 = 0, count2 = 0;
		for (int i = 0; i < cr1.Region.size(); ++i)
		{
			if (cr1.Region[i].row >= minRow && cr1.Region[i].row <= maxRow)
			{
				r1 += cr1.Region[i].step;
				count1++;
			}
		}

		for (int i = 0; i < cr2.Region.size(); ++i)
		{
			if (cr2.Region[i].row >= minRow && cr2.Region[i].row <= maxRow)
			{
				r2 += cr2.Region[i].step;
				count2++;
			}
		}

		r1 = r1 / count1;
		r2 = r2 / count2;

	}
	else
	{
		for (int i = 0; i < n1; ++i)
			r1 += cr1.Region[i].step;
		r1 = r1 / n1;

		for (int i = 0; i < n2; ++i)
			r2 += cr2.Region[i].step;
		r2 = r2 / n2;
	}

	return fabs(r1 - r2);
}

//平均高度差
double	 GetDistanceVA(CR& cr1, CR& cr2)
{
	double r1 = 0.0, r2 = 0.0;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();
	int minStep = -100, maxStep = -100;
	GetOverlappedStep(cr1.Step1, cr1.Step2, cr2.Step1, cr2.Step2, minStep, maxStep);

	if (minStep >= 0)
	{
		int count1 = 0, count2 = 0;
		for (int i = 0; i < cr1.Region.size(); ++i)
		{
			if (cr1.Region[i].step >= minStep && cr1.Region[i].step <= maxStep)
			{
				r1 += cr1.Region[i].row;
				count1++;
			}
		}

		for (int i = 0; i < cr2.Region.size(); ++i)
		{
			if (cr2.Region[i].step >= minStep && cr2.Region[i].step <= maxStep)
			{
				r2 += cr2.Region[i].row;
				count2++;
			}
		}

		r1 = r1 / count1;
		r2 = r2 / count2;
	}
	else
	{
		for (int i = 0; i < n1; ++i)
			r1 += cr1.Region[i].row;
		r1 = r1 / n1;

		for (int i = 0; i < n2; ++i)
			r2 += cr2.Region[i].row;
		r2 = r2 / n2;
	}
	return fabs(r1 - r2);
}


int		GetOverlappedCountH(CR& cr1, CR& cr2)
{
	int count = 0;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();
	for (int i = 0; i < n1; ++i)
	{
		for (int j = 0; j < n2; ++j)
		{
			if (cr1.Region[i].step == cr2.Region[j].step)
			{
				++count;
				break;
			}
		}
	}
	return count;
}

int		GetOverlappedCountV(CR& cr1, CR& cr2)
{
	int count = 0;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();
	for (int i = 0; i < n1; ++i)
	{
		for (int j = 0; j < n2; ++j)
		{
			if (cr1.Region[i].row == cr2.Region[j].row)
			{
				++count;
				break;
			}
		}
	}
	return count;
}

bool	CanCombine(Connected_Region cr1, Connected_Region cr2, uint8_t channel)
{
	Connected_Region cr = cr1;
	if (cr1.Region.size() == 0 || cr2.Region.size() == 0 /*|| (cr1.Region.size() > 5 && cr2.Region.size() > 5)*/)
	{
		return false;
	}
	if (cr2.Step1 - cr.Step2 >= 5)   //步进差不大于5
	{
		return false;
	}

	int h = 0, v = 0;
	int dist = GetDistance(cr1, cr2, &h, &v);
	if (dist >= 5)   //两连通域距离大于5（不合并）
	{
		return false;
	}
	else if (dist > 1 && cr1.Region.size() >= 5 && cr2.Region.size() >= 5)   //两连通域距离大于1、两个连通域都有超过5个出波点（不合并）
	{
		return false;
	}
	if (dist == 1)   //两连通域距离为1（合并）
	{
		return true;
	}

	int vDist = 999;
	bool bOverlapped = false;
	int iOverlappedCount = 0;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();
	for (int i = 0; i < n1; ++i)
	{
		for (int j = 0; j < n2; ++j)
		{
			if (cr1.Region[i].step == cr2.Region[j].step)
			{
				iOverlappedCount++;
				int t = Abs(cr1.Region[i].row - cr2.Region[j].row);
				vDist = t < vDist ? t : vDist;
				break;
			}
		}
	}
	if (iOverlappedCount == 1 && vDist > 3 || iOverlappedCount > 1 && vDist >= 3 || iOverlappedCount >= 3)   //限定两连通域在同一步进是否出波和出波高度差（不合并）
	{
		return false;
	}

	int hDist = 999;
	bOverlapped = false;
	iOverlappedCount = 0;
	for (int i = 0; i < n1; ++i)
	{
		for (int j = 0; j < n2; ++j)
		{
			if (cr1.Region[i].row == cr2.Region[j].row)
			{
				iOverlappedCount++;
				int t = Abs(cr1.Region[i].step - cr2.Region[j].step);
				hDist = t < hDist ? t : hDist;
				break;
			}
		}
	}
	if (iOverlappedCount == 1 && hDist > 3 || iOverlappedCount > 1 && hDist >= 3 || iOverlappedCount >= 3)  //限定两连通域在同一行是否出波和出波步进差（不合并）
	{
		return false;
	}

	if (dist == 2 && h == 1 && v == 1)
	{

	}
	else   //不满足 两连通域距离为2 高度差和步进差均为1的情况
	{
		int iFirstRow1 = 0, iFirstRow2 = 0;
		int iLastRow1 = 0, iLastRow2 = 0;
		GetCRRowInfo(cr1, iFirstRow1, iLastRow1);
		GetCRRowInfo(cr2, iFirstRow2, iLastRow2);
		if (iFirstRow1 < iLastRow1 && iFirstRow2 > iLastRow2
			|| iFirstRow1 > iLastRow1 && iFirstRow2 < iLastRow2
			|| (Abs(iFirstRow1 - iFirstRow2) >= 3 && Abs(iLastRow2 - iLastRow2) >= 3)
			|| (iFirstRow1 == iLastRow1 && cr1.Step2 - cr1.Step1 >= 2)
			|| (iFirstRow2 == iLastRow2 && cr2.Step2 - cr2.Step1 >= 2)
			)
		{
			return false;
		}
	}

	Combine(cr, cr2);
	int ifirstRow = 0, iLastRow = 0;
	GetCRRowInfo(cr, ifirstRow, iLastRow);
	if (channel == CH_D && ifirstRow < iLastRow || channel == CH_E && ifirstRow > iLastRow)
	{
		return false;
	}
	double k = -9999;
	// 基本满足两连通域平行
	if (channel == CH_D)
	{
		if (cr.Step1 != cr.Step2)
		{
			k = 1.0 * (cr.Row2 - cr.Row1) / (cr.Step2 - cr.Step1);
		}
		return k >= 0.3 && k <= 1;
	}
	else if (channel == CH_E)
	{
		if (cr.Step1 != cr.Step2)
		{
			k = 1.0 * (cr.Row2 - cr.Row1) / (cr.Step2 - cr.Step1);
		}
		return k >= 0.3 && k <= 1;
	}
	return false;
}


void    RemoveRepeatedPoints(CR& cr)
{
	if (cr.Region.size() == 0)
	{
		return;
	}

	std::sort(cr.Region.begin(), cr.Region.end());
	for (int i = cr.Region.size() - 1; i >= 1; --i)
	{
		if (cr.Region[i] == cr.Region[i - 1])   //通道、步进、行都完全相同才认为是重复出波点
		{
			cr.Region.erase(cr.Region.begin() + i);
		}
	}
}

void	CombineABC(VCR &vCR, uint8_t channel)
{
	int n = vCR.size();
	for (int idx = 0; idx < n; ++idx)
	{
		if (vCR[idx].IsUsed == 1)
		{
			continue;
		}
		for (int i_LastFind = idx + 1; i_LastFind < n; ++i_LastFind)
		{
			if (vCR[i_LastFind].Step1 - vCR[idx].Step2 > 2)
			{
				break;
			}

			int dist = GetDistance(vCR[idx], vCR[i_LastFind]);
			if (dist == 1 || dist == 2 && (vCR[idx].Region.size() >= 5 && vCR[i_LastFind].Region.size() < 5 || vCR[idx].Region.size() < 5 && vCR[i_LastFind].Region.size() >= 5))
			{
				Combine(vCR[idx], vCR[i_LastFind]);
				vCR[i_LastFind].Region.clear();
				vCR[i_LastFind].IsUsed = 1;
			}
		}
	}
}

void    CombineDE(VCR &vCR, uint8_t channel)
{
	int n = vCR.size();
	for (int idx = 0; idx < n; ++idx)
	{
		if (vCR[idx].IsUsed == 1)
		{
			continue;
		}
		for (int i_LastFind = idx + 1; i_LastFind < n; ++i_LastFind)
		{
			if (vCR[i_LastFind].Step1 - vCR[idx].Step2 > 10)
			{
				break;
			}

			if (CanCombine(vCR[idx], vCR[i_LastFind], channel))   //其他合并条件
			{
				Combine(vCR[idx], vCR[i_LastFind]);
				vCR[i_LastFind].Region.clear();
				vCR[i_LastFind].IsUsed = 1;
			}
		}
	}
}

void    CombineFG(VCR& vCR)
{
	int n = vCR.size();
	for (int idx = 0; idx < n; ++idx)
	{
		if (vCR[idx].IsUsed == 1 || vCR[idx].IsLose == 1)
		{
			continue;
		}
		int iFlag = (vCR[idx].IsLose);
		for (int i_LastFind = idx + 1; i_LastFind < n; ++i_LastFind)
		{
			if (vCR[i_LastFind].Region.size() == 0 || vCR[i_LastFind].IsLose != iFlag)
			{
				continue;
			}
			if (vCR[i_LastFind].Step2 < vCR[idx].Step1 - 2)
			{
				continue;
			}
			if (vCR[i_LastFind].Step1 > vCR[idx].Step2 + 3)   //步进差不大于3
			{
				break;
			}

			int dr1 = Abs(vCR[i_LastFind].Row1 - vCR[idx].Row1), dr2 = Abs(vCR[i_LastFind].Row2 - vCR[idx].Row2);
			if (GetDistance(vCR[i_LastFind], vCR[idx]) <= 3 &&
				(iFlag == 1 && dr1 <= 2 && dr2 <= 2 || iFlag == 0 && dr1 <= 2 && dr2 <= 2)   //距离小于3  同为失波或是出波  出波行差不大于2
				)
			{
				Combine(vCR[idx], vCR[i_LastFind]);
				vCR[i_LastFind].Region.clear();
				vCR[i_LastFind].IsUsed = 1;
			}
		}
	}
}


void    Combine(CR& cr1, CR& cr2)
{
	int n2 = cr2.Region.size();
	for (int i = 0; i < n2; ++i)
	{
		cr1.Region.emplace_back(cr2.Region[i]);
	}
	cr1.CombinedCount++;
	FillCR(cr1);
}

bool	TryCombineFGInHole(CR& cr1, CR& cr2, int lengthLimit/*= 25 */)
{
	if (cr1.Channel == cr2.Channel && cr1.Index == cr2.Index && cr1.Region.size() == cr2.Region.size())
	{
		return true;
	}

	if (cr2.Row1 < cr1.Row1 - 2 || cr2.Row2 > cr1.Row2 + 2)
	{
		return false;
	}


	if (Abs(cr1.Step2 - cr2.Step1) >= 25 || Abs(cr2.Step2 - cr1.Step1) >= 25)
	{
		if (cr1.Step1 > cr2.Step2 + 4 || cr1.Step2 < cr2.Step1 - 4)
		{
			return false;
		}

		int s = 0, e = 0;
		int os = GetOverlappedStep(cr1.Step1, cr1.Step2, cr2.Step1, cr2.Step2, s, e);
		VINT ss;
		ss.emplace_back(cr1.Step1);
		ss.emplace_back(cr1.Step2);
		ss.emplace_back(cr2.Step1);
		ss.emplace_back(cr2.Step2);
		std::sort(ss.begin(), ss.end());
		if (os <= 3 && ss[3] - ss[0] >= 25)
		{
			return false;
		}
	}

	if (max(cr2.Row2, cr1.Row2) - min(cr2.Row1, cr1.Row1) >= 3)
	{
		int s = 0, e = 0;
		int os = GetOverlappedStep(cr1.Step1, cr1.Step2, cr2.Step1, cr2.Step2, s, e);
		if (os >= 2)
		{
			return false;
		}
		double dv = GetDistanceVA(cr1, cr2);
		if (cr1.Region.size() >= 5 && cr2.Region.size() >= 5 && dv > 1)
		{
			return false;
		}
	}
	Combine(cr1, cr2);
	return true;
}

bool	TryCombineFG1InHole(CR& cr1, CR& cr2)
{
	if (cr2.Row1 < cr1.Row1 - 2 || cr2.Row2 > cr1.Row2 + 2)
	{
		return false;
	}


	if (Abs(cr1.Step2 - cr2.Step1) >= 15 || Abs(cr2.Step2 - cr1.Step1) >= 15)
	{
		int s = 0, e = 0;
		int os = GetOverlappedStep(cr1.Step1, cr1.Step2, cr2.Step1, cr2.Step2, s, e);
		VINT ss;
		ss.emplace_back(cr1.Step1);
		ss.emplace_back(cr1.Step2);
		ss.emplace_back(cr2.Step1);
		ss.emplace_back(cr2.Step2);
		std::sort(ss.begin(), ss.end());
		if (os <= 3 && ss[3] - ss[0] >= 25)
		{
			return false;
		}
	}

	Combine(cr1, cr2);
	return true;
}

void    RemoveInvalidCR(VCR& vCR)
{
	for (VCR::iterator itr = vCR.begin(); itr < vCR.end(); ++itr)
	{
		if (itr->Region.size() == 0 || (itr->Region.size() == 1 && itr->IsLose == 1))   //一个点的失波
		{
			itr = vCR.erase(itr);
			if (itr != vCR.begin())
			{
				--itr;
			}
			if (itr == vCR.end())
			{
				break;
			}
		}
	}
}

uint8_t    CreateCR(VBDB& datas, VCR* vCRs, VER& vER, VPM& vSmarts)
{
	try
	{
		std::vector<WaveData> wdata[16];	//16个通道的连通域
		int wdataCount[16] = { 0 };
		for (int i = 0; i < 16; ++i)
		{
			vCRs[i].clear();
			vCRs[i].reserve(400);
			if (i <= CH_F)
			{
				wdataCount[i] = 1000;
			}
			else
			{
				wdataCount[i] = 4000;
			}
			wdata[i].reserve(wdataCount[i]);
		}

		int dataSize = datas.size();

#pragma region AaBbCcDE
		//建立A1, A2, a1, a2, B1, B2, b1, b2
		WaveData wd;   //B超出失波记录
		for (int j = 0; j < dataSize; ++j)   //dataSize 米块个数
		{
			for (int step = 0; step < datas[j].vBStepDatas.size(); ++step)
			{
				int railType = datas[j].BlockHead.railType & 0x03;
				int maxRow = (g_iJawRow[railType] << 1);   //轨颚线行高
				for (int row = 0; row < datas[j].vBStepDatas[step].vRowDatas.size() && datas[j].vBStepDatas[step].vRowDatas[row].Row <= maxRow; ++row)
				{
					for (int m = 0; m < 8; ++m)
					{
						if (datas[j].vBStepDatas[step].vRowDatas[row].Point.Draw1 & bits[m])   //？？？
						{
							wd.block = datas[j].Index;
							wd.step = datas[j].vBStepDatas[step].Step + g_channelOffset[ChannelB2A[m]];
							wd.find = 1;   // BIT0：0：失波，1：出波，BIT7：0：未处理，1：已处理

							if (m % 2 == 0)
							{
								wd.row = datas[j].vBStepDatas[step].vRowDatas[row].Row;
								wdata[m].emplace_back(wd);
							}
							else//二次波：翻折
							{
								wd.row = maxRow - datas[j].vBStepDatas[step].vRowDatas[row].Row;
								wdata[m - 1].emplace_back(wd);   //m为通道索引 此处二次波m-1 将二次波的出波归并到一次波
							}
						}
					}
				}
			}
		}

		// C, c, D, d, E, e
		for (int j = 0; j < dataSize; ++j)
		{
			for (int step = 0; step < datas[j].vBStepDatas.size(); ++step)
			{
				for (int row = 0; row < datas[j].vBStepDatas[step].vRowDatas.size(); ++row)
				{
					for (int m = CH_C; m < CH_F; ++m)
					{
						if (datas[j].vBStepDatas[step].vRowDatas[row].Point.Draw1 & bits[m])   //十六进制数 各通道出波点匹配
						{
							wd.block = datas[j].Index;
							wd.row = datas[j].vBStepDatas[step].vRowDatas[row].Row;
							wd.step = datas[j].vBStepDatas[step].Step + g_channelOffset[ChannelB2A[m]];
							wd.find = 1;
							wdata[m].emplace_back(wd);
						}
					}
				}
			}
		}

#pragma endregion

#pragma region 分布参数计算
		//向后的最大步进偏移量，要修正0点
		int offsetMin = 0, offsetMax = 0;//offsetMin <= 0, offsetMax >= 0
		for (int i = 0; i < 12; ++i)
		{
			if (offsetMin > g_channelOffset[i])
			{
				offsetMin = g_channelOffset[i];
			}
			if (offsetMax < g_channelOffset[i])
			{
				offsetMax = g_channelOffset[i];
			}
		}

		g_iBeginStep = datas[0].IndexL2;
		g_iEndStep = datas[dataSize - 1].IndexL2 + datas[dataSize - 1].vBStepDatas.size() + offsetMax;
		g_stepCount = g_iEndStep - g_iBeginStep + 1;   //所有米块总步进数

		g_StepPoints = new StepChannelPointDistrubute[g_stepCount];  //定义每步进出波信息统计结构体，步进索引唯一，所有米块的步进连续
		memset(g_StepPoints, 0, sizeof(SCPD) * g_stepCount);   //初始化，全部赋为未出波值0，后续判断出波后再赋值 将数值0以单个字节逐个拷贝的方式放到指针变量g_StepPoints所指的内存中去


		for (int i = 0; i < g_stepCount; ++i)
		{
			g_StepPoints[i].Step = g_iBeginStep + i;
		}
		for (int i = 0; i < CH_F; ++i)
		{
			for (int j = 0; j < wdata[i].size(); ++j)
			{
				g_StepPoints[wdata[i][j].step - g_iBeginStep].Num[i]++;
				uint8_t railType = datas[wdata[i][j].block - g_iBeginBlock].BlockHead.railType & 0x3;
				uint8_t row1 = g_iJawRow[railType] - 3, row2 = g_iJawRow[railType] + 3;
				if (wdata[i][j].row >= row1 && wdata[i][j].row <= row2 || wdata[i][j].row >= g_iJawRow[railType] && i >= CH_C && i <= CH_c)
				{
					g_StepPoints[wdata[i][j].step - g_iBeginStep].NumJaw++;   //轨颚点数
				}
			}
		}

		for (int i = 0; i < g_stepCount; ++i)
		{
			for (int j = 0; j < CH_D; ++j)
			{
				if (g_StepPoints[i].Num[j] > 0)
				{
					g_StepPoints[i].ChannelNum++;
					g_StepPoints[i].TotalHead += g_StepPoints[i].Num[j];   //每步进 轨头 出波信息统计
				}
			}
			for (int j = CH_D; j < CH_F; ++j)
			{
				if (g_StepPoints[i].Num[j] > 0)
				{
					g_StepPoints[i].ChannelNum++;
					g_StepPoints[i].TotalWaist += g_StepPoints[i].Num[j];   //每步进 轨腰 出波信息统计
				}
			}
		}

		int * tempCountHead = new int[g_stepCount];   //轨头
		int * tempCountWaist = new int[g_stepCount];   //轨腰
		int* tempCountJaw = new int[g_stepCount];   //轨颚
		for (int i = 0; i < g_stepCount; ++i)
		{
			tempCountHead[i] = g_StepPoints[i].TotalHead;
			tempCountWaist[i] = g_StepPoints[i].TotalWaist;
			tempCountJaw[i] = g_StepPoints[i].NumJaw;
		}
		for (int i = 0; i < g_stepCount; ++i)
		{
			g_StepPoints[i].SumHead15 = Sum(tempCountHead, i - 15, i + 15, g_stepCount);   //轨头 15步进出波点数统计
			g_StepPoints[i].SumWaist15 = Sum(tempCountWaist, i - 15, i + 15, g_stepCount);   //轨腰 15步进出波点数统计
			g_StepPoints[i].SumJaw15 = Sum(tempCountJaw, i - 15, i + 15, g_stepCount);   //轨颚 15步进出波点数统计
		}
		delete tempCountHead;
		delete tempCountWaist;
		delete tempCountJaw;

#pragma endregion  

#pragma region 记录钢轨高度变化的位置

		std::map<uint32_t, bool> mapRising;
		for (int i = 0; i < vSmarts.size(); ++i)
		{
			for (int j = 0; j < 20; ++j)
			{
				if (vSmarts[i].Step2 + 10 - j >= 0)   //变坡点总步进
				{
					mapRising[vSmarts[i].Step2 + 10 - j] = 1;
				}
			}
		}

#pragma endregion  

#pragma region 计算FG轨底出波高度突变点

		std::vector<FullMeter> vJump;   //记突变点为vJump
		//首先计算每个步进底部FG出波行，防止底部突然出来一两个点，烦死人
		bool bFindF = true, bFindG = true, bLastFindF = true, bLastFindG = true;
		int16_t iFrow = datas[0].BlockHead.railH / 3, iGRow = iFrow;   // railH当前轨高mm
		if (iFrow < g_iBottomRow[0] - 5)  //g_iBottomRow轨底线行高  0~5行内为轨底范围
		{
			iFrow = g_iBottomRow[datas[0].BlockHead.railType & 0x03];   //datas[0]若最开始位置底部失波，跟随当前米块的出波高度
			iGRow = iFrow;
		}
		int16_t iLastFrow = iFrow, iLastGrow = iGRow;
		for (int j = 0; j < dataSize; ++j)
		{
			for (int step = 0; step < datas[j].vBStepDatas.size(); ++step)
			{
				B_Step& b_step = datas[j].vBStepDatas[step];
				b_step.isFindF = 1; b_step.isFindG = 1;
				iFrow = GetFRow(b_step);  
				iGRow = GetGRow(b_step);
				bFindF = true;
				bFindG = true;
				//若底部失波，先跟随前一步进的出波高度
				if (iFrow < 0)
				{
					bFindF = false;
					b_step.isFindF = 0;
					iFrow = iLastFrow;
				}
				if (iGRow < 0)
				{
					bFindG = false;
					b_step.isFindG = 0;
					iGRow = iLastGrow;
				}
				//若出波，但高度差大于3，记录为轨底出波高度突变点
				if (iFrow - iLastFrow >= 3 || iFrow - iLastFrow <= -3)
				{
					int16_t f2 = GetFRow2(b_step, iLastFrow);   //在固定行查找是否有出波
					if (f2 > 0)   //未出波 GetFRow2() return -1
					{
						bFindF = true;
						b_step.isFindF = 1;
						iFrow = f2;
					}
					else   //出波
					{
						FullMeter fm;
						fm.Channel = CH_F;
						fm.Block1 = j;
						fm.Block2 = step;
						vJump.emplace_back(fm);
					}
				}
				if (iGRow - iLastGrow >= 3 || iGRow - iLastGrow <= -3)
				{
					int16_t g2 = GetGRow2(b_step, iLastGrow);   //在固定行查找是否有出波
					if (g2 > 0)
					{
						bFindG = true;
						b_step.isFindG = 1;
						iGRow = g2;
					}
					else
					{
						FullMeter fm;
						fm.Channel = CH_G;
						fm.Block1 = j;
						fm.Block2 = step;
						vJump.emplace_back(fm);
					}
				}
				//把当前步进赋值给Last，循环继续
				datas[j].vBStepDatas[step].FRow = iFrow;
				datas[j].vBStepDatas[step].GRow = iGRow;
				iLastFrow = iFrow;
				iLastGrow = iGRow;
				bLastFindF = bFindF;
				bLastFindG = bFindG;
			}
		}
#pragma endregion  

#pragma region  计算FG轨底出波高度

		int StepWindowSize = 100;
		//往前后各寻找StepWindowSize个步进，确定其底部出波行
		int maxStep = datas[dataSize - 1].IndexL2 + datas[dataSize - 1].BlockHead.row;  // 最后一个米快初始步进 + 此米块多少个步进
		int minStep = datas[0].IndexL2;   // 第一个米快初始步进
		for (int i = 0; i < vJump.size(); ++i)
		{
			B_Step& bstep = datas[vJump[i].Block1].vBStepDatas[vJump[i].Block2];
			int step = bstep.Step;
			//if (step >= maxStep - StepWindowSize)
			//{
			//	continue;
			//}
			bool isContiniousChange = false;
			std::map<uint8_t, uint8_t> mapExistRows;

			if (vJump[i].Channel == CH_F)
			{
				//初始变坡点位置
				int frow = datas[vJump[i].Block1].vBStepDatas[vJump[i].Block2].FRow;
				std::vector<uint8_t> vRows1;
				std::vector<uint8_t> vRows2;
				GetFRowRegion(datas, step - StepWindowSize, step, step, mapExistRows, vRows1, false);   //获取突变点前100步进范围内初始变坡点行高
				GetFRowRegion(datas, step, step + StepWindowSize, step, mapExistRows, vRows2, false);   //获取突变点后100步进范围内初始变坡点行高
				uint8_t maxValue;
				int maxIndex;
				GetMaxValueAndIndex(vRows1, maxValue, maxIndex);   //获取突变点前100步进范围内出波最多的行
				SetFRowRegion(datas, step - StepWindowSize, step, maxIndex, mapRising);   //将计算出的出波高度应用到失波步进上
				GetMaxValueAndIndex(vRows2, maxValue, maxIndex);   //获取突变点后100步进范围内出波最多的行
				SetFRowRegion(datas, step, step + StepWindowSize, maxIndex, mapRising);   //将计算出的出波高度应用到失波步进上
			}
			else if (vJump[i].Channel == CH_G)
			{
				int frow = datas[vJump[i].Block1].vBStepDatas[vJump[i].Block2].FRow;
				std::vector<uint8_t> vRows1;
				std::vector<uint8_t> vRows2;
				GetFRowRegion(datas, step - StepWindowSize, step, step, mapExistRows, vRows1, true);
				GetFRowRegion(datas, step, step + StepWindowSize, step, mapExistRows, vRows2, true);
				uint8_t maxValue;
				int maxIndex;
				GetMaxValueAndIndex(vRows1, maxValue, maxIndex);
				SetFRowRegion(datas, step - StepWindowSize, step, maxIndex, mapRising, true);
				GetMaxValueAndIndex(vRows2, maxValue, maxIndex);
				SetFRowRegion(datas, step, step + StepWindowSize, maxIndex, mapRising, true);
			}
		}
#pragma endregion 

#pragma region 统计分布参数FG通道
		uint8_t iFind[16] = { 0 };

		//std::vector<WaveData> vLosePoint[16];

		//F, G
		for (int i = 0; i < dataSize; ++i)
		{
			bool isInSmark = false;
			for (int k = 0; k < vSmarts.size(); ++k)
			{
				if (datas[i].Index - vSmarts[k].Block <= 1 && vSmarts[k].Block - datas[i].Index <= 1)
				{
					isInSmark = true;
					break;
				}
			}

			for (int step = 0; step < datas[i].vBStepDatas.size(); ++step)
			{
				B_Step& b_step = datas[i].vBStepDatas[step];
				iFind[CH_F] = 0;
				iFind[CH_G] = 0;
				uint8_t fRow[16] = { 0 };
				fRow[CH_F] = b_step.FRow - 2;   //非轨底位置
				fRow[CH_G] = b_step.GRow - 2;   //非轨底位置
				for (int row = 0; row < b_step.vRowDatas.size(); ++row)
				{
					B_RowData& b_row = b_step.vRowDatas[row];
					//F,G通道在非轨底位置的出波
					for (int m = 14; m < 16; ++m)   //F,G通道
					{
						if (b_row.Point.Draw1 & bits[m] && b_row.Row < fRow[m])   //非轨底位置出波
						{
							wd.block = datas[i].Index;
							wd.row = b_row.Row;
							wd.step = b_step.Step + g_channelOffset[ChannelB2A[m]];
							wd.find = 1;
							wdata[m].emplace_back(wd);
							//wdata2[m].emplace_back(wd);
						}
					}

					int minRow = min(fRow[CH_F], fRow[CH_G]);
					//F,G通道在轨底的失波
					if (b_row.Row >= minRow)
					{
						if (b_row.Point.Draw1 & bits[CH_F])
						{
							wd.block = datas[i].Index;
							wd.row = b_row.Row;
							wd.step = b_step.Step + g_channelOffset[ACH_F];
							wd.find = 1;
							//wdata2[CH_F].emplace_back(wd);
							//wdata[CH_F].emplace_back(wd);
							++iFind[CH_F];
						}

						if (b_row.Point.Draw1 & bits[CH_G])
						{
							wd.block = datas[i].Index;
							wd.row = b_row.Row;
							wd.step = b_step.Step + g_channelOffset[ACH_G];
							wd.find = 1;
							//wdata2[CH_G].emplace_back(wd);
							//wdata[CH_G].emplace_back(wd);
							++iFind[CH_G];
						}
					}
				}

				if (iFind[CH_F] == 0)
				{
					wd.block = datas[i].Index;
					wd.row = fRow[CH_F] + 2;
					wd.step = b_step.Step + g_channelOffset[ACH_F];
					wd.find = 0;
					wdata[CH_F].emplace_back(wd);
					//vLosePoint[CH_F].emplace_back(wd);
				}
				if (iFind[CH_G] == 0)
				{
					wd.block = datas[i].Index;
					wd.row = fRow[CH_G] + 2;
					wd.step = b_step.Step + g_channelOffset[ACH_G];
					wd.find = 0;
					wdata[CH_G].emplace_back(wd);
					//vLosePoint[CH_G].emplace_back(wd);
				}
			}
		}
#pragma endregion

#pragma region 初步构建连通域
		//VCR vLose[16];
		//int32_t vLoseSize[16] = {0};

		//S5 获取连通域
		for (int m = 0; m < 16; ++m)
		{
			int iCount = wdata[m].size();  //wdata B超出失波记录
			int idx = 0;//寻找以第idx个出波位置为起点的连通域
			int ct = 0;
			while (idx < iCount)
			{
				Connected_Region cr;   //出波连通域结构体
				cr.IsUsed = 0;
				cr.Row1 = cr.Row2 = 0;
				cr.Step1 = cr.Step2 = 0;
				cr.Channel = m;
				int iFind = wdata[m][idx].find;   // BIT0：0:失波，1：出波，BIT7：0：未处理，1：已处理
				int t_tt = iFind & BIT7;

				if ((wdata[m][idx].find & BIT0) == 0)   // BIT0：0:失波
				{
					cr.IsLose = 1;
				}

				//if (m == 14 && wdata[m][idx].step == 871)
				//{
				//	int x = 0;
				//	x++;
				//}

				//还未处理
				if ((wdata[m][idx].find & BIT7) == 0)   // BIT7：0：未处理
				{
					wdata[m][idx].find |= BIT7;
					cr.IsUsed = 0;
					cr.Region.emplace_back(wdata[m][idx]);
					cr.Block = wdata[m][idx].block;

					++ct;
				}

				int i_LastFind = idx;
				for (int i = i_LastFind + 1; i < iCount; ++i)
				{
					WaveData &wd = wdata[m][i];
					if (wdata[m][i].find & BIT7)   //当前点已存在于某个连通域
					{
						continue;
					}
					else if (
						// 点距连通域距离为1则合并到连通域
						abs(wdata[m][i].step - wdata[m][i_LastFind].step) <= 1 &&
						abs(wdata[m][i].row - wdata[m][i_LastFind].row) <= 1 &&
						abs(wdata[m][i].block - wdata[m][i_LastFind].block) <= 1 &&
						(wdata[m][i].find & BIT0) == (wdata[m][i_LastFind].find & BIT0)
						)
					{
						wdata[m][i].find |= BIT7;
						cr.Region.emplace_back(wdata[m][i]);
						i_LastFind = i;
					}
					else if (wdata[m][i].block - wdata[m][i_LastFind].block > 1)
					{
						break;
					}
					else if (wdata[m][i].step - wdata[m][i_LastFind].step > 1 && wdata[m][i].row - wdata[m][i_LastFind].row > 1)
					{
						break;
					}
				}

				if (cr.Region.size() > 0)
				{
					vCRs[m].emplace_back(cr);
				}
				++idx;
			}
		}

		//计算连通域参数
		for (int m = 0; m < 16; ++m)
		{
			for (int i = 0; i < vCRs[m].size(); ++i)
			{
				FillCR(vCRs[m][i]);   //row1&row2 step1&step2
				vCRs[m][i].Step = vCRs[m][i].Step1 - datas[vCRs[m][i].Block - g_iBeginBlock].IndexL2;
				vCRs[m][i].IsUsed = 0;
				for (int j = 0; j < vCRs[m][i].Region.size(); ++j)
				{
					vCRs[m][i].Region[j].find &= 0x7F;   //0111 1111 BIT7全部标记为0：未处理  BIT0：0:失波，1：出波
				}

				if (vCRs[m][i].Region.size() > 0 && (vCRs[m][i].Region[0].find & BIT0) == 0)
				{
					vCRs[m][i].IsLose = 1;   //IsLose F、G通道轨底失波
				}

				//if (vCRs[m][i].IsLose == 1)
				//{
				//	vLose[m].emplace_back(vCRs[m][i]);
				//	vLoseSize[m] += vCRs[m][i].Region.size();
				//}
			}
		}
#pragma endregion

#pragma region AaBbCcDE通道连通域处理 ( 合并DE通道 + AaBbCcDE通道的二次合并 + DE通道特殊情况的分离处理 + AaBbCc通道合并去重)

		for (int m = CH_D; m <= CH_E; ++m)  //D、E
		{
			if (m == CH_d)
			{
				continue;
			}
			int idx = 0;
			int nCount = vCRs[m].size() - 1;
			while (idx < nCount)
			{
				for (int i_Last = idx + 1; i_Last < nCount; ++i_Last)
				{
					if (GetDistance(vCRs[m][idx], vCRs[m][i_Last]) == 1)   //两个连通域：水平+垂直距离<=1
					{
						for (int j = 0; j < vCRs[m][i_Last].Region.size(); ++j)
						{
							vCRs[m][idx].Region.emplace_back(vCRs[m][i_Last].Region[j]);   //将第二个连通域的所有出波点移动到第一个连通域
						}
						vCRs[m][i_Last].Region.clear();
					}

					if (vCRs[m][i_Last].Step1 > vCRs[m][idx].Step2 + 2)   //遍历的idx往后的i_Last的最小步进 > 当前处理的第idx个连通域最大步进 + 2 —— 跳出第idx个连通域的循环，开始处理的第idx+1个连通域
					{
						break;
					}
				}
				++idx;
			}
			RemoveInvalidCR(vCRs[m]);
		}

		CombineDE(vCRs[CH_D], CH_D);   //合并连通域（多条件判断）
		CombineDE(vCRs[CH_E], CH_E);   //合并连通域（多条件判断）



		// 初步融合相邻的连通域
		for (int m = 0; m < CH_e; ++m)   //A、a、B、b、C、c、D、E
		{
			if (m == CH_d)
			{
				continue;
			}
			int idx = 0;
			int nCount = vCRs[m].size() - 1;
			while (idx < nCount)
			{
				for (int i_Last = idx + 1; i_Last <= nCount; ++i_Last)
				{
					if (GetDistance(vCRs[m][idx], vCRs[m][i_Last]) == 1)   //水平+垂直距离<=1
					{
						Combine(vCRs[m][idx], vCRs[m][i_Last]);
						vCRs[m][i_Last].Region.clear();
					}

					if (vCRs[m][i_Last].Step1 > vCRs[m][idx].Step2 + 2)
					{
						break;
					}
				}
				++idx;
			}
		}
		// 分析D、E通道连通域是否为∧或∨形状，是则从中间分离成两个连通域
		for (int m = CH_D; m <= CH_E; ++m)   //D、E
		{
			for (int i = 0; i < vCRs[m].size(); ++i)
			{
				CR cr1, cr2;
				if (IsAngleCR(vCRs[m][i], datas, cr1, cr2))   //函数IsAngleCR()判断连通域是否为∧或∨形状
				{
					vCRs[m][i] = cr1;
					vCRs[m].insert(vCRs[m].begin() + i, cr2);
				}
			}

			//for (int i = 0; i < vCRs[m].size(); ++i)
			//{
			//	CR cr1, cr2; 
			//	if (IsDoubleCR(vCRs[m][i], datas, m, cr1, cr2))
			//	{
			//		vCRs[m][i] = cr1;
			//		vCRs[m].insert(vCRs[m].begin() + i, cr2);
			//	}
			//}

			for (int i = 0; i < vCRs[m].size(); ++i)   //按顺序重新赋Index索引值
			{
				vCRs[m][i].Index = i;
			}
		}


		for (int i = 0; i < CH_D; ++i)   //A、a、B、b、C、c 合并去重
		{
			RemoveInvalidCR(vCRs[i]);   //去除无效连通域（只有一个或没有出波点的连通域）
			int iSize = vCRs[i].size();
			do
			{
				CombineABC(vCRs[i], i);   //合并条件
				RemoveInvalidCR(vCRs[i]);
				if (vCRs[i].size() == iSize)
				{
					break;
				}
				else
				{
					iSize = vCRs[i].size();
				}
			} while (true);
		}

#pragma endregion  

#pragma region FG通道连通域处理

		for (int m = CH_F; m <= CH_G; ++m)
		{
			int iCount = vCRs[m].size();
			for (int i = iCount - 1; i >= 0; --i)
			{
				if (vCRs[m][i].Region.size() < 2)
				{
					vCRs[m].erase(vCRs[m].begin() + i);
				}
			}
		}
		// 合并F、G通道连通域
		CombineFG(vCRs[CH_F]);
		CombineFG(vCRs[CH_G]);

#pragma endregion

#pragma region 清除无效的连通域

		// 删除A~E通道无出波点的连通域
		for (int m = 0; m < CH_F; ++m)
		{
			RemoveInvalidCR(vCRs[m]);
		}

		// 清除FG通道出波点少、且附近无其他出波点情况
		for (int m = CH_F; m <= CH_G; ++m)
		{
			int iCount = vCRs[m].size();
			for (int i = iCount - 1; i >= 0; --i)
			{
				if (vCRs[m][i].Region.size() < 2)
				{
					bool isDirty = false;
					if (i == iCount - 1 && i - 1 >= 0)
					{
						isDirty = vCRs[m][i].Step1 - vCRs[m][i - 1].Step2 >= 10;
					}
					else if (i == 0 && i + 1 < iCount)
					{
						isDirty = vCRs[m][i + 1].Step1 - vCRs[m][i].Step2 >= 10;
					}
					else if (i > 0 && i < iCount - 1)
					{
						isDirty = (vCRs[m][i].Step1 - vCRs[m][i - 1].Step2 >= 10) && (vCRs[m][i + 1].Step1 - vCRs[m][i].Step2 >= 10);
					}

					if (isDirty)
					{
						vCRs[m].erase(vCRs[m].begin() + i);
					}
				}
			}
		}
#pragma endregion

#pragma region 存储米块索引，方便连通域定位
		std::map<int, int> blockIndex;
		for (int i = 0; i < datas.size(); ++i)
		{
			blockIndex.insert(std::make_pair(datas[i].Index, i));
		}
#pragma endregion

#pragma region 连通域参数计算和内部排序

		for (int m = 0; m < 16; ++m)
		{
			for (int i = vCRs[m].size() - 1; i >= 0; --i)
			{
				if (vCRs[m][i].Region.size() == 0)
				{
					vCRs[m].erase(vCRs[m].begin() + i);
					continue;
				}
				int nsize = vCRs[m][i].Region.size();
				vCRs[m][i].Block = vCRs[m][i].Region[0].block;
				vCRs[m][i].Step = vCRs[m][i].Region[0].step - datas[blockIndex[vCRs[m][i].Block]].IndexL2;
				vCRs[m][i].Step1 = vCRs[m][i].Region[0].step;
				vCRs[m][i].Row1 = vCRs[m][i].Region[0].row;
				vCRs[m][i].Step2 = vCRs[m][i].Region[nsize - 1].step;
				vCRs[m][i].Row2 = vCRs[m][i].Region[nsize - 1].row;
				vCRs[m][i].Index = i;

				for (int j = 0; j < nsize; ++j)
				{
					if (vCRs[m][i].Region[j].step > vCRs[m][i].Step2)
					{
						vCRs[m][i].Step2 = vCRs[m][i].Region[j].step;
					}

					if (vCRs[m][i].Region[j].step < vCRs[m][i].Step1)
					{
						vCRs[m][i].Step1 = vCRs[m][i].Region[j].step;
					}

					if (vCRs[m][i].Region[j].row < vCRs[m][i].Row1)
					{
						vCRs[m][i].Row1 = vCRs[m][i].Region[j].row;
					}

					if (vCRs[m][i].Region[j].row > vCRs[m][i].Row2)
					{
						vCRs[m][i].Row2 = vCRs[m][i].Region[j].row;
					}
				}
			}
		}

		//排序并重新标记
		for (int i = 0; i < 16; ++i)
		{
			std::sort(vCRs[i].begin(), vCRs[i].end());
		}

		for (int i = 0; i < 16; ++i)
		{
			int index = 0;
			for (int j = 0; j < vCRs[i].size(); ++j)
			{
				vCRs[i][j].Index = index++;
				vCRs[i][j].IsUsed = 0;
			}
		}
#pragma endregion

#pragma region 删除连通域中重复的出波点

		for (int i = CH_A1; i < CH_F; ++i)
		{
			for (int j = 0; j < vCRs[i].size(); ++j)
			{
				CR& cr = vCRs[i][j];
				RemoveRepeatedPoints(cr);   //函数RemoveRepeatedPoints()删除连通域中重复的出波点
				//连通域参数计算 IsReversed
				cr.IsReversed = 0;
				int firstRow = 0, lastRow = 0;
				GetCRRowInfo(cr, firstRow, lastRow);
				bool isReversed = false;
				if (cr.Channel == CH_A1 || cr.Channel == CH_A2 || cr.Channel == CH_B1 || cr.Channel == CH_B2 || cr.Channel == CH_D /* || cr.Channel == CH_C*/)
				{
					if (firstRow <= lastRow)
					{
						cr.IsReversed = 1;
					}
				}
				else if (cr.Channel == CH_a1 || cr.Channel == CH_a2 || cr.Channel == CH_b1 || cr.Channel == CH_b2 || cr.Channel == CH_E /* || cr.Channel == CH_c*/)
				{
					if (firstRow >= lastRow)
					{
						cr.IsReversed = 1;
					}
				}
				else if (cr.Channel == CH_C)
				{
					if (firstRow < lastRow - 1)
					{
						cr.IsReversed = 1;
					}
				}
				else if (cr.Channel == CH_C)
				{
					if (firstRow > lastRow + 1)
					{
						cr.IsReversed = 1;
					}
				}
			}
		}
#pragma endregion

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

uint8_t GetCR(uint8_t channel, int step1, int step2, VBDB& vBDatas, VCR& crToFind, VINT& vCrFound, int32_t iExcept, int iMinimunSize, bool bNeedUnJudged)
{
	for (int i = 0; i < crToFind.size(); ++i)
	{
		if (bNeedUnJudged && crToFind[i].IsUsed == 1)
		{
			continue;
		}
		if (crToFind[i].Step1 > step2)
		{
			break;
		}
		if (crToFind[i].Step2 < step1 || crToFind[i].Region.size() < iMinimunSize)
		{
			continue;
		}
		if (iExcept != i)
		{
			vCrFound.emplace_back(i);
		}
	}
	return vCrFound.size() > 0 ? 1 : 0;
}

uint8_t GetCR(uint8_t channel, int step1, uint8_t row1, int step2, uint8_t row2, VBDB& vBDatas, VCR& crToFind, VINT& vCrFound, int32_t iExcept, int iMinimunSize, bool bNeedUnJudged)
{
	for (int i = 0; i < crToFind.size(); ++i)
	{
		Connected_Region& cr = crToFind[i];
		if (cr.Step1 > step2)
		{
			break;
		}
		if (bNeedUnJudged && cr.IsUsed == 1)
		{
			continue;
		}
		int mi = cr.Row1 <= cr.Row2 ? cr.Row1 : cr.Row2;
		int ma = cr.Row1 >= cr.Row2 ? cr.Row1 : cr.Row2;
		if (cr.Step2 < step1 || row1 > ma || row2 < mi)
		{
			continue;
		}
		if (cr.Step2 < step1 || cr.Region.size() < iMinimunSize)
		{
			continue;
		}
		if (iExcept != i)
		{
			vCrFound.emplace_back(i);
		}
	}
	return vCrFound.size() > 0 ? 1 : 0;
}

uint8_t GetCR(uint8_t channel, int step1, uint8_t row1, int step2, uint8_t row2, VBDB& vBDatas, VCR& crToFind, VINT& vCrFound, VINT vExcept, int iMinimunSize/* = 1*/, bool bNeedUnJudged)
{
	std::map<int, int> mapExist;
	for (int i = 0; i < vExcept.size(); ++i)
	{
		mapExist[vExcept[i]] = 1;
	}
	for (int i = 0; i < crToFind.size(); ++i)
	{
		Connected_Region& cr = crToFind[i];
		if (cr.Step1 > step2)
		{
			break;
		}
		if (bNeedUnJudged && cr.IsUsed == 1)
		{
			continue;
		}
		int mi = cr.Row1 <= cr.Row2 ? cr.Row1 : cr.Row2;
		int ma = cr.Row1 >= cr.Row2 ? cr.Row1 : cr.Row2;
		if (cr.Step2 < step1 || row1 > ma || row2 < mi)
		{
			continue;
		}
		if (cr.Step2 < step1 || cr.Region.size() < iMinimunSize)
		{
			continue;
		}
		if (mapExist.find(i) == mapExist.end())
		{
			vCrFound.emplace_back(i);
		}
	}
	return vCrFound.size() > 0 ? 1 : 0;
}

void	GetMaxCR(uint8_t channel1, uint8_t channel2, int step1, int step2, VBDB& vBDatas, VCR* vCRs, CR& cr)
{
	int maxSize = 0;
	for (int i = channel1; i <= channel2; ++i)
	{

	}
}

//0~4.5: 0.6大格
//4.5~7: 1大格
//6~8.5：2大格
//void FillCRSection(CR_INFO_A& info)
//{
//	if (info.MaxH < 225)
//	{
//		info.iSection = 0;
//		return;
//	}
//	else if (info.MinH < 225 && info.MaxH > 350)
//	{
//		info.iSection = 1;
//		return;
//	}
//	else if (info.MinH < 225 && info.MaxH > 225)
//	{
//		info.d1_4 = 225 - info.MinH;
//		info.d4_7 = info.MaxH - 225;
//	}
//	else if (info.MinH < 225)
//	{
//		info.d1_4 = info.MaxH - info.MinH;
//	}
//	else if (info.MinH < 300 && info.MaxH > 350)
//	{
//		info.d4_7 = 350 - info.MinH;
//		info.d6_8 = info.MaxH - 300;
//	}
//	else if (info.MinH < 300)
//	{
//		info.d4_7 = info.MaxH - info.MinH;
//	}
//	else if (info.MinH >= 350)
//	{
//		info.d4_7 = 0;
//		info.d6_8 = info.MaxH - info.MinH;
//	}
//	else if (info.MinH >= 300 && info.MaxH > 350)
//	{
//		info.d4_7 = 350 - info.MinH;
//		info.d6_8 = info.MaxH - info.MinH;
//	}
//	else if (info.MinH >= 300 && info.MaxH < 350)
//	{
//		info.d4_7 = info.MaxH - info.MinH;
//	}
//
//
//	if (info.d1_4 >= info.d4_7 && info.d1_4 >= info.d6_8)
//	{
//		info.iSection = 0;
//	}
//	else if (info.d4_7 >= info.d1_4 && info.d4_7 >= info.d6_8)
//	{
//		info.iSection = 1;
//	}
//	else if (info.d6_8 >= info.d1_4 && info.d6_8 >= info.d4_7)
//	{
//		info.iSection = 2;
//	}
//}
//0~4.5: 0.6大格
//4.5~7.5: 1大格
//6~8.5：2大格
void	FillCRSection(CR_INFO_A& info)
{
	if (info.MinH >= 300)
	{
		info.d6_8 = info.MaxH - info.MinH;
		info.iSection = 2;
	}
	else if (info.MinH >= 225)
	{
		info.d4_7 = info.MaxH - info.MinH;
		info.iSection = 1;
	}
	else
	{
		info.d1_4 = info.MaxH - info.MinH;
		info.iSection = 0;
	}

	/*
	if (info.MaxH < 225)
	{
	info.iSection = 0;
	return;
	}
	else if (info.MinH < 225 && info.MaxH > 350)
	{
	info.iSection = 1;
	return;
	}
	else if (info.MinH < 225 && info.MaxH > 225)
	{
	info.d1_4 = 225 - info.MinH;
	info.d4_7 = info.MaxH - 225;
	}
	else if (info.MinH < 225)
	{
	info.d1_4 = info.MaxH - info.MinH;
	}
	else if (info.MinH < 300 && info.MaxH > 375)
	{
	info.d4_7 = 375 - info.MinH;
	info.d6_8 = info.MaxH - 300;
	}
	else if (info.MinH < 300)
	{
	info.d4_7 = info.MaxH - info.MinH;
	}
	else if (info.MinH >= 355)
	{
	info.d4_7 = 0;
	info.d6_8 = info.MaxH - info.MinH;
	}
	else if (info.MinH >= 300 && info.MaxH > 375)
	{
	info.d4_7 = 375 - info.MinH;
	info.d6_8 = info.MaxH - info.MinH;
	}
	else if (info.MinH >= 300 && info.MaxH < 375)
	{
	info.d4_7 = info.MaxH - info.MinH;
	}


	if (info.d1_4 >= info.d4_7 && info.d1_4 >= info.d6_8)
	{
	info.iSection = 0;
	}
	else if (info.d4_7 >= info.d1_4 && info.d4_7 >= info.d6_8)
	{
	info.iSection = 1;
	}
	else if (info.d6_8 >= info.d1_4 && info.d6_8 >= info.d4_7)
	{
	info.iSection = 2;
	}
	*/
}

void    ParseCRAFrames(CR& cr, BlockData_A& DataA, VBDB& DataB, BLOCK& blockHead, uint8_t& iChA, uint8_t isUnique)
{
	double currentGain = 0.5 * blockHead.gain[iChA];
	if (cr.IsContainA)
	{
		for (int i = 0; i < cr.vASteps.size(); ++i)
		{
			for (int j = 0; j < cr.vASteps[i].Frames.size(); ++j)
			{
				if (cr.Channel == CH_C || cr.Channel == CH_c)
				{
					if (cr.vASteps[i].Frames[j].Horizon >= 330)
					{
						continue;
					}
				}
				if (cr.vASteps[i].Frames[j].F[iChA] > 0)//此处的0改为出波抑制值
				{
					A_Frame& frame = cr.vASteps[i].Frames[j];
					cr.Info.MinH = cr.Info.MinH < frame.Horizon ? cr.Info.MinH : frame.Horizon;
					cr.Info.MaxH = cr.Info.MaxH > frame.Horizon ? cr.Info.MaxH : frame.Horizon;
					cr.Info.MaxV = cr.Info.MaxV > frame.F[iChA] ? cr.Info.MaxV : frame.F[iChA];
				}
			}
		}
		cr.Info.Shift = cr.Info.Shift2 = (cr.Info.MaxH - cr.Info.MinH);
		cr.Info.MaxV2 = cr.Info.MaxV;
		uint16_t data = cr.Info.MaxV;
		//info.MaxV = pow(10, 0.05 * (0.5 * gain - StandardGain[iChA])) * data;
		if (currentGain - g_StandardGain[iChA] >= 0 && currentGain - g_StandardGain[iChA] <= 5)
		{
			cr.Info.MaxV = (0.75 * g_filehead.deviceP2.Restr[iChA] + data) * pow(10, 0.05 * (currentGain - g_StandardGain[iChA])) - 0.75 * g_filehead.deviceP2.Restr[iChA];
		}
		if (cr.Info.MaxV < 0)
		{
			cr.Info.MaxV = 0;
		}

		//if (cr.Channel < CH_D)
		//{
		//	cr.Info.Shift = (cr.Info.MaxH - cr.Info.MinH) + (0.5 * blockHead.gain[iChA] - StandardGain[iChA]) * 3.333;
		//	if (cr.Info.Shift < 0) cr.Info.Shift = 0;
		//}
		//else if (cr.Channel < CH_F)
		//{
		//	cr.Info.Shift = (cr.Info.MaxH - cr.Info.MinH) + (0.5 * blockHead.gain[iChA] - StandardGain[iChA]) * 1.667;
		//	if (cr.Info.Shift < 0) cr.Info.Shift = 0;
		//}

		cr.MinH = cr.Info.MinH;
		cr.MaxH = cr.Info.MaxH;
		if (cr.Info.Shift < cr.H2 - cr.H1)
		{
			cr.Info.MinH = min(cr.H1, cr.Info.MinH);
			cr.Info.MaxH = max(cr.H2, cr.Info.MaxH);
			cr.Info.Shift = cr.Info.MaxH - cr.Info.MinH;
			cr.MinH = cr.Info.MinH;
			cr.MaxH = cr.Info.MaxH;
		}

		if (cr.vASteps.size() == 0)
			return;
	}
	else
	{
		cr.Info.MinH = cr.H1;
		cr.Info.MaxH = cr.H2;
		cr.Info.Shift = (cr.Info.MaxH - cr.Info.MinH);
	}

	//只对40db调整到38db，而不会从36db调整到38db
	if (cr.Channel < CH_D && currentGain - g_StandardGain[iChA] >= 0 && currentGain - g_StandardGain[iChA] <= 5 && isUnique)
	{
		int d = (0.5 * blockHead.gain[iChA] - g_StandardGain[iChA]) * 3.333;
		cr.Info.MinH = max(0, cr.Info.MinH - d / 2);
		cr.Info.MaxH = min(cr.Info.MaxH + d / 2, 500);
		cr.Info.Shift = cr.Info.MaxH - cr.Info.MinH;
	}

	if (cr.Channel < CH_C)
	{
		FillCRSection(cr.Info);
	}

	if (cr.Info.Shift < 0)
		cr.Info.Shift = 0;
}


bool	GetCRInfo(CR& cr, BlockData_A& DataA, VBDB& DataB, uint8_t isUnique)
{
	if (cr.Region.size() == 0 || cr.Channel >= CH_F && cr.IsLose == 1)
	{
		return false;
	}

	memset(&cr.Info, 0, sizeof(CR_INFO_A));
	cr.Info.MinH = 512;
	uint8_t iChA = GetAChannelByBChannel(cr.Channel);
	double angle = 0.1 * g_filehead.deviceP2.Angle[iChA].Refrac;
	BLOCK blockHead = DataB[cr.Block - g_iBeginBlock].BlockHead;
	double offset = g_filehead.deviceP2.Place[iChA] + blockHead.probOff[iChA];
	bool bFindFrame = GetCRFrames(cr, cr.vASteps, DataA, DataB, angle, offset, (double)g_filehead.step, isUnique);
	if (cr.IsContainA && !bFindFrame)
	{
		cr.IsContainA = 0;
		//return false;
	}
	if (isUnique)
	{
		cr.Info.MinH = min(cr.H1, cr.Info.MinH);
		cr.Info.MaxH = max(cr.H2, cr.Info.MaxH);
	}
	ParseCRAFrames(cr, DataA, DataB, blockHead, iChA, isUnique);
	return true;
}

void	GetCRRowInfo(CR& cr, int& firstRow, int& lastRow)
{
	/*
	bool bFindFirst = false, bFindLast = false;
	int n = cr.Region.size();
	if (cr.Channel == CH_D)
	{
		firstRow = 0; lastRow = 100;
		for (int i = 0; i < n; ++i)
		{
			if (cr.Region[i].step == cr.Step1)
			{
				if (firstRow < cr.Region[i].row)
				{
					firstRow = cr.Region[i].row;
				}
			}
			if (cr.Region[i].step == cr.Step2)
			{
				if (lastRow > cr.Region[i].row)
				{
					lastRow = cr.Region[i].row;
				}
			}
		}
	}
	else if (cr.Channel == CH_E)
	{
		firstRow = 100; lastRow = 0;
		for (int i = 0; i < n; ++i)
		{
			if (cr.Region[i].step == cr.Step1)
			{
				if (cr.Region[i].row < firstRow)
				{
					firstRow = cr.Region[i].row;
				}
			}
			if (cr.Region[i].step == cr.Step2)
			{
				if (cr.Region[i].row > lastRow)
				{
					lastRow = cr.Region[i].row;
				}
			}
		}
	}
	*/
	bool bFindFirst = false, bFindLast = false;
	int n = cr.Region.size();
	//if (cr.Channel == CH_D || cr.Channel == CH_A1 || cr.Channel == CH_A2 || cr.Channel == CH_B1 || cr.Channel == CH_B2 || cr.Channel == CH_C)
	//{
	firstRow = 0; lastRow = 100;
	for (int i = 0; i < n; ++i)
	{
		if (cr.Region[i].step == cr.Step1)
		{
			if (firstRow < cr.Region[i].row)
			{
				firstRow = cr.Region[i].row;
			}
		}
		if (cr.Region[i].step == cr.Step2)
		{
			if (lastRow > cr.Region[i].row)
			{
				lastRow = cr.Region[i].row;
			}
		}
	}
	/*}
	else if (cr.Channel == CH_E || cr.Channel == CH_a1 || cr.Channel == CH_a2 || cr.Channel == CH_b1 || cr.Channel == CH_b2 || cr.Channel == CH_c)
	{
		firstRow = 100; lastRow = 0;
		for (int i = 0; i < n; ++i)
		{
			if (cr.Region[i].step == cr.Step1)
			{
				if (cr.Region[i].row < firstRow)
				{
					firstRow = cr.Region[i].row;
				}
			}
			if (cr.Region[i].step == cr.Step2)
			{
				if (cr.Region[i].row > lastRow)
				{
					lastRow = cr.Region[i].row;
				}
			}
		}
	}*/
}

void	GetCRRowInfo1(CR& cr, std::map<uint32_t, _StepRegion>& vStep)
{
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (vStep.find(cr.Region[i].step) != vStep.end())
		{
			_StepRegion& rr = vStep[cr.Region[i].step];
			if (rr.row1 > cr.Region[i].row)
			{
				rr.row1 = cr.Region[i].row;
			}
			if (rr.row2 < cr.Region[i].row)
			{
				rr.row2 = cr.Region[i].row;
			}
		}
		else
		{
			_StepRegion sr;
			sr.step = cr.Region[i].step;
			sr.row1 = cr.Region[i].row;
			sr.row2 = cr.Region[i].row;
			vStep[sr.step] = sr;
		}
	}
}

void	GetCRRowInfo2(CR& cr, int& firstRow, int& lastRow, uint8_t channel)
{
	std::map<uint32_t, _StepRegion> vStep;
	GetCRRowInfo1(cr, vStep);

	if (channel == CH_D || channel == CH_C || channel == CH_A1 || channel == CH_A2 || channel == CH_B1 || channel == CH_B2)
	{
		firstRow = vStep[cr.Step1].row2;
		lastRow = vStep[cr.Step2].row1;
	}
	else
	{
		firstRow = vStep[cr.Step1].row1;
		lastRow = vStep[cr.Step2].row2;
	}
}

void	GetCRRowInfo3(CR& cr, int& firstRow, int& lastRow, uint8_t channel, int& firstRow2, int& lastRow2)
{
	std::map<uint32_t, _StepRegion> vStep;
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (vStep.find(cr.Region[i].step) != vStep.end())
		{
			_StepRegion& rr = vStep[cr.Region[i].step];
			if (rr.row1 > cr.Region[i].row)
			{
				rr.row1 = cr.Region[i].row;
			}
			if (rr.row2 < cr.Region[i].row)
			{
				rr.row2 = cr.Region[i].row;
			}
		}
		else
		{
			_StepRegion sr;
			sr.step = cr.Region[i].step;
			sr.row1 = cr.Region[i].row;
			sr.row2 = cr.Region[i].row;
			vStep[sr.step] = sr;
		}
	}
	if (channel == CH_D || channel == CH_C || channel == CH_A1 || channel == CH_A2 || channel == CH_B1 || channel == CH_B2)
	{
		firstRow = vStep[cr.Step1].row2;
		lastRow = vStep[cr.Step2].row1;
	}
	else
	{
		firstRow = vStep[cr.Step1].row1;
		lastRow = vStep[cr.Step2].row2;
	}

	firstRow2 = vStep[cr.Step1].row1;
	lastRow2 = vStep[cr.Step2].row2;
}


//针对螺孔FG正常出波与水平裂纹出波联通
void	GetCRRowInfo4(CR& cr, std::map<uint8_t, uint32_t>& vRowPointCount)
{
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (vRowPointCount.find(cr.Region[i].row) != vRowPointCount.end())
		{
			vRowPointCount[cr.Region[i].row] = vRowPointCount[cr.Region[i].row] + 1;
		}
		else
		{
			vRowPointCount[cr.Region[i].row] = 1;
		}
	}
}


void	GetCoreCR(CR& cr, std::map<uint8_t, int>& rowDistribute, CR& crOut, VBDB& blocks, uint8_t jawRow)
{
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (rowDistribute.find(cr.Region[i].row) != rowDistribute.end())
		{
			rowDistribute[cr.Region[i].row] = rowDistribute[cr.Region[i].row] + 1;
		}
		else
		{
			rowDistribute[cr.Region[i].row] = 1;
		}
	}

	uint8_t maxRow = rowDistribute.begin()->first;
	int maxValue = rowDistribute.begin()->second;
	for (auto itr = ++rowDistribute.begin(); itr != rowDistribute.end(); ++itr)
	{
		if (itr->second > maxValue && itr->first != 2 * jawRow)
		{
			maxRow = itr->first;
			maxValue = itr->second;
		}
	}

	uint8_t iFirstRow = maxRow, iLastRow = maxRow;
	auto itr = rowDistribute.find(maxRow);
	int lastValue = itr->second;
	for (auto itr2 = itr; itr2 != rowDistribute.begin(); --itr2)
	{
		if (itr->first - iFirstRow > 1 || itr2->second < 0.5 * lastValue && maxValue >= 5)
		{
			break;
		}
		iFirstRow = itr2->first;
		lastValue = itr2->second;
	}

	itr = rowDistribute.find(maxRow);
	lastValue = itr->second;
	for (auto itr2 = itr; itr2 != rowDistribute.end(); ++itr2)
	{
		if (itr2->first - iLastRow > 1 || itr2->second < 0.5 * lastValue && maxValue >= 5)
		{
			break;
		}
		iLastRow = itr2->first;
		lastValue = itr2->second;
	}

	for (int i = 0; i < cr.Region.size(); ++i)
	{
		if (cr.Region[i].row >= iFirstRow && cr.Region[i].row <= iLastRow)
		{
			crOut.Region.emplace_back(cr.Region[i]);
		}
	}

	crOut.Channel = cr.Channel;
	FillCR(crOut);
}

uint8_t GetCRsStep(VCR& vcr, VINT& vIndexes, uint32_t& step1, uint32_t& step2)
{
	for (int i = 0; i < vIndexes.size(); ++i)
	{
		if (vcr[vIndexes[i]].Step1 < step1)
		{
			step1 = vcr[vIndexes[i]].Step1;
		}
		if (vcr[vIndexes[i]].Step2 > step2)
		{
			step2 = vcr[vIndexes[i]].Step2;
		}
	}
	return 0;
}

int16_t GetFRow(B_Step& step)
{
	if (step.vRowDatas.size() == 0)
	{
		return -1;
	}
	int n = step.vRowDatas.size();
	for (int i = n - 1; i >= 0; --i)
	{
		if (step.vRowDatas[i].Row < 40)
		{
			return -1;
		}
		if (step.vRowDatas[i].Point.Draw1 & BIT14)
		{
			return step.vRowDatas[i].Row;
		}
	}
	return -1;
}

int16_t GetGRow(B_Step& step)
{
	if (step.vRowDatas.size() == 0)
	{
		return -1;
	}
	int n = step.vRowDatas.size();
	for (int i = n - 1; i >= 0; --i)
	{
		if (step.vRowDatas[i].Row < 40)
		{
			return -1;
		}
		if (step.vRowDatas[i].Point.Draw1 & BIT15)
		{
			return step.vRowDatas[i].Row;
		}
	}
	return -1;
}

int16_t GetFRow2(B_Step& step, int16_t desiredRow)
{
	if (step.vRowDatas.size() == 0)
	{
		return -1;
	}
	int n = step.vRowDatas.size();
	for (int i = n - 1; i >= 0; --i)
	{
		if (step.vRowDatas[i].Row < desiredRow - 10)
		{
			return -1;
		}
		if (step.vRowDatas[i].Point.Draw1 & BIT14)
		{
			if (desiredRow - step.vRowDatas[i].Row <= 2 && desiredRow - step.vRowDatas[i].Row >= -2)
			{
				return step.vRowDatas[i].Row;
			}
		}
	}
	return -1;
}

int16_t GetGRow2(B_Step& step, int16_t desiredRow)
{
	if (step.vRowDatas.size() == 0)
	{
		return -1;
	}
	int n = step.vRowDatas.size();
	for (int i = n - 1; i >= 0; --i)
	{
		if (step.vRowDatas[i].Row < desiredRow - 10)
		{
			return -1;
		}
		if (step.vRowDatas[i].Point.Draw1 & BIT15)
		{
			if (desiredRow - step.vRowDatas[i].Row <= 2 && desiredRow - step.vRowDatas[i].Row >= -2)
			{
				return step.vRowDatas[i].Row;
			}
		}
	}
	return -1;
}

uint8_t GetStepPeaks(A_Step& step, uint8_t iChA)
{
	uint8_t count = 0;
	bool bFind = false;
	int lastH = 0;
	for (int i = 0; i < step.Frames.size(); ++i)
	{
		if (!bFind && step.Frames[i].F[iChA] > 0 && (lastH == 0 || step.Frames[i].Horizon - lastH > 1))
		{
			lastH = step.Frames[i].Horizon;
			bFind = true;
			++count;
		}
		else if (bFind && step.Frames[i].F[iChA] > 0 && step.Frames[i].Horizon - lastH == 1)
		{
			++lastH;
		}
		else if (bFind && step.Frames[i].F[iChA] > 0 && step.Frames[i].Horizon - lastH > 1)
		{
			++count;
			lastH = step.Frames[i].Horizon;
			bFind = true;
		}
		else if (bFind && step.Frames[i].F[iChA] == 0)
		{
			bFind = false;
		}
	}
	return count;
}

int	GetBeginFrameIndexByStep(BlockData_A& DataA, int32_t& step)
{
	int tempStep = step;
	int iBeginFrameIndex = -1;
	while (tempStep >= 0)
	{
		iBeginFrameIndex = FindStepInAData(tempStep, DataA);
		if (iBeginFrameIndex >= 0)
		{
			break;
		}
		--tempStep;
	}
	return iBeginFrameIndex;
}

int GetEndFrameIndexByStep(BlockData_A& DataA, int32_t& step)
{
	int tempStep = step;
	int iBeginFrameIndex = -1;
	while (tempStep >= 0)
	{
		iBeginFrameIndex = FindStepInAData(tempStep, DataA);
		if (iBeginFrameIndex >= 0)
		{
			break;
		}
		--tempStep;
	}
	return iBeginFrameIndex;
}


bool GetCRASteps(uint8_t channel, Connected_Region& cr, int32_t& step1, int32_t& step2, VBDB& vBBlocks, double angle, int offset, double stepDistance)
{
	int nChA = GetAChannelByBChannel(channel);
	if (cr.Region.size() == 0)
	{
		return false;
	}
	int step = cr.Region[0].step;

	int32_t iBeginStep = 0x7FFFFFFF, iEndStep = 0 - iBeginStep;
	double rad = angle * AngleToRad;
	for (int i = 0; i < cr.Region.size(); ++i)
	{
		WaveData& wd = cr.Region[i];
		double dStep = (1.0 * offset + 3.0 * wd.row * tan(rad)) / stepDistance;
		int iRealStep = wd.step - dStep;
		if (iRealStep < iBeginStep)
		{
			iBeginStep = iRealStep;
		}
		if (iRealStep > iEndStep)
		{
			iEndStep = iRealStep;
		}
	}
	step1 = iBeginStep;
	step2 = iEndStep;
	return true;
}

bool GetCRASteps(uint8_t channel, WaveData& wd, int32_t& step1, int32_t& step2, VBDB& vBBlocks, double angle, int offset, double stepDistance)
{
	int nChA = GetAChannelByBChannel(channel);
	int step = wd.step;

	int32_t iBeginStep = 0x7FFFFFFF, iEndStep = 0 - iBeginStep;
	double rad = angle * AngleToRad;
	double dStep = (1.0 * offset + 3.0 * wd.row * tan(rad)) / stepDistance;
	int iRealStep = wd.step - dStep;
	if (iRealStep < iBeginStep)
	{
		iBeginStep = iRealStep;
	}
	if (iRealStep > iEndStep)
	{
		iEndStep = iRealStep;
	}
	step1 = iBeginStep;
	step2 = iEndStep;
	return true;
}


int	FindStepInAData(int32_t step, BlockData_A& vAFrames)
{
	int iStepIndex = -1;
	int low = 0;
	int mid = 0;
	int high = vAFrames.vAStepDatas.size() - 1;
	while (low <= high)
	{
		mid = (low + high) / 2;
		if (vAFrames.vAStepDatas[mid].Step < step)
			low = mid + 1;
		else if (vAFrames.vAStepDatas[mid].Step > step)
			high = mid - 1;
		else
		{
			return mid;
		}
	}
	return -1;
}

//一般用来获取疑似斜裂纹处FG通道的出波
bool	GetAFrames(BlockData_A& DataA, uint32_t& step1, uint32_t& step2, uint8_t ichA, uint16_t h1, uint16_t h2, VASTEPS &vFrames)
{
	int step11 = step1, step22 = step2;
	int iBeginStepIndex = GetBeginFrameIndexByStep(DataA, step11);
	int iEndStepIndex = GetEndFrameIndexByStep(DataA, step22);
	if (iBeginStepIndex < 0 && iEndStepIndex < 0)
	{
		return false;
	}
	else if (iBeginStepIndex < 0)
	{
		iBeginStepIndex = 0;
	}
	else  if (iEndStepIndex < 0)
	{
		iEndStepIndex = DataA.vAStepDatas.size() - 1;
	}

	for (int i = iBeginStepIndex; i <= iEndStepIndex; ++i)
	{
		A_Step astep = DataA.vAStepDatas[i];
		astep.Frames.clear();
		for (int j = 0; j < DataA.vAStepDatas[i].Frames.size(); ++j)
		{
			A_Frame& frame = DataA.vAStepDatas[i].Frames[j];
			if (frame.Horizon >= h1 && frame.Horizon <= h2 && frame.F[ichA] > 0)
			{
				astep.Frames.emplace_back(frame);
			}
		}
		if (astep.Frames.size() > 0)
		{
			vFrames.emplace_back(astep);
		}
	}
	return vFrames.size() > 0;
}

bool	GetCRFrames(Connected_Region& cr, std::vector<A_Step>& vSteps, BlockData_A& DataA, VBDB& DataB, double angle, int offset, double stepDistance, uint8_t isUnique)
{
	cr.IsContainA = 1;
	vSteps.clear();
	int nChA = GetAChannelByBChannel(cr.Channel);
	int step = cr.Region[0].step;
	int32_t iBeginStep = 0x7FFFFFFF, iEndStep = -1;
	int iBeginStepIndex = 0x7FFFFFFF, iEndStepIndex = -1;

	Connected_Region cr_temp = cr;
	if (cr.Channel == CH_A2 || cr.Channel == CH_B2 || cr.Channel == CH_a2 || cr.Channel == CH_b2)
	{
		for (int i = 0; i < cr_temp.Region.size(); ++i)
		{
			cr_temp.Region[i].row = 26 - cr_temp.Region[i].row;
		}
		FillCR(cr_temp);
	}

	double rad = angle * AngleToRad;
	bool bFind = false;

	//A中的步进数应小于B中的步进数
	for (int i = 0; i < cr_temp.Region.size(); ++i)
	{
		double dStep = (1.0 * offset + 3.0 * cr_temp.Region[i].row * tan(rad)) / stepDistance;
		int iRealStep = cr_temp.Region[i].step - dStep - g_channelOffset[nChA];
		if (iRealStep < iBeginStep)			iBeginStep = iRealStep;
		if (iRealStep > iEndStep)			iEndStep = iRealStep;
	}

	//二次波补偿
	if (cr_temp.Row2 > 13 && cr_temp.Channel < CH_C)
	{
		iEndStep += (cr_temp.Row2 - 13) * 0.5;
	}

	int iBS = iBeginStep, iES = iEndStep;
	iBeginStepIndex = GetBeginFrameIndexByStep(DataA, iBeginStep);
	iEndStepIndex = GetEndFrameIndexByStep(DataA, iEndStep);
	if (iBeginStepIndex < 0 && iEndStepIndex < 0)
	{
		cr.IsContainA = 0;
	}
	else if (iBeginStepIndex < 0)
	{
		iBeginStepIndex = 0;
	}
	else  if (iEndStepIndex < 0)
	{
		iEndStepIndex = DataA.vAStepDatas.size() - 1;
	}


	int railType = DataB[cr.Block - g_iBeginBlock].BlockHead.railType & 0x03;
	int railTypePrev = railType;
	if (cr.Block - g_iBeginBlock > 0)
	{
		railTypePrev = DataB[cr.Block - g_iBeginBlock - 1].BlockHead.railType & 0x03;
	}
	int iH1 = 0, iH2 = 0;
	iH1 = (353.0 / 58.0) * cr_temp.Row1 / cos(rad);
	iH2 = (353.0 / 58.0) * cr_temp.Row2 / cos(rad);
	if (railType == 3 || railTypePrev == 3)
	{
		iH1 = (353.0 / 58.0) * cr_temp.Row1 / cos(rad);
		iH2 = (353.0 / 58.0) * cr_temp.Row2 / cos(rad);

		iH1 = min(iH1, (300.0 / 58.0) * cr_temp.Row1 / cos(rad));
	}
	if (cr.Channel < CH_C)
	{
		if (cr.Row1 >= g_iJawRow[railType])
		{
			iH1 -= 1.8f * (cr.Row1 - g_iJawRow[railType]);
			iH2 -= 1.8f * (cr.Row2 - g_iJawRow[railType]);
		}
		else if (cr.Row2 >= g_iJawRow[railType])
		{
			iH2 -= 1.8f * (cr.Row2 - g_iJawRow[railType]);
		}

		cr.Info.MinH = iH1;
		cr.Info.MaxH = iH2;
	}

	int iH1Raw = iH1, iH2Raw = iH2;
	if (isUnique && cr.Channel < CH_F)
	{
		iH1 = cr.IsContainA ? max(iH1 - 50, 0) : max(iH1 - 25, 0);
		iH2 = cr.IsContainA ? min(500, iH2 + 50) : min(500, iH2 + 25);

		iBeginStepIndex -= 3;
		iEndStepIndex += 3;

		if (isUnique == 2)
		{
			iBeginStepIndex -= 10;
			iEndStepIndex += 10;
		}
	}
	if (iBeginStepIndex < 0)	iBeginStepIndex = 0;
	if (iEndStepIndex >= DataA.vAStepDatas.size())	iEndStepIndex = DataA.vAStepDatas.size() - 1;

	if (cr.IsContainA == 0)
	{
		cr.H1 = iH1;
		cr.H2 = iH2;
		return false;
	}


	int nTotalFrameCount = DataA.vAStepDatas.size() - 1;
	//首先找到A超幅值最大的那一帧，然后向两侧找
	int imaxValue = 0;
	int iIndex = 0, maxHorizon = 0;
	VINT vIndexs;
	for (int i = iBeginStepIndex; i <= iEndStepIndex; ++i)
	{
		if (GetStepPeaks(DataA.vAStepDatas[i], nChA) >= 5)
		{
			continue;
		}
		int nFramesCount = DataA.vAStepDatas[i].Frames.size();
		bool bFindFrame = false;
		int t_max = 0, t_horizon = 0;
		for (int j = 0; j < nFramesCount; ++j)
		{
			A_Frame &frame = DataA.vAStepDatas[i].Frames[j];
			if (frame.Horizon >= iH1Raw - 4 && frame.Horizon <= iH2Raw + 4 && frame.F[nChA] > 0)
			{
				/*if (cr.Channel < CH_C && frame.Horizon >= 425)
				{
					break;
				}*/
				if (frame.F[nChA] > t_max)
				{
					t_max = frame.F[nChA];
					t_horizon = frame.Horizon;
				}
				bFindFrame = true;
			}
			else if (bFindFrame && frame.F[nChA] == 0)
			{
				break;
			}
		}
		if (imaxValue < t_max)
		{
			imaxValue = t_max;
			iIndex = i;
			maxHorizon = t_horizon;
		}
		if (t_max > 0)
		{
			vIndexs.emplace_back(i);
		}
	}


	if (iEndStepIndex - iBeginStepIndex >= 10 && vIndexs.size() >= 10)
	{
		iIndex = vIndexs[vIndexs.size() / 2];
		int nFramesCount = DataA.vAStepDatas[iIndex].Frames.size();
		int t_max = 0;
		for (int j = 0; j < nFramesCount; ++j)
		{
			A_Frame &frame = DataA.vAStepDatas[iIndex].Frames[j];
			if (frame.Horizon >= iH1 - 4 && frame.Horizon <= iH2 + 4 && frame.F[nChA] > 0)
			{
				if (frame.F[nChA] > t_max)
				{
					t_max = frame.F[nChA];
					maxHorizon = frame.Horizon;
				}
			}
			else if (frame.Horizon >= iH2)
			{
				break;
			}
		}
	}

	cr.H1 = iH1;
	cr.H2 = iH2;
	int iConsideredStep = 2;
	if (cr.Channel < CH_F)
	{
		// true :向前，false：向后
		bool tDirection = (cr.Channel == CH_A1 || cr.Channel == CH_B1 || cr.Channel == CH_C || cr.Channel == CH_D);
		if (imaxValue > 0)
		{
			A_Step step;
			step.Index = DataA.vAStepDatas[iIndex].Index;
			step.Index2 = DataA.vAStepDatas[iIndex].Index2;
			step.Block = DataA.vAStepDatas[iIndex].Block;
			step.Step = DataA.vAStepDatas[iIndex].Step;
			bool bFindFrame = false;
			for (int j = 0; j < DataA.vAStepDatas[iIndex].Frames.size(); ++j)
			{
				A_Frame &frame = DataA.vAStepDatas[iIndex].Frames[j];
				if (frame.Horizon >= iH1Raw - 4 && frame.Horizon <= iH2Raw + 4 && frame.F[nChA] > 0)
				{
					bFindFrame = true;
					step.Frames.emplace_back(frame);
				}
				else if (bFindFrame && frame.F[nChA] == 0)
				{
					break;
				}
			}
			vSteps.emplace_back(step);

			//往小步进找
			int iHS1 = step.Frames[0].Horizon;
			int iHE1 = step.Frames[vSteps[0].Frames.size() - 1].Horizon;
			int iFirstFrame = step.Index2;

			//往大步进找
			int iHS2 = iHS1;
			int iHE2 = iHE1;
			int iLastFrame = step.Index2;

			//往小步进找
			if (tDirection)
			{
				//for (int i = iIndex - 1; i >= iBeginStepIndex; --i)
				for (int i = iIndex - 1; i >= iBeginStepIndex - iConsideredStep && i >= 0; --i)
				{
					if (GetStepPeaks(DataA.vAStepDatas[i], nChA) >= 5)
					{
						continue;
					}
					A_Step step;
					step.Index = DataA.vAStepDatas[i].Index;
					step.Index2 = DataA.vAStepDatas[i].Index2;
					step.Block = DataA.vAStepDatas[i].Block;
					step.Step = DataA.vAStepDatas[i].Step;
					int nFramesCount = DataA.vAStepDatas[i].Frames.size();
					bool bFindFrame = false;
					for (int j = 0; j < nFramesCount; ++j)
					{
						A_Frame &frame = DataA.vAStepDatas[i].Frames[j];
						if (frame.Horizon >= iHS1 && frame.Horizon <= iHS1 + g_iAFrameGapLimit && frame.Horizon >= iH1 - 4 && frame.Horizon <= iH2 + 4 && frame.F[nChA] > 0)
						{
							//if (cr.Channel < CH_C && frame.Horizon >= 425)
							//{
							//	continue;
							//}
							step.Frames.emplace_back(frame);
							bFindFrame = true;
						}
						else if (bFindFrame && frame.F[nChA] == 0)
						{
							break;
						}
					}
					if (step.Frames.size() > 0 && iFirstFrame - step.Index2 == 1)
					{
						vSteps.insert(vSteps.begin(), step);
						iFirstFrame = step.Index2;
						iHS1 = step.Frames[0].Horizon;
					}
					else
					{
						if (isUnique && i >= iBeginStepIndex && i <= iEndStepIndex)
						{
							iFirstFrame = step.Index2;
						}
						else
						{
							break;
						}
					}
				}

				//for (int i = iIndex + 1; i < DataA.vAStepDatas.size() && i <= iEndStepIndex; ++i)
				for (int i = iIndex + 1; i < DataA.vAStepDatas.size() && i <= iEndStepIndex + iConsideredStep; ++i)
				{
					if (GetStepPeaks(DataA.vAStepDatas[i], nChA) >= 5)
					{
						continue;
					}
					A_Step step;
					step.Index = DataA.vAStepDatas[i].Index;
					step.Index2 = DataA.vAStepDatas[i].Index2;
					step.Block = DataA.vAStepDatas[i].Block;
					step.Step = DataA.vAStepDatas[i].Step;
					int nFramesCount = DataA.vAStepDatas[i].Frames.size();
					bool bFindFrame = false;
					for (int j = 0; j < nFramesCount; ++j)
					{
						A_Frame &frame = DataA.vAStepDatas[i].Frames[j];
						if (frame.Horizon >= iHE2 - g_iAFrameGapLimit && frame.Horizon <= iHE2 /*&& frame.Horizon >= iH1 - 4 && frame.Horizon <= iH2 + 4*/ && frame.F[nChA] > 0)
						{
							//if (cr.Channel < CH_C && frame.Horizon >= 425)
							//{
							//	break;
							//}
							step.Frames.emplace_back(frame);
							bFindFrame = true;
						}
						else if (bFindFrame && frame.F[nChA] == 0)
						{
							break;
						}
					}
					if (step.Frames.size() > 0 && step.Index2 - iLastFrame == 1)
					{
						vSteps.emplace_back(step);
						iLastFrame = step.Index2;
						iHE2 = step.Frames[step.Frames.size() - 1].Horizon;
					}
					else
					{
						if (isUnique && i >= iBeginStepIndex && i <= iEndStepIndex)
						{
							iLastFrame = step.Index2;
						}
						else
						{
							break;
						}
					}
				}
			}
			else
			{
				//for (int i = iIndex - 1; i >= iBeginStepIndex; --i)
				for (int i = iIndex - 1; i >= iBeginStepIndex - iConsideredStep && i >= 0; --i)
				{
					if (GetStepPeaks(DataA.vAStepDatas[i], nChA) >= 5)
					{
						continue;
					}
					A_Step step;
					step.Index = DataA.vAStepDatas[i].Index;
					step.Index2 = DataA.vAStepDatas[i].Index2;
					step.Block = DataA.vAStepDatas[i].Block;
					step.Step = DataA.vAStepDatas[i].Step;
					int nFramesCount = DataA.vAStepDatas[i].Frames.size();
					bool bFindFrame = false;
					for (int j = 0; j < nFramesCount; ++j)
					{
						A_Frame &frame = DataA.vAStepDatas[i].Frames[j];
						if (frame.Horizon >= iHE1 - g_iAFrameGapLimit && frame.Horizon <= iHE1 /*&&frame.Horizon >= iH1 - 4 && frame.Horizon <= iH2 + 4*/ && frame.F[nChA] > 0)
						{
							//if (cr.Channel < CH_C && frame.Horizon >= 425)
							//{
							//	break;
							//}
							bFindFrame = true;
							step.Frames.emplace_back(frame);
						}
						else if (bFindFrame && frame.F[nChA] == 0)
						{
							break;
						}
					}
					if (step.Frames.size() > 0 && iFirstFrame - step.Index2 == 1)
					{
						vSteps.insert(vSteps.begin(), step);
						iFirstFrame = step.Index2;
						iHE1 = step.Frames[step.Frames.size() - 1].Horizon;
					}
					else
					{
						if (isUnique && i >= iBeginStepIndex && i <= iEndStepIndex)
						{
							iFirstFrame = step.Index2;
						}
						else
						{
							break;
						}
					}
				}

				//for (int i = iIndex + 1; i < DataA.vAStepDatas.size() && i <= iEndStepIndex; ++i)
				for (int i = iIndex + 1; i < DataA.vAStepDatas.size() && i <= iEndStepIndex + iConsideredStep; ++i)
				{
					if (GetStepPeaks(DataA.vAStepDatas[i], nChA) >= 5)
					{
						continue;
					}
					A_Step step;
					step.Index = DataA.vAStepDatas[i].Index;
					step.Index2 = DataA.vAStepDatas[i].Index2;
					step.Block = DataA.vAStepDatas[i].Block;
					step.Step = DataA.vAStepDatas[i].Step;
					int nFramesCount = DataA.vAStepDatas[i].Frames.size();
					bool bFindFrame = false;
					for (int j = 0; j < nFramesCount; ++j)
					{
						A_Frame &frame = DataA.vAStepDatas[i].Frames[j];
						if (frame.Horizon >= iHS2 && frame.Horizon <= iHS2 + g_iAFrameGapLimit && /*frame.Horizon >= iH1 - 4 && frame.Horizon <= iH2 + 4 && */frame.F[nChA] > 0)
						{
							//if (cr.Channel < CH_C && frame.Horizon >= 425)
							//{
							//	break;
							//}
							bFindFrame = true;
							step.Frames.emplace_back(frame);
						}
						else if (frame.Horizon > iHS2 + g_iAFrameGapLimit || bFindFrame && frame.F[nChA] == 0)
						{
							break;
						}
					}
					if (step.Frames.size() > 0 && step.Index2 - iLastFrame == 1)
					{
						vSteps.emplace_back(step);
						iLastFrame = step.Index2;
						iHS2 = step.Frames[0].Horizon;
					}
					else
					{
						if (isUnique && i >= iBeginStepIndex && i <= iEndStepIndex)
						{
							iLastFrame = step.Index2;
						}
						else
						{
							break;
						}
					}
				}
			}
		}
	}
	else if (cr.Channel >= CH_F && (cr.IsLose == 1 || (cr.Region[0].find & BIT0) == 0))
	{

	}
	else
	{
		if (imaxValue > 0)
		{
			A_Step step;
			step.Index = DataA.vAStepDatas[iIndex].Index;
			step.Index2 = DataA.vAStepDatas[iIndex].Index2;
			step.Block = DataA.vAStepDatas[iIndex].Block;
			step.Step = DataA.vAStepDatas[iIndex].Step;
			bool bFindFrame = false;
			for (int j = 0; j < DataA.vAStepDatas[iIndex].Frames.size(); ++j)
			{
				A_Frame &frame = DataA.vAStepDatas[iIndex].Frames[j];
				if (frame.Horizon >= iH1 - 4 && frame.Horizon <= iH2 + 4 && frame.F[nChA] > 0)
				{
					//if (cr.Channel < CH_C && frame.Horizon >= 425)
					//{
					//	break;
					//}
					bFindFrame = true;
					step.Frames.emplace_back(frame);
				}
				else if (bFindFrame && frame.F[nChA] == 0)
				{
					break;
				}
			}
			vSteps.emplace_back(step);
			int iFirstFrame = step.Index2, iLastFrame = step.Index2;

			//往小步进找
			for (int i = iIndex - 1; i >= 0; --i)
			{
				A_Step step;
				step.Index = DataA.vAStepDatas[i].Index;
				step.Index2 = DataA.vAStepDatas[i].Index2;
				step.Block = DataA.vAStepDatas[i].Block;
				step.Step = DataA.vAStepDatas[i].Step;
				int nFramesCount = DataA.vAStepDatas[i].Frames.size();
				bool bFindFrame = false;
				for (int j = 0; j < nFramesCount; ++j)
				{
					A_Frame &frame = DataA.vAStepDatas[i].Frames[j];
					if (frame.Horizon >= iH1 && frame.Horizon <= iH2 && frame.F[nChA] > 0)
					{
						//if (cr.Channel < CH_C && frame.Horizon >= 425)
						//{
						//	break;
						//}
						step.Frames.emplace_back(frame);
						bFindFrame = true;
					}
					else if (bFindFrame && frame.F[nChA] == 0)
					{
						break;
					}
				}
				if (step.Frames.size() > 0 && iFirstFrame - step.Index2 == 1)
				{
					vSteps.insert(vSteps.begin(), step);
					iFirstFrame = step.Index2;
				}
				else
				{
					break;
				}
			}

			for (int i = iIndex + 1; i < DataA.vAStepDatas.size(); ++i)
			{
				A_Step step;
				step.Index = DataA.vAStepDatas[i].Index;
				step.Index2 = DataA.vAStepDatas[i].Index2;
				step.Block = DataA.vAStepDatas[i].Block;
				step.Step = DataA.vAStepDatas[i].Step;
				int nFramesCount = DataA.vAStepDatas[i].Frames.size();
				bool bFindFrame = false;
				for (int j = 0; j < nFramesCount; ++j)
				{
					A_Frame &frame = DataA.vAStepDatas[i].Frames[j];
					if (frame.Horizon >= iH1 - g_iAFrameGapLimit && frame.Horizon <= iH2 && frame.F[nChA] > 0)
					{
						//if (cr.Channel < CH_C && frame.Horizon >= 425)
						//{
						//	break;
						//}
						step.Frames.emplace_back(frame);
						bFindFrame = true;
					}
					else if (bFindFrame && frame.F[nChA] == 0)
					{
						break;
					}
				}
				if (step.Frames.size() > 0 && step.Index2 - iLastFrame == 1)
				{
					vSteps.emplace_back(step);
					iLastFrame = step.Index2;
				}
				else
				{
					break;
				}
			}
		}
	}

	if (isUnique && cr.Channel < CH_F)
	{
		int h1 = 500, h2 = 0;
		for (int i = 0; i < cr.vASteps.size(); ++i)
		{
			int nFrames = cr.vASteps[i].Frames.size();
			for (int j = 0; j < nFrames; ++j)
			{
				if (cr.vASteps[i].Frames[j].Horizon < h1)
				{
					h1 = cr.vASteps[i].Frames[j].Horizon;
				}
				if (cr.vASteps[i].Frames[nFrames - 1].Horizon > h2)
				{
					h2 = cr.vASteps[i].Frames[nFrames - 1].Horizon;
				}
			}
		}
		cr.H1 = h1;
		cr.H2 = h2;
	}
	if (cr.IsContainA == 1 && vSteps.size() == 0 && cr.Region.size() >= 5)
	{
		cr.IsContainA = 0;
		cr.H1 = iH1;
		cr.H2 = iH2;
		cr.Info.MinH = iH1;
		cr.Info.MaxH = iH2;
	}
	if (cr.Channel <= CH_C && cr.Info.MaxH >= 425)
	{
		cr.Info.MaxH = 425;
		if (cr.Info.MinH >= 425)
		{
			cr.Info.MinH = 425;
		}
	}
	if (cr.Channel == CH_D && cr.vASteps.size() > 1)
	{
		VINT beginHorizons;
		for (int i = 0; i < cr.vASteps.size(); ++i)
		{
			beginHorizons.emplace_back(cr.vASteps[i].Frames[0].Horizon);
		}

		VINT vDelt;
		int totalDelt = 0, ave = 0;
		for (int i = 0; i < beginHorizons.size() - 1; ++i)
		{
			vDelt.emplace_back(beginHorizons[i] - beginHorizons[i + 1]);
			totalDelt += vDelt[i];
		}
		ave = totalDelt / vDelt.size();

		int d = -1;
		for (int i = 1; i < vDelt.size(); ++i)
		{
			if (vDelt[i] > ave + 10)
			{
				d = i;
				break;
			}
		}

		if (d >= 0)
		{
			VASTEPS vaas1, vaas2;
			for (int i = cr.vASteps.size() - 1; i > d; --i)
			{
				vaas1.emplace_back(cr.vASteps[i]);
			}
			for (int i = d; i >= 0; --i)
			{
				vaas2.emplace_back(cr.vASteps[i]);
			}

			int stepData[4] = { 0 };
			int os1 = GetOverlappedStep(iBeginStep, iEndStep, vaas1[vaas1.size() - 1].Step, vaas1[0].Step, stepData[0], stepData[1]);
			int os2 = GetOverlappedStep(iBeginStep, iEndStep, vaas2[vaas2.size() - 1].Step, vaas2[0].Step, stepData[2], stepData[3]);
			if (os1 > os2)
			{
				cr.vASteps = vaas1;
			}
			else
			{
				cr.vASteps = vaas2;
			}
		}
	}
	else if (cr.Channel == CH_E && cr.vASteps.size() > 1)
	{
		VINT beginHorizons;
		for (int i = 0; i < cr.vASteps.size(); ++i)
		{
			beginHorizons.emplace_back(cr.vASteps[i].Frames[0].Horizon);
		}

		VINT vDelt;
		int totalDelt = 0, ave = 0;
		for (int i = 0; i < beginHorizons.size() - 1; ++i)
		{
			vDelt.emplace_back(beginHorizons[i + 1] - beginHorizons[i]);
			totalDelt += vDelt[i];
		}
		ave = totalDelt / vDelt.size();

		int d = -1;
		for (int i = 1; i < vDelt.size(); ++i)
		{
			if (vDelt[i] > ave + 10)
			{
				d = i;
				break;
			}
		}

		if (d >= 0)
		{
			VASTEPS vaas1, vaas2;
			for (int i = cr.vASteps.size() - 1; i > d; --i)
			{
				vaas1.emplace_back(cr.vASteps[i]);
			}
			for (int i = d; i >= 0; --i)
			{
				vaas2.emplace_back(cr.vASteps[i]);
			}

			int stepData[4] = { 0 };
			int os1 = GetOverlappedStep(iBeginStep, iEndStep, vaas1[vaas1.size() - 1].Step, vaas1[0].Step, stepData[0], stepData[1]);
			int os2 = GetOverlappedStep(iBeginStep, iEndStep, vaas2[vaas2.size() - 1].Step, vaas2[0].Step, stepData[2], stepData[3]);
			if (os1 > os2)
			{
				cr.vASteps = vaas1;
			}
			else
			{
				cr.vASteps = vaas2;
			}
		}
	}
	return vSteps.size() > 0;
}



bool	RemoveHoleCR(VCR& vcr, VINT& crIndexes)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].IsGuideHole || vcr[crIndexes[i]].IsScrewHole)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool	RemoveHoleCRExcept(VCR& vcr, VINT& crIndexes, int except)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0 || crIndexes[i] == except)
		{
			continue;
		}
		if (vcr[crIndexes[i]].IsGuideHole || vcr[crIndexes[i]].IsScrewHole)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool	RemoveJointCR(VCR& vcr, VINT& crIndexes)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].IsJoint)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool RemoveCRByLengthLimit(VCR& vcr, VINT& crIndexes, int maxLength)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].Step2 - vcr[crIndexes[i]].Step1 > maxLength)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool	RemoveUsedCR(VCR& vcr, VINT& crIndexes)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].IsUsed != 0)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool	RemoveScrewHoleCR(VCR& vcr, VINT& crIndexes)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].IsScrewHole)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool	RemoveCRByRow1Limit(VCR& vcr, VINT& crIndexes, uint8_t row1Limit)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].Row1 < row1Limit)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool RemoveCRByWoundFlag(VCR& vcr, VINT& crIndexes)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].IsWound != 0)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool	RemovePMCR(VCR& vcr, VINT& crIndexes)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].IsSew || vcr[crIndexes[i]].IsJoint)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}


bool	RemoveCRByLose(VCR& vcr, VINT& crIndexes, int lose)
{
	for (int i = crIndexes.size() - 1; i >= 0; --i)
	{
		if (crIndexes[i] >= vcr.size() || crIndexes[i] < 0)
		{
			continue;
		}
		if (vcr[crIndexes[i]].IsLose != lose)
		{
			crIndexes.erase(crIndexes.begin() + i);
		}
	}
	return crIndexes.size() > 0;
}

bool RemoveDirtyCR(VCR& vcr, VINT& crIndexes, int iJawRow)
{
	int sz = 0, row1 = 0, row2 = 0;
	int maxSize = 0;
	int small = 0;
#ifdef _DEBUG
	VINT crsizes;
#endif // _DEBUG

	for (int i = 0; i < crIndexes.size(); ++i)
	{
		sz += vcr[crIndexes[i]].Region.size();
		row1 += vcr[crIndexes[i]].Row1;
		row2 += vcr[crIndexes[i]].Row2;
#ifdef _DEBUG
		crsizes.emplace_back(vcr[crIndexes[i]].Region.size());
#endif // _DEBUG
		if (maxSize < vcr[crIndexes[i]].Region.size())
		{
			maxSize = vcr[crIndexes[i]].Region.size();
		}
		if (vcr[crIndexes[i]].Region.size() <= 2)
		{
			++small;
		}
	}
	if (small >= crIndexes.size() / 2)
	{
		for (int i = crIndexes.size() - 1; i >= 0; --i)
		{
			if (vcr[crIndexes[i]].Row2 - vcr[crIndexes[i]].Row1 <= 5)
			{
				crIndexes.erase(crIndexes.begin() + i);
			}
		}
	}
	else
	{
		int avesz = sz / crIndexes.size();
		int averow1 = row1 / crIndexes.size();
		int averow2 = row2 / crIndexes.size();
		int szLimit = maxSize / 2;
		for (int i = crIndexes.size() - 1; i >= 0; --i)
		{
			if (
				vcr[crIndexes[i]].Region.size() < szLimit ||
				(vcr[crIndexes[i]].Row1 >= averow1 - 1 && vcr[crIndexes[i]].Row2 <= averow2 + 1)
				)
			{
				crIndexes.erase(crIndexes.begin() + i);
			}
		}
	}
	return crIndexes.size() > 0;
}


bool	IsCRDirty_ABC(VBDB& blocks, VCR* vCRs, CR& cr, uint8_t isDeleteWoundCR /* = 0 */)
{
	if (cr.Channel >= CH_F)
	{
		return false;
	}
	bool isOK = true;

	uint8_t iJawRow = g_iJawRow[blocks[cr.Block - g_iBeginBlock].BlockHead.railType & 0x03];
	uint8_t iMaxHeadRow = 2 * iJawRow;

	CR temp;
	std::map<uint8_t, int> rowDistribute;
	GetCoreCR(cr, rowDistribute, temp, blocks, iJawRow);
	int maxRowPointCount = (--rowDistribute.end())->second;
	int imaxRowIndex = rowDistribute.begin()->first;
	int imaxRowCount = rowDistribute.begin()->second;
	for (auto itr2 = rowDistribute.begin(); itr2 != rowDistribute.end(); ++itr2)
	{
		if (itr2->second >= imaxRowCount)
		{
			imaxRowIndex = itr2->first;
			imaxRowCount = itr2->second;
		}
	}
	if (imaxRowIndex == iMaxHeadRow && cr.Channel < CH_C && cr.Row2 - cr.Row1 <= 3)
	{
		return true;
	}

	VINT crA, crALeft, crARight, crAMiddle;
	uint8_t row1 = temp.Row1 >= 2 ? temp.Row1 - 2 : 0;
	uint8_t row2 = temp.Row2;
	if (cr.Row2 - cr.Row1 <= 3 && cr.Channel <= CH_C)
	{
		row1 = cr.Row1 - 2;
		row2 = cr.Row2 + 2;
	}
	int stepLimit = 250;
	if (cr.Region.size() >= 8 || cr.Row2 - cr.Row1 >= 4)
	{
		stepLimit = 150;
	}

	int s1 = temp.Step1 - 2 * stepLimit, s2 = temp.Step2 + 2 * stepLimit;
	int s3 = temp.Step1 - stepLimit, s4 = temp.Step2 + stepLimit;
	for (int i = 0; i < g_vReturnSteps.size(); ++i)
	{
		if (g_vReturnSteps[i] >= s1 && g_vReturnSteps[i] <= cr.Step1)
		{
			s1 = g_vReturnSteps[i];
			if (g_vReturnSteps[i] >= s3)
			{
				s3 = g_vReturnSteps[i];
			}
		}
		if (g_vReturnSteps[i] >= cr.Step2 && g_vReturnSteps[i] <= s4)
		{
			s4 = g_vReturnSteps[i];
			if (g_vReturnSteps[i] <= s2)
			{
				s2 = g_vReturnSteps[i];
			}
		}
	}
	VINT crAIn;
	crAIn.emplace_back(cr.Index);
	//GetCR(temp.Channel, cr.Step1 - 10, row, cr.Step2 + 10, temp.Row2 + 2, blocks, vCRs[temp.Channel], crAIn, -1, 1);

	GetCR(temp.Channel, max(g_iBeginStep, s1), row1, min(g_iEndStep, temp.Step2), row2, blocks, vCRs[temp.Channel], crARight, crAIn, 1);
	GetCR(temp.Channel, max(g_iBeginStep, temp.Step1), row1, min(g_iEndStep, s2), row2, blocks, vCRs[temp.Channel], crALeft, crAIn, 1);
	GetCR(temp.Channel, max(g_iBeginStep, s3), row1, min(g_iEndStep, s4), row2, blocks, vCRs[temp.Channel], crAMiddle, crAIn, 1);


	RemovePMCR(vCRs[cr.Channel], crARight);
	RemovePMCR(vCRs[cr.Channel], crALeft);
	RemovePMCR(vCRs[cr.Channel], crAMiddle);
	if (crARight.size() == 0 && crALeft.size() == 0 && crAMiddle.size() == 0)
	{
		isOK = false;
		return false;
	}

	int iMaxIndex = -1;
	if (crALeft.size() >= crARight.size())
	{
		crA = crALeft;
		iMaxIndex = 1;
	}
	else
	{
		crA = crARight;
		iMaxIndex = 2;
	}

	if (crA.size() < crAMiddle.size())
	{
		crA = crAMiddle;
		iMaxIndex = 3;
	}
	int stepRectCount = 0, iBeginStep = 0, iEndStep = 0;
	int stepRectWidth = 30;
	if (iMaxIndex == 1)
	{
		s2 = max(s4, vCRs[temp.Channel][crA[crA.size() - 1]].Step2);
		s2 = min(g_iEndStep, s2);
		stepRectCount = (s2 - temp.Step1) / stepRectWidth;

		iBeginStep = temp.Step1;
		iEndStep = s2;
	}
	else if (iMaxIndex == 2)
	{
		s1 = min(s1, vCRs[temp.Channel][crA[0]].Step1);
		s1 = max(s1, g_iBeginStep);
		stepRectCount = (temp.Step2 - s1) / stepRectWidth;
		iBeginStep = s1;
		iEndStep = temp.Step2;
	}
	else
	{
		s3 = min(s3, vCRs[temp.Channel][crA[0]].Step1);
		s3 = max(s3, g_iBeginStep);
		s4 = max(s4, vCRs[temp.Channel][crA[crA.size() - 1]].Step2);
		s4 = min(s4, g_iEndStep);
		stepRectCount = (s4 - s3) / stepRectWidth;
		iBeginStep = s3;
		iEndStep = s4;
	}

	int *filled = new int[stepRectCount + 1];
	memset(filled, 0, sizeof(int) * (stepRectCount + 1));
	//uint8_t filled[1000] = { 0 };
	VCR vTempCR;
	std::map<int, int> vMapIndexes;
	for (int j = 0; j < crA.size(); ++j)
	{
		vTempCR.emplace_back(vCRs[temp.Channel][crA[j]]);
		int index1 = (vCRs[temp.Channel][crA[j]].Step1 - iBeginStep) / stepRectWidth;
		int index2 = (vCRs[temp.Channel][crA[j]].Step2 - iBeginStep) / stepRectWidth;
		index1 = max(index1, 0);
		index2 = min(index2, stepRectCount);
		for (int k = index1; k <= index2; ++k)
		{
			filled[k] = 1;
			vMapIndexes[k] = 1;
		}

	}

	int filledCount = 0;
	for (int i = 0; i < stepRectCount + 1; ++i)
	{
		if (filled[i] == 1)
		{
			filledCount++;
		}
	}
	delete filled;


	uint32_t mins = 0x7FFFFFFF, maxs = 0;
	int nCR = crA.size();
	int nCR2 = nCR, nBigCR = 0;
	int szLimit = 0.5 * cr.Region.size() / 2;
	if (szLimit < 5)
	{
		szLimit = 5;
	}


	GetCRsStep(vCRs[cr.Channel], crA, mins, maxs);
	if (maxs - mins < 100)
	{
		isOK = false;
		goto PT_HANDLE;
	}

	if (cr.Row2 - cr.Row1 > 2 && cr.Region.size() > 7)
	{
		if (1.0 * vMapIndexes.size() <= 0.6 * (maxs - mins) / stepRectWidth)
		{
			isOK = false;
			goto PT_HANDLE;
		}
	}
	if (cr.Row2 - cr.Row1 <= 2 || cr.Region.size() <= 7)
	{
		isOK = true;
		goto PT_HANDLE;
	}

	if (cr.Row2 - cr.Row1 <= 2 && cr.Region.size() <= 7 && filledCount >= 3)
	{
		isOK = true;
		goto PT_HANDLE;
	}

	//if (vMapIndexes.size() != filledCount)
	//{
	//	FILE* pFile = fopen("D:/err.txt", "a");
	//	fprintf(pFile, "cr.Block = %d, step = %d\n", cr.Block, cr.Step);
	//	fprintf(pFile, "iMaxIndex = %d\n\n", iMaxIndex);
	//	fflush(pFile);
	//	fclose(pFile);
	//}

	for (int ia = nCR2 - 1; ia >= 0; --ia)
	{
		if (vCRs[temp.Channel][crA[ia]].IsSew != 0 || vCRs[temp.Channel][crA[ia]].IsJoint != 0)
		{
			nCR--;
			crA.erase(crA.begin() + ia);
		}
		else
		{
			if (vCRs[temp.Channel][crA[ia]].Row1 <= cr.Row1 && vCRs[temp.Channel][crA[ia]].Row2 >= cr.Row2 &&
				cr.Row1 - vCRs[temp.Channel][crA[ia]].Row1 + vCRs[temp.Channel][crA[ia]].Row2 - cr.Row2 > 0
				)
			{
				if (cr.Step1 > vCRs[temp.Channel][crA[ia]].Step1 && cr.Step1 >= vCRs[temp.Channel][crA[ia]].Step1 + 15 ||
					cr.Step1 < vCRs[temp.Channel][crA[ia]].Step1 && cr.Step1 <= vCRs[temp.Channel][crA[ia]].Step1 - 15)
				{
					goto PT_HANDLE;
					return true;
				}
			}

			if (vCRs[temp.Channel][crA[ia]].Region.size() >= szLimit)
			{
				nBigCR++;
			}
		}
	}




	int dRow = cr.Row2 - cr.Row1;
	double ra1 = 0, ra2 = 0;
	if (temp.Region.size() >= 7 && nCR >= 5 || temp.Region.size() < 7 && temp.Region.size() >= 4 && nCR >= 3 || temp.Region.size() < 4 && nCR >= 2)
	{
		if (dRow >= 5 && nCR < 5 || dRow < 5 && dRow > 3 && nCR < 2 || dRow <= 3 && nCR == 0)
		{
			isOK = false;
			goto PT_HANDLE;
		}

		if (nBigCR > 0)
		{
			for (int ia = 0; ia < nCR; ++ia)
			{
				if (vCRs[temp.Channel][crA[ia]].Region.size() >= szLimit)
				{
					ra1 += vCRs[temp.Channel][crA[ia]].Row1;
					ra2 += vCRs[temp.Channel][crA[ia]].Row2;
				}
			}

			ra1 = ra1 / nBigCR;
			ra2 = ra2 / nBigCR;
			if (nBigCR > 0 && cr.Row2 == cr.Row1)
			{

			}
			else if (nBigCR < 8 && (cr.Row1 < ra1 && fabs(ra1 - cr.Row1) >= 2.5 || cr.Row2 >= ra2 && fabs(ra2 - cr.Row2) >= 2.5)
				|| (nBigCR >= 8 && (cr.Row1 < ra1 && fabs(ra1 - cr.Row1) >= 3.5 || cr.Row2 >= ra2 && fabs(ra2 - cr.Row2) >= 3.5))
				|| 1.0 * nBigCR / (cr.Row2 - cr.Row1) <= 0.6)
			{
				isOK = false;
				goto PT_HANDLE;
			}
		}
		else
		{
			isOK = false;
			goto PT_HANDLE;
		}
	}
	else
	{
		isOK = false;
		goto PT_HANDLE;
	}

PT_HANDLE:
	//if (isDeleteWoundCR == 1 && (cr.Block == 437 || cr.Block == 7102) && isOK)
	//{
	//	FILE* pFile = fopen("D:/err.txt", "a");
	//	fprintf(pFile, "s1 = %d, s2 = %d, s3 = %d, s4 = %d\nfilledCount = %d, steprect = %d, \nnCR = %d, dRow = %d, row1 = %d, row2 = %d, ra1 = %lf, ra2 = %lf\n", s1, s2, s3, s4, filledCount, stepRectCount, nCR, dRow, cr.Row1, cr.Row2, ra1, ra2);
	//	fprintf(pFile, "iMaxIndex = %d\n\n", iMaxIndex);
	//	fflush(pFile);
	//	fclose(pFile);
	//}
	return isOK;
}


bool	IsCRDirty_DE(VBDB& blocks, VCR* vCRs, CR& cr, uint8_t isDeleteWoundCR /* = 0 */)
{
	if (cr.Channel != CH_D && cr.Channel != CH_E)
	{
		return false;
	}

	if (cr.Row2 - cr.Row1 >= 4 && cr.Region.size() >= 7 && cr.Step2 - cr.Step1 <= cr.Row2 - cr.Row1)
	{
		return false;
	}

	bool isOK = true;
	CR& temp = cr;
	VINT crA, crALeft, crARight, crAMiddle;
	uint8_t row1 = temp.Row1 >= 2 ? temp.Row1 - 2 : 0;
	uint8_t row2 = temp.Row2;
	if (cr.Row2 - cr.Row1 <= 3 && cr.Channel <= CH_C)
	{
		row1 = cr.Row1 - 2;
		row2 = cr.Row2 + 2;
	}
	int stepLimit = 250;
	if (cr.Region.size() >= 8 || cr.Row2 - cr.Row1 >= 4)
	{
		stepLimit = 150;
	}

	int s1 = temp.Step1 - 2 * stepLimit, s2 = temp.Step2 + 2 * stepLimit;
	int s3 = temp.Step1 - stepLimit, s4 = temp.Step2 + stepLimit;
	for (int i = 0; i < g_vReturnSteps.size(); ++i)
	{
		if (g_vReturnSteps[i] >= s1 && g_vReturnSteps[i] <= cr.Step1)
		{
			s1 = g_vReturnSteps[i];
			if (g_vReturnSteps[i] >= s3)
			{
				s3 = g_vReturnSteps[i];
			}
		}
		if (g_vReturnSteps[i] >= cr.Step2 && g_vReturnSteps[i] <= s4)
		{
			s4 = g_vReturnSteps[i];
			if (g_vReturnSteps[i] <= s2)
			{
				s2 = g_vReturnSteps[i];
			}
		}
	}
	VINT crAIn;
	crAIn.emplace_back(cr.Index);
	//GetCR(temp.Channel, cr.Step1 - 10, row, cr.Step2 + 10, temp.Row2 + 2, blocks, vCRs[temp.Channel], crAIn, -1, 1);

	GetCR(temp.Channel, max(g_iBeginStep, temp.Step1), row1, min(g_iEndStep, s2), row2, blocks, vCRs[temp.Channel], crALeft, crAIn, 1);
	GetCR(temp.Channel, max(g_iBeginStep, s1), row1, min(g_iEndStep, temp.Step2), row2, blocks, vCRs[temp.Channel], crARight, crAIn, 1);
	GetCR(temp.Channel, max(g_iBeginStep, s3), row1, min(g_iEndStep, s4), row2, blocks, vCRs[temp.Channel], crAMiddle, crAIn, 1);

	if (crARight.size() == 0 && crALeft.size() == 0 && crAMiddle.size() == 0)
	{
		isOK = false;
		return false;
	}

	int iMaxIndex = -1, iBeginStep = 0, iEndStep = 0;
	if (crALeft.size() >= crARight.size())
	{
		crA = crALeft;
		iMaxIndex = 1;
		iBeginStep = max(g_iBeginStep, temp.Step1);
		iEndStep = min(g_iEndStep, s2);
	}
	else
	{
		crA = crARight;
		iMaxIndex = 2;
		iBeginStep = max(g_iBeginStep, s1);
		iEndStep = min(g_iEndStep, temp.Step2);
	}

	if (crA.size() < crAMiddle.size())
	{
		crA = crAMiddle;
		iMaxIndex = 3;
		iBeginStep = max(g_iBeginStep, s3);
		iEndStep = min(g_iEndStep, s4);
	}

	int stepRectWidth = 15;
	int stepRectCount = (iEndStep - iBeginStep) / stepRectWidth;


	int *filled = new int[stepRectCount + 1];
	memset(filled, 0, sizeof(int) * (stepRectCount + 1));
	//uint8_t filled[1000] = { 0 };
	VCR vTempCR;
	std::map<int, int> vMapIndexes;
	for (int j = 0; j < crA.size(); ++j)
	{
		vTempCR.emplace_back(vCRs[temp.Channel][crA[j]]);
		int index1 = (vCRs[temp.Channel][crA[j]].Step1 - iBeginStep) / stepRectWidth;
		int index2 = (vCRs[temp.Channel][crA[j]].Step2 - iBeginStep) / stepRectWidth;
		index1 = max(index1, 0);
		index2 = min(index2, stepRectCount);
		for (int k = index1; k <= index2; ++k)
		{
			filled[k] = 1;
			vMapIndexes[k] = 1;
		}

	}

	int filledCount = 0;
	for (int i = 0; i < stepRectCount + 1; ++i)
	{
		if (filled[i] == 1)
		{
			filledCount++;
		}
	}
	delete filled;


	uint32_t mins = 0x7FFFFFFF, maxs = 0;
	int nCR = crA.size();
	int nCR2 = nCR, nBigCR = 0;
	int szLimit = 0.5 * cr.Region.size() >= 10 ? 5 : 0.5 * cr.Region.size() / 2;

	GetCRsStep(vCRs[cr.Channel], crA, mins, maxs);
	if (maxs - mins < 100)
	{
		isOK = false;
		goto PT_HANDLE;
	}

	if (1.0 * vMapIndexes.size() <= 0.4 * (iEndStep - iBeginStep) / stepRectWidth)
	{
		isOK = false;
		goto PT_HANDLE;
	}

	for (int ia = nCR2 - 1; ia >= 0; --ia)
	{
		if (vCRs[temp.Channel][crA[ia]].IsSew != 0 || vCRs[temp.Channel][crA[ia]].IsJoint != 0)
		{
			nCR--;
			crA.erase(crA.begin() + ia);
		}
		else
		{
			if (vCRs[temp.Channel][crA[ia]].Region.size() >= szLimit)
			{
				nBigCR++;
			}
		}
	}




	int dRow = cr.Row2 - cr.Row1;
	double ra1 = 0, ra2 = 0;
	if (temp.Region.size() >= 7 && nCR >= 5 || temp.Region.size() < 7 && temp.Region.size() >= 4 && nCR >= 3 || temp.Region.size() < 4 && nCR >= 2)
	{
		if (dRow >= 5 && nCR < 5 || dRow < 5 && dRow > 3 && nCR < 2 || dRow <= 3 && nCR == 0)
		{
			isOK = false;
			goto PT_HANDLE;
		}

		if (nBigCR > 0)
		{
			for (int ia = 0; ia < nCR; ++ia)
			{
				if (vCRs[temp.Channel][crA[ia]].Region.size() >= szLimit)
				{
					ra1 += vCRs[temp.Channel][crA[ia]].Row1;
					ra2 += vCRs[temp.Channel][crA[ia]].Row2;
				}
			}

			ra1 = ra1 / nBigCR;
			ra2 = ra2 / nBigCR;
			if (nBigCR > 0 && cr.Row2 == cr.Row1)
			{

			}
			else if (nBigCR < 8 && (cr.Row1 < ra1 && fabs(ra1 - cr.Row1) >= 2.5 || cr.Row2 >= ra2 && fabs(ra2 - cr.Row2) >= 2.5)
				|| 1.0 * nBigCR / (cr.Row2 - cr.Row1) <= 0.6)
			{
				isOK = false;
				goto PT_HANDLE;
			}
		}
		else
		{
			isOK = false;
			goto PT_HANDLE;
		}
	}
	else
	{
		isOK = false;
		goto PT_HANDLE;
	}

PT_HANDLE:
	//if (isDeleteWoundCR == 1 && (cr.Block == 437 || cr.Block == 7102) && isOK)
	//{
	//	FILE* pFile = fopen("D:/err.txt", "a");
	//	fprintf(pFile, "s1 = %d, s2 = %d, s3 = %d, s4 = %d\nfilledCount = %d, steprect = %d, \nnCR = %d, dRow = %d, row1 = %d, row2 = %d, ra1 = %lf, ra2 = %lf\n", s1, s2, s3, s4, filledCount, stepRectCount, nCR, dRow, cr.Row1, cr.Row2, ra1, ra2);
	//	fprintf(pFile, "iMaxIndex = %d\n\n", iMaxIndex);
	//	fflush(pFile);
	//	fclose(pFile);
	//}
	return isOK;
}


void	RemoveWoundCR(Wound_Judged& w, int index)
{
	w.vCRs.erase(w.vCRs.begin() + index);
}

void	GetFRowRegion(VBDB& blocks, int step1, int step2, int stepChanging, std::map<uint8_t, uint8_t>&mapExistRows, std::vector<uint8_t>& vRows, bool isG/* = false */)
{
	Pos pos1 = FindStepInBlock(step1, blocks, 0);
	for (int i = pos1.Block - g_iBeginBlock; i < blocks.size(); ++i)
	{
		for (int s = 0; s < blocks[i].vBStepDatas.size(); ++s)
		{
			if (blocks[i].vBStepDatas[s].Step >= step1 && blocks[i].vBStepDatas[s].Step <= step2)
			{
				int tempRow = GetFRow(blocks[i].vBStepDatas[s]);
				if (isG)
				{
					tempRow = GetGRow(blocks[i].vBStepDatas[s]);
				}
				if (tempRow > 0)
				{
					vRows.emplace_back(tempRow);
					if (blocks[i].vBStepDatas[s].Step >= stepChanging - 30 && blocks[i].vBStepDatas[s].Step <= stepChanging + 30)
					{
						mapExistRows[tempRow] = 1;
					}
				}
			}
			else if (blocks[i].vBStepDatas[s].Step >= step2)
			{
				break;
			}
		}
	}
}

void	SetFRowRegion(VBDB& blocks, int step1, int step2, uint8_t row, std::map<uint32_t, bool>& mapRising, bool isG/* = false */)
{
	Pos pos1 = FindStepInBlock(step1, blocks, 0);
	bool flag = true;
	for (int i = pos1.Block - g_iBeginBlock; i < blocks.size() && flag; ++i)
	{
		for (int s = 0; s < blocks[i].vBStepDatas.size() && flag; ++s)
		{
			if (blocks[i].vBStepDatas[s].Step >= step1 && blocks[i].vBStepDatas[s].Step <= step2)
			{
				if (mapRising.find(blocks[i].vBStepDatas[s].Step) != mapRising.end())
				{
					flag = false;
					break;
				}
				if (isG)
				{
					if (blocks[i].vBStepDatas[s].isFindG == 0 || blocks[i].vBStepDatas[s].GRow > row + 2 || blocks[i].vBStepDatas[s].GRow < row - 2)
					{
						blocks[i].vBStepDatas[s].GRow = row;
					}
				}
				else
				{
					if (blocks[i].vBStepDatas[s].isFindF == 0 || blocks[i].vBStepDatas[s].FRow > row + 2 || blocks[i].vBStepDatas[s].FRow < row - 2)
					{
						blocks[i].vBStepDatas[s].FRow = row;
					}
				}
			}
			else if (blocks[i].vBStepDatas[s].Step >= step2)
			{
				break;
			}
		}
	}
}