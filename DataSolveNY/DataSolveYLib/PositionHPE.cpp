#include "stdafx.h"
#include "CRHPE.h"
#include "PublicFunc.h"
#include "PositionHPE.h"
#include "GlobalDefine.h"


int32_t distri[20000] = { 0 };
int32_t distibute2[20000] = { 0 };

PositionFeature::PositionFeature(VBDB& blocks, VCR* vCRs, VINT* t_cr, int step1, int step2, uint8_t rowCH1, uint8_t rowCH2, uint8_t railType)
{
	memset(distri, 0, sizeof(int32_t) * 20000);
	memset(distibute2, 0, sizeof(int32_t) * 20000);
	std::vector<TPoint> points;
	iAStepBig = 0; iAStepSmall = 0; iAStepBig2 = 0; iAStepSmall2 = 0; iAStepBigNoCc = 0; iAStepSmallNoCc = 0;
	sumAaBb = 0; sumCc = 0; sum_ABC = 0; sum_abc = 0;
	SumAa = 0;	SumBb = 0;
	iSumBigChannel = 0;
	iSumSmallChannel = 0;
	memset(findEveyChannel, 0, sizeof(findEveyChannel));
	memset(sumEveyChannel, 0, sizeof(sumEveyChannel));
	memset(pointCountlInJaw, 0, 8 * sizeof(int));
	sum10_16 = 0;
	maxSize = 0; maxSizeNoCc = 0;
	memset(maxCRSize, 0, 10 * sizeof(int));
	iHeavyChannel = 0; iHeavyChannel2 = 0;
	totalCRCountNoCc = 0; totalCRCount = 0; totalCRCountNoCcInJaw = 0;
	aveCRCountNoCc = 0; aveCRCount = 0; aveCRSize = 0; aveCRSizeNoCc = 0;
	s = 0;
	percent = 0;
	ir1 = 30; ir2 = 0;
	iS2 = step1;
	iS1 = step2;

	int64_t iSumStep = 0, iSumRow = 0;
	//出波通道数
	int32_t sum = 0;

	for (int i = CH_A1; i < CH_C; ++i)
	{
		uint8_t iL = 0, iH = 0;
		for (int k = 0; k < t_cr[i].size(); ++k)
		{
			Connected_Region& crTemp = vCRs[i][t_cr[i][k]];
			if (crTemp.IsIllgeal == 1)
			{
				continue;
			}
			if (maxCRSize[i] <= crTemp.Region.size())
			{
				maxCRSize[i] = crTemp.Region.size();
			}
			if (crTemp.Region.size() >= 8)
			{
				if (i % 4 == 0)
				{
					iAStepBig2 += 0.5 * (crTemp.Step1 + crTemp.Step2) * crTemp.Region.size();
					sum_ABC += crTemp.Region.size();
				}
				else
				{
					iAStepSmall2 += 0.5 * (crTemp.Step1 + crTemp.Step2) * crTemp.Region.size();
					sum_abc += crTemp.Region.size();
				}
			}
			for (int ii = 0; ii < crTemp.Region.size(); ++ii)
			{
				WaveData& wd = crTemp.Region[ii];
				TPoint point;
				point.Step = wd.step;
				point.Row = wd.row;

				if (wd.row < g_iJawRow[railType])
				{
					findEveyChannel[i] = 1;
					sumEveyChannel[i] += 1;
				}
				else
				{
					findEveyChannel[i + 1] = 1;
					sumEveyChannel[i + 1] += 1;
				}
				if (wd.row >= g_iJawRow[railType] - 3 && wd.row <= g_iJawRow[railType] + 3)
				{
					pointCountlInJaw[i]++;
				}
				point.Channel = i;
				points.emplace_back(point);

				if (wd.row < ir1)		ir1 = wd.row;
				if (wd.row > ir2)		ir2 = wd.row;
				if (wd.step < iS1)		iS1 = wd.step;
				if (wd.step > iS2)		iS2 = wd.step;

				if (wd.row > rowCH1 && wd.row < rowCH2)	sum10_16++;

				iSumRow += wd.row;
				iSumStep += wd.step;
			}
			if (crTemp.Row1 <= g_iJawRow[railType])			iL = 1;
			if (crTemp.Row2 >= g_iJawRow[railType])			iH = 1;

			sumAaBb += crTemp.Region.size();
			maxSize = maxSize > crTemp.Region.size() ? maxSize : crTemp.Region.size();
		}
		sum += (iL + iH);
	}

	for (int i = CH_C; i < CH_D; ++i)
	{
		sum += t_cr[i].size() > 0 ? 1 : 0;
		for (int k = 0; k < t_cr[i].size(); ++k)
		{
			Connected_Region& crTemp = vCRs[i][t_cr[i][k]];
			if (crTemp.IsIllgeal == 1)
			{
				continue;
			}
			if (crTemp.Row2 < g_iJawRow[railType])
			{
				continue;
			}
			if (maxCRSize[i] <= crTemp.Region.size())
			{
				maxCRSize[i] = crTemp.Region.size();
			}
			for (int ii = 0; ii < crTemp.Region.size(); ++ii)
			{
				WaveData& wd = crTemp.Region[ii];
				TPoint point;
				point.Step = wd.step;
				point.Row = wd.row;
				point.Channel = i;
				points.emplace_back(point);

				findEveyChannel[i] = 1;
				sumEveyChannel[i] += 1;

				if (wd.row < ir1)		ir1 = wd.row;
				if (wd.row > ir2)		ir2 = wd.row;
				if (wd.step < iS1)		iS1 = wd.step;
				if (wd.step > iS2)		iS2 = wd.step;

				if (wd.row > rowCH1 && wd.row < rowCH2)	sum10_16++;
				iSumRow += wd.row;
				iSumStep += wd.step;
			}
			sumCc += crTemp.Region.size();
		}
	}

	memset(distri, 0, sizeof(distri));
	for (int i = CH_A1; i < CH_C; ++i)
	{
		for (int k = 0; k < t_cr[i].size(); ++k)
		{
			Connected_Region& crTemp = vCRs[i][t_cr[i][k]];
			if (crTemp.IsIllgeal == 1)
			{
				continue;
			}
			for (int m = 0; m < crTemp.Region.size(); ++m)
			{
				++distri[crTemp.Region[m].step - iS1];
			}
		}
	}

	int maxDis = distri[0];
	int maxIndex = 0;
	for (int i = 1; i < 100; ++i)
	{
		if (distri[i] > maxDis)
		{
			maxDis = distri[i];
			maxIndex = i;
		}
	}

	for (int i = CH_A1; i < CH_C; ++i)
	{
		if (maxCRSize[i] >= 10)
		{
			iHeavyChannel++;
		}
	}

	for (int i = 0; i < CH_C; ++i)
	{
		if (sumEveyChannel[i] >= 10)
		{
			double perc = 0;
			if (i % 2 == 0 && sumEveyChannel[i] + sumEveyChannel[i + 1] > 0)
			{
				perc = pointCountlInJaw[i] / (sumEveyChannel[i] + sumEveyChannel[i + 1]);
				if (perc >= 0.2)
				{
					++iHeavyChannel2;
				}
			}
			else if (i % 2 != 0 && sumEveyChannel[i - 1] + sumEveyChannel[i] > 0)
			{
				perc = pointCountlInJaw[i] / (sumEveyChannel[i - 1] + sumEveyChannel[i]);
				if (perc >= 0.2)
				{
					++iHeavyChannel2;
				}
			}
		}
	}


	for (int i = CH_A1; i < CH_D; ++i)
	{
		if (i < CH_C)
		{
			totalCRCountNoCc += t_cr[i].size();
			for (int j = 0; j < t_cr[i].size(); ++j)
			{
				if (!(vCRs[i][t_cr[i][j]].Row1 > rowCH2 || vCRs[i][t_cr[i][j]].Row2 < rowCH1))
				{
					totalCRCountNoCcInJaw++;
				}
			}
		}
		totalCRCount += t_cr[i].size();
	}

	if (points.size() > 0)
	{
		aveCRSize = 1.0 * (sumAaBb + sumCc) / totalCRCount;
		aveCRSizeNoCc = 1.0 *  sumAaBb / totalCRCountNoCc;

		iSumBigChannel = findEveyChannel[CH_A1] + findEveyChannel[CH_A2] + findEveyChannel[CH_B1] + findEveyChannel[CH_B2] + findEveyChannel[CH_C];
		iSumSmallChannel = findEveyChannel[CH_a1] + findEveyChannel[CH_a2] + findEveyChannel[CH_b1] + findEveyChannel[CH_b2] + findEveyChannel[CH_c];

		uint32_t iBigCount = 0, iSmallCount = 0;
		iAStepBig = 0, iAStepSmall = 0; aveStep = iSumStep / points.size();
		aveRow = iSumRow / points.size();
		for (int k = 0; k < points.size(); ++k)
		{
			s += ((points[k].Step - aveStep) * (points[k].Step - aveStep)) + ((points[k].Row - aveRow) * (points[k].Row - aveRow));
			if (points[k].Channel == CH_A1 || points[k].Channel == CH_B1)
			{
				iBigCount++;
				iAStepBig += points[k].Step;
			}
			else if (points[k].Channel == CH_a1 || points[k].Channel == CH_b1)
			{
				iSmallCount++;
				iAStepSmall += points[k].Step;
			}
		}
		if (iBigCount > 0)		iAStepBig = 1.0 * iAStepBig / iBigCount;
		if (iSmallCount > 0)	iAStepSmall = 1.0 * iAStepSmall / iSmallCount;
		if (sum_ABC > 0)			iAStepBig2 = 1.0 * iAStepBig2 / sum_ABC;
		if (sum_abc > 0)		iAStepSmall2 = 1.0 * iAStepSmall2 / sum_abc;
		iAStepBigNoCc = iAStepBig;
		iAStepSmallNoCc = iAStepSmall;
		ChannelNum = sum;
		percent = 1.0 * sum10_16 / points.size();
	}

	SumAa = sumEveyChannel[CH_A1] + sumEveyChannel[CH_A2] + sumEveyChannel[CH_a1] + sumEveyChannel[CH_a2];
	SumBb = sumEveyChannel[CH_B1] + sumEveyChannel[CH_B2] + sumEveyChannel[CH_b1] + sumEveyChannel[CH_b2];

	sumAB = sumEveyChannel[CH_A1] + sumEveyChannel[CH_A2] + sumEveyChannel[CH_B1] + sumEveyChannel[CH_B2];
	sumab = sumEveyChannel[CH_a1] + sumEveyChannel[CH_a2] + sumEveyChannel[CH_b1] + sumEveyChannel[CH_b2];

	validC = 0; validc = 0;
	if (iAStepBigNoCc >= 20 && iAStepSmallNoCc >= 20)
	{
		int middle = (iAStepBigNoCc + iAStepSmallNoCc) / 2;
		for (int i = 0; i < t_cr[CH_C].size(); ++i)
		{
			CR& tempCR = vCRs[CH_C][t_cr[CH_C][i]];
			if (tempCR.IsIllgeal == 1)
			{
				continue;
			}
			if (tempCR.Step2 < middle - 5 || tempCR.Step1 > middle + 5 || tempCR.Row2 < g_iJawRow[railType] || tempCR.Region.size() < 5)
			{
				continue;
			}
			validC++;
		}

		for (int i = 0; i < t_cr[CH_c].size(); ++i)
		{
			CR& tempCR = vCRs[CH_c][t_cr[CH_c][i]];
			if (tempCR.IsIllgeal == 1)
			{
				continue;
			}
			if (tempCR.Step2 < middle - 5 || tempCR.Step1 > middle + 5 || tempCR.Row2 < g_iJawRow[railType] || tempCR.Region.size() < 5)
			{
				continue;
			}
			validc++;
		}
	}
}

void	GetWaveLen(VCR* vcr, VINT* tcr, Section* pSections, int& aveLenNoCc)
{
	for (int i = 0; i < CH_c; ++i)
	{
		pSections[i].Start = 0x7FFFFFFF;
		pSections[i].End = 0;
		pSections[i].Flag = 0;
		for (int j = 0; j < tcr[i].size(); ++j)
		{
			if (vcr[i][tcr[i][j]].Region.size() < 2 || vcr[i][tcr[i][j]].Step2 - vcr[i][tcr[i][j]].Step1 < 2 || vcr[i][tcr[i][j]].Row2 - vcr[i][tcr[i][j]].Row1 < 1)
			{
				continue;
			}
			pSections[i].Start = min(pSections[i].Start, vcr[i][tcr[i][j]].Step1);
			pSections[i].End = max(pSections[i].End, vcr[i][tcr[i][j]].Step2);
		}
		if (tcr[i].size() > 0)
		{
			pSections[i].Flag = pSections[i].End - pSections[i].Start;
		}
	}

	int sum1 = 0;
	int count1 = 0;
	for (int i = 0; i < CH_C; ++i)
	{
		if (pSections[i].Flag > 0)
		{
			sum1 += pSections[i].Flag;
			count1++;
		}
	}
	if (count1 > 0)
	{
		aveLenNoCc = sum1 / count1;
	}
}


void	GetWaveLen2(VCR* vcr, VINT* tcr, Section* pSections, int& aveLenNoCc)
{
	for (int i = 0; i < CH_c; ++i)
	{
		pSections[i].Start = 0x7FFFFFFF;
		pSections[i].End = 0;
		pSections[i].Flag = 0;
		for (int j = 0; j < tcr[i].size(); ++j)
		{
			pSections[i].Start = min(pSections[i].Start, vcr[i][tcr[i][j]].Step1);
			pSections[i].End = max(pSections[i].End, vcr[i][tcr[i][j]].Step2);
		}
		if (tcr[i].size() > 0)
		{
			pSections[i].Flag = pSections[i].End - pSections[i].Start;
		}
	}

	int sum1 = 0;
	int count1 = 0;
	for (int i = 0; i < CH_C; ++i)
	{
		if (pSections[i].Flag > 0)
		{
			sum1 += pSections[i].Flag;
			count1++;
		}
	}
	if (count1 > 0)
	{
		aveLenNoCc = sum1 / count1;
	}
}

bool    GetStepInBlock2(uint32_t step, VBDB& blocks, int& blockIndex, int& stepIndex)
{
	for (int i = 0; i < blocks.size(); ++i)
	{
		if (blocks[i].IndexL2 <= step && blocks[i].IndexL2 + blocks[i].BlockHead.row >= step)
		{
			blockIndex = i;
			stepIndex = step - blocks[i].IndexL2;
			return true;
		}
	}
	return false;
}

int32_t		GetNearMark(uint32_t beginStep, uint32_t endstep, VPM& vPMs, PM& pm)
{
	for (int i = 0; i < vPMs.size(); ++i)
	{
		if (vPMs[i].Step2 >= beginStep && vPMs[i].Mark <= endstep && (IsJoint(vPMs[i].Mark) || IsSew(vPMs[i].Mark)) && vPMs[i].IsYoloChecked == 0)
		{
			return i;
		}
		if (vPMs[i].Step2 >= endstep)
		{
			break;
		}
	}
	return -1;
}


int32_t	GetMarkedPosition(uint32_t beginStep, VPM& vPMs, int32_t* pstep2)
{
	int endStep = beginStep + 500;
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

//返回厂焊铝热焊下标，-1表示没有
int32_t	GetMarkedPositionIndex(uint32_t beginStep, VPM& vPMs, int32_t* pstep2)
{
	int endStep = beginStep + 500;
	for (int i = vPMs.size() - 1; i >= 0; --i)
	{
		if ((IsJoint(vPMs[i].Mark) || IsSew(vPMs[i].Mark) || vPMs[i].Mark == PM_SELFDEFINE && vPMs[i].Data >= 1 && vPMs[i].Data <= 3) && vPMs[i].Manual == 1 && vPMs[i].IsOverlapped == 0)
		{
			if (vPMs[i].Step2 >= beginStep && vPMs[i].Step2 <= endStep)
			{
				if (pstep2 != nullptr)
				{
					*pstep2 = vPMs[i].Step2;
				}
				return i;
			}
		}
		if (/*vPMs[i].Step2 > g_iEndStep ||*/ vPMs[i].Step2 < g_iBeginStep)
		{
			break;
		}
	}
	return -1;
}

int32_t	GetManualJoint(uint32_t beginStep, VPM& vPMs)
{
	int endStep = beginStep + 500;
	for (int i = 0; i < vPMs.size(); ++i)
	{
		if (IsJoint(vPMs[i].Mark) && vPMs[i].Manual == 1)
		{
			if (vPMs[i].Step2 >= beginStep && vPMs[i].Step2 <= endStep)
			{
				return i;
			}
		}
		if (vPMs[i].Step2 > endStep)
		{
			break;
		}
	}
	return -1;
}

int16_t		GetMarkFromManual(PM& manualPM)
{
	int16_t mark = 0;
	if (manualPM.Mark == PM_JOINT)
	{
		return PM_JOINT2;
	}
	else if (manualPM.Mark == PM_SELFDEFINE)
	{
		if (manualPM.Data == 1)
		{
			return PM_SEW_CH;
		}
		if (manualPM.Data == 2)
		{
			return PM_SEW_LRH;
		}
		if (manualPM.Data == 3)
		{
			return PM_SEW_LIVE;
		}
		return 0;
	}
	return 0;
}

uint32_t	GetLastPos(uint32_t step, std::map<uint32_t, Pos>& vPos, uint32_t& findStep)
{
	uint32_t dt = 1000000;
	for (auto itr = vPos.begin(); itr != vPos.end(); ++itr)
	{
		int dtAbs = Abs(itr->first - step);
		if (dtAbs < dt)
		{
			dt = dtAbs;
			findStep = itr->first;
		}
		if (itr->first > step + 100)
		{
			break;
		}
	}
	return dt;
}

int32_t GetLenWithSlope(uint32_t step, PM& slope)
{
	int32_t len = 0x7FFFFFFF;
	for (auto itr = g_vSlopes.begin(); itr != g_vSlopes.end(); ++itr)
	{
		int dLen = Abs((int32_t)(itr->first - step));
		if (dLen < len)
		{
			len = dLen;
			slope = itr->second;
		}
		if (itr->first > step)
		{
			break;
		}
	}
	return len;
}

void GetLoseStep(int centerStep, VBDB& blocks, int& step1, int& step2)
{
	std::vector<B_Step> vSteps;
	int s1 = centerStep - 20;
	s1 = max(s1, 0);
	int s2 = centerStep + 20;
	Pos pos1 = FindStepInBlock(s1, g_vBlockHeads, 0);
	bool isOver = false;
	for (int i = pos1.Block; i < blocks.size() && !isOver; ++i)
	{
		for (int j = 0; j < blocks[i].vBStepDatas.size(); ++j)
		{
			if (blocks[i].vBStepDatas[j].Step >= s1 && blocks[i].vBStepDatas[j].Step <= s2)
			{
				vSteps.emplace_back(blocks[i].vBStepDatas[j]);
			}
			else if (blocks[i].vBStepDatas[j].Step > s2)
			{
				isOver = true;
				break;
			}
		}
	}

	VINT vF, vG;
	for (int i = 0; i < vSteps.size(); ++i)
	{
		int16_t iFrow = GetFRow(vSteps[i]);
		int16_t iGrow = GetGRow(vSteps[i]);
		if (iFrow < 0)
		{
			vF.emplace_back(vSteps[i].Step);
		}
		if (iGrow < 0)
		{
			vG.emplace_back(vSteps[i].Step);
		}
	}

	if (vF.size() == 0 || vG.size() == 0)
	{
		step1 = -1;
		step2 = -1;
		return;
	}

	std::vector<FullMeter> vFMF;
	FullMeter fm;
	fm.Block1 = 0; fm.Block2 = 0; fm.Channel = 0;
	int lastStep = -1;
	for (int i = 0; i < vF.size(); ++i)
	{
		if (fm.Block2 == 0 || lastStep == -1)
		{
			fm.Block1 = vF[i];
			fm.Block2 = 1;
		}
		else if (vF[i] - lastStep == 1)
		{
			fm.Block2++;
		}
		else
		{
			vFMF.emplace_back(fm);
			fm.Block2 = 0;
		}
		if (i == vF.size() - 1)
		{
			vFMF.emplace_back(fm);
		}
		lastStep = vF[i];
	}

	std::vector<FullMeter> vFMG;
	fm.Block1 = 0; fm.Block2 = 0; fm.Channel = 0;
	lastStep = -1;
	for (int i = 0; i < vG.size(); ++i)
	{
		if (fm.Block2 == 0 || lastStep == -1)
		{
			fm.Block1 = vG[i];
			fm.Block2 = 1;
		}
		else if (vG[i] - lastStep == 1)
		{
			fm.Block2++;
		}
		else
		{
			vFMG.emplace_back(fm);
			fm.Block2 = 0;
		}
		if (i == vG.size() - 1)
		{
			vFMG.emplace_back(fm);
		}
		lastStep = vG[i];
	}

	int indexF = -1;
	for (int i = 0; i < vFMF.size(); ++i)
	{
		if (vFMF[i].Block1 <= centerStep && vFMF[i].Block1 + vFMF[i].Block2 >= centerStep)
		{
			indexF = i;
			break;
		}
	}

	int indexG = -1;
	for (int i = 0; i < vFMG.size(); ++i)
	{
		if (vFMG[i].Block1 <= centerStep && vFMG[i].Block1 + vFMG[i].Block2 >= centerStep)
		{
			indexG = i;
			break;
		}
	}
	if (indexF >= 0 && indexG >= 0)
	{
		GetOverlappedStep(vFMF[indexF].Block1, vFMF[indexF].Block1 + vFMF[indexF].Block2, vFMG[indexG].Block1, vFMG[indexG].Block1 + vFMG[indexG].Block2, step1, step2);
	}
	else
	{
		step1 = -1;
		step2 = -1;
	}
}

void		IsFGLose(int step1, int step2, VBDB& blocks, VCR* vCRs, int iFRow, uint8_t& iLoseF, VINT& t_crF, uint8_t& iLoseG, VINT& t_crG, int& iLose_Right, int& iLose_Left)
{
	t_crF.clear(); t_crG.clear();
	iLose_Left = -1;	iLose_Right = -1;
	//略微考虑拼图不好的情况
	iLoseF = GetCR(CH_F, step1, iFRow - 10, step2, iFRow + 10, blocks, vCRs[CH_F], t_crF, -1, 2);
	iLoseG = GetCR(CH_G, step1, iFRow - 10, step2, iFRow + 10, blocks, vCRs[CH_G], t_crG, -1, 2);
	iLoseF = RemoveCRByLose(vCRs[CH_F], t_crF, 1);
	iLoseG = RemoveCRByLose(vCRs[CH_G], t_crG, 1);
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
			uint8_t bLoseG = GetCR(CH_G, x1, iFRow - 5, x2, iFRow + 10, blocks, vCRs[CH_G], t_crG2, -1, 2);
			for (int j = 0; j < t_crG2.size(); ++j)
			{
				if (vCRs[CH_G][t_crG2[j]].Step1 > step2 || vCRs[CH_G][t_crG2[j]].Step2 < step1)
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
}

uint32_t	ParsePosition(F_HEAD& head, VBDB& blocks, VCR* vCRs, CR& cr, int index, int16_t iFDesiredRow, uint8_t railType, VINT* t_cr, PM& pm, int& iAStepBig, int &iAStepSmall, VPM& vPMs, VPM& vPMs2)
{
	//S1统计参数计算
	//轨颚范围行高
	int32_t rowCH1 = g_iJawRow[railType] - 3, rowCH2 = g_iJawRow[railType] + 3, iRowH = g_iJawRow[railType] << 1;
	int iFindStepRight = cr.Step1 - 50;
	int iFindStepLeft = cr.Step1 + 50;
	for (int i = CH_A1; i < CH_C; ++i)
	{
		GetCR(i, iFindStepRight, 0, iFindStepLeft, iRowH, blocks, vCRs[i], t_cr[i], -1, 2, false);
	}
	for (int i = CH_C; i < CH_D; ++i)
	{
		GetCR(i, iFindStepRight, 0, iFindStepLeft, iRowH, blocks, vCRs[i], t_cr[i], -1, 1, false);
	}
	//S1.1获取各通道当前连通域前后100步进内的出波信息
	PositionFeature pf(blocks, vCRs, t_cr, cr.Step1 - 100, cr.Step2 + 100, rowCH1, rowCH2, railType);
	if (pf.iSumBigChannel + pf.iSumSmallChannel <= 20 && pf.maxSize < 5)
	{
		for (int i = CH_A1; i < CH_D; ++i)
		{
			for (int j = t_cr[i].size() - 1; j >= 0; --j)
			{
				if (IsCRDirty_ABC(blocks, vCRs, vCRs[i][t_cr[i][j]]))
				{
					t_cr[i].erase(t_cr[i].begin() + j);
				}
			}
		}

		pf = PositionFeature(blocks, vCRs, t_cr, cr.Step1 - 100, cr.Step2 + 100, rowCH1, rowCH2, railType);
	}

	if (pf.iSumBigChannel + pf.iSumSmallChannel == 0)
	{
		iAStepBig = pf.iS2;
		iAStepSmall = pf.iS1;
		Pos pos = FindStepInBlock(pf.iS1, g_vBlockHeads, 0);
		pm.Block = pos.Block;
		pm.BeginStep = iAStepSmall;
		pm.Step = pos.Step;
		pm.Step2 = pos.Step2;
		pm.Mark = 0;
		pm.Length = pf.iS2 - pf.iS1;
		for (int k = 0; k < 16; ++k)
		{
			pm.Num[k] = pf.sumEveyChannel[k];
		}
		return 0;
	}

	int maxDis = distri[0];
	int maxIndex = 0;
	for (int j = 1; j < 100; ++j)
	{
		if (distri[j] > maxDis)
		{
			maxDis = distri[j];
			maxIndex = j;
		}
	}
	int stepBegin = maxIndex + pf.iS1 - 7, stepEnd = maxIndex + pf.iS2 + 7;
	memset(&pm, 0, sizeof(pm));
	pm.Block = cr.Block;
	pm.AStep = pf.aveStep;
	pm.ARow = pf.aveRow;
	for (int k = 0; k < 16; ++k)
	{
		pm.Num[k] = pf.sumEveyChannel[k];
	}
	pm.Size = pf.sumAaBb + pf.sumCc;
	pm.BeginStep = pf.iS1;
	pm.Length = pf.iS2 - pf.iS1;
	pm.Height = pf.ir2 - pf.ir1;
	pm.ChannelNum = pf.ChannelNum;


	uint32_t iBigCount = 0, iSmallCount = 0;
	iAStepBig = pf.iAStepBig;   //左侧出波平均步进
	iAStepSmall = pf.iAStepSmall;   //右侧出波平均步进
	pm.Step = cr.Step + pm.AStep - cr.Step1;
	pm.Step2 = pm.AStep;
	if (pm.Step < 0)
	{
		Pos pmPos = FindStepInBlock(pm.Step2, blocks, 0);
		pm.Block = pmPos.Block;
		pm.Step = pmPos.Step;
	}

	int iStep1 = stepBegin, iStep2 = stepEnd;
	if (iStep2 - iStep1 >= 30 && pf.iAStepBigNoCc > 0 && pf.iAStepSmallNoCc > 0)
	{
		iStep1 = pf.iAStepBigNoCc < pf.iAStepSmallNoCc ? pf.iAStepBigNoCc - 3 : pf.iAStepSmallNoCc - 3;
		iStep2 = pf.iAStepBigNoCc >= pf.iAStepSmallNoCc ? pf.iAStepBigNoCc + 3 : pf.iAStepSmallNoCc + 3;
	}

	//S1.2 检测底部失波（若有失波：F失波，记iLoseF = 1; G失波，记iLoseG = 1）
	VINT t_crF, t_crG;
	uint8_t iLoseF, iLoseG;
	int iLose_Right, iLose_Left;
	IsFGLose(iStep1, iStep2, blocks, vCRs, iFDesiredRow, iLoseF, t_crF, iLoseG, t_crG, iLose_Right, iLose_Left);

	uint32_t iSumBigChannel = pf.sum_ABC;
	uint32_t iSumSmallChannel = pf.sum_abc;
	int iBigChannelwaveCountNoCc = pf.sumAB;//AB
	int iSmallChannelwaveCountNoCc = pf.sumab;//ab
	double percent = pf.percent;
	pm.Percent = percent;
	double percent2 = 1.0 * (pm.Num[CH_C] + pm.Num[CH_c]) / pm.Size;

#pragma region 是否有焊缝凸台
	bool isPlatForm = false;
	if (pm.Percent >= 0.3 && pm.Size >= 50)
	{
		int stepScrewL = dScrewDistance[railType][2];
		VINT crG[6];
		for (int i = 0; i < 6; ++i)
		{
			GetCR(CH_G, stepScrewL - 15, stepScrewL, stepScrewL + 15, stepScrewL, blocks, vCRs[CH_G], crG[i]);
		}

	}
	int step_small = iAStepBig < iAStepSmall ? iAStepBig : iAStepSmall;
	int step_big = iAStepBig >= iAStepSmall ? iAStepBig : iAStepSmall;
	int step_middle = (step_small + step_big) >> 1;
	if (step_small > 20)
	{
		//Pos pos1 = FindStepInBlock(step_small - 10, g_vBlockHeads, 0);
		//Pos pos2 = FindStepInBlock(step_big + 10, g_vBlockHeads, 0);
		//Pos pos = FindStepInBlock(step_middle, g_vBlockHeads, 0);
		//if (!(pos1.Block == 0 && pos1.Step == 0) && !(pos2.Block == 0 && pos2.Step == 0))
		//{
		//	double tempRowRight = blocks[pos1.Block - g_iBeginBlock].vBStepDatas[pos1.Step].FRow + blocks[pos1.Block - g_iBeginBlock].vBStepDatas[pos1.Step].GRow;
		//	double tempRowLeft = blocks[pos2.Block - g_iBeginBlock].vBStepDatas[pos2.Step].FRow + blocks[pos2.Block - g_iBeginBlock].vBStepDatas[pos2.Step].GRow;
		//	double tempRow = blocks[pos.Block - g_iBeginBlock].vBStepDatas[pos.Step].FRow + blocks[pos.Block - g_iBeginBlock].vBStepDatas[pos.Step].GRow;
		//	if (tempRow > tempRowLeft && tempRow > tempRowRight)
		//	{
		//		isPlatForm = true;
		//	}
		//}
	}
#pragma endregion

	//**S1.3 位置标初步判定
	pm.BiggerStep = pf.iAStepBigNoCc;
	pm.LessStep = pf.iAStepSmallNoCc;

	//寻找螺孔进行辅助定位
	if (
		(pm.ChannelNum >= 7 && pm.Size >= 60 && pf.maxSize >= 10 && (iLoseF && iLoseG))
		&& (pf.sumCc >= 20 || iBigChannelwaveCountNoCc >= 20 && iSmallChannelwaveCountNoCc >= 20 && pf.sumCc > 15)
		&& (iAStepBig <= iAStepSmall - 5 || iAStepBig <= iAStepSmall + 3 && pm.Num[CH_C] >= 10 && pm.Num[CH_c] >= 10 && pf.maxCRSize[CH_C] >= 12 && pf.maxCRSize[CH_c] >= 12)
		&& (pf.maxCRSize[CH_C] >= 8 || pf.maxCRSize[CH_c] >= 8)
		)//完整接头
	{
		if (pm.Percent >= 0.15)   //若pm.Percent >= 0.15，判定为接头
		{
			pm.Mark = PM_JOINT2;
			if (!(iLose_Right > pf.iAStepSmallNoCc || iLose_Left < pf.iAStepBigNoCc))   //判定是否为半边接头
			{
				iAStepBig = iLose_Right;
				iAStepSmall = iLose_Left;
			}
			if (iSmallChannelwaveCountNoCc < 20)   //若pf.sumab < 20，判定为右半边接头
			{
				pm.Mark = PM_JOINT_RIGHT;
				pm.IsHalf = 1;
				iAStepBig = pf.iAStepBig2 + 7;
				iAStepSmall = pf.iAStepBig2;
			}
			else if (iBigChannelwaveCountNoCc < 20)   //若pf.sumAB< 20，判定为左半边接头
			{
				pm.Mark = PM_JOINT_LEFT;
				pm.IsHalf = 2;
				iAStepBig = pf.iAStepSmall2;
				iAStepSmall = pf.iAStepSmall2 - 7;
			}
		}
	}
	else if (pm.ChannelNum >= 5 && pm.Size >= 60 && pm.Num[CH_C] >= 10 && pm.Num[CH_c] >= 10 && pf.maxSize >= 10 && (iLoseF && iLoseG) &&
		(iAStepBig <= iAStepSmall || iAStepBig <= iAStepSmall + 3 && pm.Num[CH_C] >= 10 && pm.Num[CH_c] >= 10)
		&& pf.maxSize >= 5
		//&& !isPlatForm
		)
	{
		if (pm.Percent >= 0.3 && pf.maxCRSize[CH_C] >= 8 && pf.maxCRSize[CH_c] >= 8)
		{
			pm.Mark = PM_JOINT2;   //判定为接头
			iAStepBig = iLose_Right;
			iAStepSmall = iLose_Left;
		}
	}
	else if (pm.ChannelNum >= 5 && pm.Size >= 60 && iBigChannelwaveCountNoCc >= 40 && iBigChannelwaveCountNoCc >= iSmallChannelwaveCountNoCc * 2 && pf.maxSize >= 10 && (iLoseF && iLoseG) && pm.Num[CH_C] >= 10 && pf.maxSize >= 5)//开始半边接头
	{
		if (pm.Percent > 0.15)
		{
			VINT vTempF;
			GetCR(CH_F, pf.iS1, iFDesiredRow - 3, pf.iS2 + 100, iFDesiredRow + 10, blocks, vCRs[CH_F], vTempF, -1, 500);   //在pf.iS1 - 100, pf.iS2范围内,寻找F通道失波，若有失波步进大于500步进
			if (vTempF.size() > 0)
			{
				pm.Mark = PM_JOINT_RIGHT;   //判定为：右半边接头
				pm.IsHalf = 1;
				iAStepSmall = vCRs[CH_F][vTempF[0]].Step1;
				iAStepBig = iAStepSmall + 10;

#ifdef _DEBUG
				Pos tempPos = FindStepInBlock(iAStepSmall, g_vBlockHeads, 0);
				Pos tempPos2 = FindStepInBlock(iAStepBig, g_vBlockHeads, 0);
				pm.Mark = PM_JOINT_RIGHT;
#endif // _DEBUG
			}
		}
	}
	else if (pm.ChannelNum >= 5 && pm.Size >= 60 && iSmallChannelwaveCountNoCc >= 40 && iSmallChannelwaveCountNoCc >= 2 * iBigChannelwaveCountNoCc && pf.maxSize >= 10 && (iLoseF && iLoseG) && pm.Num[CH_c] >= 10 && pf.maxSize >= 5)//结束半边接头
	{
		if (pm.Percent > 0.15)
		{
			VINT vTempF;
			GetCR(CH_F, pf.iS1 - 100, iFDesiredRow - 3, pf.iS2, iFDesiredRow + 10, blocks, vCRs[CH_F], vTempF, -1, 500);
			if (vTempF.size() > 0)
			{
				pm.Mark = PM_JOINT_LEFT;   //判定为：左半边接头
				pm.IsHalf = 2;
				iAStepBig = vCRs[CH_F][vTempF[0]].Step2;
				iAStepSmall = iAStepBig - 10;
			}
		}
	}
	else if (pm.ChannelNum >= 8 && pm.Size >= 60 && iBigChannelwaveCountNoCc >= 0.667 * iSmallChannelwaveCountNoCc && iBigChannelwaveCountNoCc <= 1.5 * iSmallChannelwaveCountNoCc && pm.Num[CH_C] + pm.Num[CH_c] >= 10 && pf.maxSize >= 5)
	{
		if (pm.Percent >= 0.4 && pf.iAStepBigNoCc > pf.iAStepSmallNoCc)
		{
			pm.Mark = PM_SEW_LRH;   //判定为铝热焊
		}
		else if (pm.Percent > 0.2 && pf.iAStepBigNoCc <= pf.iAStepSmallNoCc)
		{
			if (pm.Percent >= 0.4)
			{
				pm.Mark = PM_SEW_LRH;   //判定为铝热焊
				if (iLose_Right > 0)
				{
					iAStepBig = pf.iAStepSmallNoCc;
					iAStepSmall = pf.iAStepBigNoCc;
				}
			}
			else
			{
				pm.Mark = PM_JOINT2;   //判定为接头
				if (iLose_Right > 0)
				{
					iAStepBig = iLose_Right;
					iAStepSmall = iLose_Left;
				}
			}
		}
	}
	else if (pm.ChannelNum >= 5 && pm.Size >= 50 && pf.sumCc < 35 && pf.iAStepBigNoCc > pf.iAStepSmallNoCc)
	{
		if (pm.Percent >= 0.6 && pf.totalCRCountNoCc <= 6)
		{
			pm.Mark = PM_SEW_CH;   //判定为厂焊
			SetUsedFlag(vCRs[CH_F], t_crF, 1);
			SetUsedFlag(vCRs[CH_G], t_crG, 1);
		}
		else  if (pm.Percent >= 0.4 && pm.Size >= 80 && pf.maxSize >= 8 && pf.iHeavyChannel >= 2 && iSmallChannelwaveCountNoCc >= 20 && iBigChannelwaveCountNoCc >= 20
			&& pf.SumAa >= 20 && pf.SumBb >= 20)
		{
			pm.Mark = PM_SEW_LRH;   //判定为铝热焊
			SetUsedFlag(vCRs[CH_F], t_crF, 1);
			SetUsedFlag(vCRs[CH_G], t_crG, 1);
		}
		else  if (pm.Percent >= 0.4 && pm.Size < 80 && pf.maxSize < 10 && pf.totalCRCountNoCc <= 6)
		{
			pm.Mark = PM_SEW_CH;   //判定为厂焊
			SetUsedFlag(vCRs[CH_F], t_crF, 1);
			SetUsedFlag(vCRs[CH_G], t_crG, 1);
		}
		else if (pm.Percent >= 0.45 && pf.maxSize >= 5 && pf.iHeavyChannel >= 2 && iSmallChannelwaveCountNoCc >= 20 && iBigChannelwaveCountNoCc >= 20
			&& pf.SumAa >= 20 && pf.SumBb >= 20)
		{
			pm.Mark = PM_SEW_LRH;   //判定为铝热焊
			SetUsedFlag(vCRs[CH_F], t_crF, 1);
			SetUsedFlag(vCRs[CH_G], t_crG, 1);
		}
		else if (pm.Percent >= 0.45  && pf.maxSize >= 5 && pf.iHeavyChannel2 >= 2 && iSmallChannelwaveCountNoCc >= 20 && iBigChannelwaveCountNoCc >= 20
			&& pf.SumAa >= 20 && pf.SumBb >= 20)
		{
			pm.Mark = PM_SEW_LRH;   //判定为铝热焊
			SetUsedFlag(vCRs[CH_F], t_crF, 1);
			SetUsedFlag(vCRs[CH_G], t_crG, 1);
		}
		//190102Q0812DLN_0001_16S1015. 107
		else if (pm.Percent >= 0.3 && pf.maxSize >= 8 && pf.iHeavyChannel >= 2 && iSmallChannelwaveCountNoCc >= 20 && iBigChannelwaveCountNoCc >= 20 &&
			(pf.SumAa >= 20 || pf.SumAa >= 15 && (pf.sumEveyChannel[CH_A1] + pf.sumEveyChannel[CH_A2]) * (pf.sumEveyChannel[CH_a1] + pf.sumEveyChannel[CH_a2]) == 0) &&
			(pf.SumBb >= 20 || pf.SumBb >= 15 && (pf.sumEveyChannel[CH_B1] + pf.sumEveyChannel[CH_B2]) * (pf.sumEveyChannel[CH_b1] + pf.sumEveyChannel[CH_b2]) == 0)
			)
		{
			pm.Mark = PM_SEW_LRH;   //判定为铝热焊
			SetUsedFlag(vCRs[CH_F], t_crF, 1);
			SetUsedFlag(vCRs[CH_G], t_crG, 1);
		}
	}

	//寻找附近的其他通道出波，若在正确位置有对应通道出波，判定为厂焊
	else if (percent > 0.5 && cr.Row1 >= rowCH1 && (cr.Row2 <= rowCH2 || cr.Channel >= CH_C))
	{
		if (cr.Channel == CH_C)
		{
			VINT crA, crB, cra, crb, crc;
			bool bFindA = GetCR(CH_A1, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_A1], crA, -1, 1, true);
			bool bFindB = GetCR(CH_B1, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_B1], crB, -1, 1, true);
			bool bFinda = GetCR(CH_a1, cr.Step1 - 20, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_a1], cra, -1, 1, true);
			bool bFindb = GetCR(CH_b1, cr.Step1 - 20, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_b1], crb, -1, 1, true);
			bool bFindc = GetCR(CH_c, cr.Step1 - 20, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_c], crc, -1, 1, true);
			if (bFindA || bFindB || bFinda || bFindb || bFindc)
			{
				pm.Mark = PM_SEW_CH;   //判定为厂焊
			}
		}
		else if (cr.Channel == CH_c)
		{
			VINT crA, crB, crC, cra, crb;
			bool bFindA = GetCR(CH_A1, cr.Step2 + 8, rowCH1, cr.Step2 + 20, rowCH2, blocks, vCRs[CH_A1], crA, -1, 1, true);
			bool bFindB = GetCR(CH_B1, cr.Step1 + 8, rowCH1, cr.Step2 + 20, rowCH2, blocks, vCRs[CH_B1], crB, -1, 1, true);
			bool bFindC = GetCR(CH_C, cr.Step1 + 8, rowCH1, cr.Step2 + 20, rowCH2, blocks, vCRs[CH_C], crC, -1, 1, true);
			bool bFinda = GetCR(CH_a1, cr.Step2 - 2, rowCH1, cr.Step2 + 6, rowCH2, blocks, vCRs[CH_a1], cra, -1, 1, true);
			bool bFindb = GetCR(CH_b1, cr.Step2 - 2, rowCH1, cr.Step2 + 6, rowCH2, blocks, vCRs[CH_b1], crb, -1, 1, true);
			if (bFindA || bFindB || bFindC || bFinda || bFindb)
			{
				pm.Mark = PM_SEW_CH;   //判定为厂焊
			}
		}
		else if (cr.Channel == CH_A1)
		{
			VINT crB, crC, cra, crb, crc;
			bool bFindB = GetCR(CH_B1, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_B1], crB, -1, 1, true);
			bool bFindC = GetCR(CH_C, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_C], crC, -1, 1, true);
			bool bFinda = GetCR(CH_a1, cr.Step1 - 15, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_a1], cra, -1, 1, true);
			bool bFindb = GetCR(CH_b1, cr.Step1 - 15, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_b1], crb, -1, 1, true);
			bool bFindc = GetCR(CH_c, cr.Step1 - 15, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_c], crc, -1, 1, true);
			if (bFindB || bFindC || bFinda || bFindb || bFindc)
			{
				pm.Mark = PM_SEW_CH;   //判定为厂焊
			}
		}
		else if (cr.Channel == CH_a1)
		{
			VINT crA, crB, crC, crb, crc;
			bool bFindA = GetCR(CH_A1, cr.Step1 + 8, rowCH1, cr.Step2 + 15, rowCH2, blocks, vCRs[CH_A1], crA, -1, 1, true);
			bool bFindB = GetCR(CH_B1, cr.Step1 + 8, rowCH1, cr.Step2 + 15, rowCH2, blocks, vCRs[CH_B1], crB, -1, 1, true);
			bool bFindC = GetCR(CH_C, cr.Step1 + 8, rowCH1, cr.Step2 + 15, rowCH2, blocks, vCRs[CH_C], crC, -1, 1, true);
			bool bFindb = GetCR(CH_b1, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_b1], crb, -1, 1, true);
			bool bFindc = GetCR(CH_c, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_c], crc, -1, 1, true);
			if (bFindA || bFindB || bFindC || bFindb || bFindc)
			{
				pm.Mark = PM_SEW_CH;   //判定为厂焊
			}
		}
		else if (cr.Channel == CH_B1)
		{
			VINT crA, crC, cra, crb, crc;
			bool bFindA = GetCR(CH_A1, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_A1], crA, -1, 1, true);
			bool bFindC = GetCR(CH_C, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_C], crC, -1, 1, true);
			bool bFinda = GetCR(CH_a1, cr.Step1 - 15, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_a1], cra, -1, 1, true);
			bool bFindb = GetCR(CH_b1, cr.Step1 - 15, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_b1], crb, -1, 1, true);
			bool bFindc = GetCR(CH_c, cr.Step1 - 15, rowCH1, cr.Step2 - 8, rowCH2, blocks, vCRs[CH_c], crc, -1, 1, true);
			if (bFindA || bFindC || bFinda || bFindb || bFindc)
			{
				pm.Mark = PM_SEW_CH;   //判定为厂焊
			}
		}
		else if (cr.Channel == CH_b1)
		{
			VINT crA, crB, crC, cra, crc;
			bool bFindA = GetCR(CH_A1, cr.Step1 + 8, rowCH1, cr.Step2 + 15, rowCH2, blocks, vCRs[CH_A1], crA, -1, 1, true);
			bool bFindB = GetCR(CH_B1, cr.Step1 + 8, rowCH1, cr.Step2 + 15, rowCH2, blocks, vCRs[CH_B1], crB, -1, 1, true);
			bool bFindC = GetCR(CH_C, cr.Step1 + 8, rowCH1, cr.Step2 + 15, rowCH2, blocks, vCRs[CH_C], crC, -1, 1, true);
			bool bFinda = GetCR(CH_a1, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_a1], cra, -1, 1, true);
			bool bFindc = GetCR(CH_c, cr.Step1 - 2, rowCH1, cr.Step2 + 2, rowCH2, blocks, vCRs[CH_c], crc, -1, 1, true);
			if (bFindA || bFindB || bFindC || bFinda || bFindc)
			{
				pm.Mark = PM_SEW_CH;   //判定为厂焊
			}
		}
	}

	/*
	1:接头Cc出波验证
	2:道岔失波验证
	3:变坡点验证
	4:人工标记辅助校验
	*/

	//S2 校验Cc通道
	int isManual2 = 0;
	int iBigC = 0, iBigc = 0, ivalidC = 0, ivalidc = 0;
	if (percent > 0.15 && pm.ChannelNum >= 3 && pm.Size >= 80 && iLoseF && iLoseG)
	{
		//S2.1 校验c通道出波
		if (t_cr[CH_c].size() >= 2)
		{
			//S2.1.1 统计c不少于5个点出波连通域个数iBigc
			for (int i = 0; i < t_cr[CH_c].size(); ++i)
			{
				if (vCRs[CH_c][t_cr[CH_c][i]].Region.size() >= 5)
				{
					iBigc++;
				}
				if (vCRs[CH_c][t_cr[CH_c][i]].Region.size() >= 5 && vCRs[CH_c][t_cr[CH_c][i]].Row1 < g_iJawRow[railType] && vCRs[CH_c][t_cr[CH_c][i]].Row2 >= rowCH2)
				{
					ivalidc++;
				}
			}
			//S2.1.2 判定是否为接头出波
			if (iBigc >= 2 && (pf.iAStepBigNoCc <= pf.iAStepSmallNoCc || pf.sumAB <= 10 || pf.sumab <= 10))
			{
				for (int i = 0; i < t_cr[CH_c].size(); ++i)
				{
					if (vCRs[CH_c][t_cr[CH_c][i]].Step2 < (pf.iAStepBigNoCc + pf.iAStepSmallNoCc) / 2 && vCRs[CH_c][t_cr[CH_c][i]].Row1 <= g_iJawRow[railType] && vCRs[CH_c][t_cr[CH_c][i]].Row2 >= rowCH2)
					{
						pm.Mark = PM_JOINT2;   //判定为接头，位置标人工判定级别为2
						if (iLose_Left - iLose_Right <= 30)
						{
							iAStepBig = iLose_Left;
							iAStepSmall = iLose_Right;
						}
						isManual2 |= 1;
						break;
					}
				}
			}
			else if (ivalidc >= 2 && pf.iAStepBigNoCc <= pf.iAStepSmallNoCc + 5)
			{
				pm.Mark = PM_JOINT2;
				if (iLose_Left - iLose_Right <= 30)
				{
					iAStepBig = iLose_Left;
					iAStepSmall = iLose_Right;
				}
				isManual2 |= 1;
			}
		}
		//S2.2 校验C通道出波
		if (t_cr[CH_C].size() >= 2)
		{
			//S2.2.1 统计C不少于5个点出波连通域个数iBigC
			for (int i = 0; i < t_cr[CH_C].size(); ++i)
			{
				if (vCRs[CH_C][t_cr[CH_C][i]].Region.size() >= 5)
				{
					iBigC++;
				}
				if (vCRs[CH_C][t_cr[CH_C][i]].Region.size() >= 5 && vCRs[CH_C][t_cr[CH_C][i]].Row1 < g_iJawRow[railType] && vCRs[CH_C][t_cr[CH_C][i]].Row2 >= rowCH2)
				{
					ivalidC++;
				}
			}
			//S2.2.2 判定是否为接头出波
			if (iBigC >= 2 && (pf.iAStepBigNoCc <= pf.iAStepSmallNoCc || pf.sumAB <= 10 || pf.sumab <= 10))
			{
				for (int i = 0; i < t_cr[CH_C].size(); ++i)
				{
					if (vCRs[CH_C][t_cr[CH_C][i]].Step1 > (pf.iAStepBigNoCc + pf.iAStepSmallNoCc) / 2 && vCRs[CH_C][t_cr[CH_C][i]].Row1 <= g_iJawRow[railType] && vCRs[CH_C][t_cr[CH_C][i]].Row2 >= rowCH2)
					{
						pm.Mark = PM_JOINT2;   //判定为接头，位置标人工判定级别为2
						if (iLose_Left - iLose_Right <= 30)
						{
							iAStepBig = iLose_Left;
							iAStepSmall = iLose_Right;
						}
						isManual2 |= 1;
						break;
					}
				}
			}
			else if (ivalidC >= 2 && pf.iAStepBigNoCc <= pf.iAStepSmallNoCc + 5)
			{
				pm.Mark = PM_JOINT2;
				if (iLose_Left - iLose_Right <= 30)
				{
					iAStepBig = iLose_Left;
					iAStepSmall = iLose_Right;
				}
				isManual2 |= 1;
			}
		}

		//？？？
		if (iBigC > 0 && iBigc > 0 && iBigC + iBigc >= 3 && ivalidC + ivalidc > 0)
		{
			pm.Mark = PM_JOINT2;
			if (iLose_Left - iLose_Right <= 30)
			{
				iAStepBig = iLose_Left;
				iAStepSmall = iLose_Right;
			}
			isManual2 |= 1;
		}
	}

	//S3 接头再校验
	if (IsJoint(pm.Mark))
	{
		bool bOK = false;
		//S3.1 调整失波参数
		if (iLose_Right > iLose_Left)
		{
			int t = iLose_Left;
			iLose_Left = iLose_Right;
			iLose_Right = t;
		}
		//S3.2 FG失波辅助判定
		if (iLoseF == 0 || iLoseG == 0)
		{
			//且S3.2.1: 出波点数大于200且Cc出波连通域总数不大于2，判定为铝热焊
			if (pm.Size >= 200 && t_cr[CH_C].size() + t_cr[CH_c].size() <= 2)
			{
				pm.Mark = PM_SEW_LRH;   //判定为铝热焊
				iAStepBig = pf.iAStepBig2;
				iAStepSmall = pf.iAStepSmall2;
			}
			//若S3.2.2: Cc出波点数之和小于10，判定为非位置标
			else if (pm.Num[CH_C] + pm.Num[CH_c] < 10)
			{
				pm.Mark = 0;   //判定为非位置标
				return 0;
			}
		}
		else if (iLose_Right <= 0/* || iLose_Left - iLose_Right < 3*/)
		{
			iLose_Left = vCRs[CH_F][t_crF[0]].Step2;
			iLose_Right = vCRs[CH_F][t_crF[0]].Step1;
		}
		//S3.3 调整出波参数
		if (pm.Mark == PM_JOINT2)
		{
			if (!(iLose_Right > pf.iAStepSmallNoCc || iLose_Left < pf.iAStepBigNoCc))
			{
				iAStepBig = iLose_Right;
				iAStepSmall = iLose_Left;
			}
		}
	}
	
	//S5 根据焊筋宽度再次校验（非焊缝则跳过）
	if (pm.Mark == PM_SEW_LRH || pm.Mark == PM_SEW_CH)
	{
		int deltStep = iAStepBig - iAStepSmall;
		if (deltStep >= 33)
		{
			pm.Mark = 0;   //判定为非位置标
		}
		else if (deltStep >= 10 && deltStep <= 32)
		{
			if (pm.ChannelNum >= 5)
			{
				if (
					(pf.iHeavyChannel >= 2)
					|| (pf.iHeavyChannel2 >= 2 && pm.Percent >= 0.45)
					|| pm.Percent >= 0.5 && pm.Size >= 40
					)
				{
					pm.Mark = PM_SEW_LRH;   //判定为铝热焊
				}
				else if (pm.Percent > 0.5 && pf.iHeavyChannel2 >= 1)
				{
					pm.Mark = PM_SEW_LRH;   //判定为铝热焊
				}
			}
			else if (pm.Size >= 30)
			{
				pm.Mark = 0;   //判定为非位置标
			}
		}
		else if (deltStep >= 7 && pm.Size <= 60)
		{
			pm.Mark = PM_SEW_CH;   //判定为厂焊
		}
		else if (deltStep <= 2 && pm.Size < 100)
		{
			if (pm.Percent < 0.5 || pm.ChannelNum < 4 || pm.Length > 30)
			{
				pm.Mark = 0;      //判定为非位置标
			}
		}
	}

	//S6 接头补充判定
	if (!IsJoint(pm.Mark) && pm.ChannelNum >= 3 && iLoseF && iLoseG)//ABc必须有 若非接头且出波通道不小于3且FG同时失波
	{
		for (int i = 0; i < g_vReturnSteps.size(); ++i)
		{
			if (g_vReturnSteps[i] - cr.Step1 >= 0 && g_vReturnSteps[i] - cr.Step1 <= 250)
			{
				int n1 = pm.Num[CH_A1] + pm.Num[CH_A2] + pm.Num[CH_B1] + pm.Num[CH_B2] + pm.Num[CH_c];
				int n2 = pm.Num[CH_a1] + pm.Num[CH_a2] + pm.Num[CH_b1] + pm.Num[CH_b2] + pm.Num[CH_C];
				if (pm.Size >= 50 && n1 >= 1.5 * n2 && n1 >= 40 && pm.Num[CH_A1] + pm.Num[CH_A2] >= 10 && pm.Num[CH_B1] + pm.Num[CH_B2] >= 10 && pm.Num[CH_c] >= 5)
				{
					pm.Mark = PM_JOINT2;   //判定为接头
					iAStepBig = pf.iAStepBigNoCc + 10;
					iAStepSmall = pf.iAStepBigNoCc;
				}
				break;
			}
		}
	}

	//S7 道岔失波验证
	int iFBigLoseIndex = -1;
	if (pm.ChannelNum >= 3 && (pm.Size >= 30 && pm.Percent >= 0.15) || pm.Size >= 10 && pm.Percent > 0.5)
	{
		for (int k = 0; k < g_vFLoseBig.size(); ++k)
		{
			if (Abs((int)g_vFLoseBig[k].Step1 - cr.Step1) <= 50)   //右半边接头
			{
				iFBigLoseIndex = k;
				uint32_t dt2 = 0, s2 = 0;
				dt2 = GetLastPos(cr.Step1, g_vJointPos, s2);
				bool hasReturn1 = IsBacked(cr.Step1, s2, g_vReturnSteps);
				if (dt2 > 200 || dt2 < 5)
				{
					VINT crF, crG;
					GetCR(CH_F, g_vFLoseBig[k].Step1 - 150, g_iJawRow[railType], g_vFLoseBig[k].Step1, 34, blocks, vCRs[CH_F], crF);
					GetCR(CH_G, g_vFLoseBig[k].Step1 - 150, g_iJawRow[railType], g_vFLoseBig[k].Step1, 34, blocks, vCRs[CH_G], crG);
					if (pf.iAStepSmallNoCc > pf.iAStepBigNoCc || pf.iAStepSmallNoCc == 0 || crF.size() > 2 || crG.size() > 2)
					{
						pm.Mark = PM_JOINT_RIGHT;
						pm.IsHalf = 1;
						isManual2 |= 2;
						Pos pos1 = FindStepInBlock(g_vFLoseBig[k].Step1 - 1, g_vBlockHeads, 0);
						if (pos1.Block >= g_iBeginBlock && pos1.Block <= blocks[blocks.size() - 1].Index)
						{
							/***************更新与2021-04-20 210416Z47693_0003_17S5029 887 锰钢岔岔心如果出波，必有高度差*************/
							bool isFindF = blocks[pos1.Block - g_iBeginBlock].vBStepDatas[pos1.Step].isFindF;
							bool isFindG = blocks[pos1.Block - g_iBeginBlock].vBStepDatas[pos1.Step].isFindG;
							uint8_t frow = blocks[pos1.Block - g_iBeginBlock].vBStepDatas[pos1.Step].FRow;
							uint8_t grow = blocks[pos1.Block - g_iBeginBlock].vBStepDatas[pos1.Step].GRow;
							if (isFindF && Abs(g_vFLoseBig[k].Row1 - frow) <= 1 || isFindG && Abs(g_vFLoseBig[k].Row1 - grow) <= 1)
							{

							}
							else
							{
								iLose_Left = g_vFLoseBig[k].Step1 + 5;
								iLose_Right = g_vFLoseBig[k].Step1;
							}
						}
						else
						{
							iLose_Left = g_vFLoseBig[k].Step1 + 5;
							iLose_Right = g_vFLoseBig[k].Step1;
						}

						if (iAStepSmall == 0 || iLose_Right > iAStepSmall)
						{
							iAStepSmall = pf.iAStepBig2;
							iAStepBig = iAStepSmall + 10;
							if (iAStepSmall <= 20)
							{
								iAStepBig = cr.Step1 + 5;
								iAStepSmall = cr.Step1 - 5;
							}
						}

					}
				}
				break;
			}
			else if (Abs((int)g_vFLoseBig[k].Step2 - cr.Step1) <= 50)//左半边接头
			{
				iFBigLoseIndex = k;
				uint32_t dt2 = 0, s2 = 0;
				dt2 = GetLastPos(cr.Step1, g_vJointPos, s2);
				bool hasReturn1 = IsBacked(cr.Step1, s2, g_vReturnSteps);
				if (dt2 > 200 || dt2 < 5)
				{
					VINT crF, crG;
					GetCR(CH_F, g_vFLoseBig[k].Step2, g_iJawRow[railType], g_vFLoseBig[k].Step2 + 150, 34, blocks, vCRs[CH_F], crF);
					GetCR(CH_G, g_vFLoseBig[k].Step2, g_iJawRow[railType], g_vFLoseBig[k].Step2 + 150, 34, blocks, vCRs[CH_G], crG);
					if (pf.iAStepSmallNoCc > pf.iAStepBigNoCc || pf.iAStepBigNoCc == 0 || crF.size() > 2 || crG.size() > 2)
					{
						pm.Mark = PM_JOINT_LEFT;
						pm.IsHalf = 2;
						isManual2 |= 2;
						iLose_Left = g_vFLoseBig[k].Step2;
						iLose_Right = iLose_Left - 5;
						if (iAStepBig == 0 || iLose_Left < iAStepSmall)
						{
							iAStepBig = pf.iAStepSmall2;
							iAStepSmall = iAStepBig - 10;
							if (iAStepSmall <= 20)
							{
								iAStepBig = cr.Step1 + 5;
								iAStepSmall = cr.Step1 - 5;
							}
						}
					}
				}
				break;
			}
		}
	}

	//S8 变坡点验证
	PM slope;
	int dist = GetLenWithSlope(cr.Step1, slope);   //记dist为距离出波最近的变坡点的步进差
	if (dist < 300 && pm.ChannelNum >= 4 && pm.Percent >= 0.15)
	{
		VINT t_ccc[10];
		int chcount = 0;
		for (int i = CH_A1; i < CH_C; ++i)
		{
			chcount += GetCR(i, cr.Step1 - 300, 0, cr.Step2 + 300, 26, blocks, vCRs[i], t_ccc[i], t_cr[i], 5);   //记chcount为出波点前后300步进，排除掉前后100步进内的出波通道数
		}
		if (chcount == 0 && pm.Manual == 0)
		{
			if (cr.Step1 < slope.Step2 && slope.ScrewHoleCount < slope.GuideHoleCount || cr.Step1 > slope.Step2 && slope.ScrewHoleCount > slope.GuideHoleCount)
			{
				pm.Mark = PM_JOINT2;   //判定为接头
			}
			else
			{
				VINT vSteps;   //记vSteps为与变坡点步进差小于300的接头或焊缝所在的步进集合
				int iBeginStep = blocks[0].IndexL2;
				for (int i = vPMs.size() - 1; i >= 0; --i)
				{
					if (vPMs[i].Step2 < iBeginStep)
					{
						break;
					}
					if ((IsJoint(vPMs[i].Mark) || IsJoint(vPMs[i].Mark)) && Abs(vPMs[i].Step2 - slope.Step2) <= 300)
					{
						vSteps.push_back(i);
					}
				}

				if (vSteps.size() == 0)   //若vSteps为空
				{
					if (pf.iAStepBigNoCc > pf.iAStepSmallNoCc + 4)
					{
						pm.Mark = PM_SEW_LRH;   //判定为铝热焊
						isManual2 |= 4;
					}
					else
					{
						pm.Mark = PM_JOINT2;   //判定为接头
						isManual2 |= 4;
					}
				}
			}
		}
	}

	//S9 调整位置标大小通道位置
	//S9.1 前提条件
	if (pm.Mark == PM_JOINT_LEFT || pm.Mark == PM_JOINT_RIGHT || IsSew(pm.Mark) && (pm.Length >= 50 || iAStepBig - iAStepSmall >= 30))
	{
		//S9.2 计算截止通道iLastChannel，和开始结束步进s，e
		int s = 0x7FFFFFFF, e = 0;   //s，e为截止通道内，不少于5个点的出波连通域的开始步进和结束步进
		int iLastChannel = (pm.Mark == PM_JOINT2) ? CH_D : CH_C;
		for (int i = 0; i < iLastChannel; ++i)
		{
			for (int j = 0; j < t_cr[i].size(); ++j)
			{
				CR& tempCR = vCRs[i][t_cr[i][j]];
				if (tempCR.Region.size() <= 5)
				{
					continue;
				}

				s = min(s, tempCR.Step1);
				e = max(e, tempCR.Step2);
			}
		}
		pm.Length = e - s;
		Pos tempPos = FindStepInBlock(s, blocks, 0);
		pm.Block = tempPos.Block;
		pm.Step = tempPos.Step;
		pm.Step2 = tempPos.Step2;

		//S9.3 再次校对出波参数
		if (pf.sum_ABC > 0 && pf.sum_abc > 0 && e - s <= 30)
		{
			iAStepBig = e;
			iAStepSmall = s;
		}
		else if (pf.sum_ABC > 0 && pf.sum_abc == 0)
		{
			if (IsSew(pm.Mark))
			{
				iAStepBig = s;
				iAStepSmall = s - 7;
			}
			else if (IsJoint(pm.Mark))
			{
				iAStepBig = e + 10;
				iAStepSmall = s;
			}
		}
		else if (pf.sum_ABC == 0 && pf.sum_abc > 0)
		{
			if (IsSew(pm.Mark))
			{
				iAStepBig = e + 7;
				iAStepSmall = s;
			}
			else if (IsJoint(pm.Mark))
			{
				iAStepBig = e;
				iAStepSmall = s - 10;
			}
		}
	}

	int pMarkIndex = GetMarkedPositionIndex(cr.Step1 - 50, vPMs2, nullptr);   //记pIndex为从cr.Step1-50到cr.Step1的位置标索引
	//S10 人工标记再次核对
	if (pm.Percent >= 0.2 || pm.Size >= 100 && pm.ChannelNum >= 5 || pm.Size <= 20 && pm.ChannelNum < 5)
	{
		//S10.1 根据pIndex处理
		if (pMarkIndex >= 0)
		{
			if (IsJoint(vPMs2[pMarkIndex].Mark) && Abs(vPMs2[pMarkIndex].Step2 - pf.aveStep) <= 30)
			{
				if (iLoseF == 0 && iLoseG == 0 && pf.validC * pf.validc == 0)
				{
					vPMs2[pMarkIndex].IsOverlapped = 1;
				}
				else
				{
					vPMs2[pMarkIndex].IsFindWave = (pf.iAStepBigNoCc + pf.iAStepSmallNoCc) >> 1;
					pm.Mark = vPMs2[pMarkIndex].Mark;
					FillManualMark(vPMs2[pMarkIndex].Step2, pm.Step2, vPMs2);
				}
			}
			else
			{
				VINT t_cr2[10];
				uint8_t chCount = 0, chCount2 = 0;
				for (int j = 0; j < 10; ++j)
				{
					//记chcount为指定位置标步进前500步进到后20步进的的轨头出波点数不小于4的通道数
					chCount += GetCR(j, vPMs2[pMarkIndex].Step2 - 500, rowCH1, vPMs2[pMarkIndex].Step2 + 20, rowCH2, blocks, vCRs[j], t_cr2[j], t_cr[j], 4);
				}
				for (int j = 0; j < 10; ++j)
				{
					//记chcount2为指定位置标步进前500步进到后500步进的的轨头出波点数不小于4的通道数
					chCount2 += GetCR(j, vPMs2[pMarkIndex].Step2 - 500, rowCH1, vPMs2[pMarkIndex].Step2 + 500, rowCH2, blocks, vCRs[j], t_cr2[j], t_cr[j], 4);
				}
				if (IsJoint(vPMs2[pMarkIndex].Mark) && Abs(vPMs2[pMarkIndex].Step2 - cr.Step1) < 20)
				{
					pm.Mark = vPMs2[pMarkIndex].Mark;   //判定为pIndex对应的位置标的类型
				}
				else if (chCount <= 1)
				{
					//按vPMs[pIndex].Mark的分类处理
					if (vPMs2[pMarkIndex].Mark == PM_SEW_LRH || vPMs2[pMarkIndex].Mark == PM_SELFDEFINE && vPMs2[pMarkIndex].Data == 2)
					{
						if (pm.Size >= 20 && pm.ChannelNum >= 5 || chCount2 <= 1 || isManual2 > 0)
						{
							vPMs2[pMarkIndex].IsFindWave = (pf.iS1 + pf.iS2) / 2;
							pm.Mark = PM_SEW_LRH;   //判定为铝热焊
							FillManualMark(vPMs2[pMarkIndex].Step2, pm.Step2, vPMs2);
							isManual2 |= 8;
						}
					}
					else if (vPMs2[pMarkIndex].Mark == PM_SEW_LIVE || vPMs2[pMarkIndex].Mark == PM_SELFDEFINE && vPMs2[pMarkIndex].Data == 3)
					{
						if (pm.Percent >= 0.3)
						{
							vPMs2[pMarkIndex].IsFindWave = (pf.iS1 + pf.iS2) / 2;
							pm.Mark = PM_SEW_LIVE;   //现场焊
							FillManualMark(vPMs2[pMarkIndex].Step2, pm.Step2, vPMs2);
							isManual2 |= 8;
						}
					}
					else if (vPMs2[pMarkIndex].Mark == PM_SEW_CH || vPMs2[pMarkIndex].Mark == PM_SELFDEFINE && vPMs2[pMarkIndex].Data == 1)
					{
						if (pm.Size <= 50 && pm.Percent >= 0.5 || chCount2 == 0 && pm.Size <= 80 && pf.sumCc <= 20)
						{
							vPMs2[pMarkIndex].IsFindWave = (pf.iS1 + pf.iS2) / 2;
							pm.Mark = PM_SEW_CH;   //判定为厂焊
							FillManualMark(vPMs2[pMarkIndex].Step2, pm.Step2, vPMs2);
							isManual2 |= 8;
							if (pf.iAStepBigNoCc == 0 && pf.iAStepSmallNoCc == 0)
							{
								if (t_cr[CH_C].size() > 0)
								{
									iAStepBig = vCRs[CH_C][t_cr[CH_C][0]].Step1;
									iAStepSmall = vCRs[CH_C][t_cr[CH_C][0]].Step1 - 10;
								}
								else if (t_cr[CH_c].size() > 0)
								{
									iAStepSmall = vCRs[CH_c][t_cr[CH_c][0]].Step1;
									iAStepBig = vCRs[CH_c][t_cr[CH_c][0]].Step1 + 10;
								}
							}
							else if (pf.iAStepBigNoCc == 0)
							{
								pf.iAStepBigNoCc = pf.iAStepSmallNoCc + 7;
							}
							else  if (pf.iAStepSmallNoCc == 0)
							{
								pf.iAStepSmallNoCc = pf.iAStepBigNoCc - 7;
							}
						}
						else//标记错误！
						{

						}
					}
					else if (vPMs2[pMarkIndex].Mark == PM_JOINT2 || vPMs2[pMarkIndex].Mark == PM_JOINT)
					{
						if (pm.Size >= 20 && pm.ChannelNum >= 5 && pm.Num[CH_C] + pm.Num[CH_c] >= 15)
						{
							vPMs2[pMarkIndex].IsFindWave = (pf.iAStepBigNoCc + pf.iAStepSmallNoCc) >> 1;
							pm.Mark = vPMs2[pMarkIndex].Mark;
							FillManualMark(vPMs[pMarkIndex].Step2, pm.Step2, vPMs2);
							isManual2 |= 8;
						}
					}

					if (pm.Mark == PM_SEW_LRH && iSumBigChannel + iSumSmallChannel <= 15)
					{
						isManual2 &= ~8;
						pm.Mark = 0;
					}
				}
			}
		}
		//S10.2 接头补充判定1
		else if (pm.Size >= 150 && pm.Num[CH_C] + pm.Num[CH_c] >= 30 && pf.maxCRSize[CH_C] >= 8 && pf.maxCRSize[CH_c] >= 8 && pf.iAStepBigNoCc < pf.iAStepSmallNoCc)
		{
			pm.Mark = PM_JOINT2;   //判定接头
		}
		//S10.3 接头补充判定2
		else if (pm.Size >= 50 && pm.ChannelNum >= 4 && pm.Percent >= 0.15 && pf.SumAa >= 20 && pf.SumBb >= 20)
		{
			uint32_t s1 = 0, s2 = 0;
			uint32_t dt1 = GetLastPos(cr.Step1, g_vSewPos, s1);   //记dt1为出波点与上一个焊缝的步进差，s1为焊缝步进
			uint32_t dt2 = GetLastPos(cr.Step1, g_vJointPos, s2);   //记dt2为出波点与上一个接头的步进差，s2为接头步进
			if (dt1 < 100 || dt2 < 400)
			{	//设置判断标志
				//	cr.Flag = 1;
				//	continue;
			}
			else
			{
				int bigChannel = pm.Num[CH_A1] + pm.Num[CH_A2] + pm.Num[CH_B1] + pm.Num[CH_B2] + pm.Num[CH_C];
				int smallChannel = pm.Num[CH_a1] + pm.Num[CH_a2] + pm.Num[CH_b1] + pm.Num[CH_b2] + pm.Num[CH_c];
				if (pm.Num[CH_C] + pm.Num[CH_c] >= 15 && pm.ChannelNum >= 5)//A1, A2, B1, B2, C 或者a1, a2, b1, b2, c
				{
					if (bigChannel >= 3 * smallChannel)
					{
						pm.IsHalf = 1;
						pm.Mark = PM_JOINT_RIGHT;   //判定为右半边接头
						if (Abs(iAStepBig - iAStepSmall) > 30)
						{
							iAStepBig = pf.iAStepBigNoCc;
							iAStepSmall = pf.iAStepSmallNoCc - 10;
						}
					}
					else if (smallChannel >= 3 * bigChannel)
					{
						pm.IsHalf = 2;
						pm.Mark = PM_JOINT_LEFT;   //判定为左半边接头
						if (Abs(iAStepBig - iAStepSmall) > 30)
						{
							iAStepBig = pf.iAStepSmallNoCc;
							iAStepSmall = iAStepBig - 10;
						}
					}
					else if (iBigChannelwaveCountNoCc >= 0.6 * iSmallChannelwaveCountNoCc && iBigChannelwaveCountNoCc <= 1.5 * iSmallChannelwaveCountNoCc && pf.iAStepBigNoCc < pf.iAStepSmallNoCc && pf.percent < 0.4)
					{
						pm.Mark = PM_JOINT2;   //判定为接头
					}
				}
				else if (pm.ChannelNum >= 6 && pf.iAStepBigNoCc > pf.iAStepSmallNoCc)
				{
					pm.Mark = PM_SEW_LRH;   //判定为铝热焊
				}
				else if (pm.ChannelNum >= 6 && pf.iAStepBigNoCc < pf.iAStepSmallNoCc && iLoseF && iLoseG)
				{
					pm.Mark = PM_JOINT2;   //判定为接头
				}
			}
		}
	}

	//S12 厂焊过滤
	if (pm.Mark == PM_SEW_CH)
	{
		for (int i = CH_A1; i <= CH_c; ++i)
		{
			for (int j = t_cr[i].size() - 1; j >= 0; --j)
			{
				CR& tempCR = vCRs[i][t_cr[i][j]];
				if (i < CH_C && (tempCR.Row1 > rowCH2 || tempCR.Row2 < rowCH1) || i >= CH_C && tempCR.Row2 < rowCH1)
				{
					t_cr[i].erase(t_cr[i].begin() + j);
				}
			}
		}

		pf = PositionFeature(blocks, vCRs, t_cr, cr.Step1 - 100, cr.Step2 + 100, rowCH1, rowCH2, railType);
		if (pf.iAStepBigNoCc <= 0)
		{
			iAStepSmall = pf.iAStepSmallNoCc;
			iAStepBig = pf.iAStepSmallNoCc + 7;
		}
		else if (pf.iAStepSmallNoCc <= 0)
		{
			iAStepBig = pf.iAStepBigNoCc;
			iAStepSmall = pf.iAStepBigNoCc - 7;
		}

		if (pf.totalCRCountNoCcInJaw >= 5 && pm.Length >= 30 && pm.ChannelNum <= 4)
		{
			pm.Mark = 0;   //判定为非位置标
		}
	}


	//校验各通道出波长度
	if (
		!(pMarkIndex >= 0 && !IsJoint(vPMs2[pMarkIndex].Mark)) &&
		(pm.Size >= 100 && pf.sumCc >= 20 || isManual2 > 0 && pm.Mark != PM_SEW_CH || pm.Size >= 60 && pf.sumCc >= 15)
		)
	{
		Section sections[16];
		int aveLenNoCc;
		GetWaveLen(vCRs, t_cr, sections, aveLenNoCc);
		int iLenNoCc = -1, iMaxStep1 = 0, iMinStep1 = 0x7FFFFFFF;
		for (int i = 0; i < CH_C; i++)
		{
			iMaxStep1 = max(iMaxStep1, sections[i].End);
			iMinStep1 = min(iMinStep1, sections[i].Start);
		}
		iLenNoCc = iMaxStep1 - iMinStep1;
		if (iLenNoCc <= 15)
		{
			if (pm.IsHalf == 0)
			{
				pm.Mark = PM_JOINT2;
			}
			isManual2 |= 16;

			if (iLose_Left < 0)
			{
				iLose_Right = iMinStep1 + aveLenNoCc - 3;
				iLose_Left = iMinStep1 + aveLenNoCc + 3;
			}
		}
	}

	//S11 接头补充判定
	if (pm.ChannelNum >= 8 && pm.Length < 50 && iLoseF && iLoseG && pm.Num[CH_c] + pm.Num[CH_C] > 0 && pm.Mark == 0)
	{
		bool isFind = false;
		for (int i = t_cr[CH_C].size() - 1; i >= 0; --i)   //遍历C通道连通域
		{
			CR& temp = vCRs[CH_C][t_cr[CH_C][i]];
			int d = temp.Step1 - iLose_Left;   //记d为C通道连通域出波步进与iLose_Left的步进差
			if (d > 5 && d < 20 && temp.Row1 <= g_iJawRow[railType] && temp.Row2 >= rowCH2 && temp.Row2 - temp.Row1 >= 5)
			{
				isFind = true;
				break;
			}
		}
		if (isFind == false)
		{
			for (int i = t_cr[CH_c].size() - 1; i >= 0; --i)   //遍历c通道连通域
			{
				CR& temp = vCRs[CH_c][t_cr[CH_c][i]];
				int d = iLose_Right - temp.Step2;   //记d为c通道连通域出波步进与iLose_Left的步进差
				if (d > 5 && d < 20 && temp.Row1 <= g_iJawRow[railType] && temp.Row2 >= rowCH2 && temp.Row2 - temp.Row1 >= 5)
				{
					isFind = true;
					break;
				}
			}
		}

		if (isFind)
		{
			pm.Mark = PM_JOINT2;   //只要Cc有一个isFind，判定为接头
			iAStepBig = iLose_Left;
			iAStepSmall = iLose_Right;
		}
	}

	//S12 厂焊过滤
	if (pm.Mark == PM_SEW_CH)
	{
		if (isManual2 > 0 && pf.sum_ABC * pf.sum_abc > 0)
		{
			pm.Length = 15;
			pm.Manual = 0;
		}
		else if ((isManual2 & 8 == 0) && (pm.Length >= 50 && pm.Percent < 0.4 || pf.totalCRCountNoCc >= 5))
		{
			pm.Mark = 0;   //判定为非位置标
		}
	}

	//S13 调整pm.Length
	int centerStep = (pf.iS1 + pf.iS2) / 2;
	//修正step1， step2
	if ((pm.Mark != 0 && pm.Mark != PM_JOINT_LEFT && pm.Mark != PM_JOINT_RIGHT || pm.Mark == PM_SEW_LRH && pm.ChannelNum >= 7 && pm.Length > 50) && pf.sum_ABC * pf.sum_abc > 0)
	{
		int count = 0;
		for (int i = pf.iS1; i <= pf.iS2; ++i)
		{
			count += distri[i - pf.iS1];   //count为pf.iS1 ~ pf.iS2的出波点数之和（ABC共10个通道）
		}
		//if (1.0 * count / pm.Size < 0.5)
		{
			int pmLen = pf.iS2 - pf.iS1;
			memset(distibute2, 0, 20000 * sizeof(int32_t));
			for (int i = 0; i < pmLen; ++i)
			{
				distibute2[i] = Sum(distri, i - 15, i + 15, pf.iS2 - pf.iS1);   //distibute2为各步进前后15步进的出波点数和
			}
			int maxValue = distibute2[0];   //maxValue为最大的distibute2值
			int maxIndex = 0;   //maxIndex为对应的Index
			for (int i = 1; i < pmLen; ++i)
			{
				if (distibute2[i] > maxValue)
				{
					maxValue = distibute2[i];
					maxIndex = i;
				}
			}

			for (int i = 1; i < pmLen; ++i)
			{
				if (distibute2[i] == maxValue)
				{
					maxIndex = i;
					break;
				}
			}

			int iStart = maxIndex + 1;
			int continueLen = 1;
			for (int i = iStart; i < pmLen; ++i)
			{
				if (distibute2[i] == maxValue)
				{
					continueLen++;   //continueLen为maxValue持续步进
				}
				else
				{
					break;
				}
			}
			maxIndex = maxIndex + continueLen / 2;

			int s = 0, e = 0;
			s = pf.iS1 + maxIndex - 25;
			e = pf.iS1 + maxIndex + 25;
			if (maxIndex - 25 < 0)
			{
				s = pf.iS1;
			}

			if (maxIndex + 25 > pf.iS2 - pf.iS1)
			{
				e = pf.iS2;
			}

			Pos temp = FindStepInBlock(pf.iS1 + maxIndex, g_vBlockHeads, 0);
			//S13.1 处理较宽接头
			if (pm.Mark == PM_JOINT2 && iAStepBig - iAStepSmall >= 15)
			{
				centerStep = pf.iS1 + maxIndex;
				int stepF1 = pf.iS1 + maxIndex, stepF2 = pf.iS1 + maxIndex;
				int stepG1 = stepF1, stepG2 = stepF2;

				int sb = 0, se = 0;
				GetLoseStep(centerStep, blocks, sb, se);   //获取centerStep所在的失波区域，记为sb，se（失波开始结束步进范围）
				Pos temp2;
				if (sb > 0)
				{
					temp2 = FindStepInBlock(sb, g_vBlockHeads, 0);
					iAStepBig = se;
					iAStepSmall = sb;
				}
				else
				{
					t_crF.clear(); t_crG.clear();
					//略微考虑拼图不好的情况
					iLoseF = GetCR(CH_F, centerStep - 3, iFDesiredRow - 3, centerStep + 3, iFDesiredRow + 10, blocks, vCRs[CH_F], t_crF, -1, 2);
					iLoseG = GetCR(CH_G, centerStep - 3, iFDesiredRow - 3, centerStep + 3, iFDesiredRow + 10, blocks, vCRs[CH_G], t_crG, -1, 2);
					iLose_Left = -1;
					iLose_Right = -1;
					if (iLoseF)   //在centerStep – 3，centerStep + 3重新寻找FG失波的连通域
					{
						std::vector<Section> vSec;
						for (int i = 0; i < t_crF.size() && iLose_Right <= 0; ++i)
						{
							uint32_t x1 = vCRs[CH_F][t_crF[i]].Step1, x2 = vCRs[CH_F][t_crF[i]].Step2;
							if (x2 - x1 <= 1)
							{
								continue;
							}
							for (int j = 0; j < t_crG.size(); ++j)
							{
								if (vCRs[CH_G][t_crG[j]].Step1 > e || vCRs[CH_G][t_crG[j]].Step2 < s)
								{
									continue;
								}
								int tb = 0, te = 0;
								GetOverlappedStep(x1, x2, vCRs[CH_G][t_crG[j]].Step1, vCRs[CH_G][t_crG[j]].Step2, tb, te);
								//if (tb > 0)
								//{
								//	iLose_Right = tb;
								//	iLose_Left = te;
								//}
								Section sec;
								sec.Start = tb;
								sec.End = te;
								vSec.emplace_back(sec);   //vSec为找到的FG失波范围
							}

						}

						//若vSec不为空，找到与pf.iAStepBig2, pf.iAStepSmall2重叠区域最大的失波范围，记失波范围为iLose_Right， iLose_Left
						if (vSec.size() > 0)
						{
							int tb = 0, te = 0;
							int iMaxIndex = 0;
							int iMaxOS = GetOverlappedStep(vSec[0].Start, vSec[0].End, pf.iAStepBig2, pf.iAStepSmall2, tb, te);   
							iLose_Right = tb;
							iLose_Left = te;
							for (int i = 1; i < vSec.size(); ++i)
							{
								int os = GetOverlappedStep(vSec[i].Start, vSec[i].End, pf.iAStepBig2, pf.iAStepSmall2, tb, te);
								if (os > iMaxOS)
								{
									iMaxOS = os;
									iMaxIndex = i;

									iLose_Right = tb;
									iLose_Left = te;
								}
							}
						}
					}

					if (iLose_Left > 0)
					{
						iAStepBig = iLose_Left;
						iAStepSmall = iLose_Right;
					}

					if (iAStepBig < iAStepSmall)
					{
						int temp = iAStepSmall;
						iAStepSmall = iAStepBig;
						iAStepBig = temp;
					}
					temp2 = FindStepInBlock(iAStepSmall, g_vBlockHeads, 0);
				}

				sb = 100;
				se = 100;
			}
			if (pm.Mark == PM_SEW_LRH && pm.Length >= 50 || pm.Mark == PM_JOINT2 && pm.Length >= 100)
			{
				for (int i = maxIndex - 1; i >= 0; --i)
				{
					if (distibute2[i] <= 0.5 * maxValue)
					{
						s = pf.iS1 + i;
						break;
					}
				}
				for (int i = maxIndex + 1; i < pf.iS2; ++i)
				{
					if (distibute2[i] <= 0.5 * maxValue)
					{
						e = pf.iS1 + i;
						break;
					}
				}

				for (int i = 0; i < CH_C; ++i)
				{
					int nChannelCRCount = t_cr[i].size() - 1;
					for (int j = nChannelCRCount; j >= 0; --j)
					{
						CR& tempCR = vCRs[i][t_cr[i][j]];
						if (tempCR.Step1 > e || tempCR.Step2 < s)
						{
							t_cr[i].erase(t_cr[i].begin() + j);   //并删除掉各通道不在[s, e]之间的连通域
						}
					}
				}
				pm.Length = e - s;
			}
		}

		if (iLose_Left - iLose_Right > 30 && (isManual2 & 2) == 0)
		{
			iLose_Left = centerStep + 2;
			iLose_Right = centerStep - 2;
			iAStepSmall = iLose_Right;
			iAStepBig = iLose_Left;
		}
	}

#ifdef _DEBUG
	Pos te1 = FindStepInBlock(iAStepSmall, g_vBlockHeads, 0);
	Pos te2 = FindStepInBlock(iAStepBig, g_vBlockHeads, 0);
#endif // _DEBUG
	//S14 类型校验
	if (pMarkIndex < 0)
	{
		if (pm.ChannelNum >= 7 && pm.Size >= 50 && pm.Mark == PM_SEW_CH)
		{
			pm.Mark = PM_SEW_LRH;   //判定为铝热焊
		}
		if (pm.Mark == PM_SEW_LRH && pm.Manual == 0 && (pm.ChannelNum <= 6 || pm.Length > 50 || (pm.Size < 100 && pm.Percent < 0.3)))
		{
			pm.Mark = 0;   //判定为非位置标
		}
		if (pm.Mark == PM_SEW_CH && pm.Length > 40)
		{
			pm.Mark = 0;   //判定为非位置标
		}

		int gap = Abs(pm.BiggerStep - pm.LessStep);
		if (pm.Mark != 0 && gap <= 3 && pf.SumAa + pf.SumBb >= 120 && isManual2 == 0 && pm.Length >= 40 && pf.totalCRCount >= 20)
		{
			pm.Mark = PM_SEW_LRH;
		}
	}

	//
	if (IsSew(pm.Mark))
	{
		int iMiddStep = 0;
		if (iAStepBig < 0 || iAStepSmall < 0)
		{
			iAStepBig = pf.iAStepBigNoCc;
			iAStepSmall = pf.iAStepSmallNoCc;
		}
		if (pm.Mark == PM_SEW_CH)
		{
			iMiddStep = GetSewCHMiddleStep(blocks, vCRs, cr, t_cr, iAStepBig, iAStepSmall);
		}
		else if (pm.Mark == PM_SEW_LRH)
		{
			iMiddStep = GetSewLRHMiddleStep(blocks, vCRs, cr, t_cr, iAStepBig, iAStepSmall, pm);
		}

		for (int i = CH_c; i >= 0; --i)
		{
			for (int j = t_cr[i].size() - 1; j >= 0; --j)
			{
				if (i % 4 == 0 && vCRs[i][t_cr[i][j]].Step2 < iMiddStep || i % 4 != 0 && vCRs[i][t_cr[i][j]].Step1 > iMiddStep)
				{
					vCRs[i][t_cr[i][j]].IsIllgeal = 1;
				}
			}
		}

		pf = PositionFeature(blocks, vCRs, t_cr, cr.Step1 - 100, cr.Step2 + 100, rowCH1, rowCH2, railType);
		pm.Length = pf.iS2 - pf.iS1;
		if (pf.iS2 < pf.iS1)
		{
			if (pMarkIndex < 0)
			{
				pm.Mark = 0;
			}
		}
		pm.Percent = pf.percent;
		pm.Size = pf.sum_ABC + pf.sum_abc;

		for (int i = CH_c; i >= 0; --i)
		{
			for (int j = t_cr[i].size() - 1; j >= 0; --j)
			{
				if (i % 4 == 0 && vCRs[i][t_cr[i][j]].Step2 < iMiddStep || i % 4 != 0 && vCRs[i][t_cr[i][j]].Step1 > iMiddStep)
				{
					vCRs[i][t_cr[i][j]].IsIllgeal = 0;
				}
			}
		}
	}

	//S15 厂焊再校验
	//删除出波区域内轨颚以外的连通域出波
	if (pm.Mark == PM_SEW_CH)
	{
		bool bDelete = false;
		for (int i = CH_A1; i < CH_C; ++i)
		{
			for (int j = t_cr[i].size() - 1; j >= 0; --j)
			{
				if (vCRs[i][t_cr[i][j]].Row1 > rowCH2 || vCRs[i][t_cr[i][j]].Row2 < rowCH1)
				{
					t_cr[i].erase(t_cr[i].begin() + j);
					bDelete = true;
				}
			}
		}
		if (bDelete)
		{
			pf = PositionFeature(blocks, vCRs, t_cr, cr.Step1 - 100, cr.Step2 + 100, rowCH1, rowCH2, railType);
			Pos pos = FindStepInBlock(pf.iS1, blocks, 0);
			pm.Block = pos.Block;
			pm.Step = pos.Step;
			pm.Step2 = pf.iS1;
		}
	}
	else if (pm.Mark == PM_SEW_LRH && pMarkIndex < 0)
	{
		if (pm.Percent >= 0.5 && pm.Length <= 18)
		{
			if (pm.BiggerStep > 0 && pm.LessStep > 0 && pm.BiggerStep - pm.LessStep <= 8 || pm.BiggerStep * pm.LessStep == 0)
			{
				pm.Mark = PM_SEW_CH;
			}
		}
	}

	//S16 接头再处理
	if (IsJoint(pm.Mark))
	{
		if (Abs(iAStepBig - iAStepSmall) >= 40)
		{
			if (pm.IsHalf == 2)
			{
				iAStepBig = iAStepSmall;
				iAStepSmall = iAStepBig - 10;
			}
			else if (pm.IsHalf == 1)
			{
				iAStepSmall = iAStepBig;
				iAStepBig = iAStepSmall + 10;
			}
			else
			{

			}
		}

		int middle = (iAStepBig + iAStepSmall) >> 1;
		if (pMarkIndex >= 0)
		{
			middle = vPMs[pMarkIndex].Step2 + 3;
		}

		//S16.1 寻找Cc出波
		t_cr[8].clear();
		t_cr[9].clear();
		GetCR(CH_C, middle - 12, 0, middle + 42, g_iJawRow[railType] * 2, blocks, vCRs[CH_C], t_cr[CH_C]);
		GetCR(CH_c, middle - 42, 0, middle + 12, g_iJawRow[railType] * 2, blocks, vCRs[CH_c], t_cr[CH_c]);

		//S16.2 寻找Cc两通道的主出波：索引依次记为idxC，idxc
		int idxC = -1, idxc = -1;
		//S16.2.2 遍历c通道出波连通域
		for (int i = 0; i < t_cr[CH_c].size(); ++i)
		{
			CR& tempCR = vCRs[CH_c][t_cr[CH_c][i]];
			if (tempCR.Row1 <= g_iJawRow[railType] && tempCR.Row2 >= rowCH2 && tempCR.Step2 < min(pf.iAStepBigNoCc, pf.iAStepSmallNoCc))
			{
				idxc = i;
				break;
			}
		}

		//S16.2.1 遍历C通道出波连通域
		for (int i = 0; i < t_cr[CH_C].size(); ++i)
		{
			CR& tempCR = vCRs[CH_C][t_cr[CH_C][i]];
			if (tempCR.Row1 <= g_iJawRow[railType] && tempCR.Row2 >= rowCH2 && tempCR.Step1 > max(pf.iAStepBigNoCc, pf.iAStepSmallNoCc))
			{
				idxC = i;
				break;
			}
		}
		//S16.3 c出波整理
		if (idxc >= 0)
		{
			//遍历tempCR为c的出波连通域
			int s = vCRs[CH_c][t_cr[CH_c][idxc]].Step1;
			for (int i = t_cr[CH_c].size() - 1; i >= 0; --i)
			{
				CR& tempCR = vCRs[CH_c][t_cr[CH_c][i]];
				if (tempCR.Step1 < s || tempCR.Step1 > iLose_Left + 10 || tempCR.Row2 < g_iJawRow[railType] - 2)
				{
					t_cr[CH_c].erase(t_cr[CH_c].begin() + i);   //删除c出波
				}
				else if (tempCR.Step1 - s <= 25)
				{
					SetJointFlag(tempCR, 1);   //tempCR为接头正常出波
					SetUsedFlag(tempCR, 1);
				}
			}
		}
		else
		{
			for (int i = t_cr[CH_c].size() - 1; i >= 0; --i)
			{
				CR& tempCR = vCRs[CH_c][t_cr[CH_c][i]];
				if (tempCR.Step1 > iLose_Left + 5 || tempCR.Row2 < g_iJawRow[railType] - 2)
				{
					t_cr[CH_c].erase(t_cr[CH_c].begin() + i);   //删除c出波
				}
				else
				{
					SetJointFlag(tempCR, 1);   //tempCR为接头正常出波
					SetUsedFlag(tempCR, 1);
				}
			}
		}

		//S16.4 C出波整理
		if (idxC >= 0)
		{
			//遍历tempCR为C的出波连通域
			int s = vCRs[CH_C][t_cr[CH_C][idxC]].Step1;
			for (int i = t_cr[CH_C].size() - 1; i >= 0; --i)
			{
				CR& tempCR = vCRs[CH_C][t_cr[CH_C][i]];
				if (s - tempCR.Step2 <= 25)
				{
					SetJointFlag(tempCR, 1);   //tempCR为接头正常出波
				}
				if (t_cr[CH_C][i] == idxC)
				{
					SetJointFlag(tempCR, 1);   //tempCR为接头正常出波
					SetUsedFlag(tempCR, 1);
				}
				else if (tempCR.Step2 < iLose_Right - 10 || tempCR.Step1 > s || tempCR.Row2 < g_iJawRow[railType] - 2)
				{
					t_cr[CH_C].erase(t_cr[CH_C].begin() + i);   //删除C出波
				}
				else
				{
					SetJointFlag(tempCR, 1);   //tempCR为接头正常出波
					SetUsedFlag(tempCR, 1);
				}
			}
		}
		else
		{
			for (int i = t_cr[CH_C].size() - 1; i >= 0; --i)
			{
				CR& tempCR = vCRs[CH_C][t_cr[CH_C][i]];
				if (tempCR.Step2 < iLose_Right - 5 || tempCR.Row2 < g_iJawRow[railType] - 2)
				{
					t_cr[CH_C].erase(t_cr[CH_C].begin() + i);
				}
				else
				{
					SetJointFlag(tempCR, 1);
					SetUsedFlag(tempCR, 1);
				}
			}
		}

		//校正iAStepBig和iAStepSmall
		int bs, es;
		//若为接头且iLose_Right, iLose_Left, pf.iAStepBigNoCc, pf.iAStepSmallNoCc没有重叠区域
		if (GetOverlappedStep(iLose_Right, iLose_Left, pf.iAStepBigNoCc, pf.iAStepSmallNoCc, bs, es) <= 0 && pm.Mark == PM_JOINT2)
		{
			/**需要修改：对照文件：200609Q0081DLS_0001_16S1008，米块：294。找不到-1孔*/
			iAStepBig = pf.iAStepSmallNoCc > 0 ? pf.iAStepSmallNoCc : pf.iAStepBigNoCc + 10;
			iAStepSmall = pf.iAStepBigNoCc > 0 ? pf.iAStepBigNoCc : pf.iAStepSmallNoCc - 10;
		}
	}
	return pm.Mark;
		}


uint32_t	ParsePositionPost(F_HEAD& g_fileHead, VBDB& blocks, VCR* vCRs, int step1, int step2, uint8_t row1, uint8_t row2, int16_t iFDesiredRow, uint8_t railType, uint32_t mark, Position_Mark& pm, int& iAStepBig, int &iAStepSmall, VPM& vPMs)
{
	//轨颚范围行高
	int32_t rowCH1 = g_iJawRow[railType] - 3, rowCH2 = g_iJawRow[railType] + 3, iRowH = g_iJawRow[railType] << 1;
	int iFindStepRight = step1 - 50;
	int iFindStepLeft = step1 + 50;
	VINT t_cr[16];
	for (int i = CH_A1; i < CH_C; ++i)
	{
		GetCR(i, iFindStepRight, 0, iFindStepLeft, iRowH, blocks, vCRs[i], t_cr[i], -1, 2, false);
	}
	for (int i = CH_C; i < CH_D; ++i)
	{
		GetCR(i, iFindStepRight, 0, iFindStepLeft, iRowH, blocks, vCRs[i], t_cr[i], -1, 1, false);
	}

	CR cr;
	for (int order = 0; order < 10; ++order)
	{
		int channel = chParseOrder[order];
		if (t_cr[channel].size() > 0)
		{
			cr = vCRs[channel][t_cr[channel][0]];
			break;
		}
	}

	PositionFeature pf(blocks, vCRs, t_cr, cr.Step1 - 100, cr.Step2 + 100, rowCH1, rowCH2, railType);
	if (pf.iSumBigChannel + pf.iSumSmallChannel <= 20 && pf.maxSize < 5)
	{
		for (int i = CH_A1; i < CH_D; ++i)
		{
			for (int j = t_cr[i].size() - 1; j >= 0; --j)
			{
				if (IsCRDirty_ABC(blocks, vCRs, vCRs[i][t_cr[i][j]]))
				{
					t_cr[i].erase(t_cr[i].begin() + j);
				}
			}
		}

		pf = PositionFeature(blocks, vCRs, t_cr, cr.Step1 - 100, cr.Step2 + 100, rowCH1, rowCH2, railType);
	}

	if (IsJoint(mark))
	{
		if (pf.iAStepBig2 < pf.iAStepSmall2)
		{
			if (pf.percent < 0.15 || pf.percent > 0.4)
			{
				mark = 0;
			}
			else if (pf.iAStepBig2 > pf.iAStepSmall2)
			{
				mark = 0;
			}
		}
		else
		{
			mark = PM_SEW_LRH;
		}
	}

	if (IsSew(mark))
	{
		if (pf.percent < 0.3 || pf.iS2 - pf.iS1 >= 40)
		{
			mark = 0;
		}
		else if (pf.iAStepBig2 == 0)
		{
			if (pf.iAStepBig == 0)
			{
				mark = 0;
			}
			else if (pf.percent < 0.4 || pf.iAStepBigNoCc <= pf.iAStepSmallNoCc)
			{
				mark = 0;
			}
		}
		else if (pf.iAStepBig2 < pf.iAStepSmall2)
		{
			mark = 0;
		}
		else if (pf.ChannelNum <= 4 || pf.sumAB <= 10 || pf.sumab <= 10)
		{
			mark = 0;
		}
	}

	if (mark > 0)
	{
		Pos pos = FindStepInBlock(step1, blocks, 0);
		pm.Block = pos.Block;
		pm.Step = pos.Step;
		pm.Step2 = pos.Step2;
		pm.BeginStep = pos.Step2;
		pm.ARow = pf.aveRow;
		pm.AStep = pf.aveStep;
		pm.Height = pf.ir2 - pf.ir1;
		pm.Length = pf.iS2 - pf.iS1;
		pm.ChannelNum = pf.ChannelNum;
		pm.Percent = pf.percent;
		pm.Mark = mark;
		for (int i = 0; i < 16; ++i)
		{
			pm.Num[i] = pf.sumEveyChannel[i];
		}
		pm.Size = pf.sum_ABC + pf.sum_abc;
		pm.Data = 0xFFFF;
	}
	return mark;
}
