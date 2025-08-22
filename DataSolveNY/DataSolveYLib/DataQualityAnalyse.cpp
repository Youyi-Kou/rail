#include "stdafx.h"
#include "DataDefine.h"
#include "DataEntities.h"
#include "DataQualityAnalyse.h"
#include "PublicFunc.h"
#include "GlobalDefine.h"

TAlarmSta::TAlarmSta()
{
	Reset();
}

void TAlarmSta::Reset()
{
	AlarmConfirmInterval = 0;
	for (int i = 0; i < 1000; i++)
	{
		CheckAlarmInv[i] = 0;
		CheckAlarmNum[i] = 0;
	}

	memset(ADB, 0, sizeof(TAlarmData) * MAX_ALARMDATA_NUM);
	memset(FilterAlarmNum, 0, sizeof(uint32_t) * MAX_ALARMDATA_NUM);
	memset(SearchAlarmNum, 0, sizeof(uint32_t) * SEARCH_ALARMDATA_NUM);
	CheckAlarmCount = 0;
	FilterAlarmCount = 0;
	SearchAlarmCount = 0;

	AlarmBitMark[0][1] = A11_CHN_ALARM;
	AlarmBitMark[0][2] = A12_CHN_ALARM;
	// a
	AlarmBitMark[1][1] = A21_CHN_ALARM;
	AlarmBitMark[1][2] = A22_CHN_ALARM;
	// B
	AlarmBitMark[2][1] = B11_CHN_ALARM;
	AlarmBitMark[2][2] = B12_CHN_ALARM;
	// b
	AlarmBitMark[3][1] = B21_CHN_ALARM;
	AlarmBitMark[3][2] = B22_CHN_ALARM;
	// C
	AlarmBitMark[4][1] = C1_CHN_ALARM;
	// c
	AlarmBitMark[5][1] = C2_CHN_ALARM;
	// D
	AlarmBitMark[6][1] = D_CHN_ALARM;
	AlarmBitMark[6][3] = QD_CHN_ALARM;
	// E
	AlarmBitMark[7][1] = E_CHN_ALARM;
	AlarmBitMark[7][3] = QE_CHN_ALARM;
	// F
	AlarmBitMark[8][1] = F_CHN_ALARM;
	AlarmBitMark[8][3] = QF_CHN_ALARM;


	AlarmInterval[8][0] = 0;
	AlarmInterval[9][0] = 0;
	for (int i = 0; i < CH_N - 2; i++)               // CH_N-2��ʾ����Gͨ����deΪһ��ͨ��
	{
		for (int j = 0; j < 4; j++)
		{
			ChannelAlarmCount[i][j] = 0;
		}
	}
}

TDefectSta::TDefectSta()
{
	Reset();
}

void TDefectSta::Reset()
{
	CheckDefectCount = 0;
	for (int i = 0; i < 1000; ++i)
	{
		CheckDefectInv[i] = 0;
		CheckDefectNum[i] = 0;
	}

	// ��ʼ������ͳ��
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			TypeDefectCount[i][j] = 0;
			SourceTypeDefectCount[i][j] = 0;
		}
	}
	DefectDataCount = 0;
	SourceDefectDataCount = 0;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//���ƣ�CheckDefectDelete
//���ܣ�ɾ�������¼
//���룺num - ������Ϣ���
//�����
//���أ�
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckDefectDelete(TDefectSta *tds, UINT32 num)
{
	TDefectData *data1, *data2;
	for (UINT32 i = num + 1; i < tds->DefectDataCount; i++)
	{
		data1 = &tds->DDB[i - 1];
		data2 = &tds->DDB[i];
		data2->Number--;
		*data1 = *data2;
	}
	tds->DDB[tds->DefectDataCount - 1] = tds->DDB[tds->DefectDataCount];
	tds->DefectDataCount--;
	// ������ڼ�������
	if (tds->CheckDefectCount > 0)
	{
		int count = 0;
		bool Found = false;
		for (UINT16 i = 0; i < tds->CheckDefectCount - 1; i++)
		{
			if (tds->CheckDefectNum[i] > num)
			{
				tds->CheckDefectNum[i]--;
			}
			if (Found == false)
			{
				if (tds->CheckDefectNum[i] == num)
				{
					Found = true;
				}
			}
			else
			{
				tds->CheckDefectNum[i] = tds->CheckDefectNum[i + 1];
				tds->CheckDefectInv[i] = tds->CheckDefectInv[i + 1];
				count++;
			}
		}
		tds->CheckDefectNum[tds->CheckDefectCount - 1] = 0;
		tds->CheckDefectInv[tds->CheckDefectCount - 1] = 0;
		tds->CheckDefectCount--;
	}
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//���ƣ�CheckDefectMerge
//���ܣ��ϲ����������¼
//���룺num_m - ��Ҫ������Ϣ���
//      num_s - ��Ҫ������Ϣ���
//�����
//���أ�
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckDefectMerge(TDefectSta *tds, UINT32 num_m, UINT32 num_s)
{
	TDefectData *data1, *data2;

	data1 = &tds->DDB[num_m];
	data2 = &tds->DDB[num_s];

	if (data1->VerStart > data2->VerStart)
	{
		data1->VerStart = data2->VerStart;
	}
	if (data1->VerEnd < data2->VerEnd)
	{
		data1->VerEnd = data2->VerEnd;
	}
	if (data1->DefectType > data2->DefectType)
	{
		data1->DefectType = data2->DefectType;
		data1->DefectDegree = data2->DefectDegree;
	}
	else if (data1->DefectType == data2->DefectType)
	{
		if (data1->DefectDegree < data2->DefectDegree)
		{
			data1->DefectDegree = data2->DefectDegree;
		}
	}
	if (data1->DefectNum == 0 && data2->DefectNum != 0)
	{
		data1->DefectNum = data2->DefectNum;
	}
	if (data1->Dispose == 0 && data2->Dispose != 0)
	{
		data1->Dispose = data2->Dispose;
	}
	// ɾ����Ҫ��Ϣ
	CheckDefectDelete(tds, num_s);
}


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//���ƣ�CheckDefectUpdata
//���ܣ�����һ�������¼
//���룺tdd - ������Ϣָ��
//      StepInv - �������
//      VerPos - ��ȵ�
//      DefectType - ��������
//      DefectDegree - ����̶�
//      DefectNum - ������
//�����tdd - ������Ϣָ��
//���أ�
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckDefectUpdata(TDefectData *tdd, UINT16 StepInv, UINT16 VerPos, UINT8 DefectType, UINT8 DefectDegree, UINT16 DefectNum, UINT8 Dispose)
{
	tdd->StepCount += StepInv;
	if (tdd->VerStart > VerPos)
	{
		tdd->VerStart = VerPos;
	}
	if (tdd->VerEnd < VerPos)
	{
		tdd->VerEnd = VerPos;
	}
	if (tdd->DefectType > DefectType)
	{
		tdd->DefectType = DefectType;
		tdd->DefectDegree = DefectDegree;
	}
	else if (tdd->DefectType == DefectType)
	{
		if (tdd->DefectDegree < DefectDegree)
		{
			tdd->DefectDegree = DefectDegree;
		}
	}
	if (DefectNum != 0)
	{
		tdd->DefectNum = DefectNum;
	}
	if (tdd->Dispose == 0 && Dispose != 0)
	{
		tdd->Dispose = Dispose;
	}
}


int GetCurStepFromStepPos(VBDB& blocks, UINT32 MeterIndex, UINT16 StepIndex)
{
	if (MeterIndex < blocks.size())
	{
		if (StepIndex < blocks[MeterIndex].StepCount)
		{
			return blocks[MeterIndex].IndexL + StepIndex;
		}
	}
	return 0;
}


//---------------------------------------------------------------------------
// ���ݲ��������õ���Ӧ�Ĳ���λ�� *pMeter��*pStepΪ��ʼ����λ�ã��Ͳ��Һ󷵻�λ�ã�ForwardΪ���ҷ��� ���أ�true - �ɹ�
//---------------------------------------------------------------------------
bool GetStepPosFromCurStep(VBDB& blocks, INT32 CurStep, UINT32 *pMeter, UINT16 *pStep, bool Forward)
{
	UINT32 Start_M = *pMeter;
	UINT16 Start_S = *pStep;
	if (Forward == true)         // �������
	{
		for (int i = Start_M; i < blocks.size(); i++)
		{
			if (CurStep >= blocks[i].IndexL)
			{
				if (CurStep <= blocks[i].IndexL + blocks[i].StepCount - 1)
				{
					// �׿�����粽������
					int s;
					if (i == Start_M)
					{
						s = Start_S;
					}
					else
					{
						s = 0;
					}
					for (int j = s; j < blocks[i].StepCount; j++)
					{
						if (CurStep == blocks[i].IndexL + j)
						{
							*pMeter = i;
							*pStep = j;
							return true;
						}
					}
				}
			}
		}
		// δ�ҵ���������Ϊĩβ
		*pMeter = blocks.size() - 1;
		*pStep = blocks[*pMeter].StepCount;
	}
	else                        // �������
	{
		for (int i = Start_M; i >= 0; i--)
		{
			if (CurStep >= blocks[i].IndexL)
			{
				if (CurStep <= blocks[i].IndexL + blocks[i].StepCount - 1)
				{
					// �׿��������������
					int s;
					if (i == Start_M)
					{
						s = Start_S;
					}
					else
					{
						s = blocks[i].StepCount - 1;
					}
					for (int j = s; j >= 0; j--)
					{
						if (CurStep == blocks[i].IndexL + j)
						{
							*pMeter = i;
							*pStep = j;
							return true;
						}
					}
				}
			}
		}
		// δ�ҵ���������Ϊ��ͷ
		*pMeter = 0;
		*pStep = 0;
	}
	return false;
}

//---------------------------------------------------------------------------
// V2.0���ݰ汾�����һ�������¼�������б�
//---------------------------------------------------------------------------
void CheckDefectAdd(F_HEAD FileHead, BLOCK MeterHead, UINT32 MeterIndex, UINT16 StepIndex, UINT16 VerPos, UINT8 DefectType, UINT8 DefectDegree, UINT16 DefectNum, UINT8 Dispose, TDefectSta *tds)
{
	TRACK_P *tra = &FileHead.deviceP2.TrackSet;     // �ϵ�����
	int count = tds->DefectDataCount;                   // ��ǰ������

	tds->DDB[count].Number = tds->DefectDataCount;      // ���
	tds->DDB[count].Checked = false;
	tds->DDB[count].MeterIndex = MeterIndex;            // �׿�����
	tds->DDB[count].StepIndex = StepIndex;              // ��������
	tds->DDB[count].VerPos = VerPos;                    // ��ȵ�
	tds->DDB[count].DefectType = DefectType;            // ��������
	tds->DDB[count].DefectDegree = DefectDegree;        // ����̶�

	tds->DDB[count].StepCount = 1;                      // ���𲽽����
	tds->DDB[count].VerStart = VerPos;                  // �������ڵ����λ�����
	tds->DDB[count].VerEnd = VerPos;                    // �������ڵ����λ���յ�

	tds->DDB[count].SectionNum = tra->sectionNum;       // ��λ���
	tds->DDB[count].LineNum = tra->lineNum;       // �߱��
	tds->DDB[count].LineType = tra->lineType;       // �߱�
	tds->DDB[count].swNum = MeterHead.swNum;       // ������
	// ʵʱ���
	W_D wd = MeterHead.walk;
	bool walkWay;

	// ˳�����
	walkWay = (MeterHead.railType >> 4) & 0x1;
	// �б�
	tds->DDB[count].LineWay = (MeterHead.railType >> 6) & 0x3;
	// �ɱ�
	tds->DDB[count].LeftRight = (MeterHead.railType >> 5) & 0x1;
	if (walkWay)    // ˳���
	{
		wd.mm += StepIndex * HOR_POINT_FOR_LENTH;   // �������
		if (wd.mm >= 1000)
		{
			wd.mm -= 1000;
			wd.m++;
			if (wd.m >= 1000)
			{
				wd.m -= 1000;
				wd.Km++;
			}
		}
	}
	else                // �����
	{
		wd.mm -= StepIndex * HOR_POINT_FOR_LENTH;   // �������
		if (wd.mm < 0)
		{
			wd.mm += 1000;
			wd.m--;
			if (wd.m < 0)
			{
				wd.m += 1000;
				wd.Km--;
			}
		}
	}
	tds->DDB[count].Walk = wd;
	tds->DDB[count].TrackNum = MeterHead.plyNum;       // �ɵ����
	tds->DDB[count].StationNum = tra->stationNum;       // ��վ���
	// ��������
	tds->DDB[count].FindData = (FileHead.startD & 0xffffff00) | ((MeterHead.time & 0xff000000) >> 24);
	// ����ʱ��
	tds->DDB[count].FindTime = MeterHead.time & 0x00ffffff;
	tds->DDB[count].FindUser = MeterHead.user;          // ������
	tds->DDB[count].DefectNum = DefectNum;              // ������
	// �������
	for (int i = 0; i < 8; i++)
	{
		tds->DDB[count].DeviceNum[i] = FileHead.DeviceNum[i];
	}
	tds->DDB[count].RailNum = MeterHead.railNum;        // �ֹ���
	tds->DDB[count].RailType = MeterHead.railType & 0x3;        // ����
	tds->DDB[count].Dispose = Dispose;                          // �ֳ�����
	// �����������
	tds->DefectDataCount++;
}


TCheckbackSta CheckbackSta;      // ����ͳ��
//void GetAlarmDefectSignData(F_HEAD& fHead, VBDB &blocks, std::string strFileB, int iFileSize, TAlarmSta *tas, TDefectSta *tds, TSignSta *tss)
void GetAlarmDefectSignData(void* threadData)
{
	QualityData* pData = (QualityData*)threadData;
	F_HEAD fHead = *(pData->fileHead);
	VBDB* blocks = pData->vBlockHeads;
	std::string strFileB = pData->strTPB;
	int iFileSize = pData->szFileB;
	TAlarmSta *tas = pData->tas;
	TDefectSta *tds = pData->tds;
	TSignSta *tss = pData->tss;
	TCheckbackSta* tbs = pData->tbs;
	//tas->Reset();
	//tds->Reset();

	uint32_t MeterCount = blocks->size();            // �׿�����
	BLOCK MeterHead, MeterBak;                      // �׿�ͷ
	int SearchInv = 4;                              // �����������������
	tas->AlarmDataCount = 0;                        // ������������
	tds->DefectDataCount = 0;                       // �����������
	tss->SignCount = 0;                             // ��Ǽ�������

	// ��ʼ�����������������������ͱ������
	tas->CheckAlarmCount = 0;
	for (int i = 0; i < 1000; i++)
	{
		tas->CheckAlarmInv[i] = 0;
		tas->CheckAlarmNum[i] = 0;
	}
	tas->AlarmInterval[8][0] = 0;
	tas->AlarmInterval[9][0] = 0;
	// ��ʼ������ͳ��
	for (int i = 0; i < CH_N - 2; i++)               // CH_N-2��ʾ����Gͨ����deΪһ��ͨ��
	{
		for (int j = 0; j < 4; j++)
		{
			tas->ChannelAlarmCount[i][j] = 0;
		}
	}
	tas->AlarmDataCount = 0;
	// ��ʼ��ʧ�������������
	int LostNum_0 = 0,
		LostNum_37 = 0;
	// ��ʼ������������������������������
	tds->CheckDefectCount = 0;
	for (int i = 0; i < 100; i++)
	{
		tds->CheckDefectInv[i] = 0;
		tds->CheckDefectNum[i] = 0;
	}
	// ��ʼ������ͳ��
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			tds->TypeDefectCount[i][j] = 0;
			tds->SourceTypeDefectCount[i][j] = 0;
		}
	}
	tds->DefectDataCount = 0;
	tds->SourceDefectDataCount = 0;
	bool CheckingSign[SIGN_NUM];                         // ���ڼ�����Ǳ�־
	UINT16 CheckingSignNum[SIGN_NUM];                    // ���ڼ���������
	// ��ʼ���������ͱ����Ŀ����
	for (int i = 0; i < SIGN_NUM; i++)
	{
		tss->SignTypeCount[i] = 0;
		CheckingSign[i] = false;
	}


	int FallBackCurStep = -999999;                    // ���˶�Ӧ����
	// ��ʼ������ͳ��
	memset(tbs->Org_CD_List, 0, MAX_SIGNDATA_NUM * sizeof(TCheckbackData));
	memset(tbs->CD_List, 0, MAX_SIGNDATA_NUM * sizeof(TCheckbackData));
	tbs->DataCount = 0;
	tbs->OrgDataCount = 0;

	uint32_t BUFF_LENGHT = 900;           // Buff�Ĳ�������

	// �����׿��־��0 - ��Ϊ���ˣ�1 - Ϊ���ˣ�2 - ��һ�׿쿪ʼΪ���ˣ������л��˱�־��һ�׿鿪ʼ����ˣ�
	int32_t IsBackMeter = 0;

	// ���ڴ���Ļ����б����������˺ͱ����ˣ���Ϊ-1ʱ��ʾ�޴�������
	int64_t BackIndex = -1, BackIndex_Org = -1;

	// �����б��¼��Ӧ��ֹ��������
	int32_t Org_BackCurStep_S = -999999, Org_BackCurStep_E = -999999;

	//���׿�˳����������ļ�
	std::string sn = tss->SN[10];
	int tempNum;

	DQWavePoint wp;
	int szWavePoint = sizeof(DQWavePoint);

	uint8_t		isFork = 0, isBridge = 0, isCurve = 0, isTunnel = 0;
	uint16_t	lastswNum = 0;
	uint8_t		lastforkBits = 0;
	uint8_t		isForkError;//�Ƿ�����
	g_vStepsInBackArea.clear();
	PM pm;

	//���׿�˳����������ļ�
	for (int i = 0; i < MeterCount; i++)
	{
		// ��ѹ��ѹ����������
		BlockData_B block;
		BlockData_B blockTemp = (*blocks)[i];
		if (SolveTPB_C_Internal(strFileB, iFileSize, blockTemp, block, g_isPTData) != 1)
		{
			break;
		}
		MeterHead = block.BlockHead;
		int iFRow = MeterHead.railH / 3;
		(*blocks)[i].sumAaBbCc = 0;
		(*blocks)[i].sumDE = 0;
		(*blocks)[i].sumFG = 0;
		// ���β�ѯÿһ������
		for (int j = 0; j < MeterHead.row; j++)
		{
			B_Step& step = block.vBStepDatas[j];

			// �õ���ǰ������Mark��������Ϣ
			B_MARK LineMark = step.Mark;
			B_WOUND LineWound = step.Wound;


			B_WOUND AlgWound = step.Wound2;
			if (g_isPTData && AlgWound.W_Mark > 0)
			{
				g_mapWoundTypeCount[AlgWound.W_Code] = g_mapWoundTypeCount[AlgWound.W_Mark] + 1;
			}

			uint32_t mark = LineMark.Mark;

#pragma region �˹���ǵ�λ�ñ�
			if (g_vStepsInBackArea.find(step.Step) == g_vStepsInBackArea.end())
			{
				if (g_isPTData == false)
				{
					if ((mark & FORK) && isFork == 0) // ����Y
					{
						memset(&pm, 0, sizeof(pm));
						pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
						pm.Mark = PM_HJFORK_BEGIN;
						pm.Block = block.Index;
						pm.Step = j;
						pm.Step2 = step.Step;
						pm.Manual = 1;
						lastswNum = BCDToINT16(block.BlockHead.swNum);
						lastforkBits = block.BlockHead.BitS;
						/*
						if (i < blocks.size() - 1)
						{
							lastforkBits = blocks[i + 1].BlockHead.BitS;
							pm.Data = BCDToINT16(blocks[i + 1].BlockHead.swNum);
							lastswNum = pm.Data;
						}*/
						pm.Data = lastswNum;
						pm.ChannelNum = lastforkBits;
						isFork = 1;
						AddToMarks(pm, g_vPMs);
						AddToMarks(pm, g_vPMs2);

						fork.Begin = pm;
					}
					else if ((mark & FORK) == 0 && isFork == 1)
					{
						memset(&pm, 0, sizeof(pm));
						pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
						pm.Mark = PM_HJFORK_END;
						pm.Block = block.Index;
						pm.Step = j;
						pm.Step2 = block.vBStepDatas[j].Step;
						pm.Manual = 1;
						pm.Data = BCDToINT16(block.BlockHead.swNum);
						pm.ChannelNum = block.BlockHead.BitS;
						isFork = 0;
						AddToMarks(pm, g_vPMs);
						AddToMarks(pm, g_vPMs2);
						fork.ForkNo = pm.Data;
						fork.Bits = pm.ChannelNum;
						fork.End = pm;
						g_vForks.emplace_back(fork);
					}
					else if ((mark & FORK) && isFork == 1)
					{
						uint16_t swNum = BCDToINT16(block.BlockHead.swNum);
						if (lastswNum != swNum || lastforkBits != block.BlockHead.BitS)//ԭ���Ľ������µĿ�ʼ
						{
							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_HJFORK_END;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = block.vBStepDatas[j].Step;
							pm.Manual = 1;
							pm.Data = lastswNum;
							pm.ChannelNum = lastforkBits;
							AddToMarks(pm, g_vPMs);
							AddToMarks(pm, g_vPMs2);
							fork.ForkNo = pm.Data;
							fork.Bits = pm.ChannelNum;
							fork.End = pm;
							g_vForks.emplace_back(fork);

							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_HJFORK_BEGIN;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = block.vBStepDatas[j].Step;
							pm.Manual = 1;
							pm.ChannelNum = block.BlockHead.BitS;
							pm.Data = swNum;
							lastswNum = pm.Data;
							lastforkBits = pm.ChannelNum;
							AddToMarks(pm, g_vPMs);
							AddToMarks(pm, g_vPMs2);
							fork.Begin = pm;
						}
					}



					if ((mark & QIAO) && isBridge == 0)
					{
						memset(&pm, 0, sizeof(pm));
						pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
						pm.Mark = PM_BRIDGE_BEGIN;
						pm.Block = block.Index;
						pm.Step = j;
						pm.Step2 = block.vBStepDatas[j].Step;
						pm.Manual = 1;
						isBridge = 1;
						AddToMarks(pm, g_vPMs2);
					}
					else if ((mark & QIAO) == 0 && isBridge == 1)
					{
						memset(&pm, 0, sizeof(pm));
						pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
						pm.Mark = PM_BRIDGE_END;
						pm.Block = block.Index;
						pm.Step = j;
						pm.Step2 = block.vBStepDatas[j].Step;
						pm.Manual = 1;
						isBridge = 0;
						AddToMarks(pm, g_vPMs2);
					}

					if ((mark & CURVE) && isCurve == 0)
					{
						memset(&pm, 0, sizeof(pm));
						pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
						pm.Mark = PM_CURVE_BEGIN;
						pm.Block = block.Index;
						pm.Step = j;
						pm.Step2 = block.vBStepDatas[j].Step;
						pm.Manual = 1;
						isCurve = 1;
						AddToMarks(pm, g_vPMs2);
					}
					else if ((mark & CURVE) == 0 && isCurve == 1)
					{
						memset(&pm, 0, sizeof(pm));
						pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
						pm.Mark = PM_CURVE_END;
						pm.Block = block.Index;
						pm.Step = j;
						pm.Step2 = block.vBStepDatas[j].Step;
						pm.Manual = 1;
						isCurve = 0;
						AddToMarks(pm, g_vPMs2);
					}
				}
				else
				{
					if (mark & FORK)
					{
						if (isFork == 0)
						{
							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_HJFORK_BEGIN;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = step.Step;
							pm.Manual = 1;
							lastswNum = block.BlockHead.swNum;
							lastforkBits = block.BlockHead.BitS;
							/*
							if (i < blocks.size() - 1)
							{
								lastforkBits = blocks[i + 1].BlockHead.BitS;
								pm.Data = BCDToINT16(blocks[i + 1].BlockHead.swNum);
								lastswNum = pm.Data;
							}*/
							pm.Data = lastswNum;
							pm.ChannelNum = lastforkBits;
							isFork = 1;
							AddToMarks(pm, g_vPMs);
							AddToMarks(pm, g_vPMs2);

							fork.Begin = pm;
						}
						else if (isFork == 1)
						{
							uint16_t swNum = block.BlockHead.swNum;
							if (lastswNum != swNum || lastforkBits != block.BlockHead.BitS)//ԭ���Ľ������µĿ�ʼ
							{
								memset(&pm, 0, sizeof(pm));
								pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
								pm.Mark = PM_HJFORK_END;
								pm.Block = block.Index;
								pm.Step = j;
								pm.Step2 = block.vBStepDatas[j].Step;
								pm.Manual = 1;
								pm.Data = lastswNum;
								pm.ChannelNum = lastforkBits;
								AddToMarks(pm, g_vPMs);
								AddToMarks(pm, g_vPMs2);

								fork.ForkNo = pm.Data;
								fork.Bits = pm.ChannelNum;
								fork.End = pm;
								g_vForks.emplace_back(fork);

								memset(&pm, 0, sizeof(pm));
								pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
								pm.Mark = PM_HJFORK_BEGIN;
								pm.Block = block.Index;
								pm.Step = j;
								pm.Step2 = block.vBStepDatas[j].Step;
								pm.Manual = 1;
								pm.ChannelNum = block.BlockHead.BitS;
								pm.Data = swNum;
								lastswNum = pm.Data;
								lastforkBits = pm.ChannelNum;
								AddToMarks(pm, g_vPMs);
								AddToMarks(pm, g_vPMs2);
								fork.Begin = pm;
							}
							else
							{
								memset(&pm, 0, sizeof(pm));
								pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
								pm.Mark = PM_HJFORK_END;
								pm.Block = block.Index;
								pm.Step = j;
								pm.Step2 = step.Step;
								pm.Manual = 1;
								lastswNum = block.BlockHead.swNum;
								lastforkBits = block.BlockHead.BitS;
								/*
								if (i < blocks.size() - 1)
								{
									lastforkBits = blocks[i + 1].BlockHead.BitS;
									pm.Data = BCDToINT16(blocks[i + 1].BlockHead.swNum);
									lastswNum = pm.Data;
								}*/
															   
								pm.Data = lastswNum;
								pm.ChannelNum = lastforkBits;

								AddToMarks(pm, g_vPMs);
								AddToMarks(pm, g_vPMs2);

								fork.ForkNo = pm.Data;
								fork.Bits = pm.ChannelNum;
								fork.End = pm;
								g_vForks.emplace_back(fork);
								isFork = 0;
							}
						}
					}
				
					if (mark & QIAO)
					{
						if (isBridge == 0)
						{
							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_BRIDGE_BEGIN;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = block.vBStepDatas[j].Step;
							pm.Manual = 1;
							isBridge = 1;
							AddToMarks(pm, g_vPMs2);
						}
						else if (isBridge == 1)
						{
							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_BRIDGE_END;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = block.vBStepDatas[j].Step;
							pm.Manual = 1;
							isBridge = 0;
							AddToMarks(pm, g_vPMs2);
						}
					}

					if (mark & CURVE)
					{
						if (isCurve == 0)
						{
							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_CURVE_BEGIN;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = block.vBStepDatas[j].Step;
							pm.Data = (mark & BIT24) >> 24;
							pm.Manual = 1;
							isCurve = 1;
							AddToMarks(pm, g_vPMs2);
						}
						else if (isCurve == 1)
						{
							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_CURVE_END;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = block.vBStepDatas[j].Step;
							pm.Data = (mark & BIT24) >> 24;
							pm.Manual = 1;
							isCurve = 0;
							AddToMarks(pm, g_vPMs2);
						}
					}

					if (mark & TUNNEL)
					{
						if (isTunnel == 0)
						{
							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_TUNNEL_BEGIN;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = block.vBStepDatas[j].Step;
							pm.Manual = 1;
							isTunnel = 1;
							AddToMarks(pm, g_vPMs2);
						}
						else if (isTunnel == 1)
						{
							memset(&pm, 0, sizeof(pm));
							pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
							pm.Mark = PM_TUNNEL_END;
							pm.Block = block.Index;
							pm.Step = j;
							pm.Step2 = block.vBStepDatas[j].Step;
							pm.Manual = 1;
							isTunnel = 0;
							AddToMarks(pm, g_vPMs2);
						}
					}
				}

			}

			if (mark & BACK_P)//����
			{
				if (g_vReturnSteps.size() > 0 && g_vReturnSteps[g_vReturnSteps.size() - 1] == block.IndexL2 + block.BlockHead.row - 1)
				{

				}
				else
				{
					g_vReturnSteps.emplace_back(block.IndexL2 + block.BlockHead.row - 1);
					int k = j + 1;
					for (; k < block.vBStepDatas.size(); ++k)
					{
						g_vStepsInBackArea[block.IndexL2 + k] = 1;
					}
					if (i < MeterCount - 1)
					{
						for (k = 0; k < (*blocks)[i + 1].BlockHead.row; ++k)
						{
							g_vStepsInBackArea[(*blocks)[i + 1].IndexL2 + k] = 1;
						}
					}
				}
			}

			if (mark & SP_P)//����
			{
				//memset(&pm, 0, sizeof(pm));
				//pm.Walk = GetWD(blocks[i].BlockHead.walk, j, g_fileHead.step, g_direction);
				//pm.Mark = PM_OVERSPEED;
				//pm.Block = blocks[i].Index;
				//pm.Step = j;
				//pm.Step2 = blocks[i].vBStepDatas[j].Step;
				//vPMs2.emplace_back(pm);
			}
			if (mark & SEW)// ���#
			{
				if (g_vStepsInJoint.find(block.IndexL2 + j) == g_vStepsInJoint.end())
				{
					memset(&pm, 0, sizeof(pm));
					pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
					pm.Mark = PM_JOINT2;
					pm.Block = block.Index;
					pm.Step = j;
					pm.Step2 = block.vBStepDatas[j].Step;
					pm.Manual = 1;
					AddToMarks(pm, g_vPMs);
					pm.Mark = PM_JOINT;
					AddToMarks(pm, g_vPMs);
					g_vStepsInJoint[block.IndexL2 + j] = 1;
				}
			}
			if (mark & SEW2)// �ֶ�������*
			{
				if (g_vStepsInSew.find(block.IndexL2 + j) == g_vStepsInSew.end())
				{
					uint16_t data = (mark >> 20) & 0x0F;
					memset(&pm, 0, sizeof(pm));
					pm.Manual = 1;
					if (data == 1)
					{
						pm.Mark = PM_SEW_CH;
					}
					else if (data == 2)
					{
						pm.Mark = PM_SEW_LRH;
					}
					else if (data == 3)
					{
						pm.Mark = PM_SEW_LIVE;
					}

					pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
					pm.Block = block.Index;
					pm.Step = j;
					pm.Step2 = block.vBStepDatas[j].Step;
					if (data == 1 || data == 2 || data == 3)
					{
						pm.Data = data;
						AddToMarks(pm, g_vPMs);
					}

					pm.Mark = PM_SELFDEFINE;
					pm.Data = data;
					AddToMarks(pm, g_vPMs2);
					g_vStepsInSew[block.IndexL2 + j] = 1;
				}
			}
			if (mark & START)//�ϵ�
			{
				memset(&pm, 0, sizeof(pm));
				pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
				pm.Mark = PM_START;
				pm.Block = block.Index;
				pm.Step = j;
				pm.Manual = 1;
				AddToMarks(pm, g_vPMs2);
			}

			if (mark & CK_KM)
			{
				if (g_vStepsInK.find(block.IndexL2 + j) == g_vStepsInK.end())
				{
					memset(&pm, 0, sizeof(pm));
					pm.Walk = GetWD(block.BlockHead.walk, j, fHead.step, g_direction);
					pm.Mark = PM_WALKREVISE;
					pm.Block = block.Index;
					pm.Step = j;
					pm.Step2 = block.vBStepDatas[j].Step;
					pm.Manual = 1;
					AddToMarks(pm, g_vPMs);
					AddToMarks(pm, g_vPMs2);
					g_vStepsInK[block.IndexL2 + j] = 1;
				}
			}

			uint16_t& couple = block.vBStepDatas[j].Mark.Couple;
			if (couple & BIT15)
			{
				if (g_vStepsInCouple.find(block.IndexL2 + j) == g_vStepsInCouple.end())
				{
					LCP lcp;
					memset(&lcp, 0, sizeof(lcp));
					lcp.Block = block.Index;
					lcp.Step = block.vBStepDatas[j].Step;
					lcp.Channel = couple & 0xFFF;
					g_vLCP.emplace_back(lcp);
					g_vStepsInCouple[block.IndexL2 + j] = 1;
				}
			}

#pragma endregion 

			uint8_t rowCount = step.vRowDatas.size();
			// ���β�ѯÿ����
			for (int k = 0; k < rowCount; k++)
			{
				B_POINT& Point = step.vRowDatas[k].Point;
				if (g_pFileWavePoints != NULL && Point.Draw1 > 0)
				{
					wp.Channel = Point.Draw1;
					wp.Step = step.Step;
					wp.Row = step.vRowDatas[k].Row;
					wp.JawRow = g_iJawRow[blockTemp.BlockHead.railType & 0x03];
					fwrite(&wp, szWavePoint, 1, g_pFileWavePoints);

					for (int m = 0; m < CH_c; ++m)
					{
						if (Point.Draw1 & bits[m])
						{
							(*blocks)[i].sumAaBbCc++;
						}
					}
					for (int m = CH_D; m < CH_e; ++m)
					{
						if ((m == CH_D || m == CH_E) && (Point.Draw1 & bits[m]))
						{
							(*blocks)[i].sumDE++;
						}
					}

					if (step.vRowDatas[k].Row < iFRow - 10)
					{
						for (int m = CH_F; m <= CH_G; ++m)
						{
							if (Point.Draw1 & bits[m])
							{
								(*blocks)[i].sumFG++;
							}
						}
					}
				}

				//memcpy(&Point, &CompBuff[BUFF_LENGHT*k + j], sizeof(B_POINT));                 // �õ���ǰ��
				if (LineMark.Mark & ALARM_M)             // ����г�������
				{
					if (tas->AlarmDataCount < MAX_ALARMDATA_NUM && Point.Alarm & ALL_CHN_ALARM)   // �鿴�˲������Ƿ��г�������
					{
						// ���ÿһ��ͨ��
						for (int n = 0; n < CH_N - 3; n++)     // CH_N-3��ʾG��d��eͨ�����㣬
						{
							// ���ÿһ������
							for (int t = 0; t < 4; t++)
							{
								if (Point.Alarm & tas->AlarmBitMark[n][t])     // ����б���
								{
									bool new_alarm = true;          // �±�����¼��־
									// ��ÿ�����ڼ��������Ƚ�
									for (int chk = 0; chk < tas->CheckAlarmCount; chk++)
									{
										TAlarmData *ad = &tas->ADB[tas->CheckAlarmNum[chk]];      // ������ϸ��Ϣ
										if (ad->alarm_type != 0                  // ����ʧ������
											&& ad->alarm_chn == n               // ͨ����ͬ
											&& ad->alarm_type == t              // ������ͬ
											&& tas->CheckAlarmInv[chk] > 0)     // �������������
										{
											// ��ֱλ��������5��������
											//if (k >= ad->VerStart - 5 && k <= ad->VerEnd + 5)
											if (step.vRowDatas[k].Row >= ad->VerStart - 5 && step.vRowDatas[k].Row <= ad->VerEnd + 5)
											{
												// ���¶�Ӧ������Ϣ
												// ��ֱ����ǳ
												if (step.vRowDatas[k].Row < ad->VerStart)
												{
													//ad->VerStart = k;
													ad->VerStart = step.vRowDatas[k].Row;
												}
												// ��ֱ������
												if (step.vRowDatas[k].Row > ad->VerEnd)
												{
													//ad->VerEnd = k;
													ad->VerEnd = step.vRowDatas[k].Row;
												}
												// ����ͬһ�еı���
												if (tas->CheckAlarmInv[chk] < SearchInv)
												{
													ad->StepCount += 5 - tas->CheckAlarmInv[chk];
													// ����֮�����ü��
													tas->CheckAlarmInv[chk] = SearchInv;
												}
												ad->PotCount++;         // �������������
												new_alarm = false;
											}
										}
									}
									if (new_alarm)           // �����±���
									{
										tas->ADB[tas->AlarmDataCount].Number = tas->AlarmDataCount;
										tas->ADB[tas->AlarmDataCount].MeterIndex = i;
										tas->ADB[tas->AlarmDataCount].StepIndex = j;
										tas->ADB[tas->AlarmDataCount].VerPos = step.vRowDatas[k].Row;// k;
										tas->ADB[tas->AlarmDataCount].PotCount = 1;
										tas->ADB[tas->AlarmDataCount].StepCount = 1;
										tas->ADB[tas->AlarmDataCount].VerStart = step.vRowDatas[k].Row;// k;
										tas->ADB[tas->AlarmDataCount].VerEnd = step.vRowDatas[k].Row;// k;
										tas->ADB[tas->AlarmDataCount].alarm_chn = n;
										tas->ADB[tas->AlarmDataCount].alarm_type = t;
										// ʵʱ���
										//tas->ADB[tas->AlarmDataCount].Walk = GetFactWalk(i, j);
										// �ٶ�
										tas->ADB[tas->AlarmDataCount].Speed = MeterHead.speed;
										// ��Ӧ��Ϣ
										tas->ADB[tas->AlarmDataCount].HasCorrDefect = false;
										tas->ADB[tas->AlarmDataCount].CorrDefectNum = 0;
										tas->ADB[tas->AlarmDataCount].InRailGap = 0;
										// ���뱨����������
										tas->CheckAlarmNum[tas->CheckAlarmCount] = tas->AlarmDataCount;
										tas->CheckAlarmInv[tas->CheckAlarmCount] = SearchInv;
										tas->CheckAlarmCount++;
										// �޸ı���ͳ��
										tas->ChannelAlarmCount[n][t]++;
										tas->AlarmDataCount++;
									}
								}
							}
						}
					}
				}

				// ��������
				UINT8 defect_type = 0;              // ��������
				if (LineWound.W_Mark & W_MAN)              // �ֶ�����
				{
					defect_type = 1;
				}
				else if (LineWound.W_Mark & W_ADD)          // �طű���
				{
					defect_type = 2;
				}
				else if (LineWound.W_Mark & (W_D_SER | W_D_SLI | W_SIG | W_HOL))     // ���Ʊ���
				{
					defect_type = 3;
				}
				if (tds->DefectDataCount < MAX_DEFECTDATA_NUM && defect_type > 0)                // �жϴ����Ƿ�������
				{
					UINT8 defect_degree = 2;           // ����̶�
					if (1)//Point.Alarm & ALL_CHN_ALARM)           // ����˵��г���������
					{
						// �жϱ�������������
						if (Point.Wound & W_MAN)              // �ֶ�����
						{
							defect_type = 1;
							defect_degree = LineWound.W_Mark >> 8 & 0xf;
						}
						else if (Point.Wound & W_ADD)          // �طű���
						{
							defect_type = 2;
							defect_degree = LineWound.W_Mark >> 8 & 0xf;
						}
						else if (Point.Wound & (W_D_SER | W_D_SLI | W_SIG | W_HOL))     // ���Ʊ���
						{
							defect_type = 3;
							if (Point.Wound & W_D_SER)           // ��ͨ������
							{
								defect_degree = 3;
							}
							else if (Point.Wound & W_HOL)        // �ݿ�����
							{
								defect_degree = 1;
							}
							else                                // ����Ϊ����
							{
								defect_degree = 2;
							}
						}
						bool new_defect = true;          // �������¼��־
						if (tds->CheckDefectCount > 0)               // �ж��Ƿ�������������
						{
							int updata_count = 0;           // �˵���µ�������
							int updata_num[20];             // �˵���µ�����������
							// ��ÿ�����ڼ��������Ƚ�
							for (int chk = 0; chk < tds->CheckDefectCount; chk++)
							{
								TDefectData *dd = &tds->DDB[tds->CheckDefectNum[chk]];      // ������ϸ��Ϣ
								// ��ֱλ��������5��������
								if (k >= dd->VerStart - 5 && k <= dd->VerEnd + 5)
								{
									// ˮƽλ��û�����������
									if (tds->CheckDefectInv[chk] <= SearchInv)
									{
										// ����������Ϣ
										CheckDefectUpdata(dd, SearchInv - tds->CheckDefectInv[chk], k, defect_type, defect_degree, LineWound.W_Code, LineWound.Other & 0xf);
										// ����������
										tds->CheckDefectInv[chk] = SearchInv;
										// �����¼�¼��־
										new_defect = false;
										updata_num[updata_count] = chk;
										updata_count++;
									}
								}
							}
							// �˵���µ�����ֹһ��������ϲ�������Ϣ
							while (updata_count > 1)
							{
								// �ϲ�����������Ϣ
								CheckDefectMerge(tds, tds->CheckDefectNum[updata_num[0]], tds->CheckDefectNum[updata_num[1]]);
								// �޸Ķ�Ӧ���
								for (int num = 1; num < updata_count - 1; num++)
								{
									updata_num[num] = updata_num[num + 1];
									updata_num[num]--;
								}
								updata_num[updata_count - 1] = 0;
								updata_count--;
							}
						}
						if (new_defect)              // ������������¼
						{
							// ���һ��������Ϣ
							CheckDefectAdd(fHead, MeterHead, i, j, k, defect_type, defect_degree, LineWound.W_Code, LineWound.Other & 0xf, tds);
							// ���ݵ�ǰ����������Ϣ��������������
							if (!(LineMark.Mark & FORK))       // �޵�����
							{
								// ��յ�����
								tds->DDB[tds->DefectDataCount - 1].swNum = 0;
							}
							// ���������������
							tds->CheckDefectNum[tds->CheckDefectCount] = tds->DefectDataCount - 1;
							tds->CheckDefectInv[tds->CheckDefectCount] = SearchInv;
							tds->CheckDefectCount++;
						}
					}
				}
			}

			if (tas->AlarmDataCount < MAX_ALARMDATA_NUM && (LineMark.Mark & LOSE_37))         // �����37��ʧ������
			{
				if (tas->AlarmInterval[9][0] <= 0)               // ����������������׼
				{
					tas->ADB[tas->AlarmDataCount].Number = tas->AlarmDataCount;
					tas->ADB[tas->AlarmDataCount].MeterIndex = i;
					tas->ADB[tas->AlarmDataCount].StepIndex = j;
					tas->ADB[tas->AlarmDataCount].StepCount = 1;
					tas->ADB[tas->AlarmDataCount].alarm_chn = 9;
					tas->ADB[tas->AlarmDataCount].alarm_type = 0;
					// ʵʱ���
					//tas->ADB[tas->AlarmDataCount].Walk = GetFactWalk(i, j);
					// �ٶ�
					tas->ADB[tas->AlarmDataCount].Speed = MeterHead.speed;
					// ʧ��������ű���
					LostNum_37 = tas->AlarmDataCount;
					// �޸ı���ͳ��
					tas->ChannelAlarmCount[9][0]++;
					tas->AlarmDataCount++;
				}
				else
				{
					// ʧ�����������ۼ�����
					tas->ADB[LostNum_37].StepCount++;
				}
				// ���ò������
				tas->AlarmInterval[9][0] = SearchInv;
			}
			if (tas->AlarmDataCount < MAX_ALARMDATA_NUM && (LineMark.Mark & LOSE_0))         // �����0��ʧ������
			{
				if (tas->AlarmInterval[8][0] <= 0)               // ����������������׼
				{
					tas->ADB[tas->AlarmDataCount].Number = tas->AlarmDataCount;
					tas->ADB[tas->AlarmDataCount].MeterIndex = i;
					tas->ADB[tas->AlarmDataCount].StepIndex = j;
					tas->ADB[tas->AlarmDataCount].StepCount = 1;
					tas->ADB[tas->AlarmDataCount].alarm_chn = 8;
					tas->ADB[tas->AlarmDataCount].alarm_type = 0;
					// ʵʱ���
					//tas->ADB[tas->AlarmDataCount].Walk = GetFactWalk(i, j);
					// �ٶ�
					tas->ADB[tas->AlarmDataCount].Speed = MeterHead.speed;
					// ʧ��������ű���
					LostNum_0 = tas->AlarmDataCount;
					// �޸ı���ͳ��
					tas->ChannelAlarmCount[8][0]++;
					tas->AlarmDataCount++;
				}
				else
				{
					// ʧ�����������ۼ�����
					tas->ADB[LostNum_0].StepCount++;
				}
				// ���ò������
				tas->AlarmInterval[8][0] = SearchInv;
			}
			// ÿ��������ɺ󣬼�С�������
			// ��С�����������
			for (int x = 0; x < tas->CheckAlarmCount; x++)
			{
				if (tas->CheckAlarmInv[x] > 0 && tas->CheckAlarmNum[x] >= 0 && tas->ADB[tas->CheckAlarmNum[x]].StepCount < MAX_AL_DE_STEP_COUNT)
				{
					tas->CheckAlarmInv[x]--;
				}
				else                        // ���Ϊ0���߲����������꣬˵���˱����������
				{
					// �Ӽ���������ȥ��
					for (int y = x; y < tas->CheckAlarmCount - 1; y++)
					{
						tas->CheckAlarmNum[y] = tas->CheckAlarmNum[y + 1];
						tas->CheckAlarmInv[y] = tas->CheckAlarmInv[y + 1];
					}
					tas->CheckAlarmInv[tas->CheckAlarmCount - 1] = 0;
					tas->CheckAlarmCount--;
					x--;
				}
			}
			// ��Сʧ���������
			for (int x = 0; x < CH_N - 2; x++)
			{
				for (int y = 0; y < 4; y++)
				{
					if (tas->AlarmInterval[x][y] > 0)
					{
						tas->AlarmInterval[x][y]--;
					}
				}
			}
			// ��С������
			for (int x = 0; x < tds->CheckDefectCount; x++)
			{

				if (tds->CheckDefectInv[x] > 0 && tds->SDDB[tds->CheckDefectNum[x]].StepCount < MAX_AL_DE_STEP_COUNT)
				{
					tds->CheckDefectInv[x]--;
				}
				else                        // ���Ϊ0�򲽽��������꣬˵��������������
				{
					// �Ӽ���������ȥ��
					for (int y = x; y < tds->CheckDefectCount - 1; y++)
					{
						tds->CheckDefectNum[y] = tds->CheckDefectNum[y + 1];
						tds->CheckDefectInv[y] = tds->CheckDefectInv[y + 1];
					}
					tds->CheckDefectNum[tds->CheckDefectCount - 1] = 0;
					tds->CheckDefectInv[tds->CheckDefectCount - 1] = 0;
					tds->CheckDefectCount--;
					x--;
				}
			}
			// ���������Ϣ
			bool found[SIGN_NUM] = { false, false, false, false, false, false, false, false, false, false, false };   // �ҵ���ͬ���ͱ�Ǳ�־
			bool IsBackMark = false;            // �����еĻ��˱�־�������ڵ�ǰ�׿����һ������
			if (LineMark.Mark & 0xF03F8)           // ����б��
			{
				// ���β�ѯÿ�ֱ��
				if (LineMark.Mark & QIAO)    found[0] = true;            // ����
				if (LineMark.Mark & CURVE)   found[1] = true;            // ����
				if (LineMark.Mark & FORK)    found[2] = true;            // ����
				if (LineMark.Mark & SEW2)    found[3] = true;            // �ֶ���־
				if (LineMark.Mark & SEW)     found[4] = true;            // ����
				if (LineMark.Mark & BACK_P)                              // ����              
				{
					BackAction ba;
					ba.Pos1.Block = block.Index;
					ba.Pos1.Step = block.vBStepDatas.size() - 1;
					ba.Pos1.Step2 = block.IndexL2 + ba.Pos1.Step;
					ba.Pos1.Walk = GetWD(blockTemp.Walk, blockTemp.StepCount, g_filehead.step, g_direction);
					double realwalk = GetWD(blockTemp.Walk, j, g_filehead.step, g_direction);
					if (i >= MeterCount - 1)
					{
						continue;
					}
					else
					{
						if (blockTemp.IndexL + blockTemp.StepCount - 1 < (*blocks)[i + 1].BlockHead.indexL)
						{
							char szLog[100] = { 0 };
							sprintf(szLog, "�׿飺%d��������%d�л��˱�ǣ�����������\n", i, j);
							WriteLog(szLog);
							continue;
						}

						ba.Pos2 = FindRawStepInBlock(blockTemp.IndexL + blockTemp.StepCount - 1, *blocks, i + 1);
						if (ba.Pos2.Step2 == 0)
						{
							ba.Pos2.Step2 = (*blocks)[MeterCount - 1].IndexL2 + (*blocks)[MeterCount - 1].StepCount - 1;
							ba.Pos2.Step = (*blocks)[MeterCount - 1].StepCount - 1;
							ba.Pos2.Block = MeterCount - 1;
						}
						ba.Pos0.Step2 = 2 * (ba.Pos1.Step2) - ba.Pos2.Step2 + 1;
						if (ba.Pos0.Step2 < 0)
						{
							ba.Pos0.Step2 = 0;
						}
						ba.Pos0 = FindStepInBlock(ba.Pos0.Step2, *blocks, 0);

						if (fabs(ba.Pos2.Walk - ba.Pos1.Walk) > 0.2)//���˴���200�ף������������쳣
						{
							ba.Pos0.Step2 = ba.Pos1.Step2 - 1;
							ba.Pos2.Step2 = ba.Pos1.Step2 + 1;
						}

						g_vBAs.emplace_back(ba);
					}

					if (i < MeterCount - 1)            // ��Ϊ���һ���׿�
					{
						IsBackMeter = 2;        // �����׿���˱�־
					}
				}
				if (LineMark.Mark & START)   found[6] = true;            // �ϵ�
				if (LineMark.Mark & CHECK)   found[7] = true;            // У��
				if (LineMark.Mark & SP_P)     found[8] = true;           // ���ٵ�
				if (LineMark.Mark & CK_KM)   found[9] = true;            // ���У��
			}
			// �����׿�ʱ����������λ��δ�������ǵ�����
			if (IsBackMeter == 2)                // ��һ�׿�Ϊ�����׿�ʱ
			{
				if (found[0] == false && CheckingSign[0] == true)                // ����
				{
					found[0] = true;
				}
				if (found[1] == false && CheckingSign[1] == true)                // ����
				{
					found[1] = true;
				}
				if (found[2] == false && CheckingSign[2] == true)                // ����
				{
					found[2] = true;
				}
			}
			// �жϻ��˱�־״̬
			switch (IsBackMeter)
			{
			case 0:                 // �޻��˱�־�׿�
			case 1:                 // ������һ��־�׿�
				if (CheckingSign[5] == true)     // ���ڼ�������
				{
					// ���˲����Ƿ��ڻ��˷�Χ��
					if (MeterHead.indexL + j <= FallBackCurStep)
					{
						found[5] = true;
					}
					else
					{
						// ����Χ�����ʼ��
						FallBackCurStep = -999999;
					}
				}
				break;
			case 2:                 // ���˱�־�׿�
				// �жϴ˲����Ƿ�����һ���˷�Χһ���׿鲽����
				if (MeterHead.indexL + j <= FallBackCurStep + 1000 / HOR_POINT_FOR_LENTH)
				{
					found[5] = true;
					// �׿�ĩβ����
					if (j == MeterHead.row - 1)
					{
						IsBackMark = true;
					}
				}
				else
				{
					//if (g_vBAs.size() > 0)
					//{
					//	g_vBAs.erase(g_vBAs.begin() + g_vBAs.size() - 1);
					//}
					// �׿�ĩβ����
					if (j == MeterHead.row - 1)
					{
						found[5] = true;
						// ��ʼ�����ڼ�����־
						CheckingSign[5] = false;
						CheckingSignNum[5] = 0;
					}
				}
				break;
			}
			// �����ϲ���
			if ((LineMark.Couple & BIT15))// && (LineMark.Couple & 0xDBF))           // �����ϲ���
			{
				found[10] = true;
			}
			// ���ݼ���״̬���޸ı������
			UINT32 MaxNum[SIGN_NUM] = { 1000, 1000, 1000, 1000, 5000, 1000,
				500, 1000, 10000, 500, 10000 };
			UINT8 CheckMarkState;             // ������־״̬:0 - δ������1 - ���������ı�־
			for (int t = 0; t < SIGN_NUM; t++)
			{
				CheckMarkState = 0;     // ÿ�β��ң�״̬��ʼ��
				if (found[t])        // �˲����д��ֱ��
				{
					if (CheckingSign[t])     // ���ڲ��Ҵ��ֱ��
					{
						if (t == 2)      // Ϊ����ʱ��Ҫ�鿴�Ƿ����µ����
						{
							if (tss->SDB[CheckingSignNum[t]].SpecialInfo == MeterHead.swNum)
								CheckMarkState = 1;
							else
								CheckMarkState = 0;
						}
						else
						{
							CheckMarkState = 1;
						}
					}
					else
					{
						CheckMarkState = 0;
					}

					if (CheckMarkState == 1)                  // ���ڼ������ֱ��
					{
						// ���´˱����ϸ��Ϣ
						tss->SDB[CheckingSignNum[t]].MeterIndex_E = i;
						tss->SDB[CheckingSignNum[t]].StepIndex_E = j;
						tss->SDB[CheckingSignNum[t]].StepCount++;
						if (t == 5 && IsBackMark == true)          // Ϊ����ʱ���������л��˱�־
						{
							// ���˼�������
							tss->SDB[CheckingSignNum[t]].BackCount++;
						}
					}
					else if (tss->SignTypeCount[t] < MaxNum[t])                   // û�������ֱ�ǣ���ͳ��δ����
					{
						// ����һ����Ǽ�¼
						tss->SDB[tss->SignCount].Number = tss->SignCount;
						tss->SDB[tss->SignCount].MeterIndex_S = i;
						tss->SDB[tss->SignCount].StepIndex_S = j;
						tss->SDB[tss->SignCount].MeterIndex_E = i;
						tss->SDB[tss->SignCount].StepIndex_E = j;
						tss->SDB[tss->SignCount].SignType = t;
						tss->SDB[tss->SignCount].BackCount = 0;         // ��ʼ�����˼���
						tss->SDB[tss->SignCount].SwState = 0;               // ��ʼ������״̬
						// ������Ϣ
						if (t == 10)     // ʧ��ʱ��Ҫͳ��ʧ��ͨ��
						{
							tss->SDB[tss->SignCount].SpecialInfo = LineMark.Couple;
						}
						else if (t == 2)         // ����
						{
							tss->SDB[tss->SignCount].SpecialInfo = MeterHead.swNum;
							tss->SDB[tss->SignCount].SwState = MeterHead.BitS;      // ����״̬
						}
						else if (t == 3)         // �Զ���
						{
							tss->SDB[tss->SignCount].SpecialInfo = (LineMark.Mark & SEW_N) >> 20;
						}
						else if (t == 5)      // ����ʱ���õ����˾���Ĳ�������
						{
							// ���˶�Ӧ�����������׿�ĩβ��
							FallBackCurStep = (*blocks)[tss->SDB[tss->SignCount].MeterIndex_S].IndexL
								+ (*blocks)[tss->SDB[tss->SignCount].MeterIndex_S].StepCount - 1;
							if (tss->SDB[tss->SignCount].MeterIndex_S < MeterCount - 1)        // ��Ϊ���һ���׿�
							{
								// ��������Ϊ��ǰλ�ü�������һ�׿�ͷ���������Ĳ�ֵ
								tss->SDB[tss->SignCount].SpecialInfo = abs((*blocks)[tss->SDB[tss->SignCount].MeterIndex_S].IndexL +
									tss->SDB[tss->SignCount].StepIndex_S -
									(*blocks)[tss->SDB[tss->SignCount].MeterIndex_S + 1].IndexL);
							}
							else
							{
								tss->SDB[tss->SignCount].SpecialInfo = 1;
							}
							tss->SDB[tss->SignCount].BackCount = 1;         // ���û��˼���
						}
						// ��������
						tss->SDB[tss->SignCount].StepCount = 1;
						//tss->SDB[tss->SignCount].Walk = GetFactWalk(i, j);
						// ���¼�����־
						CheckingSign[t] = true;
						CheckingSignNum[t] = tss->SignCount;
						// ���±��ͳ��
						tss->SignTypeCount[t]++;
						tss->SignCount++;
					}
				}
				else        // �ޱ��
				{
					if (CheckingSign[t])                 // ���ڼ������ֱ��
					{
						if (!(IsBackMeter == 1 && t <= 2 && j < 5))              // �ų������׿�ǰ5�������������������ߡ���������
						{
							// �˱�Ǽ������
							CheckingSign[t] = false;
							CheckingSignNum[t] = 0;
						}
					}
				}
			}
			// ͳ�ƻ����б�
			// �жϻ��˱�־״̬
			UINT32 meter;
			UINT16 stepInBlock;
			INT32 temCurStep;
			if (tbs->OrgDataCount < MAX_SIGNDATA_NUM)
			{
				switch (IsBackMeter)
				{
				case 0:                 // �޻��˱�־�׿�
					if (BackIndex >= 0)          // ���ڼ�������
					{
						if (MeterHead.indexL + j <= Org_BackCurStep_E)     // �ڲ���������Χ��
						{
							tbs->CD_List[BackIndex].MeterIndex_E = i;
							tbs->CD_List[BackIndex].StepIndex_E = j;
							tbs->CD_List[BackIndex].StepCount++;
						}
						else
						{
							BackIndex = -1;
						}
					}
					if (BackIndex_Org >= 0)      // ���ڼ���������
					{
						if (MeterHead.indexL + j > Org_BackCurStep_E)     // ��������������Χ��Χ
						{
							BackIndex_Org = -1;
						}
					}
					break;
				case 1:                 // ������һ��־�׿�
					// �����»��˼�¼
					BackIndex = tbs->DataCount++;
					tbs->CD_List[BackIndex].Index = BackIndex;
					tbs->CD_List[BackIndex].MeterIndex_S = i;
					tbs->CD_List[BackIndex].StepIndex_S = j;
					tbs->CD_List[BackIndex].MeterIndex_E = i;
					tbs->CD_List[BackIndex].StepIndex_E = j;
					tbs->CD_List[BackIndex].ListIndex = tbs->OrgDataCount - 1;
					tbs->CD_List[BackIndex].CheckCount = tbs->Org_CD_List[BackIndex_Org].CheckCount + 1;
					tbs->CD_List[BackIndex].StepCount = 1;
					// ���ı����˼�¼
					tbs->Org_CD_List[BackIndex_Org].CheckCount++;
					temCurStep = GetCurStepFromStepPos(*blocks, i, j);             // ��ʱ��Ӧ��������
					if (temCurStep < Org_BackCurStep_S)             // ���˼�¼������С�ڵ�ǰ��������ʼ��
					{
						if (BackIndex_Org > 0)           // ��Ϊ��һ����¼
						{
							// �õ���һ����¼�յ�λ��
							meter = tbs->Org_CD_List[BackIndex_Org - 1].MeterIndex_E;
							stepInBlock = tbs->Org_CD_List[BackIndex_Org - 1].StepIndex_E;
							if (temCurStep <= GetCurStepFromStepPos(*blocks, meter, stepInBlock))         // ���˼�¼������С����һ�������˼�¼�յ�
							{
								// ���Ĳ��ҵĶ�Ӧ��������
								temCurStep = GetCurStepFromStepPos(*blocks, meter, stepInBlock) + 1;
							}
						}
						// �õ��������
						meter = tbs->Org_CD_List[BackIndex_Org].MeterIndex_S;
						stepInBlock = tbs->Org_CD_List[BackIndex_Org].StepIndex_S;
						// �Ӽ�¼��ʼλ�����������
						GetStepPosFromCurStep(*blocks, temCurStep, &meter, &stepInBlock, false);
						// ���ü�¼
						Org_BackCurStep_S = GetCurStepFromStepPos(*blocks, meter, stepInBlock);
						tbs->Org_CD_List[BackIndex_Org].MeterIndex_S = meter;
						tbs->Org_CD_List[BackIndex_Org].StepIndex_S = stepInBlock;
					}
					// �޸Ļ��˱�־
					IsBackMeter--;
					break;
				case 2:                 // ���˱�־�׿�
					if (BackIndex >= 0)               // ���ڼ������˼�¼
					{
						if (MeterHead.indexL + j <= Org_BackCurStep_E + 1000 / HOR_POINT_FOR_LENTH)         // �����ڳ������˷�Χһ���׿���
						{
							// ���»��˼�¼
							tbs->CD_List[BackIndex].MeterIndex_E = i;
							tbs->CD_List[BackIndex].StepIndex_E = j;
							tbs->CD_List[BackIndex].StepCount++;
						}
						else
						{
							BackIndex = -1;         // ��������
						}
					}
					// �˲����Ƿ��ڳ��������˷�Χ
					if (MeterHead.indexL + j > Org_BackCurStep_E + 1000 / HOR_POINT_FOR_LENTH)
					{
						if (BackIndex_Org >= 0)
						{
							BackIndex_Org = -1;     // ��������
						}
					}
					// �׿�ĩβ�����������޼���������
					try {
						if (j == MeterHead.row - 1 && BackIndex_Org == -1)
						{
							// �����±����˼�¼
							BackIndex_Org = tbs->OrgDataCount++;
							tbs->Org_CD_List[BackIndex_Org].Index = BackIndex_Org;
							tbs->Org_CD_List[BackIndex_Org].CheckCount = 0;
							tbs->Org_CD_List[BackIndex_Org].MeterIndex_S = i;
							tbs->Org_CD_List[BackIndex_Org].StepIndex_S = j;
							tbs->Org_CD_List[BackIndex_Org].MeterIndex_E = i;
							tbs->Org_CD_List[BackIndex_Org].StepIndex_E = j;
							tbs->Org_CD_List[BackIndex_Org].StepCount = 0;

							Org_BackCurStep_E = GetCurStepFromStepPos(*blocks, i, j);        // �õ�����������Ĳ�����Ӧ��
							Org_BackCurStep_S = Org_BackCurStep_E;
						}
					}
					catch (...)
					{
						int sdfsdf = 0;
					}
					break;
				}
			}
		}

		// �����׿�ͷ                           
		MeterBak = MeterHead;
		if (IsBackMeter > 0)             IsBackMeter--;          // �޸��׿���˱�־

		if (tds->DefectDataCount == 1)
			tempNum = tds->DDB[0].Number;
	}
	fclose(g_pFileWavePoints);
	g_pFileWavePoints = nullptr;

	// ���̶ֳ�����ͳ�ƣ������ڼ���֮����ܻ������ˣ�����Ҫ�������ͳ�ƣ�
	for (UINT32 i = 0; i < tds->DefectDataCount; i++)
	{
		tds->TypeDefectCount[tds->DDB[i].DefectType - 1][tds->DDB[i].DefectDegree - 1]++;
	}

	std::map<int, int> vBlocksBack;
	int ibeginBlock = 0;


	for (int i = g_vBAs.size() - 1; i >= 1; --i)
	{
		if (g_vBAs[i].Pos1.Step2 == g_vBAs[i - 1].Pos1.Step2)
		{
			g_vBAs[i - 1].Pos2 = g_vBAs[i].Pos2;
			g_vBAs.erase(g_vBAs.begin() + i);
		}
	}


	if (g_vBAs.size() > 0)
	{
		g_vBT.clear();
		BackTotal bt;
		bt.BackCount = 1;
		bt.BeginBackIndex = 0;
		bt.Block1 = g_vBAs[0].Pos1.Block;
		bt.Block2 = g_vBAs[0].Pos1.Block;
		bt.Step1 = g_vBAs[0].Pos1.Step2;
		bt.Step2 = g_vBAs[0].Pos2.Step2;

		for (int i = 0; i < g_vBAs.size(); ++i)
		{
			g_vBAs[i].Pos0 = FindStepInBlock(g_vBAs[i].Pos0.Step2, g_vBlockHeads, ibeginBlock);
			g_vBAs[i].Pos2 = FindStepInBlock(g_vBAs[i].Pos2.Step2, g_vBlockHeads, ibeginBlock);
			if (i > 0 && g_vBAs[i].Pos1.Step2 < g_vBAs[i - 1].Pos2.Step2)
			{
				g_vBAs[i - 1].Pos2 = g_vBAs[i].Pos1;
				g_vBAs[i].Pos0 = g_vBAs[i].Pos1;
			}
			else if (i > 0 && g_vBAs[i].Pos0.Step2 < g_vBAs[i - 1].Pos2.Step2)
			{
				if (g_vBAs[i - 1].Pos2.Step2 > g_vBAs[i].Pos1.Step2)
				{
					g_vBAs[i - 1].Pos2 = g_vBAs[i].Pos1;
				}

				if (g_vBAs[i - 1].Pos2.Step2 < g_vBAs[i].Pos1.Step2 && g_vBAs[i - 1].Pos2.Step2 > g_vBAs[i].Pos0.Step2)
				{
					g_vBAs[i].Pos0 = g_vBAs[i - 1].Pos2;
				}

				//if (g_vBAs[i - 1].Pos1.Step2 < g_vBAs[i].Pos0.Step2)
				//{
				//	g_vBAs[i - 1].Pos2 = g_vBAs[i].Pos0;
				//}
				//else if (g_vBAs[i].Pos0.Step2 < g_vBAs[i - 1].Pos2.Step2)
				//{
				//	g_vBAs[i].Pos0 = g_vBAs[i - 1].Pos2;
				//}
			}

			if (i == bt.BeginBackIndex)
			{

			}
			else if (g_vBAs[i].Pos0.Step2 <= bt.Step2)
			{
				bt.Block2 = g_vBAs[i].Pos2.Block;
				bt.Step2 = g_vBAs[i].Pos2.Step2;
				bt.BackCount++;
			}
			else
			{
				g_vBT.push_back(bt);
				if (i < g_vBAs.size() - 1)
				{
					bt.BackCount = 1;
					bt.BeginBackIndex = i;
					bt.Block1 = g_vBAs[i].Pos1.Block;
					bt.Block2 = g_vBAs[i].Pos1.Block;
					bt.Step1 = g_vBAs[i].Pos1.Step2;
					bt.Step2 = g_vBAs[i].Pos2.Step2;
				}
			}

			ibeginBlock = 0;
			vBlocksBack[g_vBAs[i].Pos1.Block] = 1;


			if (i == g_vBAs.size() - 1)
			{
				g_vBT.push_back(bt);
			}
		}


		//g_vBT.clear();
		//if (g_vBAs.size() > 0)
		//{
		//	int idx = 0, jdx = 0;
		//	int bCount = g_vBAs.size();
		//	bool isEnd = false;
		//	while (idx < bCount)
		//	{
		//		BackTotal bt;
		//		int lstIndex = tbs->CD_List[idx].ListIndex;
		//		bt.Block1 = tbs->CD_List[idx].MeterIndex_S;
		//		bt.Step1 = tbs->CD_List[idx].StepIndex_S;
		//		bt.Block2 = tbs->CD_List[idx].MeterIndex_E;
		//		bt.Step2 = tbs->CD_List[idx].StepIndex_E;
		//		bt.BackCount = 1;
		//		bt.BeginBackIndex = idx;
		//		if (idx == bCount - 1)
		//		{
		//			g_vBT.emplace_back(bt);
		//			idx++;
		//			break;
		//		}
		//		for (int i = idx + 1; i < bCount; ++i)
		//		{
		//			if (lstIndex == tbs->CD_List[i].ListIndex)
		//			{
		//				bt.Block2 = tbs->CD_List[i].MeterIndex_E;
		//				bt.Step2 = tbs->CD_List[i].StepIndex_E;
		//				bt.BackCount++;
		//				if (i == bCount - 1)
		//				{
		//					g_vBT.emplace_back(bt);
		//					idx = bCount;
		//					break;
		//				}
		//			}
		//			else
		//			{
		//				g_vBT.emplace_back(bt);
		//				idx = i;
		//				if (i == bCount - 1)
		//				{
		//					bt.BeginBackIndex = idx;
		//					bt.Block1 = tbs->CD_List[i].MeterIndex_S;
		//					bt.Step1 = tbs->CD_List[i].StepIndex_S;
		//					bt.Block2 = tbs->CD_List[i].MeterIndex_E;
		//					bt.Step2 = tbs->CD_List[i].StepIndex_E;
		//					bt.BackCount = 1;
		//					g_vBT.emplace_back(bt);
		//					idx = bCount;
		//				}
		//				break;
		//			}
		//		}
		//	}
		//}
		//WriteLog("Back Finish\n");

		for (int i = 0; i < g_vBT.size(); ++i)
		{
			//g_vBAs[g_vBT[i].BeginBackIndex + g_vBT[i].BackCount - 1].Pos2.Step2 = (*blocks)[g_vBT[i].Block2].IndexL2 + g_vBT[i].Step2;
			g_vBAs[g_vBT[i].BeginBackIndex + g_vBT[i].BackCount - 1].Pos2 = FindStepInBlock(g_vBAs[g_vBT[i].BeginBackIndex + g_vBT[i].BackCount - 1].Pos2.Step2, *blocks, 0);

			for (int j = g_vBT[i].BeginBackIndex; j < g_vBT[i].BeginBackIndex + g_vBT[i].BackCount - 2; ++j)
			{
				if (g_vBAs[j].Pos2.Step2 < g_vBAs[j + 1].Pos1.Step2)
				{
					g_vBAs[j].Pos2 = g_vBAs[j + 1].Pos1;
					g_vBAs[j + 1].Pos0 = g_vBAs[j + 1].Pos1;
				}
			}
		}
	}

#pragma region ͨ��ʧ��
	VLCP vLCP[12];
	for (int i = 0; i < g_vLCP.size(); ++i)
	{
		for (int m = 0; m < 12; ++m)
		{
			if (g_vLCP[i].Channel & bits[m])
			{
				LCP lcp = g_vLCP[i];
				lcp.Channel = m;
				vLCP[m].emplace_back(lcp);
			}
		}
	}

	int32_t lastLosingStep = 0;
	LoseCouple lc;
	for (int m = 0; m < 12; ++m)
	{
		for (int i = 0; i < vLCP[m].size(); ++i)
		{
			if (i == 0 || vLCP[m][i].Step - lastLosingStep > 100)
			{
				if (i != 0)
				{
					g_vLC.emplace_back(lc);
				}
				lc.Block1 = vLCP[m][i].Block;
				lc.Step1 = vLCP[m][i].Step;
				lc.Channel = m;
			}
			else if (vLCP[m][i].Step - lastLosingStep <= 100)
			{
				lc.Block2 = vLCP[m][i].Block;
				lc.Step2 = vLCP[m][i].Step;
			}
			if (i == vLCP[m].size() - 1)
			{
				lc.Block2 = vLCP[m][i].Block;
				lc.Step2 = vLCP[m][i].Step;
				g_vLC.emplace_back(lc);
			}
			lastLosingStep = vLCP[m][i].Step;
		}
	}

	for (int i = 0; i < g_vLC.size(); ++i)
	{
		g_vLC[i].Walk1 = FindStepInBlock(g_vLC[i].Step1, g_vBlockHeads, 0).Walk;
		g_vLC[i].Walk2 = FindStepInBlock(g_vLC[i].Step2, g_vBlockHeads, 0).Walk;
	}

	g_vLCP.clear();
	WriteLog("LoseCouple Finish\n");
#pragma endregion


	::remove(pData->strTPB.c_str());
	DeleteFileA(pData->strTPB.c_str());
	g_isQualityJudging = false;
}