#ifndef ENTITIES_H
#define ENTITIES_H

#include "DataDefine.h"

struct FILE_TIME {
	uint32_t dwLowDateTime;
	uint32_t dwHighDateTime;
};

struct DGC {
	int16_t	h[6];			// 单位(点)，最大，显示时换算成深度
	int16_t	gS[6];			// 节点起始位置增益，单位/2db
	int16_t	gE[6];			// 节点结束位置增益，单位/2db
};


struct STATUS {
	FILE_TIME	creatT;		// 创建文件的时间
	FILE_TIME	ExitT;		// 关机时间
	int16_t		ExitKm;		// 关机里程
	int16_t		CreatKm;	// 创建里程
};

// 里程
struct W_D{
	int32_t	m;   //米
	int16_t	mm;   //毫米
	int16_t	Km;   //千米
};



//-----------------------------------------------
// 所有报警或判伤的标准
//-----------------------------------------------
typedef struct _STANDARD {
	uint8_t	speed;		// 速度上限*km/h
	uint8_t	speed_D;	// 超速距离低5位为超速距离，单位m，高3位保留
	uint8_t	coup;		// 耦合度，耦合不良的评判，分为1~5级（5级为最严格）
	uint8_t	coup_D;		// 失耦阈值，失偶超过此距离，报警，单位0.1m
	uint8_t	abrasion;	// 磨耗阈值mm,超过报警，保留
	uint8_t	QAB_M;		// 70度轨面鱼鳞门内的回波移动报警距离,单位/15us
	uint8_t	QDE_M;		// 37度轨底锈蚀门内的回波移动报警距离,单位/15us
	uint8_t	QDE1_Al;	// 37度螺孔交替波间距（垂直距离）
	uint8_t	QDE1_D;		// 37度螺孔双波报警间距
	uint8_t	QF1_D;		// 0度螺孔双波报警间距
	uint8_t	QF1_AL;		// 0度螺孔交替波间距（垂直）
	uint8_t	wDouble0_D;	// 0度螺孔双波伤损间距
	uint8_t	wDouble37_D;// 37度螺孔双波伤损间距
	uint8_t	rev1;		//
	uint8_t	rev2;		//保留
	uint8_t	rev3;		//
	uint8_t	revS[4];	//
	uint8_t	rev4;		//
	uint8_t	lose_Step;	// 失波距离控制
	uint16_t	rev5;
}STD;

enum  _ALARM_OC {
	A_OC = BIT1,	// 报警音频开关，开，关
	A_SPECH = BIT2,	// 语音提示
	A_0LOS = BIT3,	// 0度失波报警方式，0：正常失波，1：距离控制(步进在管理员设置中)
	A_SEW = BIT4,	// 轨逢回波，0-关，1-开,关表示轨缝附近出波或失波不报警。
	A_QDE = BIT5,	// QDE出波
	A_QAB = BIT6,	// QAB出波
	A_ALT = BIT7,	// 交替波
	A_SPEED = BIT8,	// 超速
	A_COUPLE = BIT9,// 耦合
	//	A_FG = BIT15,	// 0度穿透失波通道选择
};

enum  _DETECT_ELSE {
	R_LS = BIT0,		// 长短轨1长，短
	R_BS = BIT1,		// 抑制大小，1-大，0-小
	C_LR = BIT2,		// 车型，1-右手车，0-左手车
	D_DUB = BIT3,		// 是否双波识别
	D_CB = BIT4,		// 是否组合伤损识别
	D_MOV = BIT5,		// 是否移动伤损识别
	D_TYP = BIT6,		// 是否轨型识别
};

// 检测设置
typedef struct _DETECT_PARAM {
	uint16_t	Alarm;			// 对照_ALARM_OC
	uint16_t	Identify;		// 对照_DETECT_ELSE
}DETECT;

//-----------------------------------------------
// 角度及偏角
//-----------------------------------------------
typedef struct _ANGLE {
	int16_t	Refrac;		// 折射角，单位0.5度
	int16_t	Angle;		// 斜70度的偏斜角，单位0.5度
}ANGLE;

//-----------------------------------------------
// 上道设置
//-----------------------------------------------
typedef struct _TRACK_PARAM {
	uint8_t	lineType;			// 上道类型0: 站线, 1: 正线
	uint8_t	lineWay;			// 行别0：单线、1：上，2：下行
	uint8_t	leftRight;			// 左右股0: 右1：左
	uint8_t	WalkWay;			// 行走方向0: 逆里程, 1: 顺里程
	uint8_t	railType;			// 轨型0~3依次表示43, 50, 60, 75
	uint8_t	rev1;
	uint8_t	rev2;
	uint8_t	rev3;

	uint16_t	railNum;		// 钢轨编号，4个BCD
	uint16_t	plyNum;			// 股道编号（站线）4个BCD
	uint16_t	KMm[2];			// 里程， KMm[0]：Km, KMm[1]：m
	uint32_t	stationNum;		// 车站编号（站线）5个BCD

	uint32_t	sectionNum;		// 单位编号5个BCD
	uint16_t	regionNum;		// 工区4个BCD
	uint16_t	lineNum;		// 线编号4个BCD
	uint16_t	teamNum;		// 班组4个BCD
	uint16_t	workerNum;		// 工号4个BCD
}TRACK_P;


//-----------------------------------------------
// 门
//-----------------------------------------------
typedef struct _DOOR {
	int16_t	start;		// 方门的起始位置，用声程表示，单位/15us
	int16_t	end;		// 方门的结束位置
	int16_t	isOn;		// 方门开关
}DOOR;


//-----------------------------------------------
// 出波点数据
//-----------------------------------------------
typedef struct _B_POINT {
	uint16_t	Draw1;			// 正常画图
	uint16_t	Draw2;			// 3dB画图
	uint16_t	Alarm;
	uint8_t		Weight;			// 权值只用低位，其他保留,
	uint8_t		Wound;			// 伤损标记（_B_WOUND_DEF）BIT7不能用！！*****
}B_POINT;



typedef struct _B_MARK {
	uint32_t	Mark;
	uint16_t	Couple;		// BIT0~BIT11: 12个通道的耦合情况，BIT15: 仪器判断耦合不良(表示有一个通道欧合不良已经达到一定距离)，
	uint16_t	rev;		// 保留，Bit15不可用！！*****
}B_MARK;

enum _B_MARK_DEF {
	ALARM_M = BIT0,				// 出波报警点
	LOSE_0 = BIT1,				// 0度失波
	LOSE_37 = BIT2,				// 37度失波
	SP_P = BIT3,				// 超速点>
	SEW = BIT4,					// 轨缝#
	BACK_P = BIT5,				// 回退@
	START = BIT6,				// 上道S
	CHECK = BIT7,				// 效验C

	CK_KM = BIT9,				// 里程校对

	SEW2 = BIT16,				// 手动焊缝轨缝*
	FORK = BIT17,				// 道岔Y
	CURVE = BIT18,				// 曲线$
	QIAO = BIT19,				// 桥梁Q
	SEW_N = BIT20 | BIT21 | BIT22 | BIT23,// 手动*标记的编号，比如*1,*2....
	CURVE_N = BIT24,            // 曲线上下股， 0：上 1：下
	TUNNEL = BIT31              // 隧道
};

typedef struct _B_WOUND {
	// BIT0~BIT7各种伤损的标记参照_B_WOUND_DEF, BIT8~BIT11伤损程度(人工设定，仪器的伤不设定), BIT12~BIT15作为其他备注（1：37度穿透伤，2：0度失波伤）
	uint16_t	W_Mark;	

	// 4个BCD码，从低到高依次表示：长度位置、截面位置、伤损状态、细化
	uint16_t	W_Code;		

	// 低4位表示处理情况，0未处理，1钻孔上夹板，2鼓包上夹，3原位复焊。其他保留，Bit31不可用！！*****
	uint32_t	Other;			
}B_WOUND;

enum _B_WOUND_DEF {
	W_MAN = BIT0,			// 手动标伤
	W_D_SER = BIT1,			// 多通道组合重伤，权值大于7
	W_D_SLI = BIT2,			// 多通道组合轻伤，权值<7
	W_SIG = BIT3,			// 自动单通道伤损
	W_HOL = BIT4,			// 螺孔双波伤\交替波\，记录为Z_WOUND，伤损等级为轻伤
	W_ADD = BIT6			// 回放添加
};


//-----------------------------------------------
// 仪器所有参数
//-----------------------------------------------
typedef struct _ALL_P {
	//DOOR	Doors[4][DOR_NUM];	// 小门
	DOOR	Doors[4 * DOR_NUM];	// 小门
	uint8_t	Gain[CH_N];			// 手动增益
	DETECT	DetectSet;			// 探测设置
	STD		Standard;			// 智能探伤报警标准
	uint16_t	fork;			// 道岔号，4个BCD
	uint16_t	rev16;			// 保留
	uint8_t	disMode;			// 显示模式
	uint8_t	volume;				// 音量16级
	uint8_t	autoGain;			// BIT0~1是否自动增益，BIT2~3探头选择(普通/复合材料)，其他保留
	uint8_t	BitState;			// BIT0道岔右左，BIT1道岔直曲
}ALL_P;

//--------------------------------------------------------------
// ALL_P2
// 此参数只存于文件头，不存米块头
//--------------------------------------------------------------
typedef struct _ALL_P2 {
	ANGLE	Angle[CH_N];	// 探头角度及偏角
	int16_t	Place[CH_N];	// 探头位置
	int16_t	Zero[CH_N];		// 探头零点, 单位/15us
	int16_t	Restr[CH_N];	// 抑制，小于此值的回波不显示
	int16_t	Trig[CH_N];		// 阈值，小于此值的回波不画B超。这个值的一半 = 抑制值
	//DOOR	Gate[4][GA_NUM];// 大门, 单位/15us
	DOOR	Gate[4*GA_NUM];// 大门, 单位/15us
	//DGC		Dgc[4][4];		// 定制DGC，保留
	DGC		Dgc[4*4];		// 定制DGC，保留
	TRACK_P	TrackSet;		// 上道设置
	STATUS	dev_S;			// 设备状态，保留
	int16_t	S_factor;		// 里程系数
	uint16_t	rev;
}ALL_P2;


//-----------------------------------------------
// 文件头
//-----------------------------------------------
typedef struct _F_HEAD
{
	uint32_t		CheckSum;
	char			Name[32];			// "SZT-800_RailTrack_Bchao"
	char			DataVerS[8];		// 数据版本，DataVerS[8] = {'2', '.', '4', 0, }; V2.2之前的数据结构是2015年7月之前的
	char			SoftVerS[8];		// 软件版本
	char			FPGAVerS[8];		// FPGA版本
	char			DeviceNum[8];		// 机器编号
	uint32_t		Reserved[30];		// 保留
	W_D				startKm;
	uint32_t		startT;				// 开始作业时间：D23~D16时,D15~D8分,D7~D0秒
	uint32_t		startD;				// 年月日
	uint32_t		endT;				// 结束作业时间：时分秒
	uint32_t		endD;				// 结束作业时间：年月日
	int32_t			distance;			// 总共行走多少距离, 步进
	uint32_t		block;				// 总共多少米块,
	uint32_t		alarm;				// 总共多少报警,保留
	uint32_t		wound;				// 总共多少伤损,保留
	uint32_t		run;				// 超速次数
	uint32_t		orderNum;			// 探伤命令号,暂定8位BCD,保留
	float			step;				// 每个步进多少毫米
	W_D				endKm;				// 结束里程
	ALL_P			deviceP;			// 仪器参数
	ALL_P2			deviceP2;			// 仪器参数2
	//DGC				DGC_Sd[R_N][4];		// 所有DGC节点位置, 幅度,4种轨型，每种轨型4条，保留
	//DGC				DGC_St[R_N][4];		// 强补偿，目前不用，保留。
	DGC				DGC_Sd[R_N*4];		// 所有DGC节点位置, 幅度,4种轨型，每种轨型4条，保留
	DGC				DGC_St[R_N*4];		// 强补偿，目前不用，保留。
	uint8_t			zero_AD[2];			// AD零点.
	uint8_t			rev8[12];				// 保留

	uint8_t			IsAddition;             // 附加信息标志（BIT0 - 附加伤损， BIT1 - 用户登录）
	uint8_t			FileType;               // 文件类型 0 - 原始；1 - 备份；2 - 截取
	uint32_t		AdditionPosition;       // 附加信息开始位置
	uint32_t		ReplayRecardPosition;   // 操作记录开始位置
	uint32_t		rev32[390];				// 保留,总共凑齐4k----------改
}F_HEAD;

//----------------------------------------------------------------------------------------
//tpA米块头
//----------------------------------------------------------------------------------------
typedef struct _BLOCK_WAVE {
	uint32_t	symbol;			// 米块标志 0xFFFFFFFF 所以要保证所有的数据小于0xFFFFFFFF
	int32_t		indexL;			// 米块起始步进，和B超中的步进相对应的
	uint32_t	len;			// 此米块数据压缩后多少字节

	uint16_t	checkD;			// 米块压缩后数据的校验和
	uint16_t	fNum;			// 此米块多少帧A超

	uint8_t		log;			// 操作日志v2.2
	uint8_t		rev;			// 保留
	uint16_t	rev1;			// 保留
}BLOCK_A;


//----------------------------------------------------------------------------------------
//tpB米块头，目前B超数据采用位压缩，米块头size应是的整数倍！！！！！
//----------------------------------------------------------------------------------------
/*
uint8_t gpsInfor[32];
gpsInfor[0]表示GPS定位状态（GPRMC） A有效V无效
gpsInfor[1]~[2]:有效卫星数量，[1]个位，[2]十位
gpsInfor[3]~[14]，纬度如："N30.42.6304";
gpsInfor[15]~[27]，经度，如"E104.02.6090"
gpsInfor[28]~[31]，UTC时间，表示：[28][29]时时，[30][31]日日
*/
typedef struct _BLOCK {
	uint32_t	symbol;			// 米块标志0xFFFFFFFF,所以要保证所有的其他数据小于0xFFFFFFFF，
	uint16_t	checkSum;		// 米块头效验和,保留, 位置不能改
	uint16_t	checkD;			// 米块数据的效验和
	int32_t		indexL;			// 当前步进
	uint32_t	len;			// 此米块数据压缩后字节数
	uint32_t	time;			// 时间：高位至低位每8位依次为：日、时、分、秒
	uint16_t	row;			// 此米块多少个步进
	uint16_t	railNum;		// 钢轨编号,4个BCD码表示
	uint16_t	swNum;			// 道岔号，4个BCD表示，一般3位
	uint16_t	user;			// 工号,4个BCD
	DETECT		detectSet;		// 探测设置
	//uint16_t	door[DOR_NUM][2];	// 小门设置，DOR_NUM=7，door[x][0]的最高位为on/off
	uint16_t	door[DOR_NUM*2];	// 小门设置，DOR_NUM=7，door[x][0]的最高位为on/off
	uint16_t	addVal[CH_N];		// 累计值，CH_N=12 de通道不用，保留
	int8_t		probOff[CH_N];		// 探头位置偏移
	W_D			walk;			// 即时里程
	int16_t		speed;			// 速度，单位0.1km/h
	uint8_t		gpsInfor[32];	// GPS经纬度、UTC时间，字符串
	uint8_t		gain[CH_N];		// 12通道，0~160表示0~80.0dB，单位0.5dB。
	uint8_t		railType;		// BIT0~1: 轨型(0为43轨，1-50，2-60，3-75), BIT4:0逆里程、1顺里程，BIT5:0右股、1左股，BIT6~7：单线、上行、下行，其他预留
	uint8_t		railH;			// 当前轨高mm
	uint8_t		autoGain;		// BIT0~1:是否自动增益, BIT2~3探头选择，其他预留
	uint8_t		log;			// 操作日志
	uint8_t		volume;			// BIT0~6：音量32级，其他预留,高位可做显示模式
	uint8_t		BitS;			// BIT0:道岔右左，BIT1：道岔直曲，BIT2:道岔后前
	uint16_t	plyNum;			// 站线的股道编号
}BLOCK;

const uint32_t szBlock = sizeof(BLOCK);

typedef struct _TIME_ANALYSIS
{
	uint8_t		second;
	uint8_t		minute;
	uint16_t	hour;
	uint8_t		day;
	uint8_t		month;
	uint16_t	year;
	struct _TIME_ANALYSIS *p;
}TIME_ANALYSIS;


//-----------------------------------------------
// A超每帧压缩包
//-----------------------------------------------
typedef struct _FRAME{
	//A_PARAM		param;					// A超需要的参数
	int16_t     ShiftStep;              // 当前帧对应步进偏移量
	uint16_t	wave[CH_N * 512];	// A超波形数据 512 * 12
}FRAME;

/************************************************************************/
/*                           导出数据格式                               */
/************************************************************************/

//B超中每行数据
typedef struct _B_RowData
{
	uint8_t		Row;			//行
	B_POINT		Point;			//画图信息
}B_RowData;

//B超每步进数据
typedef struct _B_Step
{
	//总的步进数
	int			Step;

	//总的步进数2，回退不减
	//int			Step2;

		//F底部出波高度
	uint8_t		FRow;

	//G底部出波高度
	uint8_t		GRow;

	uint8_t		isFindF;

	uint8_t		isFindG;

	//标记
	B_MARK		Mark;

	//伤损/人工标伤
	B_WOUND		Wound;

	//伤损/实时算法判伤
	B_WOUND		Wound2;

	//步进数据
	std::vector<B_RowData>	vRowDatas;
}B_Step;


//A超每帧数据
typedef struct _A_Frame
{
	uint16_t	Horizon;	//横坐标
	uint16_t	F[CH_N];	//声波能量
}A_Frame;

typedef struct _A_Step
{
	uint16_t		Index;		//米块内帧ID
	uint32_t		Block;		//米块索引
	int32_t			Step;		//步进偏移量
	int32_t			Index2;		//帧ID
	bool operator < (_A_Step step) const
	{
		if (Index2 < step.Index2)
		{
			return true;
		}
		return false;
	};

	std::vector<A_Frame> Frames;
}A_Step;
typedef std::vector<A_Step> VASTEPS;

typedef struct _BlockData_A
{
	std::vector<A_Step>		vAStepDatas;//A超帧数据
}BlockData_A;
typedef BlockData_A BDA;

typedef struct _BlockData_B
{
	BLOCK		BlockHead;			//米块头
	uint32_t	Index;				//米块索引，即第几个米块
	int32_t		StepCount;			//米块中步进总数

	int32_t		IndexL;				//米快初始步进，可能为负
	uint32_t    IndexL2;            //米快初始步进，不会为负

	int32_t	    AStartPos;          //A超起始位置点
	uint32_t	BStartPos;          //B超起始位置点

	double		Walk;				//米块开始里程，由于blockhead中的里程可能有9999，故重设字段
	double		Walk2;				//经过校正之后的里程

	double      gpsLat;				//GPS
	double      gpsLog;				//GPS，经纬度都为0表示该米块没有GPS

	uint16_t	FrameCount;			//米块头指示的A超帧数
	uint16_t	FrameCountRead;		//读到的A超帧数
	std::vector<B_Step>		vBStepDatas;//B超步进数据

	//米块开始底波行高
	int16_t		FRow;

	//米块结束底波行高
	int16_t		FRow2;

	int			sumAaBbCc;
	int			sumDE;
	int			sumFG;
}BlockData_B;
typedef BlockData_B BDB;
typedef std::vector<BlockData_B> VBDB;


const int szAHead = sizeof(BLOCK_A);
const int szBHead = sizeof(BLOCK);
const int szFileHead = sizeof(F_HEAD);

#define N_BLOCKREAD	20

//-----------------------------------------------
// 计算二次波的三角函数
//-----------------------------------------------
typedef struct _TRIANGLE
{
	float sin;
	float cos;
	float tan;
}TRIANGLE;


//-----------------------------------------------
// 波形中探头覆盖范围的数据结构
//-----------------------------------------------
struct PROBE_COVERAGE_AREA
{
	int     Direction;                 // 探头朝向：-1 - 向前；1 - 向后；0 - 垂直向下
	int     Range1;                    // 第一段范围（步进）
	int     Range2;                    // 第二段范围（步进）
};

//B超出失波记录
typedef struct _WaveData
{
	int32_t		block;	//米块索引
	int32_t		step;	//步进索引
	uint8_t		row;	//行
	uint8_t		find;	// BIT0：0:失波，1：出波，BIT7：0：未处理，1：已处理
	bool operator < (_WaveData& wd) const;
	bool operator == (_WaveData& wd) const;
}WaveData;
typedef std::vector<WaveData> VWAVEDATAS;

typedef struct _StepRegion
{
	uint32_t	step;
	int8_t		row1;
	int8_t		row2;

	bool operator < (_StepRegion& sr) const
	{
		return this->step < sr.step;
	}
}StepRegion;

typedef struct _RowRegion
{
	uint8_t		row;
	int32_t		step1;
	int32_t		step2;

	bool operator < (_RowRegion& rr) const
	{
		return this->row < rr.row;
	}
}RowRegion;

typedef struct _CR_INFO_A
{
	uint16_t	MinH;//出波的最小横坐标，A超512
	int16_t		MaxH;//出波的最大横坐标，A超512

	int16_t		MaxV;//最小赋值 或 （最大幅值，F,G检测失波时才使用该意义）
	int16_t		Shift;//位移

	uint16_t	MaxV2;//原幅值
	uint16_t	Shift2;//原位移

	uint16_t	Shift3;//根据各部分位移折算之后的位移

	uint8_t		iSection;
	uint16_t	d1_4;//1 -> 4.5
	uint16_t	d4_7;//4.5 -> 7
	uint16_t	d6_8;//6 -> 8.5
}CR_INFO_A;
const int32_t szCRA = sizeof(CR_INFO_A);


//出波连通域
class  Connected_Region
{
public:
	Connected_Region();

	uint32_t		Block;	//米块索引，测试使用
	int32_t			Step;	//米块内步进
	int32_t			Step1;	//最小步进
	uint8_t			Row1;	//最小行
	uint8_t			Row2;	//最大行
	int32_t			Step2;  //最大步进
	uint8_t			Channel;
	uint8_t			IsUsed;	//是否判断过了
	VWAVEDATAS		Region; //出波点数据！！！
	VASTEPS			vASteps;//对应的A超数据
	CR_INFO_A		Info;
	int16_t			H1;		//计算A超出波横坐标
	int16_t			H2;		//计算A超出波横坐标

	int16_t			MinH;	//最后结果的A超横坐标
	int16_t			MaxH;	//最后结果的A超横坐标

	uint8_t			IsDirty;//是否大量重复的出波（砂/鱼鳞）

	uint8_t			IsLose;	//是否失波
	uint8_t			IsSew;	//是否焊缝 0：不是，1：厂焊，2：现场焊, 10：铝热焊（德式），11：铝热焊（法式）
	uint8_t			IsJoint;
	uint8_t			IsScrewHole;
	uint8_t			IsGuideHole;
	uint8_t			IsWound;
	uint8_t			IsContainA;//是否有A超
	uint8_t			IsDoubleCR;//是否是双波
	uint8_t			IsDoubleWave;//是否螺孔、导孔双波
	uint8_t			IsEnsureNotDoubleWave;
	uint8_t			IsReversed;//是否倒打
	uint8_t			CombinedCount;//合并次数

	uint8_t			IsIllgeal;	//标志位

	int32_t			Index;	//用以处理螺孔双波

	bool operator < (Connected_Region& cr) const;

	bool			IsStart;
	bool			IsEnd;
};
typedef Connected_Region CR;
typedef std::vector<Connected_Region> VCR;

//失波连通域
class LoseConnected_Region
{
public:
	LoseConnected_Region(uint8_t channel, int step1, int step2, uint8_t row1, uint8_t row2, uint8_t isSew, uint8_t isjoint, uint8_t isScrewHole, uint8_t isGuideHole);
	uint8_t			Channel;	
	int32_t			Step1;	//最小步进
	int32_t			Step2;
	uint8_t			Row1;	//最小行
	uint8_t			Row2;

	uint8_t			IsSew;	//是否焊缝 0：不是，1：厂焊，2：现场焊, 10：铝热焊（德式），11：铝热焊（法式）
	uint8_t			IsJoint;
	uint8_t			IsScrewHole;
	uint8_t			IsGuideHole;
};

typedef LoseConnected_Region LCR;
typedef std::vector<LoseConnected_Region> VLCR;

typedef struct _Pos   //声明一个结构体类型 Pos 回退数据：位置点
{
	int			Block;
	int32_t		Step;
	int32_t		Step2;
	double		Walk;
}Pos;

typedef struct _BackAction   //声明一个结构体类型 BackAction 回退数据
{
	Pos			Pos0;//回退之前的位置
	Pos			Pos1;//回退点
	Pos			Pos2;//回退之后再往前走到回退点的位置
}BackAction;
typedef std::vector<BackAction>	VBA;

typedef struct _RECT
{
	int32_t		step1;
	int32_t		step2;
	uint8_t		row1;
	uint8_t		row2;
}TRECT;


//导出伤损数据
struct Wound_Judged
{
	int32_t						Block;
	int32_t						Step;		//米块内步进
	int32_t						Step2;		//总步进
	int32_t						StepLen;	//步进方向长度
	uint8_t						Row1;
	uint8_t						RowLen;

	char						Result[60];	// 伤损
	double						Walk;		// 里程
	uint16_t					Place;	//伤损在截面位置
	uint16_t					Type;	//种类
	uint16_t					Degree;//程度
	uint16_t					SizeX;
	uint16_t					SizeY;

	uint8_t						Checked;	//复核状态
	double						Walk2;		// 里程

	double						gps_log;
	double						gps_lat;	

	uint8_t						IsBridge;
	uint8_t						IsTunnel;	//隧道ID
	uint8_t						IsCurve;	//曲线ID
	uint8_t						IsJoint;	//接头ID

	//焊缝 0: 非焊缝， 1:厂焊， 2:铝热焊， 3:现场焊
	uint8_t						IsSew;		


	int8_t						IsScrewHole;//螺孔ID
	uint8_t						IsGuideHole;//导孔ID

	uint8_t						Cycle;		//当前周期数
	uint64_t					LastCycleID;//上周期伤损ID

	VCR							vCRs;		//伤损出波/失波数据
	VLCR						vLCRs;
	uint8_t						Flag;

	uint8_t						Manual;		//是否人工判伤，需要确定伤损类型
	std::vector<std::string>	According;  //判据


	uint64_t					FileID;		//作业ID
	uint8_t                     equType;

	uint16_t					Num[16];	//各通道出波点数
	uint8_t						ChannelNum; //出波通道数
	uint16_t					ChannelMaxNum;//最大单通道出波点数

	uint8_t						IsReversed;//是否倒打
	uint8_t						IsMatched;//是否与试块匹配

	TRECT						Rect;

public:
	Wound_Judged();
	void		SetEmpty();

	bool operator < (Wound_Judged& w) const;
};

const int szWound = sizeof(Wound_Judged);

typedef Wound_Judged WJ;
typedef std::vector<Wound_Judged> VWJ;

/*
enum WOUND_TYPE
{
	W_OTHER = 0,				//其他，非伤
	W_HEAD_HS = 1,				//核伤

	W_HS = W_HEAD_HS,	//轨头内侧核伤
	W_HEAD_HS_MID = W_HEAD_HS,	//轨头中心核伤
	W_HS = W_HEAD_HS,	//轨头外侧核伤

	W_YLS = 2,					//鱼鳞伤
	//W_SCREW_CRACK = 16,			//螺孔斜裂纹
	W_SCREW_CRACK1 = 17,
	W_SCREW_CRACK2 = 18,
	W_SCREW_CRACK3 = 19,
	W_SCREW_CRACK4 = 20,
	W_SCREW_HORIZON_CRACK = 32,		//螺孔水平裂纹
	W_VERTICAL_CRACK = 64,			//纵向水平裂纹

	W_JAW_HORIZON_CRACK = 4,		//轨颚水平裂纹
	W_WAIST_HORIZON_CRACK = 8,		//轨腰水平裂纹
	W_HEAD_HORIZON_CRACK = 9,		//轨头水平裂纹

	

	W_SKEW_CRACK = 128,				//斜裂纹
	W_BOTTOM_TRANSVERSE_CRACK = 256,	//轨底横向裂纹（半圆形，竖直截面内）

	W_JOINT = 5,					//接头伤损
	W_SEWLR = 6,					//铝热焊缝伤损
	W_SEWCH = 7,					//厂焊伤损
	W_SEWXCH = 10,					//现场焊伤损（暂时先归类到铝热焊伤损）
	W_MANUAL = 1000                  //人工标记伤损
};
*/
enum WOUND_TYPE_STANDARD
{
	W_NORMAL = 0,					//非伤
	W_HS = 1,						//核伤
	W_YLS	= 2,					//鱼鳞伤
	W_HORIZONAL_CRACK = 4,			//水平裂纹

	W_JOINT = 5,					//接头伤损
	W_SEWLR = 6,					//铝热焊缝伤损
	W_SEWCH = 7,					//厂焊伤损
	W_SEWXCH = 10,					//现场焊伤损（暂时先归类到铝热焊伤损）

	W_SCREW_CRACK1 = 17,
	W_SCREW_CRACK2 = 18,
	W_SCREW_CRACK3 = 19,
	W_SCREW_CRACK4 = 20,
	W_SCREW_HORIZON_CRACK_RIGHT = 32,		//螺孔右侧水平裂纹
	W_SCREW_HORIZON_CRACK_LEFT = 33,		//螺孔左侧水平裂纹

	W_GUIDE_CRACK1 = 41,
	W_GUIDE_CRACK2 = 42,
	W_GUIDE_CRACK3 = 43,
	W_GUIDE_CRACK4 = 44,
	W_GUIDE_HORIZON_CRACK_RIGHT = 45,		//导孔右侧水平裂纹
	W_GUIDE_HORIZON_CRACK_LEFT = 46,		//导孔左侧水平裂纹

	W_VERTICAL_CRACK = 64,			//纵向水平裂纹
	W_SKEW_CRACK = 128,				// 斜裂纹	
	W_BOTTOM_TRANSVERSE_CRACK = 256,//轨底横向裂纹（半圆形，竖直截面内）


	W_HEAD_EX = 300,				//轨头异常
	W_JAW_EX = 310,				//轨颚异常
	W_WAIST_EX = 320,				//c
	W_BOTTOM_EX = 330,				//轨底异常

	W_DOUBLEHOULE_SCREW = 400,		//螺孔套孔
	W_DOUBLEHOULE_GUIDE = 410,		//导孔套孔
	W_DOUBLE_HOLE = 420,			//多孔

	W_MANUAL = 1000                  //人工标记伤损
};

enum WOUND_DEGREE
{
	WD_OK = 100,
	WD_CAPABLIE = 0, //疑似
	WD_LESS = 1,//不到轻伤
	WD_SMALL = 2,//轻伤
	WD_MEDIUM = 3,//轻发
	WD_SERIOUS = 4,//重伤
	WD_BREAK = 5,//折断
	WD_NORMAL = 100//正常
};

enum WOUND_POSITION
{
	WP_UNKNOWN = 0,		//未知
	WP_TM = BIT0,		//轨头踏面中
	WP_HEAD_IN = BIT1,	//轨头内侧
	WP_HEAD_MID = BIT2, //轨头中侧
	WP_HEAD_OUT = BIT3, //轨头外侧
	WP_HEAD = WP_HEAD_IN | WP_HEAD_OUT | WP_HEAD_MID,//轨头
	WP_ANGLE = BIT4,	//轨距角
	WP_JAW_IN = BIT5,	//轨颚内侧
	WP_JAW_OUT = BIT6,	//轨颚外侧
	WP_WAIST = BIT7,	//轨腰
	WP_BOTTOM = BIT8,	//轨底
	WP_BOTTOM_ANGLE_IN = BIT9,	//轨底
	WP_BOTTOM_ANGLE_OUT = BIT10	//轨底
};

struct Position_Mark   //声明一个结构体类型 Position_Mark_RAW 位置标数据
{
	uint32_t				Block;	//米块
	int16_t					Step;	//米块内步进
	uint32_t				Step2;	//总步进

	uint16_t				Mark;	//位置标记类型（算法判定）

	double					Walk;	//开始里程
	double					Walk2;	//结束里程

	/*数据，可有可无
	若为人工自定义标记，该值为自定义标记的值

	若Mark为曲线起/止，则BIT0：上股，1：下股

	*/
	uint16_t				Data;	
	
	uint8_t					ARow;	//平均行
	int32_t					AStep;	//平均步进
	uint8_t					ChannelNum;

	/*
	各通道出波点数
	pm.Num[CH_d] = 1;表示该位置标由YOLO判定
	*/
	uint16_t				Num[16];
	double					Fangcha;
	uint16_t				Size;
	double					Percent;

	uint32_t				BeginStep;
	uint16_t				Length;
	uint16_t				Height;

	uint8_t					Flag;
	/*
	若Mark类型为接头焊缝，ScrewHoleCount，GuideHoleCount为本意
	若为螺孔、导孔，为FG出波长度，失波长度
	若为变坡点，螺孔为变坡之前的行高，导孔为变坡之后的行高
	*/
	uint8_t					ScrewHoleCount;//螺孔个数
	uint8_t					GuideHoleCount;//导孔个数

	uint8_t					Manual;//是否人工标记

	//只针对接头有效，0: 不是，1：左半边接头，2：左半边接头
	uint8_t					IsHalf;

	//是否找到人工标记/实际出波对应的步进，记录对应的步进
	int32_t					IsFindWave;

	//接头焊缝中线
	int32_t					MiddleStep;
	
	//大通道不含C位置
	int32_t					BiggerStep;
	
	//小通道不含c位置
	int32_t					LessStep;

	//是否被覆盖了，先标*1，再标*2，*1的标记会被记1
	uint8_t					IsOverlapped;

	/*
	0: YOLO未检出， 1： YOLO与算法检出结果相同， 2：YOLO与算法检出结果不相同
	*/
	uint8_t					IsYoloChecked;
};


struct Position_Mark_RAW   //声明一个结构体类型 Position_Mark_RAW 人工标记位置标数据
{
	uint32_t				Block;	//米块
	int16_t					Step;	//米块内步进
	uint32_t				Step2;	//总步进

	uint16_t				Mark;	//位置标记类型（算法判定）

	double					Walk;	//开始里程
	double					Walk2;	//结束里程

	/*数据，可有可无
	若为人工自定义标记，该值为自定义标记的值
	*/
	uint16_t				Data;

	uint8_t					ARow;	//平均行
	int32_t					AStep;	//平均步进
	uint8_t					ChannelNum;

	/*
	各通道出波点数
	pm.Num[CH_d] = 1;表示该位置标由YOLO判定
	*/
	uint16_t				Num[16];
	double					Fangcha;
	uint16_t				Size;
	double					Percent;

	uint32_t				BeginStep;
	uint16_t				Length;
	uint16_t				Height;

	uint8_t					Flag;
	/*
	若Mark类型为接头焊缝，ScrewHoleCount，GuideHoleCount为本意
	若为螺孔、导孔，为FG出波长度，失波长度
	若为变坡点，螺孔为变坡之前的行高，导孔为变坡之后的行高
	*/
	uint8_t					ScrewHoleCount;//螺孔个数
	uint8_t					GuideHoleCount;//导孔个数

	uint8_t					Manual;//是否人工标记
};
typedef  Position_Mark PM;
typedef  std::vector<Position_Mark> VPM;

enum POSITION_MARK
{
	PM_UNKONWN = 0,		//未标识
	PM_JOINT2 = 1,		//2孔接头
	PM_JOINT4 = 2,		//4孔接头
	PM_JOINT6 = 3,		//6孔接头


	PM_SEW_LRH = 4,		//铝热焊缝
	PM_SEW_CH = 5,		//厂焊
	PM_SEW_LIVE = 6,	//现场焊

	PM_SCREWHOLE = 7,	//螺孔
	PM_GUIDEHOLE = 8,	//导孔

	PM_SMART1 = 9,		//尖轨变坡点
	PM_SMART2 = 10,		//异型轨变坡点

	PM_HJFORK_BEGIN = 11,//合金道岔开始
	PM_HJFORK_END = 12,	//合金道岔结束

	PM_MGFORK_BEGIN = 13,//锰钢道岔开始
	PM_MGFORK_END = 14,	//锰钢道岔结束

	PM_BRIDGE_BEGIN = 15, //桥起
	PM_BRIDGE_END = 16, //桥止

	PM_CURVE_BEGIN = 17, //曲起
	PM_CURVE_END = 18, //曲止

	PM_TUNNEL_BEGIN = 50, //隧道起
	PM_TUNNEL_END = 51,		//隧道止

	PM_OVERSPEED = 19, //超速

	PM_COUPLE = 20,	//耦合

	PM_START = 30,		//上道
	PM_BACK = 31,		//回退

	PM_WALKREVISE = 40,	//里程校正
	PM_WALK_PULSE = 41, //里程突变

	PM_JOINT = 60, //自动识别的接头
	PM_JOINT_LEFT = 61, //左侧半边接头
	PM_JOINT_RIGHT = 62, //右侧半边接头

	PM_SELFDEFINE = 100	//自定义
};

typedef struct _JointReflectWave
{
	CR		cr;
	uint8_t Row1;
	uint8_t Row2;
	int8_t	RailType;
	int8_t	FRow;
}JointReflectWave;
typedef std::vector<JointReflectWave> VJRW;


typedef struct _TPoint
{
	int32_t		Step;
	uint8_t		Row;
	uint8_t		Channel;
}TPoint;

//异常出波区域
typedef struct _ErroeRect
{
	uint8_t		channel;
	uint32_t	block1;
	uint32_t	block2;
	double		walk1;
	double		walk2;
	int32_t		step1;
	int32_t		step2;
	uint8_t		row1;
	uint8_t		row2;
	bool operator < (_ErroeRect g) const
	{
		if ((channel < g.channel) || (channel == g.channel && step1 < g.step1) || (channel == g.channel && step1 == g.step1 && row1 < g.row1))
		{
			return true;
		}
		return false;
	};
}ErroeRect;   //typedef进行类型定义 ErroeRect是一个结构体类型
typedef std::vector<ErroeRect> VER;

//跳探
typedef struct _LoseDetect
{
	double	walk1;
	double	walk2;
	int32_t	step2;
	uint8_t user;
	int8_t	hour;
	int8_t	minute;
	int8_t second;
}LoseDetect;
typedef std::vector<LoseDetect> VLD;

//通道失耦信息
typedef struct LoseCouplePoint
{
	uint16_t	Channel;
	uint32_t	Block;
	uint32_t	Step;
}LCP;
typedef std::vector<LCP> VLCP;

//失耦
typedef struct _LoseCouple
{
	uint16_t	Channel;
	uint32_t	Block1;
	uint32_t	Step1;
	double		Walk1;
	uint32_t	Block2;
	uint32_t	Step2;
	double		Walk2;
}LoseCouple;
typedef std::vector<LoseCouple> VLC;


typedef struct _DQWavePoint
{		
	uint32_t	Step;
	uint16_t	Channel;
	uint8_t		Row;
	uint8_t		JawRow;
	bool operator < (_DQWavePoint g) const
	{
		if ((Step < g.Step) || (Step == g.Step && Row < g.Row))
		{
			return true;
		}
		return false;
	};

	bool operator == (_DQWavePoint g) const
	{
		if (Step == g.Step && Row == g.Row)
		{
			return true;
		}
		return false;
	};
}DQWavePoint;


typedef struct _DQRect
{
	uint32_t	Step;
	uint16_t	Total;
	uint8_t		Row;
	uint8_t		Count[16];
}DQRect;

typedef struct _DQRowRect
{
	uint16_t	Total;
	uint8_t		Count[16];
}DQRowRect;


typedef struct _FullMeter
{
	uint8_t		Channel;
	uint32_t	Block1;
	uint32_t	Block2;
}FullMeter;

typedef struct _Section
{
	int		Start;
	int		End;
	int		Flag;
}Section;


typedef struct _PointRegion
{
	//起点坐标
	int		Index;

	//持续长度
	int		Length;
}PointRegion;

class railTemplate
{
public:
	std::string railName;

	double		railLength;

	std::vector<std::string> vName;

	VWJ			vWounds;

	int			woundCount;

	VPM			vMarks;
};

typedef std::vector<railTemplate> VRT;




//每步进出波信息统计
typedef struct _StepChannelPointDistrubute
{
	int		Step;
	int		ChannelNum;
	int		TotalHead;  //轨头点数统计
	int		TotalWaist;	  //轨腰点数统计
	int		SumHead15;  //轨头15步进出波点数
	int		SumJaw15;   //轨颚15步进出波点数
	int		NumJaw;   //轨颚点数点数统计
	int		SumWaist15;	  //轨腰15步进出波点数
	int		Num[14];//A->E
}StepChannelPointDistrubute;
typedef	StepChannelPointDistrubute SCPD;
typedef std::vector<StepChannelPointDistrubute> VSCPD;



typedef struct _ChannelOffset
{
	int		BeginBlock;

	int		EndBlock;

	int		Offset[CH_N];
}ChannelOffset;

typedef std::vector<ChannelOffset> VCO;

typedef struct _Fork
{
	PM	Begin;
	PM	End;
	int iBegin;
	int iEnd;
	uint16_t ForkNo;
	uint8_t Bits;
}Fork;

typedef struct _ForkPara
{
	int		iIndex;
	int		iEndIndex;
	uint16_t ForkNo;
	uint8_t Bits;
}ForkPara;

typedef struct _HolePara
{
	uint32_t	Block;
	uint32_t	Step;
	uint32_t	step2;
	uint8_t		channels[10];
	int8_t		row1[10];
	int8_t		row2[10];
	uint16_t	mark;
	uint8_t		flag;
	uint8_t		isDoubleHole;// 0:不是，1：导孔，2：螺孔
	int32_t		isInFork;//是否在道岔上
	CR			tempF;
	CR			tempD;
	CR			tempE;
	int8_t		Row1;
	int8_t		Row2;
	uint8_t		Height;
	int8_t		FRow;
	int8_t		HoleIndex;
	int32_t		RailType;
	int32_t		FLength;
	int32_t		FLength2;
}HolePara;

typedef struct _BackTotal
{
	int32_t	Block1;
	int32_t	Step1;
	int32_t	Block2;
	int32_t	Step2;	
	int32_t	BeginBackIndex;
	int32_t	BackCount;
}BackTotal;

typedef struct _FRowRecord
{
	uint32_t	Step2;
	uint8_t		FRow;
	uint8_t		GRow;
}FRowRecord;
typedef std::vector< FRowRecord> VFR;

typedef struct _JointHoles
{
	std::vector<HolePara> vHoles;
}JointHoles;

//每次回退判伤结果
typedef struct _SectionAnalyseResult
{
	int beginStep;
	int endStep;
	int beginBlock;
	int endBlock;
	VWJ vWounds;
	VPM vPM;
	VPM vPMs2;
	VINT vSew;
	VINT vJoint;
	VINT vSew2;
	VINT vJoint2;
}SectionAnalyseResult;

typedef struct _BackInfo
{
	int backIndex;
	int step1;
	int step2;
	int JointSewMark;
	int JointSewMarkIndex;
}BackInfo;

#endif


