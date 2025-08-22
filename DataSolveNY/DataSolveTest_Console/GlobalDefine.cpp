#include "stdafx.h"
#include "GlobalDefine.h"

char g_strAlgName[] = "DataSolveY";

char g_strAlgVersion[] = "2.5.1.1";

char g_strAlgDescription[] = "Combine old algorithm(DataSolve) with YOLO v4";

const uint32_t bits[32] =
{ BIT0, BIT1, BIT2, BIT3, BIT4, BIT5, BIT6, BIT7, BIT8, BIT9,
BIT10, BIT11, BIT12, BIT13, BIT14, BIT15, BIT16, BIT17, BIT18, BIT19,
BIT20, BIT21, BIT22, BIT23, BIT24, BIT25, BIT26, BIT27, BIT28, BIT29,
BIT30, BIT31
};


/*
һ���β���Ӧ
A1 <=> a2
A2 <=> a1
B1 <=> b2
B2 <=> b1
*/
const uint8_t rcs[18] = { CH_a1, CH_a1, CH_A1, CH_A1, CH_b1, CH_b1, CH_B1, CH_B1, CH_c, CH_C, CH_E, CH_e, CH_D, CH_d, CH_G, CH_F, CH_GL, CH_FL };

const uint8_t ChannelB2A[] = { ACH_A, ACH_A, ACH_a, ACH_a, ACH_B, ACH_B, ACH_b, ACH_b, ACH_C, ACH_c, ACH_D, ACH_d, ACH_E, ACH_e, ACH_F, ACH_G, ACH_F, ACH_G};

const double AngleToRad = 3.1415926 / 180;

char ChannelNames[13] = "AaBbCDdFceEG";

std::string ChannelNamesB[16] = { "A1", "A2", "a1", "a2", "B1", "B2", "b1", "b2", "C", "c", "D", "d", "E", "e", "F", "G" };

const int chParseOrder[10] = { CH_c, CH_a1, CH_A1, CH_b1, CH_B1, CH_C, CH_D, CH_E, CH_F, CH_G };

std::map<uint16_t, char*> g_strMarkDefines;

std::map<uint8_t, char*> g_strGuBieDefines;// = { { 0, "�ҹ�" }, { 1, "���" } };

std::map<uint8_t, char*> g_strXingBieDefines;// = { { 0, "����" }, { 1, "����" }, { 2, "����" } };

std::map<uint16_t, char*> g_strWoundDefines;/* = {
	{ 1, "����" }, { 2, "������" }, { 4, "���ˮƽ����" }, { 8, "����ˮƽ����" }, { 16, "�ݿ�б����" },
	{ 17, "�ݿ�һ��������" }, { 18, "�ݿ�2��������" }, { 19, "�ݿ�3��������" }, { 20, "�ݿ�����������" },
	{ 32, "�ݿ�ˮƽ����" }, { 64, "��������" }, { 128, "б����" }, { 256, "��׺�������" }
};*/

std::map<uint16_t, char*> g_strDegreeDefines;// = { { 0, "����" }, { 1, "��������" }, { 2, "����" }, { 3, "�ᷢ" }, { 4, "����" }, { 5, "�۶�" } };

std::map<uint16_t, char*> g_strWoundPlaceDefines;/* =
{
	{ 1, "��ͷ̤����" }, { 2, "��ͷ��" }, { 4, "��ͷ��" }, { 8, "��ͷ��" },
	{ 16, "����" }, { 32, "����ڲ�" }, { 64, "������" }, { 128, "����" },
	{ 256, "���" }, { 512, "��׽���" }, { 1024, "��׽���" }
};*/

std::map<uint8_t, char*> g_strCheckStateDefines;// = { { 0, "δ����" }, { 1, "�Ѹ���" } };

//��ǰ����
uint32_t			CurrentYear = 0;
uint32_t			CurrentMonth = 0;
uint32_t			CurrentDay = 0;


/*
�Ƿ��Կ�����
*/
uint8_t		g_isTestEqu;

/*
���˷�ֵ
*/
uint16_t	g_iAmpl;

/*
�б�
*/
uint8_t		g_xingbie;

/*
�߱�
*/
uint8_t		g_xianbie;

/*
�ɱ�
*/
uint8_t		g_gubie;

/*
true��˳��̣�false�������
*/
bool		g_direction;

/*
���������������ļ�����
*/
int32_t		g_iFileIndex;

/*
���������������ļ���Ŀ
*/
int32_t		g_iFileCount;

int year = 0;
int month = 0;
int day = 0;


//�ظֹ�λ��
std::map<uint8_t, std::string> g_strWoundAlongPlace;

//����λ��
std::map<uint8_t, std::string>  g_strWoundVercPlace;

//����״̬
std::map<uint8_t, std::string>  g_strWoundState;

//ϸ��
std::map<uint8_t, std::string>  g_strWoundThinning;

//����̶�
std::map<uint8_t, std::string>  g_strWoundDegree;

//�������
std::map<uint8_t, std::string>  g_strWoundDeal;

//ʵʱ�㷨��������
std::map<int, int> g_mapWoundTypeCount;

//������
FILE* g_pFileWavePoints;

//ϵͳ��־
FILE* g_pFileLog;

int			g_screwholeRange = 12;

//����߸߶�(mm)
const uint16_t rail_hDC[4] = { 33, 35, 38, 46 };
//�ֹ�߶�(mm)
const uint16_t rail_uDC[4] = { 140, 152, 176, 192 };

//������и�
uint8_t g_iJawRow[4] = { 11, 12, 13, 16 };
//������и�
uint8_t g_iBottomRow[4] = { 47, 51, 59, 64 };

//�˽Ƿ���DE�߶�
int		g_iCornerReflectionRowL[4] = {12, 13, 16, 18};
//�˽Ƿ���DE�߶�
int		g_iCornerReflectionRowH[4] = {16, 18, 19, 21};

//�ݿ�Dͨ����������
const uint16_t iLuokong_D_Row1_L[4] = { 16, 18, 21, 21 };//4�ֹ����ݿ׳���D����
const uint16_t iLuokong_D_Row1_H[4] = { 26, 28, 33, 35 };

//�ݿ�Fͨ����������
const uint16_t g_iScrewHoleFRowL[4] = { 16, 18, 22, 24 };
const uint16_t g_iScrewHoleFRowH[4] = { 23, 26, 28, 30 };
const int g_iScrewHorizonDEDistance = 5;

//��ͷλ��C��������������
const float fC_L[4] = { 0, 25, 45, 0 };
const float fC_H[4] = { 0, 35, 55, 0 };

// A, a, B, b, C, D, d, F, c, e, E, G��׼����ֵ
double g_StandardGain[CH_N] = { 38.0, 38.0, 38.0, 38.0, 38.0,
47.0, 47.0, 46.0, 38.0, 47.0, 47.0, 46.0 };

int g_channelOffset[CH_N] = { 0 };

//4�ֹ��Ͷ�Ӧ��ͷ/����6����Բ���
const double dScrewDistance[4][6] = { { -326, -166, -56, 56, 166, 326 }, { -356, -216, -66, 66, 216, 356 }, { -356, -216, -76, 76, 216, 356 }, { -446, -316, -96, 96, 316, 446 } };

//���Ⱥ����캸�����ಽ����
const int SewLRHLegalStep = 20;

int			hour = 0;
int			minute = 0;
int			second = 0;
int			dLines[100] = { 0 };

int			g_iTotalBlock = 0;
int			g_iBeginBlock = 0;
int			g_iBeginStep = 0;
int			g_iEndStep = 0;
int			g_stepCount = 0;
char		tempAccording[1024];

uint8_t		bShowWoundDetail = 0;

float		pixel = 58.0f / 7.0f;


//�ݿײ���
int8_t		iLastScrewHoleRow = 0;
int8_t		iLastScrewHoleRow2 = 0;
int8_t		iLastScrewHoleFRow = 0;
int8_t		iLastScrewHoleIndex = 0;
int32_t		iLastScrewHoleStep = 0;
int32_t		iLastScrewHoleRailType = 2;
int32_t		iLastScrewHoleFLength = 0;
int32_t		iLastScrewHoleFLength2 = 0;


//���ײ���
int8_t		iLastGuideHoleRow = 0;
int8_t		iLastGuideHoleRow2 = 0;
int8_t		iLastGuideHoleFRow = 0;
int8_t		iLastGuideHoleIndex = 0;
int32_t		iLastGuideHoleStep = 0;
int32_t		iLastGuideHoleRailType = 2;
int32_t		iLastGuideHoleFLength = 0;
int32_t		iLastGuideHoleFLength2 = 0;

bool		g_isQualityJudging = false;

bool		g_isPTData = false;




F_HEAD	g_filehead;
SCPD*	g_StepPoints = NULL;
VCO		g_vChannelOffsets;
VBDB    g_vBlockHeads;

VFR		g_vFR;

std::vector<Fork> g_vForks;
std::map<int, int> g_vHeavyPos;//key��Step2�� value��sumHead
std::map<int, int> g_vHeavyPos2;//key��Step2�� value��sumHead
std::map<int, int> g_vHeavyPos3;//key��Step2�� value��sumHead

std::map<uint32_t, Pos> g_vJointPos;
std::map<uint32_t, Pos> g_vSewPos;

VPM		vExistedPMs;
std::map<uint32_t, HolePara>	g_vHoleParas;
HolePara						g_LastScrewHole;
HolePara						g_LastGuideHole;
VINT		g_vReturnSteps;
VCR			g_vFLoseBig;

//��ͷ�˽Ƿ���ز�
VJRW		g_vJWRD;
VJRW		g_vJWRE;

std::map<uint32_t, uint8_t> g_vStepsInBackArea;//���������
std::map<uint32_t, uint8_t> g_vStepsInJoint;	//�����ͷ
std::map<uint32_t, uint8_t> g_vStepsInSew;	//������
std::map<uint32_t, uint8_t> g_vStepsInK;		//�������У��
std::map<uint32_t, uint8_t> g_vStepsInCouple;	//����ʧ��
std::map<uint32_t, uint8_t> g_vStepsInWound;	//�������
std::map<uint32_t, PM>		g_vSlopes;		//���µ�
	


Fork fork;





VPM         g_vPMs;
VPM			g_vPMs2;//���б����Ϣ���������죬У���
VWJ         g_vWounds;
VBA         g_vBAs;
VER			g_vER;	//��ͨ����������
VER			g_vER2;	//����ͨ����������
VLC			g_vLC;
VLCP		g_vLCP;
VLD			g_vLD;
VPM			g_vPMsYOLO;

std::vector<BackTotal> g_vBT;


uint8_t		g_isDeleteDQFile = 1;

int32_t		g_beginSolveMeterIndex;


FileSolveInfo*	g_PrevFileInfo;
int32_t			g_PrevFileCount;


VINT		g_vBlockSolvedFlags;
VINT		g_vBlockBackFlag;
int			g_iSolvedBlockCount = 0;


bool	g_isYoloRuning = false;
std::string g_yolo_cfg_file;
std::string g_yolo_names_file;
std::string g_yolo_weights_file;
std::vector<std::string> g_obj_names;
int		g_ImgWidth = 1920;
int		g_ImgHeight = 330;
float	g_threshold = 0.5;
bool	g_isSaveRawImage = false;
bool	g_isYoloPost = false;