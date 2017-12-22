#ifndef __AKEY_COS_H__
#define __AKEY_COS_H__


namespace AKey
{

//////////////////////////////////////////////////////////////////////////////////////////////////
// COS
#pragma pack(4)
//此结构为COS内部结构
typedef struct _TOKEN_COSINFO
{
	UINT16   u16Len;          // 结构长度: 两个字节
	UINT16   u16Version;             // 结构版本: 两个字节

	struct _SYSTEM_VERSION
	{
		BYTE abModel[4];// 硬件型号 19A3,19I3,19V3,19D3,19F3;A131,I131 首位字母小写表示+B
		struct _COS_VERSION
		{
			BYTE btMajor;
			BYTE btMinor;
			BYTE btBuild;
			BYTE btRFU;
		}ver;  // Cos版本号
		struct _COS_KEYSECURE
		{
			BYTE btCfg1; 
			BYTE abRFU[3];
		}serCfg;
		BYTE abDate[4]; // BUILD日期
	}sysVersion;

	BYTE abSerialNum[16];     //  序列号 
	BYTE abChipNum[16];       //  芯片号 
	BYTE abShellNum[16];     //  外壳号 
	struct _SupportAlg
	{
		UINT32 bSupportCbc:  1 ;   // 0 (1Bit) cos 是否内部支持CBC 
		UINT32 bSupporttDirectKey:  1 ;   // 0 cos 是否支持密钥和明文(密文)一起传输
		UINT32 bSupportSCB2: 1 ;   // 0  (1Bit) cos 是否内部支持33 
		UINT32 bSupport33  : 1 ;   // 0  (1Bit) cos 是否内部支持scb2 
		UINT32 bSupportRsaBit: 4;   // 4 (4Bit ) Rsa bit ,以256bit为基数
		UINT32 bSupportMD5 : 1;  // 1  是否支持MD5
		UINT32 bSupportSHA1  : 1;  // 1  是否支持SHA-1
		UINT32 bSupportHiPRand :1;     // 是否支持Hip输入扰乱
		UINT32 u32Reserver      : 21;  // 0  保留 21 bit
	}supportAlg;
	UINT32 u32Space;                 // 剩余空间
	BYTE btApduCfg;			// b7-是否支持扩展APDU, b0-6: apdu 缓冲区大小，以128字节为倍数，最大为128*128=16k
	BYTE btMiscCfg;			// 如果Bit_0=1,初始化时，根据Pin的模式传入Pin，否则按照明文传入Pin； bit_1==1表示加密通讯
	BYTE abRFU[2];

	// 以上结构及类型定义为Cos原来已有的，所以结构 DeviceInfoStruct 中的部分成员以及类型做了部分改动。
	//结构 SysCfg4LHWStruct 是为硬件提供的信息，应用层可以不关心。
	struct _CONGIG4_LHW
	{
		//b7: 通讯模式，1: Hid方式；0: BOT方式
		//b6:工作模式，1: 生产模式; 0: 用户模式
		//b5:字库有效模式，1:字库有效；0:字库无效
		//b4:字库模式，1:GBK18030;   0: GBK字库
		//b3~b0: RFU
		BYTE btHardwareMode;   
		BYTE btIsoWriteMode;     //b7: 1: 允许写；0: 禁止写入
		//b6~b0: RFU
		UINT16 u16IsoCRC;
		UINT32 u32IsoLen;
	}cfg4LHW;

	struct _GLOBAL_STATUS
	{
		BYTE btFlagChangePhase; //Added by zyt
		BYTE btLifeCycle;				//生命周期状态字节[注意：因为需要最后编程，所以此变量必须放在最后]
		BYTE btCritSignWaitLimit;		// 上电后 （CritSignWaitLimit×0.1)秒内，不准做关键交易签名，防止瞬间攻击 
		BYTE btMiscCfgRfu :6;
		BYTE bMiscCfgBtnFunctoion :1;   // 0x40 按钮功能启动，否则作为A来使用，不查按钮状态
		BYTE bMiscCfgBtnHwSupport :1;   // 0x80 硬件是否带有按钮支持
	}globalStatus;
}TOKEN_COSINFO, *TOKEN_COSINFO_PTR;
#pragma pack()
#define TOKEN_MAX_COSINFO_SIZE 0x80


//////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(1)
// 密码属性
typedef struct _TOKEN_PINATTR
{
	BYTE btMaxLen;
	BYTE btMaxTime;
	BYTE btMinTime;
	BYTE btMode; // 0-明文，1-DES，2-HIP；99-XOR
}TOKEN_PINATTR, *TOKEN_PINATTR_PTR;


// fileid= 0xA310，AC：只读
typedef struct _TOKEN_APPINFO_R
{
	UINT16 		u16Len;							//该结构大小
	UINT16		u16Version;						//结构版本，两个字节
	BYTE		abCustomID[4];					//客户号
	BYTE		btMaxKeyPairNum;				//客户可用的最大密钥对的个数
	BYTE		btManagerKeyPairPos;			//管理密钥对开始编号

	struct _Config
	{
		//PIN
		UINT32		bSecurityAlwaysNeedPin: 1;		//每次私钥操作总是需要验证口令
		UINT32		bSupportHipInputPin:1;			//支持HIP输入口令
		UINT32		bSupportKeyBoardInputPin:1;		//支持键盘输入口令
		//按钮
		UINT32		bSecurityAlwaysNeedBotton:1;	//每次私钥操作都需要按钮，生成密钥对除外
		UINT32		bMd5CommonSignClosed:	 1;		//关闭md5普通签名
		UINT32		bSHA1CommonSignClosed:	 1;		//关闭sha1普通签名
		UINT32		bMD5SHA1CommonSignClosed: 1;		//关闭md5sha1普通签名
		//导入密钥对
		UINT32		bSupportPriKeyImport:1;			//是否支持私钥导入
		//解密
		UINT32		bSupportPriKeyDecrypt:1;		//是否支持私钥解密
		//签名
		UINT32		bNeedCrucialSign:1;				//是否允许关键签名
		UINT32		bNeedSignedData:1;				//是否允许传入原始数据 sign
		//应用初始化保护模式
		UINT32		bAppFormatSafeMode:1;			//0 自由，1 key
		UINT32		bAppInitACKeyId:1;				//初始化保护密钥ID
		//密码是否要做MD5
		UINT32		bSupportUserPinMD5:1;
		UINT32		bSupportSoPinMD5:1;
		UINT32		u32SupportReserver1 : 17;
	}config;
	// 
	BYTE		btDevFlags;
	BYTE		abReserver[3];
}TOKEN_APPINFO_R, *TOKEN_APPINFO_R_PTR;
#define TOKEN_MAX_APPINFO_R_SIZE 0x20

//// fileid = 0xA311 , AC ：读写
//typedef struct _TOKEN_APPINFO_RW
//{
//	UINT16   u16Len;
//	UINT16   u16CheckSum;							// 校验位
//
//	TOKEN_P11INTO tokenP11Info;
//
//	// 自定义
//	UINT32	u32Timeout;							//以秒为单位
//	struct _Config
//	{	
//		UINT32	bSecurityAlwaysNeedPin:1;			//应用层每次私钥操作总是需要验证口令。
//		UINT32	btInputPinMode:3;					//密码输入模式	0-键盘，1-HIP		
//		UINT32	u32SecurityReserver1 : 28;
//	}config;
//
//	UINT32	u32MaxKeyPairNum;					//最大密钥对的个数（证书）
//	UINT32	u32Reserver1;						//
//	UINT32	u32Reserver2;						//
//}TOKEN_APPINFO_RW, *TOKEN_APPINFO_RW_PTR;
#define TOKEN_MAX_APPINFO_RW_SIZE 0xE0

// 单对象信息
typedef struct _TOKEN_OBJECT_INFO
{
	BYTE btFlags;
	BYTE btFirstBlockId;
	UINT16 u16Length;
}TOKEN_OBJECT_INFO, *TOKEN_OBJECT_INFO_PTR;

// fileid = 0xA312 , AC ：读写
// 多对象头描述
#define TOKEN_MAX_OBJECT_NUM				46		// 支持最大对象个数
#define TOKEN_FLAG_OJB_PRI					0x01	// 对象数据受PIN保护	ONKEY_FLAG_PRIVATE
#define TOKEN_FLAG_OJB_PUB					0x02	// 对象数据读写自由		ONKEY_FLAG_PUBLIC
#define TOKEN_FLAG_OJB_P11_CERT				0x80	// 标记P11证书对象（有MY_KEYID属性（0x80000002））
#define TOKEN_OBJECT_BLOCK_LENGTH(pHeader) (((pHeader)->blockAttr.u16BlockSizeType == 1)? 255 : 127)
#define TOKEN_OBJECTS_OFFSET_SIZE(idx)	(44 + idx * sizeof(TOKEN_OBJECT_INFO))
typedef struct _TOKEN_OBJECTS_HEADER // size=8+32++4+4*46 = 228
{
	UINT16	u16InitFlag;			// 其值为0x4D5D,0x4C5D，头信息已初始化；每块大小=128Bytes，私有块的起始位置=24
	UINT16	u16KeyUsed;				// 卡片密钥对的使用情况
	UINT16	u16KeyImport;
	UINT16	u16CurrObjIdx;
	BYTE	abBlockUsed[32];		// 块的使用情况，8*32 * 128 = 32K
	struct _BLOCK_ATTR
	{
		UINT16 u16BlockSizeType : 2;// 1-每块大小=256字节, 0-每块大小=128字节
		UINT16 u16PriBlockStartID : 6;// 私有块的起始位置
		UINT16 u16Reserver : 8;  // 0  保留 8 bit
	}blockAttr;
	UINT16	u16Reserve1;				// 保留
	TOKEN_OBJECT_INFO objects[TOKEN_MAX_OBJECT_NUM];
}TOKEN_OBJECTS_HEADER, *TOKEN_OBJECTS_HEADER_PTR;

#pragma pack()

};

#endif 
