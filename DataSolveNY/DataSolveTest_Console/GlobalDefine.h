#ifndef GLOBALDEF_H
#define GLOBALDEF_H

#include "DataEntities.h"
#include "DataEntitiesForNode.h"

extern char g_strAlgName[];

extern char  g_strAlgVersion[];

extern char g_strAlgDescription[];


extern const uint32_t bits[32];


/*
一二次波对应
A1 <=> a2
A2 <=> a1
B1 <=> b2
B2 <=> b1
*/
extern const uint8_t rcs[18];

extern const uint8_t ChannelB2A[];

extern char ChannelNames[13];
extern std::string ChannelNamesB[16];

extern const int chParseOrder[10];

extern const double AngleToRad;

extern int year;
extern int month;
extern int day;

extern uint32_t CurrentYear;
extern uint32_t CurrentMonth;
extern uint32_t CurrentDay;



extern uint16_t	g_iAmpl;

/*
线别：
1：正线
0：站线
*/
extern uint8_t	g_xianbie;
extern uint8_t	g_xingbie;
extern uint8_t  g_gubie;
extern bool		g_direction;
extern uint8_t	g_isTestEqu;

extern int32_t	g_iFileIndex;

extern int32_t	g_iFileCount;



//沿钢轨位置
extern std::map<uint8_t, std::string> g_strWoundAlongPlace;

//截面位置
extern std::map<uint8_t, std::string>  g_strWoundVercPlace;

//伤损状态
extern std::map<uint8_t, std::string>  g_strWoundState;

//细化
extern std::map<uint8_t, std::string>  g_strWoundThinning;

//伤损程度
extern std::map<uint8_t, std::string>  g_strWoundDegree;

//伤损处理情况
extern std::map<uint8_t, std::string>  g_strWoundDeal;

extern std::map<int, int> g_mapWoundTypeCount;

extern FILE* g_pFileWavePoints;

//系统日志
extern FILE* g_pFileLog;

extern int	g_screwholeRange;

//轨颚出波的行
extern uint8_t g_iJawRow[4];
//轨底出波的行
extern uint8_t g_iBottomRow[4];

//端角反射DE高度
extern int	g_iCornerReflectionRowL[4];
//端角反射DE高度
extern int	g_iCornerReflectionRowH[4];

//4种轨型螺孔出波D的行
extern const uint16_t iLuokong_D_Row1_L[4];
extern const uint16_t iLuokong_D_Row1_H[4];

//4种轨型螺孔出波F的行
extern const uint16_t g_iScrewHoleFRowL[4];
extern const uint16_t g_iScrewHoleFRowH[4];
extern const int g_iScrewHorizonDEDistance;

//接头位置C两处出波步进差
extern const float fC_L[4];
extern const float fC_H[4];

//轨颚线高度
extern const uint16_t rail_hDC[4];

//轨底高度
extern const uint16_t rail_uDC[4];

// A, a, B, b, C, D, d, F, c, e, E, G标准增益值
extern double g_StandardGain[CH_N];

//拼图偏移量
extern int g_channelOffset[CH_N];

//4种轨型对应接头/焊缝6孔相对步进
extern const double dScrewDistance[4][6];

//铝热焊焊缝焊筋两侧步进数
extern const int SewLRHLegalStep;

extern int		hour;
extern int		minute;
extern int		second;


extern int		dLines[100];

extern int		g_iTotalBlock;
extern int		g_iBeginBlock;
extern int		g_iBeginStep;
extern int		g_iEndStep;
extern int		g_stepCount;
extern char		tempAccording[1024];

extern uint8_t	bShowWoundDetail;

extern float	pixel;

//螺孔参数
extern int8_t	iLastScrewHoleRow;
extern int8_t	iLastScrewHoleRow2;
extern int8_t	iLastScrewHoleFRow;
extern int8_t	iLastScrewHoleIndex;
extern int32_t	iLastScrewHoleStep;
extern int32_t	iLastScrewHoleRailType;
extern int32_t	iLastScrewHoleFLength;
extern int32_t	iLastScrewHoleFLength2;

//导孔参数
extern int8_t	iLastGuideHoleRow;
extern int8_t	iLastGuideHoleRow2;
extern int8_t	iLastGuideHoleFRow;
extern int8_t	iLastGuideHoleIndex;
extern int32_t	iLastGuideHoleStep;
extern int32_t	iLastGuideHoleRailType;
extern int32_t	iLastGuideHoleFLength;
extern int32_t	iLastGuideHoleFLength2;

extern bool		g_isQualityJudging;
extern bool		g_isPTData;






extern F_HEAD	g_filehead;
extern SCPD*	g_StepPoints;
extern VCO		g_vChannelOffsets;
extern VBDB		g_vBlockHeads;

extern VFR		g_vFR;

extern std::vector<Fork> g_vForks;
extern std::map<int, int> g_vHeavyPos;//key：Step2， value：sumHead
extern std::map<int, int> g_vHeavyPos2;
extern std::map<int, int> g_vHeavyPos3;
extern std::map<uint32_t, Pos> g_vJointPos;
extern std::map<uint32_t, Pos> g_vSewPos;

extern VPM		vExistedPMs;
extern std::map<uint32_t, HolePara>	g_vHoleParas;
extern HolePara						g_LastScrewHole;
extern HolePara						g_LastGuideHole;
extern VINT		g_vReturnSteps;
extern VCR		g_vFLoseBig;
extern VJRW		g_vJWRD;
extern VJRW		g_vJWRE;;


extern std::map<uint32_t, uint8_t> g_vStepsInBackArea;//处理道岔用
extern std::map<uint32_t, uint8_t> g_vStepsInJoint;//处理接头
extern std::map<uint32_t, uint8_t> g_vStepsInSew;//处理焊缝
extern std::map<uint32_t, uint8_t> g_vStepsInK;//处理里程校正
extern std::map<uint32_t, uint8_t> g_vStepsInCouple;//处理失耦
extern std::map<uint32_t, uint8_t> g_vStepsInWound;//处理标伤
extern std::map<uint32_t, PM>	   g_vSlopes;		//变坡点

extern Fork fork;





extern VPM		g_vPMs;
extern VPM		g_vPMs2;//所有标记信息，包含焊缝，校验等
extern VWJ		g_vWounds;
extern VBA		g_vBAs;
extern VER		g_vER;	//单通道出波区域
extern VER		g_vER2;	//所有通道出波区域
extern VLC		g_vLC;
extern VLCP		g_vLCP;
extern VLD		g_vLD;

extern VPM		g_vPMsYOLO;

extern std::vector<BackTotal> g_vBT;


extern uint8_t	g_isDeleteDQFile;

extern int32_t	g_beginSolveMeterIndex;


extern FileSolveInfo*	g_PrevFileInfo;
extern int32_t			g_PrevFileCount;




extern VINT		g_vBlockSolvedFlags;
extern VINT		g_vBlockBackFlag;
extern int		g_iSolvedBlockCount;






extern bool	g_isYoloRuning;
extern std::string g_yolo_cfg_file;
extern std::string g_yolo_names_file;
extern std::string g_yolo_weights_file;
extern std::vector<std::string> g_obj_names;
extern int g_ImgWidth;
extern int g_ImgHeight;

extern float g_threshold;
extern bool	g_isSaveRawImage;
extern bool	g_isYoloPost;
#endif // 
