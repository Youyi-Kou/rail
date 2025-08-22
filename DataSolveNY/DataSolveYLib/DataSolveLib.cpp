#include "stdafx.h"
#include "DataSolveLib.h"
#include "SolveData.h"
#include "CRHPE.h"
#include "PublicFunc.h"
#include "GlobalDefine.h"
#include "Judge.h"

#include <time.h>
#include <process.h>
#include <math.h>

#include <algorithm>

#include <shellapi.h>
#pragma comment(lib, "shell32.lib")


#undef WINMINMAX


#ifdef _DEBUG
#include <errno.h>
#include <string.h>
#endif // _DEBUG

#ifdef NET_SUPPORT
#include <WinSock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif // NET_SUPPORT


//#include <python.h>

#include "DataQualityAnalyse.h"
#include "BaseException.h"

//SZT-800数据统计
QualityData g_qualitydata;
int32_t		g_sztWoundCount = 0, g_sztAlarmCount = 0;
TSignSta	g_tss;
TAlarmSta	g_tas;
TDefectSta	g_tds;
TCheckbackSta g_tbs;

std::vector<DQWavePoint> g_vPoints;


std::map<uint16_t, uint32_t>	g_mapWoundTypeCountTemp;

int32_t		g_indexes[16] = { 0 };

VRT			g_vTemplate;

uint32_t	g_threadId = 0;

double		g_walk1 = 0;
double		g_walk2 = 0;

#ifdef NET_SUPPORT
SOCKET		g_socket;
#endif


bool GetWoundRect(WJ & wound, TRECT* rect, uint8_t channel)
{
	rect->step1 = 0x7FFFFFFF;
	rect->step2 = 0;
	rect->row1 = 0xFF;
	rect->row2 = 0;
	bool isFind = false;
	for (int i = 0; i < wound.vCRs.size(); ++i)
	{
		if (channel != wound.vCRs[i].Channel)
		{
			continue;
		}
		if (wound.vCRs[i].Step2 > rect->step2)	rect->step2 = wound.vCRs[i].Step2;
		if (wound.vCRs[i].Step1 < rect->step1)	rect->step1 = wound.vCRs[i].Step1;
		if (wound.vCRs[i].Row2 > rect->row2)	rect->row2 = wound.vCRs[i].Row2;
		if (wound.vCRs[i].Row1 < rect->row1)	rect->row1 = wound.vCRs[i].Row1;
		isFind = true;
	}
	return	isFind;
}

void FindER(uint8_t channel, uint32_t step, VER& vER)
{
	for (int i = g_indexes[channel]; i < g_vER.size(); ++i)
	{
		if (g_vER[i].channel != channel || g_vER[i].step1 > step)
		{
			break;
		}
		else if (g_vER[i].step1 <= step && g_vER[i].step2 >= step)
		{
			vER.emplace_back(g_vER[i]);
		}
	}
}

void GetAlgName(char* algName)
{
	memcpy(algName, g_strAlgName, strlen(g_strAlgName) + 1);
}

void GetAlgVersion(char* algVersion)
{
	memcpy(algVersion, g_strAlgVersion, strlen(g_strAlgVersion) + 1);
}

void GetAlgDescription(char* algDesc)
{
	memcpy(algDesc, g_strAlgDescription, strlen(g_strAlgDescription) + 1);
}

void InitialTemplate()
{
	railTemplate rt;
	rt.railName = "GTS-60C 顺向";
	rt.railLength = 0.0018;//1.8米,两侧一定是接头

	Wound_Judged wound;
	wound.SetEmpty();

	//正方向
	wound.Walk = wound.Walk2 = 0;
	wound.Step2 = 0;
	wound.StepLen = 12;
	wound.Row1 = 11;
	wound.RowLen = 2;
	wound.Type = W_HORIZONAL_CRACK;
	memcpy(wound.Result, "轨颚水平裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨面35mm下Φ4×40平底孔");

	wound.Step2 = 100;
	wound.StepLen = 20;
	wound.Row1 = 55;
	wound.RowLen = 8;
	wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
	memcpy(wound.Result, "轨底横向裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨底中心宽12mm高6mm激光切槽，轨底横向裂纹");

	wound.Step2 = 140;
	wound.Type = W_HS;
	wound.StepLen = 10;
	wound.Row1 = 1;
	wound.RowLen = 12;
	memcpy(wound.Result, "核伤", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("Φ3×30mm横孔");

	wound.Step2 = 176;
	wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
	wound.StepLen = 10;
	wound.Row1 = 55;
	wound.RowLen = 7;
	memcpy(wound.Result, "轨底横向裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨底中心宽10mm高4mm激光切槽，轨底横向裂纹");

	//wound.Step2 = 215;
	//wound.Type = W_HS;
	//wound.Place = WP_HEAD_MID;
	//wound.StepLen = 10;
	//wound.Row1 = 13;
	//wound.RowLen = 6;
	//memcpy(wound.Result, "核伤", 60);
	//rt.vWounds.emplace_back(wound);
	//rt.vName.emplace_back("轨面下58mmΦ3横通孔");
	//wound.Place = 0;

	wound.Step2 = 215;
	wound.Type = W_HS;
	wound.StepLen = 10;
	wound.Row1 = 1;
	wound.RowLen = 10;
	memcpy(wound.Result, "核伤", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("4×20平底孔");

	wound.Step2 = 250;
	wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
	wound.StepLen = 10;
	wound.Row1 = 55;
	wound.RowLen = 6;
	memcpy(wound.Result, "轨底横向裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨底中心宽8mm高2mm激光切槽，轨底横向裂纹");

	wound.Step2 = 328;
	wound.Type = W_HS;
	wound.StepLen = 5;
	wound.Row1 = 11;
	wound.RowLen = 3;
	memcpy(wound.Result, "核伤", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("Φ4*5*120°");

	wound.Step2 = 340;
	wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
	wound.StepLen = 12;
	wound.Row1 = 55;
	wound.RowLen = 6;
	memcpy(wound.Result, "轨底横向裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨底中心Φ10×10mm，顶部120°锥孔");


	wound.Step2 = 440;
	wound.Type = W_HS;
	wound.StepLen = 10;
	wound.Row1 = 0;
	wound.RowLen = 11;
	memcpy(wound.Result, "核伤", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("Φ4*20 26°平底孔");

	wound.Step2 = 528;
	wound.Type = W_SCREW_CRACK1;
	wound.StepLen = 5;
	wound.Row1 = 27;
	wound.RowLen = 5;
	memcpy(wound.Result, "-3孔一象限斜裂纹", 60);
	wound.IsScrewHole = -3;
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("3孔 25°3mm上裂纹");

	wound.Step2 = 528;
	wound.Type = W_SCREW_CRACK4;
	wound.StepLen = 5;
	wound.Row1 = 34;
	wound.RowLen = 5;
	memcpy(wound.Result, "-3孔四象限斜裂纹", 60);
	wound.IsScrewHole = -3;
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("3孔 37°3mm下裂纹");


	wound.Step2 = 574;
	wound.Type = W_SCREW_HORIZON_CRACK_RIGHT;
	wound.StepLen = 5;
	wound.Row1 = 31;
	wound.RowLen = 2;
	memcpy(wound.Result, "-2孔右侧水平裂纹", 60);
	wound.IsScrewHole = -2;
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("2孔 5mm水平裂纹");

	wound.Step2 = 590;
	wound.Type = W_SCREW_CRACK2;
	wound.StepLen = 5;
	wound.Row1 = 27;
	wound.RowLen = 5;
	memcpy(wound.Result, "-2孔二象限斜裂纹", 60);
	wound.IsScrewHole = -2;
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("2孔 53°");

	wound.Step2 = 632;
	wound.Type = W_SCREW_CRACK1;
	wound.StepLen = 5;
	wound.Row1 = 27;
	wound.RowLen = 5;
	memcpy(wound.Result, "-1孔一象限斜裂纹", 60);
	wound.IsScrewHole = -1;
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("1孔 37°");

	wound.Step2 = 650;
	wound.Type = W_SCREW_CRACK3;
	wound.StepLen = 5;
	wound.Row1 = 34;
	wound.RowLen = 5;
	memcpy(wound.Result, "-1孔三象限斜裂纹", 60);
	wound.IsScrewHole = -1;
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("1孔 37°");

	rt.woundCount = rt.vWounds.size();
	for (int i = 0; i < rt.woundCount; ++i)
	{
		rt.vWounds[i].Walk = rt.vWounds[i].Step2 * 2.66 / 1000000;
	}
	g_vTemplate.emplace_back(rt);

	//反方向
	rt.railName = "GTS-60C 逆向";
	rt.vName.clear();
	rt.vWounds.clear();

	wound.Step2 = 10;
	wound.Type = W_SCREW_CRACK3;
	wound.StepLen = 5;
	wound.Row1 = 34;
	wound.RowLen = 5;
	memcpy(wound.Result, "1孔三象限斜裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("1孔 37°");

	wound.Step2 = 24;
	wound.Type = W_SCREW_CRACK2;
	wound.StepLen = 5;
	wound.Row1 = 27;
	wound.RowLen = 5;
	memcpy(wound.Result, "1孔二象限斜裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("1孔 37°");


	wound.Step2 = 70;
	wound.Type = W_SCREW_CRACK1;
	wound.StepLen = 5;
	wound.Row1 = 27;
	wound.RowLen = 5;
	memcpy(wound.Result, "2孔一象限斜裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("2孔 37°");

	wound.Step2 = 80;
	wound.Type = W_SCREW_HORIZON_CRACK_LEFT;
	wound.StepLen = 5;
	wound.Row1 = 31;
	wound.RowLen = 2;
	memcpy(wound.Result, "2孔左侧水平裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("2孔左侧水平裂纹");

	wound.Step2 = 130;
	wound.Type = W_SCREW_CRACK2;
	wound.StepLen = 5;
	wound.Row1 = 27;
	wound.RowLen = 5;
	memcpy(wound.Result, "3孔二象限斜裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("3孔 25°");

	wound.Step2 = 134;
	wound.Type = W_SCREW_CRACK3;
	wound.StepLen = 5;
	wound.Row1 = 34;
	wound.RowLen = 5;
	memcpy(wound.Result, "3孔三象限斜裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("3孔 37°");

	wound.Step2 = 230;
	wound.Type = W_HS;
	wound.StepLen = 10;
	wound.Row1 = 0;
	wound.RowLen = 11;
	memcpy(wound.Result, "核伤", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("Φ4*20 26°平底孔");

	wound.Step2 = 303;
	wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
	wound.StepLen = 12;
	wound.Row1 = 55;
	wound.RowLen = 6;
	memcpy(wound.Result, "轨底横向裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨底中心Φ10×10mm，顶部120°锥孔");

	wound.Step2 = 330;
	wound.Type = W_HS;
	wound.StepLen = 5;
	wound.Row1 = 11;
	wound.RowLen = 3;
	memcpy(wound.Result, "核伤", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("Φ4*5*120°");

	wound.Step2 = 400;
	wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
	wound.StepLen = 10;
	wound.Row1 = 55;
	wound.RowLen = 6;
	memcpy(wound.Result, "轨底横向裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨底中心宽8mm高2mm激光切槽，轨底横向裂纹");

	//wound.Step2 = 470;
	//wound.Type = W_HS;
	//wound.Place = WP_HEAD_MID;
	//wound.StepLen = 10;
	//wound.Row1 = 13;
	//wound.RowLen = 6;
	//memcpy(wound.Result, "核伤", 60);
	//rt.vWounds.emplace_back(wound);
	//rt.vName.emplace_back("轨面下58mmΦ3横通孔");
	//wound.Place = 0;

	wound.Step2 = 485;
	wound.Type = W_HS;
	wound.StepLen = 10;
	wound.Row1 = 1;
	wound.RowLen = 10;
	memcpy(wound.Result, "核伤", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("4×20平底孔");

	wound.Step2 = 470;
	wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
	wound.StepLen = 10;
	wound.Row1 = 55;
	wound.RowLen = 7;
	memcpy(wound.Result, "轨底横向裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨底中心宽10mm高4mm激光切槽，轨底横向裂纹");

	wound.Step2 = 515;
	wound.Type = W_HS;
	wound.StepLen = 10;
	wound.Row1 = 1;
	wound.RowLen = 5;
	memcpy(wound.Result, "核伤", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("Φ3×30mm横孔");

	wound.Step2 = 546;
	wound.Type = W_BOTTOM_TRANSVERSE_CRACK;
	wound.StepLen = 20;
	wound.Row1 = 55;
	wound.RowLen = 8;
	memcpy(wound.Result, "轨底横向裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨底中心宽12mm高6mm激光切槽，轨底横向裂纹");

	wound.Step2 = 652;
	wound.Type = W_HORIZONAL_CRACK;
	wound.StepLen = 12;
	wound.Row1 = 11;
	wound.RowLen = 2;
	memcpy(wound.Result, "轨颚水平裂纹", 60);
	rt.vWounds.emplace_back(wound);
	rt.vName.emplace_back("轨面35mm下Φ4×40平底孔");

	rt.woundCount = rt.vWounds.size();
	for (int i = 0; i < rt.woundCount; ++i)
	{
		rt.vWounds[i].Walk = 2.66 * rt.vWounds[i].Step2 / 1000000;
	}
	g_vTemplate.emplace_back(rt);

}


void AlgInit(char* strWorkPath, int len)   //算法初始化
//输入参数
//strWorkPath：工作路径，必须是绝对路径，’\’需要替换成’ / ’
//len : 路径字节数
{
	int sz = sizeof(StepChannelPointDistrubute);   //每步进出波信息统计

	if (g_isInitialized == 1)
	{
		return;
	}
	g_isInitialized = 0;

#ifdef OUTPUT_EX
	SET_DEFULTER_HANDLER();
	SET_DEFAUL_EXCEPTION();

	try
	{
#endif // OUTPUT_EX

#pragma region 系统参数初始化
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_NORMAL, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_HS, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_YLS, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_HORIZONAL_CRACK, 0));

		g_mapWoundTypeCountTemp.insert(std::make_pair(W_JOINT, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SEWLR, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SEWCH, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SEWXCH, 0));

		g_mapWoundTypeCountTemp.insert(std::make_pair(W_GUIDE_CRACK1, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_GUIDE_CRACK2, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_GUIDE_CRACK3, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_GUIDE_CRACK4, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_GUIDE_HORIZON_CRACK_RIGHT, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_GUIDE_HORIZON_CRACK_LEFT, 0));


		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SCREW_CRACK1, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SCREW_CRACK2, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SCREW_CRACK3, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SCREW_CRACK4, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SCREW_HORIZON_CRACK_RIGHT, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SCREW_HORIZON_CRACK_LEFT, 0));

		g_mapWoundTypeCountTemp.insert(std::make_pair(W_VERTICAL_CRACK, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_SKEW_CRACK, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_BOTTOM_TRANSVERSE_CRACK, 0));


		g_mapWoundTypeCountTemp.insert(std::make_pair(W_HEAD_EX, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_JAW_EX, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_WAIST_EX, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_BOTTOM_EX, 0));

		g_mapWoundTypeCountTemp.insert(std::make_pair(W_DOUBLEHOULE_SCREW, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_DOUBLEHOULE_GUIDE, 0));
		g_mapWoundTypeCountTemp.insert(std::make_pair(W_DOUBLE_HOLE, 0));


		g_mapWoundTypeCountTemp.insert(std::make_pair(W_MANUAL, 0));






		g_mapWoundTypeCount.insert(std::make_pair(W_NORMAL, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_HS, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_YLS, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_HORIZONAL_CRACK, 0));

		g_mapWoundTypeCount.insert(std::make_pair(W_JOINT, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SEWLR, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SEWCH, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SEWXCH, 0));

		g_mapWoundTypeCount.insert(std::make_pair(W_GUIDE_CRACK1, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_GUIDE_CRACK2, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_GUIDE_CRACK3, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_GUIDE_CRACK4, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_GUIDE_HORIZON_CRACK_RIGHT, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_GUIDE_HORIZON_CRACK_LEFT, 0));


		g_mapWoundTypeCount.insert(std::make_pair(W_SCREW_CRACK1, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SCREW_CRACK2, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SCREW_CRACK3, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SCREW_CRACK4, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SCREW_HORIZON_CRACK_RIGHT, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SCREW_HORIZON_CRACK_LEFT, 0));

		g_mapWoundTypeCount.insert(std::make_pair(W_VERTICAL_CRACK, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_SKEW_CRACK, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_BOTTOM_TRANSVERSE_CRACK, 0));


		g_mapWoundTypeCount.insert(std::make_pair(W_HEAD_EX, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_JAW_EX, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_WAIST_EX, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_BOTTOM_EX, 0));

		g_mapWoundTypeCount.insert(std::make_pair(W_DOUBLEHOULE_SCREW, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_DOUBLEHOULE_GUIDE, 0));
		g_mapWoundTypeCount.insert(std::make_pair(W_DOUBLE_HOLE, 0));


		g_mapWoundTypeCount.insert(std::make_pair(W_MANUAL, 0));

		g_strWoundAlongPlace[0] = "0 - 钢轨全长范围(或全长的大部分)";
		g_strWoundAlongPlace[1] = "1 - 轨身的局部区域";
		g_strWoundAlongPlace[2] = "2 - 夹板接头(轨端、螺栓孔和夹板长度范围的钢轨)区域";
		g_strWoundAlongPlace[3] = "3 - 焊补区域";
		g_strWoundAlongPlace[4] = "4 - 接续线焊接区域";
		g_strWoundAlongPlace[5] = "5 - 闪光焊接头(含电极灼伤部位)";
		g_strWoundAlongPlace[6] = "6 - 铝热焊接头";
		g_strWoundAlongPlace[7] = "7 - 气压焊接头";
		g_strWoundAlongPlace[8] = "8 - ";
		g_strWoundAlongPlace[9] = "9 - 其他形式焊接的焊缝和热影响区";

		g_strWoundVercPlace[0] = "0 - 整个钢轨截面或外表面";
		g_strWoundVercPlace[1] = "1 - 轨头表面(踏面、轨距角、轨头侧面)";
		g_strWoundVercPlace[2] = "2 - 轨头内部";
		g_strWoundVercPlace[3] = "3 - 轨头下颚";
		g_strWoundVercPlace[4] = "4 - 轨腰";
		g_strWoundVercPlace[5] = "5 - 螺栓孔";
		g_strWoundVercPlace[6] = "6 - 轨底(轨底下表面、轨底边缘或轨底角侧面)";


		g_strWoundState[0] = "0 - 弯曲变形";
		g_strWoundState[1] = "1 - 磨耗、压溃、压陷(或凹陷 )";
		g_strWoundState[2] = "2 - 波浪磨耗";
		g_strWoundState[3] = "3 - 接触疲劳裂纹(剥离裂纹)及其引起的掉块和疲劳断裂";
		g_strWoundState[4] = "4 - 内部裂纹或内部缺陷(白点、夹杂物、成分偏析、淬火缺陷、焊接缺陷、焊补缺陷等)及其引起的疲劳断裂";
		g_strWoundState[5] = "5 - 表面缺陷及其引起的疲劳断裂";
		g_strWoundState[6] = "6 - 外伤(擦伤、碰伤等)及其引起的疲劳断裂";
		g_strWoundState[7] = "7 - 锈蚀及其引起的疲劳断裂";
		g_strWoundState[8] = "8 - 没有明显疲劳裂纹的脆性断裂";
		g_strWoundState[9] = "9 - 其他";


		g_strWoundThinning[0] = "0 - 没有细化";
		g_strWoundThinning[1] = "1 - 曲线上股轨头磨耗超限";
		g_strWoundThinning[2] = "2 - 曲线下股轨头全长压溃和辗边";
		g_strWoundThinning[3] = "3 - 直线钢轨交替不均匀侧面磨耗";
		g_strWoundThinning[4] = "4 - 曲线上股轨头磨耗超限";
		g_strWoundThinning[5] = "5 - 轨头踏面处斜线状裂纹、局部凹陷和疲劳断裂";
		g_strWoundThinning[6] = "6 - 曲线下股轨头踏面剥离裂纹和浅层剥离掉块";

		g_strWoundDegree[1] = "1 - 不到轻伤";
		g_strWoundDegree[2] = "2 - 轻伤";
		g_strWoundDegree[3] = "3 - 重伤";
		g_strWoundDegree[4] = "4 - 折断";

		g_strWoundDeal[0] = "0 - 未处理";
		g_strWoundDeal[1] = "1 - 钻孔上夹板";
		g_strWoundDeal[2] = "2 - 鼓包上夹";
		g_strWoundDeal[3] = "3 - 原位复焊";
#pragma endregion

		printf("Version = %s, Path = %s\n", g_strAlgVersion, strWorkPath);

#ifdef DQ_COMBINE
		WSADATA wsaData;
		WORD wVersion;
		wVersion = MAKEWORD(2, 2);
		if (WSAStartup(wVersion, &wsaData) != 0)
		{
			return;
		}
		g_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (g_socket == INVALID_SOCKET)
		{
			return;
		}

		int timeout = 3000; //3s
		if (setsockopt(g_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
		{
			return;
		}

		SOCKADDR_IN server_addr;
		memset(&server_addr, 0, sizeof(server_addr));
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(6902);
		//inet_pton(AF_INET, "192.168.0.100", (void*)&server_addr.sin_addr.S_un.S_addr);
		inet_pton(AF_INET, "172.20.186.58", (void*)&server_addr.sin_addr.S_un.S_addr);
		//inet_pton(AF_INET, "172.20.186.107", (void*)&server_addr.sin_addr.S_un.S_addr);

		if (connect(g_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
		{
			closesocket(g_socket);
			return;
		}
#endif
		g_strWorkPath = std::string(strWorkPath, len);
		if (g_strWorkPath[len - 1] == '/')
		{
			g_strWorkPath = g_strWorkPath.substr(0, len - 1);
		}
		g_strDataPath = g_strWorkPath + "/RawData/";
		g_strLogPath = g_strWorkPath + "/AlgLog";
		CreateDirectoryA(g_strLogPath.c_str(), NULL);
		CreateDirectoryA(g_strDataPath.c_str(), NULL);
		std::string strLogFilePath = g_strLogPath + "/AlgLog.txt";
#ifdef _DEBUG
		//strLogFilePath = "./AlgLog.txt";
#endif // _DEBUG
		g_pFileLog = _fsopen(strLogFilePath.c_str(), "w", SH_DENYWR);
		if (g_pFileLog == NULL)
		{
			g_isInitialized = 0;
			return;
		}
		InitialTemplate();

		if (g_StepPoints == NULL)
		{
			g_isInitialized = 0;
		}
		g_isInitialized = 1;
		//int *p = NULL;
		//*p = 1;

		//VINT vInt;
		//vInt[1] = 0;	
#ifdef OUTPUT_EX
	}
	catch (CBaseException &e)
	{
		e.ShowExceptionInformation();
	}
#endif // OUTPUT_EX	
}

void ShutDown()
{
	g_isRunning = 0;
	Sleep(100);
	g_vWounds.clear();
	g_vPMs.clear();
	g_vBAs.clear();
	g_vER.clear();
	g_vPMs2.clear();
	g_vLC.clear();
	g_vLCP.clear();
	g_vLD.clear();
	g_sztWoundCount = 0;
	g_sztAlarmCount = 0;

	if (g_StepPoints != NULL)
	{
		delete g_StepPoints;
		g_StepPoints = NULL;
	}

	if (g_pFileB != NULL)
	{
		fclose(g_pFileB);
	}

	if (g_pFileA != NULL)
	{
		fclose(g_pFileA);
	}
	if (g_pFileLog != NULL)
	{
		fclose(g_pFileLog);
	}

	g_isInitialized = 0;
}


uint8_t GetFileInfo(char* strTPB, int len, File4Nodejs* pInfo)
{
	if (g_isInitialized == 0)
	{
		return 0;
	}
	FILE* pFileB = _fsopen(strTPB, "rb", _SH_DENYWR);
	if (pFileB == NULL)
	{
		return 0;
	}
	fseek(pFileB, 0L, SEEK_END);
	pInfo->sizeB = ftell(pFileB);

	fseek(pFileB, 0, SEEK_SET);
	F_HEAD head;
	int sz = fread(&head, 1, szFileHead, pFileB);
	fclose(pFileB);
	if (sz < szFileHead)
	{
		return 0;
	}

	pInfo->gwd = BCDToINT32(head.deviceP2.TrackSet.sectionNum);
	//printf("gwd = %d\n", pInfo->gwd);
	pInfo->section = BCDToINT16(head.deviceP2.TrackSet.regionNum);
	pInfo->group = BCDToINT16(head.deviceP2.TrackSet.teamNum);
	pInfo->railno = BCDToINT16(head.deviceP2.TrackSet.lineNum);
	pInfo->xingbie = head.deviceP2.TrackSet.lineWay;
	pInfo->xianbie = 1;
	pInfo->leftright = head.deviceP2.TrackSet.leftRight;
	pInfo->direction = head.deviceP2.TrackSet.WalkWay;

	memset(pInfo->s_time, 0, 20);
	std::string sTime = GetTimeString(head.startD, head.startT);
	strncpy(pInfo->s_time, sTime.c_str(), sTime.length());
	//printf("s_time = %s\n", pInfo->s_time);

	memset(pInfo->e_time, 0, 20);
	std::string eTime = GetTimeString(head.endD, head.endT);
	strncpy(pInfo->e_time, eTime.c_str(), eTime.length());
	//printf("e_time = %s\n", pInfo->e_time);

	strncpy(pInfo->instruNo, head.DeviceNum, 8);
	//printf("instruNo = %s\n", pInfo->instruNo);

	pInfo->s_mil = 0.000001 * head.startKm.mm + 0.001 * head.startKm.m + head.startKm.Km;
	pInfo->e_mil = 0.000001 * head.endKm.mm + 0.001 * head.endKm.m + head.endKm.Km;

	strncpy(pInfo->dataV, head.DataVerS, 8);
	//printf("dataV = %s\n", pInfo->dataV);
	strncpy(pInfo->softV, head.SoftVerS, 8);
	//printf("softV = %s\n", pInfo->softV);
	strncpy(pInfo->fpgaV, head.FPGAVerS, 8);
	//printf("fpgaV = %s\n", pInfo->fpgaV);

	pInfo->blockCount = head.block;
	pInfo->sdType = head.deviceP2.TrackSet.lineType;//上道类型0: 站线, 1: 正线
	//printf("GA_NUM = %d\n", GA_NUM);
	for (int i = 0; i < CH_N; ++i)
	{
		pInfo->Offset[i] = head.deviceP2.Place[i];
	}
	for (int i = 0; i < 4 * GA_NUM; ++i)
	{
		pInfo->Gate[i] = head.deviceP2.Gate[i];
		pInfo->Gate[i].start = GetBigGatePixel(i / 4, pInfo->Gate[i].start & 0x7FFF);
		pInfo->Gate[i].end = GetBigGatePixel(i / 4, pInfo->Gate[i].end & 0x7FFF);
	}

	char strNewTPA[500] = { 0 }, strNewTPB[500] = { 0 };
	sprintf(strNewTPA, "%s%s.tpA", g_strDataPath.c_str(), pInfo->NewFileName);
	FILE* pFileA = fopen(strNewTPA, "rb");
	if (pFileA == NULL)
	{
		printf("tpA Open Failed");
		return 0;
	}
	fseek(pFileA, 0L, SEEK_END);
	pInfo->sizeA = ftell(pFileA);
	fclose(pFileA);
	return 1;
}

void ParseER(std::vector<DQWavePoint>& vPoints, FILE* pFileEx)
{
#ifdef OUTPUT_EX
	try
	{
#endif // OUTPUT_EX
		time_t t1 = time(NULL);
		g_vER.clear();
		if (vPoints.size() == 0)
		{
			return;
		}

		if (pFileEx != NULL)
		{
			fprintf(pFileEx, "File = %s\n", g_strFileName.c_str());
			fflush(pFileEx);
		}

		const int rectSizeStep = 5, rectSizeRow = 8;
		const int meterWindow = 5000;
		const int stepWindow = 370 * 5000;//大约5000米1个窗口

		int iPointSize = vPoints.size();
		int maxStep = vPoints[iPointSize - 1].Step;
		int meterBegin = 0, stepBegin = 0;

		int stepEnd = stepBegin + stepWindow;
		int iBeginPointIndex = 0, iBeginRectIndex = 0;

		while (stepBegin < maxStep)
		{
			int xCount = ((stepEnd - stepBegin) / rectSizeStep) + 1;
			int yCount = 14;
			int rectCount = xCount * yCount;
			if (pFileEx != NULL)
			{
				fprintf(pFileEx, "xCount = %d, rectCount = %d, stepBegin = %d, stepEnd = %d\n", xCount, rectCount, stepBegin, stepEnd);
				fflush(pFileEx);
		}
			DQRect **pRect = new DQRect*[xCount];
			if (pRect == NULL)
			{
				return;
			}
			if (pFileEx != NULL)
			{
				fprintf(pFileEx, "pRect = %d\n", pRect);
				fflush(pFileEx);
			}
			for (int i = 0; i < xCount; ++i)
			{
				pRect[i] = new DQRect[yCount];
				memset(pRect[i], 0, sizeof(DQRect) * yCount);
			}
			if (pFileEx != NULL)
			{
				fprintf(pFileEx, "pRect Alloc Finish %s\n", "");
				fflush(pFileEx);
			}


#ifdef _DEBUG
			std::vector<DQWavePoint> wavePoints[16];
#endif // _DEBUG

			meterBegin = stepBegin / 370;
			iBeginRectIndex = stepBegin / 5;
			for (int i = iBeginPointIndex; i < iPointSize; ++i)
			{
				if (vPoints[i].Step >= stepEnd)
				{
					iBeginPointIndex = i;
					break;
				}

				for (int m = 0; m < 16; ++m)
				{
					if (vPoints[i].Channel & bits[m])
					{
#ifdef _DEBUG
						wavePoints[m].emplace_back(vPoints[i]);
#endif // _DEBUG
						if ((m == CH_F || m == CH_G) && vPoints[i].Row >= 40)
						{
							continue;
						}
						int xIndex = (vPoints[i].Step / rectSizeStep) - iBeginRectIndex;
						if (xIndex < 0 || xIndex >= xCount)
						{
							FILE* pFileLoggg = fopen("11111.txt", "w");
							fprintf(pFileLoggg, "stepBegin = %d, iBeginPointIndex = %d, vPoints[i].Step = %d, rectSizeStep = %d, iBeginRectIndex = %d, xIndex = %d", stepBegin, iBeginPointIndex, vPoints[i].Step, rectSizeStep, iBeginRectIndex, xIndex);
							fflush(pFileLoggg);
							fclose(pFileLoggg);
							throw;
						}
						if (m == CH_A2 || m == CH_a2 || m == CH_B2 || m == CH_b2)
						{
							pRect[xIndex][(2 * vPoints[i].JawRow - vPoints[i].Row) / rectSizeRow].Count[m - 1] ++;
						}
						else
						{
							pRect[xIndex][vPoints[i].Row / rectSizeRow].Count[m] ++;
						}
						pRect[xIndex][vPoints[i].Row / rectSizeRow].Total++;
					}
				}
		}

			int meterCount = (stepEnd - stepBegin) / 370 + 1;

#ifdef _DEBUG
			static int pCount[5001] = { 0 };
			static DQRowRect pCount2[5001][14] = { 0 };
			memset(pCount, 0, 5001 * 4);
			memset(pCount2, 0, 5001 * 14 * sizeof(DQRowRect));
#else
			int *pCount = new int[meterCount] {0};
			DQRowRect** pCount2 = new DQRowRect*[meterCount];
			for (int i = 0; i < meterCount; ++i)
			{
				pCount2[i] = new DQRowRect[yCount];
				memset(pCount2[i], 0, sizeof(DQRowRect) * yCount);
			}
#endif // _DEBUG



			if (pFileEx != NULL)
			{
				fprintf(pFileEx, "pCount = %d\n", pCount);
				fflush(pFileEx);
			}
			int thre = 50, thre2 = 100;
			for (int m = 0; m < 16; ++m)
			{
				if (m == CH_A2 || m == CH_a2 || m == CH_B2 || m == CH_b2)
				{
					continue;
				}
				int lastERBlock = meterBegin;
				if (pFileEx != NULL)
				{
					fprintf(pFileEx, "Channel = %d\n", m);
					fflush(pFileEx);
				}

				VINT vHasPoints;
				memset(pCount, 0, sizeof(int) * meterCount);
				for (int i = 0; i < xCount; ++i)
				{
					for (int j = 0; j < yCount; ++j)
					{
						if (pRect[i][j].Count[m] > 0)
						{
							int meter = i / 74;
							pCount[meter]++;
							pCount2[meter][j].Total += pRect[i][j].Count[m];
							pCount2[meter][j].Count[m] += pRect[i][j].Count[m];
							if (vHasPoints.size() > 0 && vHasPoints[vHasPoints.size() - 1] == i)
							{

							}
							else
							{
								vHasPoints.emplace_back(i);
							}
						}
					}
				}

				std::vector<ContinueRegion> vCR;
				ContinueRegion cr;
				int sz = vHasPoints.size() - 1;
				if (sz > 0)
				{
					for (int i = 0; i <= sz; ++i)
					{
						if (i == 0)
						{
							cr.vRectIndexes.emplace_back(vHasPoints[i]);
						}
						if (i == sz)
						{
							vCR.emplace_back(cr);
						}

						if (i != 0 && i != sz)
						{
							bool isIsolated = vHasPoints[i] - vHasPoints[i - 1] >= 10;
							if (isIsolated)
							{
								if (cr.vRectIndexes.size() != 0)
								{
									vCR.emplace_back(cr);
									cr.vRectIndexes.clear();
								}

								cr.vRectIndexes.emplace_back(vHasPoints[i]);
							}
							else
							{
								cr.vRectIndexes.emplace_back(vHasPoints[i]);
							}
						}
					}
				}

				for (int i = 0; i < vCR.size(); ++i)
				{
					if (vCR[i].vRectIndexes.size() <= 20)
					{
						continue;
					}


					int beginStep = (vCR[i].vRectIndexes[0] + iBeginRectIndex) * rectSizeStep;
					int endStep = (vCR[i].vRectIndexes[vCR[i].vRectIndexes.size() - 1] + iBeginRectIndex) * rectSizeStep;

					DQWavePoint dp1;
					dp1.Channel = m;
					dp1.Step = beginStep;
					dp1.Row = 0;

					DQWavePoint dp2;
					dp2.Channel = m;
					dp2.Step = endStep;
					dp2.Row = VALID_ROW - 1;

					auto itr1 = std::lower_bound(vPoints.begin(), vPoints.end(), dp1);
					auto itr2 = std::upper_bound(vPoints.begin(), vPoints.end(), dp2);

					int rowCount[VALID_ROW] = { 0 };
					for (auto itr = itr1; itr != itr2; ++itr)
					{
						if (itr->Channel & bits[m])
						{
							++rowCount[itr->Row];
						}

						if (m == CH_A1 || m == CH_a1 || m == CH_B1 || m == CH_b1)
						{
							if (itr->Channel & bits[m + 1])
							{
								++rowCount[2 * itr->JawRow - itr->Row];
							}
						}
					}


					int total = rowCount[0], maxRowIndex = 0, maxRowValue = rowCount[0];
					for (int j = 1; j < VALID_ROW; ++j)
					{
						total += rowCount[j];
						if (maxRowValue < rowCount[j])
						{
							maxRowIndex = j;
							maxRowValue = rowCount[j];
						}
					}
					int ave = m >= CH_D ? (total / VALID_ROW) : (total / 26);
					ave = ((std::max))(1, ave);
					ave += maxRowValue / 2;
					//取这些步进该通道出波的行
					VINT vRowPointCount;
					for (int j = 0; j < VALID_ROW; ++j)
					{
						if (rowCount[j] >= ave)
						{
							vRowPointCount.emplace_back(j);
						}
					}

					std::vector<Section> vSec;
					if (vRowPointCount.size() >= 1)
					{
						Section sec;
						sec.Start = vRowPointCount[0];
						uint8_t lstRow = vRowPointCount[0];
						for (int j = 0; j < vRowPointCount.size(); ++j)
						{
							if (j == vRowPointCount.size() - 1 && vRowPointCount[j] - lstRow <= 6)
							{
								sec.End = vRowPointCount[j];
								vSec.emplace_back(sec);
							}
							else if (vRowPointCount[j] - lstRow > 6)
							{
								sec.End = lstRow;
								vSec.emplace_back(sec);
								sec.Start = vRowPointCount[j];
							}
							lstRow = vRowPointCount[j];
						}
					}

					for (int j = 0; j < vSec.size(); ++j)
					{
						ErroeRect er;
						er.channel = m;
						er.step1 = beginStep;
						er.step2 = endStep;
						er.row1 = vSec[j].Start;
						er.row2 = vSec[j].End;

						int blockBegin = beginStep / 370;
						int endBlock = endStep / 370;
						int rowIndex1 = er.row1 / rectSizeRow;
						int rowIndex2 = er.row2 / rectSizeRow;
						/*for (int k = blockBegin; k >= lastERBlock; --k)
						{
							bool isOk = false;
							for (int ll = rowIndex1; ll <= rowIndex2; ++ll)
							{
								if (pCount2[k - meterBegin][ll].Count[m] >= 20)
								{
									isOk = true;
									break;
								}
							}
							if (isOk)
							{
								er.step1 = k * 370;
							}
							else
							{
								break;
							}
						}

						for (int k = endBlock; k < meterBegin + 5000; ++k)
						{
							bool isOk = false;
							for (int ll = rowIndex1; ll <= rowIndex2; ++ll)
							{
								if (pCount2[ll - meterBegin][k].Count[m] >= 20)
								{
									isOk = true;
									break;
								}
							}
							if (isOk)
							{
								er.step2 = (k + 1) * 370;
							}
							else
							{
								break;
							}
						}*/
						g_vER.emplace_back(er);
						lastERBlock = er.step1 / 370;
					}
				}
			}
#ifdef _DEBUG
#else
			for (int i = 0; i < meterCount; ++i)
			{
				delete[] pCount2[i];
			}
			delete[] pCount2;

			for (int i = 0; i < xCount; ++i)
			{
				delete[] pRect[i];
			}
			delete[] pRect;
#endif // _DEBUG
			stepBegin += stepWindow;
			stepEnd = stepBegin + stepWindow;
	}

		if (g_vER.size() == 0)
		{
			return;
		}

		for (int i = 0; i < g_vER.size(); ++i)
		{
			Pos pos1 = FindStepInBlock(g_vER[i].step1, g_vBlockHeads, 0);
			Pos pos2 = FindStepInBlock(g_vER[i].step2, g_vBlockHeads, 0);
			g_vER[i].block1 = pos1.Block;
			g_vER[i].walk1 = pos1.Walk;
			g_vER[i].block2 = pos2.Block;
			g_vER[i].walk2 = pos2.Walk;
			if (g_vER[i].channel == CH_A2 || g_vER[i].channel == CH_a2 || g_vER[i].channel == CH_B2 || g_vER[i].channel == CH_b2)
			{
				g_vER[i].channel = g_vER[i].channel - 1;
				uint8_t to = g_vER[i].row1;
				g_vER[i].row1 = 26 - g_vER[i].row2;
				g_vER[i].row2 = 26 - to;
}
		}

		uint8_t* isBad = new uint8_t[g_vER.size()];
		memset(isBad, 0, g_vER.size());
		for (int i = g_vER.size() - 1; i >= 0; --i)
		{
			if (g_vER[i].channel == CH_F || g_vER[i].channel == CH_G)
			{
				if (g_vER[i].row1 >= 40)
				{
					isBad[i] = 1;
				}
			}
		}
		VER vER = g_vER;
		g_vER.clear();
		for (int i = 0; i < vER.size(); ++i)
		{
			if (isBad[i] != 1)
			{
				g_vER.emplace_back(vER[i]);
			}
		}
		vER.clear();
		delete isBad;

		std::sort(g_vER.begin(), g_vER.end());


#ifdef _DEBUG
		time_t t2 = time(NULL);
		double timeCost = difftime(t2, t1);
		std::string strDQFilePath = g_strWorkPath + "/DQ.txt";
		FILE* pFile2 = fopen(strDQFilePath.c_str(), "w");
		fprintf(pFile2, "File = %s, TimeCost = %lf, ERCount = %d\n", g_strFileName.c_str(), timeCost, g_vER.size());
		fflush(pFile2);
		for (int i = 0; i < g_vER.size(); ++i)
		{
			fprintf(pFile2, "%s, Block1 = %d, Step1 = %d, Block2 = %d, Step2 = %d, row1 = %d, row2 = %d\n", ChannelNamesB[g_vER[i].channel].c_str(), g_vER[i].block1 + 1, g_vER[i].step1 - g_vBlockHeads[g_vER[i].block1].IndexL2, g_vER[i].block2 + 1, g_vER[i].step2 - g_vBlockHeads[g_vER[i].block2].IndexL2, g_vER[i].row1, g_vER[i].row2);
			fflush(pFile2);
		}
		fclose(pFile2);
#endif
		memset(g_indexes, 0, sizeof(int) * 16);
		if (g_vER.size() > 0)
		{
			uint8_t lstChannel = g_vER[0].channel;
			for (int i = 0; i < g_vER.size(); ++i)
			{
				if (g_vER[i].channel != lstChannel)
				{
					g_indexes[g_vER[i].channel] = i;
				}
				lstChannel = g_vER[i].channel;
			}

			int lstIndex = 0;
			for (int i = 1; i < 16; ++i)
			{
				if (g_indexes[i] == 0 && g_indexes[i - 1] != 0)
				{
					g_indexes[i] = g_indexes[i - 1];
				}
			}
		}
#ifdef OUTPUT_EX
	}
	catch (CBaseException &e)
	{
		e.ShowExceptionInformation();
	}
#endif // OUTPUT_EX
}

void GetRectDistance(TRECT rc1, TRECT rc2, int* pDistanceStep, int * pDistanceRow)
{
	if (rc1.step1 > rc2.step1)
	{
		std::swap(rc1, rc2);
	}
	if (rc1.step2 < rc2.step1)
	{
		*pDistanceStep = rc2.step1 - rc1.step2;
	}
	else
	{
		*pDistanceStep = 0;
	}

	if (rc1.row1 > rc2.row1)
	{
		std::swap(rc1, rc2);
	}

	if (rc1.row2 < rc2.row1)
	{
		*pDistanceRow = rc2.row1 - rc1.row2;
	}
	else
	{
		*pDistanceRow = 0;
	}
}

#pragma region 伤损过滤
//是否在回退区域
bool IsInBackArea(uint32_t step)
{
	bool result = false;
	for (int i = 0; i < g_vBAs.size(); ++i)
	{
		if (step >= g_vBAs[i].Pos0.Step2 && step <= g_vBAs[i].Pos1.Step2)
		{
			result = true;
			break;
		}
	}
	return result;
}



bool IsExistBackArea(uint32_t step1, uint32_t step2, VBA& vBA, int backCount)
{
	bool result = false;
	for (int i = 0; i < backCount - 1; ++i)
	{
		if (step1 <= vBA[i].Pos1.Step2 && step2 >= vBA[i + 1].Pos1.Step2)
		{
			result = true;
			break;
		}
		else if (vBA[i].Pos1.Step2 > step1)
		{
			break;
		}
	}
	return result;
}

bool IsExistBackArea(uint32_t step1, uint32_t step2, BackAction* pBA, int backCount)
{
	bool result = false;
	for (int i = 0; i < backCount - 1; ++i)
	{
		if (step1 <= pBA[i].Pos1.Step2 && step2 >= pBA[i + 1].Pos1.Step2)
		{
			result = true;
			break;
		}
		else if (pBA[i].Pos1.Step2 > step1)
		{
			break;
		}
	}
	return result;
}

void FilterBySkew()
{
	int woundCount = g_vWounds.size();

	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].IsReversed || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		if (g_vWounds[i].Type == W_SKEW_CRACK || g_vWounds[i].Type == W_SCREW_CRACK1 || g_vWounds[i].Type == W_SCREW_CRACK2 || g_vWounds[i].Type == W_SCREW_CRACK3 || g_vWounds[i].Type == W_SCREW_CRACK4)
		{
			bool isOK = true;
			for (int j = 0; j < g_vWounds[i].vCRs.size(); ++j)
			{
				if ((g_vWounds[i].vCRs[j].Channel == CH_F || g_vWounds[i].vCRs[j].Channel == CH_G) && (g_vWounds[i].vCRs[j].IsLose == 1))
				{
					continue;
				}
				else if ((g_vWounds[i].vCRs[j].Channel == CH_E || g_vWounds[i].vCRs[j].Channel == CH_D) && g_vWounds[i].vCRs[j].IsDoubleWave)
				{
				}
				else if ((g_vWounds[i].vCRs[j].Channel == CH_E || g_vWounds[i].vCRs[j].Channel == CH_D) && g_vWounds[i].vCRs[j].Row1 == g_vWounds[i].vCRs[j].Row2)
				{
				}
				else
				{
					isOK = false;
				}
			}
			if (isOK)
			{
				SetWoundFlag(g_vWounds[i], 1);
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

//尖轨
void FilterBySmart(VPM& vRising)
{
	return;

	VWJ vw = g_vWounds;
	int woundCount = g_vWounds.size();

	for (int i = 0; i < woundCount; ++i)
	{
		if (vw[i].Manual == 1 || vw[i].IsMatched == 1)
		{
			continue;
		}
		//尖轨变坡处的FG
		if (vw[i].Type == W_HORIZONAL_CRACK || vw[i].Type == W_VERTICAL_CRACK)
		{
			uint8_t count[16] = { 0 };
			uint8_t count2 = 0;
			for (int j = 0; j < vw[i].vCRs.size(); ++j)
			{
				count[vw[i].vCRs[j].Channel]++;
				count2 += vw[i].vCRs[j].Channel < CH_F ? 1 : 0;
			}

			if (count2 == 0)
			{
				/*for (int j = 0; j < vRising.size(); ++j)
				{
					if (abs((int)(vw[i].Step2 - vRising[j].Step2)) <= 60)
					{
						SetWoundFlag(vw[i], 1);
					}
				}*/
			}
		}
	}

	g_vWounds.clear();
	for (int i = 0; i < vw.size(); ++i)
	{
		if (vw[i].Flag == 0)
		{
			g_vWounds.emplace_back(vw[i]);
		}
	}
}

//相邻步进同类伤损去重
void FilterBySameType()
{
	int woundCount = g_vWounds.size();

	std::vector<TRECT> vRect;
	for (int i = 0; i < woundCount; ++i)
	{
		TRECT rect;
		GetWoundRect2(g_vWounds[i], g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03], &rect, 1);
		vRect.emplace_back(rect);
	}

	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Flag == 1 || g_vWounds[i].According.size() > 0)
		{
			continue;
		}
		int stepLimit = 25;
		if (g_vWounds[i].Manual == 1 || g_vWounds[i].Type == W_SCREW_CRACK1 || g_vWounds[i].Type == W_SCREW_CRACK2 || g_vWounds[i].Type == W_SCREW_CRACK3 || g_vWounds[i].Type == W_SCREW_CRACK4
			|| g_vWounds[i].Type == W_GUIDE_CRACK1 || g_vWounds[i].Type == W_GUIDE_CRACK2 || g_vWounds[i].Type == W_GUIDE_CRACK3 || g_vWounds[i].Type == W_GUIDE_CRACK4)
		{
			stepLimit = 5;
		}
		if (g_vWounds[i].Type == W_SCREW_HORIZON_CRACK_RIGHT || g_vWounds[i].Type == W_SCREW_HORIZON_CRACK_LEFT || g_vWounds[i].Type == W_GUIDE_HORIZON_CRACK_RIGHT || g_vWounds[i].Type == W_GUIDE_HORIZON_CRACK_LEFT)
		{
			stepLimit = 10;
		}
		if (g_vWounds[i].Type == W_HORIZONAL_CRACK)
		{
			stepLimit = 100;
		}
		if (g_vWounds[i].Type == W_DOUBLE_HOLE || g_vWounds[i].Type == W_DOUBLEHOULE_SCREW || g_vWounds[i].Type == W_DOUBLEHOULE_GUIDE)
		{
			stepLimit = 1;
		}
		int iLastIndex = i;
		for (int j = i + 1; j < woundCount; ++j)
		{
			if (g_vWounds[j].Manual == 1 || g_vWounds[j].IsMatched == 1 || g_vWounds[j].Flag == 1 || g_vWounds[j].According.size() > 0)
			{
				continue;
			}
			if (vRect[j].step1 - vRect[iLastIndex].step2 <= stepLimit && vRect[iLastIndex].step2 - vRect[j].step1 <= stepLimit &&
				(strcmp(g_vWounds[iLastIndex].Result, g_vWounds[j].Result) == 0 || (g_vWounds[i].Type == g_vWounds[j].Type && g_vWounds[i].Place == g_vWounds[j].Place))
				)
			{
				int dstep = 0, drow = 0;
				GetRectDistance(vRect[i], vRect[j], &dstep, &drow);
				if (g_vWounds[i].Type == W_SKEW_CRACK && drow > 10)
				{
					continue;
				}
				CombineWound(g_vWounds[i], g_vWounds[j]);
				iLastIndex = j;
			}
			if ((g_vWounds[j].Step2 - g_vWounds[iLastIndex].Step2 > stepLimit))
			{
				break;
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

void FilterByCombineSameTypePrev()
{
	int woundCount = g_vWounds.size();
	int		rowLCount[VALID_ROW] = { 0 };
	int		rowHCount[VALID_ROW] = { 0 };
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Flag == 1 || g_vWounds[i].Manual == 1 || g_vWounds[i].According.size() > 0)
		{
			continue;
		}

		memset(rowLCount, 0, sizeof(int) * VALID_ROW);
		memset(rowHCount, 0, sizeof(int) * VALID_ROW);
		int iJawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03];
		TRECT rect;
		GetWoundRect2(g_vWounds[i], iJawRow, &rect);
		int		place = g_vWounds[i].Place;
		int		type = g_vWounds[i].Type;
		for (int j = i + 1; j < woundCount; ++j)
		{
			if (g_vWounds[j].Manual == 1 || g_vWounds[j].Flag == 1 || g_vWounds[j].According.size() > 0)
			{
				continue;
			}
			TRECT rect2;
			GetWoundRect2(g_vWounds[j], iJawRow, &rect2);
			int dstep = 0, drow = 0;
			GetRectDistance(rect, rect2, &dstep, &drow);
			if (dstep < 10 && g_vWounds[j].Type == g_vWounds[i].Type && g_vWounds[i].Place == g_vWounds[j].Place && drow <= 10)
			{
				CombineWound(g_vWounds[i], g_vWounds[j]);
				GetWoundRect2(g_vWounds[i], iJawRow, &rect);
			}
			else if (dstep <= 5 && drow <= 5 &&
				g_vWounds[i].IsScrewHole == g_vWounds[j].IsScrewHole &&
				g_vWounds[i].IsGuideHole == g_vWounds[j].IsGuideHole &&
				g_vWounds[i].IsJoint == g_vWounds[j].IsJoint &&
				g_vWounds[i].IsSew == g_vWounds[j].IsSew)
			{
				if ((g_vWounds[i].IsScrewHole > 0 || g_vWounds[i].IsGuideHole > 0 || g_vWounds[i].IsJoint > 0 || g_vWounds[i].IsSew > 0) && g_vWounds[i].Type != g_vWounds[j].Type)
				{
					continue;
				}
				CombineWound(g_vWounds[i], g_vWounds[j]);
				GetWoundRect2(g_vWounds[i], iJawRow, &rect);
			}
			else if (dstep > 10)
			{
				break;
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

//合并相邻步进核伤，在模板匹配之前
void FilterByCombineSameType()
{
	int woundCount = g_vWounds.size();
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Flag == 1 || g_vWounds[i].Manual == 1/* || g_vWounds[i].According.size() > 0*/)
		{
			continue;
		}
		int iJawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03];

		TRECT rect;
		GetWoundRect2(g_vWounds[i], iJawRow, &rect);
		int		rowLCount[VALID_ROW] = { 0 };
		int		rowHCount[VALID_ROW] = { 0 };
		int		place = g_vWounds[i].Place;
		int		type = g_vWounds[i].Type;
		for (int j = i + 1; j < woundCount; ++j)
		{
			if (g_vWounds[j].Manual == 1 || g_vWounds[j].Flag == 1 || g_vWounds[j].IsMatched != g_vWounds[i].IsMatched)
			{
				continue;
			}
			TRECT rect2;
			GetWoundRect2(g_vWounds[j], iJawRow, &rect2);
			int dstep = 0, drow = 0;
			GetRectDistance(rect, rect2, &dstep, &drow);
			if (dstep < 10 && g_vWounds[j].Type == g_vWounds[i].Type && g_vWounds[i].Place == g_vWounds[j].Place && drow <= 10)
			{
				CombineWound(g_vWounds[i], g_vWounds[j]);
				GetWoundRect2(g_vWounds[i], iJawRow, &rect);
			}
			else if (dstep <= 5 && drow <= 5 &&
				g_vWounds[i].IsScrewHole == g_vWounds[j].IsScrewHole &&
				g_vWounds[i].IsGuideHole == g_vWounds[j].IsGuideHole &&
				g_vWounds[i].IsJoint == g_vWounds[j].IsJoint &&
				g_vWounds[i].IsSew == g_vWounds[j].IsSew)
			{
				if ((g_vWounds[i].IsScrewHole > 0 || g_vWounds[i].IsGuideHole > 0 || g_vWounds[i].IsJoint > 0 || g_vWounds[i].IsSew > 0) && g_vWounds[i].Type != g_vWounds[j].Type)
				{
					continue;
				}
				CombineWound(g_vWounds[i], g_vWounds[j]);
				GetWoundRect2(g_vWounds[i], iJawRow, &rect);
			}
			else if (drow == 0 && dstep <= 50 && rect.row1 > iJawRow && g_vWounds[i].IsScrewHole == 0 && g_vWounds[i].IsScrewHole == g_vWounds[j].IsScrewHole)
			{
				if (g_vWounds[i].IsScrewHole > 0 && g_vWounds[i].Type != g_vWounds[j].Type)
				{
					continue;
				}
				if (dstep > 30 && g_vWounds[i].Type == W_BOTTOM_TRANSVERSE_CRACK)
				{
					continue;
				}
				CombineWound(g_vWounds[i], g_vWounds[j]);
				GetWoundRect2(g_vWounds[i], iJawRow, &rect);
			}
			else if (dstep > 50)
			{
				break;
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}


//合并相邻步进核伤、轨头水平裂纹、轨颚水平裂纹
void FilterByCombineHS()
{
	int woundCount = g_vWounds.size();
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Flag == 1 || g_vWounds[i].IsMatched == 1 || g_vWounds[i].According.size() > 0 || g_vWounds[i].Manual == 1)
		{
			continue;
		}
		int iJawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03];
		if (g_vWounds[i].Type == W_HS || g_vWounds[i].Type == W_HORIZONAL_CRACK || g_vWounds[i].Type == W_SKEW_CRACK)
		{
			TRECT rect;
			GetWoundRect2(g_vWounds[i], iJawRow, &rect);
			int		rowLCount[VALID_ROW] = { 0 };
			int		rowHCount[VALID_ROW] = { 0 };
			std::map<uint16_t, uint8_t> existedtypes;
			std::map<uint16_t, uint8_t> existedplaces;
			for (int j = i + 1; j < woundCount; ++j)
			{
				if (g_vWounds[j].Manual == 1 || g_vWounds[j].IsMatched == 1 || g_vWounds[j].Flag == 1 || g_vWounds[j].According.size() > 0)
				{
					continue;
				}
				TRECT rect2;
				GetWoundRect2(g_vWounds[j], iJawRow, &rect2);
				int dstep = 0, drow = 0;
				GetRectDistance(rect, rect2, &dstep, &drow);
				if (
					(dstep < 15 && (g_vWounds[j].Type == W_HS || g_vWounds[j].Type == W_HORIZONAL_CRACK) && drow <= 5)
					|| (g_vWounds[i].Type == g_vWounds[j].Type && g_vWounds[i].Place == g_vWounds[j].Place && dstep <= 30 && drow < 10)
					)
				{
					existedtypes[g_vWounds[j].Type] = 1;
					existedplaces[g_vWounds[j].Place] = 1;
					CombineWound(g_vWounds[i], g_vWounds[j]);
					GetWoundRect2(g_vWounds[i], iJawRow, &rect);
				}
				else if (dstep > 50)
				{
					break;
				}
			}

			if ((existedplaces.find(WP_HEAD_MID)) != existedplaces.end())
			{
				g_vWounds[i].Place = WP_HEAD_MID;
				if (existedtypes.find(W_HORIZONAL_CRACK) != existedtypes.end())
				{
					g_vWounds[i].Type = W_HORIZONAL_CRACK;
					memcpy(g_vWounds[i].Result, "轨头水平裂纹", 60);
				}
				else if (existedtypes.find(W_HORIZONAL_CRACK) != existedtypes.end())
				{
					g_vWounds[i].Type = W_HORIZONAL_CRACK;
					memcpy(g_vWounds[i].Result, "轨颚水平裂纹", 60);
				}
				else
				{
					g_vWounds[i].Type = W_HS;
					memcpy(g_vWounds[i].Result, "轨头中部核伤", 60);
				}

				for (int j = i + 1; j < woundCount; ++j)
				{
					if (g_vWounds[j].Step2 - rect.step2 < 10 && (g_vWounds[j].Type == W_HS || g_vWounds[j].Type == W_HORIZONAL_CRACK))
					{
						CombineWound(g_vWounds[i], g_vWounds[j]);
					}
				}
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

//N
void FilterByConnector()
{
	VWJ vw = g_vWounds;
	g_vWounds.clear();
	int oldCount = vw.size();
	for (int i = 0; i < oldCount; ++i)
	{
		if (vw[i].Manual == 1 || vw[i].IsMatched == 1)
		{
			continue;
		}

		if (vw[i].Block % N_BLOCKREAD == N_BLOCKREAD - 1)
		{
			BlockData_B tempBlock;
			VBDB blocks;
			BlockData_A dataA;
			uint32_t iBeginFrame = 0;

			int endBlock = (std::min)((int)(vw[i].Block + 2), (int)g_vBlockHeads.size());
			int beginBlock = (std::max)(vw[i].Block - 2, 0);
			for (int k = beginBlock; k < endBlock; ++k)
			{
				tempBlock = g_vBlockHeads[k];
				if (g_isPTData)
				{
					GetBlockBStepsPT(g_pFileB, g_szFileB, g_vBlockHeads[k], tempBlock);
				}
				else
				{
					GetBlockBSteps(g_pFileB, g_szFileB, g_vBlockHeads[k], tempBlock);
				}
				blocks.emplace_back(tempBlock);
			}

			beginBlock = (std::max)(vw[i].Block - 3, 0);
			endBlock = (std::min)((int)(vw[i].Block + 3), (int)g_vBlockHeads.size());
			for (int k = beginBlock; k < endBlock; ++k)
			{
				GetBlockAFrames(g_pFileA, g_szFileA, g_vBlockHeads[k], dataA, iBeginFrame);
			}

			VWJ vwounds;
			VPM vPositionMark, vPositionMark2;
			VER vER;
			VLCP vLCP;
			VPM vPMsYOLO;
			Analyse(dataA, blocks, g_vBAs, vwounds, vPositionMark, vER, vPositionMark2, vLCP, vPMsYOLO);

			bool bFind = false;
			for (int k = 0; k < vwounds.size(); ++k)
			{
				if (vw[i].Step2 == vwounds[k].Step2 && vw[i].Type == vwounds[k].Type && vw[i].Place == vwounds[k].Place)
				{
					bFind = true;
					break;
				}
				else if (vwounds[k].Step2 > vw[i].Step2)
				{
					break;
				}
			}

			if (bFind == false)
			{
				SetWoundFlag(vw[i], 1);
			}
		}
	}
	for (int i = 0; i < oldCount; ++i)
	{
		if (vw[i].Flag == 0)
		{
			g_vWounds.emplace_back(vw[i]);
		}
	}
}

void FilterBySmall()
{
	if (g_isTestEqu)
	{
		return;
	}

	int oldCount = g_vWounds.size();
	for (int i = 0; i < oldCount; ++i)
	{
		if (g_vWounds[i].Manual == 1 || g_vWounds[i].IsMatched == 1 || g_vWounds[i].According.size() > 0)
		{
			continue;
		}
		memset(g_vWounds[i].Num, 0, 16 * 2);
		g_vWounds[i].ChannelMaxNum = 0;
		g_vWounds[i].ChannelNum = 0;
		g_vWounds[i].Flag = 0;

		Wound_Judged& wd = g_vWounds[i];
		if (wd.vCRs.size() == 0)
		{
			continue;
		}

		VCR vCRs[16];
		std::vector<A_Step> vASteps[16];
		for (int k = 0; k < wd.vCRs.size(); ++k)
		{
			wd.Num[wd.vCRs[k].Channel] += wd.vCRs[k].Region.size();
			vCRs[wd.vCRs[k].Channel].emplace_back(wd.vCRs[k]);
			for (int j = 0; j < wd.vCRs[k].vASteps.size(); ++j)
			{
				vASteps[wd.vCRs[k].Channel].emplace_back(wd.vCRs[k].vASteps[j]);
			}
		}

		int maxCh = -1;
		uint16_t maxValue = 0;
		for (int m = 0; m < 16; ++m)
		{
			if (vCRs[m].size() > 0)
			{
				++wd.ChannelNum;
				if (wd.Num[m] > maxValue)
				{
					maxCh = m;
					maxValue = wd.Num[m];
				}
			}
		}

		int total = 0;
		for (int m = 0; m < 16; ++m)
		{
			total += wd.Num[m];
		}

		if ((wd.ChannelNum == 1 || wd.Type == W_HS) && (wd.Type < W_SCREW_CRACK1 || wd.Type > W_SCREW_HORIZON_CRACK_LEFT) && wd.Type != W_HORIZONAL_CRACK && wd.Type != W_HORIZONAL_CRACK && wd.Type != W_HORIZONAL_CRACK && wd.Type != W_VERTICAL_CRACK && wd.IsJoint != 1 && wd.IsSew != 1)
		{
			if (wd.Place == WP_HEAD_MID)
			{

			}
			else if (total < 5)
			{
				/*	TRECT rect;
					uint8_t jawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 3];
					GetWoundRect2(g_vWounds[i], g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 3], &rect);
					if (wd.Type == W_HS && rect.row1 > jawRow + 2 || wd.Type != W_HS)
					{*/
				if (wd.Degree == WD_LESS || wd.Degree == WD_SMALL)
				{
					SetWoundFlag(wd, 1);
				}

				/*}
				else
				{
					SetWoundFlag(wd, 1);
				}*/
			}
		}

		if (total == 1)
		{
			SetWoundFlag(wd, 1);
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

//回退区域G出波
void FilterByBackArea()
{
	int woundCount = g_vWounds.size();
	int iBA = 0;
	int szStep = (g_vBlockHeads[0].BlockHead.probOff[ACH_G] + g_filehead.deviceP2.Place[ACH_G]) / g_filehead.step;
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Manual == 1 || g_vWounds[i].IsMatched == 1 || g_vWounds[i].According.size() > 0)
		{
			continue;
		}
		for (int j = iBA; j < g_vBAs.size(); ++j)
		{
			if (g_vWounds[i].Step2 >= g_vBAs[j].Pos1.Step2 - szStep && g_vWounds[i].Step2 <= g_vBAs[j].Pos1.Step2 && (g_vWounds[i].Type == W_VERTICAL_CRACK || g_vWounds[i].Type == W_HORIZONAL_CRACK))
			{
				iBA = j;
				uint8_t railType = g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03;
				TRECT rect;
				GetWoundRect2(g_vWounds[i], g_iJawRow[railType], &rect, 1);
				if (rect.row2 - rect.row1 > 2 || rect.row1 >= g_iScrewHoleFRowH[railType] + 1 || rect.row2 <= g_iScrewHoleFRowL[railType] - 1)
				{
					break;
				}
				else
				{
					SetWoundFlag(g_vWounds[i], 1);
					break;
				}
			}
			if (g_vWounds[i].Step2 >= g_vBAs[j].Pos0.Step2 && g_vWounds[i].Step2 <= g_vBAs[j].Pos1.Step2 && g_vWounds[i].Type == W_VERTICAL_CRACK && g_vWounds[i].IsJoint + g_vWounds[i].IsSew == 0)
			{
				SetWoundFlag(g_vWounds[i], 1);
				break;
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

//轨腰FG出波（推偏）
void FilterByFGWaist()
{
	int woundCount = g_vWounds.size();
	int maxStep = g_vBlockHeads[g_vBlockHeads.size() - 1].IndexL2 + g_vBlockHeads[g_vBlockHeads.size() - 1].vBStepDatas.size();
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Manual == 1 || g_vWounds[i].IsMatched == 1 || g_vWounds[i].Flag == 1)
		{
			continue;
		}
		if (g_vWounds[i].Type == W_HORIZONAL_CRACK || g_vWounds[i].Type == W_VERTICAL_CRACK)
		{
			if (g_vWounds[i].Type == W_VERTICAL_CRACK && maxStep - g_vWounds[i].Step2 <= 100)
			{
				for (int j = g_vWounds[i].vCRs.size() - 1; j >= 0; --j)
				{
					if (g_vWounds[i].vCRs[j].Channel >= CH_F && g_vWounds[i].vCRs[j].IsLose == 1 && maxStep <= g_vWounds[i].vCRs[j].Step2)
					{
						g_vWounds[i].vCRs.erase(g_vWounds[i].vCRs.begin() + j);
						if (g_vWounds[i].vCRs.size() == 0)
						{
							SetWoundFlag(g_vWounds[i], 1);
						}
					}
				}
			}
			if (g_vWounds[i].According.size() > 0)
			{
				continue;
			}

			uint8_t count = 0;
			VINT vFind;
			vFind.emplace_back(i);
			int row1 = g_vWounds[i].vCRs[0].Row1, row2 = g_vWounds[i].vCRs[0].Row2;
			uint8_t channel = g_vWounds[i].vCRs[0].Channel;
			int iTempStep = g_vWounds[i].vCRs[0].Step2;
			for (int j = i + 1; j < woundCount; ++j)
			{
				if (g_vWounds[j].Manual == 1 || g_vWounds[j].IsMatched == 1)
				{
					continue;
				}
				if (g_vWounds[j].Type == g_vWounds[i].Type && g_vWounds[j].vCRs[0].Channel == channel && g_vWounds[j].vCRs[0].Row1 >= row1 - 1 && g_vWounds[j].vCRs[0].Row2 <= row2 + 1)
				{
					bool isInBack = IsInBackArea(g_vWounds[j].Step2);
					if (!isInBack)
					{
						vFind.emplace_back(j);
						iTempStep = g_vWounds[j].vCRs[0].Step2;
						++count;
					}
				}
				if (g_vWounds[j].Step2 > iTempStep + 1000)
				{
					break;
				}
			}

			if (count >= 5)
			{
				for (int k = 0; k < vFind.size(); ++k)
				{
					SetWoundFlag(g_vWounds[vFind[k]], 1);
				}
			}

		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

void FilterByHorizonalCrack()
{
	std::sort(g_vWounds.begin(), g_vWounds.end());
	for (int i = 0; i < g_vWounds.size(); ++i)
	{
		if (g_vWounds[i].Type == W_HORIZONAL_CRACK)
		{
			for (int j = 0; j < g_vWounds.size(); ++j)
			{
				if (g_vWounds[j].Rect.step1 > g_vWounds[i].Rect.step2)
				{
					break;
				}
				if (g_vWounds[j].Rect.step2 < g_vWounds[i].Rect.step1 || j == i)
				{
					continue;
				}

				if (g_vWounds[j].Type == W_HORIZONAL_CRACK || g_vWounds[j].Type == W_VERTICAL_CRACK)
				{
					CombineWound(g_vWounds[i], g_vWounds[j]);
					g_vWounds[j].Flag = 1;
				}
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}


//Cc螺孔导孔出波
void FilterByCCHole()
{
	int woundCount = g_vWounds.size();
	auto itrHole = g_vHoleParas.begin();
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Manual == 1 || g_vWounds[i].IsMatched == 1 || g_vWounds[i].According.size() > 0 || g_vWounds[i].vCRs.size() == 0)
		{
			continue;
		}
		if (g_vWounds[i].Type == W_HS && g_vWounds[i].Place == WP_HEAD_MID)
		{
			int railType = g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03;
			int step1 = 0, step2 = 0;
			if (g_vWounds[i].vCRs[0].Channel == CH_C)
			{
				step1 = g_vWounds[i].vCRs[0].Step1 - 10;
				step2 = g_vWounds[i].vCRs[0].Step2 + 10;
			}
			else if (g_vWounds[i].vCRs[0].Channel == CH_c)
			{
				step1 = g_vWounds[i].vCRs[0].Step1 - 20;
				step2 = g_vWounds[i].vCRs[0].Step2 + 10;
			}

			//Todo
			for (; itrHole != g_vHoleParas.end(); ++itrHole)
			{
				if (itrHole->first >= step1 && itrHole->first <= step2 && g_vWounds[i].vCRs[0].Row1 >= g_iJawRow[railType])
				{
					//SetWoundFlag(g_vWounds[i], 1);
					for (int k = g_vWounds[i].vCRs.size() - 1; k >= 0; --k)
					{
						if (g_vWounds[i].vCRs[k].Channel <= CH_c && itrHole->second.channels[g_vWounds[i].vCRs[k].Channel] > 0)
						{
							if (g_vWounds[i].vCRs[k].Step1 >= step1 && g_vWounds[i].vCRs[k].Step1 <= step2)
							{
								RemoveWoundCR(g_vWounds[i], k);
							}
						}
					}
					break;
				}
				if (itrHole->first > step2)
				{
					break;
				}
			}
		}
	}

	for (int i = 0; i < g_vWounds.size(); ++i)
	{
		if (g_vWounds[i].Flag == 1)
		{
			g_vWounds.erase(g_vWounds.begin() + i);
		}
	}
}

//杂波区域杂波范围内出波
void FilterByER()
{
	uint32_t woundCount = g_vWounds.size();
	TRECT rect;
	uint8_t bFindThisChannel = 0, bInclude = 0;
	uint8_t isOK = true;
	VER  vER;
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Manual == 1 || g_vWounds[i].IsMatched == 1 || g_vWounds[i].vCRs.size() == 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsJoint != 0 || g_vWounds[i].IsSew != 0)
		{
			continue;
		}
		if (g_vWounds[i].Type == W_JOINT || g_vWounds[i].Type == W_SEWCH || g_vWounds[i].Type == W_SEWLR || g_vWounds[i].Type == W_SCREW_HORIZON_CRACK_RIGHT || g_vWounds[i].Type == W_SCREW_HORIZON_CRACK_LEFT || g_vWounds[i].Type == W_SCREW_CRACK1 || g_vWounds[i].Type == W_SCREW_CRACK2 || g_vWounds[i].Type == W_SCREW_CRACK3 || g_vWounds[i].Type == W_SCREW_CRACK4
			)
		{
			continue;
		}
		isOK = true;
		for (int j = 0; j < 16; ++j)
		{
			bInclude = 0;
			vER.clear();
			bFindThisChannel = GetWoundRect(g_vWounds[i], &rect, j);
			if (bFindThisChannel)
			{
				FindER(j, rect.step1, vER);
				for (int k = 0; k < vER.size(); ++k)
				{
					if (rect.step1 >= vER[k].step1 && rect.step2 <= vER[k].step2 &&
						rect.row1 >= vER[k].row1 - 3 && rect.row2 <= vER[k].row2 + 3)
					{
						bInclude = 1;
					}
				}
			}
			else
			{
				bInclude = 1;
			}

			if (bInclude == 0)
			{
				isOK = false;
				break;
			}
		}

		if (isOK)
		{
			SetWoundFlag(g_vWounds[i], 1);
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

//过滤轨底横向裂纹
void FilterBottomCrack()
{
	if (g_isTestEqu == 1)
	{
		return;
	}

	int woundCount = g_vWounds.size();
	VINT vIndexes;//所有轨底横向裂纹的索引
	double rows1 = 0, rows2 = 0;
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Manual == 1 || g_vWounds[i].IsMatched == 1 || g_vWounds[i].According.size() > 0)
		{
			continue;
		}
		if (g_vWounds[i].Type == W_BOTTOM_TRANSVERSE_CRACK)
		{
			uint8_t row1 = VALID_ROW - 1, row2 = 0;
			for (int j = 0; j < g_vWounds[i].vCRs.size(); ++j)
			{
				if (row1 > g_vWounds[i].vCRs[j].Row1)	row1 = g_vWounds[i].vCRs[j].Row1;
				if (row2 < g_vWounds[i].vCRs[j].Row2)	row2 = g_vWounds[i].vCRs[j].Row2;
			}
			double iFRow = g_vBlockHeads[g_vWounds[i].Block].BlockHead.railH / 3;
			rows1 += (row1 - iFRow);
			rows2 += (row2 - iFRow);
			vIndexes.emplace_back(i);
		}
	}

	double rowL = rows1 / g_mapWoundTypeCountTemp[W_BOTTOM_TRANSVERSE_CRACK];
	double rowH = rows2 / g_mapWoundTypeCountTemp[W_BOTTOM_TRANSVERSE_CRACK];
	for (int k = 0; k < vIndexes.size(); ++k)
	{
		int i = vIndexes[k];
		/*********************2020-06-11*******************************/
		//SetWoundFlag(g_vWounds[i], 1);
		uint8_t row1 = VALID_ROW - 1, row2 = 0;
		int countD = 0, countE = 0;
		uint8_t rowD1 = VALID_ROW - 1, rowD2 = 0, rowE1 = VALID_ROW - 1, rowE2 = 0;
		for (int j = 0; j < g_vWounds[i].vCRs.size(); ++j)
		{
			if (row1 > g_vWounds[i].vCRs[j].Row1)	row1 = g_vWounds[i].vCRs[j].Row1;
			if (row2 < g_vWounds[i].vCRs[j].Row2)	row2 = g_vWounds[i].vCRs[j].Row2;
			if (g_vWounds[i].vCRs[j].Channel == CH_D)
			{
				countD += g_vWounds[i].vCRs[j].Region.size();
				if (g_vWounds[i].vCRs[j].Row1 < rowD1)
				{
					rowD1 = g_vWounds[i].vCRs[j].Row1;
				}
				if (g_vWounds[i].vCRs[j].Row2 > rowD2)
				{
					rowD2 = g_vWounds[i].vCRs[j].Row2;
				}
			}
			else
			{
				countE += g_vWounds[i].vCRs[j].Region.size();
				if (g_vWounds[i].vCRs[j].Row1 < rowE1)
				{
					rowE1 = g_vWounds[i].vCRs[j].Row1;
				}
				if (g_vWounds[i].vCRs[j].Row2 > rowE2)
				{
					rowE2 = g_vWounds[i].vCRs[j].Row2;
				}
			}
		}
		double iFRow = g_vBlockHeads[g_vWounds[i].Block].BlockHead.railH / 3;
		if ((row1 - iFRow < rowL - 2 || row2 - iFRow > rowH + 2) && rowD2 - rowD1 + rowE2 - rowE1 <= 7 && countD + countE <= 15)
		{
			SetWoundFlag(g_vWounds[i], 1);
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

//合并鱼鳞伤
void FilterByCombineFish()
{
	int woundCount = g_vWounds.size();
	for (int i = 0; i < woundCount; ++i)
	{
		if (g_vWounds[i].Flag == 1 || g_vWounds[i].Manual == 1 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		int iJawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03];
		if (g_vWounds[i].Type == W_YLS)
		{
			TRECT rect;
			GetWoundRect2(g_vWounds[i], iJawRow, &rect);
			int		rowLCount[VALID_ROW] = { 0 };
			int		rowHCount[VALID_ROW] = { 0 };
			int		place = g_vWounds[i].Place;
			int		type = g_vWounds[i].Type;
			for (int j = i + 1; j < woundCount; ++j)
			{
				if (g_vWounds[j].Manual == 1 || g_vWounds[j].Flag == 1 || g_vWounds[j].IsMatched == 1)
				{
					continue;
				}
				TRECT rect2;
				GetWoundRect2(g_vWounds[j], iJawRow, &rect2);
				int dstep = 0, drow = 0;
				GetRectDistance(rect, rect2, &dstep, &drow);
				if (dstep < 100 && g_vWounds[j].Type == W_YLS && g_vWounds[i].Place == g_vWounds[j].Place)
				{
					place |= g_vWounds[j].Place;
					type |= g_vWounds[j].Type;
					CombineWound(g_vWounds[i], g_vWounds[j]);
					GetWoundRect2(g_vWounds[i], iJawRow, &rect);
				}
				else if (dstep >= 100)
				{
					break;
				}
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Flag == 0 || g_vWounds[i].Manual != 0 || g_vWounds[i].According.size() > 0 || g_vWounds[i].IsMatched == 1)
		{
			continue;
		}
		g_vWounds.erase(g_vWounds.begin() + i);
	}
}

void FilterByNoCR()
{
	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].vCRs.size() == 0 && g_vWounds[i].According.size() == 0 && g_vWounds[i].Manual == 0)
		{
			g_vWounds.erase(g_vWounds.begin() + i);
		}
	}
}

void FilterByJointReflectWavw()
{
	std::sort(g_vJWRD.begin(), g_vJWRD.end(), [&](JointReflectWave& a, JointReflectWave& b) {return a.cr.Step1 < b.cr.Step1; });
	for (int i = g_vJWRD.size() - 1; i >= 1; --i)
	{
		if (g_vJWRD[i].cr.Step1 == g_vJWRD[i - 1].cr.Step1 && g_vJWRD[i].cr.Step2 == g_vJWRD[i - 1].cr.Step2)
		{
			g_vJWRD.erase(g_vJWRD.begin() + i);
			if (i < g_vJWRD.size() - 1)
			{
				i++;
			}
		}
	}
	int rowLimitD[4][2] = { 0 };
	int rowLimitD2[4][2] = { 0 };
	int countD[4] = { 0 };
	for (int i = 0; i < g_vJWRD.size(); ++i)
	{
		rowLimitD[g_vJWRD[i].RailType][0] += g_vJWRD[i].cr.Row1;
		rowLimitD[g_vJWRD[i].RailType][1] += g_vJWRD[i].cr.Row2;
		countD[g_vJWRD[i].RailType] ++;
	}

	for (int i = 0; i < 4; ++i)
	{
		if (countD[i] > 0)
		{
			rowLimitD2[i][0] = 1.0 * rowLimitD[i][0] / countD[i] + 0.5;
			rowLimitD2[i][1] = 1.0 * rowLimitD[i][1] / countD[i] + 0.5;
		}
	}

	std::sort(g_vJWRE.begin(), g_vJWRE.end(), [&](JointReflectWave& a, JointReflectWave& b) {return a.cr.Step1 < b.cr.Step1; });
	for (int i = g_vJWRE.size() - 1; i >= 1; --i)
	{
		if (g_vJWRE[i].cr.Step1 == g_vJWRE[i - 1].cr.Step1 && g_vJWRE[i].cr.Step2 == g_vJWRE[i - 1].cr.Step2)
		{
			g_vJWRE.erase(g_vJWRE.begin() + i);
			if (i < g_vJWRE.size() - 1)
			{
				i++;
			}
		}
	}
	int rowLimitE[4][2] = { 0 };
	int rowLimitE2[4][2] = { 0 };
	int countE[4] = { 0 };
	for (int i = 0; i < g_vJWRE.size(); ++i)
	{
		rowLimitE[g_vJWRE[i].RailType][0] += g_vJWRE[i].cr.Row1;
		rowLimitE[g_vJWRE[i].RailType][1] += g_vJWRE[i].cr.Row2;
		countE[g_vJWRE[i].RailType] ++;
	}

	for (int i = 0; i < 4; ++i)
	{
		if (countE[i] > 0)
		{
			rowLimitE2[i][0] = 1.0 * rowLimitE[i][0] / countE[i] + 0.5;
			rowLimitE2[i][1] = 1.0 * rowLimitE[i][1] / countE[i] + 0.5;
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Type == W_SKEW_CRACK)
		{
			int rt = g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03;
			for (int j = g_vWounds[i].vCRs.size() - 1; j >= 0; --j)
			{
				CR& cr = g_vWounds[i].vCRs[j];
				if (cr.Channel == CH_D && cr.IsJoint == 1)
				{
					if (Abs(cr.Row1 - rowLimitD2[rt][0]) <= 1 && Abs(cr.Row2 - rowLimitD2[rt][1]) <= 1)
					{
						g_vWounds[i].vCRs.erase(g_vWounds[i].vCRs.begin() + j);
					}
				}
				else if (cr.Channel == CH_E && cr.IsJoint == 1)
				{
					if (Abs(cr.Row1 - rowLimitE2[rt][0]) <= 1 && Abs(cr.Row2 - rowLimitE2[rt][1]) <= 1)
					{
						g_vWounds[i].vCRs.erase(g_vWounds[i].vCRs.begin() + j);
					}
				}
			}
		}
	}

}


#pragma endregion

uint32_t IsMatch(int iMarkIndex1, int iMarkIndex2, int itemplateIndex, std::map<int, int>& mapIndexes)
{
	uint32_t step2 = g_vPMs[iMarkIndex1].Step2, step1 = g_vPMs[iMarkIndex2].Step2;
	VWJ vw;
	int iBgeinIndex = -1;
	for (int i = 0; i < g_vWounds.size(); ++i)
	{
		if (g_vWounds[i].Step2 >= step1 - 20 && g_vWounds[i].Step2 <= step2 + 20)
		{
			if (vw.size() == 0)
			{
				iBgeinIndex = i;
			}
			if (g_vWounds[i].StepLen < 0 && g_vWounds[i].Manual == 0)
			{
				TRECT rect;
				GetWoundRect2(g_vWounds[i], 13, &rect, 1);
				g_vWounds[i].Step2 = rect.step1;
				g_vWounds[i].StepLen = rect.step2 - rect.step1 + 1;
				g_vWounds[i].Row1 = rect.row1;
				g_vWounds[i].RowLen = rect.row2 - rect.row1 + 1;

			}
			vw.emplace_back(g_vWounds[i]);
		}
		if (g_vWounds[i].Step2 > step2)
		{
			break;
		}
	}

	railTemplate& rt = g_vTemplate[itemplateIndex];

	if (vw.size() < rt.woundCount / 2)
	{
		return 0;
	}

	int matchCount = 0;
	int deltStep = step1;
	int lastd = 0;
	int j = 0;
	int iLasti = -1, iLastj = -1;
	for (int i = 0; i < rt.woundCount; ++i)
	{
		j = j - 5 >= 0 ? j - 5 : 0;
		for (; j < vw.size(); ++j)
		{
			if (mapIndexes.find(j + iBgeinIndex) != mapIndexes.end())
			{
				continue;
			}
			int ts = vw[j].Step2 - deltStep - rt.vWounds[i].Step2;
			int ts2 = ts;
			if (mapIndexes.size() > 0)
			{
				ts2 = vw[j].Step2 - rt.vWounds[i].Step2 - (vw[iLastj].Step2 - rt.vWounds[iLasti].Step2);
			}
			if ((ts >= -20 && ts <= 20 || ts2 >= -20 && ts2 <= 20) && (vw[j].Type == rt.vWounds[i].Type || vw[j].Type == W_YLS && rt.vWounds[i].Type == W_HS))
			{
				if (rt.vWounds[i].Place != 0 && vw[j].Place != rt.vWounds[i].Place)
				{
					continue;
				}

				int r1 = 0, r2 = 0;
				int ovl = GetOverlappedStep(rt.vWounds[i].Row1, rt.vWounds[i].Row1 + rt.vWounds[i].RowLen, vw[j].Row1, vw[j].Row1 + vw[j].RowLen, r1, r2);
				if (ovl <= 0)
				{
					continue;
				}

				matchCount++;
				mapIndexes[j + iBgeinIndex] = i;
				iLasti = i;
				iLastj = j;
				++j;
				break;
			}
			else if (ts > 20 && ts2 > 20)
			{
				break;
			}
		}
	}

	if (matchCount >= 0.7 * rt.woundCount)
	{
		return 1;
	}
	return 0;
}

void ReviseWalk(std::vector< AlignedBack> &vAligned)
{
	for (int i = 0; i < vAligned.size(); ++i)
	{
		Pos pos1 = FindStepInBlock(vAligned[i].Step1, g_vBlockHeads, 0);
		Pos Pos2 = FindStepInBlock(vAligned[i].Step2, g_vBlockHeads, 0);
	}
}

void FindMarksInBackAction(uint32_t step1, uint32_t step2, VPM& vAll, VPM& vFind)
{
	for (int i = 0; i < vAll.size(); ++i)
	{
		if (vAll[i].Step2 >= step1 && vAll[i].Step2 <= step2 &&
			(vAll[i].Mark == PM_JOINT2 || vAll[i].Mark == PM_JOINT4 || vAll[i].Mark == PM_JOINT6 || vAll[i].Mark == PM_SEW_CH || vAll[i].Mark == PM_SEW_LRH || vAll[i].Mark == PM_SEW_LIVE)
			)
		{
			vFind.emplace_back(vAll[i]);
		}
		if (vAll[i].Step2 > step2)
		{
			break;
		}
	}
}



bool cmp1(std::pair<int, int>a, std::pair<int, int>b)
{
	return a.first < b.first;
}

std::pair<int*, int> merge_main(int buff[], int size)
{
	std::vector<std::pair<int, int> > vec_sort;
	std::vector<std::pair<int, int> > vec_merge;
	std::pair<int*, int> return_pair;
	std::pair<int, int> input_pair;
	std::pair<int, int> lower;
	int *result = (int*)malloc(size * sizeof(int));
	int rst_cnt = 0;
	memset(result, 0, sizeof(result));
	int upper_bound;
	int a, b;
	int i = 0;
	while (i < size)
	{
		if (i % 2 == 0)
		{
			input_pair.first = buff[i];
			input_pair.second = buff[i + 1];
			vec_sort.emplace_back(input_pair);
		}
		i = i + 2;

	}
	/*while (inFile >> a >> b)
	{
		vec_sort.emplace_back({ a, b });
	}*/
	sort(vec_sort.begin(), vec_sort.end(), cmp1);
	std::vector<std::pair<int, int> >::iterator it;
	for (it = vec_sort.begin(); it != vec_sort.end(); it++)
	{
		if (vec_merge.empty())
		{
			input_pair.first = it->first;
			input_pair.second = it->second;
			vec_merge.emplace_back(input_pair);
		}
		else
		{
			lower = vec_merge.back();
			if (it->first <= lower.second)
			{
				upper_bound = (std::max)(lower.second, it->second);
				vec_merge.pop_back();
				input_pair.first = lower.first;
				input_pair.second = lower.second;
				vec_merge.emplace_back(input_pair);
			}
			else
			{
				input_pair.first = it->first;
				input_pair.second = it->second;
				vec_merge.emplace_back(input_pair);
			}
		}
	}
	std::vector<std::pair<int, int> >::iterator it_1;
	for (it_1 = vec_merge.begin(); it_1 != vec_merge.end(); it_1++)
	{
		result[rst_cnt] = it_1->first;
		result[rst_cnt + 1] = it_1->second;
		rst_cnt += 2;
		printf("%d,%d\n", it_1->first, it_1->second);
		//cout << it->first << "," << it_1->second << endl;
	}
	return_pair.first = result;
	return_pair.second = rst_cnt;
	printf("rst_cnt = %d\n", rst_cnt);
	// test
	/*std::vector<pair<int, int>>::iterator it_2;
	for (it_2 = vec_merge.begin(); it_2 != vec_merge.end(); it_2++)
	{
		cout << it_2->first << "," << it_2->second << endl;

	}*/
	return return_pair;
}



void  FillForkPara(Fork& fork)
{
	if (fork.Begin.Data == 0 && fork.End.Data != 0)
	{
		fork.Begin.Data = fork.End.Data;
		fork.Begin.ChannelNum = fork.End.ChannelNum;
		fork.ForkNo = fork.End.Data;
		fork.Bits = fork.End.ChannelNum;
	}
	else if (fork.Begin.Data != 0 && fork.End.Data == 0)
	{
		fork.End.Data = fork.Begin.Data;
		fork.End.ChannelNum = fork.Begin.ChannelNum;
		fork.ForkNo = fork.End.Data;
		fork.Bits = fork.End.ChannelNum;
	}
	else if (fork.Begin.Data == 0 && fork.End.Data == 0)
	{
		for (int j = fork.Begin.Block; j < fork.End.Block; ++j)
		{
			if (g_vBlockHeads[j].BlockHead.swNum != 0)
			{
				fork.ForkNo = BCDToINT16(g_vBlockHeads[j].BlockHead.swNum);
				fork.Bits = g_vBlockHeads[j].BlockHead.BitS;
				break;
			}
		}
	}
}


void ReviseWoundPlace()
{
	uint8_t chs[14] = { 0 };
	uint8_t	row1[14] = { 0 };
	uint8_t	row2[14] = { 0 };
	for (int i = 0; i < g_vWounds.size(); ++i)
	{
		if (g_vWounds[i].Manual == 1)
		{
			continue;
		}
		if (g_vWounds[i].Type == W_HS || g_vWounds[i].Type == W_YLS || g_vWounds[i].Type == W_JOINT || g_vWounds[i].Type == W_SEWLR || g_vWounds[i].Type == W_SEWXCH || g_vWounds[i].Type == W_SEWCH)
		{
			int iJawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03];
			int iFRow = rail_uDC[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03] / 3;
			bool carType = g_vBlockHeads[g_vWounds[i].Block].BlockHead.detectSet.Identify & BIT2;// 车型，1-右手车，0-左手车
			memset(chs, 0, 14);
			memset(row1, 0xFF, 14);
			memset(row2, 0, 14);
			for (int j = 0; j < g_vWounds[i].vCRs.size(); ++j)
			{
				uint8_t ch = g_vWounds[i].vCRs[j].Channel;
				if (ch < CH_F)
				{
					chs[ch] = 1;
					if (g_vWounds[i].vCRs[j].Row1 < row1[ch])
					{
						row1[ch] = g_vWounds[i].vCRs[j].Row1;
					}
					if (g_vWounds[i].vCRs[j].Row2 > row2[ch])
					{
						row2[ch] = g_vWounds[i].vCRs[j].Row2;
					}
				}
			}
			bool bAa = chs[CH_A1] + chs[CH_A2] + chs[CH_a1] + chs[CH_a2];
			bool bBd = chs[CH_B1] + chs[CH_B2] + chs[CH_b1] + chs[CH_b2];
			bool bCc = chs[CH_C] + chs[CH_c];
			bool bDE = chs[CH_D] + chs[CH_E];
			if (bCc)
			{
				g_vWounds[i].Place = WP_HEAD_MID;
			}
			else if (bDE)
			{
				if (chs[CH_D] > 0 && row2[CH_D] < iJawRow - 3 || chs[CH_E] > 0 && row2[CH_E] < iJawRow - 3)
				{
					g_vWounds[i].Place = WP_HEAD_MID;
				}
				else if (chs[CH_D] > 0 && row2[CH_D] > iFRow - 5 || chs[CH_E] > 0 && row2[CH_E] > iFRow - 5)
				{
					g_vWounds[i].Place = WP_BOTTOM;
				}
				else if (chs[CH_D] > 0 && row2[CH_D] < iJawRow + 3 || chs[CH_E] > 0 && row2[CH_E] < iJawRow + 3)
				{
					g_vWounds[i].Place = WP_WAIST;
				}
				else
				{
					g_vWounds[i].Place = WP_JAW_IN;
				}
			}
			else
			{
				if (carType == true && bAa || carType == false && bBd)
				{
					g_vWounds[i].Place = WP_HEAD_IN;
				}
				else
				{
					g_vWounds[i].Place = WP_HEAD_OUT;
				}
			}
		}
	}

	for (int i = g_vWounds.size() - 1; i >= 0; --i)
	{
		if (g_vWounds[i].Type == W_HS)
		{
			memset(chs, 0, 14);
			int chCount = 0;
			for (int j = 0; j < g_vWounds[i].vCRs.size(); ++j)
			{
				if (g_vWounds[i].vCRs[j].Channel < CH_F)
				{
					if (chs[g_vWounds[i].vCRs[j].Channel] == 0)
					{
						chCount++;
					}
					chs[g_vWounds[i].vCRs[j].Channel] = 1;
				}
			}

			if (chCount == 0)
			{
				g_vWounds.erase(g_vWounds.begin() + i);
				continue;
			}
		}

		for (int i = 0; i < g_vWounds.size(); ++i)
		{
			uint8_t isjoint = g_vWounds[i].IsJoint;
			uint8_t issew = g_vWounds[i].IsSew;
			for (int j = 0; j < g_vWounds[i].vCRs.size(); ++j)
			{
				if (g_vWounds[i].vCRs[j].IsJoint > 0)
				{
					isjoint = g_vWounds[i].vCRs[j].IsJoint;
					break;
				}
				if (g_vWounds[i].vCRs[j].IsSew > 0)
				{
					issew = g_vWounds[i].vCRs[j].IsSew;
					break;
				}
			}
			g_vWounds[i].IsJoint = isjoint;
			g_vWounds[i].IsSew = issew;
		}
	}
}


void MultiCycleAlign()
{
	if (g_PrevFileCount < 1)
	{
		return;
	}

#pragma region 精对齐数据准备

	std::string lastFolder = g_strWorkPath + "/Data1";
	std::string currFolder = g_strWorkPath + "/Data2";

	CreateDirectoryA(lastFolder.c_str(), NULL);//上周期文件
	CreateDirectoryA(currFolder.c_str(), NULL);//本周期文件

	FileSolveInfo& prevFile = g_PrevFileInfo[g_PrevFileCount - 1];

	std::string strFileMarkPrev = lastFolder + "/mark.txt";// "D:/Files/Data1/mark.txt";
	FILE* pFile = fopen(strFileMarkPrev.c_str(), "w");
	for (int i = 0; i < prevFile.MarkCount; ++i)
	{
		//fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d,%d\n", pMarksPrev[i].Block, pMarksPrev[i].Mark, pMarksPrev[i].Walk2, pMarksPrev[i].Size, pMarksPrev[i].ScrewHoleCount, pMarksPrev[i].Manual, pMarksPrev[i].Step);
		fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d\n", prevFile.Marks[i].Block, prevFile.Marks[i].Mark, prevFile.Marks[i].Walk2, prevFile.Marks[i].Size, prevFile.Marks[i].ScrewHoleCount, prevFile.Marks[i].Manual);
	}
	fclose(pFile);

	std::string strFileBlockPrev = lastFolder + "/block.txt";// "D:/Files/Data1/block.txt";
	pFile = fopen(strFileBlockPrev.c_str(), "w");
	for (int i = 0; i < prevFile.BlockCount; ++i)
	{
		//fprintf(pFile, "%d,%.6lf,%lf,%lf,%d,%d,%d,%d\n", pFiles->Blocks[i].Index, pFiles->Blocks[i].Walk2, pFiles->Blocks[i].gpsLog, pFiles->Blocks[i].gpsLat, pFiles->Blocks[i].blockHead.swNum, pFiles->Blocks[i].blockHead.railType, pFiles->Blocks[i].blockHead.railH, pFiles->Blocks[i].blockHead.row);
		fprintf(pFile, "%d,%.6lf,%lf,%lf,%d,%d,%d\n", prevFile.Blocks[i].Index, prevFile.Blocks[i].Walk2, prevFile.Blocks[i].gpsLog, prevFile.Blocks[i].gpsLat, prevFile.Blocks[i].blockHead.swNum, prevFile.Blocks[i].blockHead.railType, prevFile.Blocks[i].blockHead.railH);
	}
	fclose(pFile);

	std::string strFileBackPrev = lastFolder + "/back.txt";// "D:/Files/Data1/back.txt";
	pFile = fopen(strFileBackPrev.c_str(), "w");
	for (int i = 0; i < prevFile.BackCount; ++i)
	{
		if (prevFile.Backs[i].Pos1.Block >= prevFile.BlockCount)
		{
			continue;
		}
		fprintf(pFile, "%d, %lf\n", prevFile.Backs[i].Pos1.Block, prevFile.Blocks[prevFile.Backs[i].Pos1.Block + 1].Walk - prevFile.Backs[i].Pos1.Walk);
	}
	fclose(pFile);

	std::string strFileHeadPrev = lastFolder + "/filehead.txt";// "D:/Files/Data1/filehead.txt";
	pFile = fopen(strFileHeadPrev.c_str(), "w");
	int idx = -1, iFindCount = 0;
	for (int i = FILENAME_LEN - 1; i >= 0; --i)
	{
		if (prevFile.DataPath[i] == '_')
		{
			iFindCount++;
			if (iFindCount == 2)
			{
				idx = i;
				break;
			}
		}
	}

	fprintf(pFile, "%d\n", prevFile.DataPath[idx - 8] == 'S' ? 1 : 0);
	fclose(pFile);

	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "SN = %d\n", prevFile.FileName[idx - 8] == 'S' ? 1 : 0);
		fflush(g_pFileLog);
	}

	bool bDirectionPrev = prevFile.FileName[idx - 8] == 'S' ? 1 : 0;

	std::string strFileWoundPrev = lastFolder + "/wound.txt";// "D:/Files/Data1/wound.txt";
	pFile = fopen(strFileWoundPrev.c_str(), "w");
	for (int i = 0; i < prevFile.WoundCount; ++i)
	{
		fprintf(pFile, "%d,%lf,%d\n", prevFile.Wounds[i].Block, prevFile.Wounds[i].Walk2, prevFile.Wounds[i].Type);
	}
	fclose(pFile);


	std::string strFileMark = currFolder + "/mark.txt";// "D:/Files/Data2/mark.txt";
	pFile = fopen(strFileMark.c_str(), "w");
	for (int i = 0; i < g_vPMs.size(); ++i)
	{
		//fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d,%d\n", pMarks[i].Block, pMarks[i].Mark, pMarks[i].Walk2, pMarks[i].Size, pMarks[i].ScrewHoleCount, pMarks[i].Manual, pMarks[i].Step);
		fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d\n", g_vPMs[i].Block, g_vPMs[i].Mark, g_vPMs[i].Walk2, g_vPMs[i].Size, g_vPMs[i].ScrewHoleCount, g_vPMs[i].Manual);
	}
	fclose(pFile);

	std::string strFileBlock = currFolder + "/block.txt";//"D:/Files/Data2/block.txt";
	pFile = fopen(strFileBlock.c_str(), "w");
	for (int i = 0; i < g_vBlockHeads.size(); ++i)
	{
		//fprintf(pFile, "%d,%.6lf,%lf,%lf,%d,%d,%d,%d\n", pBlocks[i].Index, pBlocks[i].Walk, pBlocks[i].gpsLog, pBlocks[i].gpsLat, pBlocks[i].blockHead.swNum, pBlocks[i].blockHead.railType, pBlocks[i].blockHead.railH, pBlocks[i].blockHead.row);
		fprintf(pFile, "%d,%.6lf,%lf,%lf,%d,%d,%d\n", g_vBlockHeads[i].Index, g_vBlockHeads[i].Walk, g_vBlockHeads[i].gpsLog, g_vBlockHeads[i].gpsLat, g_vBlockHeads[i].BlockHead.swNum, g_vBlockHeads[i].BlockHead.railType, g_vBlockHeads[i].BlockHead.railH);
	}
	fclose(pFile);

	std::string strBack = currFolder + "/back.txt";//"D:/Files/Data2/back.txt";
	pFile = fopen(strBack.c_str(), "w");
	for (int i = 0; i < g_vBAs.size(); ++i)
	{
		if (g_vBAs[i].Pos1.Block >= g_vBlockHeads.size())
		{
			continue;
		}
		fprintf(pFile, "%d, %lf\n", g_vBAs[i].Pos1.Block, g_vBlockHeads[g_vBAs[i].Pos1.Block + 1].Walk - g_vBAs[i].Pos1.Walk);
	}
	fclose(pFile);

	std::string strFileHead = currFolder + "/filehead.txt";//"D:/Files/Data2/filehead.txt";
	pFile = fopen(strFileHead.c_str(), "w");
	fprintf(pFile, "%d\n", g_direction ? 1 : 0);
	fclose(pFile);

	std::string strFileWound = currFolder + "/wound.txt";//"D:/Files/Data2/wound.txt";
	pFile = fopen(strFileWound.c_str(), "w");
	for (int i = 0; i < g_vWounds.size(); ++i)
	{
		fprintf(pFile, "%d,%lf,%d\n", g_vWounds[i].Block, g_vWounds[i].Walk, g_vWounds[i].Type);
	}
	fclose(pFile);

	std::string strFileOut = g_strWorkPath + "/result.txt";//"D:/Files/result.txt";
	std::string strFileOut2 = g_strWorkPath + "/result2.txt";//"D:/Files/result2.txt";
	remove(strFileOut.c_str());
	remove(strFileOut2.c_str());


	WriteLog("Align Begin\n");
	time_t t1 = time(NULL);
	//int nRet = (int)ShellExecuteA(NULL, "open", "cmd.exe", "/c python D:\\Files\\main.py D:\\Files\\Data2\\ D:\\Files\\Data1\\ D:\\Files\\result.txt D:\\Files\\result2.txt", NULL, SW_HIDE);
	(int)ShellExecuteA(NULL, "open", "cmd.exe", "/c python ./main.py ./Data2/ ./Data1/ ./result.txt ./result2.txt", g_strWorkPath.c_str(), SW_HIDE);
	WriteLog("Read Result Begin\n");
	while (1)
	{
		time_t t2 = time(NULL);
		double timeCost = difftime(t2, t1);
		pFile = fopen(strFileOut2.c_str(), "r+");
		if (pFile != NULL)
		{
			Sleep(1);
			fprintf(pFile, "Timecost = %lf\n", timeCost);
			fflush(pFile);
			fclose(pFile);
			break;
		}
		if (timeCost >= 120)
		{
			WriteLog("GetResult2 timeCost > 60s\n");
			break;
		}
		Sleep(10);
	}

	pFile = fopen(strFileOut.c_str(), "r");
	if (pFile == NULL)
	{
		WriteLog("GetResult2 Result File Open failed\n");
		WriteLog("LastCycleFile: ");
		WriteLog(prevFile.FileName);
		WriteLog("\tLastCycleFile: ");
		WriteLog(prevFile.DataPath);
		WriteLog("\n");
		return;
	}

	int nBlock;
	double wd;

	char sztemp[20];
	fread(sztemp, 1, 20, pFile);
	int idxDrumma = -1, idxDot = -1;
	for (int i = 0; i < 20; ++i)
	{
		if (sztemp[i] == '.')
		{
			idxDot = i;
			if (idxDrumma < 0)
			{
				fclose(pFile);
				WriteLog("MultiCycle Error\n");
				remove(strFileOut.c_str());
				remove(strFileOut2.c_str());
				return;
			}
		}
		if (sztemp[i] == ',')
		{
			idxDrumma = i;
		}
	}

	fseek(pFile, 0, SEEK_SET);
	while (!feof(pFile))
	{
		fscanf(pFile, "%d, %lf", &nBlock, &wd);
		g_vBlockHeads[nBlock].Walk2 = wd;
	}
	fclose(pFile);
	WriteLog("Read Result End\n");
#pragma endregion

	//std::string newFileResultName = "D:/Files/";
	//newFileResultName += g_strFileName;
	//newFileResultName += "-";
	//newFileResultName += pFiles[0].FileName;
	//newFileResultName += ".txt";
	//::CopyFileA(strFileOut.c_str(), newFileResultName.c_str(), false);


	//先计算位置标的值，再更新米块的起始里程
	double delt2 = g_direction == 1 ? 0.000001 * g_filehead.step : -0.000001 * g_filehead.step;
	for (int i = 0; i < g_vPMs.size(); ++i)
	{
		if (g_vPMs[i].Mark != PM_WALKREVISE)
		{
			g_vPMs[i].Walk2 = g_vBlockHeads[g_vPMs[i].Block].Walk2 + delt2 * g_vPMs[i].Step;
		}
	}

	for (int i = 0; i < g_vWounds.size(); ++i)
	{
		double w = g_vBlockHeads[g_vWounds[i].Block].Walk2 + delt2 * g_vWounds[i].Step;
		g_vWounds[i].Walk2 = w;
	}

	remove(strFileOut.c_str());
	remove(strFileOut2.c_str());

	int8_t* flags = new int8_t[g_vWounds.size()]();
	int8_t* flagsPrev = new int8_t[g_PrevFileInfo[g_PrevFileCount - 1].WoundCount]();

	for (int i = 0; i < g_vWounds.size(); ++i)
	{

	}

}

bool FindStepByWalk2(double walk2, bool direction, BLOCK_B4Nodejs* blocks, int blockCount, std::vector<Pos>& vPos, int iBeginBlock)
{
	if (direction)
	{
		for (int i = iBeginBlock; i < blockCount; ++i)
		{
			if (blocks[i].Walk2 < walk2 && blocks[i].Walk2 + 0.000001 * 2.66 * blocks[i].blockHead.row >= walk2)
			{
				Pos pos;
				pos.Block = i;
				pos.Step = (walk2 - blocks[i].Walk2) / (0.000001 * 2.66);
				pos.Step2 = blocks[i].IndexL2 + pos.Step;
				vPos.push_back(pos);
			}
		}
	}
	else
	{
		for (int i = iBeginBlock; i < blockCount; ++i)
		{
			if (blocks[i].Walk2 >= walk2 && blocks[i].Walk2 - 0.000001 * 2.66 * blocks[i].blockHead.row <= walk2)
			{
				Pos pos;
				pos.Block = i;
				pos.Step = (blocks[i].Walk2 - walk2) / (0.000001 * 2.66);
				pos.Step2 = blocks[i].IndexL2 + pos.Step;
				vPos.push_back(pos);
			}
		}
	}
	return vPos.size() > 0;
}


void AnalyseSection(int beginBlock, int beginStep2, int endBlock, int endStep2, SectionAnalyseResult& result)
{
	result.beginBlock = beginBlock;
	result.endBlock = endBlock;
	result.beginStep = beginStep2;
	result.endStep = endStep2;
	BlockData_B tempBlock;
	BlockData_B tempBlock2;
	BackAction  ba;
	BlockData_A vAFrames;
	VBDB        vBdatas(40);
	uint32_t    iBeginFrame = 0;
	uint32_t    szEnd = g_szFileB - szFileHead;
	uint32_t	blockCount = g_vBlockHeads.size();
	vBdatas.clear();
	vAFrames.vAStepDatas.clear();

	for (int i = beginBlock; i <= endBlock && i < blockCount; ++i)
	{
		tempBlock.vBStepDatas.clear();
		int indexL2 = g_vBlockHeads[i].IndexL2;
		if (g_isPTData)
		{
			GetBlockBStepsPT(g_pFileB, g_szFileB, g_vBlockHeads[i], tempBlock);
		}
		else
		{
			GetBlockBSteps(g_pFileB, g_szFileB, g_vBlockHeads[i], tempBlock);
		}
		vBdatas.emplace_back(tempBlock);
	}

	for (int i = beginBlock == 0 ? beginBlock : beginBlock - 1; i < endBlock + 1 && i < blockCount; ++i)
	{
		GetBlockAFrames(g_pFileA, g_szFileA, g_vBlockHeads[i], vAFrames, iBeginFrame);
	}

	int woundPrev = g_vWounds.size();
	VBA vBA;
	VER vER;
	VLCP vLCP;
	VPM vPMsYOLO;
	Analyse(vAFrames, vBdatas, vBA, result.vWounds, result.vPM, vER, result.vPMs2, vLCP, vPMsYOLO);
	std::sort(result.vWounds.begin(), result.vWounds.end());
	std::sort(result.vPM.begin(), result.vPM.end(), PMCompare);
	std::sort(result.vPMs2.begin(), result.vPMs2.end(), PMCompare);
	for (int i = result.vPM.size() - 1; i >= 0; --i)
	{
		if (result.vPM[i].Step2 > endStep2 || result.vPM[i].Step2 < beginStep2)
		{
			result.vPM.erase(result.vPM.begin() + i);
		}
	}
	for (int i = result.vPMs2.size() - 1; i >= 0; --i)
	{
		if (result.vPMs2[i].Step2 > endStep2 || result.vPMs2[i].Step2 < beginStep2)
		{
			result.vPMs2.erase(result.vPMs2.begin() + i);
		}
	}
	for (int i = result.vWounds.size() - 1; i >= 0; --i)
	{
		if (result.vWounds[i].Step2 > endStep2 || result.vWounds[i].Step2 < beginStep2)
		{
			result.vWounds.erase(result.vWounds.begin() + i);
		}
	}
	for (int i = 0; i < result.vPM.size(); ++i)
	{
		if (IsSew(result.vPM[i].Mark))
		{
			result.vSew.emplace_back(i);
		}
		else if (IsJoint(result.vPM[i].Mark))
		{
			result.vJoint.emplace_back(i);
		}
	}
	for (int i = 0; i < result.vPMs2.size(); ++i)
	{
		if (IsSew(result.vPMs2[i].Mark))
		{
			result.vSew2.emplace_back(i);
		}
		else if (IsJoint(result.vPMs2[i].Mark))
		{
			result.vJoint2.emplace_back(i);
		}
	}
}


void AnalyseBody(int iBeginBlock, int iEndBlock, VWJ& vWounds, VPM& vPMs, VPM& vPMs2, VPM& vPMsYOLO, VER& vER, VLCP& vLCP, int backFlag, int btIndex)
{
	char szLog[1024] = { 0 };
	sprintf(szLog, "AnalyseBody %d -> %d, btIndex = %d, Len = %d\n", iBeginBlock, iEndBlock, btIndex, iEndBlock - iBeginBlock + 1);
	WriteLog(szLog);
	VBDB vBdatas;   //所有B超数据
	BlockData_A vAFrames;
	BlockData_B tempBlock;   //一个米块的B超数据
	for (int i = iBeginBlock; i <= iEndBlock && i < g_vBlockHeads.size(); ++i)
	{
		if (g_vBlockSolvedFlags[i] == 0)
		{
			g_iSolvedBlockCount++;
			g_vBlockSolvedFlags[i] = 1;
		}
		tempBlock.vBStepDatas.clear();   //清空
		int indexL2 = g_vBlockHeads[i].IndexL2;   //米快初始步进!!!!!  GetBBlocks()
		if (g_isPTData)
		{
			//B超解压文件函数
			GetBlockBStepsPT(g_pFileB, g_szFileB, g_vBlockHeads[i], tempBlock);
		}
		else
		{
			GetBlockBSteps(g_pFileB, g_szFileB, g_vBlockHeads[i], tempBlock);
		}
		vBdatas.emplace_back(tempBlock);   //推入数据
	}

	if (btIndex < 0)
	{
		iEndBlock++;
	}
	uint32_t iBeginFrame = 0;
	for (int i = (iBeginBlock - 1 < 0 ? 0 : iBeginBlock - 1); i < iEndBlock + 1 && i < g_vBlockHeads.size(); ++i)
	{
		g_vBlockHeads[i].FrameCountRead = GetBlockAFrames(g_pFileA, g_szFileA, g_vBlockHeads[i], vAFrames, iBeginFrame);   //读到的A超帧数
	}

	int woundPrev = vWounds.size();
	//进入分段分析
	Analyse(vAFrames, vBdatas, g_vBAs, vWounds, vPMs, vER, vPMs2, vLCP, vPMsYOLO, backFlag, btIndex);

	//多周期：新旧伤损对比
	int newWound = vWounds.size() - woundPrev;
	int newBlock = vBdatas.size();
	int iBottom = 0, iScrew = 0;
	for (int i = woundPrev; i < newWound; ++i)
	{
		if (vWounds[i].Type == W_SCREW_CRACK1 || vWounds[i].Type == W_SCREW_CRACK2 || vWounds[i].Type == W_SCREW_CRACK3 || vWounds[i].Type == W_SCREW_CRACK4
			|| vWounds[i].Type == W_SCREW_HORIZON_CRACK_RIGHT || vWounds[i].Type == W_SCREW_HORIZON_CRACK_LEFT)
		{
			iScrew++;
		}
		if (vWounds[i].Type == W_BOTTOM_TRANSVERSE_CRACK)
		{
			iBottom++;
		}
	}
	if (newBlock >= 3 && 1.0 * newWound / newBlock >= 5 && iBottom >= newBlock && iScrew >= newBlock)
	{
		g_isTestEqu = 1;
	}
}

int	GetMinValueIndex(VINT& data)
{
	int minValue = 0xFFFF;
	int minIndex = 0;
	for (int i = 0; i < data.size(); ++i)
	{
		if (data[i] < minValue)
		{
			minValue = data[i];
			minIndex = i;
		}
	}
	return minIndex;
}

bool IsClean(int block)
{
	return g_vBlockHeads[block].sumAaBbCc <= 1 && g_vBlockHeads[block].sumDE <= 1 && g_vBlockHeads[block].sumFG <= 1;
}

int GetNextSplitBlock(int block1, int block2, int btIndex = -1)   //找米块分界位置
{
	int block3 = block2;
	if (btIndex < 0)   //无回退分界位置查找
	{
		while (block3 < g_vBlockHeads.size() && block3 - block1 < N_BLOCKREAD + 10 && IsClean(block3) == false && g_vBlockBackFlag[block3] == 0 && g_vBlockSolvedFlags[block3] == 0)
		{
			block3++;
		}

		if (block3 - block1 == N_BLOCKREAD + 10 && IsClean(block3) == false)
		{
			while (block3 - block1 > N_BLOCKREAD - 10 && IsClean(block3) == false && g_vBlockBackFlag[block3] == 0)
			{
				block3--;
			}

			if ((block3 - block1 <= N_BLOCKREAD - 10 || g_vBlockBackFlag[block3] != 0) && IsClean(block3) == false)
			{
				std::vector<int> sums;
				for (int i = block1 + N_BLOCKREAD - 10; i < block1 + N_BLOCKREAD + 10; ++i)
				{
					int s = g_vBlockHeads[i].sumAaBbCc + g_vBlockHeads[i].sumDE + g_vBlockHeads[i].sumFG;
					sums.emplace_back(s);
				}

				int minIndex = GetMinValueIndex(sums);
				block3 = block1 + N_BLOCKREAD - 10 + minIndex;
			}
		}
	}
	else
	{
		block3 = block2;
		if (block2 - block1 <= N_BLOCKREAD)
		{
			if (block2 + 1 < g_vBlockHeads.size() && g_vBlockBackFlag[block2 + 1] != 0)
			{

			}
			else
			{
				while (block3 < g_vBlockHeads.size() && block3 - block1 < N_BLOCKREAD + 10 && IsClean(block3) == true && g_vBlockBackFlag[block3] == 0)
				{
					block3++;
				}
			}
		}
		else
		{

		}
	}
	if (block3 == g_vBlockHeads.size())
	{
		block3 = block3 - 1;
	}
	else if (block3 > block2 && IsClean(block3) == false)
	{
		block3 = block3 - 1;
	}
	return block3;
}

void*  AnalyseThread(void* params)
{
#ifdef OUTPUT_EX
	try
	{
#endif // OUTPUT_EX
		WriteLog("AnalyseThread Begin\n");

#pragma region SZT数据分析，提取回退，道岔，等人工标记数据
		g_qualitydata.fileHead = &g_filehead;
		g_qualitydata.vBlockHeads = &g_vBlockHeads;
		g_qualitydata.strTPB = g_strDataPath + g_strFileNameWithoutExt + "temp";
		::remove(g_qualitydata.strTPB.c_str());
		DeleteFileA(g_qualitydata.strTPB.c_str());
		::CopyFileA(g_strTPB.c_str(), g_qualitydata.strTPB.c_str(), FALSE);
		g_qualitydata.szFileB = g_szFileB;

		g_tas.Reset();   // 报警统计列表相关数据结构
		g_tds.Reset();   // 伤损统计列表相关数据结构
		//SZT-800数据统计
		g_qualitydata.tas = &g_tas;   // 报警统计列表相关数据结构
		g_qualitydata.tds = &g_tds;   // 一般标记统计数据结构
		g_qualitydata.tss = &g_tss;   // 一般标记统计数据结构
		g_qualitydata.tbs = &g_tbs;   // 回退统计数据结构
		g_isQualityJudging = true;
		GetAlarmDefectSignData(&g_qualitydata);   // 回退统计
		g_sztWoundCount = g_tds.DefectDataCount;   // 伤损总数
		g_sztAlarmCount = g_tas.AlarmDataCount;   // 报警总数
		WriteLog("GetAlarmDefectSignData Finish\n");
#pragma endregion

#pragma region 文件内跳探
		if (g_vBlockHeads.size() > 0)
		{
			LoseDetect ld;
			int time = g_vBlockHeads[0].BlockHead.time;
			int lasttime = time;
			for (int i = 1; i < g_vBlockHeads.size(); ++i)
			{
				int delt = GetTimeSpan(g_vBlockHeads[i].BlockHead.time, lasttime);
				if (delt > 15)   //相邻米块之间的时间差大于15s判断跳探
				{
					ld.walk1 = g_vBlockHeads[i - 1].Walk;   //米块开始里程
					ld.walk2 = g_vBlockHeads[i].Walk;   //米块开始里程
					ld.step2 = g_vBlockHeads[i].IndexL2;    //米快初始步进
					ld.user = g_vBlockHeads[i - 1].BlockHead.user;   // 工号
					ld.second = g_vBlockHeads[i - 1].BlockHead.time & 0xFF;
					ld.minute = (g_vBlockHeads[i - 1].BlockHead.time >> 8) & 0xFF;
					ld.hour = (g_vBlockHeads[i - 1].BlockHead.time >> 16) & 0xFF;
					g_vLD.emplace_back(ld);   //跳探信息
				}
				lasttime = g_vBlockHeads[i].BlockHead.time;
			}
		}
		WriteLog("LoseDetect Finish\n");
#pragma endregion

#pragma region 数据解析
		uint32_t	blockCount = g_vBlockHeads.size();
		g_iCurrentBlock = g_beginSolveMeterIndex;

		//米块是否判断标记
		g_vBlockSolvedFlags.resize(blockCount);
		g_vBlockBackFlag.resize(blockCount);
		for (int i = 0; i < blockCount; ++i)
		{
			g_vBlockSolvedFlags[i] = 0;
			g_vBlockBackFlag[i] = 0;
		}

		for (int i = 0; i < g_vBAs.size(); ++i)
		{
			for (int j = g_vBAs[i].Pos0.Block; j < g_vBAs[i].Pos2.Block; ++j)   //回退之后的位置 < 回退之后再往前走到回退点的位置
			{
				g_vBlockBackFlag[j] = 1;
			}
		}

		g_iSolvedBlockCount = 0;

		for (int i = 0; i < g_vBT.size(); ++i)   //解析有回退米块
		{
			if (g_vBT[i].Block2 - g_vBT[i].Block1 <= N_BLOCKREAD)   
			{
				int iBeginBlock = g_vBAs[g_vBT[i].BeginBackIndex].Pos0.Block;
				if (iBeginBlock > 0 && g_vBlockBackFlag[iBeginBlock - 1] == 0)
				{
					iBeginBlock = iBeginBlock - 1;
				}
				int iEndBlock = g_vBT[i].Block2;
				iEndBlock = GetNextSplitBlock(iBeginBlock, iEndBlock, i);
				//核心分析部分
				AnalyseBody(iBeginBlock, iEndBlock, g_vWounds, g_vPMs, g_vPMs2, g_vPMsYOLO, g_vER, g_vLCP, 0xFFFF, i);
			}
			else
			{
				int iBeginBlock = g_vBAs[g_vBT[i].BeginBackIndex].Pos0.Block;
				int iEndBlock = g_vBAs[g_vBT[i].BeginBackIndex + g_vBT[i].BackCount - 1].Pos2.Block;
				for (int j = g_vBT[i].BackCount - 1; j >= 0; --j)
				{
					if (iEndBlock - g_vBAs[g_vBT[i].BeginBackIndex + j].Pos1.Block > N_BLOCKREAD)
					{
						//核心分析部分
						AnalyseBody(g_vBAs[g_vBT[i].BeginBackIndex + j + 1].Pos1.Block, iEndBlock, g_vWounds, g_vPMs, g_vPMs2, g_vPMsYOLO, g_vER, g_vLCP, 0x00FF, i);
						iEndBlock = g_vBAs[g_vBT[i].BeginBackIndex + j + 1].Pos1.Block;
					}
				}
			}
			}

		do  //解析无回退米块
		{
			while (g_iCurrentBlock < blockCount && g_vBlockSolvedFlags[g_iCurrentBlock] != 0)
			{
				g_iCurrentBlock++;
			}
			if (g_iCurrentBlock >= blockCount)
			{
				break;
			}
			int iEndBlock = g_iCurrentBlock + 1;
			while (iEndBlock < blockCount && g_vBlockSolvedFlags[iEndBlock] == 0 && iEndBlock - g_iCurrentBlock < N_BLOCKREAD)
			{
				++iEndBlock;
			}
			if (iEndBlock >= blockCount)
			{
				iEndBlock = blockCount - 1;
			}
			else if (IsClean(iEndBlock) == false)
			{
				iEndBlock--;
				iEndBlock = GetNextSplitBlock(g_iCurrentBlock, iEndBlock, -1);
			}
			//核心分析部分
			AnalyseBody(g_iCurrentBlock, iEndBlock, g_vWounds, g_vPMs, g_vPMs2, g_vPMsYOLO, g_vER, g_vLCP, 0, -1);
			g_iCurrentBlock = iEndBlock;
		}
#ifdef _DEBUG
		//while (g_iCurrentBlock < 400);
		while (g_iCurrentBlock < blockCount && g_isRunning != 0);
#else
		while (g_iCurrentBlock < blockCount && g_isRunning != 0);
#endif
#pragma endregion

#pragma region 数据整理

#pragma region S1 伤损&位置标排序
		/*
		if (g_isRunning == 0)
		{
			return NULL;
		}
		*/

		std::sort(g_vPMs.begin(), g_vPMs.end(), PMCompare);

		std::sort(g_vPMs2.begin(), g_vPMs2.end(), PMCompare);

		std::sort(g_vPMsYOLO.begin(), g_vPMsYOLO.end(), PMCompare);

		std::sort(g_vWounds.begin(), g_vWounds.end());
		for (int i = g_vWounds.size() - 1; i >= 0; --i)
		{
			if (g_vWounds[i].Flag == 1)
			{
				g_vWounds.erase(g_vWounds.begin() + i);
			}
		}


#pragma endregion

#pragma region S2 基础数据统计

		VPM vJointHalf;   //定义结构体类型 VPM 的变量 vJointHalf
		for (int i = 0; i < g_vPMs.size(); ++i)   //人工标记位置标数据数量
		{
			if (g_vPMs[i].Mark == PM_JOINT_LEFT || g_vPMs[i].Mark == PM_JOINT_RIGHT)   //位置标记类型（左侧半边接头/右侧半边接头）
			{
				vJointHalf.emplace_back(g_vPMs[i]);
			}
		}
#pragma endregion

#pragma region S3 位置标去重
		int markCount = g_vPMs.size();
		for (int i = 0; i < markCount; i++)
		{
			Pos pos = FindStepInBlock(g_vPMs[i].Step2, g_vBlockHeads, 0);
			g_vPMs[i].Block = pos.Block;
			g_vPMs[i].Step = pos.Step;
			g_vPMs[i].Walk2 = GetWD(g_vBlockHeads[pos.Block].Walk2, pos.Step, g_filehead.step, g_direction);
			g_vPMs[i].Flag = 0;
		}

		VPM marks = g_vPMs;
		g_vPMs.clear();
		for (int i = 0; i < markCount; ++i)
		{
			if ((IsJoint(marks[i].Mark) || IsSew(marks[i].Mark)) && marks[i].Flag == 0)
			{
				if (marks[i].Manual == 1)
				{
					for (int j = i + 1; j < markCount; ++j)
					{
						int dStep = marks[j].Step2 - marks[i].Step2;
						if (dStep > 500 || IsExistBackArea(marks[i].Step2, marks[j].Step2, g_vBAs, g_vBAs.size()))
						{
							break;
						}
						if ((IsSew(marks[i].Mark) && IsSew(marks[j].Mark)) || (IsJoint(marks[i].Mark) && IsJoint(marks[j].Mark)))
						{
							if (IsSew(marks[j].Mark))
							{
								marks[i].Flag = 1;
							}
							else if (IsJoint(marks[j].Mark))
							{
								marks[i] = marks[j];
								marks[j].Flag = 1;
							}
						}
					}
				}
				else
				{
					for (int j = i + 1; j < markCount; ++j)
					{
						int dStep = marks[j].Step2 - marks[i].Step2;
						if (dStep > 500 || IsExistBackArea(marks[i].Step2, marks[j].Step2, g_vBAs, g_vBAs.size()))
						{
							break;
						}
						if ((IsSew(marks[i].Mark) && IsSew(marks[j].Mark)) || (IsJoint(marks[i].Mark) && IsJoint(marks[j].Mark)))
						{
							if (marks[j].Manual == 1)
							{
								if (IsSew(marks[j].Mark))
								{
									marks[j].Flag = 1;
								}
								else if (IsJoint(marks[j].Mark))
								{
									marks[j] = marks[i];
									marks[i].Flag = 1;
								}
							}
							else
							{
								marks[j].Flag = 1;
							}
						}
					}
				}
			}
			else
			{
				for (int j = i + 1; j < markCount; ++j)
				{
					int dStep = marks[j].Step2 - marks[i].Step2;
					if (dStep >= 10)
					{
						break;
					}
					if (marks[i].Mark == marks[j].Mark && dStep < 10)
					{
						marks[i].Flag = 1;
					}
				}
			}
		}

#pragma region 变坡点
		VPM vRising;
		for (int i = 0; i < markCount; ++i)
		{
			if (marks[i].Flag == 0)
			{
				g_vPMs.emplace_back(marks[i]);
				if (marks[i].Mark == PM_SMART1 || marks[i].Mark == PM_SMART2)
				{
					vRising.emplace_back(marks[i]);
				}
			}
		}
#pragma endregion

#pragma endregion

#pragma region S4 道岔起点终点计算

		for (int i = 0; i < g_vForks.size(); ++i)
		{
			if (g_vForks[i].ForkNo == 0)
			{
				if (
					g_vForks[i].End.Block - g_vForks[i].Begin.Block <= 1 &&
					(i > 0 && g_vForks[i].Begin.Block - g_vForks[i - 1].End.Block < 1 ||
						i < g_vForks.size() - 1 && g_vForks[i + 1].Begin.Block - g_vForks[i].End.Block < 1))
				{
					continue;
				}
				for (int j = g_vForks[i].Begin.Block; j <= g_vForks[i].End.Block; ++j)
				{
					if (g_vBlockHeads[j].BlockHead.swNum > 0)
					{
						if (g_isPTData)
						{
							g_vForks[i].ForkNo = g_vBlockHeads[j].BlockHead.swNum;
						}
						else
						{
							g_vForks[i].ForkNo = BCDToINT16(g_vBlockHeads[j].BlockHead.swNum);
						}
						g_vForks[i].Bits = g_vBlockHeads[j].BlockHead.BitS;
						break;
					}
				}
			}
		}

		std::vector<ForkPara> forkParas;
		ForkPara para;
		int idx = 0;
		bool isFinish = false;
		while (idx < g_vForks.size() && isFinish == false)
		{
			para.iIndex = idx;
			para.iEndIndex = idx;
			para.ForkNo = g_vForks[idx].ForkNo;
			para.Bits = g_vForks[idx].Bits;

			if (idx == g_vForks.size() - 1)
			{
				forkParas.emplace_back(para);
				break;
			}

			for (int j = idx + 1; j < g_vForks.size(); ++j)
			{
				if (g_vForks[j].Begin.Step2 - g_vForks[para.iEndIndex].End.Step2 > 1
					|| para.ForkNo != 0 && g_vForks[j].ForkNo != 0 && para.ForkNo != g_vForks[j].ForkNo
					|| para.Bits != 0 && g_vForks[j].Bits != 0 && para.Bits != g_vForks[j].Bits
					)
				{
					if (para.ForkNo == 0)
					{
						para.ForkNo = g_vForks[j].ForkNo;
						para.Bits = g_vForks[j].Bits;
					}
					forkParas.emplace_back(para);
					idx = j;
					break;
				}
				else
				{
					if (para.ForkNo == 0)
					{
						para.ForkNo = g_vForks[j].ForkNo;
						para.Bits = g_vForks[j].Bits;
					}
					para.iEndIndex = j;

					if (j == g_vForks.size() - 1)
					{
						forkParas.emplace_back(para);
						isFinish = true;
						idx = j;
						break;
					}
				}
			}
		}

		for (int i = 0; i < forkParas.size(); ++i)
		{
			if (forkParas[i].ForkNo == 0)
			{
				for (int j = g_vForks[forkParas[i].iIndex].Begin.Block; j <= g_vForks[forkParas[i].iEndIndex].End.Block; ++j)
				{
					if (g_vBlockHeads[j].BlockHead.swNum > 0)
					{
						if (g_isPTData)
						{
							forkParas[i].ForkNo = g_vBlockHeads[j].BlockHead.swNum;
						}
						else
						{
							forkParas[i].ForkNo = BCDToINT16(g_vBlockHeads[j].BlockHead.swNum);
						}
						//forkParas[i].ForkNo = BCDToINT16(g_vBlockHeads[j].BlockHead.swNum);
						forkParas[i].Bits = g_vBlockHeads[j].BlockHead.BitS;
						break;
					}
				}
			}
		}

		VPM vForkPMs;
		for (int i = 0; i < forkParas.size(); ++i)
		{
			g_vForks[forkParas[i].iIndex].Begin.Data = forkParas[i].ForkNo;
			g_vForks[forkParas[i].iIndex].Begin.ChannelNum = forkParas[i].Bits;
			g_vForks[forkParas[i].iIndex].End.Data = forkParas[i].ForkNo;
			g_vForks[forkParas[i].iIndex].End.ChannelNum = forkParas[i].Bits;

			vForkPMs.emplace_back(g_vForks[forkParas[i].iIndex].Begin);
			vForkPMs.emplace_back(g_vForks[forkParas[i].iEndIndex].End);
		}

		for (int i = g_vPMs2.size() - 1; i >= 0; --i)
		{
			if (g_vPMs2[i].Mark == PM_HJFORK_BEGIN || g_vPMs2[i].Mark == PM_HJFORK_END || g_vPMs2[i].Mark == PM_MGFORK_BEGIN || g_vPMs2[i].Mark == PM_MGFORK_END)
			{
				g_vPMs2.erase(g_vPMs2.begin() + i);
			}
		}
		for (int i = 0; i < vForkPMs.size(); ++i)
		{
			AddToMarks(vForkPMs[i], g_vPMs2);
		}
		std::sort(g_vPMs2.begin(), g_vPMs2.end(), PMCompare);
#pragma endregion

#pragma region S5 人工标记位置标去重
		VPM marks2 = g_vPMs2;
		int markCount2 = g_vPMs2.size();
		for (int i = 0; i < markCount2; i++)
		{
			g_vPMs2[i].Flag = 0;
		}

		VPM vCurves, vBridges;
		for (int i = 0; i < markCount2; ++i)
		{
			for (int j = i + 1; j < markCount2; ++j)
			{
				if (marks2[i].Mark == marks2[j].Mark && marks2[i].Step2 == marks2[j].Step2)
				{
					marks2[j].Flag = 1;
				}
				else if (marks2[j].Step2 != marks2[i].Step2)
				{
					break;
				}
			}

			if (marks2[i].Mark == PM_CURVE_BEGIN || marks2[i].Mark == PM_CURVE_END)
			{
				vCurves.emplace_back(marks2[i]);
				marks2[i].Flag = 1;
			}
			if (marks2[i].Mark == PM_BRIDGE_BEGIN || marks2[i].Mark == PM_BRIDGE_END)
			{
				vBridges.emplace_back(marks2[i]);
				marks2[i].Flag = 1;
			}
		}

		if (vCurves.size() > 1)
		{
			double lstEnd = vCurves[1].Walk;
			for (int i = 2; i < vCurves.size(); i += 2)
			{
				if (fabs(vCurves[i].Walk - lstEnd) < 0.01)
				{
					vCurves[i].Flag = 1;
					vCurves[i - 1].Flag = 1;
				}
				if (i + 1 < vCurves.size())
				{
					lstEnd = vCurves[i + 1].Walk;
				}
			}
		}

		if (vBridges.size() > 0)
		{
			double lstBridgeEnd = vBridges[1].Walk;
			double lstEnd = vBridges[1].Walk;
			for (int i = 2; i < vBridges.size(); i += 2)
			{
				if (fabs(vBridges[i].Walk - lstEnd) < 0.01)
				{
					vBridges[i].Flag = 1;
					vBridges[i - 1].Flag = 1;
				}
				if (i + 1 < vBridges.size())
				{
					lstEnd = vBridges[i + 1].Walk;
				}
			}
		}

		g_vPMs2.clear();
		for (int i = 0; i < vCurves.size(); ++i)
		{
			if (vCurves[i].Flag == 0)
			{
				AddToMarks(vCurves[i], g_vPMs2);
			}
		}
		for (int i = 0; i < vBridges.size(); ++i)
		{
			if (vBridges[i].Flag == 0)
			{
				AddToMarks(vBridges[i], g_vPMs2);
			}
		}
		for (int i = 0; i < markCount2; ++i)
		{
			if (marks2[i].Flag == 0)
			{
				AddToMarks(marks2[i], g_vPMs2);
			}
		}
		std::sort(g_vPMs2.begin(), g_vPMs2.end(), PMCompare);
#pragma endregion		

#pragma region S6 修正里程
		VPM vpms, vpms2;
		for (int i = 0; i < g_vPMs.size(); ++i)
		{
			if (g_vPMs[i].Mark == PM_WALKREVISE || g_vPMs[i].Mark == PM_BACK)
			{
				AddToMarks(g_vPMs[i], vpms);
			}
			if (g_vPMs[i].Mark == PM_JOINT2 || g_vPMs[i].Mark == PM_JOINT4 || g_vPMs[i].Mark == PM_JOINT6 || g_vPMs[i].Mark == PM_SEW_CH || g_vPMs[i].Mark == PM_SEW_LRH || g_vPMs[i].Mark == PM_SEW_LIVE)
			{
				AddToMarks(g_vPMs[i], vpms2);
			}
		}

		VPM vFind, vFind2;
		for (int i = 0; i < g_vBAs.size(); ++i)
		{
			vFind.clear();
			vFind2.clear();
			FindMarksInBackAction(g_vBAs[i].Pos0.Step2, g_vBAs[i].Pos1.Step2, g_vPMs, vFind);
			FindMarksInBackAction(g_vBAs[i].Pos1.Step2, g_vBAs[i].Pos2.Step2, g_vPMs, vFind2);

			if (vFind.size() == 1 && vFind2.size() == 1)
			{
				int ii = 0, jj = 0;
				if (IsJoint(vFind[ii].Mark) && IsJoint(vFind2[jj].Mark) || IsSew(vFind[ii].Mark) && IsSew(vFind2[jj].Mark))
				{
					double dwalk = g_vPMs[jj].Walk - g_vPMs[ii].Walk;
					for (int k = g_vBAs[i].Pos1.Block + 1; k < g_vBlockHeads.size(); ++k)
					{
						g_vBlockHeads[k].Walk2 = g_vBlockHeads[k].Walk2 + dwalk;
					}
				}
			}
		}

#pragma region 调整里程
		double r = g_direction ? 0.000001 * g_filehead.step : -0.000001 * g_filehead.step;
		for (int i = 0; i < g_vWounds.size(); ++i)
		{
			g_vWounds[i].Walk = g_vBlockHeads[g_vWounds[i].Block].Walk + r * g_vWounds[i].Step;
			g_vWounds[i].Walk2 = g_vBlockHeads[g_vWounds[i].Block].Walk2 + r * g_vWounds[i].Step;
		}

		for (int i = 0; i < g_vPMs.size(); ++i)
		{
			g_vPMs[i].Walk = g_vBlockHeads[g_vPMs[i].Block].Walk + r * g_vPMs[i].Step;
			g_vPMs[i].Walk2 = g_vBlockHeads[g_vPMs[i].Block].Walk2 + r * g_vPMs[i].Step;
		}

		for (int i = 0; i < g_vPMs2.size(); ++i)
		{
			g_vPMs2[i].Walk = g_vBlockHeads[g_vPMs2[i].Block].Walk + r * g_vPMs2[i].Step;
			g_vPMs2[i].Walk2 = g_vBlockHeads[g_vPMs2[i].Block].Walk2 + r * g_vPMs2[i].Step;
		}
#pragma endregion
#pragma endregion

#pragma region S7 模板数据匹配
		FilterByJointReflectWavw();

		FilterByCombineSameTypePrev();
		WriteLog("FilterByCombineSameTypePrev Finish\n");

		int iBeginMarkIndex = -1, iEndiMarkIndex = -1;
		int iTemplateIndex = -1;
		int iMatchedCount = 0;
		std::map<int, int> mapIndexes;
		for (int i = g_vPMs.size() - 1; i >= 0; --i)
		{
			if (IsJoint(g_vPMs[i].Mark) || g_vPMs[i].Mark == PM_SEW_LRH)
			{
				for (int j = i - 1; j >= 0; --j)
				{
					if (IsJoint(g_vPMs[j].Mark) || g_vPMs[j].Mark == PM_SEW_LRH)
					{
						double dw = g_vPMs[i].Walk - g_vPMs[j].Walk;
						int dStep = g_vPMs[i].Step2 - g_vPMs[j].Step2;
						for (int k = 0; k < g_vTemplate.size(); ++k)
						{
							int stepCount = 1000000.0 * g_vTemplate[k].railLength / g_filehead.step;
							if ((fabs(dw) > g_vTemplate[k].railLength + 0.002 || fabs(dw) < g_vTemplate[k].railLength - 0.002) && (dStep >= stepCount + 100 || dStep < stepCount - 100))
							{
								break;
							}
							else if (dStep < stepCount + 100)//±20cm
							{
								mapIndexes.clear();
								if (IsMatch(i, j, k, mapIndexes) == 1)
								{
									g_isTestEqu = 1;
									iMatchedCount++;
									iBeginMarkIndex = i;
									iEndiMarkIndex = j;
									iTemplateIndex = k;

									std::map<int, int> mapIndexes2;
									for (auto itr = mapIndexes.begin(); itr != mapIndexes.end(); ++itr)
									{
										mapIndexes2[itr->second] = itr->first;
										g_vWounds[itr->first].Flag = 0;
										g_vWounds[itr->first].IsMatched = 1;
										memset(g_vWounds[itr->first].Result, 0, 60);
										memcpy(g_vWounds[itr->first].Result, g_vTemplate[k].vName[itr->second].c_str(), g_vTemplate[k].vName[itr->second].length());
										g_vWounds[itr->first].Type = g_vTemplate[k].vWounds[itr->second].Type;
									}

									railTemplate& rt = g_vTemplate[iTemplateIndex];
									for (int ii = 0; ii < rt.woundCount; ++ii)
									{
										if (mapIndexes2.find(ii) == mapIndexes2.end())
										{
											Wound_Judged wd;
											wd.SetEmpty();
											wd.Step2 = g_vPMs[iEndiMarkIndex].Step2 + rt.vWounds[ii].Step2;
											Pos pos = FindStepInBlock(wd.Step2, g_vBlockHeads, 0);
											wd.Block = pos.Block;
											wd.Step = pos.Step;
											wd.StepLen = rt.vWounds[ii].StepLen;
											wd.Row1 = rt.vWounds[ii].Row1;
											wd.RowLen = rt.vWounds[ii].RowLen;
											wd.Walk = pos.Walk;
											wd.Type = rt.vWounds[ii].Type;
											wd.Place = rt.vWounds[ii].Place;
											memcpy(wd.Result, rt.vName[ii].c_str(), rt.vName[ii].length());
											std::string accor = "此处出波不够，但与模板[";
											accor += rt.railName;
											accor += "]匹配此处应有伤损";
											wd.According.emplace_back(accor);

											wd.IsMatched = 1;
											wd.Flag = 0;
											g_vWounds.emplace_back(wd);
										}
									}
									break;
								}
							}
						}
					}
				}
			}
		}

		//HeapSort(g_vWounds, g_vWounds.size());
		std::sort(g_vWounds.begin(), g_vWounds.end());
		WriteLog("Match Finish\n");

#pragma endregion

#pragma region S8 伤损过滤	
		int oldCount = g_vWounds.size();
		FilterByCombineSameType();
		WriteLog("FilterByCombineSameType Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-FilterByCombineSameType.txt");

		int woundCount = g_vWounds.size();

		int iBottom = 0, iScrew = 0;
		for (int i = 0; i < woundCount; ++i)
		{
			if (g_vWounds[i].Type == W_SCREW_CRACK1 || g_vWounds[i].Type == W_SCREW_CRACK2 || g_vWounds[i].Type == W_SCREW_CRACK3 || g_vWounds[i].Type == W_SCREW_CRACK4 || g_vWounds[i].Type == W_SCREW_HORIZON_CRACK_RIGHT || g_vWounds[i].Type == W_SCREW_HORIZON_CRACK_LEFT)
			{
				iScrew++;
			}
			if (g_vWounds[i].Type == W_BOTTOM_TRANSVERSE_CRACK)
			{
				iBottom++;
			}
		}
		int nBlockCount = g_vBlockHeads.size();
		for (int i = 0; i < g_vBAs.size(); ++i)
		{
			nBlockCount -= (g_vBAs[i].Pos1.Block - g_vBAs[i].Pos0.Block);
		}
		if (nBlockCount <= 0)
		{
			nBlockCount = 1;
		}
		g_isTestEqu = (g_vWounds.size() >= (nBlockCount * 3) && iScrew >= nBlockCount * 0.8 && iBottom >= nBlockCount * 0.2) ? 1 : 0;
		if (g_pFileLog != 0)
		{
			fprintf(g_pFileLog, "IsTestEqu: %d\n", g_isTestEqu);
			fflush(g_pFileLog);
		}

		//S8.2 过滤孔上伤损
		FilterByHole(g_vWounds, vForkPMs, g_vBlockHeads, g_vBAs);
		WriteLog("FilterByHole Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-1FilterByHole.txt");

		//S8.3 过滤道岔上的伤损
		FilterByFork(g_vWounds, g_vPMs, g_vBlockHeads, g_pFileB, g_szFileB);
		WriteLog("FilterByFork Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-2FilterByFork.txt");

		//S8.4 过滤孔上的Cc螺孔出波
		FilterByCCHole();
		WriteLog("FilterByCCHole Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-3FilterByCCHole.txt");

		//S8.5 过滤斜裂纹
		FilterBySkew();
		WriteLog("FilterBySkew Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-4FilterBySkew.txt");

		//S8.6 过滤出波量小的伤损
		FilterBySmall();
		WriteLog("FilterBySmall Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-5FilterBySmall.txt");

		//S8.7 合并同类伤损
		FilterBySameType();
		WriteLog("FilterBySameType Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-6FilterBySameType.txt");

		FilterBySmart(vRising);
		WriteLog("FilterBySmart Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-7FilterBySmart.txt");

		//S8.8 合并核伤
		FilterByCombineHS();
		WriteLog("FilterByCombineHS Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-8FilterByCombineHS.txt");

		//S8.9 过滤回退点附近的水平裂纹和纵向裂纹
		FilterByBackArea();
		WriteLog("FilterByBackArea Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-9FilterByBackArea.txt");

		//S8.10 过滤轨腰推偏导致的FG出波误判
		FilterByFGWaist();
		WriteLog("FilterByFGWaist Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-10FilterByFGWaist.txt");

		for (int i = 0; i < g_vWounds.size(); ++i)
		{
			int iJawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03];
			GetWoundRect(i, iJawRow, &g_vWounds[i].Rect);
		}
		//S8.11 过滤纵向裂纹
		FilterByHorizonalCrack();
		WriteLog("FilterByHorizonalCrack Finish\n");

		//合并鱼鳞伤
		FilterByCombineFish();
		WriteLog("FilterByCombineFish Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-11FilterByCombineFish.txt");

		//S8.15 过滤掉没有判据的伤损
		FilterByNoCR();
		WriteLog("FilterByNoCR Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-12FilterByNoCR.txt");

		FilterByVertical(g_vWounds, vForkPMs, g_vBlockHeads);
		WriteLog("FilterByVertical Finish\n");
		//PrintWound(g_vWounds, "D:/Files/AlgLog/wound-13FilterByVertical.txt");

		if (g_isTestEqu == 0)
		{
#pragma region 计算各类伤损分布
			woundCount = g_vWounds.size();
			size_t szTypes = g_mapWoundTypeCountTemp.size();
			for (int i = 0; i < woundCount; ++i)
			{
				++g_mapWoundTypeCountTemp[g_vWounds[i].Type];
			}
#pragma endregion
			//S8.12 过滤轨底横向裂纹
			FilterBottomCrack();
			WriteLog("FilterBottomCrack Finish\n");
#pragma region 处理高频鱼鳞伤
			uint32_t woundStep;
			int		rowLCount[VALID_ROW] = { 0 };
			int		rowHCount[VALID_ROW] = { 0 };
			std::vector<uint8_t> vChannels;
			for (int i = 0; i < g_vWounds.size(); ++i)
			{
				VINT vNear;
				if (g_vWounds[i].Type == W_HS || g_vWounds[i].Type == W_VERTICAL_CRACK)
				{
					woundStep = g_vWounds[i].Step2;
					for (int j = i + 1; j < g_vWounds.size(); ++j)
					{
						if (g_vWounds[j].Type == g_vWounds[i].Type && g_vWounds[j].Step2 - woundStep < 40)
						{
							vNear.emplace_back(j);
							woundStep = g_vWounds[j].Step2;
						}
						if (g_vWounds[j].Step2 - woundStep > 500)
						{
							break;
						}
					}

					if (g_vWounds[i].Type == W_HS && vNear.size() >= 10 || g_vWounds[i].Type == W_VERTICAL_CRACK && vNear.size() >= 3)
					{
						for (int j = 0; j < vNear.size(); ++j)
						{
							SetWoundFlag(g_vWounds[vNear[j]], 1);
						}
					}
				}
			}


			for (int i = g_vWounds.size() - 1; i >= 0; --i)
			{
				if (g_vWounds[i].Flag != 0)
				{
					g_vWounds.erase(g_vWounds.begin() + i);
				}
			}

			woundCount = g_vWounds.size();
			WriteLog("Fish Finish\n");
#pragma endregion		

#pragma region 根据数据质量过滤大量的重复伤损
			g_vER2.clear();
			time_t t1 = time(NULL);

			int pointsize_predict = g_vBlockHeads.size() * 200;
			//vPoints.resize(pointsize_predict);
			g_vPoints.reserve(pointsize_predict);
			WriteLog("reserve Finish\n");
			DQWavePoint pt;
			int sz = sizeof(pt);
			WriteLog("Read DQFile Begin\n");
			FILE* pFile2 = fopen(g_strWavePointFile.c_str(), "rb");
			fseek(pFile2, 0, SEEK_END);
			int fileSz = ftell(pFile2);
			int pointcount = fileSz / sz;

			char* szLog = new char[200];
			sprintf(szLog, "PointCount = %d\n", pointcount);
			WriteLog(szLog);

			memset(szLog, 0, 200);
			sprintf(szLog, "WaveFileSz=%d, sz=%d, pointcount=%d\n", fileSz, sz, pointcount);
			WriteLog(szLog);
			delete szLog;

			fseek(pFile2, 0, SEEK_SET);


			//FILE* pfilePlain = fopen("D:/plain.txt", "w");
			for (int i = 0; i < pointcount; ++i)
			{
				fread(&pt, sz, 1, pFile2);
				//	fprintf(pfilePlain, "pt.Step, pt.Row, pt.Channel, pt.JawRow:%d\t, %d\t, %X\t, %d\n", pt.Step, pt.Row, pt.Channel, pt.JawRow);
					//vPoints.emplace_back(pt);
				if (pt.Row >= 40 && (pt.Channel & 0x3FFF) == 0)
				{
					continue;
				}
				g_vPoints.emplace_back(pt);
			}
			//fclose(pfilePlain);
			fclose(pFile2);

			//vPoints.clear();
			WriteLog("Read DQFile Finish\n");
			ParseER(g_vPoints, NULL);
			WriteLog("ParseER Finish\n");

			g_vPoints.clear();
			if (g_isDeleteDQFile)
			{
				remove(g_strWavePointFile.c_str());
				DeleteFileA(g_strWavePointFile.c_str());
			}


#ifdef DQ_COMBINE
			uint8_t isCombined = 0;
			uint32_t *pSteps = new uint32_t[g_vER.size() << 1];
			for (int i = 0; i < g_vER.size(); ++i)
			{
				pSteps[2 * i] = g_vER[i].step1;
				pSteps[2 * i + 1] = g_vER[i].step2;
			}
			uint32_t sends = 8 * g_vER.size();
			send(g_socket, (char*)&sends, 4, 0);
			send(g_socket, (char*)pSteps, 8 * g_vER.size(), 0);
			delete pSteps;

			uint32_t erBytes = 0;
			char pt[4] = { 0 };
			recv(g_socket, pt, 4, 0);
			memcpy(&erBytes, pt, 4);

			memset(pSteps, 0, 8 * g_vER.size());
			//char *recvBytes = new char[erBytes];
			char recvBytes[1000] = { 0 };
			int recvCount = recv(g_socket, recvBytes, erBytes, 0);
			if (recvCount < erBytes || recvCount % 8 != 0)
			{
				return (void*)-1;
			}
			uint32_t* p = (uint32_t*)recvBytes;
			int er2Count = recvCount >> 3;
			ErroeRect er;
			for (int i = 0; i < er2Count; ++i)
			{
				er.step1 = *p++;
				er.step2 = *p++;
				g_vER2.emplace_back(er);
			}
			delete recvBytes;

			for (int i = 0; i < er2Count; ++i)
			{
				Pos pos1 = FindStepInBlock(g_vER2[i].step1, g_vBlockHeads, 0);
				Pos pos2 = FindStepInBlock(g_vER2[i].step2, g_vBlockHeads, 0);
				g_vER2[i].block1 = pos1.Block;
				g_vER2[i].walk1 = pos1.Walk;
				g_vER2[i].block2 = pos2.Block;
				g_vER2[i].walk2 = pos2.Walk;
			}
#endif
			FilterByER();
			WriteLog("FilterByER Finish\n");
#pragma endregion
		}
		//S8.14 过滤多孔
		FilterByDoubleHole(g_vWounds, vForkPMs, g_vBlockHeads);
		WriteLog("FilterByDoubleHole Finish\n");

		for (int i = 0; i < g_vWounds.size(); ++i)
		{
			int iJawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03];
			GetWoundRect(i, iJawRow, &g_vWounds[i].Rect);
		}
		std::sort(g_vWounds.begin(), g_vWounds.end());

		FilterByFork2(g_vWounds, g_vPMs, g_vPMs2, g_vBlockHeads);
		WriteLog("FilterByFork2 Finish\n");

		if (g_vWounds.size() > 0 && g_vWounds[0].Type == W_VERTICAL_CRACK && g_vWounds[0].Step2 == 0)
		{
			g_vWounds.erase(g_vWounds.begin());
		}

#pragma endregion

#pragma region S10 保证所有伤损步进唯一
		std::map<uint32_t, uint8_t> isExsit;
		std::vector<int32_t> vMulti;

		VWJ tempWounds = g_vWounds;

		for (int i = 0; i < g_vWounds.size(); ++i)
		{
			g_vWounds[i].FileID = 0;
			if (isExsit.find(g_vWounds[i].Step2) != isExsit.end())
			{
				isExsit[g_vWounds[i].Step2] = isExsit[g_vWounds[i].Step2] + 1;
				vMulti.emplace_back(i);
			}
			else
			{
				isExsit[g_vWounds[i].Step2] = 0;
			}
		}

		for (int i = 0; i < vMulti.size(); ++i)
		{
			int step2 = g_vWounds[vMulti[i]].Step2;
			while (isExsit.find(step2) != isExsit.end() && step2 >= 0)
			{
				step2++;
			}
			g_vWounds[vMulti[i]].FileID = (step2 - g_vWounds[vMulti[i]].Step2);
			g_vWounds[vMulti[i]].Step2 = step2;
			g_vWounds[vMulti[i]].Rect.step1 = step2;
		}
#pragma endregion

#pragma region S11 重新计算伤损类型和伤损位置
		if (g_pFileA != NULL)
		{
			fclose(g_pFileA); g_pFileA = NULL;
		}
		if (g_pFileB != NULL)
		{
			fclose(g_pFileB); g_pFileB = NULL;
		}
		g_isFileOpened = 0;

		for (int i = 0; i < g_vWounds.size(); ++i)
		{
			int iJawRow = g_iJawRow[g_vBlockHeads[g_vWounds[i].Block].BlockHead.railType & 0x03];
			if (g_vWounds[i].Manual == 0 && GetWoundRect(i, iJawRow, &g_vWounds[i].Rect))
			{
				Pos pos = FindStepInBlock(g_vWounds[i].Rect.step1, g_vBlockHeads, 0);
				g_vWounds[i].Block = pos.Block;
				g_vWounds[i].Step = pos.Step;
			}
			else if (g_vWounds[i].Manual != 0)
			{
				g_vWounds[i].Rect.step1 = g_vWounds[i].Step2 - 5;
				g_vWounds[i].Rect.step2 = g_vWounds[i].Step2 - 5;
				if (g_vWounds[i].Step2 < 5)
				{
					g_vWounds[i].Rect.step1 = g_vWounds[i].Step2;
					g_vWounds[i].Rect.step2 = g_vWounds[i].Step2;
				}
				g_vWounds[i].Rect.row1 = 0;
				g_vWounds[i].Rect.row2 = 60;
			}
			if (g_vWounds[i].FileID > 0)
			{
				Pos pos = FindStepInBlock(g_vWounds[i].Rect.step1, g_vBlockHeads, 0);
				g_vWounds[i].Block = pos.Block;
				g_vWounds[i].Step = pos.Step;
				g_vWounds[i].Rect.step1 = g_vWounds[i].Rect.step1;
				g_vWounds[i].Rect.step2 = g_vWounds[i].Rect.step2;
			}
		}

		ReviseWoundPlace();
		std::sort(g_vWounds.begin(), g_vWounds.end());

		for (int i = 0; i < g_vWounds.size(); i++)
		{
			if (g_vWounds[i].Rect.step2 - g_vWounds[i].Rect.step1 > 1000)
			{
				char logInfo[100];
				sprintf(logInfo, "Len > 1000, block = %d, step = %d\n", g_vWounds[i].Block, g_vWounds[i].Step);
				WriteLog(logInfo);
			}
		}

		if (g_PrevFileCount >= 1)
		{
			WriteLog("MultiCycleAlign");
			MultiCycleAlign();
			WriteLog("MultiCycleAlign Finish\n");
		}

		WriteLog("AnalyseThread Finish\n");
		g_isFinish = 1;
		g_isRunning = 0;
#pragma endregion
#pragma endregion

#ifdef OUTPUT_EX
		}
	catch (CBaseException &e)
	{
		e.ShowExceptionInformation();
	}
#endif // OUTPUT_EX	

	return NULL;
	}

int32_t SetFile(char* strTPB, int32_t len, File4Nodejs* pInfo)  //设置要解析的文件
//输入参数
//strTPB：待解析的tpB文件的完整路径，’\’需要替换成’/’
//len: tpB文件的完整路径字节数
//输出参数
//pInfo：文件头信息
{
#ifdef OUTPUT_EX
	try
	{
#endif // OUTPUT_EX
		char *szLog = new char[len + 20];
		sprintf(szLog, "SetFile %s\n", strTPB);
		WriteLog(szLog);
		delete szLog;

		if (g_isInitialized == 0 || g_isRunning == 1)
		{
			return -100;
		}
		if (g_pFileA != NULL)
		{
			fclose(g_pFileA);
			g_pFileA = NULL;
		}
		if (g_pFileB != NULL)
		{
			fclose(g_pFileB);
			g_pFileB = NULL;
		}

		int iFind = -1, iFind2 = -1;
		g_xianbie = 1;//默认正线
		pInfo->leftright = 0;//默认右股
		for (int i = len - 1; i >= 0; --i)
		{
			if (strTPB[i] == '/')
			{
				iFind = i;
				break;
			}
			if (strTPB[i] == 'Z' || strTPB[i] == 'z')
			{
				g_xianbie = 0;//站线
			}
			if (strTPB[i] == 'L' || strTPB[i] == 'l')
			{
				pInfo->leftright = 1;//左股
			}
			if (strTPB[i] == '.' && iFind2 < 0)
			{
				iFind2 = i;
			}
		}
		if (iFind < 0)
		{
			printf("Error FileName: %s\n", strTPB);
			return -1;
		}

		memset(pInfo->FileName, 0, 36);
		for (int i = iFind + 1; i < iFind2; ++i)
		{
			pInfo->FileName[i - iFind - 1] = strTPB[i];
		}
		printf("FileName = %s\n", pInfo->FileName);


		g_isFileOpened = 0;
		g_pFileB = _fsopen(strTPB, "rb", _SH_DENYWR);
		if (g_pFileB == NULL)
		{
			printf("Fail open tpB: %s\n", strTPB);
			return -2;
		}

		int sz = fread(&g_filehead, 1, szFileHead, g_pFileB);
		fclose(g_pFileB);
		if (sz < szFileHead)
		{
			printf("Error FileSize: %d\n", sz);
			return -3;
		}

		g_strTPB = std::string(strTPB, len);
		int32_t index = -1;
		for (int32_t i = len - 5; i >= 0; --i)
		{
			if (strTPB[i] == '\\' || strTPB[i] == '/')
			{
				index = i;
				g_splitter = strTPB[i];
				break;
			}
		}
		if (index <= 0)
		{
			return -4;
		}

		g_isPTData = false;
		for (int i = 0; i < 8; i++)
		{
			if (g_filehead.DeviceNum[i] == 'p' || g_filehead.DeviceNum[i] == 'P')
			{
				g_isPTData = true;
			}
		}

		g_strFolder = g_strTPB.substr(0, index);
		g_strFileName = g_strTPB.substr(index + 1);
		g_strFileNameWithoutExt = g_strFileName.substr(0, g_strFileName.find_last_of('.'));


		std::string anaTime = GetCurrentTimeString();
		strncpy(pInfo->anaTime, anaTime.c_str(), anaTime.length());
		printf("anaTime = %s\n", pInfo->anaTime);

		if (g_isPTData)
		{
			pInfo->gwd = g_filehead.deviceP2.TrackSet.sectionNum;   // 单位编号5个BCD
			pInfo->section = g_filehead.deviceP2.TrackSet.regionNum;   // 工区4个BCD
			pInfo->group = g_filehead.deviceP2.TrackSet.teamNum;   // 班组4个BCD
			pInfo->railno = g_filehead.deviceP2.TrackSet.lineNum;   // 线编号4个BCD
			pInfo->PlyNo = g_filehead.deviceP2.TrackSet.plyNum;   // 股道编号（站线）4个BCD
			pInfo->StationNum = g_filehead.deviceP2.TrackSet.stationNum;   // 车站编号（站线）5个BCD
		}
		else
		{
			pInfo->gwd = BCDToINT32(g_filehead.deviceP2.TrackSet.sectionNum);
			pInfo->section = BCDToINT16(g_filehead.deviceP2.TrackSet.regionNum);
			pInfo->group = BCDToINT16(g_filehead.deviceP2.TrackSet.teamNum);
			pInfo->railno = BCDToINT16(g_filehead.deviceP2.TrackSet.lineNum);
			pInfo->PlyNo = BCDToINT16(g_filehead.deviceP2.TrackSet.plyNum);
			pInfo->StationNum = BCDToINT32(g_filehead.deviceP2.TrackSet.stationNum);
		}
		printf("gwd = %d\n", pInfo->gwd);
		pInfo->direction = g_filehead.deviceP2.TrackSet.WalkWay;   // 行走方向0: 逆里程, 1: 顺里程
		g_direction = g_filehead.deviceP2.TrackSet.WalkWay;  
		pInfo->xingbie = g_filehead.deviceP2.TrackSet.lineWay;   // 行别0：单线、1：上，2：下行
		g_xingbie = g_filehead.deviceP2.TrackSet.lineWay;
		pInfo->xianbie = g_xianbie;   // 线别

		strncpy(pInfo->instruNo, g_filehead.DeviceNum, 8);   ////仪器编号
		printf("instruNo = %s\n", pInfo->instruNo);   

		strncpy(pInfo->dataV, g_filehead.DataVerS, 8);   // 数据版本
		printf("dataV = %s\n", pInfo->dataV);
		strncpy(pInfo->softV, g_filehead.SoftVerS, 8);   // 软件版本
		printf("softV = %s\n", pInfo->softV);
		strncpy(pInfo->fpgaV, g_filehead.FPGAVerS, 8);   // FPGA版本
		printf("fpgaV = %s\n", pInfo->fpgaV);



		pInfo->blockCount = g_filehead.block;
		pInfo->sdType = g_filehead.deviceP2.TrackSet.lineType;   //上道类型0: 站线, 1: 正线
		for (int i = 0; i < CH_N; ++i)
		{
			pInfo->Offset[i] = g_filehead.deviceP2.Place[i];   // 探头位置
		}
		for (int i = 0; i < 4 * GA_NUM; ++i)
		{
			pInfo->Gate[i].isOn = g_filehead.deviceP2.Gate[i].isOn;   // 方门开关
			if (g_isPTData == false)
			{
				pInfo->Gate[i].start = GetBigGatePixel(i / 5, g_filehead.deviceP2.Gate[i].start & 0x7FFF);   // 方门的起始位置
				pInfo->Gate[i].end = GetBigGatePixel(i / 5, g_filehead.deviceP2.Gate[i].end & 0x7FFF);   // 方门的结束位置
			}
			else
			{
				pInfo->Gate[i].start = 1.0 * (g_filehead.deviceP2.Gate[i].start & 0x7FFF);   // 方门的起始位置
				pInfo->Gate[i].end = 1.0 * (g_filehead.deviceP2.Gate[i].end);   // 方门的结束位置
			}
		}

		g_strTPA = g_strTPB.substr(0, len - 1) + "A";
		g_pFileA = _fsopen(g_strTPA.c_str(), "rb", _SH_DENYWR);
		if (g_pFileA == NULL)
		{
			printf("tpA Open Failed");
			return -5;
		}
		g_pFileB = _fsopen(g_strTPB.c_str(), "rb", _SH_DENYWR);
		if (g_pFileB == NULL)
		{
			printf("tpB Open Failed");
			return -6;
		}
		fseek(g_pFileA, 0L, SEEK_END);
		g_szFileA = ftell(g_pFileA);

		fseek(g_pFileB, 0L, SEEK_END);
		g_szFileB = ftell(g_pFileB);
		pInfo->sizeA = g_szFileA;
		pInfo->sizeB = g_szFileB;
		if (g_szFileA < szFileHead || g_szFileB <= szFileHead)
		{
			return -7;
		}

		fseek(g_pFileB, 0L, SEEK_SET);
		fread(&g_filehead, 1, szFileHead, g_pFileB);
		g_filehead.step = 2.66;   // 每个步进多少毫米
		g_isFileOpened = 1;

		g_vBlockHeads.clear();
		GetBBlocks(g_pFileB, g_vBlockHeads, g_szFileB);   //获取全部B超米块头

		fclose(g_pFileA);
		fclose(g_pFileB);

		pInfo->blockCount = g_vBlockHeads.size();
		for (int i = 0; i < pInfo->blockCount; ++i)
		{
			uint8_t railType = g_vBlockHeads[i].BlockHead.railType & 0x03;   // BIT0~1: 轨型(0为43轨，1-50，2-60，3-75), BIT4:0逆里程、1顺里程，BIT5:0右股、1左股，BIT6~7：单线、上行、下行，其他预留
			for (int j = 0; j < 7; ++j)
			{
				uint16_t isOn = (0x8000 & g_vBlockHeads[i].BlockHead.door[2 * j]);   // 小门设置，DOR_NUM=7，door[x][0]的最高位为on/off
				if (g_isPTData)
				{
					g_vBlockHeads[i].BlockHead.door[2 * j] = g_vBlockHeads[i].BlockHead.door[2 * j] & 0x7FFF;
					g_vBlockHeads[i].BlockHead.door[2 * j + 1] = g_vBlockHeads[i].BlockHead.door[2 * j + 1];
					if (isOn)
					{
						g_vBlockHeads[i].BlockHead.door[2 * j] |= 0x8000;
					}
				}
				else
				{
					g_vBlockHeads[i].BlockHead.door[2 * j] = GetSmallGatePixel(railType, g_vBlockHeads[i].BlockHead.door[2 * j] & 0x7FFF, j);
					g_vBlockHeads[i].BlockHead.door[2 * j + 1] = GetSmallGatePixel(railType, g_vBlockHeads[i].BlockHead.door[2 * j + 1] & 0x7FFF, j);
					if (isOn)
					{
						g_vBlockHeads[i].BlockHead.door[2 * j] |= 0x8000;
					}
				}

			}
			g_vBlockHeads[i].Walk2 = g_vBlockHeads[i].Walk = GetWD(g_vBlockHeads[i].BlockHead.walk);   //米块开始里程
			if (g_vBlockHeads[i].Walk > 9999)
			{
				g_vBlockHeads[i].Walk = g_vBlockHeads[i].Walk - 10000;
				g_vBlockHeads[i].Walk2 = g_vBlockHeads[i].Walk;
			}

			if (!g_isPTData)
			{
				g_vBlockHeads[i].BlockHead.user = BCDToINT16(g_vBlockHeads[i].BlockHead.user);   // 工号,4个BCD
				g_vBlockHeads[i].BlockHead.railNum = BCDToINT16(g_vBlockHeads[i].BlockHead.railNum);   // 钢轨编号,4个BCD码表示
			}
		}

		memset(pInfo->s_time, 0, 20);
		std::string sTime = GetTimeString(g_filehead.startD, g_filehead.startT);   // 年月日  // 开始作业时间
		strncpy(pInfo->s_time, sTime.c_str(), sTime.length());
		printf("s_time = %s\n", pInfo->s_time);

		memset(pInfo->e_time, 0, 20);
		std::string eTime = GetTimeString(g_filehead.endD, g_filehead.endT);   // 结束作业时间
		strncpy(pInfo->e_time, eTime.c_str(), eTime.length());
		printf("e_time = %s\n", pInfo->e_time);

		if (g_isPTData)
		{
			int timestack = g_vBlockHeads[g_vBlockHeads.size() - 1].BlockHead.time;   // 时间：高位至低位每8位依次为：日、时、分、秒
			uint16_t year = g_filehead.startD >> 16;   // 年月日
			uint8_t	month = ((g_filehead.startD & 0xFF00) >> 8);

			int day = timestack >> 24;
			int hour = (timestack >> 16) & 0xFF;
			int minute = (timestack >> 8) & 0xFF;
			int second = (timestack) & 0xFF;

			sprintf(pInfo->e_time, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
		}

		pInfo->s_mil = 0.000001 * g_filehead.startKm.mm + 0.001 * g_filehead.startKm.m + g_filehead.startKm.Km;   //开始里程
		pInfo->e_mil = 0.000001 * g_filehead.endKm.mm + 0.001 * g_filehead.endKm.m + g_filehead.endKm.Km;   //结束里程

		if (g_vBlockHeads.size() == 0)
		{
			printf("Error File\n");
			return -8;
		}

		//校正左右股
		pInfo->carType = (g_vBlockHeads[0].BlockHead.detectSet.Identify & C_LR) != 0 ? 1 : 0;
		/*
		if (pInfo->carType == 1)//右手车
		{
			if (g_direction)
			{
				pInfo->leftright = 0;
			}
			else
			{
				pInfo->leftright = 1;
			}
		}
		else//左手车
		{
			if (g_direction)
			{
				pInfo->leftright = 1;
			}
			else
			{
				pInfo->leftright = 0;
			}
		}
		*/
		g_gubie = pInfo->leftright;

		memset(pInfo->NewFileName, 0, 60);
		std::string newFileName = GetNewFileName(g_filehead, g_strFileName, pInfo->leftright, pInfo->xianbie);   //文件名
		int newFileNameLen = newFileName.length();
		strncpy(pInfo->NewFileName, newFileName.c_str(), newFileNameLen);
		printf("NewFileName = %s\n", pInfo->NewFileName);


		g_isTestEqu = 0;   //是否试块数据
		g_iAmpl = 120;   //判伤幅值
		if (g_filehead.block < 800)   //总共多少米块
		{
			g_isTestEqu = 1; 
			g_iAmpl = 90;
		}
		WriteLog("SetFile Finish\n");
		g_iTotalBlock = g_vBlockHeads.size();

		for (int i = 0; i < CH_N; ++i)
		{
			g_channelOffset[i] = 0;   //各通道参数初始化
		}

		g_vWounds.clear();   //伤损数据
		g_vPMs.clear();   //人工标记位置标数据
		g_vBAs.clear();   //回退数据
		g_vER.clear();   //单通道出波区域
		g_vPMs2.clear();   //所有标记信息
		g_vLC.clear();   //失耦信息
		g_vLCP.clear();   //通道失耦信息
		g_vLD.clear();   //跳探信息
		g_vPMsYOLO.clear();   //所有标记信息（该位置标由YOLO判定）
		g_vBlockSolvedFlags.clear();
		g_vForks.clear();
		g_vPoints.clear();

		g_sztWoundCount = 0;
		g_sztAlarmCount = 0;
		for (auto itr = g_mapWoundTypeCountTemp.begin(); itr != g_mapWoundTypeCountTemp.end(); ++itr)
		{
			itr->second = 0;
		}
		for (auto itr = g_mapWoundTypeCount.begin(); itr != g_mapWoundTypeCount.end(); ++itr)
		{
			itr->second = 0;
		}
		g_iCurrentBlock = 0;

#ifdef YOLOV4
		std::string strPicFolder = g_strFolder + "/pics/";
		SHFILEOPSTRUCTA FileOp;
		FileOp.fFlags = FOF_NOCONFIRMATION | FOF_ALLOWUNDO;
		FileOp.hNameMappings = NULL;
		FileOp.hwnd = NULL;
		FileOp.lpszProgressTitle = NULL;
		FileOp.pFrom = strPicFolder.c_str();
		FileOp.pTo = NULL;
		FileOp.wFunc = FO_DELETE;
		return SHFileOperationA(&FileOp) == 0;
#else

		return 1;
#endif // YOLO	

#ifdef OUTPUT_EX
	}
	catch (CBaseException &e)
	{
		e.ShowExceptionInformation();
	}
#endif // OUTPUT_EX
	return 0;
}

void SetStandardGain(double * pGain, int count)
{
	for (int i = 0; i < count; ++i)
	{
		g_StandardGain[i] = pGain[i];
	}
}

void SetChannelOffset(int* offset, int count)    //设置拼图参数
//输入参数
//offset：各通道拼图参数
//count: 通道个数
{
	for (int i = 0; i < count; ++i)
	{
		g_channelOffset[i] = offset[i];
	}
}

void SetChannelOffset2(ChannelOffset* pOffset, int count)
{
	g_vChannelOffsets.clear();
	for (int i = 0; i < count; ++i)
	{
		g_vChannelOffsets.emplace_back(pOffset[i]);
	}
}

int GetBlocks(BLOCK_B4Nodejs* pBlocks, int blockCount, char* tpbLocalPath)
{
	FILE* pFile = _fsopen(tpbLocalPath, "rb", _SH_DENYWR);
	if (pFile == NULL)
	{
		return 0;
	}
	fseek(pFile, 0L, SEEK_END);
	int filesize = ftell(pFile);
	if (filesize <= szFileHead)
	{
		fclose(pFile);
		return 0;
	}

	fseek(pFile, 0L, SEEK_SET);
	F_HEAD head;
	fread(&head, 1, szFileHead, pFile);

	VBDB blocks;
	GetBBlocks(pFile, blocks, filesize);
	fclose(pFile);

	for (int i = 0; i < blockCount; ++i)
	{
		pBlocks[i].blockHead = blocks[i].BlockHead;
	}
	return blocks.size() == blockCount ? 1 : 0;
}

uint32_t    GetBlockCount()
{
	return g_vBlockHeads.size();
}

void GetWalk(double* walk1, double* walk2)   //获取当前文件作业里程
//输出参数
//walk1：作业区间开始里程
//walk2：作业区间结束里程
{
	int blockCount = g_vBlockHeads.size();
	if (blockCount > 1)
	{
		std::vector<WalkInfo> vInfo;
		WalkInfo wi;
		wi.Block1 = 0;
		wi.Block2 = 0;
		wi.walk1 = g_vBlockHeads[0].Walk;
		wi.walk2 = g_vBlockHeads[0].Walk;
		wi.Length = 0;

		for (int i = 1; i < blockCount; ++i)
		{
			int delt = GetTimeSpan(g_vBlockHeads[i].BlockHead.time, g_vBlockHeads[i - 1].BlockHead.time);
			if (delt > 30 * 60)//半小时分割
			{
				double dwalk = g_vBlockHeads[i].Walk - g_vBlockHeads[i - 1].Walk;
				if (dwalk > 1)//大于1Km
				{
					wi.Block2 = i - 1;
					wi.walk2 = g_vBlockHeads[i - 1].Walk;
					wi.Length = abs(wi.walk2 - wi.walk1);
					vInfo.push_back(wi);
					wi.Block1 = i;
					wi.walk1 = g_vBlockHeads[i].Walk;
				}
			}

			if (i == blockCount - 1)
			{
				wi.Block2 = i;
				wi.walk2 = g_vBlockHeads[i].Walk;
				wi.Length = abs(wi.walk2 - wi.walk1);
				vInfo.push_back(wi);
			}
		}

		int iMaxIndex = 0;
		double maxLen = vInfo[0].Length;
		for (int i = 1; i < vInfo.size(); ++i)
		{
			if (vInfo[i].Length > maxLen)
			{
				maxLen = vInfo[i].Length;
				iMaxIndex = i;
			}
		}

		//一定程度兼容中间里程校正时输错1位的情况
		for (int i = vInfo[iMaxIndex].Block1; i < vInfo[iMaxIndex].Block2; ++i)
		{
			if (abs(g_vBlockHeads[i + 1].Walk - g_vBlockHeads[i].Walk) >= 1)//1 Km
			{
				//2021-10-13 更新，211012Q0008SRS_0002_17S6014，1610米块
				if (i >= 5)
				{
					PM pm;
					memset(&pm, 0, sizeof(PM));
					pm.Block = i + 1;
					pm.Step = 0;
					pm.Step2 = g_vBlockHeads[i + 1].IndexL2;
					pm.Mark = PM_WALK_PULSE;
					pm.Manual = 1;
					g_vPMs2.push_back(pm);
				}

				/*
				char sz[5] = { 0 }, sz2[5] = {0};
				sprintf(sz, "%04d", (int)g_vBlockHeads[i].Walk);
				sprintf(sz2, "%04d", (int)g_vBlockHeads[i + 1].Walk);

				int misMatch = 0;
				for (int j = 0; j < 4; ++j)
				{
					if (sz[j] != sz2[j])
					{
						misMatch++;
					}
				}

				if (misMatch == 1 && g_filehead.deviceP2.TrackSet.lineType == 1)
				{
					double dwalk = g_vBlockHeads[i + 1].Walk - g_vBlockHeads[i].Walk;
					double k = g_direction ? 0.001 : -0.001;
					for (int j = i; j >= vInfo[iMaxIndex].Block1; --j)
					{
						g_vBlockHeads[j].Walk = g_vBlockHeads[i + 1].Walk - k * (i + 1 - j);
					}
				}*/
			}
		}

		*walk1 = g_vBlockHeads[vInfo[iMaxIndex].Block1].Walk;
		*walk2 = g_vBlockHeads[vInfo[iMaxIndex].Block2].Walk;

		/*int iBlockK = -1;
		int iMark = -1;
		if (*walk1 - *walk2 >= 25 || *walk1 - *walk2 <= -25)
		{
			for (int i = iMark + 1; i < g_vPMs.size(); ++i)
			{
				if (g_vPMs[i].Mark == PM_WALKREVISE)
				{
					iBlockK = g_vPMs[i].Block;
					iMark = i;
					break;
				}
			}

			if (iBlockK > 0)
			{
				double w2 = g_vBlockHeads[iBlockK].Walk;
				double w1 = g_vBlockHeads[iBlockK - 1].Walk;
				double delt = w2 - w1 - 0.001;
				for (int i = 0; i < iBlockK; ++i)
				{
					g_vBlockHeads[i].Walk = g_vBlockHeads[i].Walk + delt;
				}
				*walk1 = g_vBlockHeads[0].Walk;
			}
			else
			{
				for (int i = 1; i < blockCount; ++i)
				{
					double dw = g_vBlockHeads[i].Walk - g_vBlockHeads[i - 1].Walk;
					if (fabs(dw) >= 0.5)
					{
						for (int j = 0; j < i; ++j)
						{
							g_vBlockHeads[j].Walk = g_vBlockHeads[j].Walk + dw;
						}
						break;
					}
				}
				*walk1 = g_vBlockHeads[0].Walk;
			}
		}*/
	}
	else  if (blockCount == 1)
	{
		*walk1 = g_vBlockHeads[0].Walk;
		*walk2 = *walk1;
	}
	else
	{
		*walk1 = 0;
		*walk2 = 0;
	}
	for (int i = 0; i < blockCount; ++i)
	{
		g_vBlockHeads[i].Walk2 = g_vBlockHeads[i].Walk;
		if (g_vBlockHeads[i].Walk2 > 9000)
		{
			g_vBlockHeads[i].Walk2 = g_vBlockHeads[i].Walk2 - 10000;
		}
	}

	if (*walk1 > 9000)
	{
		*walk1 = *walk1 - 10000;
	}
	if (*walk2 > 9000)
	{
		*walk2 = *walk2 - 10000;
	}
}


uint8_t BeginAnalyse(int beginSolveIndex, FileSolveInfo* PrevCycleFiles, int PrevCycleCount)   //开始解析SetFile指定的文件
//输入参数
//beginSolveIndex：开始解析米块索引
//PrevCycleFiles：历史周期tpB信息
//PrevCycleCount：历史周期个数
{
	WriteLog("Begin Analyse\n");
	if (g_isInitialized == 0)
	{
		return 0;
	}
	if (g_isRunning == 1 || g_isInitialized == 0)
	{
		return 0;
	}

#ifdef OUTPUT_EX
	try
	{
#endif // OUTPUT_EX
		std::string g_strTPA = g_strTPB.substr(0, g_strTPB.length() - 1);   //原TPA、tpB完整路径
		g_strTPA.append("A");
		g_pFileA = _fsopen(g_strTPA.c_str(), "rb", _SH_DENYWR);
		if (g_pFileA == NULL)
		{
			printf("tpA Open Failed");
			return 0;
		}
		g_pFileB = _fsopen(g_strTPB.c_str(), "rb", _SH_DENYWR);
		if (g_pFileB == NULL)
		{
			printf("tpB Open Failed");
			return 0;
		}

		SetFileHead(g_filehead);   // 设置文件头
		GetABlocks(g_pFileA, g_vBlockHeads, g_szFileA);   //获取A超米块信息vBlockHeads
		for (int i = 0; i < g_vBlockHeads.size(); ++i)
		{
			ParseGPS(g_vBlockHeads[i].BlockHead.gpsInfor, g_vBlockHeads[i].gpsLog, g_vBlockHeads[i].gpsLat);   //gpsLog：经度, gpsLat：纬度
		}
		g_isFinish = 0;

		g_strWavePointFile = g_strLogPath + "/" + g_strFileNameWithoutExt + ".wd";   //创建文件路径 写入所有出波点二进制数据文件路径 g_strFileNameWithoutExt 原数据文件名，不带后缀
		g_pFileWavePoints = fopen(g_strWavePointFile.c_str(), "wb");   //g_pFileWavePoints指针 指向出波点二进制数据文件路径
		if (g_pFileWavePoints == nullptr)   //出波点
		{
			char sss[1000] = { 0 };
			sprintf(sss, "open fail errno = %d reason = %s \n", errno, strerror(errno));
			WriteLog(sss);
		}

		double w1, w2;
		GetWalk(&w1, &w2);   //获取当前文件作业里程
		g_walk1 = w1;
		g_walk2 = w2;

		g_isRunning = 1;
		g_beginSolveMeterIndex = beginSolveIndex;
		g_PrevFileInfo = PrevCycleFiles;   
		g_PrevFileCount = PrevCycleCount;
		//分析线程
		uintptr_t res = _beginthreadex(nullptr, 0, (unsigned int(__stdcall *)(void *))AnalyseThread, nullptr, 0, &g_threadId);
		if (res <= 0)
		{
			return 0;
		}
		return 1;
#ifdef OUTPUT_EX
	}
	catch (CBaseException &e)
	{
		e.ShowExceptionInformation();
	}
#endif // OUTPUT_EX
	return 0;
}

void GetAnalyseProcess(uint32_t* currentUseL, uint32_t* currentCount, uint32_t* woundCount, uint8_t* isFinish)   //查询当前解析进度
//输出参数
//currentUseLess：当前使用了的B超字节数
//currentCount：当前解析的米块数
//woundCount：当前检出伤损数量
//isFinish：整个文件是否解析完成
{
	if (g_iSolvedBlockCount < g_vBlockHeads.size())
	{
		*currentUseL = g_iSolvedBlockCount / g_vBlockHeads.size() * g_szFileB;
	}
	else
	{
		*currentUseL = g_szFileB;
	}
	*currentCount = g_iSolvedBlockCount;
	*woundCount = g_vWounds.size();
	*isFinish = g_isFinish;
}

void StopAnalyse()
{
	g_isRunning = 0;
}


void GetResultCount(uint32_t* woundCount, uint32_t* backCount, uint32_t* markCount, uint32_t* blockCount)
{
	*woundCount = g_vWounds.size();
	*backCount = g_vBAs.size();
	*markCount = g_vPMs.size();
	*blockCount = g_vBlockHeads.size();
}

void GetSolveInfoItemCount(FileSolveInfo* info)   //获取当前文件解析结果数
{
	WriteLog("GetSolveInfoItemCount Begin");

	memset(info->DataPath, 0, 100);
	for (int i = 0; i < g_strNewFileName.length(); ++i)
	{
		info->DataPath[i] = g_strNewFileName[i];
	}

	memset(info->FileName, 0, 100);
	for (int i = 0; i < g_strFileNameWithoutExt.length(); ++i)
	{
		info->FileName[i] = g_strFileNameWithoutExt[i];
	}

	info->BlockCount = g_vBlockHeads.size();
	info->BackCount = g_vBAs.size();
	info->MarkCount = g_vPMs.size();
	info->WoundCount = g_vWounds.size();

	info->QualityInfo.ErroeRectCount = g_vER.size();
	info->QualityInfo.LoseCoupleCount = g_vLC.size();
	info->QualityInfo.ManualMarkCount = g_vPMs2.size();
	info->QualityInfo.LoseDetectCount = g_vLD.size();

	info->IsTestEqu = g_isTestEqu;
	info->sztAlarmCount = g_sztAlarmCount;
	info->sztWoundCount = g_sztWoundCount;
}

void GetSolveInfo(FileSolveInfo* info)   //获取当前文件解析结果数
{
	WriteLog("GetSolveInfo Begin");
	for (int i = 0; i < info->BlockCount; ++i)
	{
		info->Blocks[i].Index = g_vBlockHeads[i].Index;
		info->Blocks[i].IndexL = g_vBlockHeads[i].IndexL;
		info->Blocks[i].IndexL2 = g_vBlockHeads[i].IndexL2;
		//info->Blocks[i].Walk = g_vBlockHeads[i].Walk;
		info->Blocks[i].Walk = g_vBlockHeads[i].BlockHead.walk.Km + 0.001 * g_vBlockHeads[i].BlockHead.walk.m + 0.000001 * g_vBlockHeads[i].BlockHead.walk.mm;
		info->Blocks[i].Walk2 = g_vBlockHeads[i].Walk2;
		info->Blocks[i].gpsLat = g_vBlockHeads[i].gpsLat;
		info->Blocks[i].gpsLog = g_vBlockHeads[i].gpsLog;
		info->Blocks[i].AStartPos = g_vBlockHeads[i].AStartPos;
		info->Blocks[i].BStartPos = g_vBlockHeads[i].BStartPos;
		info->Blocks[i].FrameCount = g_vBlockHeads[i].FrameCount;
		info->Blocks[i].FrameCountRead = g_vBlockHeads[i].FrameCountRead;
		info->Blocks[i].blockHead = g_vBlockHeads[i].BlockHead;
	}
	printf("%s\n", "block finish");

	printf("wound begin : %d\n", g_vWounds.size());
	memset(info->Wounds, 0, info->WoundCount * sizeof(Wound4Nodejs));
	std::string str = "", strAccording = "";
	std::vector<std::string> vStr(20);
	char strTemp[400] = { 0 };
	for (int i = 0; i < info->WoundCount; ++i)
	{
		printf("wound index : %d\n", i);
		strncpy(info->Wounds[i].Result, g_vWounds[i].Result, 60);
		str = GetWoundAccordding(g_vWounds[i], g_vBlockHeads);
		int len = str.length();
		if (len > 999)
		{
			strncpy(info->Wounds[i].According, str.c_str(), 999);
			info->Wounds[i].According[999] = '\0';
		}
		else
		{
			strncpy(info->Wounds[i].According, str.c_str(), len);
		}

		info->Wounds[i].Type = g_vWounds[i].Type;
		info->Wounds[i].XSize = g_vWounds[i].SizeX;
		info->Wounds[i].YSize = g_vWounds[i].SizeY;
		info->Wounds[i].Place = g_vWounds[i].Place;
		info->Wounds[i].Degree = g_vWounds[i].Degree;

		info->Wounds[i].Block = g_vWounds[i].Block;
		info->Wounds[i].Step = g_vWounds[i].Step;
		info->Wounds[i].Step2 = g_vWounds[i].Rect.step1;
		info->Wounds[i].StepLeft = g_vWounds[i].Rect.step2;
		info->Wounds[i].Row1 = g_vWounds[i].Rect.row1;
		info->Wounds[i].Row2 = g_vWounds[i].Rect.row2;


		info->Wounds[i].Walk = g_vWounds[i].Walk;
		info->Wounds[i].Walk2 = g_vWounds[i].Walk2;

		info->Wounds[i].Lat = info->Blocks[info->Wounds[i].Block].gpsLat;
		info->Wounds[i].Log = info->Blocks[info->Wounds[i].Block].gpsLog;

		info->Wounds[i].Manual = g_vWounds[i].Manual;
		info->Wounds[i].equType = g_vWounds[i].equType;

		info->Wounds[i].IsBridge = g_vWounds[i].IsBridge;
		info->Wounds[i].IsTunnel = g_vWounds[i].IsTunnel;
		info->Wounds[i].IsCurve = g_vWounds[i].IsCurve;

		info->Wounds[i].IsJoint = g_vWounds[i].IsJoint;
		info->Wounds[i].IsSew = g_vWounds[i].IsSew;
		info->Wounds[i].IsScrewHole = g_vWounds[i].IsScrewHole;
		info->Wounds[i].IsGuideHole = g_vWounds[i].IsGuideHole;
		//info->Wounds[i].Level = BIT1; 
		info->Wounds[i].Level = (uint8_t)g_vWounds[i].FileID;
		info->Wounds[i].Cycle = 1;
	}

	//FILE* pFileWound = fopen("D:/wound.txt", "w");
	//for (int i = 0; i < *woundCount; ++i)
	//{
	//	fprintf(pFileWound, "%d, %d, %d\n", pWounds[i].Block, pWounds[i].Step, pWounds[i].Type);
	//} 
	//fclose(pFileWound);

	std::map<uint32_t, uint8_t> isExsit;
	std::vector<int32_t> vMulti;
	for (int i = 0; i < info->WoundCount; ++i)
	{
		if (isExsit.find(info->Wounds[i].Step2) != isExsit.end())
		{
			isExsit[info->Wounds[i].Step2] = isExsit[info->Wounds[i].Step2] + 1;
			vMulti.emplace_back(i);
		}
		else
		{
			isExsit[info->Wounds[i].Step2] = 0;
		}
	}

	for (int i = 0; i < vMulti.size(); ++i)
	{
		int step2 = info->Wounds[vMulti[i]].Step2;
		while (isExsit.find(step2) != isExsit.end() && step2 >= 0)
		{
			step2++;
		}
		info->Wounds[vMulti[i]].Step2 = step2;
		isExsit[step2] = 0;
	}

	printf("%s\n", "wound finish");

	for (int i = 0; i < info->BackCount; ++i)
	{
		info->Backs[i] = g_vBAs[i];
	}
	printf("%s\n", "back finish");

	for (int i = 0; i < info->MarkCount; ++i)
	{
		PMToPMRaw(g_vPMs[i], info->Marks[i]);
	}
	printf("%s\n", "mark finish");


	for (int i = 0; i < g_vPMs2.size(); ++i)
	{
		PMToPMRaw(g_vPMs2[i], info->QualityInfo.ManualMarKs[i]);
	}

	for (int i = 0; i < g_vLC.size(); ++i)
	{
		info->QualityInfo.LoseCouples[i] = g_vLC[i];
	}

	for (int i = 0; i < g_vER.size(); ++i)
	{
		info->QualityInfo.ErroeRects[i] = g_vER[i];
	}
	for (int i = 0; i < g_vLD.size(); ++i)
	{
		info->QualityInfo.LoseDetects[i] = g_vLD[i];
	}
}

void GetRealTimeAlgSolveDetail(int * woundTypes, int * woundCounts, int * woundTypeCount)
{
	*woundTypeCount = g_mapWoundTypeCount.size();
	auto& itr = g_mapWoundTypeCount.begin();
	for (int i = 0; i < *woundTypeCount; ++i)
	{		
		woundTypes[i] = itr->first;
		woundCounts[i] = itr->second;
		itr++;
	}
}

void GetResult2(FileBlock4Nodejs* pFiles, uint32_t fileCount,
	Position_Mark_RAW* pMarksPrev, uint32_t markCountPrev,
	Wound4Nodejs* pWoundPrev, uint32_t woundCountPrev,
	BackAction* pBackActionsPrev, uint32_t backCountPrev,

	Position_Mark_RAW* pMarks, uint32_t*markCount,
	Wound4Nodejs* pWounds, uint32_t* woundCount,
	BackAction* pBackActions, uint32_t* backCount,
	BLOCK_B4Nodejs* pBlocks, uint32_t*blockCount
)
{
#pragma region 单周期数据赋值
	char* szLog = new char[1000];
	sprintf(szLog, "GetResult2 Begin： fileCount = %d\n", fileCount);
	WriteLog(szLog);
	delete szLog;
	*blockCount = g_vBlockHeads.size();
	for (int i = 0; i < *blockCount; ++i)
	{
		ParseGPS(g_vBlockHeads[i].BlockHead.gpsInfor, g_vBlockHeads[i].gpsLog, g_vBlockHeads[i].gpsLat);
		pBlocks[i].Index = g_vBlockHeads[i].Index;
		pBlocks[i].IndexL = g_vBlockHeads[i].IndexL;
		pBlocks[i].IndexL2 = g_vBlockHeads[i].IndexL2;
		pBlocks[i].Walk = g_vBlockHeads[i].Walk;
		pBlocks[i].Walk2 = g_vBlockHeads[i].Walk2;
		pBlocks[i].gpsLat = g_vBlockHeads[i].gpsLat;
		pBlocks[i].gpsLog = g_vBlockHeads[i].gpsLog;
		pBlocks[i].AStartPos = g_vBlockHeads[i].AStartPos;
		pBlocks[i].BStartPos = g_vBlockHeads[i].BStartPos;
		pBlocks[i].FrameCount = g_vBlockHeads[i].FrameCount;
		pBlocks[i].FrameCountRead = g_vBlockHeads[i].FrameCountRead;
		pBlocks[i].blockHead = g_vBlockHeads[i].BlockHead;
	}
	printf("%s\n", "block finish");

	printf("wound begin : %d\n", g_vWounds.size());
	*woundCount = (uint32_t)g_vWounds.size();
	printf("sizeof(Wound) = %d\n", sizeof(Wound4Nodejs));
	memset(pWounds, 0, *woundCount * sizeof(Wound4Nodejs));
	std::string str = "", strAccording = "";
	std::vector<std::string> vStr(20);
	char strTemp[400] = { 0 };
	for (int i = 0; i < *woundCount; ++i)
	{
		printf("wound index : %d\n", i);
		strncpy(pWounds[i].Result, g_vWounds[i].Result, 60);
		str = GetWoundAccordding(g_vWounds[i], g_vBlockHeads);
		int len = str.length();
		if (len > 999)
		{
			strncpy(pWounds[i].According, str.c_str(), 999);
			pWounds[i].According[999] = '\0';
		}
		else
		{
			strncpy(pWounds[i].According, str.c_str(), len);
		}

		pWounds[i].Type = g_vWounds[i].Type;
		pWounds[i].XSize = g_vWounds[i].SizeX;
		pWounds[i].YSize = g_vWounds[i].SizeY;
		pWounds[i].Place = g_vWounds[i].Place;
		pWounds[i].Degree = g_vWounds[i].Degree;

		pWounds[i].Block = g_vWounds[i].Block;
		pWounds[i].Step = g_vWounds[i].Step;
		pWounds[i].Step2 = g_vWounds[i].Rect.step1;
		pWounds[i].StepLeft = g_vWounds[i].Rect.step2;
		pWounds[i].Row1 = g_vWounds[i].Rect.row1;
		pWounds[i].Row2 = g_vWounds[i].Rect.row2;


		pWounds[i].Walk = g_vWounds[i].Walk;
		pWounds[i].Walk2 = g_vWounds[i].Walk;

		pWounds[i].Lat = pBlocks[pWounds[i].Block].gpsLat;
		pWounds[i].Log = pBlocks[pWounds[i].Block].gpsLog;

		pWounds[i].Manual = g_vWounds[i].Manual;
		pWounds[i].equType = g_vWounds[i].equType;

		pWounds[i].IsBridge = g_vWounds[i].IsBridge;
		pWounds[i].IsTunnel = g_vWounds[i].IsTunnel;
		pWounds[i].IsCurve = g_vWounds[i].IsCurve;

		pWounds[i].IsJoint = g_vWounds[i].IsJoint;
		pWounds[i].IsSew = g_vWounds[i].IsSew;
		pWounds[i].IsScrewHole = g_vWounds[i].IsScrewHole;
		pWounds[i].IsGuideHole = g_vWounds[i].IsGuideHole;
		//pWounds[i].Level = BIT1; 
		pWounds[i].Level = (uint8_t)g_vWounds[i].FileID;
		pWounds[i].Cycle = 1;
	}

	//FILE* pFileWound = fopen("D:/wound.txt", "w");
	//for (int i = 0; i < *woundCount; ++i)
	//{
	//	fprintf(pFileWound, "%d, %d, %d\n", pWounds[i].Block, pWounds[i].Step, pWounds[i].Type);
	//} 
	//fclose(pFileWound);

	std::map<uint32_t, uint8_t> isExsit;
	std::vector<int32_t> vMulti;
	for (int i = 0; i < *woundCount; ++i)
	{
		if (isExsit.find(pWounds[i].Step2) != isExsit.end())
		{
			isExsit[pWounds[i].Step2] = isExsit[pWounds[i].Step2] + 1;
			vMulti.emplace_back(i);
		}
		else
		{
			isExsit[pWounds[i].Step2] = 0;
		}
	}

	for (int i = 0; i < vMulti.size(); ++i)
	{
		int step2 = pWounds[vMulti[i]].Step2;
		while (isExsit.find(step2) != isExsit.end() && step2 >= 0)
		{
			step2++;
		}
		pWounds[vMulti[i]].Step2 = step2;
		isExsit[step2] = 0;
	}

	printf("%s\n", "wound finish");
	*backCount = (uint32_t)g_vBAs.size();
	for (int i = 0; i < *backCount; ++i)
	{
		pBackActions[i] = g_vBAs[i];
	}
	printf("%s\n", "back finish");
	*markCount = (uint32_t)g_vPMs.size();
	memset(pMarks, 0, *markCount * sizeof(Position_Mark_RAW));
	for (int i = 0; i < *markCount; ++i)
	{
		g_vPMs[i].Walk = GetWD(g_vBlockHeads[g_vPMs[i].Block].Walk, g_vPMs[i].Step, 2.66, g_direction);
		PMToPMRaw(g_vPMs[i], pMarks[i]);
		pMarks[i].Walk2 = pMarks[i].Walk;
	}
	printf("%s\n", "mark finish");
#pragma endregion

	if (fileCount > 0)
	{
#pragma region 精对齐数据准备

		std::string lastFolder = g_strWorkPath + "/Data1";
		std::string currFolder = g_strWorkPath + "/Data2";

		CreateDirectoryA(lastFolder.c_str(), NULL);//上周期文件
		CreateDirectoryA(currFolder.c_str(), NULL);//笨周期文件
		std::string strFileMarkPrev = lastFolder + "/mark.txt";// "D:/Files/Data1/mark.txt";
		FILE* pFile = fopen(strFileMarkPrev.c_str(), "w");
		for (int i = 0; i < markCountPrev; ++i)
		{
			//fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d,%d\n", pMarksPrev[i].Block, pMarksPrev[i].Mark, pMarksPrev[i].Walk2, pMarksPrev[i].Size, pMarksPrev[i].ScrewHoleCount, pMarksPrev[i].Manual, pMarksPrev[i].Step);
			fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d\n", pMarksPrev[i].Block, pMarksPrev[i].Mark, pMarksPrev[i].Walk2, pMarksPrev[i].Size, pMarksPrev[i].ScrewHoleCount, pMarksPrev[i].Manual);
		}
		fclose(pFile);

		std::string strFileBlockPrev = lastFolder + "/block.txt";// "D:/Files/Data1/block.txt";
		pFile = fopen(strFileBlockPrev.c_str(), "w");
		for (int i = 0; i < pFiles->BlockCount; ++i)
		{
			//fprintf(pFile, "%d,%.6lf,%lf,%lf,%d,%d,%d,%d\n", pFiles->Blocks[i].Index, pFiles->Blocks[i].Walk2, pFiles->Blocks[i].gpsLog, pFiles->Blocks[i].gpsLat, pFiles->Blocks[i].blockHead.swNum, pFiles->Blocks[i].blockHead.railType, pFiles->Blocks[i].blockHead.railH, pFiles->Blocks[i].blockHead.row);
			fprintf(pFile, "%d,%.6lf,%lf,%lf,%d,%d,%d\n", pFiles->Blocks[i].Index, pFiles->Blocks[i].Walk2, pFiles->Blocks[i].gpsLog, pFiles->Blocks[i].gpsLat, pFiles->Blocks[i].blockHead.swNum, pFiles->Blocks[i].blockHead.railType, pFiles->Blocks[i].blockHead.railH);
		}
		fclose(pFile);

		std::string strFileBackPrev = lastFolder + "/back.txt";// "D:/Files/Data1/back.txt";
		pFile = fopen(strFileBackPrev.c_str(), "w");
		for (int i = 0; i < backCountPrev; ++i)
		{
			if (pBackActionsPrev[i].Pos1.Block >= pFiles->BlockCount)
			{
				continue;
			}
			fprintf(pFile, "%d, %lf\n", pBackActionsPrev[i].Pos1.Block, pFiles->Blocks[pBackActionsPrev[i].Pos1.Block + 1].Walk - pBackActionsPrev[i].Pos1.Walk);
		}
		fclose(pFile);

		std::string strFileHeadPrev = lastFolder + "/filehead.txt";// "D:/Files/Data1/filehead.txt";
		pFile = fopen(strFileHeadPrev.c_str(), "w");
		int idx = -1, iFindCount = 0;
		for (int i = 59; i >= 0; --i)
		{
			if (pFiles[0].FileName[i] == '_')
			{
				iFindCount++;
				if (iFindCount == 2)
				{
					idx = i;
					break;
				}
			}
		}

		fprintf(pFile, "%d\n", pFiles[0].FileName[idx - 8] == 'S' ? 1 : 0);
		fclose(pFile);

		if (g_pFileLog != NULL)
		{
			fprintf(g_pFileLog, "SN = %d\n", pFiles[0].FileName[idx - 8] == 'S' ? 1 : 0);
			fflush(g_pFileLog);
		}

		bool bDirectionPrev = pFiles[0].FileName[idx - 8] == 'S' ? 1 : 0;

		std::string strFileWoundPrev = lastFolder + "/wound.txt";// "D:/Files/Data1/wound.txt";
		pFile = fopen(strFileWoundPrev.c_str(), "w");
		for (int i = 0; i < woundCountPrev; ++i)
		{
			fprintf(pFile, "%d,%lf,%d\n", pWoundPrev[i].Block, pWoundPrev[i].Walk2, pWoundPrev[i].Type);
		}
		fclose(pFile);


		std::string strFileMark = currFolder + "/mark.txt";// "D:/Files/Data2/mark.txt";
		pFile = fopen(strFileMark.c_str(), "w");
		for (int i = 0; i < *markCount; ++i)
		{
			//fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d,%d\n", pMarks[i].Block, pMarks[i].Mark, pMarks[i].Walk2, pMarks[i].Size, pMarks[i].ScrewHoleCount, pMarks[i].Manual, pMarks[i].Step);
			fprintf(pFile, "%d,%d,%.6lf,%d,%d,%d\n", pMarks[i].Block, pMarks[i].Mark, pMarks[i].Walk2, pMarks[i].Size, pMarks[i].ScrewHoleCount, pMarks[i].Manual);
		}
		fclose(pFile);

		std::string strFileBlock = currFolder + "/block.txt";//"D:/Files/Data2/block.txt";
		pFile = fopen(strFileBlock.c_str(), "w");
		for (int i = 0; i < *blockCount; ++i)
		{
			//fprintf(pFile, "%d,%.6lf,%lf,%lf,%d,%d,%d,%d\n", pBlocks[i].Index, pBlocks[i].Walk, pBlocks[i].gpsLog, pBlocks[i].gpsLat, pBlocks[i].blockHead.swNum, pBlocks[i].blockHead.railType, pBlocks[i].blockHead.railH, pBlocks[i].blockHead.row);
			fprintf(pFile, "%d,%.6lf,%lf,%lf,%d,%d,%d\n", pBlocks[i].Index, pBlocks[i].Walk, pBlocks[i].gpsLog, pBlocks[i].gpsLat, pBlocks[i].blockHead.swNum, pBlocks[i].blockHead.railType, pBlocks[i].blockHead.railH);
		}
		fclose(pFile);

		std::string strBack = currFolder + "/back.txt";//"D:/Files/Data2/back.txt";
		pFile = fopen(strBack.c_str(), "w");
		for (int i = 0; i < *backCount; ++i)
		{
			if (pBackActions[i].Pos1.Block >= *blockCount)
			{
				continue;
			}
			fprintf(pFile, "%d, %lf\n", pBackActions[i].Pos1.Block, pBlocks[pBackActions[i].Pos1.Block + 1].Walk - pBackActions[i].Pos1.Walk);
		}
		fclose(pFile);

		std::string strFileHead = currFolder + "/filehead.txt";//"D:/Files/Data2/filehead.txt";
		pFile = fopen(strFileHead.c_str(), "w");
		fprintf(pFile, "%d\n", g_direction ? 1 : 0);
		fclose(pFile);

		std::string strFileWound = currFolder + "/wound.txt";//"D:/Files/Data2/wound.txt";
		pFile = fopen(strFileWound.c_str(), "w");
		for (int i = 0; i < *woundCount; ++i)
		{
			fprintf(pFile, "%d,%lf,%d\n", pWounds[i].Block, pWounds[i].Walk, pWounds[i].Type);
		}
		fclose(pFile);

		std::string strFileOut = g_strWorkPath + "/result.txt";//"D:/Files/result.txt";
		std::string strFileOut2 = g_strWorkPath + "/result2.txt";//"D:/Files/result2.txt";
		remove(strFileOut.c_str());
		remove(strFileOut2.c_str());


		WriteLog("Align Begin\n");
		time_t t1 = time(NULL);
		//int nRet = (int)ShellExecuteA(NULL, "open", "cmd.exe", "/c python D:\\Files\\main.py D:\\Files\\Data2\\ D:\\Files\\Data1\\ D:\\Files\\result.txt D:\\Files\\result2.txt", NULL, SW_HIDE);
		(int)ShellExecuteA(NULL, "open", "cmd.exe", "/c python ./main.py ./Data2/ ./Data1/ ./result.txt ./result2.txt", g_strWorkPath.c_str(), SW_HIDE);
		WriteLog("Read Result Begin\n");
		while (1)
		{
			time_t t2 = time(NULL);
			double timeCost = difftime(t2, t1);
			pFile = fopen(strFileOut2.c_str(), "r+");
			if (pFile != NULL)
			{
				Sleep(1);
				fprintf(pFile, "Timecost = %lf\n", timeCost);
				fflush(pFile);
				fclose(pFile);
				break;
			}
			if (timeCost >= 60)
			{
				WriteLog("GetResult2 timeCost > 60s\n");
				break;
			}
			Sleep(10);
		}

		pFile = fopen(strFileOut.c_str(), "r");
		if (pFile == NULL)
		{
			WriteLog("GetResult2 Result File Open failed\n");
			WriteLog("LastCycleFile:");
			WriteLog(pFiles[0].FileName);
			WriteLog("\n");
			return;
		}

		int nBlock;
		double wd;

		char sztemp[20];
		fread(sztemp, 1, 20, pFile);
		int idxDrumma = -1, idxDot = -1;
		for (int i = 0; i < 20; ++i)
		{
			if (sztemp[i] == '.')
			{
				idxDot = i;
				if (idxDrumma < 0)
				{
					fclose(pFile);
					WriteLog("MultiCycle Error\n");
					remove(strFileOut.c_str());
					remove(strFileOut2.c_str());
					return;
				}
			}
			if (sztemp[i] == ',')
			{
				idxDrumma = i;
			}
		}

		fseek(pFile, 0, SEEK_SET);
		while (!feof(pFile))
		{
			fscanf(pFile, "%d, %lf", &nBlock, &wd);
			pBlocks[nBlock].Walk2 = wd;
		}
		fclose(pFile);
		WriteLog("Read Result End\n");
#pragma endregion

		//std::string newFileResultName = "D:/Files/";
		//newFileResultName += g_strFileName;
		//newFileResultName += "-";
		//newFileResultName += pFiles[0].FileName;
		//newFileResultName += ".txt";
		//::CopyFileA(strFileOut.c_str(), newFileResultName.c_str(), false);


		//先计算位置标的值，再更新米块的起始里程
		double delt2 = g_direction == 1 ? 0.000001 * g_filehead.step : -0.000001 * g_filehead.step;
		for (int i = 0; i < *markCount; ++i)
		{
			if (pMarks[i].Mark != PM_WALKREVISE)
			{
				pMarks[i].Walk2 = pBlocks[pMarks[i].Block].Walk2 + delt2 * pMarks[i].Step;
			}
		}

		for (int i = 0; i < *woundCount; ++i)
		{
			double w = pBlocks[pWounds[i].Block].Walk2 + delt2 * pWounds[i].Step;
			pWounds[i].Walk2 = w;
		}

		remove(strFileOut.c_str());
		remove(strFileOut2.c_str());

		WriteLog("Align Finish\n");
	}
	WriteLog("GetResult2 Finish\n");
}


void		 GetDQCount(uint32_t* manualMarkCount, uint32_t* loseCoupleCount, uint32_t* erCount, uint32_t* ldCount)
{
	*manualMarkCount = g_vPMs2.size();
	*loseCoupleCount = g_vLC.size();
	*erCount = g_vER.size();
	*ldCount = g_vLD.size();
}

void		 GetDQ(Position_Mark_RAW* pMarks, LoseCouple* pLC, ErroeRect* pER, LoseDetect* pLD)
{
	if (pMarks != nullptr)
	{
		for (int i = 0; i < g_vPMs2.size(); ++i)
		{
			PMToPMRaw(g_vPMs2[i], pMarks[i]);
		}
	}

	if (pLC != nullptr)
	{
		for (int i = 0; i < g_vLC.size(); ++i)
		{
			pLC[i] = g_vLC[i];
		}
	}

	if (pER != nullptr)
	{
		for (int i = 0; i < g_vER.size(); ++i)
		{
			pER[i] = g_vER[i];
		}
	}

	if (pLD != nullptr)
	{
		for (int i = 0; i < g_vLD.size(); ++i)
		{
			pLD[i] = g_vLD[i];
		}
	}
}

void		 GetFileAttr(uint8_t& isTestEqu, int& sztWoundCount, int& sztAlarmCount)
{
	isTestEqu = g_isTestEqu;
	sztWoundCount = g_sztWoundCount;
	sztAlarmCount = g_sztAlarmCount;
}

uint32_t SolveTPA(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t filesize, A_Frame4Nodejs* pFrames, uint32_t* frameCount)   //获取1米A超数据
//输入参数
//strFile：文件完整路径
//useL:A超该米块在文件中的字节位置
//blockIndex：当前米块索引
//beginStep：该米块起始步进
//filesize：tpA文件字节数
//输出参数
//pFrames：A超帧
//frameCount：帧数
{
	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "\n%s %d Begin GetAData\n", strFile, blockIndex);
		fflush(g_pFileLog);
	}
	*frameCount = 0;
	int ret = SolveTPA_Export(strFile, useL, blockIndex, beginStep, filesize, pFrames, frameCount);
	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "End GetAData\n", "");
		fflush(g_pFileLog);
	}
	return ret;
}

uint32_t SolveTPB(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t fileSize, BLOCK* blockHead, B_Step4Nodejs* steps, uint32_t* stepCount, B_Row4Nodejs* pRow, uint32_t* rowCount)   //获取1米B超数据
//输入参数
//strFile：文件完整路径
//useL : B超该米块在文件中的字节位置
//blockIndex：当前米块索引
//beginStep：该米块起始步进
//filesize：tpB文件字节数
//输出参数
//blockHead：B超米块头
//pSteps：B超步进数据
//stepCount：步进数
//pRow：B超行数据
//rowCount：B超行数
{
	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "\n%s %d Begin GetBData\n", strFile, blockIndex);
		fflush(g_pFileLog);
	}

	*stepCount = 0;
	*rowCount = 0;
	uint32_t ret = SolveTPB_Export(strFile, useL, blockIndex, beginStep, fileSize, blockHead, steps, stepCount, pRow, rowCount);

	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "End GetBData\n", strFile, blockIndex);
		fflush(g_pFileLog);
	}
	return *stepCount;
}


uint32_t SolveTPB2(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t fileSize, BLOCK* blockHead, B_Step4NodejsPT* steps, uint32_t* stepCount, B_POINT* pPoints, uint32_t* pointCount)
{
	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "\n%s %d Begin GetBData\n", strFile, blockIndex);
		fflush(g_pFileLog);
	}

	*stepCount = 0;
	uint32_t ret = SolveTPB_ExportPT(strFile, useL, blockIndex, beginStep, fileSize, blockHead, steps, stepCount, pPoints, pointCount);

	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "End GetBData\n", strFile, blockIndex);
		fflush(g_pFileLog);
	}
	return *stepCount;
}

uint32_t SolveTPB3(char* strFile, int useL, uint32_t blockIndex, uint32_t beginStep, uint32_t fileSize, BLOCK* blockHead, B_Step4NodejsPT* steps, uint32_t* stepCount, B_Row4Nodejs* pRow, uint32_t* rowCount)
{
	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "\n%s %d Begin GetBData\n", strFile, blockIndex);
		fflush(g_pFileLog);
	}

	*stepCount = 0;
	uint32_t ret = SolveTPB_ExportPT2(strFile, useL, blockIndex, beginStep, fileSize, blockHead, steps, stepCount, pRow, rowCount);

	if (g_pFileLog != NULL)
	{
		fprintf(g_pFileLog, "End GetBData\n", strFile, blockIndex);
		fflush(g_pFileLog);
	}
	return *stepCount;
}



uint32_t    SolveTPA_C(std::string& strFileA, int iFileSize, BlockData_B& blockB, BlockData_A& vADatas, uint32_t& iBeginFrame)
{
	FILE* pFileA = fopen(strFileA.c_str(), "rb");
	uint16_t ret = GetBlockAFrames(pFileA, iFileSize, blockB, vADatas, iBeginFrame);
	fclose(pFileA);
	return ret;
}

uint32_t    SolveTPB_C(std::string& strFileB, int iFileSize, BlockData_B& blockB, BlockData_B& block)
{
	FILE* pFileB = fopen(strFileB.c_str(), "rb");
	uint32_t steps = GetBlockBSteps(pFileB, iFileSize, blockB, block);
	fclose(pFileB);
	return steps;
}



uint8_t		GetWoundRect(int index, uint8_t iJawRow, TRECT* rect, uint8_t isFold /* = 0 */)
{
	if (index < 0 || index >= g_vWounds.size() || (g_vWounds[index].vCRs.size() == 0 && g_vWounds[index].vLCRs.size() == 0 && g_vWounds[index].IsMatched == 0))
	{
		return 0;
	}

	return GetWoundRect2(g_vWounds[index], iJawRow, rect, isFold);
}

void		 GetResult_C(VWJ& vWounds, VPM& vPMs, VBA& vBA, VPM& vPMs2, VBDB& vBlocks, VLC& vLC, VER& vER, VLD& vLD)
{
	vWounds = g_vWounds;
	vPMs = g_vPMs;
	vPMs2 = g_vPMs2;
	vBA = g_vBAs;
	vBlocks = g_vBlockHeads;
	vLC = g_vLC;
	vER = g_vER;
	vLD = g_vLD;
}


void		 GetResult_CPP(SW_FileInfo& fileInfo)
{
	fileInfo.strTPB = g_strTPB;
	fileInfo.head = g_filehead;
	fileInfo.isTestEqu = g_isTestEqu;
	fileInfo.sztWoundCount = g_sztWoundCount;
	fileInfo.sztAlarmCount = g_sztAlarmCount;

	fileInfo.blocks = g_vBlockHeads;
	fileInfo.wounds = g_vWounds;
	fileInfo.marks = g_vPMs;
	fileInfo.marks2 = g_vPMs2;
	fileInfo.backs = g_vBAs;
	fileInfo.loseCouples = g_vLC;
	fileInfo.loseDetects = g_vLD;
	fileInfo.errorRects = g_vER;
}

void SetDeleteTempFile(uint8_t _isDeleteFile)
{
	g_isDeleteDQFile = _isDeleteFile;
}

void SetSaveRawIamge(uint8_t _isSaveRawImage)
{
	g_isSaveRawImage = (_isSaveRawImage == 1);
}