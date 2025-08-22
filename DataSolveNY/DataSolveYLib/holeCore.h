#ifndef _HOLECORE_H
#define _HOLECORE_H

#include "DataEntities.h"

//cr2是否cr1的多次反射回波
uint8_t	IsScrewHoleSecondWave(CR& cr1, CR& cr2, VBDB& blocks, double angle, double offset, double step);

//cr2是否cr1的多次反射回波
uint8_t	IsGuideHoleSecondWave(CR& cr1, CR& cr2, VBDB& blocks, double angle, double offset, double step);

void	GetCRRelativeInfo(CR& cr1, CR& cr2, int& overlappedH, int& overlappedV, double& dh2, double& dv2);


bool	ParseScrewHoleCrackLeft(F_HEAD& head, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int16_t& iFDesiredRow, CR& crFG, int iExceptF, int iExceptG, int iScrewIndex, uint8_t bJoint, bool bAutoJoint, Wound_Judged& w, VPM& vPMs);

bool	ParseScrewHoleCrackRight(F_HEAD& head, BLOCK& blockHead, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int16_t& iFDesiredRow, CR& crFG, int iExceptF, int iExceptG, int iScrewIndex, uint8_t bJoint, bool bAutoJoint, Wound_Judged& w, VPM& vPMs);


void	ParseScrewHoleSkew1(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleE, double& offsetE,
	CR& tempE2,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP);



void	ParseScrewHoleSkew2(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleD, double& offsetD,
	CR& tempD2,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP);


void	ParseScrewHoleSkew3(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleE, double& offsetE,
	CR& tempE2,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP);



void	ParseScrewHoleSkew4(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleD, double& offsetD,
	CR& tempD2,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP);



void	ParseReverseWaves(F_HEAD& head, BlockData_A& DataA, BLOCK& blockHead, VBDB& blocks, VCR* vCRs, CR& tempD, CR& tempE, CR& tempF,
	double& angleD, double& offsetD,
	CR& tempD2,
	int& iLoseCh, VINT& vF,
	int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex, HolePara& lastHP);

#endif // _HOLECORE_H
