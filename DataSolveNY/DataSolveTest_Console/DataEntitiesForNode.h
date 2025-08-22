#ifndef DATAENTITIESFORNODE_H
#define DATAENTITIESFORNODE_H

#include "DataEntities.h"

struct GPSINFO
{
	double      gpsLat;   //纬度
	double      gpsLog;   //经度
	bool operator < (GPSINFO g) const
	{
		if ((gpsLog < g.gpsLog) || (gpsLat == g.gpsLat && gpsLog < g.gpsLog))
		{
			return true;
		}
		return false;
	};

	bool operator == (GPSINFO g) const
	{
		if (gpsLog == g.gpsLog && gpsLat == g.gpsLat)
		{
			return true;
		}
		return false;
	};
};

struct BLOCK_B4Nodejs   //声明一个结构体类型 BLOCK_B4Nodejs 米块头
{
	BLOCK		blockHead;//米块头
	uint32_t    Index;  //米块索引
	int32_t     IndexL; //开始步进，可能负
	uint32_t    IndexL2;//开始步进，非负
	int32_t     AStartPos;//A超开始字节，如果未-1表示该米块没有A超
	uint32_t    BStartPos;//B超开始字节
	double      Walk;     //里程
	double      Walk2;    //里程
	double      gpsLat;   //GPS
	double      gpsLog;   //GPS，经纬度都为0表示该米块没有GPS
	uint16_t	FrameCount;			//米块头指示的A超帧数
	uint16_t	FrameCountRead;		//读到的A超帧数
};

struct A_Frame4Nodejs   //声明一个结构体类型 A_Frame4Nodejs A超每帧结构
{
	int32_t		FrameIndex;
	int32_t     Step;
	int32_t     Horizon;
	uint16_t    F[CH_N];   //12个通道的幅值
};

struct B_Step4Nodejs   //声明一个结构体类型 B_Step4Nodejs B超每步进数据
{
	uint32_t    Step;
	B_MARK      Mark;
	B_WOUND     Wound;
};

struct B_Step4NodejsPT
{
	uint32_t    Step;
	B_MARK      Mark;
	B_WOUND     Wound;
	B_WOUND     Wound2;
};

struct B_Row4Nodejs   //声明一个结构体类型 B_Row4Nodejs B超每步进每行出波数据
{
	uint32_t    Step;
	uint8_t     Row;
	B_POINT     Point;
};

struct File4Nodejs   //声明一个结构体类型 File4Nodejs 
{
	char        FileName[36];   //原文件名
	char        NewFileName[60];//新文件名
	char        anaTime[20];    //算法开始分析时间，格式 2011-11-11 11:11:11
	uint32_t	sizeA;			//tpA字节数
	uint32_t	sizeB;			//tpB字节数
	uint32_t    gwd;            //单位（工务段）
	uint32_t    section;        //工区
	uint32_t    group;          //班组
	uint16_t    railno;         //线编号
	uint8_t     xingbie;        //行别，0：单线、1：上，2：下行
	uint8_t     xianbie;        //线别
	uint8_t     leftright;      //股别，0: 右1：左
	uint8_t     direction;      //顺逆里程，0逆里程，1顺里程

	char        s_time[20];     //开始作业时间，格式 2011-11-11 11:11:11
	char        e_time[20];		//结束作业时间，格式 2011-11-11 11:11:11

	char        instruNo[8];	//仪器编号
	double      s_mil;			//开始里程
	double      e_mil;			//结束里程

	char        dataV[8];		//数据版本
	char        softV[8];		//软件版本
	char        fpgaV[8];		//FPGA版本

	uint32_t    blockCount;		//总米块数
	uint8_t     sdType;			//上道类型 0: 站线, 1: 正线
	uint8_t		carType;		//车型

	int16_t     Offset[12];     //探头位置
	DOOR	    Gate[4 * GA_NUM];// 大门, 单位/15us

	uint16_t	PlyNo;			//站线股道编号

	uint32_t	StationNum;		//站编号
};

struct Wound4Nodejs   //声明一个结构体类型 Wound4Nodejs 伤损数据
{
	uint32_t    XSize;
	uint32_t    YSize;
	uint16_t    Place;  //伤损位置
	uint16_t    Type;   //伤损类型
	uint16_t    Degree; //伤损程度
	double      Lat;
	double      Log;    //经纬度

	uint32_t    Block;  //米块
	int32_t     Step;   //米块内步进
	uint32_t    Step2;  //总步进2，非负
	uint32_t	StepLeft;
	uint8_t		Row1;
	uint8_t		Row2;
	double      Walk;   //里程
	double      Walk2;  //对齐后的里程

	uint8_t     Manual; //是否人工
	uint8_t     equType;//设备类型
	uint8_t		IsBridge;//是否桥
	uint8_t		IsTunnel;//是否隧道
	uint8_t		IsCurve; //是否曲线
	uint8_t     IsJoint;//是否接头
	uint8_t     IsSew;  //是否焊缝
	uint8_t     IsScrewHole;//是否螺孔
	uint8_t     IsGuideHole;//是否导孔
	char        Result[60]; //判伤结果
	char        According[1000];//判据

	/*
	判伤等级
	单周期判出：BIT0
	图形判出：BIT1
	多周期判出：BIT2
	*/
	uint8_t		Level;

	uint8_t		Cycle;//周期数
};

struct FileBlock4Nodejs
{
	char            FileName[60];
	char			FileID[40];
	uint32_t        BlockCount;
	BLOCK_B4Nodejs  Blocks[25000];
};


struct RevisedWalk
{
	uint32_t        Block;
	double          Walk;
};

struct AlignedMark
{
	int32_t			Prev;//Previous
	int32_t			Curr;//Current
};

struct  AlignedPosition
{
	int32_t			StepCurr;//本周期步进
	int32_t			StepPrev;//上周期步进
	inline bool operator < (AlignedPosition& ap)
	{
		return this->StepCurr < StepPrev;
	}
};

struct AlignedBack
{
	int32_t			Step1;//回退前
	int32_t			Step2;//回退后
};

struct ContinueRegion
{
	VINT vRectIndexes;
};

struct BLOCKDetail
{
	uint32_t    Index;  //米块索引
	int32_t     IndexL; //开始步进，可能负
	uint32_t    IndexL2;//开始步进，非负
	int32_t     AStartPos;//A超开始字节，如果未-1表示该米块没有A超
	uint32_t    BStartPos;//B超开始字节
	double      Walk;     //里程
	double      Walk2;    //里程
	double      gpsLat;   //GPS
	double      gpsLog;   //GPS，经纬度都为0表示该米块没有GPS
	uint16_t	FrameCount;			//米块头指示的A超帧数
};
typedef std::vector<BLOCKDetail> HBLOCKS;

struct WoundInfoDetail1
{
	//原文件名
	char		fileName[50];

	//新文件名
	char		newFileName[70];

	int32_t		Step1;

	int32_t		Step2;

	//工务段
	uint32_t	gwd;

	//工区
	uint32_t	section;

	//线别
	uint8_t		lineType;

	//股别
	uint8_t		gubie;

	//上下行
	uint8_t		xingbie;

	//顺逆里程
	uint8_t		direction;

	//线编号/站编号
	uint32_t	rsNo;

	//仪器编号
	char		instruNo[8];

	//执机人
	uint16_t	userNo;

	//年
	int32_t		year;

	//月
	int32_t		month;
};


//结构化信息
struct WoundDetailInfo
{
	WoundInfoDetail1 info1;

	//米块
	VBDB	vBlocks;

	//检出伤损
	VWJ		vWounds;

	//标记伤损
	VWJ		vWoundsManual;

	//检出位置标
	VPM		vPMs;

	//标记位置标
	VPM		vPMsManual;

	VER		vER;
};

//超声文件信息
typedef struct SW_FileInfo
{
	//完整文件名
	std::string	strTPB;
	F_HEAD		head;

	uint8_t		isTestEqu; 
	int			sztWoundCount;
	int			sztAlarmCount;

	VBDB		blocks;
	VWJ			wounds;
	VPM			marks;
	VPM			marks2;
	VBA			backs;
	VLC			loseCouples;
	VLD			loseDetects;
	VER			errorRects;

}SW_FileInfo;


typedef struct _FileQualityInfo   //声明一个结构体类型 FileQualityInfo 数据质量信息
{
	int32_t		ManualMarkCount;   //人工标记数量
	Position_Mark_RAW*			ManualMarKs;

	int32_t		LoseCoupleCount;   //失耦数量
	LoseCouple* LoseCouples;

	int32_t		ErroeRectCount;   //异常出波区域数量
	ErroeRect*	ErroeRects;

	int32_t		LoseDetectCount;   //跳探数量
	LoseDetect* LoseDetects;
}FileQualityInfo;

typedef struct _WalkInfo
{
	int Block1;
	int Block2;

	double walk1;
	double walk2;

	double Length;
}WalkInfo;


typedef struct _FileSolveInfo   //文件解析结果信息
{
	char FileName[100];   //原文件名
	char DataPath[100];   ////新文件名

	int32_t		IsTestEqu;   //是否试块

	int32_t		BlockCount;   //米块
	BLOCK_B4Nodejs* Blocks;

	int32_t		MarkCount;   //位置标
	Position_Mark_RAW*			Marks;

	int32_t		WoundCount;   //伤损
	Wound4Nodejs* Wounds;

	int32_t		BackCount;   //回退
	BackAction* Backs;

	int32_t		sztWoundCount;   //SZT800伤损数
	int32_t		sztAlarmCount;   //SZT800报警数

	FileQualityInfo QualityInfo;   //数据质量信息
}FileSolveInfo;
#endif // DATAENTITIESFORNODE_H
