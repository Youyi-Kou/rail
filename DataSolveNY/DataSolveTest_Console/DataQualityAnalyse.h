#include "SolveData.h"
#include <stdio.h>
#include <string>

#define MAX_DEFECTDATA_NUM      300000
#define SEARCH_DEFECTDATA_NUM   100
#define MAX_ALARMDATA_NUM       500000
#define SEARCH_ALARMDATA_NUM    500
#define MAX_SIGNDATA_NUM        32000
#define MAX_OVER_SPEED_NUM      10000
#define MAX_EDIT_DEFECT_NUM     1000        // 最大伤损编辑数目
#define SIGN_NUM                11          // 一般标记数目


#define LINE_P                  72          // 步进列的点数
#define LINE_P_V20              68          // 步进列的点数

//-----------------------------------------------
// 一般标记数据结构
//-----------------------------------------------
struct TSignData
{
	uint32_t Number;                          // 序号（从0开始）
	uint32_t MeterIndex_S;                      // 起点所在的米块索引号
	uint16_t StepIndex_S;                       // 起点所在的步进索引号（在米块内的相对位置）
	uint32_t MeterIndex_E;                      // 终点所在的米块索引号
	uint16_t StepIndex_E;                       // 终点所在的步进索引号（在米块内的相对位置）
	uint16_t SignType;                        // 标记类型（0~8，对应Mark的BIT15~BIT7，9 - 里程校正，10 - 耦合不良）
	uint16_t SpecialInfo;                      // 特别信息：耦合不良通道/自定义的编号/回退距离
	uint16_t BackCount;                       // 回退计数
	uint8_t  SwState;                          // 道岔状态
	uint16_t StepCount;                     // 步进计数
	W_D Walk;                               // 实际里程
};
//-----------------------------------------------
// 一般标记统计数据结构
//-----------------------------------------------
struct TSignSta
{
	uint32_t SignCount;                          // 标记总数
	uint32_t SignTypeCount[SIGN_NUM];                  // 各种类型标记数目
	TSignData SDB[MAX_SIGNDATA_NUM];        // 详细标记数组
	std::string SN[SIGN_NUM];                      // 标记名称数组
};





// B超点出波报警位
#define ALL_CHN_ALARM       0xffff
#define A11_CHN_ALARM       0x1
#define A12_CHN_ALARM       0x2
#define A21_CHN_ALARM       0x4
#define A22_CHN_ALARM       0x8
#define B11_CHN_ALARM       0x10
#define B12_CHN_ALARM       0x20
#define B21_CHN_ALARM       0x40
#define B22_CHN_ALARM       0x80
#define C1_CHN_ALARM        0x100
#define C2_CHN_ALARM        0x200
#define D_CHN_ALARM         0x400
#define QD_CHN_ALARM        0x800
#define E_CHN_ALARM         0x1000
#define QE_CHN_ALARM        0x2000
#define F_CHN_ALARM         0x4000
#define QF_CHN_ALARM        0x8000


//-----------------------------------------------
// 报警信息数据结构
//-----------------------------------------------
struct TAlarmData
{
	uint32_t Number;                          // 报警序号
	bool     Enbale;                          // 报警使能标志
	uint32_t FilterNum;                       // 过滤序号

	uint32_t MeterIndex;                      // 报警所在的米块索引号
	uint16_t StepIndex;                       // 报警所在的步进索引号（在米块内的相对位置）
	uint16_t VerPos;                          // 报警所在的深度点位置（从上往下，从0开始）

	//uint32_tMeterIndexEnd;                  // 报警结束点所在的米块索引号
	//uint16_t StepIndexEnd;                  // 报警结束点所在的步进索引号（在米块内的相对位置）

	uint16_t PotCount;                        // 点计数
	uint16_t StepCount;                       // 报警的水平长度（步进）
	uint16_t VerStart;                        // 报警垂直方向起点（B超点）
	uint16_t VerEnd;                          // 报警垂直方向终点（B超点）

	uint8_t alarm_chn;                        // 报警通道
	uint8_t alarm_type;                       // 报警类型

	W_D Walk;                                 // 报警实际里程
	uint16_t Speed;                           // 报警实际速度
	uint8_t  InRailGap;                       // 在轨缝区域内：0 - 不在，1 - ＃
	uint8_t  Check;                           // 复核标记：0-未复核，1-被复核，2-复核

	bool HasCorrDefect;                       // 对应伤损标志
	uint32_t CorrDefectNum;                   // 对应伤损序号
};

//-----------------------------------------------
// 报警统计列表相关数据结构
//-----------------------------------------------
struct TAlarmSta
{
	TAlarmSta();

	void Reset();

	uint8_t AlarmConfirmInterval;                           // 报警确认的步进间隔数
	uint8_t  alarm_type_bak[CH_N - 2];                      // 每个通道报警类型的备份：CH_N-2表示不算G通道，de为一个通道，下同
	uint8_t AlarmInterval[CH_N - 2][4];                     // 每种报警步进间隔
	uint32_t AlarmBitMark[CH_N - 2][4];                     // 各通道不同类型的标志位

	uint16_t CheckAlarmCount;                               // 正在检索的报警数目
	int32_t  CheckAlarmNum[1000];                            // 正在检索的报警索引号数组
	uint8_t  CheckAlarmInv[1000];                             // 正在检索的报警间隔

	uint32_t ChannelAlarmCount[CH_N - 2][4];                // 各个通道不同报警的数目
	uint32_t AlarmDataCount;                                // 报警总数
	TAlarmData ADB[MAX_ALARMDATA_NUM];						// 详细报警数据

	uint32_t FilterChannelAlarmCount[CH_N - 2][4];          // 过滤后各个通道不同报警的数目
	uint32_t FilterAlarmCount;                              // 过滤后的报警数目
	uint32_t FilterAlarmNum[MAX_ALARMDATA_NUM];             // 过滤后的报警索引数组

	uint32_t SearchAlarmCount;                              // 搜索到的报警总数
	uint32_t SearchAlarmNum[SEARCH_ALARMDATA_NUM];          // 搜索到报警索引号

	//string AC[CH_N - 2];									// 报警通道名数组
	//string AT[4];											// 报警类型名数组
};

//-----------------------------------------------
// 伤损信息数据结构
//-----------------------------------------------
struct TDefectData
{
	bool Enbale;                            // 伤损使能标志
	bool Checked;                           // 查看标志

	uint32_t Number;                          // 伤损序号
	uint32_t FilterNum;                       // 过滤序号

	uint32_t MeterIndex;                      // 伤损所在的米块索引号
	uint16_t StepIndex;                       // 伤损所在的步进索引号（在米块内的相对位置）
	uint16_t VerPos;                          // 伤损所在的深度点位置（从上往下，从0开始）

	//uint32_tMeterIndexEnd;                 // 报警结束点所在的米块索引号
	//uint16_t StepIndexEnd;                  // 报警结束点所在的步进索引号（在米块内的相对位置）

	uint16_t StepCount;                     // 伤损的水平长度（步进）
	uint16_t VerStart;                      // 伤损垂直方向起点（B超点）
	uint16_t VerEnd;                        // 伤损垂直方向终点（B超点）

	uint8_t  DefectType;                    // 标伤类型：1 - 人工；2 - 回放；3 - 自动
	uint8_t  DefectDegree;                  // 伤损程度：（自动）1 - 一级；2 - 二级；3 - 三级（最严重）
	//           （其他）1 - 不到轻伤，2 - 轻伤，3 - 重伤，4 - 折断
	uint32_t	SectionNum;		// 单位编号 5个BCD
	uint16_t	LineNum;		// 线编号4个BCD
	uint8_t	LineWay;		// 行别 0：单线、1：上，2：下行
	uint8_t	LineType;		// 线别 0: 站线, 1: 正线
	uint8_t	LeftRight;		// 左右股 0: 右 1：左
	uint16_t	swNum;			// 道岔号，4个BCD表示，一般3位
	W_D     Walk;           // 实际里程
	uint16_t	TrackNum;		// 股道编号（站线）4个BCD
	uint32_t	StationNum;		// 车站编号（站线）5个BCD
	uint32_t FindData;       // 发现日期：年（4位）、月（2位）、日（2位）
	uint32_t	FindTime;		// 发现时间：时（2位）、分（2位）、秒（2位）
	uint16_t	FindUser;		// 发现人工号,4个BCD
	uint16_t	DefectNum;		// 伤损编号4个BCD
	char    DeviceNum[8];   // 仪器编号
	uint16_t	RailNum;		// 钢轨编号,4个BCD
	int8_t    RailType;
	uint8_t   Dispose;        // 现场处理：0未处理，1钻孔上夹板，2鼓包上夹，3原位复焊

	uint32_t InputData;      // 导入日期：年（4位）、月（2位）、日（2位）
	uint32_t	InputTime;		// 导入时间：时（2位）、分（位）、秒（位）
	uint16_t	InputUser;		// 导入人工号,4个BCD
};
//-----------------------------------------------
// 伤损统计列表相关数据结构
//-----------------------------------------------
struct TDefectSta
{
	TDefectSta();

	void Reset();
	TDefectData DefectDataBak;                              // 伤损数据备份（用于检索）

	uint16_t CheckDefectCount;                                // 正在检索的伤损数目
	uint32_t CheckDefectNum[1000];                             // 正在检索的伤损索引号数组
	uint8_t CheckDefectInv[1000];                              // 正在检索的伤损间隔

	uint32_t TypeDefectCount[3][4];                           // 各类型的数目：[类型][程度]
	uint32_t DefectDataCount;                                 // 伤损总数
	TDefectData DDB[MAX_DEFECTDATA_NUM];                    // 详细伤损数据

	uint32_t SourceTypeDefectCount[3][4];                     // 源各类型的数目：[类型][程度]
	uint32_t SourceDefectDataCount;                           // 源伤损总数
	TDefectData SDDB[MAX_DEFECTDATA_NUM];                   // 源详细伤损数据

	uint32_t FilterDefectCount;                               // 过滤后的报警数目
	uint32_t FilterDefectNum[MAX_DEFECTDATA_NUM];             // 过滤后的报警索引数组

	uint32_t SearchDefectCount;                               // 搜索到的伤损总数
	uint32_t SearchDefectNum[MAX_DEFECTDATA_NUM];             // 搜索到伤损索引号

	std::string DT[3];											// 伤损类型描述数组
	std::string DD[3];											// 伤损程度描述数组
};


//-----------------------------------------------
// 回退列表数据结构
//-----------------------------------------------
struct TCheckbackData
{
	UINT32 Index;                               // 序号（从0开始）

	UINT32 MeterIndex_S;                        // 起点所在的米块索引号
	UINT16 StepIndex_S;                         // 起点所在的步进索引号（在米块内的相对位置）

	UINT32 ListIndex;                           // 标记列表对应序号（从0开始）
	UINT16 CheckCount;                          // 检查点内回退计数（从1开始）

	UINT32 MeterIndex_E;                        // 终点所在的米块索引号
	UINT16 StepIndex_E;                         // 终点所在的步进索引号（在米块内的相对位置）

	UINT16 StepCount;                           // 步进计数
};
//-----------------------------------------------
// 回退统计数据结构
//-----------------------------------------------
struct TCheckbackSta
{
	UINT32 DataCount;                               // 回退记录计数
	UINT32 OrgDataCount;                            // 被复核记录计数


	TCheckbackData Org_CD_List[MAX_SIGNDATA_NUM];     // 被复核记录列表
	TCheckbackData CD_List[MAX_SIGNDATA_NUM];         // 复核记录列表
};

const float HOR_POINT_FOR_LENTH = 2.7f;
const float VER_POINT_FOR_LENTH = 3.0f;


#define MAX_AL_DE_STEP_COUNT    300         // 最大报警、伤损步进宽度



typedef struct _QualityData
{
	F_HEAD		*fileHead;
	VBDB		*vBlockHeads;
	std::string	strTPB;
	uint32_t	szFileB;
	TAlarmSta	*tas;
	TDefectSta	*tds;
	TSignSta	*tss;
	TCheckbackSta *tbs;
}QualityData;


//TAlarmSta AlarmSta;             // 报警相关信息
//TDefectSta DefectSta;           // 伤损相关信息

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//名称：CheckDefectDelete
//功能：删除伤损记录
//输入：num - 伤损信息序号
//输出：
//返回：
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckDefectDelete(TDefectSta *tds, UINT32 num);

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//名称：CheckDefectMerge
//功能：合并两个伤损记录
//输入：num_m - 主要伤损信息序号
//      num_s - 次要伤损信息序号
//输出：
//返回：
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckDefectMerge(TDefectSta *tds, UINT32 num_m, UINT32 num_s);


//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//名称：CheckDefectUpdata
//功能：更新一个伤损记录
//输入：tdd - 伤损信息指针
//      StepInv - 步进间隔
//      VerPos - 深度点
//      DefectType - 伤损类型
//      DefectDegree - 伤损程度
//      DefectNum - 伤损编号
//输出：tdd - 伤损信息指针
//返回：
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckDefectUpdata(TDefectData *tdd, UINT16 StepInv, UINT16 VerPos, UINT8 DefectType, UINT8 DefectDegree, UINT16 DefectNum, UINT8 Dispose);


//---------------------------------------------------------------------------
// V2.0数据版本的添加一个伤损记录到伤损列表
//---------------------------------------------------------------------------
void CheckDefectAdd(F_HEAD FileHead, BLOCK MeterHead, UINT32 MeterIndex, UINT16 StepIndex, UINT16 VerPos, UINT8 DefectType, UINT8 DefectDegree, UINT16 DefectNum, UINT8 Dispose, TDefectSta *tds);


/*
if (LineMark.Mark & QIAO)    found[0] = true;            // 桥梁
if (LineMark.Mark & CURVE)   found[1] = true;            // 曲线
if (LineMark.Mark & FORK)    found[2] = true;            // 道岔
if (LineMark.Mark & SEW2)    found[3] = true;            // 手动标志
if (LineMark.Mark & SEW)     found[4] = true;            // 焊缝
found[5] = true;	//回退
if (LineMark.Mark & START)   found[6] = true;            // 上道
if (LineMark.Mark & CHECK)   found[7] = true;            // 校验
if (LineMark.Mark & SP_P)     found[8] = true;           // 超速点
if (LineMark.Mark & CK_KM)   found[9] = true;            // 里程校对
if (LineMark.Couple & BIT15)   found[10] = true;         // 通道耦合
*/
void GetAlarmDefectSignData(void* threadData);