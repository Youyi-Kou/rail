#ifndef DATADEFINE_H
#define DATADEFINE_H

#include <map>
#include <vector>
#include <string>

//常量定义
#define 	BIT0 		0x00000001
#define 	BIT1 		0x00000002
#define 	BIT2 		0x00000004
#define 	BIT3 		0x00000008
#define 	BIT4 		0x00000010
#define 	BIT5 		0x00000020
#define 	BIT6 		0x00000040
#define 	BIT7 		0x00000080
#define 	BIT8 		0x00000100
#define 	BIT9 		0x00000200
#define 	BIT10 		0x00000400
#define 	BIT11 		0x00000800
#define 	BIT12 		0x00001000
#define 	BIT13 		0x00002000
#define 	BIT14 		0x00004000
#define 	BIT15 		0x00008000
#define 	BIT16 		0x00010000
#define 	BIT17 		0x00020000
#define 	BIT18 		0x00040000
#define 	BIT19 		0x00080000
#define 	BIT20 		0x00100000
#define 	BIT21 		0x00200000
#define 	BIT22 		0x00400000
#define 	BIT23 		0x00800000
#define 	BIT24 		0x01000000
#define 	BIT25 		0x02000000
#define 	BIT26 		0x04000000
#define 	BIT27 		0x08000000
#define 	BIT28 		0x10000000
#define 	BIT29 		0x20000000
#define 	BIT30 		0x40000000
#define 	BIT31 		0x80000000

//B超通道定义
#define CH_A1	0
#define CH_A2	1
#define CH_a1	2
#define CH_a2	3
#define CH_B1	4
#define CH_B2	5
#define CH_b1	6
#define CH_b2	7
#define CH_C	8
#define CH_c	9
#define CH_D	10
#define CH_d	11
#define CH_E	12
#define CH_e	13
#define CH_F	14
#define CH_G	15

//F失波
#define CH_FL	16
//G失波
#define CH_GL	17

//A中通道定义
#define ACH_A	0
#define ACH_a	1
#define ACH_B	2
#define ACH_b	3
#define ACH_C	4
#define ACH_D	5
#define ACH_d	6
#define ACH_F	7
#define ACH_c	8
#define ACH_e	9
#define ACH_E	10
#define ACH_G	11

typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef short               int16_t;
typedef unsigned short      uint16_t;
typedef int                 int32_t;
typedef unsigned int        uint32_t;
typedef long long           int64_t;
typedef unsigned long long  uint64_t;

typedef std::vector<int>	VINT;
	
//大门数
#define    	GA_NUM   	5

//小门数
#define    	DOR_NUM   	7

//A超通道数
#define    	CH_N   		12

//轨型数
#define   	R_N   		4



#define F_RLE		0x8000000000000000

#define ROW			68
#define VALID_ROW	(ROW - 2)

#define COLUMN		1000
#define WAVE_LEN	512					// A超波形数据长度

enum _FILE_ACCESS
{
	WRITE_B_SIZE = 0x1000,
	WRITE_A_SIZE = 0x4000,
	WRITE_A_LEN = 0x2000, // B 超文文件一一次写入入 4K
	// A 超文文件一一次写入入 16K

	ACCESS_SIZE = 0x8000,
	ACCESS_LEN = 0x4000, // 32K 一一次获取字节大大小小

	// 一一次读写的⻓长度(16bit)
	CASH_N = 32, // Achao 共有多少个 cash(意思是)
	CASH_SIZE = (CASH_N*ACCESS_SIZE),//0x00100000 32 * 32 K = 1024 K 1M 空间 指的是 B 超的压缩后整体的最大大数据量量

	CASH_LEN = (CASH_SIZE >> 1),//0x00080000 512K 空间
};



enum COMPRESS_FLAG {
	FLAG_COMPRESS = 0x8000,	// A超每帧压缩包标志，表示接下来的数据是一个压缩包
	F_ZERO = 0x4000,	// A超压缩标志，BIT14+x个(x<=MAX_ZERO)
	ZERO_L = 0x3fff,	//0 的长度
	F_MASK = 0xC000,
	MAX_ZERO = 0x3ff0,	// A超压缩
};

#define FILENAME_LEN 100

#endif // DATADEFINE_H



