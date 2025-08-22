#ifndef _HOLE_H
#define _HOLE_H

#include "DataEntities.h"


bool	IsExistScrewHole(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint, bool bAutoJoint, uint8_t bSew, int iScrewIndex);

bool	ParseScrewHole(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint = 0, bool bAutoJoint = false, uint8_t bSew = 0, int iScrewIndex = 0);

//轨腰不出FG波，但有D或E，底部有失波
bool	ParseScrewHoleNoFG(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, int step1, int step2, int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs, uint8_t bJoint = 0, bool bAutoJoint = false, uint8_t bSew = 0, int iScrewIndex = 0);

bool	ParseGuideHole(F_HEAD& head, BlockData_A& DataA, VBDB& blocks, VCR* vCRs, CR&cr, int i, int16_t iFRow, uint8_t railType, VWJ& vWounds, VPM& vPMs);


#endif // _HOLE_H
