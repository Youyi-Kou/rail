#include "stdafx.h"


map<uint16_t, char*> g_strMarkDefines;

map<uint8_t, char*> g_strGuBieDefines;// = { { 0, "右股" }, { 1, "左股" } };

map<uint8_t, char*> g_strXingBieDefines;// = { { 0, "上行" }, { 1, "下行" }, { 2, "单线" } };

map<uint16_t, const TCHAR*> g_strWoundDefines;/* = {
	{ 1, "核伤" }, { 2, "鱼鳞伤" }, { 4, "轨颚水平裂纹" }, { 8, "轨腰水平裂纹" }, { 16, "螺孔斜裂纹" },
	{ 17, "螺孔一象限裂纹" }, { 18, "螺孔2象限裂纹" }, { 19, "螺孔3象限裂纹" }, { 20, "螺孔四象限裂纹" },
	{ 32, "螺孔水平裂纹" }, { 64, "纵向裂纹" }, { 128, "斜裂纹" }, { 256, "轨底横向裂纹" }
};*/

map<uint16_t, const TCHAR*> g_strDegreeDefines;// = { { 0, "疑似" }, { 1, "不到轻伤" }, { 2, "轻伤" }, { 3, "轻发" }, { 4, "重伤" }, { 5, "折断" } };

map<uint16_t, const TCHAR*> g_strWoundPlaceDefines;/* =
{
	{ 1, "轨头踏面中" }, { 2, "轨头内" }, { 4, "轨头中" }, { 8, "轨头外" },
	{ 16, "轨距角" }, { 32, "轨颚内侧" }, { 64, "轨颚外侧" }, { 128, "轨腰" },
	{ 256, "轨底" }, { 512, "轨底角内" }, { 1024, "轨底角外" }
};*/

map<uint8_t, char*> g_strCheckStateDefines;// = { { 0, "未复核" }, { 1, "已复核" } };


