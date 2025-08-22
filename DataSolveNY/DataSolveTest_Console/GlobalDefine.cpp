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
一二次波对应
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

std::map<uint8_t, char*> g_strGuBieDefines;// = { { 0, "右股" }, { 1, "左股" } };

std::map<uint8_t, char*> g_strXingBieDefines;// = { { 0, "上行" }, { 1, "下行" }, { 2, "单线" } };

std::map<uint16_t, char*> g_strWoundDefines;/* = {
	{ 1, "核伤" }, { 2, "鱼鳞伤" }, { 4, "轨颚水平裂纹" }, { 8, "轨腰水平裂纹" }, { 16, "螺孔斜裂纹" },
	{ 17, "螺孔一象限裂纹" }, { 18, "螺孔2象限裂纹" }, { 19, "螺孔3象限裂纹" }, { 20, "螺孔四象限裂纹" },
	{ 32, "螺孔水平裂纹" }, { 64, "纵向裂纹" }, { 128, "斜裂纹" }, { 256, "轨底横向裂纹" }
};*/

std::map<uint16_t, char*> g_strDegreeDefines;// = { { 0, "疑似" }, { 1, "不到轻伤" }, { 2, "轻伤" }, { 3, "轻发" }, { 4, "重伤" }, { 5, "折断" } };

std::map<uint16_t, char*> g_strWoundPlaceDefines;/* =
{
	{ 1, "轨头踏面中" }, { 2, "轨头内" }, { 4, "轨头中" }, { 8, "轨头外" },
	{ 16, "轨距角" }, { 32, "轨颚内侧" }, { 64, "轨颚外侧" }, { 128, "轨腰" },
	{ 256, "轨底" }, { 512, "轨底角内" }, { 1024, "轨底角外" }
};*/

std::map<uint8_t, char*> g_strCheckStateDefines;// = { { 0, "未复核" }, { 1, "已复核" } };

//当前日期
uint32_t			CurrentYear = 0;
uint32_t			CurrentMonth = 0;
uint32_t			CurrentDay = 0;


/*
是否试块数据
*/
uint8_t		g_isTestEqu;

/*
判伤幅值
*/
uint16_t	g_iAmpl;

/*
行别
*/
uint8_t		g_xingbie;

/*
线别
*/
uint8_t		g_xianbie;

/*
股别
*/
uint8_t		g_gubie;

/*
true：顺里程，false：逆里程
*/
bool		g_direction;

/*
数据质量评估的文件索引
*/
int32_t		g_iFileIndex;

/*
数据质量评估的文件数目
*/
int32_t		g_iFileCount;

int year = 0;
int month = 0;
int day = 0;


//沿钢轨位置
std::map<uint8_t, std::string> g_strWoundAlongPlace;

//截面位置
std::map<uint8_t, std::string>  g_strWoundVercPlace;

//伤损状态
std::map<uint8_t, std::string>  g_strWoundState;

//细化
std::map<uint8_t, std::string>  g_strWoundThinning;

//伤损程度
std::map<uint8_t, std::string>  g_strWoundDegree;

//处理情况
std::map<uint8_t, std::string>  g_strWoundDeal;

//实时算法伤损数量
std::map<int, int> g_mapWoundTypeCount;

//出波点
FILE* g_pFileWavePoints;

//系统日志
FILE* g_pFileLog;

int			g_screwholeRange = 12;

//轨颚线高度(mm)
const uint16_t rail_hDC[4] = { 33, 35, 38, 46 };
//钢轨高度(mm)
const uint16_t rail_uDC[4] = { 140, 152, 176, 192 };

//轨颚线行高
uint8_t g_iJawRow[4] = { 11, 12, 13, 16 };
//轨底线行高
uint8_t g_iBottomRow[4] = { 47, 51, 59, 64 };

//端角反射DE高度
int		g_iCornerReflectionRowL[4] = {12, 13, 16, 18};
//端角反射DE高度
int		g_iCornerReflectionRowH[4] = {16, 18, 19, 21};

//螺孔D通道出波的行
const uint16_t iLuokong_D_Row1_L[4] = { 16, 18, 21, 21 };//4种轨型螺孔出波D的行
const uint16_t iLuokong_D_Row1_H[4] = { 26, 28, 33, 35 };

//螺孔F通道出波的行
const uint16_t g_iScrewHoleFRowL[4] = { 16, 18, 22, 24 };
const uint16_t g_iScrewHoleFRowH[4] = { 23, 26, 28, 30 };
const int g_iScrewHorizonDEDistance = 5;

//接头位置C两处出波步进差
const float fC_L[4] = { 0, 25, 45, 0 };
const float fC_H[4] = { 0, 35, 55, 0 };

// A, a, B, b, C, D, d, F, c, e, E, G标准增益值
double g_StandardGain[CH_N] = { 38.0, 38.0, 38.0, 38.0, 38.0,
47.0, 47.0, 46.0, 38.0, 47.0, 47.0, 46.0 };

int g_channelOffset[CH_N] = { 0 };

//4种轨型对应接头/焊缝6孔相对步进
const double dScrewDistance[4][6] = { { -326, -166, -56, 56, 166, 326 }, { -356, -216, -66, 66, 216, 356 }, { -356, -216, -76, 76, 216, 356 }, { -446, -316, -96, 96, 316, 446 } };

//铝热焊焊缝焊筋两侧步进数
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


//螺孔参数
int8_t		iLastScrewHoleRow = 0;
int8_t		iLastScrewHoleRow2 = 0;
int8_t		iLastScrewHoleFRow = 0;
int8_t		iLastScrewHoleIndex = 0;
int32_t		iLastScrewHoleStep = 0;
int32_t		iLastScrewHoleRailType = 2;
int32_t		iLastScrewHoleFLength = 0;
int32_t		iLastScrewHoleFLength2 = 0;


//导孔参数
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
std::map<int, int> g_vHeavyPos;//key：Step2， value：sumHead
std::map<int, int> g_vHeavyPos2;//key：Step2， value：sumHead
std::map<int, int> g_vHeavyPos3;//key：Step2， value：sumHead

std::map<uint32_t, Pos> g_vJointPos;
std::map<uint32_t, Pos> g_vSewPos;

VPM		vExistedPMs;
std::map<uint32_t, HolePara>	g_vHoleParas;
HolePara						g_LastScrewHole;
HolePara						g_LastGuideHole;
VINT		g_vReturnSteps;
VCR			g_vFLoseBig;

//接头端角反射回波
VJRW		g_vJWRD;
VJRW		g_vJWRE;

std::map<uint32_t, uint8_t> g_vStepsInBackArea;//处理道岔用
std::map<uint32_t, uint8_t> g_vStepsInJoint;	//处理接头
std::map<uint32_t, uint8_t> g_vStepsInSew;	//处理焊缝
std::map<uint32_t, uint8_t> g_vStepsInK;		//处理里程校正
std::map<uint32_t, uint8_t> g_vStepsInCouple;	//处理失耦
std::map<uint32_t, uint8_t> g_vStepsInWound;	//处理标伤
std::map<uint32_t, PM>		g_vSlopes;		//变坡点
	


Fork fork;





VPM         g_vPMs;
VPM			g_vPMs2;//所有标记信息，包含焊缝，校验等
VWJ         g_vWounds;
VBA         g_vBAs;
VER			g_vER;	//单通道出波区域
VER			g_vER2;	//所有通道出波区域
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