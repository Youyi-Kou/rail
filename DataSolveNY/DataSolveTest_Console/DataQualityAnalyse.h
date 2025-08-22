#include "SolveData.h"
#include <stdio.h>
#include <string>

#define MAX_DEFECTDATA_NUM      300000
#define SEARCH_DEFECTDATA_NUM   100
#define MAX_ALARMDATA_NUM       500000
#define SEARCH_ALARMDATA_NUM    500
#define MAX_SIGNDATA_NUM        32000
#define MAX_OVER_SPEED_NUM      10000
#define MAX_EDIT_DEFECT_NUM     1000        // �������༭��Ŀ
#define SIGN_NUM                11          // һ������Ŀ


#define LINE_P                  72          // �����еĵ���
#define LINE_P_V20              68          // �����еĵ���

//-----------------------------------------------
// һ�������ݽṹ
//-----------------------------------------------
struct TSignData
{
	uint32_t Number;                          // ��ţ���0��ʼ��
	uint32_t MeterIndex_S;                      // ������ڵ��׿�������
	uint16_t StepIndex_S;                       // ������ڵĲ��������ţ����׿��ڵ����λ�ã�
	uint32_t MeterIndex_E;                      // �յ����ڵ��׿�������
	uint16_t StepIndex_E;                       // �յ����ڵĲ��������ţ����׿��ڵ����λ�ã�
	uint16_t SignType;                        // ������ͣ�0~8����ӦMark��BIT15~BIT7��9 - ���У����10 - ��ϲ�����
	uint16_t SpecialInfo;                      // �ر���Ϣ����ϲ���ͨ��/�Զ���ı��/���˾���
	uint16_t BackCount;                       // ���˼���
	uint8_t  SwState;                          // ����״̬
	uint16_t StepCount;                     // ��������
	W_D Walk;                               // ʵ�����
};
//-----------------------------------------------
// һ����ͳ�����ݽṹ
//-----------------------------------------------
struct TSignSta
{
	uint32_t SignCount;                          // �������
	uint32_t SignTypeCount[SIGN_NUM];                  // �������ͱ����Ŀ
	TSignData SDB[MAX_SIGNDATA_NUM];        // ��ϸ�������
	std::string SN[SIGN_NUM];                      // �����������
};





// B�����������λ
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
// ������Ϣ���ݽṹ
//-----------------------------------------------
struct TAlarmData
{
	uint32_t Number;                          // �������
	bool     Enbale;                          // ����ʹ�ܱ�־
	uint32_t FilterNum;                       // �������

	uint32_t MeterIndex;                      // �������ڵ��׿�������
	uint16_t StepIndex;                       // �������ڵĲ��������ţ����׿��ڵ����λ�ã�
	uint16_t VerPos;                          // �������ڵ���ȵ�λ�ã��������£���0��ʼ��

	//uint32_tMeterIndexEnd;                  // �������������ڵ��׿�������
	//uint16_t StepIndexEnd;                  // �������������ڵĲ��������ţ����׿��ڵ����λ�ã�

	uint16_t PotCount;                        // �����
	uint16_t StepCount;                       // ������ˮƽ���ȣ�������
	uint16_t VerStart;                        // ������ֱ������㣨B���㣩
	uint16_t VerEnd;                          // ������ֱ�����յ㣨B���㣩

	uint8_t alarm_chn;                        // ����ͨ��
	uint8_t alarm_type;                       // ��������

	W_D Walk;                                 // ����ʵ�����
	uint16_t Speed;                           // ����ʵ���ٶ�
	uint8_t  InRailGap;                       // �ڹ�������ڣ�0 - ���ڣ�1 - ��
	uint8_t  Check;                           // ���˱�ǣ�0-δ���ˣ�1-�����ˣ�2-����

	bool HasCorrDefect;                       // ��Ӧ�����־
	uint32_t CorrDefectNum;                   // ��Ӧ�������
};

//-----------------------------------------------
// ����ͳ���б�������ݽṹ
//-----------------------------------------------
struct TAlarmSta
{
	TAlarmSta();

	void Reset();

	uint8_t AlarmConfirmInterval;                           // ����ȷ�ϵĲ��������
	uint8_t  alarm_type_bak[CH_N - 2];                      // ÿ��ͨ���������͵ı��ݣ�CH_N-2��ʾ����Gͨ����deΪһ��ͨ������ͬ
	uint8_t AlarmInterval[CH_N - 2][4];                     // ÿ�ֱ����������
	uint32_t AlarmBitMark[CH_N - 2][4];                     // ��ͨ����ͬ���͵ı�־λ

	uint16_t CheckAlarmCount;                               // ���ڼ����ı�����Ŀ
	int32_t  CheckAlarmNum[1000];                            // ���ڼ����ı�������������
	uint8_t  CheckAlarmInv[1000];                             // ���ڼ����ı������

	uint32_t ChannelAlarmCount[CH_N - 2][4];                // ����ͨ����ͬ��������Ŀ
	uint32_t AlarmDataCount;                                // ��������
	TAlarmData ADB[MAX_ALARMDATA_NUM];						// ��ϸ��������

	uint32_t FilterChannelAlarmCount[CH_N - 2][4];          // ���˺����ͨ����ͬ��������Ŀ
	uint32_t FilterAlarmCount;                              // ���˺�ı�����Ŀ
	uint32_t FilterAlarmNum[MAX_ALARMDATA_NUM];             // ���˺�ı�����������

	uint32_t SearchAlarmCount;                              // �������ı�������
	uint32_t SearchAlarmNum[SEARCH_ALARMDATA_NUM];          // ����������������

	//string AC[CH_N - 2];									// ����ͨ��������
	//string AT[4];											// ��������������
};

//-----------------------------------------------
// ������Ϣ���ݽṹ
//-----------------------------------------------
struct TDefectData
{
	bool Enbale;                            // ����ʹ�ܱ�־
	bool Checked;                           // �鿴��־

	uint32_t Number;                          // �������
	uint32_t FilterNum;                       // �������

	uint32_t MeterIndex;                      // �������ڵ��׿�������
	uint16_t StepIndex;                       // �������ڵĲ��������ţ����׿��ڵ����λ�ã�
	uint16_t VerPos;                          // �������ڵ���ȵ�λ�ã��������£���0��ʼ��

	//uint32_tMeterIndexEnd;                 // �������������ڵ��׿�������
	//uint16_t StepIndexEnd;                  // �������������ڵĲ��������ţ����׿��ڵ����λ�ã�

	uint16_t StepCount;                     // �����ˮƽ���ȣ�������
	uint16_t VerStart;                      // ����ֱ������㣨B���㣩
	uint16_t VerEnd;                        // ����ֱ�����յ㣨B���㣩

	uint8_t  DefectType;                    // �������ͣ�1 - �˹���2 - �طţ�3 - �Զ�
	uint8_t  DefectDegree;                  // ����̶ȣ����Զ���1 - һ����2 - ������3 - �����������أ�
	//           ��������1 - �������ˣ�2 - ���ˣ�3 - ���ˣ�4 - �۶�
	uint32_t	SectionNum;		// ��λ��� 5��BCD
	uint16_t	LineNum;		// �߱��4��BCD
	uint8_t	LineWay;		// �б� 0�����ߡ�1���ϣ�2������
	uint8_t	LineType;		// �߱� 0: վ��, 1: ����
	uint8_t	LeftRight;		// ���ҹ� 0: �� 1����
	uint16_t	swNum;			// ����ţ�4��BCD��ʾ��һ��3λ
	W_D     Walk;           // ʵ�����
	uint16_t	TrackNum;		// �ɵ���ţ�վ�ߣ�4��BCD
	uint32_t	StationNum;		// ��վ��ţ�վ�ߣ�5��BCD
	uint32_t FindData;       // �������ڣ��꣨4λ�����£�2λ�����գ�2λ��
	uint32_t	FindTime;		// ����ʱ�䣺ʱ��2λ�����֣�2λ�����루2λ��
	uint16_t	FindUser;		// �����˹���,4��BCD
	uint16_t	DefectNum;		// ������4��BCD
	char    DeviceNum[8];   // �������
	uint16_t	RailNum;		// �ֹ���,4��BCD
	int8_t    RailType;
	uint8_t   Dispose;        // �ֳ�����0δ����1����ϼа壬2�İ��ϼУ�3ԭλ����

	uint32_t InputData;      // �������ڣ��꣨4λ�����£�2λ�����գ�2λ��
	uint32_t	InputTime;		// ����ʱ�䣺ʱ��2λ�����֣�λ�����루λ��
	uint16_t	InputUser;		// �����˹���,4��BCD
};
//-----------------------------------------------
// ����ͳ���б�������ݽṹ
//-----------------------------------------------
struct TDefectSta
{
	TDefectSta();

	void Reset();
	TDefectData DefectDataBak;                              // �������ݱ��ݣ����ڼ�����

	uint16_t CheckDefectCount;                                // ���ڼ�����������Ŀ
	uint32_t CheckDefectNum[1000];                             // ���ڼ�������������������
	uint8_t CheckDefectInv[1000];                              // ���ڼ�����������

	uint32_t TypeDefectCount[3][4];                           // �����͵���Ŀ��[����][�̶�]
	uint32_t DefectDataCount;                                 // ��������
	TDefectData DDB[MAX_DEFECTDATA_NUM];                    // ��ϸ��������

	uint32_t SourceTypeDefectCount[3][4];                     // Դ�����͵���Ŀ��[����][�̶�]
	uint32_t SourceDefectDataCount;                           // Դ��������
	TDefectData SDDB[MAX_DEFECTDATA_NUM];                   // Դ��ϸ��������

	uint32_t FilterDefectCount;                               // ���˺�ı�����Ŀ
	uint32_t FilterDefectNum[MAX_DEFECTDATA_NUM];             // ���˺�ı�����������

	uint32_t SearchDefectCount;                               // ����������������
	uint32_t SearchDefectNum[MAX_DEFECTDATA_NUM];             // ����������������

	std::string DT[3];											// ����������������
	std::string DD[3];											// ����̶���������
};


//-----------------------------------------------
// �����б����ݽṹ
//-----------------------------------------------
struct TCheckbackData
{
	UINT32 Index;                               // ��ţ���0��ʼ��

	UINT32 MeterIndex_S;                        // ������ڵ��׿�������
	UINT16 StepIndex_S;                         // ������ڵĲ��������ţ����׿��ڵ����λ�ã�

	UINT32 ListIndex;                           // ����б��Ӧ��ţ���0��ʼ��
	UINT16 CheckCount;                          // �����ڻ��˼�������1��ʼ��

	UINT32 MeterIndex_E;                        // �յ����ڵ��׿�������
	UINT16 StepIndex_E;                         // �յ����ڵĲ��������ţ����׿��ڵ����λ�ã�

	UINT16 StepCount;                           // ��������
};
//-----------------------------------------------
// ����ͳ�����ݽṹ
//-----------------------------------------------
struct TCheckbackSta
{
	UINT32 DataCount;                               // ���˼�¼����
	UINT32 OrgDataCount;                            // �����˼�¼����


	TCheckbackData Org_CD_List[MAX_SIGNDATA_NUM];     // �����˼�¼�б�
	TCheckbackData CD_List[MAX_SIGNDATA_NUM];         // ���˼�¼�б�
};

const float HOR_POINT_FOR_LENTH = 2.7f;
const float VER_POINT_FOR_LENTH = 3.0f;


#define MAX_AL_DE_STEP_COUNT    300         // ��󱨾������𲽽����



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


//TAlarmSta AlarmSta;             // ���������Ϣ
//TDefectSta DefectSta;           // ���������Ϣ

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//���ƣ�CheckDefectDelete
//���ܣ�ɾ�������¼
//���룺num - ������Ϣ���
//�����
//���أ�
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void CheckDefectDelete(TDefectSta *tds, UINT32 num);

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
void CheckDefectMerge(TDefectSta *tds, UINT32 num_m, UINT32 num_s);


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
void CheckDefectUpdata(TDefectData *tdd, UINT16 StepInv, UINT16 VerPos, UINT8 DefectType, UINT8 DefectDegree, UINT16 DefectNum, UINT8 Dispose);


//---------------------------------------------------------------------------
// V2.0���ݰ汾�����һ�������¼�������б�
//---------------------------------------------------------------------------
void CheckDefectAdd(F_HEAD FileHead, BLOCK MeterHead, UINT32 MeterIndex, UINT16 StepIndex, UINT16 VerPos, UINT8 DefectType, UINT8 DefectDegree, UINT16 DefectNum, UINT8 Dispose, TDefectSta *tds);


/*
if (LineMark.Mark & QIAO)    found[0] = true;            // ����
if (LineMark.Mark & CURVE)   found[1] = true;            // ����
if (LineMark.Mark & FORK)    found[2] = true;            // ����
if (LineMark.Mark & SEW2)    found[3] = true;            // �ֶ���־
if (LineMark.Mark & SEW)     found[4] = true;            // ����
found[5] = true;	//����
if (LineMark.Mark & START)   found[6] = true;            // �ϵ�
if (LineMark.Mark & CHECK)   found[7] = true;            // У��
if (LineMark.Mark & SP_P)     found[8] = true;           // ���ٵ�
if (LineMark.Mark & CK_KM)   found[9] = true;            // ���У��
if (LineMark.Couple & BIT15)   found[10] = true;         // ͨ�����
*/
void GetAlarmDefectSignData(void* threadData);