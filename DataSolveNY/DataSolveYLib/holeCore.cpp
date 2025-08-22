#include "stdafx.h"
#include "holeCore.h"

#include "CRHPE.h"
#include "PublicFunc.h"
#include "GlobalDefine.h"
#include "Judge.h"

uint8_t	IsScrewHoleSecondWave(CR& cr1, CR& cr2, VBDB& blocks, double angle, double offset, double step)
{
	if (cr1.Region.size() == 0 || cr2.Region.size() == 0)
	{
		return  0;
	}
	if (cr2.IsEnsureNotDoubleWave == 1 || cr2.IsReversed == 1)
	{
		return 0;
	}

	int32_t step1 = 0, step2 = 0;
	GetCRASteps(cr1.Channel, cr1, step1, step2, blocks, angle, offset, step);
	int iCount = 0;

	int32_t step11 = 0, step22 = 0;
	for (int i = 0; i < cr2.Region.size(); ++i)
	{
		GetCRASteps(cr2.Channel, cr2.Region[i], step11, step22, blocks, angle, offset, step);
		if (step11 >= step1 && step22 <= step2)
		{
			iCount++;
		}
	}
	if (iCount == cr2.Region.size())
	{
		cr2.IsDoubleWave = 2;
		return 2;
	}
	if (((double)iCount) >= (double)(0.6 * cr2.Region.size() + 0.5))
	{
		cr2.IsDoubleWave = 1;
		return 1;
	}
	else
	{
		return 0;
	}
}

uint8_t	IsGuideHoleSecondWave(CR& cr1, CR& cr2, VBDB& blocks, double angle, double offset, double step)
{
	if (cr1.Region.size() == 0 || cr2.Region.size() == 0)
	{
		return  0;
	}
	if (cr2.IsEnsureNotDoubleWave == 1 || cr2.IsReversed == 1)
	{
		return 0;
	}
	int32_t step1 = 0, step2 = 0;
	GetCRASteps(cr1.Channel, cr1, step1, step2, blocks, angle, offset, step);
	int iCount = 0;

	int32_t step11 = 0, step22 = 0;
	for (int i = 0; i < cr2.Region.size(); ++i)
	{
		GetCRASteps(cr2.Channel, cr2.Region[i], step11, step22, blocks, angle, offset, step);
		if (step11 >= step1 && step22 <= step2)
		{
			iCount++;
		}
	}
	if (iCount == cr2.Region.size())
	{
		cr2.IsDoubleWave = 2;
		return 2;
	}
	if (iCount >= 0.5 * cr2.Region.size())
	{
		cr2.IsDoubleWave = 1;
		return 1;
	}
	else
	{
		return 0;
	}
}


void	GetCRRelativeInfo(CR& cr1, CR& cr2, int& overlappedH, int& overlappedV, double& dh2, double& dv2)
{
	overlappedH = 0;
	overlappedV = 0;
	dh2 = 0;
	dv2 = 0;
	int n1 = cr1.Region.size(), n2 = cr2.Region.size();
	for (int i = 0; i < n1; ++i)
	{
		for (int j = 0; j < n2; ++j)
		{
			if (cr1.Region[i].row == cr2.Region[j].row)
			{
				overlappedV++;
				dh2 += cr1.Region[i].step - cr2.Region[j].step;
				break;
			}
			if (cr1.Region[i].step == cr2.Region[j].step)
			{
				overlappedH++;
				dv2 += cr1.Region[i].row - cr2.Region[j].row;
				break;
			}
		}
	}
	if (overlappedH > 0)
	{
		dv2 = dv2 / overlappedH;
	}
	if (overlappedV > 0)
	{
		dh2 = dh2 / overlappedV;
	}
}


bool	ParseScrewHoleCrackLeft(F_HEAD& head, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int16_t& iFDesiredRow, CR& crFG, int iExceptF, int iExceptG, int iScrewIndex, uint8_t bJoint, bool bAutoJoint, Wound_Judged& w, VPM& vPMs)
{
	VINT crF;
	bool bFindF = GetCR(CH_F, crFG.Step2 - 1, crFG.Row2 + 2, crFG.Step2 + 10, crFG.Row2 + 10, blocks, vCRs[CH_F], crF, iExceptF, 2, true);
	SetScrewHoleFlag(vCRs[CH_F], crF, 1);

	bool bWound = false;
	for (int i = 0; i < crF.size(); ++i)
	{
		CR& tpF = vCRs[CH_F][crF[i]];
		if (tpF.Step1 > crFG.Step1 && tpF.Step2 < crFG.Step2 || (crFG.Channel == CH_F && crF[i] == iExceptF) || tpF.Row1 <= crFG.Row2 + 2 && tpF.Row2 - crFG.Row1 <= 4)
		{
			continue;
		}

		if (!bWound)
		{
			FillWound(w, blockHead, head);
			w.IsScrewHole = iScrewIndex + 10;
			w.Block = tpF.Block;
			w.Step = tpF.Step;
			w.Step2 = tpF.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Type = W_SCREW_HORIZON_CRACK_LEFT;
			w.Place = WP_WAIST;

			w.SizeX = head.step * (tpF.Step2 - tpF.Step1);
			w.SizeY = 1;

			sprintf(w.Result, "%d孔左侧水平裂纹", iScrewIndex);
			w.Degree = WD_SERIOUS;

			bWound = true;
			bool bFindG2 = false, bFindD = false, bFindE = false;

			int iFStep1 = tpF.Step1, iFStep2 = tpF.Step2;
			uint8_t iFRow1 = tpF.Row1, iFRow2 = tpF.Row2;
			CR crFGWound = tpF;

			VINT crGt;
			GetCR(CH_G, tpF.Step1 - 3, tpF.Row1 - 1, tpF.Step2 + 3, tpF.Row2 + 1, blocks, vCRs[CH_G], crGt, -1, 1);
			for (int i = crGt.size() - 1; i >= 0; --i)
			{
				bool isSameRow = !(vCRs[CH_G][crGt[i]].Row1 > crFGWound.Row2 || vCRs[CH_G][crGt[i]].Row2 < crFGWound.Row1);
				if (GetDistance(vCRs[CH_G][crGt[i]], tpF) <= 2 || isSameRow)
				{
					SetUsedFlag(vCRs[CH_G][crGt[i]], 1);
					iFStep1 = min(iFStep1, vCRs[CH_G][crGt[i]].Step1);
					iFStep2 = max(iFStep2, vCRs[CH_G][crGt[i]].Step2);
					iFRow1 = min(iFRow1, vCRs[CH_G][crGt[i]].Row1);
					iFRow2 = min(iFRow2, vCRs[CH_G][crGt[i]].Row2);
					Combine(crFGWound, vCRs[CH_G][crGt[i]]);
					bFindG2 = true;
				}
				else
				{
					crGt.erase(crGt.begin() + i);
				}
			}

			VINT crD;
			GetCR(CH_D, iFStep1 - 2, iFRow1, iFStep2 + 1, iFRow2 + 1, blocks, vCRs[CH_D], crD, -1, 2);
			for (int i = crD.size() - 1; i >= 0; --i)
			{
				if (GetDistance(vCRs[CH_D][crD[i]], crFGWound) <= g_iScrewHorizonDEDistance && vCRs[CH_D][crD[i]].Row2 >= crFGWound.Row2 && vCRs[CH_D][crD[i]].Step2 <= crFGWound.Step2)
				{
					SetUsedFlag(vCRs[CH_D][crD[i]], 1);
					bFindD = true;
				}
				else
				{
					crD.erase(crD.begin() + i);
				}
			}

			VINT crE;
			GetCR(CH_E, iFStep1 - 1, iFRow1 - 1, iFStep2, iFRow2 + 1, blocks, vCRs[CH_E], crE, -1, 2);
			for (int i = crE.size() - 1; i >= 0; --i)
			{
				if (GetDistance(vCRs[CH_E][crE[i]], crFGWound) <= g_iScrewHorizonDEDistance && vCRs[CH_E][crE[i]].Row2 >= crFGWound.Row2  && vCRs[CH_E][crE[i]].Step2 >= crFGWound.Step2 - 1)
				{
					SetUsedFlag(vCRs[CH_E][crE[i]], 1);
					bFindE = true;
				}
				else
				{
					crE.erase(crE.begin() + i);
				}
			}

			if (bFindD && bFindE/* && bFindG2*/)//也是一个螺孔
			{
				w.Type = W_DOUBLE_HOLE;
				AddWoundData(w, tpF);
				AddWoundData(w, vCRs[CH_D], crD);
				AddWoundData(w, vCRs[CH_E], crE);
				AddWoundData(w, vCRs[CH_G], crGt);

				PM pm;
				pm.Step2 = vCRs[CH_D][crD[0]].Step1;
				pm.Length = vCRs[CH_E][crE[0]].Step2 - vCRs[CH_D][crD[0]].Step1;
				Pos pos = FindStepInBlock(pm.Step2, blocks, 0);
				pm.Block = pos.Block;
				pm.Step = pos.Step;
				pm.Mark = PM_GUIDEHOLE;
				AddToMarks(pm, vPMs);
			}
			else
			{
				AddWoundData(w, tpF);
				AddWoundData(w, vCRs[CH_D], crD);
				AddWoundData(w, vCRs[CH_E], crE);
				AddWoundData(w, vCRs[CH_G], crGt);
			}
		}
		else
		{
			AddWoundData(w, tpF);
		}
	}


	VINT crG;
	bool bFindG = GetCR(CH_G, crFG.Step2 - 1, crFG.Row2 + 2, crFG.Step2 + 10, crFG.Row2 + 10, blocks, vCRs[CH_G], crG, iExceptG, 2, true);
	SetScrewHoleFlag(vCRs[CH_G], crG, 1);
	for (int i = 0; i < crG.size(); ++i)
	{
		CR& tpG = vCRs[CH_G][crG[i]];
		if (tpG.Step1 >= crFG.Step1 && tpG.Step2 <= crFG.Step2 || (crFG.Channel == CH_G && crG[i] == iExceptG) || tpG.Row1 <= crFG.Row2 + 2 && tpG.Row2 - crFG.Row1 <= 4)
		{
			continue;
		}

		if (tpG.IsUsed == 1)
		{
			continue;
		}

		if (!bWound)
		{
			FillWound(w, blockHead, head);
			w.IsScrewHole = iScrewIndex + 10;
			w.Block = tpG.Block;
			w.Step = tpG.Step;
			w.Step2 = tpG.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Type = W_SCREW_HORIZON_CRACK_LEFT;
			w.Place = WP_WAIST;

			w.SizeX = head.step * (tpG.Step2 - tpG.Step1);
			w.SizeY = 1;
			sprintf(w.Result, "%d孔左侧水平裂纹", iScrewIndex);
			w.Degree = WD_SERIOUS;

			bWound = true;
			bool bFindF2 = false, bFindD = false, bFindE = false;
			VINT crFt;
			GetCR(CH_F, tpG.Step1 - 1, tpG.Row1 - 1, tpG.Step2 + 1, tpG.Row2 + 1, blocks, vCRs[CH_F], crFt, -1, 1);
			for (int i = crFt.size() - 1; i >= 0; --i)
			{
				if (GetDistance(vCRs[CH_F][crFt[i]], tpG) <= 2)
				{
					SetUsedFlag(vCRs[CH_F][crFt[i]], 1);
					bFindF2 = true;
				}
				else
				{
					crFt.erase(crFt.begin() + i);
				}
			}

			VINT crD;
			GetCR(CH_D, tpG.Step1 - 2, tpG.Row1 - 1, tpG.Step2 + 1, tpG.Row2 + 1, blocks, vCRs[CH_D], crD, -1, 2);
			for (int i = crD.size() - 1; i >= 0; --i)
			{
				if (GetDistance(vCRs[CH_D][crD[i]], tpG) <= g_iScrewHorizonDEDistance && vCRs[CH_D][crD[i]].Row2 >= tpG.Row2 && vCRs[CH_D][crD[i]].Step2 <= tpG.Step2)
				{
					SetUsedFlag(vCRs[CH_D][crD[i]], 1);
					bFindD = true;
				}
				else
				{
					crD.erase(crD.begin() + i);
				}
			}

			VINT crE;
			GetCR(CH_E, tpG.Step1 - 1, tpG.Row1 - 1, tpG.Step2 + 2, tpG.Row2 + 1, blocks, vCRs[CH_E], crE, -1, 2);
			for (int i = crE.size() - 1; i >= 0; --i)
			{
				if (GetDistance(vCRs[CH_E][crE[i]], tpG) <= 2 && vCRs[CH_E][crE[i]].Row2 >= tpG.Row2 && vCRs[CH_E][crE[i]].Step2 >= tpG.Step2 - 1)
				{
					SetUsedFlag(vCRs[CH_E][crE[i]], 1);
					bFindE = true;
				}
				else
				{
					crE.erase(crE.begin() + i);
				}
			}

			if (bFindD && bFindE/* && bFindF2*/)//也是一个螺孔
			{
				w.Type = W_DOUBLE_HOLE;
				AddWoundData(w, tpG);
				AddWoundData(w, vCRs[CH_D], crD);
				AddWoundData(w, vCRs[CH_E], crE);
				AddWoundData(w, vCRs[CH_F], crFt);

				PM pm;
				pm.Step2 = vCRs[CH_D][crD[0]].Step1;
				pm.Length = vCRs[CH_E][crE[0]].Step2 - vCRs[CH_D][crD[0]].Step1;
				Pos pos = FindStepInBlock(pm.Step2, blocks, 0);
				pm.Block = pos.Block;
				pm.Step = pos.Step;
				pm.Mark = PM_GUIDEHOLE;
				AddToMarks(pm, vPMs);
			}
			else
			{
				AddWoundData(w, tpG);
				AddWoundData(w, vCRs[CH_D], crD);
				AddWoundData(w, vCRs[CH_E], crE);
				AddWoundData(w, vCRs[CH_F], crFt);
			}
		}
	}
	SetUsedFlag(vCRs[CH_F], crF, 1);
	SetUsedFlag(vCRs[CH_G], crG, 1);
	return w.Type > 0;
}

bool	ParseScrewHoleCrackRight(F_HEAD& head, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int16_t& iFDesiredRow, CR& crFG, int iExceptF, int iExceptG, int iScrewIndex, uint8_t bJoint, bool bAutoJoint, Wound_Judged& w, VPM& vPMs)
{
	VINT crF;
	bool bFindF = GetCR(CH_F, crFG.Step1 - 10, crFG.Row2 + 1, crFG.Step1 + 2, crFG.Row2 + 10, blocks, vCRs[CH_F], crF, iExceptF, 2, true);
	SetScrewHoleFlag(vCRs[CH_F], crF, 1);

	bool bWound = false;
	for (int i = 0; i < crF.size(); ++i)
	{
		CR& tpF = vCRs[CH_F][crF[i]];
		if (tpF.Step1 >= crFG.Step1 && tpF.Step2 <= crFG.Step2 || (crFG.Channel == CH_F && crF[i] == iExceptF) || tpF.Row1 <= crFG.Row2)
		{
			continue;
		}
		if (tpF.Step2 >= crFG.Step2 && iScrewIndex == 0)
		{
			continue;
		}
		if (tpF.IsUsed == 1)
		{
			continue;
		}
		if (!bWound)
		{
			w.IsScrewHole = iScrewIndex + 10;
			FillWound(w, blockHead, head);
			w.Block = tpF.Block;
			w.Step = tpF.Step;
			w.Step2 = tpF.Step2;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Type = W_SCREW_HORIZON_CRACK_RIGHT;
			w.Place = WP_WAIST;

			w.SizeX = head.step * (tpF.Step2 - tpF.Step1);
			w.SizeY = 1;
			sprintf(w.Result, "%d孔右侧水平裂纹", iScrewIndex);
			w.Degree = WD_SERIOUS;

			int iFStep1 = tpF.Step1, iFStep2 = tpF.Step2; 
			uint8_t iFRow1 = tpF.Row1, iFRow2 = tpF.Row2;
			CR crFGWound = tpF;

			bWound = true;
			bool bFindG2 = false, bFindD = false, bFindE = false;

			VINT crGt;
			GetCR(CH_G, tpF.Step1 - 1, tpF.Row1 - 1, tpF.Step2 + 1, tpF.Row2 + 1, blocks, vCRs[CH_G], crGt, -1, 1);
			for (int i = crGt.size() - 1; i >= 0; --i)
			{
				bool isSameRow = !(vCRs[CH_G][crGt[i]].Row1 > crFGWound.Row2 || vCRs[CH_G][crGt[i]].Row2 < crFGWound.Row1);
				if (GetDistance(vCRs[CH_G][crGt[i]], tpF) <= 2 || isSameRow)
				{
					SetUsedFlag(vCRs[CH_G][crGt[i]], 1);
					iFStep1 = min(iFStep1, vCRs[CH_G][crGt[i]].Step1);
					iFStep2 = max(iFStep2, vCRs[CH_G][crGt[i]].Step2);
					iFRow1 = min(iFRow1, vCRs[CH_G][crGt[i]].Row1);
					iFRow2 = min(iFRow2, vCRs[CH_G][crGt[i]].Row2);
					Combine(crFGWound, vCRs[CH_G][crGt[i]]);
					bFindG2 = true;
				}
				else
				{
					crGt.erase(crGt.begin() + i);
				}
			}

			VINT crD;
			GetCR(CH_D, iFStep1 - 2, iFRow1 - 1, iFStep2 + 1, iFRow2 + 1, blocks, vCRs[CH_D], crD, -1, 2);
			for (int i = crD.size() - 1; i >= 0; --i)
			{
				if (GetDistance(vCRs[CH_D][crD[i]], crFGWound) <= g_iScrewHorizonDEDistance && vCRs[CH_D][crD[i]].Row2 >= crFGWound.Row2 && vCRs[CH_D][crD[i]].Step2 <= crFGWound.Step2)
				{
					SetUsedFlag(vCRs[CH_D][crD[i]], 1);
					bFindD = true;
					break;
				}
				else
				{
					crD.erase(crD.begin() + i);
				}
			}

			VINT crE;
			GetCR(CH_E, iFStep1 - 1, iFRow1 - 1, max(iFStep2 + 2, crFG.Step2), iFRow2 + 1, blocks, vCRs[CH_E], crE, -1, 2);
			for (int i = crE.size() - 1; i >= 0; --i)
			{
				if (GetDistance(vCRs[CH_E][crE[i]], crFGWound) <= g_iScrewHorizonDEDistance && vCRs[CH_E][crE[i]].Row2 >= crFGWound.Row2 && vCRs[CH_E][crE[i]].Step2 >= crFGWound.Step2 - 1)
				{
					SetUsedFlag(vCRs[CH_E][crE[i]], 1);
					bFindE = true;
					break;
				}
				else
				{
					crE.erase(crE.begin() + i);
				}
			}

			if (bFindD && bFindE/* && bFindG2*/)//也是一个螺孔
			{
				w.Type = W_DOUBLE_HOLE;
				AddWoundData(w, tpF);
				AddWoundData(w, vCRs[CH_D], crD);
				AddWoundData(w, vCRs[CH_E], crE);
				AddWoundData(w, vCRs[CH_G], crGt);

				PM pm;
				pm.Step2 = vCRs[CH_D][crD[0]].Step1;
				pm.Length = vCRs[CH_E][crE[0]].Step2 - vCRs[CH_D][crD[0]].Step1;
				Pos pos = FindStepInBlock(pm.Step2, blocks, 0);
				pm.Block = pos.Block;
				pm.Step = pos.Step;
				pm.Mark = PM_GUIDEHOLE;
				AddToMarks(pm, vPMs);
			}
			else
			{
				AddWoundData(w, tpF);
				AddWoundData(w, vCRs[CH_D], crD);
				AddWoundData(w, vCRs[CH_E], crE);
				AddWoundData(w, vCRs[CH_G], crGt);
			}
		}
	}

	VINT crG;
	bool bFindG = GetCR(CH_G, crFG.Step1 - 10, crFG.Row2 + 1, crFG.Step1 + 2, crFG.Row2 + 10, blocks, vCRs[CH_G], crG, iExceptG, 2, true);
	SetScrewHoleFlag(vCRs[CH_G], crG, 1);

	for (int i = 0; i < crG.size(); ++i)
	{
		CR& tpG = vCRs[CH_G][crG[i]];
		if (tpG.Step1 >= crFG.Step1 && tpG.Step2 <= crFG.Step2 || (crFG.Channel == CH_G && crG[i] == iExceptG) || tpG.Row1 <= crFG.Row2)
		{
			continue;
		}
		if (tpG.Step2 >= crFG.Step2 && iScrewIndex == 0)
		{
			continue;
		}
		if (tpG.IsUsed == 1)
		{
			continue;
		}
		bWound = true;
		if (bWound)
		{
			w.IsScrewHole = iScrewIndex + 10;
			FillWound(w, blockHead, head);
			w.Block = tpG.Block;
			w.Step = tpG.Step;
			w.Step2 = tpG.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Type = W_SCREW_HORIZON_CRACK_RIGHT;
			w.Place = WP_WAIST;

			w.SizeX = head.step * (tpG.Step2 - tpG.Step1);
			w.SizeY = 1;

			sprintf(w.Result, "%d孔右侧水平裂纹", iScrewIndex);
			w.Degree = WD_SERIOUS;
		}

		bWound = true;

		bool bFindF2 = false, bFindD = false, bFindE = false;
		VINT crFt;
		GetCR(CH_F, tpG.Step1 - 1, tpG.Row1 - 1, tpG.Step2 + 1, tpG.Row2 + 1, blocks, vCRs[CH_F], crFt, -1, 1);
		for (int i = crFt.size() - 1; i >= 0; --i)
		{
			if (GetDistance(vCRs[CH_F][crFt[i]], tpG) <= 2)
			{
				SetUsedFlag(vCRs[CH_F][crFt[i]], 1);
				bFindF2 = true;
			}
			else
			{
				crFt.erase(crFt.begin() + i);
			}
		}

		VINT crD;
		GetCR(CH_D, tpG.Step1 - 2, tpG.Row1 - 1, tpG.Step2 + 1, tpG.Row2 + 1, blocks, vCRs[CH_D], crD, -1, 2);
		for (int i = crD.size() - 1; i >= 0; --i)
		{
			if (GetDistance(vCRs[CH_D][crD[i]], tpG) <= 5 && vCRs[CH_D][crD[i]].Row2 >= tpG.Row2 && vCRs[CH_D][crD[i]].Step2 <= tpG.Step2)
			{
				SetUsedFlag(vCRs[CH_D][crD[i]], 1);
				bFindD = true;
			}
			else
			{
				crD.erase(crD.begin() + i);
			}
		}

		VINT crE;
		GetCR(CH_E, tpG.Step1 - 1, tpG.Row1 - 1, max(tpG.Step2 + 2, crFG.Step2), tpG.Row2 + 1, blocks, vCRs[CH_E], crE, -1, 2);
		for (int i = crE.size() - 1; i >= 0; --i)
		{
			if (GetDistance(vCRs[CH_E][crE[i]], tpG) <= 5 && vCRs[CH_E][crE[i]].Row2 >= tpG.Row2 && vCRs[CH_E][crE[i]].Step2 >= tpG.Step2 - 1)
			{
				SetUsedFlag(vCRs[CH_E][crE[i]], 1);
				bFindE = true;
			}
			else
			{
				crE.erase(crE.begin() + i);
			}
		}

		if (bFindD && bFindE/* && bFindF2*/)//也是一个螺孔
		{
			w.Type = W_DOUBLE_HOLE;
			AddWoundData(w, tpG);
			AddWoundData(w, vCRs[CH_D], crD);
			AddWoundData(w, vCRs[CH_E], crE);
			AddWoundData(w, vCRs[CH_F], crFt);

			PM pm;
			pm.Step2 = vCRs[CH_D][crD[0]].Step1;
			pm.Length = vCRs[CH_E][crE[0]].Step2 - vCRs[CH_D][crD[0]].Step1;
			Pos pos = FindStepInBlock(pm.Step2, blocks, 0);
			pm.Block = pos.Block;
			pm.Step = pos.Step;
			pm.Mark = PM_GUIDEHOLE;
			AddToMarks(pm, vPMs);
		}
		else
		{
			AddWoundData(w, tpG);
			AddWoundData(w, vCRs[CH_D], crD);
			AddWoundData(w, vCRs[CH_E], crE);
			AddWoundData(w, vCRs[CH_F], crFt);
		}
	}
	SetUsedFlag(vCRs[CH_F], crF, 1);
	SetUsedFlag(vCRs[CH_G], crG, 1);
	return w.Type > 0;
}



void	ParseScrewHoleSkew1(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleE, double& offsetE,
	CR& tempE2,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP)
{
	int idxE = tempE.Index;
	VINT crEt;
	uint8_t iFind_1 = 0;
	if (tempF.Step2 - tempF.Step1 < 10 || iScrewIndex == 0 || iScrewIndex == 1)
	{
		iFind_1 = GetCR(CH_E, tempF.Step1 - 10, tempF.Row1 - 3, tempF.Step2 + 5, tempF.Row2 + 3, blocks, vCRs[CH_E], crEt, idxE, 1, true);
	}
	else
	{
		iFind_1 = GetCR(CH_E, tempF.Step1, tempF.Row1 - 3, tempF.Step2, tempF.Row2 + 3, blocks, vCRs[CH_E], crEt, idxE, 1, true);
	}
	if (tempD.Step2 > 0)
	{
		for (int i = crEt.size() - 1; i >= 0; --i)
		{
			CR& tpE = vCRs[CH_E][crEt[i]];
			if (tpE.Step2 < tempD.Step1 || tpE.IsWound == 1 && tpE.Row2 - tempF.Row2 >= 4)
			{
				crEt.erase(crEt.begin() + i);
			}
		}
	}
	for (int k = 0; k < crEt.size(); ++k)
	{
		CR& tpE = vCRs[CH_E][crEt[k]];
		SetScrewHoleFlag(tpE, 1);
		if (tpE.Region.size() < 3 || tpE.Step1 > (tempF.Step1 + tempF.Step2) / 2 + 3)
		{
			continue;
		}

		double dh = GetDistanceHA(tempE, tpE);
		double dv = GetDistanceVA(tempE, tpE);
		int overlappedH = GetOverlappedCountH(tempE, tpE);
		int overlappedV = GetOverlappedCountV(tempE, tpE);
		uint8_t isSecond = IsScrewHoleSecondWave(tempE, tpE, blocks, angleE, offsetE, g_filehead.step);
		if (isSecond == 2)
		{
			SetUsedFlag(tpE, 1);
			continue;
		}
		else if (isSecond == 1)
		{
			if (overlappedH < 3 && overlappedV < 3)
			{
				SetUsedFlag(tpE, 1);
				continue;
			}
			else if (tpE.Row1 > tempE.Row1 && tpE.Row2 > tempE.Row2 && tpE.Step2 < tempE.Step2)
			{
				SetUsedFlag(tpE, 1);
				continue;
			}
			else
			{
				tpE.IsDoubleWave = 0;
				tpE.IsEnsureNotDoubleWave = 1;
			}

		}
		else if (overlappedH >= 3 && overlappedV >= 3 && tpE.Row1 > tempE.Row1 && tpE.Row2 > tempE.Row2 && tpE.Step2 < tempE.Step2)
		{
			tpE.IsDoubleWave = 1;
			SetUsedFlag(tpE, 1);
			continue;
		}

		if (bJoint && iScrewIndex == 1)
		{
			int iFirstRow = 0, iLastRow = 0;
			GetCRRowInfo2(tpE, iFirstRow, iLastRow, CH_E);
			if (iFirstRow >= iLastRow)
			{
				SetUsedFlag(tpE, 1);
				continue;
			}
		}


		int iEStep2 = (tpE.Step2 + tpE.Step1) >> 1;
		GetCRInfo(tpE, DataA, blocks);
		if (idxE >= 0)
		{
			int distWithE = GetDistance(tpE, tempE);
			if ((dh >= 5.0 && dh <= 11 || dh >= 5.0 && (dh <= 18 || dh > 18 && tpE.Step2 >= tempF.Step1) || tpE.Step1 + tpE.Step2 < tempF.Step1 + tempF.Step2) && (dv <= 5 || overlappedV >= 3))
			{
				if (iScrewIndex == -1 && (tpE.Step1 + tpE.Step2) <= (tempF.Step1 + tempF.Step2))
				{
					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = tpE.Block;
					w.Step = tpE.Step;
					w.Step2 = tpE.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔一象限斜裂纹", iScrewIndex);
					w.Type = W_SCREW_CRACK1;
					w.Place = WP_WAIST;
					w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
					w.SizeY = 1;
					AddWoundData(w, vCRs[CH_E], crEt); FillWound2(w, blocks);
					AddToWounds(vWounds, w);

					VINT crEE;
					GetCR(CH_E, tempF.Step1 - 20, tempF.Row1, tempF.Step2 + 2, iFRow - 4, blocks, vCRs[CH_E], crEE, idxE);
					for (int k = 0; k < crEE.size(); ++k)
					{
						CR& tpEE = vCRs[CH_E][crEE[k]];
						int iFirstRow = 0, iLastRow = 0;
						GetCRRowInfo2(tpEE, iFirstRow, iLastRow, CH_E);
						if (iFirstRow < iLastRow)
						{
							continue;
						}
						uint8_t isSecond = IsScrewHoleSecondWave(tpE, tpEE, blocks, angleE, offsetE, g_filehead.step);
						if (isSecond == 2)
						{
							SetUsedFlag(tpEE, 1);
							continue;
						}
					}
					break;
				}
				if ((tpE.Step1 + tpE.Step2 <= tempF.Step1 + tempF.Step2 + 3 || overlappedV > 3) && distWithE >= 4)
				{
					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = tpE.Block;
					w.Step = tpE.Step;
					w.Step2 = tpE.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔一象限斜裂纹", iScrewIndex);
					w.Type = W_SCREW_CRACK1;
					w.Place = WP_WAIST;
					w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
					w.SizeY = 1;
					AddWoundData(w, tpE);
					FillWound2(w, blocks);
					AddToWounds(vWounds, w);

					VINT crEE;
					GetCR(CH_E, tempF.Step1 - 20, tempF.Row1, tempF.Step2 + 2, iFRow - 4, blocks, vCRs[CH_E], crEE, idxE);
					for (int k = 0; k < crEE.size(); ++k)
					{
						CR& tpEE = vCRs[CH_E][crEE[k]];
						int iFirstRow = 0, iLastRow = 0;
						GetCRRowInfo2(tpEE, iFirstRow, iLastRow, CH_E);
						if (iFirstRow < iLastRow)
						{
							continue;
						}
						uint8_t isSecond = IsScrewHoleSecondWave(tpE, tpEE, blocks, angleE, offsetE, g_filehead.step);
						if (isSecond == 2)
						{
							SetUsedFlag(tpEE, 1);
							continue;
						}
					}
					break;
				}
			}
			else if (tempF.Step2 - tempF.Step1 >= 25)
			{
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tpE.Block;
				w.Step = tpE.Step;
				w.Step2 = tpE.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "套孔", iScrewIndex);
				w.Type = W_SCREW_CRACK1;
				w.Place = WP_WAIST;
				w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpE);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
			else if (dh < 18 && distWithE > 2)
			{
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tpE.Block;
				w.Step = tpE.Step;
				w.Step2 = tpE.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔一象限裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK1;
				w.Place = WP_WAIST;
				w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpE);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);

				VINT crEE;
				GetCR(CH_E, tempF.Step1 - 20, tempF.Row1, tempF.Step2 + 2, iFRow - 4, blocks, vCRs[CH_E], crEE, idxE);
				for (int k = 0; k < crEE.size(); ++k)
				{
					CR& tpEE = vCRs[CH_E][crEE[k]];
					int iFirstRow = 0, iLastRow = 0;
					GetCRRowInfo2(tpEE, iFirstRow, iLastRow, CH_E);
					if (iFirstRow < iLastRow)
					{
						continue;
					}
					uint8_t isSecond = IsScrewHoleSecondWave(tpE, tpEE, blocks, angleE, offsetE, g_filehead.step);
					if (isSecond == 2)
					{
						SetUsedFlag(tpEE, 1);
						continue;
					}
				}
				break;
			}
		}
		else
		{
			Wound_Judged w;
			w.IsScrewHole = iScrewIndex + 10;
			FillWound(w, blockHead, head);
			w.Block = tpE.Block;
			w.Step = tpE.Step;
			w.Step2 = tpE.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Degree = WD_SERIOUS;
			sprintf(w.Result, "%d孔一象限斜裂纹", iScrewIndex);
			w.Type = W_SCREW_CRACK1;
			w.Place = WP_WAIST;
			w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
			w.SizeY = 1;
			AddWoundData(w, tpE);
			FillWound2(w, blocks);
			AddToWounds(vWounds, w);

			VINT crEE;
			GetCR(CH_E, tempF.Step1 - 20, tempF.Row1, tempF.Step2 + 2, iFRow - 4, blocks, vCRs[CH_E], crEE, idxE);
			for (int k = 0; k < crEE.size(); ++k)
			{
				CR& tpEE = vCRs[CH_E][crEE[k]];
				int iFirstRow = 0, iLastRow = 0;
				GetCRRowInfo2(tpEE, iFirstRow, iLastRow, CH_E);
				if (iFirstRow < iLastRow)
				{
					continue;
				}
				uint8_t isSecond = IsScrewHoleSecondWave(tpE, tpEE, blocks, angleE, offsetE, g_filehead.step);
				if (isSecond == 2)
				{
					SetUsedFlag(tpEE, 1);
					continue;
				}
			}
			break;
		}
	}
	SetUsedFlag(vCRs[CH_E], crEt, 1);

}


void	ParseScrewHoleSkew2(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleD, double& offsetD,
	CR& tempD2,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP)
{
	int idxD = tempD.Index;
	VINT crDt;
	uint8_t iFindD_2 = GetCR(CH_D, tempF.Step1, tempF.Row1 - 2, tempF.Step2 + 5, tempF.Row2 + 2, blocks, vCRs[CH_D], crDt, idxD, 1);
	if (tempE.Step2 > 0)
	{
		for (int k = crDt.size() - 1; k >= 0; --k)
		{
			CR& tpD = vCRs[CH_D][crDt[k]];
			if (tpD.Step1 > tempE.Step2 || tpD.IsWound == 1 && tpD.Row2 - tempF.Row2 >= 4)
			{ 
				crDt.erase(crDt.begin() + k);
			}
		}
	}
	for (int k = 0; k < crDt.size(); ++k)
	{
		CR& tpD = vCRs[CH_D][crDt[k]];
		SetUsedFlag(tpD, 1);
		SetScrewHoleFlag(tpD, 1);
		if (tpD.Region.size() < 3)
		{
			continue;
		}

		int firstRow = 0, lastRow = 0;
		GetCRRowInfo2(tpD, firstRow, lastRow, CH_D);
		if (firstRow <= lastRow)
		{
			bJoint = true;
			continue;
		}

		double dh = GetDistanceHA(tempD, tpD);
		double dv = GetDistanceVA(tempD, tpD);
		int overlappedH = GetOverlappedCountH(tempD, tpD);
		int overlappedV = GetOverlappedCountV(tempD, tpD);
		uint8_t isSecond = IsScrewHoleSecondWave(tempD, tpD, blocks, angleD, offsetD, g_filehead.step);
		if (isSecond == 2)
		{
			SetUsedFlag(tpD, 1);
			continue;
		}
		else if (isSecond == 1)
		{
			if (overlappedH < 3 && overlappedV < 3)
			{
				SetUsedFlag(tpD, 1);
				continue;
			}
			else if (tpD.Row1 > tempD.Row1 && tpD.Row2 > tempD.Row2 && tpD.Step1 < tempD.Step1)
			{
				SetUsedFlag(tpD, 1);
				continue;
			}
			else
			{
				tpD.IsDoubleWave = 0;
				tpD.IsEnsureNotDoubleWave = 1;
			}

		}

		//if (bJoint && iScrewIndex == -1)
		//{


		GetCRInfo(tpD, DataA, blocks);
		if (idxD >= 0 && tpD.Step2 > (tempF.Step1 + tempF.Step2) / 2 - 3)
		{
			if ((dh >= 4.5 && dh <= 11 || dh >= 4.5 && (dh <= 18 || dh > 18 && tpD.Step1 <= tempF.Step2)))//11
			{
				if (tpD.Step2 < (tempF.Step1 + tempF.Step2) / 2)
				{
					continue;
				}
				if (tempD.Row2 > 0 && tpD.Row2 > tempD.Row2 + 3)
				{
					if (bJoint && iScrewIndex == 1)
					{
						if (tempD.Row2 <= tempF.Row2 + 1)
						{
							if (tpD.Row2 >= tempF.Row2 + 5)
							{
								continue;
							}
						}
						else
						{
							continue;
						}
					}
				}
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tpD.Block;
				w.Step = tpD.Step;
				w.Step2 = tpD.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔二象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK2;
				w.Place = WP_WAIST;
				w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpD);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);

				VINT crDD;
				if (bJoint && iScrewIndex == 1)
				{
					GetCR(CH_D, tempF.Step1 - 5, tempF.Row1, tempF.Step2 + 20, iFRow - 4, blocks, vCRs[CH_D], crDD, idxD);
				}
				else
				{
					GetCR(CH_D, tempF.Step1 - 2, tempF.Row1, tempF.Step2 + 20, iFRow - 4, blocks, vCRs[CH_D], crDD, idxD);
				}
				for (int k = 0; k < crDD.size(); ++k)
				{
					CR& tpDD = vCRs[CH_D][crDD[k]];
					int iFirstRow = 0, iLastRow = 0;
					GetCRRowInfo2(tpDD, iFirstRow, iLastRow, CH_D);
					if (iFirstRow < iLastRow)
					{
						continue;
					}
					uint8_t isSecond = IsScrewHoleSecondWave(tpD, tpDD, blocks, angleD, offsetD, g_filehead.step);
					if (isSecond == 2)
					{
						SetUsedFlag(tpDD, 1);
						continue;
					}
				}

				break;
			}
			else if (dh > 11 && dh < 18)
			{
				SetUsedFlag(tpD, 1);
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tpD.Block;
				w.Step = tpD.Step;
				w.Step2 = tpD.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "套孔", iScrewIndex);
				w.Type = W_SCREW_CRACK2;
				w.Place = WP_WAIST;
				w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpD);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
		else if (idxD < 0)
		{
			bool bwound = false;
			double d = GetDistance(tpD, tempF);
			if (d >= 2)
			{
				bwound = false;
			}
			else if (tpD.Step1 >= (tempF.Step1 + tempF.Step2) / 2 - 1 || tpD.Step2 >= tempE.Step1)//11
			{
				bwound = true;
			}
			else
			{
				int bs = 0, es = 0;
				int os = GetOverlappedStep(tempF.Step1, tempF.Step2, tpD.Step1, tpD.Step2, bs, es);
				if (tempF.Step2 - tempF.Step1 >= 12 && tpD.Step1 < tempF.Step1)
				{
					bwound = false;
				}
				else if (os <= 0)
				{
					bwound = true;
				}
				else
				{
					int r1 = tpD.Row2;
					for (int k = 0; k < tpD.Region.size(); ++k)
					{
						if (tpD.Region[k].step == bs && tpD.Region[k].row < r1)
						{
							r1 = tpD.Region[k].row;
						}
					}
					double deltR = 1.0 * (tempF.Row1 - r1) * 12 / (tempF.Step2 - tempF.Step1);
					if (deltR < -3)
					{
						bwound = true;
					}
				}
			}
			if (bwound)
			{
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tpD.Block;
				w.Step = tpD.Step;
				w.Step2 = tpD.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔二象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK2;
				w.Place = WP_WAIST;
				w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpD);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);

				VINT crDD;
				if (bJoint && iScrewIndex == 1)
				{
					GetCR(CH_D, tempF.Step1 - 5, tempF.Row1, tempF.Step2 + 20, iFRow - 4, blocks, vCRs[CH_D], crDD, idxD);
				}
				else
				{
					GetCR(CH_D, tempF.Step1 - 2, tempF.Row1, tempF.Step2 + 20, iFRow - 4, blocks, vCRs[CH_D], crDD, idxD);
				}
				for (int k = 0; k < crDD.size(); ++k)
				{
					CR& tpDD = vCRs[CH_D][crDD[k]];
					int iFirstRow = 0, iLastRow = 0;
					GetCRRowInfo2(tpDD, iFirstRow, iLastRow, CH_D);
					if (iFirstRow < iLastRow)
					{
						continue;
					}
					uint8_t isSecond = IsScrewHoleSecondWave(tpD, tpDD, blocks, angleD, offsetD, g_filehead.step);
					if (isSecond == 2)
					{
						SetUsedFlag(tpDD, 1);
						continue;
					}
				}
			}
		}
	}
}


void	ParseScrewHoleSkew3(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleE, double& offsetE,
	CR& tempE2,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP)
{
	int idxE = tempE.Index;
	VINT crEt, crDt;
	uint8_t iFind_3 = GetCR(CH_E, tempF.Step2 - 8, tempF.Row2 + 1, tempF.Step2 + 5, tempF.Row2 + 10, blocks, vCRs[CH_E], crEt, idxE, 1, true);
	if (idxE < 0)
	{
		for (int k = 0; k < crEt.size(); ++k)
		{
			CR& tpE = vCRs[CH_E][crEt[k]];
			SetScrewHoleFlag(tpE, 1);
			if (tpE.Region.size() < 3)
			{
				SetUsedFlag(tpE, 1);
				continue;
			}
			if ((tpE.Row1 + tpE.Row2) / 2 >= (tempF.Row1 + tempF.Row2) / 2 + 3 && tpE.Row1 >= tempF.Row2 + 3 && tpE.Row2 >= tempD.Row2 + 2)
			{
				SetUsedFlag(tpE, 1);
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tpE.Block;
				w.Step = tpE.Step;
				w.Step2 = tpE.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔三象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK3;
				if (iScrewIndex == -1 && bJoint)
				{
					sprintf(w.Result, "%d孔四象限斜裂纹", 1);
					w.Type = W_SCREW_CRACK4;
				}
				w.Place = WP_WAIST;
				w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpE);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
	}
	else
	{
		for (int k = 0; k < crEt.size(); ++k)
		{
			CR& tpE = vCRs[CH_E][crEt[k]];
			SetScrewHoleFlag(tpE, 1);
			if (tpE.Region.size() <= 3 || (tpE.Index == tempE2.Index && tpE.Region.size() == tempE2.Region.size()))
			{
				SetUsedFlag(tpE, 1);
				continue;
			}

			int overlappedH = GetOverlappedCountH(tempE2, tpE);
			int overlappedV = GetOverlappedCountV(tempE2, tpE);
			uint8_t isSecond = IsScrewHoleSecondWave(tempE, tpE, blocks, angleE, offsetE, g_filehead.step);
			if (isSecond == 2)
			{
				SetUsedFlag(tpE, 1);
				continue;
			}
			else if (isSecond == 1)
			{
				if (overlappedH < 3 && overlappedV < 3)
				{
					SetUsedFlag(tpE, 1);
					continue;
				}
				//else if (tpE.Row1 > tempE.Row1 && tpE.Row2 > tempE.Row2 && tpE.Step2 < tempE.Step2)
				//{
				//	SetUsedFlag(tpE, 1);
				//	continue;
				//}
				else
				{
					tpE.IsDoubleWave = 0;
					tpE.IsEnsureNotDoubleWave = 1;
				}

			}
			else if (overlappedH >= 3 && overlappedV >= 3 && tpE.Row1 > tempE.Row1 && tpE.Row2 > tempE.Row2 && tpE.Step2 < tempE.Step2)
			{
				tpE.IsDoubleWave = 1;
				SetUsedFlag(tpE, 1);
				continue;
			}

			int firstRow, lastRow;
			GetCRRowInfo2(tpE, firstRow, lastRow, CH_E);
			double dv = GetDistanceVA(tempE2, tpE);
			GetCRInfo(tpE, DataA, blocks);
			if (dv <= 3.0)
			{
				if ((tpE.Step2 > tempE.Step2 && tpE.Row2 > tempE.Row2) ||
					(overlappedH >= 2 && tpE.Row1 > tempF.Row2 && tpE.Row2 > tempE.Row2)
					)
				{
					SetUsedFlag(tpE, 1);
					if (tpE.Row1 < tpE.Row2)
					{
						bool isWound = true;
						if (Abs(lastHP.step2 - tpE.Step2) <= 100 && tpE.Row2 <= lastHP.tempE.Row2 && lastHP.tempE.Region.size() >= 5)
						{
							isWound = false;
						}
						if (isWound)
						{
							Wound_Judged w;
							w.IsGuideHole = 1;
							w.Block = tpE.Block;
							w.Step = tpE.Step;
							w.Step2 = tpE.Step1;
							w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
							w.Place = WP_WAIST;
							FillWound(w, blockHead, head);
							w.Type = W_SCREW_HORIZON_CRACK_LEFT;
							w.Degree = WD_SERIOUS;
							memcpy(w.Result, "螺孔水平裂纹", 30);
							AddWoundData(w, tpE);
							w.SizeX = head.step * (tpE.Step2 - tpE.Step1) / 0.8;
							w.SizeY = 1;
							vWounds.emplace_back(w);
						}
					}
				}
			}
			else if (dv >= 3.0 && dv <= 13 && (overlappedH >= 2 || tpE.Step1 <= tempF.Step2))
			{
				if (tpE.Row1 < tempF.Row1 && tpE.Step2 < tempE.Step1)
				{
					SetUsedFlag(tpE, 1);
					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = tpE.Block;
					w.Step = tpE.Step;
					w.Step2 = tpE.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔一象限斜裂纹", iScrewIndex);
					w.Type = W_SCREW_CRACK1;
					w.Place = WP_WAIST;
					w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
					w.SizeY = 1;
					AddWoundData(w, tpE);
					FillWound2(w, blocks);
					AddToWounds(vWounds, w);
				}
				else if ((tpE.Row1 >= tempF.Row2 + 2 || tpE.Row2 - tpE.Row1 >= 8 && tpE.Row2 >= tempF.Row2 + 5) && tpE.Row2 >= tempD.Row2 + 2)
				{
					SetUsedFlag(tpE, 1);
					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = tpE.Block;
					w.Step = tpE.Step;
					w.Step2 = tpE.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔三象限斜裂纹", iScrewIndex);
					w.Type = W_SCREW_CRACK3;
					if (iScrewIndex == -1 && bJoint)
					{
						sprintf(w.Result, "%d孔四象限斜裂纹", 1);
						w.Type = W_SCREW_CRACK4;
					}

					w.Place = WP_WAIST;
					w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
					w.SizeY = 1;
					AddWoundData(w, tpE); FillWound2(w, blocks);
					AddToWounds(vWounds, w);
				}
			}
		}
	}
}


void	ParseScrewHoleSkew4(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleD, double& offsetD,
	CR& tempD2,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP)
{
	int idxD = tempD.Index;
	VINT crEt, crDt;

	uint8_t iFindD_4 = GetCR(CH_D, tempF.Step1 - 8, tempF.Row2 + 1, tempF.Step1 + 3, tempF.Row2 + 10, blocks, vCRs[CH_D], crDt, idxD, 1, true);
	if (idxD < 0)
	{
		for (int k = 0; k < crDt.size(); ++k)
		{
			CR& tpD = vCRs[CH_D][crDt[k]];
			SetScrewHoleFlag(tpD, 1);
			if (tpD.Region.size() < 3)
			{
				SetUsedFlag(tpD, 1);
				continue;
			}
			if ((tpD.Row1 + tpD.Row2) / 2 >= (tempF.Row1 + tempF.Row2) / 2 + 3 && tpD.Row1 >= tempF.Row2 + 2 && tpD.Row2 >= tempF.Row2 + 5 && tpD.Row2 >= tempE.Row2 + 2)
			{
				SetUsedFlag(tpD, 1);
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tpD.Block;
				w.Step = tpD.Step;
				w.Step2 = tpD.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔四象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK4;
				if (iScrewIndex == 1 && bJoint)
				{
					sprintf(w.Result, "%d孔三象限斜裂纹", -1);
					w.Type = W_SCREW_CRACK3;
				}
				w.Place = WP_WAIST;
				w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpD); FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
	}
	else
	{
		for (int k = 0; k < crDt.size(); ++k)
		{
			CR& tpD = vCRs[CH_D][crDt[k]];
			SetScrewHoleFlag(tpD, 1);
			if (tpD.Region.size() <= 3 || tpD.Index == tempD2.Index)
			{
				SetUsedFlag(tpD, 1);
				continue;
			}

			int overlappedH = GetOverlappedCountH(tempD2, tpD);
			int overlappedV = GetOverlappedCountV(tempD2, tpD);
			uint8_t isSecond = IsScrewHoleSecondWave(tempD, tpD, blocks, angleD, offsetD, g_filehead.step);
			if (isSecond == 2)
			{
				SetUsedFlag(tpD, 1);
				continue;
			}
			else if (isSecond == 1)
			{
				if (overlappedH < 3 && overlappedV < 3)
				{
					SetUsedFlag(tpD, 1);
					continue;
				}
				//else if (tpD.Row1 > tempD.Row1 && tpD.Row2 > tempD.Row2 && tpD.Step1 > tempD.Step1)
				//{
				//	SetUsedFlag(tpD, 1);
				//	continue;
				//}
				else
				{
					tpD.IsDoubleWave = 0;
					tpD.IsEnsureNotDoubleWave = 1;
				}
			}
			else if (overlappedH >= 3 && overlappedV >= 3 && tpD.Row1 > tempD.Row1 && tpD.Row2 > tempD.Row2 && tpD.Step1 > tempD.Step1)
			{
				tpD.IsDoubleWave = 1;
				SetUsedFlag(tpD, 1);
				continue;
			}

			GetCRInfo(tpD, DataA, blocks);
			double dv = GetDistanceVA(tempD2, tpD);
			int distWithD = GetDistance(tempD, tpD);
			if (dv > 3 && dv < 10 && tpD.Step1 < tempD.Step1 && tpD.Row1 > tempD.Row1)
			{
				double k = 1.0 * (tpD.Row2 - tempD.Row1)/ (tempD.Step2 - tpD.Step1);
				if (k < 1.5 && k > 0.3)
				{
					SetUsedFlag(tpD, 1);
					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = tpD.Block;
					w.Step = tpD.Step;
					w.Step2 = tpD.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔四象限斜裂纹", iScrewIndex);
					w.Type = W_SCREW_CRACK4;
					if (iScrewIndex == 1 && bJoint)
					{
						sprintf(w.Result, "%d孔三象限斜裂纹", -1);
						w.Type = W_SCREW_CRACK3;
					}
					w.Place = WP_WAIST;
					w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
					w.SizeY = 1;
					AddWoundData(w, tpD); FillWound2(w, blocks);
					AddToWounds(vWounds, w);
					break;
				}
			}
			if (dv < 3.0 && overlappedH >= 2)
			{
				if (tpD.Row1 > tempF.Row2 && tpD.Row2 > tempD.Row2)
				{
					SetUsedFlag(tpD, 1);
					bool isWound = true;
					if (Abs(lastHP.step2 - tpD.Step2) <= 100 && tpD.Row2 <= lastHP.tempD.Row2 && lastHP.tempD.Region.size() >= 5)
					{
						isWound = false;
					}
					if (isWound)
					{
						Wound_Judged w;
						w.IsScrewHole = iScrewIndex + 10;
						FillWound(w, blockHead, head);
						w.Block = tpD.Block;
						w.Step = tpD.Step;
						w.Step2 = tpD.Step1;
						w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
						w.Degree = WD_SERIOUS;
						sprintf(w.Result, "%d孔左侧水平裂纹", iScrewIndex);
						w.Type = W_SCREW_HORIZON_CRACK_LEFT;
						w.Place = WP_WAIST;
						w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
						w.SizeY = 1;
						AddWoundData(w, tpD); FillWound2(w, blocks);
						AddToWounds(vWounds, w);
					}
				}
			}
			if (dv >= 3.0 && dv <= 12 && (overlappedH >= 2 || tpD.Step2 >= tempF.Step1))
			{
				if (tpD.Row1 < tempF.Row1 && distWithD > 2)
				{
					SetUsedFlag(tpD, 1);
					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = tpD.Block;
					w.Step = tpD.Step;
					w.Step2 = tpD.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔二象限斜裂纹", iScrewIndex);
					w.Type = W_SCREW_CRACK2;
					w.Place = WP_WAIST;
					w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
					w.SizeY = 1;
					AddWoundData(w, tpD); FillWound2(w, blocks);
					AddToWounds(vWounds, w);
					break;
				}
				else if (tpD.Row2 >= tempE.Row2 + 2 && tpD.Row1 > tempF.Row2)
				{
					SetUsedFlag(tpD, 1);
					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = tpD.Block;
					w.Step = tpD.Step;
					w.Step2 = tpD.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔四象限斜裂纹", iScrewIndex);
					w.Type = W_SCREW_CRACK4;
					if (iScrewIndex == 1 && bJoint)
					{
						sprintf(w.Result, "%d孔三象限斜裂纹", -1);
						w.Type = W_SCREW_CRACK3;
					}
					w.Place = WP_WAIST;
					w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
					w.SizeY = 1;
					AddWoundData(w, tpD); FillWound2(w, blocks);
					AddToWounds(vWounds, w);
					break;
				}
			}
		}
	}
}

void	ParseReverseWaves(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleD, double& offsetD,
	CR& tempD2,
	int& iLoseCh, VINT& vF,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP)
{
	int idxD = tempD.Index, idxE = tempE.Index;
	int stepF1 = tempF.Step1, stepF2 = tempF.Step2;
	int iDLeft = (iScrewIndex == 1) ? ((stepF1 + stepF2) >> 1) : (stepF1 + 5);
	int iERight = (iScrewIndex == -1) ? ((stepF1 + stepF2) >> 1) : (stepF2 - 3);

	if (bJoint && iScrewIndex == -1 || iScrewIndex == 0 && GetNearestHeavyStepLen(tempF.Step) < 100)//需考虑倒打
	{
		VINT crDDD;
		bool bFinddddd = GetCR(CH_D, iERight, iLuokong_D_Row1_L[railType] + 3, stepF2 + 15, iLuokong_D_Row1_H[railType] + 10, blocks, vCRs[CH_D], crDDD, idxD, 1);
		for (int i = 0; i < crDDD.size(); ++i)
		{
			CR& td = vCRs[CH_D][crDDD[i]];
			SetScrewHoleFlag(td, 1);
			int lastRow = 0, firstRow = 0, lastRow2 = 0, firstRow2 = 0;
			GetCRRowInfo3(td, firstRow, lastRow, CH_E, firstRow2, lastRow2);
			int dF = td.Row1 - tempF.Row2;
			if (td.Region.size() >= 3 && firstRow < lastRow && td.Row1 > tempF.Row2 + 2 && dF <= 10 && td.Step2 >= (tempF.Step1 + tempF.Step2) / 2 && td.Row2 >= tempF.Row2 + 5 && td.Row2 >= tempD.Row2 + 2)
			{
				Wound_Judged w;
				w.IsReversed = 1;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = td.Block;
				w.Step = td.Step;
				w.Step2 = td.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔倒打三象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK3;
				w.Place = WP_WAIST;
				w.SizeX = (td.Row2 - td.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, td);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
		SetUsedFlag(vCRs[CH_D], crDDD, 1);

		VINT crEEE;
		bool bFindeeee = GetCR(CH_E, iERight, iLuokong_D_Row1_L[railType] + 3, stepF2 + 15, iLuokong_D_Row1_H[railType] + 10, blocks, vCRs[CH_E], crEEE, idxE, 1, true);
		for (int i = 0; i < crEEE.size(); ++i)
		{
			CR& te = vCRs[CH_E][crEEE[i]];
			SetScrewHoleFlag(te, 1);
			int lastRow = 0, firstRow = 0;
			GetCRRowInfo2(te, firstRow, lastRow, CH_D);
			if (te.Region.size() >= 3 && firstRow <= lastRow + 1 && te.Row1 > vCRs[iLoseCh][vF[0]].Row2 + 2 && te.Row2 >= tempF.Row2 + 5 && te.Row2 >= tempD.Row2 + 2)
			{
				Wound_Judged w;
				w.IsReversed = 1;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = te.Block;
				w.Step = te.Step;
				w.Step2 = te.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "1孔倒打四象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK4;
				w.Place = WP_WAIST;
				w.SizeX = (te.Row2 - te.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, te);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
		SetUsedFlag(vCRs[CH_E], crEEE, 1);
	}
	else if (bJoint && iScrewIndex == 1 || iScrewIndex == 0 && GetNearestHeavyStepLen(tempF.Step) < 100)//需考虑倒打
	{
		VINT crDDD;
		bool bFindddd = GetCR(CH_D, stepF1 - 15, tempF.Row2 + 2, iDLeft, tempF.Row2 + 15, blocks, vCRs[CH_D], crDDD, idxD, 1, true);
		for (int i = 0; i < crDDD.size(); ++i)
		{
			if (vCRs[CH_D][crDDD[i]].Region.size() < 3)
			{
				continue;
			}
			CR& td = vCRs[CH_D][crDDD[i]];
			SetScrewHoleFlag(td, 1);
			int lastRow = 0, firstRow = 0;
			GetCRRowInfo2(td, firstRow, lastRow, CH_E);
			if (td.Region.size() > 3 && firstRow >= lastRow - 1 && td.Row1 > tempF.Row2 + 2 && td.Row2 >= tempF.Row2 + 5 && td.Row2 >= tempE.Row2 + 2)
			{
				Wound_Judged w;
				w.IsReversed = 1;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = td.Block;
				w.Step = td.Step;
				w.Step2 = td.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔倒打四象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK4;
				w.Place = WP_WAIST;
				w.SizeX = (td.Row2 - td.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, td);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
		SetUsedFlag(vCRs[CH_D], crDDD, 1);

		VINT crEEE;
		bool bFindeeee = GetCR(CH_E, stepF1 - 15, tempF.Row2 + 2, iDLeft, tempF.Row2 + 15, blocks, vCRs[CH_E], crEEE, idxE, 1, true);
		for (int i = 0; i < crEEE.size(); ++i)
		{
			if (vCRs[CH_E][crEEE[i]].Region.size() < 3)
			{
				continue;
			}
			CR& te = vCRs[CH_E][crEEE[i]];
			SetScrewHoleFlag(te, 1);
			int lastRow = 0, firstRow = 0, lastRow2 = 0, firstRow2 = 0;;
			GetCRRowInfo3(te, firstRow, lastRow, CH_D, firstRow2, lastRow2);
			//if(firstRow2 < lastRow2)
			if (te.Region.size() >= 3 && firstRow >= lastRow - 1 && firstRow2 >= lastRow2 && te.Row1 > tempF.Row2 + 2 && te.Row2 >= tempF.Row2 + 5 && te.Row2 >= tempD.Row2 + 2)
			{
				Wound_Judged w;
				w.IsReversed = 1;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = te.Block;
				w.Step = te.Step;
				w.Step2 = te.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔倒打四象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK4;
				w.Place = WP_WAIST;
				w.SizeX = (te.Row2 - te.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, te);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
		SetUsedFlag(vCRs[CH_E], crEEE, 1);
	}
}



