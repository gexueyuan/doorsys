//
//  IDCardConfig.h
//  TDRToken
//
//  Created by wz on 15/9/24.
//  Copyright © 2015年 lujun. All rights reserved.
//

#ifndef IDCardConfig_h
#define IDCardConfig_h

#include <limits.h>

enum IDReaderCacheTagGroup
{
    TAG_GROUP_SAM_CERT_VERSION = 1,
    TAG_GROUP_SAM_CERT         ,
};

typedef enum IDREADER_PARAM
{
    IDCARD_PARAM_READCERT  =0, //读取读卡器证书
    IDCARD_PARAM_COSVERSION=1, //读取读卡器的cos信息
    IDCARD_PARAM_COSSPEED  =2, //cos ready
}IDREADER_PARAM;

typedef enum IDREADER_STEP
{
    STEP_BUILD_SAFECHANNEL = 0x00,      //建立安全通道
    STEP_AUTHENTICATION    = 0x01,      //认证
    STEP_READ_IDINFO       = 0x02,      //读取身份证信息
	STEP_READ_IMAGE		   = 0x03,      //读取图片
}IDREADER_STEP; //与sockCfg[4]对应

typedef enum IDCARD_NET_TYPE
{
	NET_TYPE_DEFAULT       = 0x00,      //默认的通信方式
	NET_TYPE_TCP		   = 0x01,		//TCP通信
	NET_TYPE_UDP		   = 0x02,      //UDP通信
	NET_TYPE_TCP_UDP       = 0x03,		//TCP,UDP兼容模式
}IDCARD_NET_TYPE;

static const IDCARD_NET_TYPE g_NetType[4] = {
	NET_TYPE_TCP,			//建立安全通道
	NET_TYPE_TCP,			//认证
	NET_TYPE_TCP,			//读取身份证信息
	NET_TYPE_TCP,			//读取图片
};

typedef enum IDREADER_PROCESS_STATUS
{
    PROGRESS_BUILD_SAFECHANNEL1      =0x01,//第一次建立安全通道
    PROGRESS_BUILD_SAFECHANNEL2      =0x02,//第二次建立安全通道
    PROGRESS_CMD_EXCHANGE_BEGIN      =0x03,//指令交互开始
    PROGRESS_CMD_EXCHANGE_SINGLE     =0x04,//单指令交互
    PROGRESS_CMD_EXCHANGE_MUL        =0x05,//多指令交互
    PROGRESS_CMD_EXCHANGE_END        =0x06,//指令交互结束
    PROGRESS_REQUEST_IMAGE           =0x07,//请求图片
    PROGRESS_VERSION_MANAGER         =0x08,//版本管理
    PROGRESS_GET_PORT                =0x09,//申请端口
    PROGRESS_RELEASE_PORT            =0x0A,//释放端口
    PROGRESS_CMD_ERROR               =0xEE,//错误
}IDREADER_PROCESS_STATUS;

typedef enum IDCARD_WORD_MODE
{
    MODE_NET_REQ_PORT     =     0x01, //支持申请端口模式
    MODE_NET_NO_REQ_PORT  =     0x02, //不支持申请端口模式
	MODE_NET_GATEWAY	  =     0x03, //网关模式
}IDCARD_WORD_MODE;

typedef enum IDCARD_SERVER_TYPE
{
	SERVER_TYPE_REQUEST		= 0x01,//申请端口服务器(java)
	SERVER_TYPE_SAM			= 0x02,//网卡模组服务器
}IDCARD_SERVER_TYPE;

typedef struct IDCARD_PACKAGE_HEADER
{
	BYTE version[2]; //固定F516,可区分旧版本
	BYTE ChainNum[16];//唯一链号
	BYTE samIndex;//SAM索引号
	BYTE retryTime;//重发次数
	BYTE errCode;//错误码
	BYTE bRFU;//保留
	BYTE addr1;//PCB
	BYTE addr2;//包号，从1计数
}IDCARD_PACKAGE_HEADER,*PTR_IDCARD_PACKAGE_HEADER;



#define COMM_PACKAGE_VERSION   0x02
#define INNERT_VERSION         "1.7.0"  //与内部文档保持一致

#define IDCARD_NET_DATA_VERSION	    "v1.0.0.1"
#define IDCARD_TIME_DATA_VERSION	"v1.0.0.1"

#define IDCARD_SDK_VERSION			"1.0.3.12"

#define IDCARD_PACKAGE_HEADER_VERSION	"\xF5\x16"

#define IDCARD_DEFAULT_ENC_KEY				"\xD2\x7B\x35\x62\x1C\x12\x08\x35\x93\x97\xD2\xAD\xFE\x9D\x67\xA8"

#define IDCARD_SELECT_IDCARD_AID			     "\x00\xA4\x04\x00\x0B\x74\x64\x72\x6F\x6E\x99\x05\x69\x64\x52\x64"

#define IDCARD_SELECT_GOFUN_AID			     "\x00\xA4\x04\x00\x0B\x74\x64\x72\x6F\x6E\x99\x07\x69\x64\x52\x64"

#define LOG_BUFFERLEN       4*1024
#define DEFAULT_BUFFERLEN    4*1024
#define USERDATA_SEND_BLOCK  8*1024

#define IDCARD_SAM_VER_LEN	12

#define IDWORK_TIMEOUT		60*1000

#define IDWORK_SAM_WAITTIME   500//单位ms


//读取到身份证文本信息的回调
typedef void (*fnreadIDInfo)(LPBYTE pbIDInfo,UINT32 u32IDInfoLen);
//读取到身份证图片的回调
typedef void (*fnreadIDImage)(LPBYTE pbIDImage,UINT32 u32IDImageLen,LPBYTE pbRefNum,UINT32 u32RefNumLen,LPBYTE pbTimeBuf,UINT32 u32TimeLen);
//读取到读卡器序列号和cos版本号的回调
typedef void (*fnreadParamInfo)(LPBYTE pbSN,UINT32 u32SNLen,LPBYTE pbCosVer,UINT32 u32CosVer);
//08,09,0A,0B指令的回调
typedef void (*fnreadCmdInfo)(BYTE bCmd);
//身份证可以离场的回调
typedef void (*fnreadCardFinish)();
//身份证读取过程中出现错误的回调
typedef void (*fnreadCardError)(HRESULT errorCode,LPBYTE pbRefNum,UINT32 u32RefNumLen);
typedef void (*fnNetRetryTime)(IDCARD_NET_TYPE netType,UINT32 u32RetryTime);


//等待sam的排队人数的回调
typedef void (*fnWaitSamNum)(UINT32 u32SamNum);

//错误日志是通过http上传
//正确的日志有底层上传
typedef struct TDR_PARAM_Context
{
    char IpAddr[20];					//SAM服务器ip地址
    int  Port;							//SAM服务器端口号
    char businessID[16];			    //商户号
	int  nBusinessLen;
	BYTE sockCfg[4];					//socket的配置：[0:默认 1:TCP 2:UDP] 默认配置为[1,1,1,1]
#ifdef WIN32
    char errFile[MAX_PATH];             //错误日志文件路径
#else
    char errFile[NAME_MAX];             //错误日志文件路径
#endif
    BYTE   bRetryFlag;                  //重试开关:0(关闭) 1:打开
    BYTE   bWorkMode;                   //工作模式:(IDCARD_WORD_MODE)
    BYTE   bDeviceType;                 //设备类型(0:未设置 1:蓝牙 2:OTG 3:PC)
	BYTE   bBindMode;					//绑定模式：（如果是绑定模式，07指令后执行F0F4812200，解绑F0F4812100）
	int    nBusNoTimeOut;				//业务超时（nBusNoTimeOut：0 默认是60秒超时 nBusNoTimeOut：-1 不超时）,单位秒
	UINT32 u32PerPkgTimeOut;			//通信每一包的超时时间，单位毫秒，默认是6000毫秒
	UINT32 u32NetRetry;                 //总网络重试次数
	UINT32 u32ReaderRetry;              //总读卡器重试次数
	UINT32 u32ConnTime;				    //连接服务器超时时间(单位为毫秒，TCP默认为2秒)
	UINT32 u32TCPSRTime;				//TCP发送和接收数据的超时时间（单位为毫秒，TCP默认为2秒）
	UINT32 u32UDPSRTime;				//UDP发送和接收数据的超时时间（单位为毫秒，UDP默认为50毫秒）
}TDR_PARAM_Context;

typedef struct IDCARD_CORE_CONTEXT
{
    IDCARD_WORD_MODE bWorkMode;
	UINT32 u32ConnTime;				    //连接TCP服务器超时时间(默认为2秒)
	UINT32 u32TCPSRTime;				//与TCP服务器发送和接收数据的超时时间（默认为2秒）
	UINT32 u32UDPSRTime;				//与UDP服务器发送和接收数据的超时时间（默认为50毫秒）
}IDCARD_CORE_CONTEXT;

typedef struct TDR_CALLBACK_CONTEXT
{
    fnreadCardFinish finishCb; //读取成功，用户可以拿走身份的回调通知
    fnreadIDInfo   IDInfoCb;   //读取身份证文本信息的回调
    fnreadIDImage  IDImageCb;  //读取身份证图片的回调
    fnreadCmdInfo  cmdCb;      //指令数据的回调
    fnreadCardError errCb;     //读取身份证错误的回调
	fnNetRetryTime  netRetryCb;//网络重试次数的回调
    fnWaitSamNum    waitSamCb; //等待Sam的排队人数的回调
}TDR_CALLBACK_CONTEXT;

typedef struct TDR_SSL_Context
{
    BYTE abSessionID[16]; //16字节的会话ID
    BYTE abSKey[16];      //16字节的SKey
    BYTE abIV[16];        //16字节的IV
}TDR_SSL_Context;

// 令牌信息
typedef union _TOKEN_BITMAP
{
    struct _VALUE
    {
        UINT32 v1;
        UINT32 v2;
    }vals;
    struct _BITMAP
    {
        UINT32 bBitmap64 : 1; // bit0
        UINT32 bShellNum : 1;
        UINT32 bCustomId : 1;
        UINT32 bUpdateTickCount : 1;
        UINT32 bPin1Times : 1;
        UINT32 bPin2Times : 1;
        UINT32 bLangIdCharset : 1;
        UINT32 bFunctionList1 : 1; // bit7
        UINT32 bRFU1 : 1; // bit8
        UINT32 bRFU2 : 1;
        UINT32 bDeviceRand : 1;
        UINT32 bPin1MaxTimes : 1;
        UINT32 bPin2MaxTimes : 1;
        UINT32 bBluetoothAddress: 1;
        UINT32 bReadyStatus:1;//bit 22
    }bits;
}TOKEN_BITMAP, *TOKEN_BITMAP_TPR;

typedef struct _TOKEN_STATUS
{
    UINT32 btTokenId: 4;				// 当前令牌索引（最多支持15个令牌）
    UINT32 bMustSession : 1;			// 执行任何指令前必须先建立通道(INIT)
    UINT32 bSupportEncryptApdu : 1;		// 支持加密指令(INIT)
    UINT32 bMustEncryptApdu: 1;			// 必须用加密指令(INIT)
    UINT32 bForceNotLoadPairKey1B: 1;	// 不加载1B号密钥，如果更改该配置需要删除缓存文件才作用(INIT)
    UINT32 bForceNotCheckDevice: 1;		// 不检测设备(INIT)
    UINT32 bDeviceChecked : 1;			// 是否检测过设备
    UINT32 bExistTokenCache: 1;			// 是否存在TOKEN缓存
    UINT32 bVerifyTokenCache: 1;		// 是否验证TOKEN缓存
    UINT32 un32RFU2: 20;
}TOKEN_STATUS, *TOKEN_STATUS_PTR;

typedef struct _TOKEN_INFO
{
    TOKEN_BITMAP tokenBitmap;
    UINT32 bSupportQuickReadCert: 1;	// 支持快速读证书（存储）
    UINT32 bObjectDataValid : 1;		// 是否读取对象数据（存储）
    UINT32 bLoadedSignPacketSizeList : 1;	// 临时
    UINT32 un32RFU1 : 29;
    
    BYTE abFindCardTime[4];            //[bit:24] B0B1：无卡寻卡间隔时间，单位：ms B2B3：有卡寻卡间隔时间，单位：ms
    BYTE abBatteryHealth[29];		   //[bit:25] 电池健康信息
    
    BYTE abRFU[100];						// 保留（对齐）

}TOKEN_INFO, *TOKEN_INFO_PTR;



#endif /* IDCardConfig_h */
