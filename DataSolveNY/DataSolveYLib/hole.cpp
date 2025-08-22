#include "stdafx.h"
#include "hole.h"
#include "holeCore.h"

#include "CRHPE.h"
#include "PublicFunc.h"
#include "GlobalDefine.h"
#include "Judge.h"

bool	IsExistScrewHole(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex)
{
	VINT crD, crE, crF, crG, crF2, crG2;
#pragma region FG通道的出波
	uint8_t bFindF = GetCR(CH_F, step1, g_iScrewHoleFRowL[railType] - 1, step2, g_iScrewHoleFRowH[railType] + 1, blocks, vCRs[CH_F], crF2, -1, 2, true);//F螺孔高度出波
	uint8_t bFindG = GetCR(CH_G, step1, g_iScrewHoleFRowL[railType] - 1, step2, g_iScrewHoleFRowH[railType] + 1, blocks, vCRs[CH_G], crG2, -1, 2, true);//G螺孔高度出波
	if (!bFindF && !bFindG && (!bJoint && !bSew))
	{
		if (bFindF && !bFindG)
		{
			int s1 = vCRs[CH_F][crF2[0]].Step1;
			int s2 = vCRs[CH_F][crF2[0]].Step2;
			for (int i = 1; i < crF2.size(); ++i)
			{
				if (vCRs[CH_F][crF2[i]].Step1 < s1)
				{
					s1 = vCRs[CH_F][crF2[i]].Step1;
				}
				if (vCRs[CH_F][crF2[i]].Step2 > s2)
				{
					s2 = vCRs[CH_F][crF2[i]].Step2;
				}
			}
			bFindG = GetCR(CH_G, s1, g_iScrewHoleFRowL[railType] - 1, s2, g_iScrewHoleFRowH[railType] + 1, blocks, vCRs[CH_G], crG2, -1, 2, true);//G螺孔高度出波
		}
		else if (!bFindF && bFindG)
		{
			int s1 = vCRs[CH_G][crG2[0]].Step1;
			int s2 = vCRs[CH_G][crG2[0]].Step2;
			for (int i = 1; i < crG2.size(); ++i)
			{
				if (vCRs[CH_G][crG2[i]].Step1 < s1)
				{
					s1 = vCRs[CH_G][crG2[i]].Step1;
				}
				if (vCRs[CH_G][crG2[i]].Step2 > s2)
				{
					s2 = vCRs[CH_G][crG2[i]].Step2;
				}
			}
			bFindF = GetCR(CH_F, s1, g_iScrewHoleFRowL[railType] - 1, s2, g_iScrewHoleFRowH[railType] + 1, blocks, vCRs[CH_F], crF2, -1, 2, true);//F螺孔高度出波
		}
		return false;
	}
#pragma endregion

	VINT vF, vG;
	uint8_t iLoseCh = 0;
	if (bFindF && vCRs[CH_F][crF2[0]].Step2 - vCRs[CH_F][crF2[0]].Step1 <= 30 && vCRs[CH_F][crF2[0]].Region.size() > 5)
	{
		vF = crF2;
		iLoseCh = CH_F;
		vG = crG2;
	}
	else if (bFindG && vCRs[CH_G][crG2[0]].Step2 - vCRs[CH_G][crG2[0]].Step1 <= 30 && vCRs[CH_G][crG2[0]].Region.size() > 5)
	{
		vF = crG2;
		iLoseCh = CH_G;
		vG = crF2;
	}
	else
	{
		return ParseScrewHoleNoFG(head, DataA, blocks, vCRs, step1, step2, iFRow, railType, vWounds, vPMs, bJoint, bAutoJoint, bSew, iScrewIndex);
	}

	VINT crFDown, crGDown;
	GetCR(CH_F, step1, g_iJawRow[railType] + 1, step2, iFRow - 10, blocks, vCRs[CH_F], crFDown);
	GetCR(CH_G, step1, g_iJawRow[railType] + 1, step2, iFRow - 10, blocks, vCRs[CH_G], crGDown);
	if (crFDown.size() == 0 && crGDown.size() == 0)
	{
		return false;
	}

	int iIndexF = -1;
	int iMinRowF = 100;
	for (int i = 0; i < vF.size(); ++i)
	{
		if (iLastScrewHoleRailType == railType && (vCRs[iLoseCh][vF[i]].Row2 < iLastScrewHoleRow - 2 || vCRs[iLoseCh][vF[i]].Row1 > iLastScrewHoleRow + 2) && iLastScrewHoleRow != 0)
		{
			if (vCRs[iLoseCh][vF[i]].Row2 > g_iScrewHoleFRowH[railType] + 1 || vCRs[iLoseCh][vF[i]].Row1 < g_iScrewHoleFRowL[railType] - 1)
			{
				continue;
			}
		}
		if (iLastScrewHoleIndex != 0 && abs(step1 - iLastScrewHoleRailType) <= 300 && (vCRs[iLoseCh][vF[i]].Row2 - iLastScrewHoleRow >= 3 || iLastScrewHoleRow - vCRs[iLoseCh][vF[i]].Row1 >= 3))
		{
			continue;
		}

		if (iIndexF == -1)
		{
			iIndexF = i;
			iMinRowF = vCRs[iLoseCh][vF[i]].Row1;
			continue;
		}

		if (vCRs[iLoseCh][vF[i]].Row1 < vCRs[iLoseCh][vF[iIndexF]].Row1)
		{
			iIndexF = i;
			iMinRowF = vCRs[iLoseCh][vF[i]].Row1;
		}
	}

	if (iIndexF < 0)
	{
		return false;
	}

	CR&  tempF = vCRs[iLoseCh][vF[iIndexF]];
	for (int i = 0; i < vF.size(); ++i)
	{
		if (vCRs[iLoseCh][vF[i]].Row1 >= tempF.Row1 - 1 && vCRs[iLoseCh][vF[i]].Row2 <= tempF.Row2 + 1 && i != iIndexF && vF[i] != vF[iIndexF])
		{
			Combine(tempF, vCRs[iLoseCh][vF[i]]);
		}
	}

	int tempChannel = CH_F + CH_G - iLoseCh;
	for (int i = 0; i < vG.size(); ++i)
	{
		if (vCRs[tempChannel][vG[i]].Row1 >= tempF.Row1 - 1 && vCRs[tempChannel][vG[i]].Row2 <= tempF.Row2 + 1)
		{
			Combine(tempF, vCRs[tempChannel][vG[i]]);
		}
	}

	iLastScrewHoleStep = tempF.Step1;
	iLastScrewHoleRow = tempF.Row1;
	iLastScrewHoleRailType = railType;
	iLastScrewHoleIndex = iScrewIndex;

	PM pm;
	memset(&pm, 0, sizeof(pm));
	pm.Block = tempF.Block;
	pm.Step = tempF.Step;
	pm.Step2 = tempF.Step1;
	//pm.Walk = GetWD(blocks[pm.Block - g_iBeginBlock].BlockHead.walk, pm.Block, pm.Step, g_direction);
	pm.Mark = PM_SCREWHOLE;
	AddToMarks(pm, vPMs);
	return true;
}

bool	ParseScrewHole(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex)
{
	VINT crD, crE, crF, crG, crF2, crG2;
	int igapF = 0, igapG = 0;
	uint8_t bLoseF = 0, bLoseG = 0, bFindF = 0, bFindG = 0;
	uint8_t iR1 = g_iScrewHoleFRowL[railType], iR2 = g_iScrewHoleFRowH[railType];
	if (Abs(step1 - iLastScrewHoleStep) <= 100)
	{
		iR1 = min(iLastScrewHoleRow, iR1);
		iR2 = max(iLastScrewHoleRow2, iR2);
	}

	HolePara lastHP;
	GetNearestHole(step1, PM_SCREWHOLE, lastHP);

	int iLoseGap;

#ifdef _DEBUG
	Pos posss = FindStepInBlock(step1, blocks, 0);
#endif // _DEBUG


#pragma region FG失波检测
	bLoseF = GetCR(CH_F, step1, iFRow - 3, step2, iFRow + 20, blocks, vCRs[CH_F], crF);
	bLoseG = GetCR(CH_G, step1, iFRow - 3, step2, iFRow + 20, blocks, vCRs[CH_G], crG);
	if (bLoseF && !bLoseG)
	{
		int s1 = vCRs[CH_F][crF[0]].Step1, s2 = vCRs[CH_F][crF[0]].Step2;
		uint8_t r1 = vCRs[CH_F][crF[0]].Row1, r2 = vCRs[CH_F][crF[0]].Row2;
		for (int i = 1; i < crF.size(); ++i)
		{
			if (vCRs[CH_F][crF[i]].Step1 < s1)
			{
				s1 = vCRs[CH_F][crF[i]].Step1;
			}
			if (vCRs[CH_F][crF[i]].Step2 > s2)
			{
				s2 = vCRs[CH_F][crF[i]].Step2;
			}
		}
		bLoseG = GetCR(CH_G, s1, r1 - 1, s2, r2 + 10, blocks, vCRs[CH_G], crG, -1, 2, true);//G螺孔高度出波
	}
	else if (!bLoseF && bLoseG)
	{
		int s1 = vCRs[CH_G][crG[0]].Step1, s2 = vCRs[CH_G][crG[0]].Step2;
		uint8_t r1 = vCRs[CH_G][crG[0]].Row1, r2 = vCRs[CH_G][crG[0]].Row2;
		for (int i = 1; i < crG.size(); ++i)
		{
			if (vCRs[CH_G][crG[i]].Step1 < s1)
			{
				s1 = vCRs[CH_G][crG[i]].Step1;
			}
			if (vCRs[CH_G][crG[i]].Step2 > s2)
			{
				s2 = vCRs[CH_G][crG[i]].Step2;
			}
		}
		bLoseF = GetCR(CH_F, s1, r1 - 1, s2, r2 + 10, blocks, vCRs[CH_F], crF, -1, 2, true);//G螺孔高度出波
	}
	for (int i = crF.size() - 1; i >= 0; --i)
	{
		int bs = 0x7FFFFFFF, es = 0;
		CR& tpF = vCRs[CH_F][crF[i]];
		if (GetOverlappedStep(tpF.Step1, tpF.Step2, step1 - 10, step2 + 10, bs, es) <= 0)
		{
			crF.erase(crF.begin() + i);
		}
	}
	bLoseF = crF.size() > 0;
	for (int i = crG.size() - 1; i >= 0; --i)
	{
		int bs = 0x7FFFFFFF, es = 0;
		CR& tpG = vCRs[CH_G][crG[i]];
		if (GetOverlappedStep(tpG.Step1, tpG.Step2, step1 - 10, step2 + 10, bs, es) <= 0)
		{
			crG.erase(crG.begin() + i);
		}
	}
	bLoseG = crG.size() > 0;
	if (!bLoseF || !bLoseG)
	{
		return false;
	}

	if (crF.size() > 1 || crG.size() > 1)
	{
		if (crF.size() > 1)
		{
			for (int i = crF.size() - 1; i >= 0; --i)
			{
				crF2.clear();
				crG2.clear();
				GetCR(CH_F, vCRs[CH_F][crF[i]].Step1, iR1 - 3, vCRs[CH_F][crF[i]].Step2, iR2 + 3, blocks, vCRs[CH_F], crF2, -1, 2, true);//G螺孔高度出波
				GetCR(CH_G, vCRs[CH_F][crF[i]].Step1, iR1 - 3, vCRs[CH_F][crF[i]].Step2, iR2 + 3, blocks, vCRs[CH_G], crG2, -1, 2, true);//G螺孔高度出波
				if (crF2.size() == 0 && crG2.size() == 0)
				{
					crF.erase(crF.begin() + i);
				}
			}
		}

		if (crG.size() > 1)
		{
			for (int i = crG.size() - 1; i >= 0; --i)
			{
				crF2.clear();
				crG2.clear();
				GetCR(CH_F, vCRs[CH_G][crG[i]].Step1, iR1 - 3, vCRs[CH_G][crG[i]].Step2, iR2 + 3, blocks, vCRs[CH_F], crF2, -1, 2, true);//G螺孔高度出波
				GetCR(CH_G, vCRs[CH_G][crG[i]].Step1, iR1 - 3, vCRs[CH_G][crG[i]].Step2, iR2 + 3, blocks, vCRs[CH_G], crG2, -1, 2, true);//G螺孔高度出波
				if (crF2.size() == 0 && crG2.size() == 0)
				{
					crG.erase(crG.begin() + i);
				}
			}
		}
	}

	for (int i = 0; i < crF.size(); ++i)
	{
		if (vCRs[CH_F][crF[i]].Step2 - vCRs[CH_F][crF[i]].Step1 > igapF)
		{
			igapF = vCRs[CH_F][crF[i]].Step2 - vCRs[CH_F][crF[i]].Step1;
		}
	}
	for (int i = 0; i < crG.size(); ++i)
	{
		if (vCRs[CH_G][crG[i]].Step2 - vCRs[CH_G][crG[i]].Step1 > igapG)
		{
			igapG = vCRs[CH_G][crG[i]].Step2 - vCRs[CH_G][crG[i]].Step1;
		}
	}
	if ((igapF < 4 || igapG < 4) && ((igapF <= 5 || igapF >= 30) && (igapG <= 5 || igapG >= 30)) && iScrewIndex == 0)
	{
		VINT crD;
		GetCR(CH_D, step1 - 3, g_iJawRow[railType], step2, iFRow - 10, blocks, vCRs[CH_D], crD, -1, 1, true);
		if (crD.size() > 0)
		{
			CR cD = vCRs[CH_D][crD[0]];
			return ParseGuideHole(head, DataA, blocks, vCRs, cD, cD.Index, iFRow, railType, vWounds, vPMs);
		}
		else
		{
			VINT crE;
			GetCR(CH_E, step1, g_iJawRow[railType], step2 + 3, iFRow - 10, blocks, vCRs[CH_E], crE, -1, 1, true);
			if (crE.size() > 0)
			{
				CR cE = vCRs[CH_E][crE[0]];
				return ParseGuideHole(head, DataA, blocks, vCRs, cE, cE.Index, iFRow, railType, vWounds, vPMs);
			}
		}
	}

	iLoseGap = (igapF <= igapG ? igapF : igapG);
#pragma endregion

#pragma region 在失波的步进检测出波

	int sLoseSteps[4] = { 0x7FFFFFFF, 0, 0x7FFFFFFF, 0 };
	for (int i = 0; i < crF.size(); ++i)
	{
		sLoseSteps[0] = min(sLoseSteps[0], vCRs[CH_F][crF[i]].Step1);
		sLoseSteps[1] = max(sLoseSteps[1], vCRs[CH_F][crF[i]].Step2);
	}
	for (int i = 0; i < crG.size(); ++i)
	{
		sLoseSteps[2] = min(sLoseSteps[2], vCRs[CH_G][crG[i]].Step1);
		sLoseSteps[3] = max(sLoseSteps[3], vCRs[CH_G][crG[i]].Step2);
	}
	int sFLoseStep1 = 0x7FFFFFFF, sFLoseStep2 = 0;
	int sGLoseStep1 = 0x7FFFFFFF, sGLoseStep2 = 0;
	int bothLoseStep1 = 0, bothLoseStep2 = 0;
	int bothLoseRegion = GetOverlappedStep(sLoseSteps[0], sLoseSteps[1], sLoseSteps[2], sLoseSteps[3], bothLoseStep1, bothLoseStep2);
	for (int i = 0; i < crF.size(); ++i)
	{
		int bs, es;
		if (GetOverlappedStep(bothLoseStep1, bothLoseStep2, vCRs[CH_F][crF[i]].Step1, vCRs[CH_F][crF[i]].Step2, bs, es) > 0)
		{
			sFLoseStep1 = vCRs[CH_F][crF[i]].Step1;
			sFLoseStep2 = vCRs[CH_F][crF[i]].Step2;
		}
	}

	for (int i = 0; i < crG.size(); ++i)
	{
		int bs, es;
		if (GetOverlappedStep(bothLoseStep1, bothLoseStep2, vCRs[CH_G][crG[i]].Step1, vCRs[CH_G][crG[i]].Step2, bs, es) > 0)
		{
			sGLoseStep1 = vCRs[CH_G][crG[i]].Step1;
			sGLoseStep2 = vCRs[CH_G][crG[i]].Step2;
		}
	}
	if (sFLoseStep2 - sFLoseStep1 >= 20 && sGLoseStep2 - sGLoseStep1 < 20)
	{
		sFLoseStep1 = sGLoseStep1;
		sFLoseStep2 = sGLoseStep2;
	}
	else if (sGLoseStep2 - sGLoseStep1 >= 20 && sFLoseStep2 - sFLoseStep1 < 20)
	{
		sGLoseStep1 = sFLoseStep1;
		sGLoseStep2 = sFLoseStep2;
	}

	if (sFLoseStep2 - sFLoseStep1 > 30)
	{
		sFLoseStep1 = step1;
		sFLoseStep2 = step2;
	}
	if (sGLoseStep2 - sGLoseStep1 > 30)
	{
		sGLoseStep1 = step1;
		sGLoseStep2 = step2;
	}

	crF2.clear(); crG2.clear();
	bFindF = GetCR(CH_F, sFLoseStep1 - 1, iR1 - 1, sFLoseStep2 + 1, iR2 + 1, blocks, vCRs[CH_F], crF2, crF, 1, true);
	bFindG = GetCR(CH_G, sGLoseStep1 - 1, iR1 - 1, sGLoseStep2 + 1, iR2 + 1, blocks, vCRs[CH_G], crG2, crG, 1, true);

	if (!(bFindF && bFindG))
	{
		if (bFindF && !bFindG)
		{
			int s1 = vCRs[CH_F][crF2[0]].Step1, s2 = vCRs[CH_F][crF2[0]].Step2;
			uint8_t r1 = vCRs[CH_F][crF2[0]].Row1, r2 = vCRs[CH_F][crF2[0]].Row2;
			for (int i = 1; i < crF2.size(); ++i)
			{
				if (vCRs[CH_F][crF2[i]].Step1 < s1)
				{
					s1 = vCRs[CH_F][crF2[i]].Step1;
				}
				if (vCRs[CH_F][crF2[i]].Step2 > s2)
				{
					s2 = vCRs[CH_F][crF2[i]].Step2;
				}
			}
			bFindG = GetCR(CH_G, s1, r1 - 1, s2, r2 + 1, blocks, vCRs[CH_G], crG2, -1, 2, true);//G螺孔高度出波
		}
		else if (!bFindF && bFindG)
		{
			int s1 = vCRs[CH_G][crG2[0]].Step1, s2 = vCRs[CH_G][crG2[0]].Step2;
			uint8_t r1 = vCRs[CH_G][crG2[0]].Row1, r2 = vCRs[CH_G][crG2[0]].Row2;
			for (int i = 1; i < crG2.size(); ++i)
			{
				if (vCRs[CH_G][crG2[i]].Step1 < s1)
				{
					s1 = vCRs[CH_G][crG2[i]].Step1;
				}
				if (vCRs[CH_G][crG2[i]].Step2 > s2)
				{
					s2 = vCRs[CH_G][crG2[i]].Step2;
				}
			}
			bFindF = GetCR(CH_F, s1, r1 - 1, s2, r2 + 1, blocks, vCRs[CH_F], crF2, -1, 2, true);//F螺孔高度出波
		}
		else if (!bJoint && !bSew)
		{
			return false;
		}
	}

#pragma endregion

#pragma region FG出波合并
	VINT vFind[2] = { crF2, crG2 };
	if (lastHP.mark == PM_SCREWHOLE && Abs(lastHP.step2 - step1) <= 100 && lastHP.RailType == railType && Abs(lastHP.FRow - iFRow) <= 1)
	{
		for (int i = 0; i < 2; ++i)
		{
			for (int j = vFind[i].size() - 1; j >= 0; --j)
			{
				CR& tpF = vCRs[CH_F + i][vFind[i][j]];
				if (tpF.Row1 < lastHP.Row1 - 2 || tpF.Row2 > tpF.Row2 + 2)
				{
					vFind[i].erase(vFind[i].begin() + j);
				}
			}
		}
	}
	//出波高度检测
	int iIndexF[2] = { -1, -1 };
	int iMinRowF[2] = { 100, 100 };
	VINT vValid[2];
	for (int i = 0; i < 2; ++i)
	{
		if (vFind[i].size() > 1 && bothLoseRegion >= 3)
		{
			int bs, es;
			int maxIndex = 0;
			int maxOS = GetOverlappedStep(vCRs[CH_F + i][vFind[i][0]].Step1, vCRs[CH_F + i][vFind[i][0]].Step2, bothLoseStep1, bothLoseStep2, bs, es);
			for (int j = 1; j < vFind[i].size(); ++j)
			{
				int ost = GetOverlappedStep(vCRs[CH_F + i][vFind[i][j]].Step1, vCRs[CH_F + i][vFind[i][j]].Step2, bothLoseStep1, bothLoseStep2, bs, es);
				if (ost >= maxOS - 3 && vCRs[CH_F + i][vFind[i][j]].Row1 < vCRs[CH_F + i][vFind[i][maxIndex]].Row1)
				{
					maxIndex = j;
					maxOS = ost;
				}
			}
			iIndexF[i] = vFind[i][maxIndex];
			vValid[i].emplace_back(vFind[i][maxIndex]);
			/*
			for (int j = 0; j < vFind[i].size(); ++j)
			{
				CR& tpF = vCRs[CH_F + i][vFind[i][j]];
				int dFRow = blocks[tpF.Block - g_iBeginBlock].vBStepDatas[tpF.Step].FRow - iLastScrewHoleFRow;
				CR core;
				std::map<uint8_t, int> rowdis;
				GetCoreCR(tpF, rowdis, core, blocks, g_iJawRow[railType] & 0x03);
				if (iLastScrewHoleRailType == railType && Abs(dFRow) <= 1 && (core.Row2 < iLastScrewHoleRow - 2 || core.Row1 > iLastScrewHoleRow + 2) && iLastScrewHoleRow != 0)
				{
					if (core.Row2 > g_iScrewHoleFRowH[railType] || core.Row1 < g_iScrewHoleFRowL[railType])
					{
						continue;
					}
				}
				if (iLastScrewHoleIndex != 0 && abs(step1 - iLastScrewHoleStep) <= 300 && Abs(dFRow) <= 1 && (core.Row2 - iLastScrewHoleRow >= 3 || iLastScrewHoleRow - core.Row1 >= 3))
				{
					continue;
				}

				if (core.Row1 < iLastScrewHoleRow - 1 && abs(iLastScrewHoleStep - core.Step1) <= 100 && !bJoint && !bSew)
				{
					continue;
				}

				if (iIndexF[i] == -1)
				{
					iIndexF[i] = vFind[i][j];
					iMinRowF[i] = core.Row1;
					vValid[i].emplace_back(vFind[i][j]);
					continue;
				}

				vValid[i].emplace_back(vFind[i][j]);
				if (iIndexF >= 0 && core.Row1 < iMinRowF[i])
				{
					iIndexF[i] = vFind[i][j];
					iMinRowF[i] = core.Row1;
				}
			}
			*/
		}
		else if (vFind[i].size() == 1)
		{
			iIndexF[i] = vFind[i][0];
			vValid[i].emplace_back(vFind[i][0]);
		}
	}

	bFindF = (iIndexF[0] >= 0);
	bFindG = (iIndexF[1] >= 0);


	uint32_t	sFFindStep1 = 0x7FFFFFFF;
	uint32_t	sFFindStep2 = 0;
	for (int i = 0; i < 2; ++i)
	{
		for (int k = 0; k < vValid[i].size(); ++k)
		{
			if (sFFindStep1 > vCRs[CH_F + i][vValid[i][k]].Step1)
			{
				sFFindStep1 = vCRs[CH_F + i][vValid[i][k]].Step1;
			}

			if (sFFindStep2 < vCRs[CH_F + i][vValid[i][k]].Step2)
			{
				sFFindStep2 = vCRs[CH_F + i][vValid[i][k]].Step2;
			}
		}
	}

	VINT crFDown, crGDown;
	GetCR(CH_F, sFLoseStep1, g_iJawRow[railType] + 1, sFLoseStep2, iFRow - 10, blocks, vCRs[CH_F], crFDown);
	GetCR(CH_G, sFLoseStep1, g_iJawRow[railType] + 1, sFLoseStep2, iFRow - 10, blocks, vCRs[CH_G], crGDown);
	if (crFDown.size() == 0 && crGDown.size() == 0 && !bJoint && !bSew)
	{
		//throw "crFDown.size() == 0 && crGDown.size() == 0 Line: 729";
		return false;
	}

	if (iIndexF[0] < 0 && iIndexF[1] < 0/* || vCRs[iLoseCh][vF[iIndexF]].Row1 < g_iGuiERow[railType] + 5*/)
	{
		return ParseScrewHoleNoFG(head, DataA, blocks, vCRs, step1, step2, iFRow, railType, vWounds, vPMs, bJoint, bAutoJoint, bSew, iScrewIndex);;
	}
	if (vValid[0].size() == 0 && iIndexF[0] >= 0)
	{
		vValid[0].emplace_back(iIndexF[0]);
	}
	if (vValid[1].size() == 0 && iIndexF[1] >= 0)
	{
		vValid[1].emplace_back(iIndexF[1]);
	}

	int iLoseCh = iIndexF[0] >= 0 ? CH_F : CH_G;
	VINT vF = iIndexF[0] >= 0 ? vValid[0] : vValid[1];
	VINT vG = iIndexF[0] >= 0 ? vValid[1] : vValid[0];

	CR&  tempF = vCRs[iLoseCh][vF[0]], tempG;

	VINT screwHoleFlag[2];
	screwHoleFlag[tempF.Channel - CH_F].emplace_back(tempF.Index);

	CR crF1, crG1;
	if (vF.size() > 0)
	{
		for (int i = 1; i < vF.size(); ++i)
		{
			if (vCRs[iLoseCh][vF[i]].Row1 >= tempF.Row1 - 2 && vCRs[iLoseCh][vF[i]].Row2 <= tempF.Row2 + 2 && vF[i] != vF[0])
			{
				if (TryCombineFG1InHole(tempF, vCRs[iLoseCh][vF[i]]))
				{
					screwHoleFlag[vCRs[iLoseCh][vF[i]].Channel - CH_F].emplace_back(vCRs[iLoseCh][vF[i]].Index);
				}
			}
		}

		std::map<uint8_t, uint32_t> vRowPointCount;
		GetCRRowInfo4(tempF, vRowPointCount);
		auto itr = vRowPointCount.begin();
		int  iBeginRow = -1, iBeginRow1 = -1, ivalue = itr->second;
		for (++itr; itr != vRowPointCount.end(); ++itr)
		{
			if (iBeginRow1 > 0 && itr->second > ivalue)
			{
				iBeginRow = itr->first;
				break;
			}
			if (itr->second < ivalue)
			{
				iBeginRow1 = itr->first;
			}
			ivalue = itr->second;
		}

		if (iBeginRow > 0)
		{
			crF1 = tempF;
			for (int i = crF1.Region.size() - 1; i >= 0; --i)
			{
				if (crF1.Region[i].row < iBeginRow)
				{
					crF1.Region.erase(crF1.Region.begin() + i);
				}
				else
				{
					tempF.Region.erase(tempF.Region.begin() + i);
				}
			}
		}
	}

	if (vG.size() > 0)
	{
		int tempChannel = CH_F + CH_G - iLoseCh;
		tempG = vCRs[tempChannel][vG[0]];
		for (int i = 1; i < vG.size(); ++i)
		{
			if (vCRs[tempChannel][vG[i]].Row1 >= tempG.Row1 - 2 && vCRs[tempChannel][vG[i]].Row2 <= tempG.Row2 + 2)
			{
				if (TryCombineFG1InHole(tempG, vCRs[tempChannel][vG[i]]))
				{
					screwHoleFlag[tempChannel - CH_F].emplace_back(vCRs[tempChannel][vG[i]].Index);
				}
			}
		}

		std::map<uint8_t, uint32_t> vRowPointCount;
		GetCRRowInfo4(tempG, vRowPointCount);
		auto itr = vRowPointCount.begin();
		int  iBeginRow = -1, iBeginRow1 = -1, ivalue = itr->second;
		for (++itr; itr != vRowPointCount.end(); ++itr)
		{
			if (iBeginRow1 > 0 && itr->second > ivalue)
			{
				iBeginRow = itr->first;
				break;
			}
			if (itr->second < ivalue)
			{
				iBeginRow1 = itr->first;
			}
			ivalue = itr->second;
		}

		if (iBeginRow > 0)
		{
			crG1 = tempG;
			for (int i = crG1.Region.size() - 1; i >= 0; --i)
			{
				if (crG1.Region[i].row < iBeginRow)
				{
					crG1.Region.erase(crG1.Region.begin() + i);
				}
				else
				{
					tempG.Region.erase(tempG.Region.begin() + i);
				}
			}
		}
	}

	if (crF1.Region.size() > 0 || crG1.Region.size() > 0)
	{
		FillCR(crF1); FillCR(crG1);
		FillCR(tempF); FillCR(tempG);

		Wound_Judged w;
		//BLOCK& blockHead = blocks[crF1.Block - g_iBeginBlock].BlockHead;
		w.IsScrewHole = iScrewIndex + 10;
		BLOCK blockHead;
		if (crF1.Region.size() > 0)
		{
			blockHead = blocks[crF1.Block - g_iBeginBlock].BlockHead;
			w.Block = crF1.Block;
			w.Step = crF1.Step;
			w.Step2 = crF1.Step1;
			w.SizeX = (crF1.Step2 - crF1.Step1) * g_filehead.step;
		}
		else
		{
			blockHead = blocks[crG1.Block - g_iBeginBlock].BlockHead;
			w.Block = crG1.Block;
			w.Step = crG1.Step;
			w.Step2 = crG1.Step1;
			w.SizeX = (crG1.Step2 - crG1.Step1) * g_filehead.step;
		}


		FillWound(w, blockHead, head);

		w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
		w.Degree = WD_SERIOUS;
		sprintf(w.Result, "%d孔右侧水平裂纹", iScrewIndex);
		w.Type = W_SCREW_HORIZON_CRACK_RIGHT;
		if (crF1.Step1 > tempF.Step1)
		{
			sprintf(w.Result, "%d孔左侧水平裂纹", iScrewIndex);
			w.Type = W_SCREW_HORIZON_CRACK_LEFT;
		}
		w.Place = WP_WAIST;
		w.SizeY = 1;
		w.IsScrewHole = 1;
		VINT crDD, crEE;
		if (crF1.Region.size() > 0)
		{
			AddWoundData(w, crF1);
			GetCR(CH_D, crF1.Step1 - 1, crF1.Row1 - 1, crF1.Step2 + 1, crF1.Row2 + 1, blocks, vCRs[CH_D], crDD, -1, 1);
			GetCR(CH_E, crF1.Step1 - 1, crF1.Row1 - 1, crF1.Step2 + 1, crF1.Row2 + 1, blocks, vCRs[CH_E], crEE, -1, 1);
		}
		if (crG1.Region.size() > 0)
		{
			AddWoundData(w, crG1);
			GetCR(CH_D, crG1.Step1 - 1, crG1.Row1 - 1, crG1.Step2 + 1, crG1.Row2 + 1, blocks, vCRs[CH_D], crDD, -1, 1);
			GetCR(CH_E, crG1.Step1 - 1, crG1.Row1 - 1, crG1.Step2 + 1, crG1.Row2 + 1, blocks, vCRs[CH_E], crEE, -1, 1);
		}
		FillWound2(w, blocks);
		if (crDD.size() > 0 && crEE.size() > 0)
		{
			int ss1 = 0x7FFFFFFF, ss2 = 0;
			if (crF1.Region.size() > 0)
			{
				ss1 = min(ss1, crF1.Step1);
				ss2 = max(ss2, crF1.Step2);
			}
			if (crG1.Region.size() > 0)
			{
				ss1 = min(ss1, crG1.Step1);
				ss2 = max(ss2, crG1.Step2);
			}

			bool isLeft = crF1.Region.size() > 0 && crF1.Step1 > tempF.Step1 || crG1.Region.size() > 0 && crG1.Step1 > tempF.Step1;
			if (isLeft)//左侧水平裂纹
			{
				for (int i = crDD.size() - 1; i >= 0; --i)
				{
					if (vCRs[CH_D][crDD[i]].Step2 < ss1 - 1)
					{
						crDD.erase(crDD.begin() + i);
					}
				}
				for (int i = crEE.size() - 1; i >= 0; --i)
				{
					if (vCRs[CH_E][crEE[i]].Step2 < ss2)
					{
						crEE.erase(crEE.begin() + i);
					}
				}
			}
			else//右侧水平裂纹
			{
				for (int i = crDD.size() - 1; i >= 0; --i)
				{
					if (vCRs[CH_D][crDD[i]].Step2 > ss2)
					{
						crDD.erase(crDD.begin() + i);
					}
				}
				for (int i = crEE.size() - 1; i >= 0; --i)
				{
					if (vCRs[CH_E][crEE[i]].Step2 > ss2)
					{
						crEE.erase(crEE.begin() + i);
					}
				}
			}
			AddWoundData(w, vCRs[CH_D], crDD);
			AddWoundData(w, vCRs[CH_E], crEE);
			SetUsedFlag(vCRs[CH_D], crDD, 1);
			SetUsedFlag(vCRs[CH_E], crEE, 1);
			w.Type = W_DOUBLE_HOLE;
		}

		AddToWounds(vWounds, w);

		VINT crF3, crG3;
		if (crF1.Region.size() > 0)
		{
			GetCR(iLoseCh, crF1.Step1, crF1.Row2 + 1, crF1.Step2, iFRow - 3, blocks, vCRs[iLoseCh], crF3);
			for (int i = 0; i < crF3.size(); ++i)
			{
				CR& tpF = vCRs[iLoseCh][crF3[i]];
				if (tpF.Step1 >= min(crF1.Step1, tempF.Step1) && tpF.Step2 <= max(crF1.Step2, tempF.Step2))
				{
					SetUsedFlag(tpF, 1);
				}
			}

		}
		if (crG1.Region.size() > 0)
		{
			GetCR(CH_F + CH_G - iLoseCh, crG1.Step1, crG1.Row2 + 1, crG1.Step2, iFRow - 3, blocks, vCRs[CH_F + CH_G - iLoseCh], crG3);
			for (int i = 0; i < crG3.size(); ++i)
			{
				CR& tpG = vCRs[CH_F + CH_G - iLoseCh][crG3[i]];
				if (tpG.Step1 >= min(crG1.Step1, tempG.Step1) && tpG.Step2 <= max(crG1.Step2, tempG.Step2))
				{
					SetUsedFlag(tpG, 1);
				}
			}
		}

	}

	TryCombineFG1InHole(tempF, tempG);

	if (bothLoseRegion > 300 && bJoint && (tempF.Step1 > step2 || tempF.Step2 < step1))
	{
		return false;
	}

	for (int j = 0; j < vF.size(); ++j)
	{
		screwHoleFlag[iLoseCh - CH_F].emplace_back(vF[j]);
	}
	for (int j = 0; j < vG.size(); ++j)
	{
		screwHoleFlag[1 + CH_F - iLoseCh].emplace_back(vG[j]);
	}

	int iExceptF = -1, iExceptG = -1;
	if (iLoseCh == CH_F)	 iExceptF = vF[0];
	else if (iLoseCh == CH_G) iExceptG = vF[0];

	VINT vIsolate[2];
	GetCR(CH_F, tempF.Step1 - 7, tempF.Row1, tempF.Step2 + 7, tempF.Row2, blocks, vCRs[CH_F], vIsolate[0], crF2);
	GetCR(CH_G, tempF.Step1 - 7, tempF.Row1, tempF.Step2 + 7, tempF.Row2, blocks, vCRs[CH_G], vIsolate[1], crG2);
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < vIsolate[i].size(); ++j)
		{
			CR & tpFG = vCRs[CH_F + i][vIsolate[i][j]];
			if (TryCombineFGInHole(tempF, tpFG))
			{
				screwHoleFlag[i].emplace_back(tpFG.Index);
			}
		}
	}
	vIsolate[0] = crF2;
	vIsolate[1] = crG2;
	for (int i = 0; i < 2; ++i)
	{
		for (int j = 0; j < vIsolate[i].size(); ++j)
		{
			CR & tpFG = vCRs[CH_F + i][vIsolate[i][j]];
			if (TryCombineFGInHole(tempF, tpFG))
			{
				screwHoleFlag[i].emplace_back(tpFG.Index);
			}
		}
	}

	int stepF1 = tempF.Step1, stepF2 = tempF.Step2;
	int stepFMiddle = (tempF.Step1 + tempF.Step2) >> 1;
	int rowF1 = tempF.Row1, rowF2 = tempF.Row2;

	if (!bJoint && !bSew && (!bFindF || !bFindG) && Abs(tempF.Step1 - lastHP.step2) <= 300 && iLoseGap < min(lastHP.FLength2 - 3, 10) && (tempF.Step2 - tempF.Step1) < min(10, lastHP.FLength))
	{
		return false;
	}

	if (tempF.Step2 - tempF.Step1 <= min(igapF, igapG) && min(igapF, igapG) <= 20)
	{
		uint8_t bFindD = GetCR(CH_D, tempF.Step1 - 5, tempF.Row1, tempF.Step1 + 4, tempF.Row2, blocks, vCRs[CH_D], crD, -1, 3);

		if (bFindD)
		{
			for (int i = 0; i < crD.size(); ++i)
			{
				if (vCRs[CH_D][crD[i]].Step2 >= sFLoseStep1)
				{
					sFLoseStep1 = max((vCRs[CH_D][crD[i]].Step1 + vCRs[CH_D][crD[i]].Step2) / 2, sFLoseStep1);
					break;
				}
			}
		}

		uint8_t bFindE = GetCR(CH_E, tempF.Step2 - 4, tempF.Row1, tempF.Step2 + 5, tempF.Row2, blocks, vCRs[CH_E], crE, -1, 3);
		if (bFindE)
		{
			for (int i = crE.size() - 1; i >= 0; --i)
			{
				if (vCRs[CH_E][crE[i]].Step1 <= sFLoseStep2)
				{
					sFLoseStep2 = min((vCRs[CH_E][crE[i]].Step1 + vCRs[CH_E][crE[i]].Step2) / 2, sFLoseStep2);
					break;
				}
			}
		}
		for (int i = sFLoseStep1 - 1; i <= sFLoseStep2 + 1; ++i)
		{
			WaveData wd;
			wd.block = tempF.Block;
			wd.find = 1;
			wd.step = i;
			wd.row = ((tempF.Row1 + tempF.Row2) >> 1);
			tempF.Region.emplace_back(wd);
		}
		FillCR(tempF);
		crD.clear();
		crE.clear();
	}

	uint8_t bFindD = GetCR(CH_D, tempF.Step1 - 5, iLuokong_D_Row1_L[railType], tempF.Step1 + 4, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, 3);
	uint8_t bFindE = GetCR(CH_E, tempF.Step2 - 4, iLuokong_D_Row1_L[railType], tempF.Step2 + 5, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, 3);
	if (bFindD + bFindE == 0 && !bJoint && !bSew)
	{
		return false;
	}
	crD.clear();
	crE.clear();

	int iFindGap = tempF.Step2 - tempF.Step1;
	for (int j = 0; j < crF.size(); ++j)
	{
		screwHoleFlag[0].emplace_back(crF[j]);
	}
	for (int j = 0; j < crG.size(); ++j)
	{
		screwHoleFlag[1].emplace_back(crG[j]);
	}
	for (int i = 0; i < 2; ++i)
	{
		SetScrewHoleFlag(vCRs[CH_F + i], screwHoleFlag[i], 1);
		SetUsedFlag(vCRs[CH_F + i], screwHoleFlag[i], 1);
	}
#pragma endregion

#pragma region 判断tempF是否异常
	if (tempF.Step1 - iLastScrewHoleStep <= 200)
	{
		int count = 0;
		CR exF = tempF;
		exF.Region.clear();
		for (int i = 0; i < tempF.Region.size(); ++i)
		{
			if (tempF.Region[i].row > iLastScrewHoleRow2 + 1 || tempF.Region[i].row < iLastScrewHoleRow - 1)
			{
				++count;
				exF.Region.emplace_back(tempF.Region[i]);
			}
		}
		if (count >= 4)
		{
			bool carType = false;
			ParseHorizonalCrack(head, DataA, blocks, vCRs, exF, g_direction, carType, iFRow, vWounds, bJoint, bSew);
		}
	}
#pragma endregion

	iLastScrewHoleStep = tempF.Step1;
	iLastScrewHoleRow = tempF.Row1;
	iLastScrewHoleRow2 = tempF.Row2;
	iLastScrewHoleFRow = blocks[tempF.Block - g_iBeginBlock].vBStepDatas[tempF.Step].FRow;
	iLastScrewHoleRailType = railType;
	iLastScrewHoleIndex = iScrewIndex;
	iLastScrewHoleFLength = tempF.Step2 - tempF.Step1;
	iLastScrewHoleFLength2 = iLoseGap;

#pragma region 螺孔水平裂纹
	BLOCK& blockHead = blocks[tempF.Block - g_iBeginBlock].BlockHead;
	Wound_Judged w;
	//出波
	w.SetEmpty();
	if (ParseScrewHoleCrackRight(head, blockHead, DataA, blocks, vCRs, iFRow, tempF, iIndexF[0], iExceptG, iScrewIndex, bJoint, bAutoJoint, w, vPMs))
	{
		FillWound2(w, blocks);
		AddToWounds(vWounds, w);
	}

	w.SetEmpty();
	if (ParseScrewHoleCrackLeft(head, blockHead, DataA, blocks, vCRs, iFRow, tempF, iIndexF[1], iExceptG, iScrewIndex, bJoint, bAutoJoint, w, vPMs))
	{
		FillWound2(w, blocks);
		AddToWounds(vWounds, w);
	}
#pragma endregion

	crFDown.clear();
	crGDown.clear();
	GetCR(CH_F, tempF.Step1, vCRs[iLoseCh][vF[0]].Row2 + 1, tempF.Step2, iFRow - 3, blocks, vCRs[CH_F], crFDown);
	GetCR(CH_G, tempF.Step1, vCRs[iLoseCh][vF[0]].Row2 + 1, tempF.Step2, iFRow - 3, blocks, vCRs[CH_G], crGDown);
	SetUsedFlag(vCRs[CH_F], crFDown, 1);
	SetUsedFlag(vCRs[CH_G], crGDown, 1);
	SetScrewHoleFlag(vCRs[CH_F], crFDown, 1);
	SetScrewHoleFlag(vCRs[CH_G], crGDown, 1);

	//获取D,E的八形波
	double angleD = 0.1 * head.deviceP2.Angle[ACH_D].Refrac, offsetD = head.deviceP2.Place[ACH_D] + blockHead.probOff[ACH_D];
	double angleE = 0.1 * head.deviceP2.Angle[ACH_E].Refrac, offsetE = head.deviceP2.Place[ACH_E] + blockHead.probOff[ACH_E];

	int iDLeft = (iScrewIndex == 1) ? ((stepF1 + stepF2) >> 1) : (stepF1 + 5);
	int iminiDsize = (iScrewIndex == 1) ? 1 : 3;
	int iminiEsize = (iScrewIndex == -1) ? 1 : 3;
	bFindD = GetCR(CH_D, stepF1 - 15, iLuokong_D_Row1_L[railType], iDLeft, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, iminiDsize);

	int iERight = (iScrewIndex == -1) ? ((stepF1 + stepF2) >> 1) : (stepF2 - 3);
	bFindE = GetCR(CH_E, stepF1 - 12, iLuokong_D_Row1_L[railType], stepF2 + 15, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, iminiEsize);

	int iStepDIndex = 0x7FFFFFFF, iStepEIndex = 0;
	int idxD = -1, idxE = -1;
	int dist = 1000;
	for (int i = 0; i < crD.size(); ++i)
	{
		CR& tpD = vCRs[CH_D][crD[i]];

		if (lastHP.tempF.Step2 > 0 && lastHP.tempE.Step2 > 0 && Abs(lastHP.step2 - tempF.Step1) <= 100)
		{
			int d = Abs((tpD.Step2 - tempF.Step2) - (lastHP.tempD.Step2 - lastHP.tempF.Step2));
			if (d >= 8 && tpD.Region.size() >= 5 && lastHP.tempD.Region.size() >= 5)
			{
				continue;
			}
		}

		if (tpD.Step1 >= stepFMiddle - 2)
		{
			continue;
		}
		int d = GetDistance(tpD, tempF);
		double dh = GetDistanceHA(tempF, tpD);
		double dv = GetDistanceVA(tempF, tpD);
		if (d < dist && d <= 3 && tpD.Step1 <= iStepDIndex && tpD.Row1 <= tempF.Row2 + 1)
		{
			dist = d;
			idxD = crD[i];
			iStepDIndex = tpD.Step1;
		}
		else if (d <= 3 && dv <= 2 && tpD.Step1 <= iStepDIndex)
		{
			dist = d;
			idxD = crD[i];
		}
		else if (d <= 1 && dv <= 2 && tpD.Step1 <= tempF.Step1 + 1)
		{
			for (int k = 0; k < tpD.Region.size(); ++k)
			{
				if (tpD.Region[k].step == tempF.Step1)
				{
					if (tpD.Region[k].row <= tempF.Row1 + 1)
					{
						dist = d;
						idxD = crD[i];
						break;
					}
				}
			}
		}
	}

	if (crD.size() >= 2 && idxD >= 0)
	{
		for (int i = 0; i < crD.size(); ++i)
		{
			if (crD[i] == idxD)
			{
				continue;
			}
			double dh = GetDistanceHA(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
			double dv = GetDistanceVA(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
			int oh = GetOverlappedCountH(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
			int ov = GetOverlappedCountV(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
			if (vCRs[CH_D][crD[i]].Step2 <= vCRs[CH_D][idxD].Step1 && vCRs[CH_D][crD[i]].Row1 >= vCRs[CH_D][idxD].Row2 && vCRs[CH_D][crD[i]].Row1 - vCRs[CH_D][idxD].Row2 <= 3 && vCRs[CH_D][idxD].Step2 - vCRs[CH_D][crD[i]].Step1 <= 5
				||
				vCRs[CH_D][idxD].Step2 <= vCRs[CH_D][crD[i]].Step1 && vCRs[CH_D][idxD].Row2 <= vCRs[CH_D][crD[i]].Row1 && vCRs[CH_D][idxD].Row1 - vCRs[CH_D][crD[i]].Row2 <= 3 && vCRs[CH_D][crD[i]].Step1 - vCRs[CH_D][idxD].Step2 <= 5
				)
			{
				if (dh <= 4 && dv <= 3 && oh <= 2)
				{
					Combine(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
					SetUsedFlag(vCRs[CH_D][crD[i]], 1);
					SetScrewHoleFlag(vCRs[CH_D][crD[i]], 1);
				}
			}
		}
	}
	/********************2020-04-02*************************/
	else if (crD.size() == 1 && idxD < 0)
	{
		CR& tpD = vCRs[CH_D][crD[0]];
		int d = GetDistance(tpD, tempF);
		int r1 = 0, r2 = 0;
		int or = GetOverlappedStep(tpD.Row1, tpD.Row2, tempF.Row1, tempF.Row2, r1, r2);
		if (d <= 1 && tpD.Step1 <= sFLoseStep1 || r1 > 0 && tpD.Step1 >= sLoseSteps[0] - 3 && tpD.Step2 < sLoseSteps[1] || tpD.Step1 <= tempF.Step1 - 5 && tpD.Row1 > tempF.Row2 && tpD.Row1 - tempF.Row2 <= 2)
		{
			idxD = crD[0];
		}
	}
	/******************************************************/

	dist = 1000;
	for (int i = 0; i < crE.size(); ++i)
	{
		CR& tpE = vCRs[CH_E][crE[i]];
		if (lastHP.tempF.Step2 > 0 && lastHP.tempE.Step2 > 0 && Abs(lastHP.step2 - tempF.Step1) <= 100)
		{
			int d = Abs((tpE.Step2 - tempF.Step2) - (lastHP.tempE.Step2 - lastHP.tempF.Step2));
			if (d >= 8 && tpE.Region.size() >= 5 && lastHP.tempE.Region.size() >= 5)
			{
				continue;
			}
		}
		//if (vCRs[CH_E][crE[i]].Step2 < stepFMiddle + 2)
		if (tpE.Step2 <= stepFMiddle + 3 && tpE.Region.size() >= 6 || tpE.Step2 < stepFMiddle + 3)
		{
			continue;
		}
		int d = GetDistance(tpE, tempF);
		double dh = GetDistanceHA(tempF, tpE);
		double dv = GetDistanceVA(tempF, tpE);
		if (d < dist && d <= 3 && tpE.Step1 >= iStepEIndex)
		{
			dist = d;
			idxE = crE[i];
			iStepEIndex = tpE.Step1;
		}
		else if (d <= 3 && (dv <= 2 || dv > 2 && tpE.Row1 <= tempF.Row1 + 1) && iStepEIndex <= tpE.Step1)
		{
			dist = d;
			idxE = crE[i];
		}
		else if (crE.size() == 1 && dv <= 2 && tpE.Step1 - tempF.Step1 <= 18 && tpE.Step2 - tempF.Step1 > 8 && tempF.Step2 - tempF.Step1 <= 5)
		{
			dist = d;
			idxE = crE[i];
		}
		else if (dv <= 2 && dh > 3)
		{
			if (Abs(tempF.Step2 - lastHP.step2) <= 100)
			{
				if (idxD >= 0)
				{
					int lastDE = lastHP.tempE.Step1 - lastHP.tempD.Step1;
					int currDE = tpE.Step1 - vCRs[CH_D][idxD].Step1;
					if (Abs(currDE - lastDE) <= 5)
					{
						dist = d;
						idxE = crE[i];
					}
				}
			}
		}
	}
	if (crE.size() >= 2 && idxE >= 0)
	{
		for (int i = 0; i < crE.size(); ++i)
		{
			if (crE[i] == idxE)
			{
				continue;
			}
			double dh = GetDistanceHA(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
			double dv = GetDistanceVA(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
			int oh = GetOverlappedCountH(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
			int ov = GetOverlappedCountV(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
			if (vCRs[CH_E][crE[i]].Step1 >= vCRs[CH_E][idxE].Step2 && vCRs[CH_E][crE[i]].Row1 >= vCRs[CH_E][idxE].Row2 && vCRs[CH_E][crE[i]].Row1 - vCRs[CH_E][idxE].Row2 <= 3 && vCRs[CH_E][crE[i]].Step1 - vCRs[CH_E][idxE].Step2 <= 5
				||
				vCRs[CH_E][crE[i]].Step2 <= vCRs[CH_E][idxE].Step1 && vCRs[CH_E][crE[i]].Row2 <= vCRs[CH_E][idxE].Row1 && vCRs[CH_E][idxE].Row1 - vCRs[CH_E][crE[i]].Row2 <= 3 && vCRs[CH_E][idxE].Step1 - vCRs[CH_E][crE[i]].Step2 <= 5
				)
			{
				if (dh <= 4 && dv <= 3 && oh <= 2)
				{
					Combine(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
					SetUsedFlag(vCRs[CH_E][crE[i]], 1);
					SetScrewHoleFlag(vCRs[CH_E][crE[i]], 1);
				}
			}
		}
	}
	else if (crE.size() == 1 && idxE < 0)
	{
		CR& tpE = vCRs[CH_E][crE[0]];
		int d = GetDistance(tpE, tempF);
		int r1 = 0, r2 = 0;
		int or = GetOverlappedStep(tpE.Row1, tpE.Row2, tempF.Row1, tempF.Row2, r1, r2);
		if (d <= 1 && tpE.Step1 >= sFLoseStep1 || r1 > 0 && tpE.Step1 >= sLoseSteps[2] + 3 && tpE.Step2 < sLoseSteps[3] + 3)
		{
			if (tempF.Step2 - tempF.Step1 >= 10 && tpE.Step1 < tempF.Step1)
			{

			}
			else if (((int)tpE.Step2 - (int)tempF.Step2) >= -3)
			{
				idxE = crE[0];
			}
		}
	}

	CR tempD, tempE;
	int stepD1 = 0, stepD2 = 0, stepE1 = 0, stepE2 = 0;
	int stepDA1 = 0, stepDA2 = 0, stepEA1 = 0, stepEA2 = 0;
	if (idxD >= 0)
	{
		tempD = vCRs[CH_D][idxD];
		SetScrewHoleFlag(vCRs[CH_D][idxD], 1);
		GetCRASteps(CH_D, tempD, stepD1, stepD2, blocks, angleD, offsetD, head.step);
		VINT crDD;
		if (bJoint && iScrewIndex == 1)
		{
			GetCR(CH_D, stepF1 - 5, tempF.Row1, stepF2 + 20, iFRow - 4, blocks, vCRs[CH_D], crDD, idxD);
		}
		else
		{
			GetCR(CH_D, stepF1 - 2, tempF.Row1, stepF2 + 20, iFRow - 4, blocks, vCRs[CH_D], crDD, idxD);
		}
		for (int k = 0; k < crDD.size(); ++k)
		{
			CR& tpD = vCRs[CH_D][crDD[k]];
			int iFirstRow = 0, iLastRow = 0;
			GetCRRowInfo2(tpD, iFirstRow, iLastRow, CH_D);
			if (iFirstRow < iLastRow)
			{
				continue;
			}
			uint8_t isSecond = IsScrewHoleSecondWave(tempD, tpD, blocks, angleD, offsetD, g_filehead.step);
			if (isSecond == 2)
			{
				SetUsedFlag(tpD, 1);
				continue;
			}
			else if (isSecond == 1)
			{
				int oh = GetOverlappedCountH(tempD, tpD);
				int ov = GetOverlappedCountV(tempD, tpD);
				if (oh < 3 && ov < 3 && tpD.Row1 - tempF.Row1 > 10)
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
			if (bJoint && iScrewIndex == -1)
			{
				int sss = 0, eee = 0;
				GetCRASteps(CH_D, tempD, stepDA1, stepDA2, blocks, angleD, offsetD, g_filehead.step);
				GetCRASteps(CH_D, tpD, sss, eee, blocks, angleD, offsetD, g_filehead.step);
				if (eee < stepDA2 + 5 && sss >= stepDA1)
				{
					SetUsedFlag(tpD, 1);
					continue;
				}
			}
		}
		if (idxD >= 0 && iScrewIndex == 1 && bJoint == 1 && crDD.size() == 1)
		{
			if (tempD.Step2 < vCRs[CH_D][crDD[0]].Step1 && vCRs[CH_D][crDD[0]].Row1 <= tempF.Row1 && vCRs[CH_D][crDD[0]].Row1 < tempD.Row1 && vCRs[CH_D][crDD[0]].Step1 < (tempF.Step1 + tempF.Step2) / 2)
			{
				SetUsedFlag(vCRs[CH_D][idxD], 1);
				SetUsedFlag(vCRs[CH_D], crDD, 1);
				tempD = vCRs[CH_D][crDD[0]];
				idxD = crDD[0];
			}
		}

		if (tempD.Row2 - tempF.Row2 >= 5)
		{
			CR cr1, cr2;
			if (IsDoubleCR(tempD, blocks, CH_D, cr1, cr2) && (cr2.Step2 - cr2.Step1 >= 3 || cr2.Row2 - cr2.Row1 >= 3 || cr2.Region.size() >= 5) && cr2.Row1 > tempF.Row2)
			{
				if (cr2.Row1 < cr2.Row2)
				{
					tempD = cr1;
					vCRs[CH_D][tempD.Index] = cr1;
					for (int i = vWounds.size() - 1; i >= 0; --i)
					{
						if ((vWounds[i].Type == W_SCREW_HORIZON_CRACK_RIGHT || vWounds[i].Type == W_SCREW_HORIZON_CRACK_LEFT) && Abs(cr2.Step2 - vWounds[i].Step2) <= 15)
						{
							for (int j = vWounds[i].vCRs.size() - 1; j >= 0; --j)
							{
								if (vWounds[i].vCRs[j].Channel == CH_D && vWounds[i].vCRs[j].Index == tempD.Index)
								{
									vWounds[i].vCRs[j] = cr2;
								}
							}
						}
						if (i < vWounds.size() - 5)
						{
							break;
						}
					}

					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = cr2.Block;
					w.Step = cr2.Step;
					w.Step2 = cr2.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔右侧水平裂纹", iScrewIndex);
					w.Type = W_SCREW_HORIZON_CRACK_RIGHT;
					w.Place = WP_WAIST;
					w.SizeX = (cr2.Row2 - cr2.Row1) / 0.8;
					w.SizeY = 1;
					AddWoundData(w, cr2);
					FillWound2(w, blocks);
					AddToWounds(vWounds, w);
				}
			}
		}
		else if (tempD.Region.size() >= 40)
		{
			Wound_Judged w;
			w.IsScrewHole = iScrewIndex + 10;
			FillWound(w, blockHead, head);
			w.Block = tempD.Block;
			w.Step = tempD.Step;
			w.Step2 = tempD.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Degree = WD_SERIOUS;
			sprintf(w.Result, "%d孔二象限斜裂纹", iScrewIndex);
			w.Type = W_SCREW_CRACK2;
			w.Place = WP_WAIST;
			w.SizeX = (tempD.Row2 - tempD.Row1) / 0.8;
			w.SizeY = 1;
			AddWoundData(w, tempD);
			FillWound2(w, blocks);
			AddToWounds(vWounds, w);
		}

		SetUsedFlag(vCRs[CH_D][idxD], 1);
	}

	if (idxE >= 0)
	{
		tempE = vCRs[CH_E][idxE];
		SetScrewHoleFlag(vCRs[CH_E][idxE], 1);
		GetCRASteps(CH_E, tempE, stepE1, stepE2, blocks, angleE, offsetE, head.step);
		GetCRInfo(tempE, DataA, blocks);
		if (tempE.vASteps.size() > 0)
		{
			stepEA1 = tempE.vASteps[0].Step;
			stepEA2 = tempE.vASteps[tempE.vASteps.size() - 1].Step;
		}

		VINT crEE;
		GetCR(CH_E, stepF1 - 20, tempF.Row1, stepF2 + 2, iFRow - 4, blocks, vCRs[CH_E], crEE, idxE);
		for (int k = 0; k < crEE.size(); ++k)
		{
			CR& tpE = vCRs[CH_E][crEE[k]];
			int iFirstRow = 0, iLastRow = 0;
			GetCRRowInfo2(tpE, iFirstRow, iLastRow, CH_E);
			if (iFirstRow > iLastRow)
			{
				continue;
			}
			uint8_t isSecond = IsScrewHoleSecondWave(tempE, tpE, blocks, angleE, offsetE, g_filehead.step);
			if (isSecond == 2)
			{
				SetUsedFlag(tpE, 1);
			}
			else if (isSecond == 1)
			{
				if (GetOverlappedCountH(tempE, tpE) < 3 && GetOverlappedCountV(tempE, tpE) < 3 && tpE.Row1 - tempF.Row1 > 10)
				{
					SetUsedFlag(tpE, 1);
				}
			}
			if (bJoint && iScrewIndex == 1)
			{
				//int sss = 0, eee = 0;
				//GetCRASteps(CH_E, tempE, stepEA1, stepEA2, blocks, angleE, offsetE, g_filehead.step);
				//GetCRASteps(CH_E, tpE, sss, eee, blocks, angleE, offsetE, g_filehead.step);
				//if (eee <= stepEA2 && sss > stepEA1)
				//{
				//	SetUsedFlag(tpE, 1);
				//	continue;
				//}
			}
		}

		if (tempE.Row2 - tempF.Row2 >= 5)
		{
			CR cr1, cr2;
			if (IsDoubleCR(tempE, blocks, CH_E, cr1, cr2))
			{
				if (cr2.Step2 - cr2.Step1 >= 3 && cr2.Row1 > tempF.Row2)
				{
					if (cr2.Row1 < cr2.Row2)
					{
						tempE = cr1;
						vCRs[CH_E][tempE.Index] = cr1;
						for (int i = vWounds.size() - 1; i >= 0; --i)
						{
							if ((vWounds[i].Type == W_SCREW_HORIZON_CRACK_RIGHT || W_SCREW_HORIZON_CRACK_LEFT) && Abs(cr2.Step2 - vWounds[i].Step2) <= 15)
							{
								for (int j = vWounds[i].vCRs.size() - 1; j >= 0; --j)
								{
									if (vWounds[i].vCRs[j].Channel == CH_E && vWounds[i].vCRs[j].Index == tempE.Index)
									{
										vWounds[i].vCRs[j] = cr2;
									}
								}
							}
							if (i < vWounds.size() - 5)
							{
								break;
							}
						}

						Wound_Judged w;
						w.IsScrewHole = iScrewIndex + 10;
						FillWound(w, blockHead, head);
						w.Block = cr2.Block;
						w.Step = cr2.Step;
						w.Step2 = cr2.Step1;
						w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
						w.Degree = WD_SERIOUS;
						sprintf(w.Result, "%d孔左侧水平裂纹", iScrewIndex);
						w.Type = W_SCREW_HORIZON_CRACK_LEFT;
						w.Place = WP_WAIST;
						w.SizeX = (cr2.Row2 - cr2.Row1) / 0.8;
						w.SizeY = 1;
						AddWoundData(w, cr2);
						FillWound2(w, blocks);
						AddToWounds(vWounds, w);
					}
				}
				else if (cr2.Row1 > tempF.Row2 + 3 && cr2.Region.size() >= 3)
				{
					tempE = cr1;
					vCRs[CH_E][tempE.Index] = cr1;
					Wound_Judged w;
					w.IsScrewHole = iScrewIndex + 10;
					FillWound(w, blockHead, head);
					w.Block = cr2.Block;
					w.Step = cr2.Step;
					w.Step2 = cr2.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Degree = WD_SERIOUS;
					sprintf(w.Result, "%d孔三象限斜裂纹", iScrewIndex);
					w.Type = W_SCREW_CRACK3;
					w.Place = WP_WAIST;
					w.SizeX = (cr2.Row2 - cr2.Row1) / 0.8;
					w.SizeY = 1;
					AddWoundData(w, cr2);
					FillWound2(w, blocks);
					AddToWounds(vWounds, w);
				}
			}
		}
		else if (tempE.Region.size() >= 25)
		{
			Wound_Judged w;
			w.IsScrewHole = iScrewIndex + 10;
			FillWound(w, blockHead, head);
			w.Block = tempE.Block;
			w.Step = tempE.Step;
			w.Step2 = tempE.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Degree = WD_SERIOUS;
			sprintf(w.Result, "%d孔一象限斜裂纹", iScrewIndex);
			w.Type = W_SCREW_CRACK1;
			w.Place = WP_WAIST;
			w.SizeX = (tempE.Row2 - tempE.Row1) / 0.8;
			w.SizeY = 1;
			AddWoundData(w, tempE);
			FillWound2(w, blocks);
			AddToWounds(vWounds, w);
		}

		SetScrewHoleFlag(vCRs[CH_E][tempE.Index], 1);
		SetUsedFlag(vCRs[CH_E][tempE.Index], 1);
		//SetUsedFlag(tempE, 1);
	}

	PM pm;
	memset(&pm, 0, sizeof(pm));
	if (idxD < 0)
	{
		pm.Block = tempF.Block;
		pm.Step = tempF.Step;
		pm.Step2 = tempF.Step1;
	}
	else
	{
		pm.Block = tempD.Block;
		pm.Step = tempD.Step;
		pm.Step2 = tempD.Step1;
	}
	pm.Data = iScrewIndex + 10;
	pm.ScrewHoleCount = tempF.Step2 - tempF.Step1;
	pm.GuideHoleCount = iLoseGap;
	//pm.Walk = GetWD(blocks[pm.Block - g_iBeginBlock].BlockHead.walk, pm.Block, pm.Step, g_direction);
	pm.Mark = PM_SCREWHOLE;
	AddToMarks(pm, vPMs);

	for (int i = crD.size() - 1; i >= 0; --i)
	{
		CR& td = vCRs[CH_D][crD[i]];
		int lastRow = 0, firstRow = 0, lastRow2 = 0, firstRow2 = 0;
		GetCRRowInfo3(td, firstRow, lastRow, CH_E, firstRow2, lastRow2);
		if (firstRow < lastRow)
		{
			td.IsReversed = 1;
			crD.erase(crD.begin() + i);
		}
	}

	for (int i = crE.size() - 1; i >= 0; --i)
	{
		CR& te = vCRs[CH_E][crE[i]];
		int lastRow = 0, firstRow = 0, lastRow2 = 0, firstRow2 = 0;
		GetCRRowInfo3(te, firstRow, lastRow, CH_D, firstRow2, lastRow2);
		if (firstRow > lastRow)
		{
			te.IsReversed = 1;
			crE.erase(crE.begin() + i);
		}
	}

	uint8_t repeatChannel1 = 0, repeatChannel2 = 0;
	{
		VINT crdd, cree;
		GetCR(CH_D, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, tempF.Row2 + 2, blocks, vCRs[CH_D], crdd, -1, 4);
		GetCR(CH_E, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, tempF.Row2 + 2, blocks, vCRs[CH_E], cree, -1, 4);
		if (crdd.size() >= 2)	repeatChannel1++;
		if (cree.size() >= 2)	repeatChannel1++;
		if (crF2.size() >= 2)	repeatChannel2++;
		if (crG2.size() >= 2)	repeatChannel2++;
	}


	if (
		pm.ScrewHoleCount > 25 && (repeatChannel1 + repeatChannel2 > 0) ||
		pm.ScrewHoleCount >= 20 && lastHP.FLength2 > 5 && pm.GuideHoleCount > 1.5 * lastHP.FLength2 && pm.GuideHoleCount < 35 && pm.ScrewHoleCount > 1.5 * lastHP.FLength && Abs(tempF.Step1 - lastHP.step2) < 1000 && repeatChannel1 + repeatChannel2 >= 2 && repeatChannel1 != 0 ||
		pm.ScrewHoleCount >= 20 && pm.ScrewHoleCount >= min(1.5 * lastHP.FLength, lastHP.FLength + 5) && pm.GuideHoleCount >= 10 && pm.GuideHoleCount >= min(1.5 * lastHP.FLength2, lastHP.FLength2 + 5) && repeatChannel1 + repeatChannel2 >= 2 && repeatChannel1 != 0
		)
	{
		crD.clear();
		crE.clear();
		crF2.clear();
		crG2.clear();
		GetCR(CH_D, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, iFRow - 3, blocks, vCRs[CH_D], crD, idxD, 3, true);
		GetCR(CH_E, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, iFRow - 3, blocks, vCRs[CH_E], crE, idxE, 3, true);

		GetCR(CH_F, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, iFRow - 3, blocks, vCRs[CH_F], crF2, screwHoleFlag[0], 1, true);
		GetCR(CH_G, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, iFRow - 3, blocks, vCRs[CH_G], crG2, screwHoleFlag[1], 1, true);

		Wound_Judged w;
		w.IsScrewHole = iScrewIndex + 10;
		FillWound(w, blockHead, head);
		w.Block = tempF.Block;
		w.Step = tempF.Step;
		w.Step2 = tempF.Step1;
		w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
		w.Degree = WD_SERIOUS;
		sprintf(w.Result, "螺孔套孔", iScrewIndex);
		//w.Type = W_DOUBLEHOULE_SCREW;
		w.Type = W_DOUBLE_HOLE;
		w.Place = WP_WAIST;
		w.SizeX = 1;
		w.SizeY = 1;
		AddWoundData(w, vCRs[CH_D], crD);
		AddWoundData(w, vCRs[CH_E], crE);
		AddWoundData(w, vCRs[CH_F], crF);
		AddWoundData(w, vCRs[CH_G], crG);
		SetScrewHoleFlag(vCRs[CH_D], crD, 1);
		SetScrewHoleFlag(vCRs[CH_E], crE, 1);
		SetScrewHoleFlag(vCRs[CH_F], crF, 1);
		SetScrewHoleFlag(vCRs[CH_G], crG, 1);
		SetUsedFlag(vCRs[CH_D], crD, 1);
		SetUsedFlag(vCRs[CH_E], crE, 1);
		SetUsedFlag(vCRs[CH_F], crF, 1);
		SetUsedFlag(vCRs[CH_G], crG, 1);
		FillWound2(w, blocks);
		AddToWounds(vWounds, w);

		HolePara hp;
		hp.Block = tempF.Block;
		hp.Step = tempF.Step;
		hp.step2 = tempF.Step1;
		hp.mark = PM_SCREWHOLE;
		hp.flag = 0;
		hp.isDoubleHole = 2;
		hp.isInFork = 0;
		hp.tempD = tempD;
		//hp.tempE = tempE;
		//hp.tempF = tempF;
		hp.Row1 = tempF.Row1;
		hp.Row2 = tempF.Row2;
		hp.Height = hp.Row2 - hp.Row1;
		hp.FRow = iFRow;
		hp.HoleIndex = iScrewIndex;
		hp.RailType = railType;
		hp.FLength = (tempF.Step2 - tempF.Step1) / 2;
		hp.FLength2 = bothLoseRegion / 2;
		g_vHoleParas[hp.step2] = hp;
		g_LastScrewHole = hp;

		Pos posTemp = FindStepInBlock((tempF.Step1 + tempF.Step2) >> 1, blocks, 0);
		hp.Block = posTemp.Block;
		hp.Step = posTemp.Step;
		hp.step2 = (tempF.Step1 + tempF.Step2) >> 1;
		hp.mark = PM_SCREWHOLE;
		hp.flag = 0;
		hp.isDoubleHole = 2;
		hp.isInFork = 0;
		//hp.tempD = tempD;
		hp.tempE = tempE;
		//hp.tempF = tempF;
		hp.Row1 = tempF.Row1;
		hp.Row2 = tempF.Row2;
		hp.Height = hp.Row2 - hp.Row1;
		hp.FRow = iFRow;
		hp.HoleIndex = iScrewIndex;
		hp.RailType = railType;
		hp.FLength = (tempF.Step2 - tempF.Step1) / 2;
		hp.FLength2 = bothLoseRegion / 2;
		g_vHoleParas[hp.step2] = hp;
		g_LastScrewHole = hp;


		VINT crC, crc;
		GetCR(CH_C, stepF1, tempF.Row1 - 10, stepF2, tempF.Row2, blocks, vCRs[CH_C], crC, -1, 1, true);
		GetCR(CH_c, stepF1, tempF.Row1 - 10, stepF2, tempF.Row2, blocks, vCRs[CH_c], crc, -1, 1, true);
		SetScrewHoleFlag(vCRs[CH_C], crC, 1);
		SetScrewHoleFlag(vCRs[CH_c], crc, 1);
		bool carType = blocks[tempF.Block - g_iBeginBlock].BlockHead.detectSet.Identify & C_LR;
		double wdTemp = GetWD(blockHead.walk);
		for (int i = 0; i < crC.size(); ++i)
		{
			GetCRInfo(vCRs[CH_C][crC[i]], DataA, blocks);
			ParseHS(DataA, blocks, vCRs, vCRs[CH_C][crC[i]], vCRs[CH_C][crC[i]].Info, railType, g_direction, carType, iFRow, wdTemp, vWounds, vPMs, bJoint, 0, 1, 0);
			SetUsedFlag(vCRs[CH_C][crC[i]], 1);
		}

		for (int i = 0; i < crc.size(); ++i)
		{
			GetCRInfo(vCRs[CH_c][crc[i]], DataA, blocks);
			ParseHS(DataA, blocks, vCRs, vCRs[CH_c][crc[i]], vCRs[CH_c][crc[i]].Info, railType, g_direction, carType, iFRow, wdTemp, vWounds, vPMs, bJoint, 0, 1, 0);
			SetUsedFlag(vCRs[CH_c][crc[i]], 1);
		}
		return true;
	}

#pragma region 分析接头倒打回波（上裂纹）
	CR tempE2 = tempE;
	if (iScrewIndex == -1 && bJoint)//1孔
	{
		VINT crEt2;
		GetCR(CH_E, tempE.Step2, tempE.Row1, tempE.Step2 + 10, tempF.Row2 + 2, blocks, vCRs[CH_E], crEt2, idxE, 3);
		if (crEt2.size() > 0)
		{
			tempE2 = vCRs[CH_E][crEt2[0]];
			/*CR cr1, cr2;
			if (IsDoubleCR(tempE2, blocks, tempE2.Channel, cr1, cr2) && cr2.Step2 - cr2.Step1 >= 3 && cr2.Row2 >= tempD.Row2 + 2)
			{
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tempE2.Block;
				w.Step = tempE2.Step;
				w.Step2 = tempE2.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔三象限裂纹", iScrewIndex);
				w.Type = W_SCREW_HORIZON_CRACK;
				w.Place = WP_WAIST;
				w.SizeX = (tempE2.Row2 - tempE2.Row1) / 1.6;
				w.SizeY = (tempE2.Row2 - tempE2.Row1) / 1.2;
				AddWoundData(w, tempE2);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
			}*/
			SetUsedFlag(tempE2, 1);
		}
	}

	CR tempD2 = tempD;
	if (iScrewIndex == 1 && bJoint)//1孔
	{
		VINT crDt2;
		if (idxD >= 0)
		{
			GetCR(CH_D, tempD.Step1 - 10, tempD.Row1, tempD.Step2, tempF.Row2 + 2, blocks, vCRs[CH_D], crDt2, idxD, 3);
		}
		else if (tempF.Block > 0)
		{
			GetCR(CH_D, tempF.Step1 - 10, tempF.Row1, tempF.Step2, tempF.Row2 + 2, blocks, vCRs[CH_D], crDt2, idxD, 3);
		}
		if (crDt2.size() > 0)
		{
			tempD2 = vCRs[CH_D][crDt2[0]];
			/*CR cr1, cr2;
			if (IsDoubleCR(tempD2, blocks, CH_E, cr1, cr2) && cr2.Step2 - cr2.Step1 >= 3)
			{
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = tempD2.Block;
				w.Step = tempD2.Step;
				w.Step2 = tempD2.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔一象限裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK1;
				w.Place = WP_WAIST;
				w.SizeX = (cr2.Row2 - cr2.Row1) / 1.6;
				w.SizeY = (cr2.Row2 - cr2.Row1) / 1.2;
				AddWoundData(w, cr1);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
			}*/
			SetUsedFlag(tempD2, 1);
		}
	}
#pragma endregion

#pragma region  4个象限是否有斜裂纹

	ParseScrewHoleSkew3(head, DataA, blockHead, blocks, vCRs, tempD, tempE, tempF, angleE, offsetE, tempE2, iFRow, railType, vWounds, vPMs, bJoint, bAutoJoint, bSew, iScrewIndex, lastHP);

	ParseScrewHoleSkew4(head, DataA, blockHead, blocks, vCRs, tempD, tempE, tempF, angleD, offsetD, tempD2, iFRow, railType, vWounds, vPMs, bJoint, bAutoJoint, bSew, iScrewIndex, lastHP);

	ParseScrewHoleSkew1(head, DataA, blockHead, blocks, vCRs, tempD, tempE, tempF, angleE, offsetE, tempE2, iFRow, railType, vWounds, vPMs, bJoint, bAutoJoint, bSew, iScrewIndex, lastHP);

	ParseScrewHoleSkew2(head, DataA, blockHead, blocks, vCRs, tempD, tempE, tempF, angleD, offsetD, tempD2, iFRow, railType, vWounds, vPMs, bJoint, bAutoJoint, bSew, iScrewIndex, lastHP);

	ParseReverseWaves(head, DataA, blockHead, blocks, vCRs, tempD, tempE, tempF, angleD, offsetD, tempD2, iLoseCh, vF, iFRow, railType, vWounds, vPMs, bJoint, bAutoJoint, bSew, iScrewIndex, lastHP);
#pragma endregion

	//SetUsedFlag(vCRs[CH_D], crD, 1);
	//SetUsedFlag(vCRs[CH_E], crE, 1);
	//SetUsedFlag(vCRs[CH_F], crF2, 1);
	//SetUsedFlag(vCRs[CH_G], crG2, 1);
	SetUsedFlag(vCRs[CH_F], crF, 1);
	SetUsedFlag(vCRs[CH_G], crG, 1);


	//排除螺孔下方的多次回波
	if (idxD >= 0)
	{
		VINT crDt;
		GetCR(CH_D, tempD.Step1, tempD.Row2, tempF.Step2 + 30, iFRow, blocks, vCRs[CH_D], crDt, idxD, 1, false);
		for (int i = 0; i < crDt.size(); ++i)
		{
			if (IsScrewHoleSecondWave(tempD, vCRs[CH_D][crDt[i]], blocks, angleD, offsetD, g_filehead.step))
			{
				SetUsedFlag(vCRs[CH_D][crDt[i]], 1);
				SetScrewHoleFlag(vCRs[CH_D][crDt[i]], 1);
			}

			else if (vCRs[CH_D][crDt[i]].Row1 >= iFRow - 6 && vCRs[CH_D][crDt[i]].Row2 < iFRow &&vCRs[CH_D][crDt[i]].Row2 - vCRs[CH_D][crDt[i]].Row1 <= 3)
			{
				SetUsedFlag(vCRs[CH_D][crDt[i]], 1);
			}
		}
	}
	if (idxE >= 0)
	{
		VINT crEt;
		GetCR(CH_E, tempF.Step1 - 30, tempE.Row2, tempE.Step2, iFRow, blocks, vCRs[CH_E], crEt, idxE, 1, false);
		for (int i = 0; i < crEt.size(); ++i)
		{
			if (IsScrewHoleSecondWave(tempE, vCRs[CH_E][crEt[i]], blocks, angleE, offsetE, g_filehead.step))
			{
				SetUsedFlag(vCRs[CH_E][crEt[i]], 1);
				SetScrewHoleFlag(vCRs[CH_E][crEt[i]], 1);
			}
			else if (vCRs[CH_E][crEt[i]].Row1 >= iFRow - 6 && vCRs[CH_E][crEt[i]].Row2 < iFRow &&vCRs[CH_E][crEt[i]].Row2 - vCRs[CH_E][crEt[i]].Row1 <= 3)
			{
				SetUsedFlag(vCRs[CH_E][crEt[i]], 1);
			}
		}
	}


	VINT crC, crc;
	GetCR(CH_C, stepF1, tempF.Row1 - 10, stepF2, tempF.Row2, blocks, vCRs[CH_C], crC, -1, 1, true);
	GetCR(CH_c, stepF1, tempF.Row1 - 10, stepF2, tempF.Row2, blocks, vCRs[CH_c], crc, -1, 1, true);
	SetScrewHoleFlag(vCRs[CH_C], crC, 1);
	SetScrewHoleFlag(vCRs[CH_c], crc, 1);
	bool carType = blocks[tempF.Block - g_iBeginBlock].BlockHead.detectSet.Identify & C_LR;
	double wdTemp = GetWD(blockHead.walk);
	for (int i = 0; i < crC.size(); ++i)
	{
		GetCRInfo(vCRs[CH_C][crC[i]], DataA, blocks);
		ParseHS(DataA, blocks, vCRs, vCRs[CH_C][crC[i]], vCRs[CH_C][crC[i]].Info, railType, g_direction, carType, iFRow, wdTemp, vWounds, vPMs, bJoint, 0, 1, 0);
		SetScrewHoleFlag(vCRs[CH_C][crC[i]], 1);
		SetUsedFlag(vCRs[CH_C][crC[i]], 1);
	}

	for (int i = 0; i < crc.size(); ++i)
	{
		GetCRInfo(vCRs[CH_c][crc[i]], DataA, blocks);
		ParseHS(DataA, blocks, vCRs, vCRs[CH_c][crc[i]], vCRs[CH_c][crc[i]].Info, railType, g_direction, carType, iFRow, wdTemp, vWounds, vPMs, bJoint, 0, 1, 0);
		SetScrewHoleFlag(vCRs[CH_c][crc[i]], 1);
		SetUsedFlag(vCRs[CH_c][crc[i]], 1);
	}

	HolePara hp;
	hp.Block = tempF.Block;
	hp.Step = tempF.Step;
	hp.step2 = tempF.Step1;
	hp.mark = PM_SCREWHOLE;
	hp.flag = 0;
	hp.isInFork = 0;
	hp.tempD = tempD;
	hp.tempE = tempE;
	hp.tempF = tempF;
	hp.Row1 = tempF.Row1;
	hp.Row2 = tempF.Row2;
	hp.Height = tempF.Row2 - tempF.Row1;
	hp.FRow = iFRow;
	hp.HoleIndex = iScrewIndex;
	hp.RailType = railType;

	hp.FLength = tempF.Step2 - tempF.Step1;
	hp.FLength2 = bothLoseRegion;
	if (tempD.Index >= 0 && tempE.Index >= 0 && hp.FLength2 > hp.FLength)
	{
		hp.FLength2 = (tempE.Step1 + tempE.Step2) / 2 - (tempD.Step1 + tempD.Step2) / 2;
	}

	g_vHoleParas[hp.step2] = hp;
	g_LastScrewHole = hp;

	memset(&pm, 0, sizeof(pm));
	pm.Mark = PM_SCREWHOLE;
	pm.Block = hp.Block;
	pm.Step = hp.Step;
	pm.Step2 = hp.step2;

	if (tempD.Region.size() > 0 && tempE.Region.size() > 0)
	{
		pm.Length = tempE.Step2 - tempD.Step1;
	}
	else if (tempD.Region.size() > 0 && tempF.Region.size() > 0)
	{
		pm.Length = tempF.Step2 - tempD.Step1 + 5;
	}
	else if (tempE.Region.size() > 0 && tempF.Region.size() > 0)
	{
		pm.Length = tempE.Step2 - tempF.Step1 + 5;
	}
	else if (tempF.Region.size() > 0)
	{
		pm.Length = hp.FLength + 8;
	}
	AddToMarks(pm, vPMs);
	return true;
}

bool	ParseScrewHoleNoFG(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex)
{
#ifdef _DEBUG
	Pos posss = FindStepInBlock(step1, blocks, 0);
#endif // _DEBUG

	BLOCK& blockHead = blocks[0].BlockHead;
	//获取D,E的八形波
	double angleD = 0.1 * head.deviceP2.Angle[ACH_D].Refrac, offsetD = head.deviceP2.Place[ACH_D] + blockHead.probOff[ACH_D];
	double angleE = 0.1 * head.deviceP2.Angle[ACH_E].Refrac, offsetE = head.deviceP2.Place[ACH_E] + blockHead.probOff[ACH_E];
	VINT crD, crE, crF, crG, crF2, crG2;

	int stepF1 = 0, stepF2 = 0;
	int iStepDIndex = 0x7FFFFFFF, iStepEIndex = 0;
	int stepFMiddle = (stepF1 + stepF2) >> 1;

	int iminiDsize = (iScrewIndex == 1) ? 1 : 3;
	int iminiEsize = (iScrewIndex == -1) ? 1 : 3;
	uint8_t bFindD = GetCR(CH_D, step1, iLuokong_D_Row1_L[railType], step2, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_D], crD, -1, iminiDsize, true);
	uint8_t bFindE = GetCR(CH_E, step1, iLuokong_D_Row1_L[railType], step2, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_E], crE, -1, iminiEsize, true);
	uint8_t bFindF = GetCR(CH_F, step1, iLuokong_D_Row1_L[railType], step2, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_F], crF2, -1, 1, true);
	uint8_t bFindG = GetCR(CH_G, step1, iLuokong_D_Row1_L[railType], step2, iLuokong_D_Row1_H[railType], blocks, vCRs[CH_G], crG2, -1, 1, true);

	uint8_t bLoseF = GetCR(CH_F, step1, iFRow - 3, step2, iFRow + 20, blocks, vCRs[CH_F], crF, -1, 2, true);//F底部失波
	uint8_t bLoseG = GetCR(CH_G, step1, iFRow - 3, step2, iFRow + 20, blocks, vCRs[CH_G], crG, -1, 2, true);//G底部失波
	if (bFindD)
	{
		int s1 = vCRs[CH_D][crD[0]].Step1, s2 = vCRs[CH_D][crD[0]].Step2, row1 = vCRs[CH_D][crD[0]].Row1, row2 = vCRs[CH_D][crD[0]].Row2;
		if (bFindE == 0)
		{
			bFindE = GetCR(CH_E, s1, row1 - 2, s1 + 20, row2 + 2, blocks, vCRs[CH_E], crE, -1, iminiEsize, true);
		}
		if (bFindF == 0 || bFindG == 0 || bLoseF == 0 || bLoseG == 0)
		{
			bFindF = GetCR(CH_F, s1, row1 - 2, s2 + 10, row2 + 2, blocks, vCRs[CH_F], crF2, -1, 1, true);
			bFindG = GetCR(CH_G, s1, row1 - 2, s2 + 10, row2 + 2, blocks, vCRs[CH_G], crG2, -1, 1, true);
			bLoseF = GetCR(CH_F, s1, iFRow - 3, s2 + 10, iFRow + 10, blocks, vCRs[CH_F], crF, -1, 2, true);
			bLoseG = GetCR(CH_G, s1, iFRow - 3, s2 + 10, iFRow + 10, blocks, vCRs[CH_G], crG, -1, 2, true);
		}
	}
	else if (bFindE)
	{
		int s1 = vCRs[CH_E][crE[0]].Step1, s2 = vCRs[CH_E][crE[0]].Step2, row1 = vCRs[CH_E][crE[0]].Row1, row2 = vCRs[CH_E][crE[0]].Row2;
		if (bFindD == 0)
		{
			bFindD = GetCR(CH_D, s1 - 20, row1 - 2, s1, row2 + 2, blocks, vCRs[CH_D], crD, -1, iminiEsize, true);
		}
		if (bFindF == 0 || bFindG == 0 || bLoseF == 0 || bLoseG == 0)
		{
			bFindF = GetCR(CH_F, s1 - 10, row1 - 2, s2, row2 + 2, blocks, vCRs[CH_F], crF2, -1, 1, true);
			bFindG = GetCR(CH_G, s1 - 10, row1 - 2, s2, row2 + 2, blocks, vCRs[CH_G], crG2, -1, 1, true);
			bLoseF = GetCR(CH_F, s1 - 10, iFRow - 3, s2, iFRow + 10, blocks, vCRs[CH_F], crF, -1, 2, true);
			bLoseG = GetCR(CH_G, s1 - 10, iFRow - 3, s2, iFRow + 10, blocks, vCRs[CH_G], crG, -1, 2, true);
		}
	}


	if (!bLoseF && !bLoseF && (!bJoint && !bSew))
	{
		return false;
	}

	int idxD = -1, idxE = -1;
	int stepD = 0x7FFFFFFF, stepE = 0;
	for (int i = 0; i < crD.size(); ++i)
	{
		if (vCRs[CH_D][crD[i]].Step1 <= stepD && vCRs[CH_D][crD[i]].Row1 < g_iScrewHoleFRowH[railType] + 2)
		{
			stepD = vCRs[CH_D][crD[i]].Step1;
			idxD = crD[i];
		}
	}

	for (int i = 0; i < crE.size(); ++i)
	{
		//if (vCRs[CH_E][crE[i]].Step2 < stepFMiddle + 2)
		if (vCRs[CH_E][crE[i]].Step2 > stepE  && vCRs[CH_E][crE[i]].Row1 < g_iScrewHoleFRowH[railType] + 2)
		{
			stepE = vCRs[CH_E][crE[i]].Step1;
			idxE = crE[i];
		}
	}

	CR tempF;
	if (idxD < 0 && idxE < 0)
	{
		return false;
	}

	if (idxD >= 0 && idxE < 0)
	{
		stepE = stepD + 15;
	}
	else if (idxD < 0 && idxE >= 0)
	{
		stepD = stepE - 15;
	}
	for (int i = stepD; i <= stepE; ++i)
	{
		WaveData wd;
		wd.find = 1;
		wd.row = (g_iScrewHoleFRowL[railType] + g_iScrewHoleFRowH[railType]) / 2;
		wd.step = i;
		tempF.Region.emplace_back(wd);
	}

	tempF.Channel = CH_F;
	FillCR(tempF);

	stepF1 = tempF.Step1;
	stepF2 = tempF.Step2;
	stepFMiddle = (tempF.Step1 + tempF.Step2) >> 1;
	//rowF1 = tempF.Row1;
	//rowF2 = tempF.Row2;

	crF.clear();
	crG.clear();
	bLoseF = GetCR(CH_F, stepF1, iFRow - 3, stepF2, iFRow + 10, blocks, vCRs[CH_F], crF);
	bLoseG = GetCR(CH_G, stepF1, iFRow - 3, stepF2, iFRow + 10, blocks, vCRs[CH_G], crG);
	if (bLoseF == 0 || bLoseG == 0)
	{
		return false;
	}
	else
	{
		int igapF = 0, igapG = 0;
		for (int i = 0; i < crF.size(); ++i)
		{
			if (vCRs[CH_F][crF[i]].Step2 - vCRs[CH_F][crF[i]].Step1 > igapF)
			{
				igapF = vCRs[CH_F][crF[i]].Step2 - vCRs[CH_F][crF[i]].Step1;
			}
		}
		for (int i = 0; i < crG.size(); ++i)
		{
			if (vCRs[CH_G][crG[i]].Step2 - vCRs[CH_G][crG[i]].Step1 > igapG)
			{
				igapG = vCRs[CH_G][crG[i]].Step2 - vCRs[CH_G][crG[i]].Step1;
			}
		}
		if (igapF < 4 || igapG < 4)
		{
			return false;
		}
	}



	Pos posD = FindStepInBlock(stepD, g_vBlockHeads, 0);
	tempF.Block = posD.Block;
	tempF.Step = posD.Step;
	stepF1 = tempF.Step1;
	stepF2 = tempF.Step2;

	VINT crC, crc;
	GetCR(CH_C, stepF1, 13, stepF2, 21, blocks, vCRs[CH_C], crC);
	GetCR(CH_c, stepF1, 13, stepF2, 21, blocks, vCRs[CH_c], crc);
	SetScrewHoleFlag(vCRs[CH_C], crC, 1);
	SetScrewHoleFlag(vCRs[CH_c], crc, 1);
	bool carType = blockHead.detectSet.Identify & C_LR;
	double wdTemp = GetWD(blockHead.walk);
	for (int i = 0; i < crC.size(); ++i)
	{
		GetCRInfo(vCRs[CH_C][crC[i]], DataA, blocks);
		ParseHS(DataA, blocks, vCRs, vCRs[CH_C][crC[i]], vCRs[CH_C][crC[i]].Info, railType, g_direction, carType, iFRow, wdTemp, vWounds, vPMs, bJoint, 0, 1, 0);
	}

	for (int i = 0; i < crc.size(); ++i)
	{
		GetCRInfo(vCRs[CH_c][crc[i]], DataA, blocks);
		ParseHS(DataA, blocks, vCRs, vCRs[CH_c][crc[i]], vCRs[CH_c][crc[i]].Info, railType, g_direction, carType, iFRow, wdTemp, vWounds, vPMs, bJoint, 0, 1, 0);
	}


	CR tempD, tempE;
	int stepD1 = 0, stepD2 = 0, stepE1 = 0, stepE2 = 0;
	int stepDA1 = 0, stepDA2 = 0, stepEA1 = 0, stepEA2 = 0;
	if (idxD >= 0)
	{
		if (crD.size() >= 2 && idxD >= 0)
		{
			for (int i = 0; i < crD.size(); ++i)
			{
				if (i == idxD)
				{
					continue;
				}
				double dh = GetDistanceHA(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
				double dv = GetDistanceVA(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
				int oh = GetOverlappedCountH(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
				int ov = GetOverlappedCountV(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
				if (vCRs[CH_D][crD[i]].Step2 <= vCRs[CH_D][idxD].Step1 && vCRs[CH_D][crD[i]].Row1 >= vCRs[CH_D][idxD].Row2 && vCRs[CH_D][crD[i]].Row1 - vCRs[CH_D][idxD].Row2 <= 3 && vCRs[CH_D][idxD].Step2 - vCRs[CH_D][crD[i]].Step1 <= 5
					||
					vCRs[CH_D][idxD].Step2 <= vCRs[CH_D][crD[i]].Step1 && vCRs[CH_D][idxD].Row2 <= vCRs[CH_D][crD[i]].Row1 && vCRs[CH_D][idxD].Row1 - vCRs[CH_D][crD[i]].Row2 <= 3 && vCRs[CH_D][crD[i]].Step1 - vCRs[CH_D][idxD].Step2 <= 5
					)
				{
					if (dh <= 4 && dv <= 3 && oh <= 2)
					{
						Combine(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
						SetUsedFlag(vCRs[CH_D][crD[i]], 1);
						SetScrewHoleFlag(vCRs[CH_D][crD[i]], 1);
					}
				}
			}
		}
		tempD = vCRs[CH_D][idxD];
		GetCRASteps(CH_D, tempD, stepD1, stepD2, blocks, angleD, offsetD, head.step);
		VINT crDD;
		GetCR(CH_D, stepF1 - 2, tempD.Row1, stepF2 + 20, iFRow - 4, blocks, vCRs[CH_D], crDD, idxD);
		for (int k = 0; k < crDD.size(); ++k)
		{
			int iFirstRow = 0, iLastRow = 0;
			GetCRRowInfo2(vCRs[CH_D][crDD[k]], iFirstRow, iLastRow, CH_D);
			if (iFirstRow < iLastRow + 2)
			{
				continue;
			}
			if (idxD >= 0 && IsScrewHoleSecondWave(tempD, vCRs[CH_D][crDD[k]], blocks, angleD, offsetD, g_filehead.step))
			{
				if (GetOverlappedCountH(tempD, vCRs[CH_D][crDD[k]]) < 3 && GetOverlappedCountV(tempD, vCRs[CH_D][crDD[k]]) < 3)
				{
					SetUsedFlag(vCRs[CH_D][crDD[k]], 1);
					SetScrewHoleFlag(vCRs[CH_D][crDD[k]], 1);
				}
			}
		}

		if (tempD.Row2 - tempD.Row1 >= 5)
		{
			CR cr1, cr2;
			if (IsDoubleCR(tempD, blocks, CH_D, cr1, cr2) && cr2.Step2 - cr2.Step1 >= 3 && cr2.Row1 > tempF.Row2)
			{
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = cr2.Block;
				w.Step = cr2.Step;
				w.Step2 = cr2.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔右侧水平裂纹", iScrewIndex);
				w.Type = W_SCREW_HORIZON_CRACK_RIGHT;
				w.Place = WP_WAIST;
				w.SizeX = (cr2.Row2 - cr2.Row1) / 0.8;
				w.SizeY = 1;
				AddWoundData(w, cr2);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
			}
		}
		else if (tempD.Region.size() >= 25)
		{
			Wound_Judged w;
			w.IsScrewHole = iScrewIndex + 10;
			FillWound(w, blockHead, head);
			w.Block = tempD.Block;
			w.Step = tempD.Step;
			w.Step2 = tempD.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Degree = WD_SERIOUS;
			sprintf(w.Result, "%d孔二象限斜裂纹", iScrewIndex);
			w.Type = W_SCREW_CRACK2;
			w.Place = WP_WAIST;
			w.SizeX = (tempD.Row2 - tempD.Row1) / 0.8;
			w.SizeY = 1;
			AddWoundData(w, tempD);
			FillWound2(w, blocks);
			AddToWounds(vWounds, w);
		}

		SetUsedFlag(tempD, 1);
	}

	if (idxE >= 0)
	{
		if (crE.size() >= 2 && idxE >= 0)
		{
			for (int i = 0; i < crE.size(); ++i)
			{
				if (i == idxE)
				{
					continue;
				}
				double dh = GetDistanceHA(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
				double dv = GetDistanceVA(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
				int oh = GetOverlappedCountH(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
				int ov = GetOverlappedCountV(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
				if (vCRs[CH_E][crE[i]].Step1 >= vCRs[CH_E][idxE].Step2 && vCRs[CH_E][crE[i]].Row1 >= vCRs[CH_E][idxE].Row2 && vCRs[CH_E][crE[i]].Row1 - vCRs[CH_E][idxE].Row2 <= 3 && vCRs[CH_E][crE[i]].Step1 - vCRs[CH_E][idxE].Step2 <= 5
					||
					vCRs[CH_E][crE[i]].Step2 <= vCRs[CH_E][idxE].Step1 && vCRs[CH_E][crE[i]].Row2 <= vCRs[CH_E][idxE].Row1 && vCRs[CH_E][idxE].Row1 - vCRs[CH_E][crE[i]].Row2 <= 3 && vCRs[CH_E][idxE].Step1 - vCRs[CH_E][crE[i]].Step2 <= 5
					)
				{
					if (dh <= 4 && dv <= 3 && oh <= 2)
					{
						Combine(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
						SetUsedFlag(vCRs[CH_E][crE[i]], 1);
						SetScrewHoleFlag(vCRs[CH_E][crE[i]], 1);
					}
				}
			}
		}
		tempE = vCRs[CH_E][idxE];
		GetCRASteps(CH_E, tempE, stepE1, stepE2, blocks, angleE, offsetE, head.step);
		GetCRInfo(tempE, DataA, blocks);
		if (tempE.vASteps.size() > 0)
		{
			stepEA1 = tempE.vASteps[0].Step;
			stepEA2 = tempE.vASteps[tempE.vASteps.size() - 1].Step;
		}

		VINT crEE;
		GetCR(CH_E, stepF1 - 20, tempE.Row1, stepF2 + 2, iFRow - 4, blocks, vCRs[CH_E], crEE, idxE);
		for (int k = 0; k < crEE.size(); ++k)
		{
			int iFirstRow = 0, iLastRow = 0;
			GetCRRowInfo2(vCRs[CH_E][crEE[k]], iFirstRow, iLastRow, CH_E);
			if (iFirstRow > iLastRow - 2)
			{
				continue;
			}
			if (idxE >= 0 && IsScrewHoleSecondWave(tempE, vCRs[CH_E][crEE[k]], blocks, angleE, offsetE, g_filehead.step))
			{
				if (GetOverlappedCountH(tempE, vCRs[CH_E][crEE[k]]) < 3 && GetOverlappedCountV(tempE, vCRs[CH_E][crEE[k]]) < 3)
				{
					SetUsedFlag(vCRs[CH_E][crEE[k]], 1);
					SetScrewHoleFlag(vCRs[CH_E][crEE[k]], 1);
				}
			}
		}

		if (tempE.Row2 - tempE.Row1 >= 5)
		{
			CR cr1, cr2;
			if (IsDoubleCR(tempE, blocks, CH_E, cr1, cr2) && cr2.Step2 - cr2.Step1 >= 3 && cr2.Row1 > tempF.Row2)
			{
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = cr2.Block;
				w.Step = cr2.Step;
				w.Step2 = cr2.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔左侧水平裂纹", iScrewIndex);
				w.Type = W_SCREW_HORIZON_CRACK_LEFT;
				w.Place = WP_WAIST;
				w.SizeX = (cr2.Row2 - cr2.Row1) / 0.8;
				w.SizeY = 1;
				AddWoundData(w, cr2);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
			}
		}
		else if (tempE.Region.size() >= 25)
		{
			Wound_Judged w;
			w.IsScrewHole = iScrewIndex + 10;
			FillWound(w, blockHead, head);
			w.Block = tempE.Block;
			w.Step = tempE.Step;
			w.Step2 = tempE.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Degree = WD_SERIOUS;
			sprintf(w.Result, "%d孔一象限斜裂纹", iScrewIndex);
			w.Type = W_SCREW_CRACK1;
			w.Place = WP_WAIST;
			w.SizeX = (tempE.Row2 - tempE.Row1) / 0.8;
			w.SizeY = 1;
			AddWoundData(w, tempE);
			FillWound2(w, blocks);
			AddToWounds(vWounds, w);
		}

		SetUsedFlag(tempE, 1);
	}


	CR tempE2 = tempE;
	if (iScrewIndex == -1 && bJoint)//1孔
	{
		VINT crEt2;
		GetCR(CH_E, tempE.Step2, tempE.Row1, tempE.Step2 + 10, tempF.Row2 + 2, blocks, vCRs[CH_E], crEt2, idxE, 3);
		if (crEt2.size() > 0)
		{
			tempE2 = vCRs[CH_E][crEt2[0]];
			SetUsedFlag(vCRs[CH_E][crEt2[0]], 1);
			SetScrewHoleFlag(vCRs[CH_E][crEt2[0]], 1);
		}
	}
	CR tempD2 = tempD;
	if (iScrewIndex == 1 && bJoint)//1孔
	{
		VINT crDt2;
		GetCR(CH_E, tempD.Step1 - 10, tempD.Row1, tempD.Step2, tempF.Row2 + 2, blocks, vCRs[CH_D], crDt2, idxD, 3);
		if (crDt2.size() > 0)
		{
			tempD2 = vCRs[CH_D][crDt2[0]];
			SetUsedFlag(vCRs[CH_D][crDt2[0]], 1);
			SetScrewHoleFlag(vCRs[CH_D][crDt2[0]], 1);
		}
	}

	if (idxD >= 0 && idxE >= 0)
	{
		tempF.Row1 = (tempD.Row1 + tempE.Row1) / 2;
		tempF.Row2 = (tempD.Row2 + tempE.Row2) / 2;

	}
	else if (idxD >= 0 && idxE < 0)
	{
		tempF.Row1 = tempD.Row1;
		tempF.Row2 = tempD.Row2;
	}
	else if (idxD < 0 && idxE >= 0)
	{
		tempF.Row1 = tempE.Row1;
		tempF.Row2 = tempE.Row2;
	}
	else
	{
		return false;
	}

	iLastScrewHoleStep = step1;
	if (tempF.Block >= g_iBeginBlock && tempF.Block <= blocks[blocks.size() - 1].Index)
	{
		iLastScrewHoleFRow = blocks[tempF.Block - g_iBeginBlock].vBStepDatas[tempF.Step].FRow;
	}
	else
	{
		iLastScrewHoleFRow = blocks[0].vBStepDatas[tempF.Step].FRow;
	}
	iLastScrewHoleRow = tempF.Row1;
	iLastScrewHoleRow2 = tempF.Row2;
	iLastScrewHoleIndex = iScrewIndex;
	iLastScrewHoleRailType = railType;

	SetScrewHoleFlag(vCRs[CH_F], crF2, 1);
	SetScrewHoleFlag(vCRs[CH_G], crG2, 1);
	SetScrewHoleFlag(vCRs[CH_F], crF, 1);
	SetScrewHoleFlag(vCRs[CH_G], crG, 1);

	PM pm;
	memset(&pm, 0, sizeof(pm));
	pm.Block = tempF.Block;
	pm.Step = tempF.Step;
	pm.Step2 = tempF.Step1;
	if (idxD >= 0)
	{
		pm.Block = tempD.Block;
		pm.Step = tempD.Step;
		pm.Step2 = tempD.Step1;
	}
	pm.Data = iScrewIndex + 10;
	//pm.Walk = GetWD(blocks[pm.Block - g_iBeginBlock].BlockHead.walk, pm.Block, pm.Step, g_direction);
	pm.Mark = PM_SCREWHOLE;
	AddToMarks(pm, vPMs);


	Wound_Judged w;
	//出波
	w.SetEmpty();
	if (ParseScrewHoleCrackRight(head, blockHead, DataA, blocks, vCRs, iFRow, tempF, -1, -1, iScrewIndex, bJoint, bAutoJoint, w, vPMs))
	{
		FillWound2(w, blocks);
		AddToWounds(vWounds, w);
	}

	w.SetEmpty();
	if (ParseScrewHoleCrackLeft(head, blockHead, DataA, blocks, vCRs, iFRow, tempF, -1, -1, iScrewIndex, bJoint, bAutoJoint, w, vPMs))
	{
		FillWound2(w, blocks);
		AddToWounds(vWounds, w);
	}



	//4个象限是否有斜裂纹
	stepFMiddle = (tempF.Step1 + tempF.Step2) >> 1;
	bool b1 = false, b2 = false, b3 = false, b4 = false;

	VINT crEt;
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
			if ((tpE.Row1 + tpE.Row2) / 2 >= (tempF.Row1 + tempF.Row2) / 2 + 3 && tpE.Row2 >= tempF.Row2 + 5 && tpE.Row2 >= tempD.Row2 + 2)
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
	for (int k = 0; k < crEt.size(); ++k)
	{
		CR& tpE = vCRs[CH_E][crEt[k]];
		SetScrewHoleFlag(tpE, 1);
		if (tpE.Region.size() < 3)
		{
			continue;
		}
		int overlappedH = GetOverlappedCountH(tempE2, tpE);
		int overlappedV = GetOverlappedCountV(tempE2, tpE);
		int firstRow, lastRow;
		GetCRRowInfo2(tpE, firstRow, lastRow, CH_E);
		double dv = GetDistanceVA(tempE2, tpE);
		GetCRInfo(tpE, DataA, blocks);
		if (dv < 3.0 && overlappedH >= 2)
		{
			if (tpE.Row1 >= tempF.Row1)
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
				sprintf(w.Result, "%d孔右侧水平裂纹", iScrewIndex);
				w.Type = W_SCREW_HORIZON_CRACK_RIGHT;
				w.Place = WP_WAIST;
				w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpE);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
			}
		}
		if (dv >= 3.0 && dv <= 13 && (overlappedH >= 2 || tpE.Step1 <= tempF.Step2))
		{
			b3 = true;
			if (tpE.Row1 < tempF.Row1)
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
			else if (tpE.Row2 >= tempF.Row2 + 5 && tpE.Row2 >= tempD.Row2 + 2)
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
			}
		}
	}
	SetUsedFlag(vCRs[CH_E], crEt, 1);

	VINT crDt;
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
			if ((tpD.Row1 + tpD.Row2) / 2 >= (tempF.Row1 + tempF.Row2) / 2 + 3 && tpD.Row2 >= tempF.Row2 + 5 && tpD.Row2 >= tempE.Row2 + 2)
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
				AddWoundData(w, tpD);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
	}
	for (int k = 0; k < crDt.size(); ++k)
	{
		CR& tpD = vCRs[CH_D][crDt[k]];
		SetScrewHoleFlag(tpD, 1);
		if (tpD.Region.size() <= 3)
		{
			SetUsedFlag(tpD, 1);
			continue;
		}
		int overlappedH = GetOverlappedCountH(tempD2, tpD);
		int overlappedV = GetOverlappedCountV(tempD2, tpD);
		GetCRInfo(tpD, DataA, blocks);
		double dv = GetDistanceVA(tempD2, tpD);
		if (dv <= 12 && (overlappedH >= 2 || tpD.Step2 >= tempF.Step1))
		{
			if (dv < 3.0 && tpD.Row1 < tempF.Row1)
			{
				SetUsedFlag(tpD, 1);
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = vCRs[CH_D][crDt[k]].Block;
				w.Step = vCRs[CH_D][crDt[k]].Step;
				w.Step2 = vCRs[CH_D][crDt[k]].Step1;
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
				break;
			}
			else if (dv < 3.0 && tpD.Row2 > tempF.Row2 + 1)
			{
				SetUsedFlag(tpD, 1);
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = vCRs[CH_D][crDt[k]].Block;
				w.Step = vCRs[CH_D][crDt[k]].Step;
				w.Step2 = vCRs[CH_D][crDt[k]].Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔右侧水平裂纹", iScrewIndex);
				w.Type = W_SCREW_HORIZON_CRACK_RIGHT;
				w.Place = WP_WAIST;
				w.SizeX = (tpD.Row2 - tpD.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, tpD);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
			else if (dv >= 3.0 && tpD.Row2 >= tempF.Row2 + 5 && tpD.Row2 >= tempE.Row2 + 2)
			{
				SetUsedFlag(tpD, 1);
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = vCRs[CH_D][crDt[k]].Block;
				w.Step = vCRs[CH_D][crDt[k]].Step;
				w.Step2 = vCRs[CH_D][crDt[k]].Step1;
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
				AddWoundData(w, tpD);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
				break;
			}
		}
	}
	SetUsedFlag(vCRs[CH_D], crDt, 1);

	crEt.clear();
	GetCR(CH_E, tempF.Step1 - 10, tempF.Row1 - 3, tempF.Step2 + 5, tempF.Row2 + 3, blocks, vCRs[CH_E], crEt, idxE, 1, true);
	for (int k = 0; k < crEt.size(); ++k)
	{
		CR& tpE = vCRs[CH_E][crEt[k]];
		SetScrewHoleFlag(tpE, 1);
		if (tpE.Region.size() <= 3 || tpE.Step1 > stepFMiddle + 3)
		{
			continue;
		}
		int iFirstRow = 0, iLastRow = 0;
		GetCRRowInfo2(tpE, iFirstRow, iLastRow, CH_E);
		if (iFirstRow >= iLastRow)
		{
			continue;
		}
		int iEStep2 = (tpE.Step2 + tpE.Step1) >> 1;
		GetCRInfo(tpE, DataA, blocks);
		if (idxE >= 0)
		{
			double dh = GetDistanceHA(tempE, tpE);
			double dv = GetDistanceVA(tempE, tpE);
			int overlappedH = GetOverlappedCountH(tempE, tpE);
			int overlappedV = GetOverlappedCountV(tempE, tpE);
			if ((dh >= 5.0 && dh <= 11 || dh >= 5.0 && dh <= 18 || tpE.Region.size() > 20) && (dv <= 5 || overlappedV >= 3) && overlappedH <= 4)
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
					AddWoundData(w, vCRs[CH_E], crEt);
					FillWound2(w, blocks);
					AddToWounds(vWounds, w);
					break;
				}
				if (/*iEStep2 < stepF2 - 3 && */tpE.Row1 <= tempF.Row2 + 3)
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
					AddWoundData(w, vCRs[CH_E], crEt);
					FillWound2(w, blocks);
					AddToWounds(vWounds, w);
					break;
				}
			}
			else if (dh > 11 && dh < 18)
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
				w.Type = W_SCREW_CRACK2;
				w.Place = WP_WAIST;
				w.SizeX = (tpE.Row2 - tpE.Row1) / 0.6;
				w.SizeY = 1;
				AddWoundData(w, vCRs[CH_E], crEt);
				FillWound2(w, blocks);
				AddToWounds(vWounds, w);
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
			AddWoundData(w, vCRs[CH_E], crEt);
			FillWound2(w, blocks);
			AddToWounds(vWounds, w);
			break;
		}
	}
	SetUsedFlag(vCRs[CH_E], crEt, 1);

	crDt.clear();
	uint8_t iFindD_2 = GetCR(CH_D, tempF.Step1, tempF.Row1 - 3, tempF.Step2 + 5, tempF.Row2 + 3, blocks, vCRs[CH_D], crDt, idxD, 1);
	for (int k = 0; k < crDt.size(); ++k)
	{
		CR& tpD = vCRs[CH_D][crDt[k]];
		SetScrewHoleFlag(tpD, 1);
		if (tpD.Region.size() <= 3)
		{
			SetUsedFlag(tpD, 1);
			continue;
		}

		int firstRow = 0, lastRow = 0;
		GetCRRowInfo2(tpD, firstRow, lastRow, CH_D);
		if (firstRow <= lastRow)
		{
			continue;
		}

		SetUsedFlag(tpD, 1);
		GetCRInfo(tpD, DataA, blocks);
		if (idxD >= 0 && tpD.Step2 > stepFMiddle - 3)
		{
			double dh = GetDistanceHA(tempD, tpD);
			if ((dh >= 5.0 && dh <= 11 || dh >= 5.0 && dh <= 18 || tpD.Region.size() > 20))//11
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
				break;
			}
			else if (dh > 11 && dh < 18 || tpD.Region.size() > 20)
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
			if (tpD.Step1 >= (tempF.Step1 + tempF.Step2) / 2 || tpD.Step2 >= (tempF.Step1 + 2 * tempF.Step2) / 3)//11
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
				break;
			}
		}
	}

	if (bJoint && (iScrewIndex == -1 || iScrewIndex == 0))//需考虑倒打
	{
		VINT crDDD;
		bool bFinddddd = GetCR(CH_D, (tempF.Step1 + tempF.Step2) / 2, iLuokong_D_Row1_L[railType] + 3, tempF.Step2 + 15, iLuokong_D_Row1_H[railType] + 10, blocks, vCRs[CH_D], crDDD, idxD, 1, true);
		for (int i = 0; i < crDDD.size(); ++i)
		{
			CR& td = vCRs[CH_D][crDDD[i]];
			SetScrewHoleFlag(td, 1);
			int lastRow = 0, firstRow = 0;
			GetCRRowInfo2(td, firstRow, lastRow, CH_E);
			int dF = td.Row1 - tempF.Row2;
			if (td.Region.size() > 3 && firstRow < lastRow && td.Row1 > tempF.Row2 + 2 && dF <= 10 && td.Row2 >= tempF.Row2 + 5 && td.Row2 >= tempE.Row2 + 2)//
			{
				Wound_Judged w;
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
		bool bFindeeee = GetCR(CH_E, (tempF.Step1 + tempF.Step2) / 2, iLuokong_D_Row1_L[railType] + 3, tempF.Step2 + 15, iLuokong_D_Row1_H[railType] + 10, blocks, vCRs[CH_E], crEEE, idxE, 1, true);
		for (int i = 0; i < crEEE.size(); ++i)
		{
			CR& te = vCRs[CH_E][crEEE[i]];
			SetScrewHoleFlag(te, 1);
			int lastRow = 0, firstRow = 0;
			GetCRRowInfo2(te, firstRow, lastRow, CH_D);
			if (te.Region.size() > 3 && firstRow < lastRow && te.Row1 > tempF.Row2 + 2 && te.Row2 >= tempF.Row2 + 5 && te.Row2 >= tempD.Row2 + 2)
			{
				Wound_Judged w;
				w.IsScrewHole = iScrewIndex + 10;
				FillWound(w, blockHead, head);
				w.Block = te.Block;
				w.Step = te.Step;
				w.Step2 = te.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Degree = WD_SERIOUS;
				sprintf(w.Result, "%d孔倒打三象限斜裂纹", iScrewIndex);
				w.Type = W_SCREW_CRACK3;
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
	else if (bJoint && (iScrewIndex == 1 || iScrewIndex == 0))//需考虑倒打
	{
		VINT crDDD;
		bool bFindddd = GetCR(CH_D, tempF.Step1 - 15, iLuokong_D_Row1_L[railType] + 2, (tempF.Step1 + tempF.Step2) / 2, iLuokong_D_Row1_H[railType] + 10, blocks, vCRs[CH_D], crDDD, idxD, 1, true);
		for (int i = 0; i < crDDD.size(); ++i)
		{
			CR& td = vCRs[CH_D][crDDD[i]];
			SetScrewHoleFlag(td, 1);
			if (td.Region.size() <= 3)
			{
				continue;
			}

			int lastRow = 0, firstRow = 0;
			GetCRRowInfo2(td, firstRow, lastRow, CH_E);
			if (td.Region.size() > 3 && firstRow >= lastRow && td.Row1 > tempF.Row2 + 2 && td.Row2 >= tempF.Row2 + 5 && td.Row2 >= tempE.Row2 + 2)
			{
				Wound_Judged w;
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
		bool bFindeeee = GetCR(CH_E, tempF.Step1 - 15, iLuokong_D_Row1_L[railType] + 2, (tempF.Step1 + tempF.Step2) / 2, iLuokong_D_Row1_H[railType] + 10, blocks, vCRs[CH_E], crEEE, idxE, 1, true);
		for (int i = 0; i < crEEE.size(); ++i)
		{
			CR& te = vCRs[CH_E][crEEE[i]];
			SetScrewHoleFlag(te, 1);
			if (te.Region.size() <= 3)
			{
				continue;
			}

			int lastRow = 0, firstRow = 0;
			GetCRRowInfo2(te, firstRow, lastRow, CH_D);
			if (te.Region.size() > 3 && firstRow > lastRow  && te.Row1 > tempF.Row2 + 2 && te.Row2 >= tempF.Row2 + 5 && te.Row2 >= tempD.Row2 + 2)
			{
				Wound_Judged w;
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

	if (idxD >= 0)
	{
		crDt.clear();
		GetCR(CH_D, tempD.Step1, tempD.Row2, tempF.Step2 + 15, iFRow - 5, blocks, vCRs[CH_D], crDt, idxD, 1, false);
		for (int i = 0; i < crDt.size(); ++i)
		{
			if (IsScrewHoleSecondWave(tempD, vCRs[CH_D][crDt[i]], blocks, angleD, offsetD, g_filehead.step))
			{
				SetUsedFlag(vCRs[CH_D][crDt[i]], 1);
				SetScrewHoleFlag(vCRs[CH_D][crDt[i]], 1);
			}
		}
	}
	if (idxE >= 0)
	{
		crEt.clear();
		GetCR(CH_E, tempF.Step1 - 15, tempE.Row2, tempE.Step2, iFRow - 5, blocks, vCRs[CH_E], crEt, idxE, 1, false);
		for (int i = 0; i < crEt.size(); ++i)
		{
			if (IsScrewHoleSecondWave(tempE, vCRs[CH_E][crEt[i]], blocks, angleE, offsetE, g_filehead.step))
			{
				SetUsedFlag(vCRs[CH_E][crEt[i]], 1);
				SetScrewHoleFlag(vCRs[CH_E][crEt[i]], 1);
			}
		}
	}

	SetUsedFlag(vCRs[CH_D], crD, 1);
	SetUsedFlag(vCRs[CH_E], crE, 1);
	SetUsedFlag(vCRs[CH_F], crF2, 1);
	SetUsedFlag(vCRs[CH_G], crG2, 1);
	SetUsedFlag(vCRs[CH_F], crF, 1);
	SetUsedFlag(vCRs[CH_G], crG, 1);

	HolePara hp;
	hp.Block = tempF.Block;
	hp.Step = tempF.Step;
	hp.step2 = tempF.Step1;
	hp.mark = PM_SCREWHOLE;
	hp.flag = 0;
	hp.isInFork = 0;
	hp.tempD = tempD;
	hp.tempE = tempE;
	hp.tempF = tempF;
	hp.Row1 = tempF.Row1;
	hp.Row2 = tempF.Row2;
	hp.Height = tempF.Row2 - tempF.Row1;
	hp.FRow = iFRow;
	hp.HoleIndex = iScrewIndex;
	hp.RailType = railType;
	hp.FLength = tempF.Step2 - tempF.Step1;
	//TODO:
	hp.FLength2 = hp.FLength;

	g_vHoleParas[hp.step2] = hp;
	g_LastScrewHole = hp;

	memset(&pm, 0, sizeof(pm));
	pm.Mark = PM_SCREWHOLE;
	pm.Block = hp.Block;
	pm.Step = hp.Step;
	pm.Step2 = hp.step2;
	if (tempD.Region.size() > 0 && tempE.Region.size() > 0)
	{
		pm.Length = tempE.Step2 - tempD.Step1;
	}
	else if (tempD.Region.size() > 0 && tempF.Region.size() > 0)
	{
		pm.Length = tempF.Step2 - tempD.Step1 + 5;
	}
	else if (tempE.Region.size() > 0 && tempF.Region.size() > 0)
	{
		pm.Length = tempE.Step2 - tempF.Step1 + 5;
	}
	else if (tempF.Region.size() > 0)
	{
		pm.Length = hp.FLength + 8;
	}
	AddToMarks(pm, vPMs);
	return true;
}

bool	ParseGuideHole(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR&cr, int i, int16_t iDesiredFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs)
{
	VINT crD, crF, crG, crE, crF2, crG2;
	double angleD = 0.1 * head.deviceP2.Angle[ACH_D].Refrac;
	double angleE = 0.1 * head.deviceP2.Angle[ACH_E].Refrac;

	BLOCK blockHead = blocks[cr.Block - g_iBeginBlock].BlockHead;
	double offsetD = head.deviceP2.Place[ACH_D] + blockHead.probOff[ACH_D];
	double offsetE = head.deviceP2.Place[ACH_E] + blockHead.probOff[ACH_E];

	uint8_t bFindD = 0, bFindE = 0, bFindF = 0, bFindG = 0, bLoseF = 0, bLoseG = 0;
	int stepD1 = 0, stepD2 = 0, stepE1 = 0, stepE2 = 0;
	int s1 = cr.Step1, s2 = cr.Step2 + 10, row1 = cr.Row1 - 2, row2 = cr.Row2;

	int stepELeft = cr.Step2;
	bool bFront = cr.Channel == CH_A1 || cr.Channel == CH_B1 || cr.Channel == CH_C || cr.Channel == CH_D;
	if (cr.Channel == CH_D)
	{
		s1 = cr.Step1;
		s2 = max(cr.Step2 + 20, cr.Step1 + 24);
		stepELeft = s2;
	}
	else if (cr.Channel == CH_E)
	{
		s1 = cr.Step1 - 15;
		s2 = cr.Step2;
		stepELeft = cr.Step2;
	}
	else if (bFront)
	{
		s1 = s1 - 15;
		stepELeft = cr.Step2 + 15;
	}
	else
	{
		stepELeft = cr.Step2 + 4;
	}
	if (cr.Channel == CH_C || cr.Channel == CH_c)
	{
		row1 = cr.Row1;
		row2 = cr.Row2 + 16;
	}

	if (bFront)
	{
		bFindD = GetCR(CH_D, s1 - 2, row1, s2, row2, blocks, vCRs[CH_D], crD, -1, 2);
		bFindE = GetCR(CH_E, s1 - 2, row1, stepELeft, row2, blocks, vCRs[CH_E], crE, -1, 2);
		if (bFindE && vCRs[CH_E][crE[0]].Step2 > cr.Step2)
		{
			s2 = vCRs[CH_E][crE[0]].Step2;
			stepELeft = s2;
		}
		bFindF = GetCR(CH_F, s1 - 2, row1, s2, row2, blocks, vCRs[CH_F], crF2, -1, 2);//F螺孔高度出波
		bFindG = GetCR(CH_G, s1 - 2, row1, s2, row2, blocks, vCRs[CH_G], crG2, -1, 2);//G螺孔高度出波
	}
	else
	{
		bFindD = GetCR(CH_D, s1 - 8, row1, s2, row2, blocks, vCRs[CH_D], crD, -1, 2);
		bFindE = GetCR(CH_E, s1 - 5, row1, s2, row2, blocks, vCRs[CH_E], crE);
		bFindF = GetCR(CH_F, s1 - 5, row1, s2, row2, blocks, vCRs[CH_F], crF2, -1, 2);//F螺孔高度出波
		bFindG = GetCR(CH_G, s1 - 5, row1, s2, row2, blocks, vCRs[CH_G], crG2, -1, 2);//G螺孔高度出波		
	}
	

	HolePara lastHPScrew;
	GetNearestHole(cr.Step1, PM_SCREWHOLE, lastHPScrew);

	HolePara lastHPGuide;
	GetNearestHole(cr.Step1, PM_GUIDEHOLE, lastHPGuide);


	if (bFindF || bFindG)//FG至少有没有
	{
		for (int i = crF2.size() - 1; i >= 0; --i)
		{
			if (cr.Channel == CH_D)
			{
				CR& tpF = vCRs[CH_F][crF2[i]];
				if (tpF.Step2 - tpF.Step1 >= 7 && tpF.Step1 - cr.Step1 >= 5)
				{
					crF2.erase(crF2.begin() + i);
				}
			}
		}
		bFindF = RemoveCRByLengthLimit(vCRs[CH_F], crF2, 30);

		for (int i = crG2.size() - 1; i >= 0; --i)
		{
			CR& tpG = vCRs[CH_G][crG2[i]];
			if (cr.Channel == CH_D)
			{
				if (tpG.Step2 - tpG.Step1 >= 7 && tpG.Step1 - cr.Step1 >= 5)
				{
					crG2.erase(crG2.begin() + i);
				}
			}
		}
		bFindG = RemoveCRByLengthLimit(vCRs[CH_G], crG2, 30);

		/*
		if (bFindF && !bFindG)
		{
			if (vCRs[CH_F][crF2[0]].Step2 - vCRs[CH_F][crF2[0]].Step1 >= 4)
			{
				s1 = vCRs[CH_F][crF2[0]].Step1;
				s2 = vCRs[CH_F][crF2[0]].Step2;
				row1 = vCRs[CH_F][crF2[0]].Row1;
				row2 = vCRs[CH_F][crF2[0]].Row2;
				for (int k = 1; k < crF2.size(); ++k)
				{
					CR& temp = vCRs[CH_F][crF2[k]];
					if (temp.Step2 - temp.Step1 < 4)
					{
						continue;
					}
					if (temp.Step1 < s1)
					{
						s1 = temp.Step1;
					}
					if (temp.Step2 > s2)
					{
						s2 = temp.Step2;
					}

					if (temp.Row1 < row1)
					{
						row1 = temp.Row1;
					}
					if (temp.Row2 > row2)
					{
						row2 = temp.Row2;
					}
				}
			}
		}
		else if (!bFindF && bFindG)
		{
			if (vCRs[CH_G][crG2[0]].Step2 - vCRs[CH_G][crG2[0]].Step1 >= 4)
			{
				s1 = vCRs[CH_G][crG2[0]].Step1;
				s2 = vCRs[CH_G][crG2[0]].Step2;
				row1 = vCRs[CH_G][crG2[0]].Row1;
				row2 = vCRs[CH_G][crG2[0]].Row2;
				for (int k = 1; k < crG2.size(); ++k)
				{
					CR& temp = vCRs[CH_G][crG2[k]];
					if (temp.Step2 - temp.Step1 < 4)
					{
						continue;
					}
					if (temp.Step1 < s1)
					{
						s1 = temp.Step1;
					}
					if (temp.Step2 > s2)
					{
						s2 = temp.Step2;
					}
					if (temp.Row1 < row1)
					{
						row1 = temp.Row1;
					}
					if (temp.Row2 > row2)
					{
						row2 = temp.Row2;
					}
				}
			}
		}
		else if (bFindF && bFindG)
		{
			if (vCRs[CH_F][crF2[0]].Step2 - vCRs[CH_F][crF2[0]].Step1 >= 4)
			{


				s1 = vCRs[CH_F][crF2[0]].Step1;
				s2 = vCRs[CH_F][crF2[0]].Step2;
				row1 = vCRs[CH_F][crF2[0]].Row1;
				row2 = vCRs[CH_F][crF2[0]].Row2;
				for (int k = 1; k < crF2.size(); ++k)
				{
					CR& temp = vCRs[CH_F][crF2[k]];
					if (temp.Step2 - temp.Step1 < 4)
					{
						continue;
					}
					if (temp.Step1 < s1)
					{
						s1 = temp.Step1;
					}
					if (temp.Step2 > s2)
					{
						s2 = temp.Step2;
					}

					if (temp.Row1 < row1)
					{
						row1 = temp.Row1;
					}
					if (temp.Row2 > row2)
					{
						row2 = temp.Row2;
					}
				}
				for (int k = 0; k < crG2.size(); ++k)
				{
					CR& temp = vCRs[CH_G][crG2[k]];
					if (temp.Step2 - temp.Step1 < 4)
					{
						continue;
					}
					if (temp.Step1 < s1)
					{
						s1 = temp.Step1;
					}
					if (temp.Step2 > s2)
					{
						s2 = temp.Step2;
					}
					if (temp.Row1 < row1)
					{
						row1 = temp.Row1;
					}
					if (temp.Row2 > row2)
					{
						row2 = temp.Row2;
					}
				}
			}
		}
		*/
	}


	crD.clear();
	bFindD = GetCR(CH_D, s1 - 5, row1 - 2, stepELeft, row2 + 4, blocks, vCRs[CH_D], crD, -1, 1, true);
	if (cr.Channel == CH_D)
	{
		bFindD = RemoveHoleCRExcept(vCRs[CH_D], crD, cr.Index);
		crD.emplace_back(cr.Index);
	}
	else
	{
		bFindD = RemoveHoleCR(vCRs[CH_D], crD);
	}

	crF2.clear();
	bFindF = GetCR(CH_F, s1, row1 - 2, stepELeft, row2 + 1, blocks, vCRs[CH_F], crF2, -1, 2, true);//G螺孔高度出波
	bFindF = RemoveHoleCR(vCRs[CH_F], crF2);
	bFindF = RemoveCRByLengthLimit(vCRs[CH_F], crF2, 60);

	crF.clear();
	bLoseF = GetCR(CH_F, s1, iDesiredFRow - 3, stepELeft, iDesiredFRow + 3, blocks, vCRs[CH_F], crF, -1, 1, true);

	crG2.clear();
	bFindG = GetCR(CH_G, s1, row1 - 2, stepELeft, row2 + 1, blocks, vCRs[CH_G], crG2, -1, 2, true);//G螺孔高度出波
	bFindG = RemoveHoleCR(vCRs[CH_G], crG2);
	bFindG = RemoveCRByLengthLimit(vCRs[CH_G], crG2, 60);

	//螺孔的标准出波
	CR tempFG, tempD, tempE;
	if (bFindF)
	{
		tempFG = vCRs[CH_F][crF2[0]];
		for (int i = 1; i < crF2.size(); ++i)
		{
			TryCombineFGInHole(tempFG, vCRs[CH_F][crF2[i]], 10);
		}
		if (bFindG)
		{
			for (int i = 0; i < crG2.size(); ++i)
			{
				TryCombineFGInHole(tempFG, vCRs[CH_G][crG2[i]], 10);
			}
		}
	}
	else if (bFindG)
	{
		tempFG = vCRs[CH_G][crG2[0]];
		for (int i = 1; i < crG2.size(); ++i)
		{
			TryCombineFGInHole(tempFG, vCRs[CH_G][crG2[i]], 10);
		}
	}

	crE.clear();
	if (bFindF || bFindG)
	{
		bFindE = GetCR(CH_E, min(cr.Step2, tempFG.Step1), tempFG.Row1 - 2, max(stepELeft, tempFG.Step2 + 5), tempFG.Row2 + 4, blocks, vCRs[CH_E], crE, -1, 1, true);
	}
	else
	{
		bFindE = GetCR(CH_E, s1 - 2, row1 - 2, stepELeft, row2 + 4, blocks, vCRs[CH_E], crE, -1, 1, true);
	}
	bFindE = RemoveHoleCR(vCRs[CH_E], crE);

	crG.clear();
	bLoseG = GetCR(CH_G, s1 - 2, iDesiredFRow - 3, stepELeft, iDesiredFRow + 3, blocks, vCRs[CH_G], crG, -1, 1, true);

	for (int i = 0; i < crD.size(); ++i)
	{
		if (vCRs[CH_D][crD[i]].Row1 < row1)
		{
			row1 = vCRs[CH_D][crD[i]].Row1;
		}
	}
	for (int i = 0; i < crE.size(); ++i)
	{
		if (vCRs[CH_E][crE[i]].Row1 < row1)
		{
			row1 = vCRs[CH_E][crE[i]].Row1;
		}
	}

	int ijawRow = 0, ifRow = 0;
	int rt = 0;
	GetJawRow(0, s1, ijawRow, ifRow, rt);

	if (row1 < ijawRow + 5)
	{
		return false;
		row1 = ijawRow + 5;
	}

	if (bFindD + bFindE + bFindF + bFindG < 2)
	{
		return false;
	}

	for (int index = crF.size() - 1; index >= 0; --index)
	{
		if (vCRs[CH_F][crF[index]].IsLose == 0)
		{
			crF.erase(crF.begin() + index);
		}
	}
	for (int index = crG.size() - 1; index >= 0; --index)
	{
		if (vCRs[CH_G][crG[index]].IsLose == 0)
		{
			crG.erase(crG.begin() + index);
		}
	}
	bLoseF = crF.size() > 0;
	bLoseG = crG.size() > 0;


	iLastGuideHoleRow = row1;
	iLastGuideHoleRow2 = row2;
	iLastGuideHoleFRow = iDesiredFRow;
	iLastGuideHoleStep = cr.Step1;
	iLastGuideHoleFLength = 0;
	if (bFindF)
	{
		iLastGuideHoleFLength = vCRs[CH_F][crF2[0]].Step2 - vCRs[CH_F][crF2[0]].Step1;
		iLastGuideHoleRow = vCRs[CH_F][crF2[0]].Row1;
		iLastGuideHoleRow2 = vCRs[CH_F][crF2[0]].Row2;
	}
	else if (bFindG)
	{
		iLastGuideHoleFLength = vCRs[CH_G][crG2[0]].Step2 - vCRs[CH_G][crG2[0]].Step1;
		iLastGuideHoleRow = vCRs[CH_G][crG2[0]].Row1;
		iLastGuideHoleRow2 = vCRs[CH_G][crG2[0]].Row2;
	}
	iLastGuideHoleFLength2 = 0;
	if (bLoseF && bLoseG)
	{
		int bs = 0, es = 0;
		int s1 = vCRs[CH_F][crF[0]].Step1, s2 = vCRs[CH_F][crF[0]].Step2;
		int s3 = vCRs[CH_G][crG[0]].Step1, s4 = vCRs[CH_G][crG[0]].Step2;
		iLastGuideHoleFLength2 = GetOverlappedStep(s1, s2, s3, s4, bs, es);
	}

	if (bFindF || bFindG)
	{
		uint32_t ssss1 = 0x7FFFFFFF, ssss2 = 0;
		GetCRsStep(vCRs[CH_F], crF2, ssss1, ssss2);
		GetCRsStep(vCRs[CH_G], crG2, ssss1, ssss2);

		VINT vtempF, vtempG;
		GetCR(CH_F, ssss1, iLastGuideHoleRow2 + 1, ssss2, 40, blocks, vCRs[CH_F], vtempF, crF2);
		GetCR(CH_G, ssss1, iLastGuideHoleRow2 + 1, ssss2, 40, blocks, vCRs[CH_G], vtempG, crG2);

		for (int i = 0; i < vtempF.size(); ++i)
		{
			CR& tpF = vCRs[CH_F][vtempF[i]];
			if (tpF.Step1 >= ssss1 && tpF.Step2 <= ssss2 && tpF.IsLose == 0)
			{
				SetGuideHoleFlag(tpF, 1);
				SetUsedFlag(tpF, 1);
			}
		}

		for (int i = 0; i < vtempG.size(); ++i)
		{
			CR& tpG = vCRs[CH_G][vtempG[i]];
			if (tpG.Step1 >= ssss1 && tpG.Step2 <= ssss2 && tpG.IsLose == 0)
			{
				SetGuideHoleFlag(tpG, 1);
				SetUsedFlag(tpG, 1);
			}
		}
	}

	int istepMiddle = 0;
	if (bFindF)	istepMiddle = (vCRs[CH_F][crF2[0]].Step1 + vCRs[CH_F][crF2[0]].Step2) >> 1;
	else if (bFindG) istepMiddle = (vCRs[CH_G][crG2[0]].Step1 + vCRs[CH_G][crG2[0]].Step2) >> 1;
	else if (bFindD && bFindE)
	{
		istepMiddle = (vCRs[CH_D][crD[0]].Step1 + vCRs[CH_E][crE[0]].Step1 + vCRs[CH_D][crD[0]].Step2 + vCRs[CH_E][crE[0]].Step2) >> 2;
	}



	int idxD = -1, idxE = -1;
	/**********************2020-03-28**************************/
	uint32_t ss1 = 0x7FFFFFFF, ss2 = 0;
	if (bFindF || bFindG)
	{
		GetCRsStep(vCRs[CH_F], crF2, ss1, ss2);
		GetCRsStep(vCRs[CH_G], crG2, ss1, ss2);
	}

	bool hasNeiberScrew = Abs(lastHPScrew.step2 - s1) <= 200;	
	if (crD.size() > 0)
	{
		if ((bFindF || bFindG) && ss2 - ss1 >= 4)
		{
			for (int i = crD.size() - 1; i >= 0; --i)
			{
				CR& tpD = vCRs[CH_D][crD[i]];
				if (Abs(lastHPGuide.step2 - cr.Step1) < 500 && lastHPGuide.tempF.Step2 > 0 && lastHPGuide.tempD.Step2 > 0 && lastHPGuide.tempF.Step2 - lastHPGuide.tempD.Step1 > ss2 - tpD.Step1)
				{
					continue;
				}
				if (hasNeiberScrew && lastHPScrew.tempF.Step2 > 0 && lastHPScrew.tempD.Step1 > 0 && ss2 - tpD.Step1 < lastHPScrew.tempF.Step2 - lastHPScrew.tempD.Step1)
				{
					continue;
				}
				if (tpD.Step2 > ss2 + 5 || tpD.Step2 < ss1 - 5)
				{
					crD.erase(crD.begin() + i);
				}
			}
		}
	}

	if (crE.size() > 0)
	{
		if (cr.Channel == CH_D)
		{
			if (bFindF || bFindG)
			{				
				if (ss2 - ss1 >= 4)
				{
					for (int i = crE.size() - 1; i >= 0; --i)
					{
						CR& tpE = vCRs[CH_E][crE[i]];
						if (Abs(lastHPGuide.step2 - cr.Step1) < 500 && lastHPGuide.tempF.Step2 > 0 && lastHPGuide.tempE.Step2 > 0 && lastHPGuide.tempE.Step2 - lastHPGuide.tempF.Step1 > cr.Step2 - ss1)
						{
							continue;
						}
						if (hasNeiberScrew && lastHPScrew.tempF.Step2 > 0 && lastHPScrew.tempE.Step1 > 0 && tpE.Step2 - ss1 < lastHPScrew.tempE.Step2 - lastHPScrew.tempF.Step1)
						{
							continue;
						}
						if (tpE.Step1 <= ss2 + 5)
						{
							continue;
						}
						crE.erase(crE.begin() + i);
					}
				}
			}
			else
			{
				for (int i = crE.size() - 1; i >= 0; --i)
				{
					CR& tpE = vCRs[CH_E][crE[i]];
					if (Abs(lastHPGuide.step2 - cr.Step1) < 500 && lastHPGuide.tempD.Step2 > 0 && lastHPGuide.tempE.Step2 > 0 && lastHPGuide.tempE.Step1 - lastHPGuide.tempD.Step2 >= tpE.Step1 - cr.Step2)
					{
						continue;
					}
					if (tpE.Step1 - cr.Step2 >= 20)
					{
						crE.erase(crE.begin() + i);
					}
				}
			}
		}

		if (crE.size() > 0)
		{
			int iLeftEIndex = crE[0];
			int iLeftEStep = vCRs[CH_E][iLeftEIndex].Step2;
			for (int i = 1; i < crE.size(); ++i)
			{
				if (vCRs[CH_E][crE[i]].Step2 > iLeftEStep)
				{
					iLeftEStep = vCRs[CH_E][crE[i]].Step2;
					iLeftEIndex = crE[i];
				}
			}
			for (int i = crF2.size() - 1; i >= 0; --i)
			{
				if (vCRs[CH_F][crF2[i]].Step1 > iLeftEStep)
				{
					crF2.erase(crF2.begin() + i);
				}
			}
			for (int i = crG2.size() - 1; i >= 0; --i)
			{
				if (vCRs[CH_G][crG2[i]].Step1 > iLeftEStep)
				{
					crG2.erase(crG2.begin() + i);
				}
			}

			for (int i = crD.size() - 1; i >= 0; --i)
			{
				if (vCRs[CH_D][crD[i]].Step1 > iLeftEStep)
				{
					crD.erase(crD.begin() + i);
				}
			}

			if (bFindF || bFindG)
			{
				GetCRsStep(vCRs[CH_F], crF2, ss1, ss2);
				GetCRsStep(vCRs[CH_G], crG2, ss1, ss2);

				if (ss2 - ss1 >= 4)
				{
					for (int i = crD.size() - 1; i >= 0; --i)
					{
						CR& tpD = vCRs[CH_D][crD[i]];
						if (ss1 <= tpD.Step2 + 5 && iLeftEStep - tpD.Step1 < 20)
						{
							continue;
						}
						if (bFindF || bFindG)
						{
							if (hasNeiberScrew && lastHPScrew.tempD.Step2 > 0 && lastHPScrew.tempE.Step1 > 0 && iLeftEStep - tpD.Step1 < lastHPScrew.tempE.Step2 - lastHPScrew.tempD.Step1)
							{
								continue;
							}
							if (ss1 > vCRs[CH_D][crD[i]].Step2 + 5)
							{
								crD.erase(crD.begin() + i);
							}
						}
						else
						{
							crD.erase(crD.begin() + i);
						}
					}
				}
			}
		}
	}
	bFindD = crD.size() > 0;
	bFindE = crE.size() > 0;
	bFindF = crF2.size() > 0;
	bFindG = crG2.size() > 0;
	/***********************************************************/	

	SetGuideHoleFlag(vCRs[CH_F], crF, 1);
	SetGuideHoleFlag(vCRs[CH_G], crG, 1);
	SetGuideHoleFlag(vCRs[CH_F], crF2, 1);
	SetGuideHoleFlag(vCRs[CH_G], crG2, 1);

	#pragma region 分析导孔水平裂纹
	int iFStep_1 = 0, iFStep_2 = 0, irowF1 = VALID_ROW - 1, iRowF2 = 0;
	int iFStep2 = 0;
	int iLeft = 0, iRight = 0x7FFFFFFF, iRow2 = cr.Row2;
	iRow2 = 0;
	int idxF = -1, rowF = 100;
	int idxG = -1, rowG = 100;
	if (bFindF || bFindG)
	{
		CR tempF;
		if (bFindF)
		{
			idxF = 0;
			rowF = vCRs[CH_F][crF2[0]].Row1;
			for (int k = 1; k < crF2.size(); ++k)
			{
				if (vCRs[CH_F][crF2[k]].Row1 < rowF)
				{
					rowF = vCRs[CH_F][crF2[k]].Row1;
					idxF = k;
				}
			}
		}
		if (bFindG)
		{
			idxG = 0;
			rowG = vCRs[CH_G][crG2[0]].Row1;
			for (int k = 1; k < crG2.size(); ++k)
			{
				if (vCRs[CH_G][crG2[k]].Row1 < rowG)
				{
					rowG = vCRs[CH_G][crG2[k]].Row1;
					idxG = k;
				}
			}
		}

		tempF = rowF < rowG ? vCRs[CH_F][crF2[idxF]] : vCRs[CH_G][crG2[idxG]];
		for (int i = 0; i < crF2.size(); ++i)
		{
			if (vCRs[CH_F][crF2[i]].Row1 >= tempF.Row1 - 1 && vCRs[CH_F][crF2[i]].Row2 <= tempF.Row2 + 1)
			{
				TryCombineFGInHole(tempF, vCRs[CH_F][crF2[i]], 15);
			}
		}
		for (int i = 0; i < crG2.size(); ++i)
		{
			if (vCRs[CH_G][crG2[i]].Row1 >= tempF.Row1 - 1 && vCRs[CH_G][crG2[i]].Row2 <= tempF.Row2 + 1)
			{
				TryCombineFGInHole(tempF, vCRs[CH_G][crG2[i]], 15);
			}
		}
		FillCR(tempF);
		irowF1 = tempF.Row1;
		iRowF2 = tempF.Row2;
		iFStep_1 = tempF.Step1;
		iFStep_2 = tempF.Step2;
		iRight = iFStep_1;
		iLeft = iFStep_2;
		iRow2 = tempF.Row2;
		if (tempF.Step2 - tempF.Step1 >= 22 && tempF.Step2 - tempF.Step1 <= 35 && crD.size() >= 2 && crE.size() >= 2)//导孔套孔
		{
			HolePara hp;
			GetNearestHole(tempF.Step1, PM_GUIDEHOLE, hp);
			GetCR(CH_D, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, iDesiredFRow - 3, blocks, vCRs[CH_D], crD, idxD, 1, true);
			GetCR(CH_E, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, iDesiredFRow - 3, blocks, vCRs[CH_E], crE, idxE, 1, true);
			GetCR(CH_F, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, iDesiredFRow - 3, blocks, vCRs[CH_F], crF2, -1, 1, true);
			GetCR(CH_G, tempF.Step1 - 5, tempF.Row1 - 2, tempF.Step2 + 5, iDesiredFRow - 3, blocks, vCRs[CH_G], crG2, -1, 1, true);

			Wound_Judged w;
			w.IsGuideHole = 1;
			FillWound(w, blockHead, head);
			w.Block = tempF.Block;
			w.Step = tempF.Step;
			w.Step2 = tempF.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Degree = WD_SERIOUS;
			sprintf(w.Result, "导孔套孔");
			w.IsGuideHole = 1;
			w.Type = W_DOUBLE_HOLE;
			w.Place = WP_WAIST;
			w.SizeX = 1;
			w.SizeY = 1;
			AddWoundData(w, vCRs[CH_D], crD);
			AddWoundData(w, vCRs[CH_E], crE);
			AddWoundData(w, vCRs[CH_F], crF);
			AddWoundData(w, vCRs[CH_G], crG);
			SetScrewHoleFlag(vCRs[CH_D], crD, 1);
			SetScrewHoleFlag(vCRs[CH_E], crE, 1);
			SetScrewHoleFlag(vCRs[CH_F], crF, 1);
			SetScrewHoleFlag(vCRs[CH_G], crG, 1);
			SetUsedFlag(vCRs[CH_D], crD, 1);
			SetUsedFlag(vCRs[CH_E], crE, 1);
			SetUsedFlag(vCRs[CH_F], crF, 1);
			SetUsedFlag(vCRs[CH_G], crG, 1);
			FillWound2(w, blocks);
			AddToWounds(vWounds, w);

			return true;
		}
	}
	else
	{
		if (bFront)
		{
			iRight = cr.Step1;			iLeft = iRight + 10;
		}
		else
		{
			iLeft = cr.Step2;			iRight = iLeft - 10;
		}
	}

	VINT crHt[2];
	GetCR(CH_F, iRight - 3, iRow2 + 2, iLeft + 3, iRow2 + 6, blocks, vCRs[CH_F], crHt[0]);
	GetCR(CH_G, iRight - 3, iRow2 + 2, iLeft + 3, iRow2 + 6, blocks, vCRs[CH_G], crHt[1]);
	SetGuideHoleFlag(vCRs[CH_F], crHt[0], 1);
	SetGuideHoleFlag(vCRs[CH_G], crHt[1], 1);

	for (int i = 0; i < 2; ++i)
	{
		for (int k = 0; k < crHt[i].size(); ++k)
		{
			CR& cF = vCRs[CH_F + i][crHt[i][k]];
			if (cF.IsLose == 1)
			{
				continue;
			}
			if (cF.Step1 >= iRight && cF.Step2 <= iLeft || cF.Region.size() < 2 || cF.Row1 <= iRowF2 + 2)
			{
				continue;
			}

			Wound_Judged w;
			w.IsGuideHole = 1;
			w.Block = cF.Block;
			w.Step = cF.Step;
			w.Step2 = cF.Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Place = WP_WAIST;
			FillWound(w, blockHead, head);
			w.Type = W_GUIDE_HORIZON_CRACK_RIGHT;
			if (cF.Step1 > tempFG.Step1)
			{
				w.Type = W_GUIDE_HORIZON_CRACK_LEFT;
			}
			w.Degree = WD_SERIOUS;
			memcpy(w.Result, "导孔下方水平裂纹", 30);
			GetCRInfo(cF, DataA, blocks, 0);
			AddWoundData(w, cF);
			w.SizeX = head.step * (cF.Step2 - cF.Step1);
			w.SizeY = 1;

			VINT crDt;
			GetCR(CH_D, cF.Step1, cF.Row1, cF.Step2, cF.Row2, blocks, vCRs[CH_D], crDt, -1, 1, false);
			for (int k = 0; k < crDt.size(); ++k)
			{
				CR cr1, cr2;
				bool bDouble = IsDoubleCR(vCRs[CH_D][crDt[k]], blocks, CH_D, cr1, cr2);
				if (bDouble)
				{
					AddWoundData(w, cr2);
				}
				else
				{
					AddWoundData(w, vCRs[CH_D][crDt[k]]);
				}
			}
			SetUsedFlag(vCRs[CH_D], crDt, 1);

			VINT crEt;
			GetCR(CH_E, cF.Step1, cF.Row1, cF.Step2, cF.Row2, blocks, vCRs[CH_E], crEt, -1, 1, false);
			for (int k = 0; k < crEt.size(); ++k)
			{
				CR cr1, cr2;
				bool bDouble = IsDoubleCR(vCRs[CH_E][crEt[k]], blocks, CH_E, cr1, cr2);
				if (bDouble)
				{
					AddWoundData(w, cr2);
				}
				else
				{
					AddWoundData(w, vCRs[CH_E][crEt[k]]);
				}
			}
			SetUsedFlag(vCRs[CH_E], crEt, 1);
			if (crDt.size() > 0 && crEt.size() > 0)
			{
				w.Type = W_DOUBLE_HOLE;
				AddToWounds(vWounds, w);
			}
			else
			{
				AddToWounds(vWounds, w);
			}
			break;
		}
	}
	SetUsedFlag(vCRs[CH_F], crHt[0], 1);
	SetUsedFlag(vCRs[CH_G], crHt[1], 1);
	SetGuideHoleFlag(vCRs[CH_F], crHt[0], 1);
	SetGuideHoleFlag(vCRs[CH_G], crHt[1], 1);

	crHt[0].clear();
	crHt[1].clear();
	GetCR(CH_F, iRight - 3, iRow2 + 7, iLeft + 3, iDesiredFRow + 3, blocks, vCRs[CH_F], crHt[0]);
	GetCR(CH_G, iRight - 3, iRow2 + 7, iLeft + 3, iDesiredFRow + 3, blocks, vCRs[CH_G], crHt[1]);
	for (int i = 0; i < 2; ++i)
	{
		for (int k = 0; k < crHt[i].size(); ++k)
		{
			CR& cF = vCRs[CH_F + i][crHt[i][k]];
			if (cF.Step1 >= iRight - 1 && cF.Step2 <= iLeft + 1 || cF.Region.size() < 2)
			{
				SetUsedFlag(cF, 1);
				SetGuideHoleFlag(cF, 1);
			}
		}
	}
	#pragma endregion

	VINT crC, crc;
	int iexceptC = -1, iexceptc = -1;
	if (cr.Channel == CH_C)
	{
		iexceptC = i;
	}
	else if (cr.Channel == CH_c)
	{
		iexceptc = i;
	}
	uint8_t bFindC = GetCR(CH_C, s1, s2, blocks, vCRs[CH_C], crC, iexceptC);
	uint8_t bFindc = GetCR(CH_c, s1, s2, blocks, vCRs[CH_c], crc, iexceptc);
	bool carType = blocks[cr.Block - g_iBeginBlock].BlockHead.detectSet.Identify & C_LR;
	VPM vTempPMs;
	double wdTemp = GetWD(blockHead.walk);
	for (int i = 0; i < crC.size(); ++i)
	{
		GetCRInfo(vCRs[CH_C][crC[i]], DataA, blocks);
		ParseHS(DataA, blocks, vCRs, vCRs[CH_C][crC[i]], vCRs[CH_C][crC[i]].Info, railType, g_direction, carType, iDesiredFRow, wdTemp, vWounds, vTempPMs, 0, 0, 0, 1);
	}

	for (int i = 0; i < crc.size(); ++i)
	{
		GetCRInfo(vCRs[CH_c][crc[i]], DataA, blocks);
		ParseHS(DataA, blocks, vCRs, vCRs[CH_c][crc[i]], vCRs[CH_c][crc[i]].Info, railType, g_direction, carType, iDesiredFRow, wdTemp, vWounds, vTempPMs, 0, 0, 0, 1);
	}

	if (cr.Channel <= CH_D)
	{
		int idxRowD1 = 0, idxStepD = 0;
		if (cr.Channel <= CH_D && crD.size() > 0)
		{
			idxD = crD[0];
			idxRowD1 = vCRs[CH_D][idxD].Row1;
			idxStepD = vCRs[CH_D][idxD].Step1;
		}

		for (int k = 0; k < crD.size(); ++k)
		{
			if (vCRs[CH_D][crD[k]].Row1 < ijawRow + 5)
			{
				continue;
			}
			else if (vCRs[CH_D][crD[k]].Step1 < idxStepD)
			{
				idxRowD1 = vCRs[CH_D][crD[k]].Row1;
				idxStepD = vCRs[CH_D][crD[k]].Step1;
				idxD = crD[k];
				break;
			}
		}

		if (idxRowD1 >= ijawRow + 5)
		{
			int iMaxSize = vCRs[CH_D][idxD].Region.size();
			for (int k = 0; k < crD.size(); ++k)
			{
				CR& tpD = vCRs[CH_D][crD[k]];
				if (tpD.Region.size() < 3 || tpD.Row1 <= g_iJawRow[railType] || crD[k] == idxD)
				{
					continue;
				}
				if (tpD.Row1 < idxRowD1 + 1 && tpD.Row1 <= row2 + 1 && tpD.Region.size() >= iMaxSize && tpD.Step1 < idxStepD)
				{
					idxD = crD[k];
					idxRowD1 = tpD.Row1;
					idxStepD = tpD.Step1;
					iMaxSize = tpD.Region.size();
				}
				else if (tpD.Row1 <= idxRowD1 + 1 && tpD.Step1 <= idxStepD && tpD.Row1 <= row2 + 1 && tpD.Region.size() >= iMaxSize)
				{
					idxD = crD[k];
					idxRowD1 = tpD.Row1;
					idxStepD = tpD.Step1;
					iMaxSize = tpD.Region.size();
				}
			}

			if (crD.size() >= 2 && idxD >= 0)
			{
				for (int i = 0; i < crD.size(); ++i)
				{
					if (i == idxD)
					{
						continue;
					}
					if (vCRs[CH_D][crD[i]].Step2 <= vCRs[CH_D][idxD].Step1 && vCRs[CH_D][crD[i]].Row1 >= vCRs[CH_D][idxD].Row2 && vCRs[CH_D][crD[i]].Row1 - vCRs[CH_D][idxD].Row2 <= 3 && vCRs[CH_D][idxD].Step2 - vCRs[CH_D][crD[i]].Step1 <= 5
						||
						vCRs[CH_D][idxD].Step2 <= vCRs[CH_D][crD[i]].Step1 && vCRs[CH_D][idxD].Row2 <= vCRs[CH_D][crD[i]].Row1 && vCRs[CH_D][idxD].Row1 - vCRs[CH_D][crD[i]].Row2 <= 3 && vCRs[CH_D][crD[i]].Step1 - vCRs[CH_D][idxD].Step2 <= 5
						)
					{
						Combine(vCRs[CH_D][idxD], vCRs[CH_D][crD[i]]);
						SetUsedFlag(vCRs[CH_D][crD[i]], 1);
						SetGuideHoleFlag(vCRs[CH_D][crD[i]], 1);
					}
				}
			}

			tempD = vCRs[CH_D][idxD];
			GetCRInfo(cr, DataA, blocks, false);
			if (tempD.Step1 >= istepMiddle && istepMiddle != 0)
			{
				Wound_Judged w;
				w.IsGuideHole = 1;
				FillWound(w, blockHead, head);
				w.Block = tempD.Block;
				w.Step = tempD.Step;
				w.Step2 = tempD.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Type = W_GUIDE_CRACK2;
				w.Place = WP_WAIST;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "导孔二象限斜裂纹", 30);
				w.SizeX = head.step * (tempD.Step2 - tempD.Step1) / 0.8;
				w.SizeY = 1;
				AddWoundData(w, tempD);
				AddToWounds(vWounds, w);
			}
			if (tempD.Row2 - tempD.Row1 >= 5)
			{
				CR cr1, cr2;
				if (IsDoubleCR(tempD, blocks, CH_d, cr1, cr2))
				{
					Wound_Judged w;
					w.IsGuideHole = 1;
					FillWound(w, blockHead, head);
					w.Block = cr2.Block;
					w.Step = cr2.Step;
					w.Step2 = cr2.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Type = W_GUIDE_CRACK4;
					w.Place = WP_WAIST;
					w.Degree = WD_SERIOUS;
					memcpy(w.Result, "导孔四象限斜裂纹", 30);
					w.SizeX = head.step * (cr2.Step2 - cr2.Step1) / 0.8;
					w.SizeY = 1;
					AddWoundData(w, cr2);
					AddToWounds(vWounds, w);
				}
			}

			SetGuideHoleFlag(tempD, 1);
			SetUsedFlag(vCRs[CH_D][tempD.Index], 1);
		}

		if (!bFindF && !bFindG)
		{
			row1 = tempD.Row1;
			row2 = tempD.Row2;
		}
	}


	//分析crE
	int idxRowE1 = 100, idxStepE2 = 0x7FFFFFFF;
	for (int k = 0; k < crE.size(); ++k)
	{
		if (vCRs[CH_E][crE[k]].Row1 < ijawRow + 5)
		{
			continue;
		}
		else
		{
			idxRowE1 = vCRs[CH_E][crE[k]].Row1;
			idxStepE2 = vCRs[CH_E][crE[k]].Step2;
			idxE = crE[k];
			break;
		}
	}

	if (idxE >= 0 && idxRowE1 >= ijawRow + 5)
	{
		if (tempFG.Step2 - tempFG.Step1 > 5)
		{
			int dist = GetDistance(tempFG, vCRs[CH_E][idxE]);
			for (int k = 0; k < crE.size(); ++k)
			{
				if (vCRs[CH_E][crE[k]].Region.size() < 3 || crE[k] == idxE)
				{
					continue;
				}
				int d = GetDistance(tempFG, vCRs[CH_E][crE[k]]);
				if (vCRs[CH_E][crE[k]].Row1 < idxRowE1 + 1 && vCRs[CH_E][crE[k]].Row1 <= row2 + 1 && vCRs[CH_E][crE[k]].Row1 >= ijawRow + 5 && vCRs[CH_E][crE[k]].Step2 >= idxStepE2 - 2 && d <= dist)
				{
					idxE = crE[k];
					idxRowE1 = vCRs[CH_E][crE[k]].Row1;
					idxStepE2 = vCRs[CH_E][crE[k]].Step2;
					dist = d;
				}
				else if (vCRs[CH_E][crE[k]].Row1 <= idxRowE1 + 1 && vCRs[CH_E][crE[k]].Step2 > idxStepE2 && vCRs[CH_E][crE[k]].Row1 <= row2 + 1 && vCRs[CH_E][crE[k]].Row1 >= g_iJawRow[railType] + 5 && d <= dist)
				{
					idxE = crE[k];
					idxRowE1 = vCRs[CH_E][crE[k]].Row1;
					idxStepE2 = vCRs[CH_E][crE[k]].Step2;
					dist = d;
				}
				else if (dist == 0 && vCRs[CH_E][crE[k]].Step2 >= idxStepE2)
				{
					idxE = crE[k];
					idxRowE1 = vCRs[CH_E][crE[k]].Row1;
					idxStepE2 = vCRs[CH_E][crE[k]].Step2;
					dist = d;
				}
			}
		}
		else
		{
			for (int k = 0; k < crE.size(); ++k)
			{
				if (vCRs[CH_E][crE[k]].Region.size() < 3 || crE[k] == idxE)
				{
					continue;
				}
				if (vCRs[CH_E][crE[k]].Row1 < idxRowE1 + 1 && vCRs[CH_E][crE[k]].Row1 <= row2 + 1 && vCRs[CH_E][crE[k]].Row1 >= ijawRow + 5 && vCRs[CH_E][crE[k]].Step2 >= idxStepE2 - 2)
				{
					idxE = crE[k];
					idxRowE1 = vCRs[CH_E][crE[k]].Row1;
					idxStepE2 = vCRs[CH_E][crE[k]].Step2;
				}
				else if (vCRs[CH_E][crE[k]].Row1 <= idxRowE1 + 1 && vCRs[CH_E][crE[k]].Step2 > idxStepE2 && vCRs[CH_E][crE[k]].Row1 <= row2 + 1 && vCRs[CH_E][crE[k]].Row1 >= g_iJawRow[railType] + 5)
				{
					idxE = crE[k];
					idxRowE1 = vCRs[CH_E][crE[k]].Row1;
					idxStepE2 = vCRs[CH_E][crE[k]].Step2;
				}
			}
		}

		if (crE.size() >= 2 && idxE >= 0)
		{
			for (int i = 0; i < crE.size(); ++i)
			{
				if (i == idxE)
				{
					continue;
				}
				if (vCRs[CH_E][crE[i]].Step1 >= vCRs[CH_E][idxE].Step2 && vCRs[CH_E][crE[i]].Row1 >= vCRs[CH_E][idxE].Row2 && vCRs[CH_E][crE[i]].Row1 - vCRs[CH_E][idxE].Row2 <= 3 && vCRs[CH_E][crE[i]].Step1 - vCRs[CH_E][idxE].Step2 <= 5
					||
					vCRs[CH_E][crE[i]].Step2 <= vCRs[CH_E][idxE].Step1 && vCRs[CH_E][crE[i]].Row2 <= vCRs[CH_E][idxE].Row1 && vCRs[CH_E][idxE].Row1 - vCRs[CH_E][crE[i]].Row2 <= 3 && vCRs[CH_E][idxE].Step1 - vCRs[CH_E][crE[i]].Step2 <= 5
					)
				{
					Combine(vCRs[CH_E][idxE], vCRs[CH_E][crE[i]]);
					SetUsedFlag(vCRs[CH_E][crE[i]], 1);
					SetGuideHoleFlag(vCRs[CH_E][crE[i]], 1);
				}
			}
		}

		for (int i = crE.size() - 1; i >= 0; --i)
		{
			if (crE[i] == idxE)
			{
				continue;
			}

			int r1, r2;
			int ov = GetOverlappedStep(vCRs[CH_E][crE[i]].Row1, vCRs[CH_E][crE[i]].Row2, vCRs[CH_E][idxE].Row1, vCRs[CH_E][idxE].Row2, r1, r2);
			if (ov > 0)
			{
				int s1 = -1, s2 = -1;
				for (int k = 0; k < vCRs[CH_E][crE[i]].Region.size(); ++k)
				{
					if (vCRs[CH_E][crE[i]].Region[k].row == r1)
					{
						s1 = vCRs[CH_E][crE[i]].Region[k].step;
						break;
					}
				}

				for (int k = 0; k < vCRs[CH_E][idxE].Region.size(); ++k)
				{
					if (vCRs[CH_E][idxE].Region[k].row == r1)
					{
						s2 = vCRs[CH_E][idxE].Region[k].step;
						break;
					}
				}

				if (s1 > s2)
				{
					crE.erase(crE.begin() + i);
				}
			}
		}

		if (idxE >= 0)
		{
			tempE = vCRs[CH_E][idxE];
			SetGuideHoleFlag(vCRs[CH_E][idxE], 1);
			GetCRASteps(CH_E, tempE, stepE1, stepE2, blocks, angleE, offsetE, head.step);
			GetCRInfo(vCRs[CH_E][idxE], DataA, blocks);
			if (tempE.Step1 + tempE.Step2 < 2 * istepMiddle - 2 && (tempD.Step2 > 0 && tempE.Step1 < tempD.Step2 || tempD.Step2 == 0))
			{
				Wound_Judged w;
				w.IsGuideHole = 1;
				w.Block = vCRs[CH_E][idxE].Block;
				w.Step = vCRs[CH_E][idxE].Step;
				w.Step2 = vCRs[CH_E][idxE].Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Place = WP_WAIST;
				FillWound(w, blockHead, head);
				w.Type = W_GUIDE_CRACK1;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "导孔一象限斜裂纹", 30);
				AddWoundData(w, tempE);
				SetUsedFlag(tempE, 1);
				w.SizeX = head.step * (vCRs[CH_E][idxE].Step2 - vCRs[CH_E][idxE].Step1) / 0.8;
				w.SizeY = 1;
				AddToWounds(vWounds, w);
			}
			if (tempE.Row2 - tempE.Row1 >= 5)
			{
				CR cr1, cr2;
				if (IsDoubleCR(tempE, blocks, CH_E, cr1, cr2) && cr2.Step2 - cr2.Step1 >= 3)
				{
					Wound_Judged w;
					w.IsGuideHole = 1;
					FillWound(w, blockHead, head);
					w.Block = cr2.Block;
					w.Step = cr2.Step;
					w.Step2 = cr2.Step1;
					w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
					w.Type = W_GUIDE_HORIZON_CRACK_LEFT;
					w.Place = WP_WAIST;
					w.Degree = WD_SERIOUS;
					memcpy(w.Result, "导孔左侧水平裂纹", 30);
					w.SizeX = head.step * (cr2.Step2 - cr2.Step1) / 0.8;
					w.SizeY = 1;
					AddWoundData(w, cr2);
					AddToWounds(vWounds, w);
				}
			}
		}

		for (int k = 0; k < crE.size(); ++k)
		{
			if (crE[k] == idxE)
			{
				continue;
			}
			CR& tpE = vCRs[CH_E][crE[k]];
			SetGuideHoleFlag(tpE, 1);
			if (idxE >= 0 && IsGuideHoleSecondWave(tempE, tpE, blocks, angleE, offsetE, g_filehead.step))
			{
				SetUsedFlag(tpE, 1);
				continue;
			}
			if (tpE.Step1 > tempE.Step2 && idxE >= 0)
			{
				continue;
			}

			double dh = GetDistanceHA(tempE, tpE);
			double dv = GetDistanceVA(tempE, tpE);
			int overlappedH = GetOverlappedCountH(tempE, tpE);
			int overlappedV = GetOverlappedCountV(tempE, tpE);
			if (dh <= 4 && dv <= 10 && overlappedH >= 2)
			{
				SetUsedFlag(tpE, 1);
				Wound_Judged w;
				w.IsGuideHole = 1;
				w.Block = tpE.Block;
				w.Step = tpE.Step;
				w.Step2 = tpE.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Place = WP_WAIST;
				FillWound(w, blockHead, head);
				w.Type = W_GUIDE_CRACK3;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "导孔三象限斜裂纹", 30);
				AddWoundData(w, tpE);
				w.SizeX = head.step * (vCRs[CH_E][crE[k]].Step2 - vCRs[CH_E][crE[k]].Step1) / 0.8;
				w.SizeY = 1;
				AddToWounds(vWounds, w);
			}
			else if (dv <= 3 && tpE.Step2 > tempE.Step2 && tpE.Row2 > tempE.Row2)
			{
				if (tpE.Row1 < tpE.Row2)
				{
					SetUsedFlag(tpE, 1);
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
					AddToWounds(vWounds, w);
				}
			}
			else if (dv <= 4 && dh >= 2 && dh <= 15)
			{
				SetUsedFlag(tpE, 1);
				if (dh < 3 || crD.size() < 2)
				{
					continue;
				}

				Wound_Judged w;
				w.IsGuideHole = 1;
				w.Block = tpE.Block;
				w.Step = tpE.Step;
				w.Step2 = tpE.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Place = WP_WAIST;
				FillWound(w, blockHead, head);
				w.Type = W_GUIDE_CRACK1;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "导孔套孔", 30);
				AddWoundData(w, tpE);
				w.SizeX = head.step * (tpE.Step2 - tpE.Step1) / 0.8;
				w.SizeY = 1;
				AddToWounds(vWounds, w);
				break;
			}
		}
		SetUsedFlag(vCRs[CH_E][tempE.Index], 1);
	}

	GetCRASteps(cr.Channel, tempD, stepD1, stepD2, blocks, angleD, offsetD, head.step);
	VINT crDt;
	uint8_t iFindD_4 = GetCR(CH_D, tempD.Step1, tempD.Row1, tempD.Step2, tempD.Row2 + 10, blocks, vCRs[CH_D], crDt, idxD, 1, true);
	RemoveHoleCR(vCRs[CH_D], crDt);
	if (iFindD_4)//D通道检测到伤损
	{
		for (int k = 0; k < crDt.size(); ++k)
		{
			CR& tpD = vCRs[CH_D][crDt[k]];
			SetGuideHoleFlag(tpD, 1);
			if (tpD.Region.size() < 3)
			{
				continue;
			}

			if (idxD >= 0 && IsGuideHoleSecondWave(tempD, tpD, blocks, angleD, offsetD, g_filehead.step))
			{
				SetUsedFlag(tpD, 1);
				continue;
			}

			double dh = GetDistanceHA(tempD, tpD);
			double dv = GetDistanceVA(tempD, tpD);
			if (GetCRInfo(tpD, DataA, blocks) && tpD.Info.MaxV >= g_iAmpl && dv >= 2 && tpD.Row1 >= tempD.Row2)
			{
				Wound_Judged w;
				w.IsGuideHole = 1;
				FillWound(w, blockHead, head);
				w.Block = tpD.Block;
				w.Step = tpD.Step;
				w.Step2 = tpD.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Type = W_GUIDE_CRACK4;
				w.Place = WP_WAIST;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "导孔四象限斜裂纹", 30);
				w.SizeX = head.step * (tpD.Step2 - tpD.Step1) / 0.8;
				w.SizeY = 1;
				SetUsedFlag(tpD, 1);
				AddWoundData(w, tpD);
				AddToWounds(vWounds, w);
				break;
			}
		}
	}

	crDt.clear();
	int dEnd = istepMiddle > 0 ? 2 * istepMiddle - cr.Step1 : cr.Step2 + 5;
	uint8_t iFindD_2 = GetCR(CH_D, tempD.Step2 + 1, tempD.Row1 - 2, dEnd, tempD.Row2 + 2, blocks, vCRs[CH_D], crDt, idxD, 1, true);
	//RemoveHoleCR(vCRs[CH_D], crDt);
	for (int k = crDt.size() - 1; k >= 0; --k)
	{
		CR& tpD = vCRs[CH_D][crDt[k]];
		if (s2 - s1 >= 5 && tpD.Step1 > s2)
		{
			crDt.erase(crDt.begin() + k);
		}
	}
	if (idxE > 0)
	{
		int s = vCRs[CH_E][idxE].Step2;
		for (int k = crDt.size() - 1; k >= 0; --k)
		{
			CR& tpD = vCRs[CH_D][crDt[k]];
			if (tpD.Step1 - s >= 2)
			{
				crDt.erase(crDt.begin() + k);
			}
		}
	}
	for (int k = 0; k < crDt.size(); ++k)
	{
		CR& tpD = vCRs[CH_D][crDt[k]];
		SetGuideHoleFlag(tpD, 1);
		if (tpD.Region.size() < 3)
		{
			continue;
		}

		if (idxD >= 0 && IsGuideHoleSecondWave(tempD, tpD, blocks, angleD, offsetD, g_filehead.step))
		{
			SetUsedFlag(tpD, 1);
			continue;
		}

		int firstRow = 0, lastrow = 0;
		GetCRRowInfo2(tpD, firstRow, lastrow, CH_D);
		if (firstRow < lastrow)
		{
			SetUsedFlag(tpD, 1);
			continue;
		}

		double dh = GetDistanceHA(tempD, tpD);
		double dv = GetDistanceVA(tempD, tpD);
		int overlappedH = GetOverlappedCountH(tempD, tpD);
		int overlappedV = GetOverlappedCountV(tempD, tpD);
		if (overlappedV >= 3)
		{
			if (dh >= 9)
			{
				Wound_Judged w;
				w.IsGuideHole = 1;
				w.Block = tpD.Block;
				w.Step = tpD.Step;
				w.Step2 = tpD.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Place = WP_WAIST;
				FillWound(w, blockHead, head);
				w.Type = W_SCREW_CRACK2;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "0孔二象限斜裂纹", 30);
				AddWoundData(w, tpD);
				SetUsedFlag(tpD, 1);
				w.SizeX = head.step * (tpD.Step2 - tpD.Step1) / 0.8;
				w.SizeY = 1;
				AddToWounds(vWounds, w);
				break;
			}
			else
			{
				Wound_Judged w;
				w.IsGuideHole = 1;
				w.Block = tpD.Block;
				w.Step = tpD.Step;
				w.Step2 = tpD.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Place = WP_WAIST;
				FillWound(w, blockHead, head);
				w.Type = W_GUIDE_CRACK2;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "导孔二象限斜裂纹", 30);
				AddWoundData(w, tpD);
				SetUsedFlag(tpD, 1);
				w.SizeX = head.step * (tpD.Step2 - tpD.Step1) / 0.8;
				w.SizeY = 1;
				AddToWounds(vWounds, w);
				break;
			}
		}
	}
	SetUsedFlag(vCRs[CH_D], crDt, 1);

	crDt.clear();
	GetCR(CH_D, tempD.Step1, tempD.Row2, tempD.Step2 + 30, iDesiredFRow, blocks, vCRs[CH_D], crDt, idxD, 1);
	for (int k = 0; k < crDt.size(); ++k)
	{
		if (IsGuideHoleSecondWave(tempD, vCRs[CH_D][crDt[k]], blocks, angleD, offsetD, g_filehead.step))
		{
			SetGuideHoleFlag(vCRs[CH_D][crDt[k]], 1);
			SetUsedFlag(vCRs[CH_D][crDt[k]], 1);
		}
	}


	VINT crEt;
	GetCR(CH_E, tempE.Step1 - 2, tempE.Row2, tempE.Step2 + 5, tempE.Row2 + 10, blocks, vCRs[CH_E], crEt, idxE, 1, true);
	RemoveHoleCR(vCRs[CH_E], crEt);
	for (int k = 0; k < crEt.size(); ++k)
	{
		CR& tpE = vCRs[CH_E][crEt[k]];
		SetGuideHoleFlag(tpE, 1);
		if (idxE >= 0 && IsGuideHoleSecondWave(tempE, tpE, blocks, angleE, offsetE, g_filehead.step))
		{
			SetUsedFlag(tpE, 1);
			continue;
		}

		double dh = GetDistanceHA(tempE, tpE);
		double dv = GetDistanceVA(tempE, tpE);
		int overlappedH = GetOverlappedCountH(tempE, tpE);
		int overlappedV = GetOverlappedCountV(tempE, tpE);
		if (dv >= 2 && overlappedH >= 3)
		{
			Wound_Judged w;
			w.IsGuideHole = 1;
			w.Block = vCRs[CH_E][crEt[k]].Block;
			w.Step = vCRs[CH_E][crEt[k]].Step;
			w.Step2 = vCRs[CH_E][crEt[k]].Step1;
			w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
			w.Place = WP_WAIST;
			FillWound(w, blockHead, head);
			w.Type = W_GUIDE_CRACK3;
			w.Degree = WD_SERIOUS;
			memcpy(w.Result, "导孔三象限斜裂纹", 30);
			AddWoundData(w, tpE);
			SetUsedFlag(tpE, 1);
			w.SizeX = head.step * (vCRs[CH_E][crEt[k]].Step2 - vCRs[CH_E][crEt[k]].Step1) / 0.8;
			w.SizeY = 1;
			AddToWounds(vWounds, w);
			break;
		}
	}
	SetUsedFlag(vCRs[CH_E], crEt, 1);


	crEt.clear();
	GetCR(CH_E, tempE.Step1 - 30, tempE.Row2, tempE.Step2, row2 + 10, blocks, vCRs[CH_E], crEt, idxE, 1, true);
	RemoveHoleCR(vCRs[CH_E], crEt);
	for (int k = crEt.size() - 1; k >= 0; --k)
	{
		if (IsGuideHoleSecondWave(tempE, vCRs[CH_E][crEt[k]], blocks, angleE, offsetE, g_filehead.step))
		{
			SetUsedFlag(vCRs[CH_E][crEt[k]], 1);
			SetGuideHoleFlag(vCRs[CH_E][crEt[k]], 1);
		}
		else
		{
			CR& tpE = vCRs[CH_E][crEt[k]];
			if (tpE.Region.size() < 3)
			{
				continue;
			}

			int oh = GetOverlappedCountH(tpE, tempE);
			int ov = GetOverlappedCountV(tpE, tempE);
			if (tempE.Region.size() > 0 && oh == 0 && ov == 0)
			{
				continue;
			}

			if (tpE.Row1 > row2 + 3)
			{
				Wound_Judged w;
				w.IsGuideHole = 1;
				w.Block = tpE.Block;
				w.Step = tpE.Step;
				w.Step2 = tpE.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Place = WP_WAIST;
				FillWound(w, blockHead, head);
				w.Type = W_GUIDE_CRACK3;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "导孔三象限斜裂纹", 30);
				AddWoundData(w, tpE);
				SetUsedFlag(tpE, 1);
				w.SizeX = head.step * (tpE.Step2 - tpE.Step1) / 0.8;
				w.SizeY = 1;
				AddToWounds(vWounds, w);
			}
			else
			{
				if (tpE.Step2 < tempD.Step1)
				{
					continue;
				}
				Wound_Judged w;
				w.IsGuideHole = 1;
				w.Block = tpE.Block;
				w.Step = tpE.Step;
				w.Step2 = tpE.Step1;
				w.Walk = GetWD(blocks[w.Block - g_iBeginBlock].BlockHead.walk, w.Step, head.step, g_direction);
				w.Place = WP_WAIST;
				FillWound(w, blockHead, head);
				w.Type = W_GUIDE_CRACK1;
				w.Degree = WD_SERIOUS;
				memcpy(w.Result, "导孔一象限斜裂纹", 30);
				AddWoundData(w, tpE);
				SetUsedFlag(tpE, 1);
				w.SizeX = head.step * (tpE.Step2 - tpE.Step1) / 0.8;
				w.SizeY = 1;
				AddToWounds(vWounds, w);
			}
		}
	}

	if (tempD.Region.size() > 0)
	{
		SetUsedFlag(vCRs[CH_D][tempD.Index], 1);
		SetGuideHoleFlag(vCRs[CH_D][tempD.Index], 1);
		crD.clear();
		GetCR(CH_D, s1 - 10, s2 + 30, blocks, vCRs[CH_D], crD, idxD);
		for (int k = 0; k < crD.size(); ++k)
		{
			if (IsGuideHoleSecondWave(tempD, vCRs[CH_D][crD[k]], blocks, angleD, offsetD, head.step))
			{
				SetGuideHoleFlag(vCRs[CH_D][crD[k]], 1);
				SetUsedFlag(vCRs[CH_D][crD[k]], 1);
			}
		}

		s1 = min(s1, tempD.Step1);
	}
	if (tempE.Region.size() > 0)
	{
		SetUsedFlag(vCRs[CH_E][tempE.Index], 1);
		SetGuideHoleFlag(vCRs[CH_E][tempE.Index], 1);
		crE.clear();
		GetCR(CH_E, s1 - 10, s2 + 30, blocks, vCRs[CH_E], crE, idxE);
		for (int k = 0; k < crE.size(); ++k)
		{
			if (IsGuideHoleSecondWave(tempE, vCRs[CH_E][crE[k]], blocks, angleE, offsetE, head.step))
			{
				SetGuideHoleFlag(vCRs[CH_E][crE[k]], 1);
				SetUsedFlag(vCRs[CH_E][crE[k]], 1);
			}
		}

		s2 = max(s2, tempE.Step2);
	}

	for (int i = crD.size() - 1; i >= 0; --i)
	{
		if (vCRs[CH_D][crD[i]].Step1 > s2 || vCRs[CH_D][crD[i]].Step2 < s1)
		{
			crD.erase(crD.begin() + i);
		}
	}

	for (int i = crE.size() - 1; i >= 0; --i)
	{
		if (vCRs[CH_E][crE[i]].Step1 > s2 || vCRs[CH_E][crE[i]].Step2 < s1)
		{
			crE.erase(crE.begin() + i);
		}
	}

	for (int i = crF.size() - 1; i >= 0; --i)
	{
		if (vCRs[CH_F][crF[i]].Step1 > s2 || vCRs[CH_F][crF[i]].Step2 < s1)
		{
			crF.erase(crF.begin() + i);
		}
	}

	for (int i = crG.size() - 1; i >= 0; --i)
	{
		if (vCRs[CH_G][crG[i]].Step1 > s2 || vCRs[CH_G][crG[i]].Step2 < s1)
		{
			crG.erase(crG.begin() + i);
		}
	}

	for (int i = crF2.size() - 1; i >= 0; --i)
	{
		if (vCRs[CH_F][crF2[i]].Step1 > s2 || vCRs[CH_F][crF2[i]].Step2 < s1)
		{
			crF2.erase(crF2.begin() + i);
		}
	}

	for (int i = crG2.size() - 1; i >= 0; --i)
	{
		if (vCRs[CH_G][crG2[i]].Step1 > s2 || vCRs[CH_G][crG2[i]].Step2 < s1)
		{
			crG2.erase(crG2.begin() + i);
		}
	}

	SetGuideHoleFlag(vCRs[CH_D], crD, 1);
	SetGuideHoleFlag(vCRs[CH_E], crE, 1);

	SetUsedFlag(vCRs[CH_D], crD, 1);
	SetUsedFlag(vCRs[CH_E], crE, 1);

	SetUsedFlag(vCRs[CH_F], crF, 1);
	SetUsedFlag(vCRs[CH_G], crG, 1);
	SetUsedFlag(vCRs[CH_F], crF2, 1);
	SetUsedFlag(vCRs[CH_G], crG2, 1);

	Pos tempPos = FindStepInBlock(s1, blocks, 0);
	HolePara hp;
	hp.Block = tempPos.Block;
	hp.Step = tempPos.Step;
	hp.step2 = s1;
	hp.mark = PM_GUIDEHOLE;
	hp.flag = 0;
	hp.isInFork = 0;
	hp.tempD = tempD;
	hp.tempE = tempE;
	//hp.tempF = tempF;
	hp.Row1 = row1;
	hp.Row2 = row2;
	hp.Height = hp.Row2 - hp.Row1;
	hp.FRow = iDesiredFRow;
	hp.HoleIndex = -10;
	hp.RailType = railType;
	hp.FLength = s2 - s1;
	g_vHoleParas[hp.step2] = hp;
	g_LastGuideHole = hp;

	PM pm;
	memset(&pm, 0, sizeof(pm));
	pm.Mark = PM_GUIDEHOLE;
	pm.Block = tempPos.Block;
	pm.Step = tempPos.Step;
	pm.Step2 = tempPos.Step2;
	pm.Length = s2 - s1;
	pm.Height = row2 - row1;
	AddToMarks(pm, vPMs);
	return true;
}
