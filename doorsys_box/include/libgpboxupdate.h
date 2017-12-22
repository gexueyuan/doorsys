///////////////////////// File State /////////////////////////
////    #include "libgpboxupdate.h"
////    2017/7/4	Created by Hyper @ TendyRon
////
//////////////////////////////////////////////////////////////


#ifndef  _LIBGPBOXUPDATE_H_
#define  _LIBGPBOXUPDATE_H_


////////////////////////////// INCLUDE //////////////////////////////
////
////




////////////////////////////// DEFINE ////////////////////////////
////
////
#define HTTPS_PRE          "https://"

#define BOX_TAG_55         0x55
#define BOX_TAG_75         0x75

#define BOX_TRAN_CODE_01   0x01   // 注册绑定 
#define BOX_TRAN_CODE_02   0x02   // 升级查询接口
#define BOX_TRAN_CODE_03   0x03   // 下载升级包
#define BOX_TRAN_CODE_04   0x04   // 远程监控
#define BOX_TRAN_CODE_05   0x05   // 日志上送

#define CJSON_REP_SUCCESS		'0'
#define CJSON_REP_FAILURE		'1'

#define UP_CODE_PGM		(0x01)
#define UP_CODE_FRW		(0x02)
#define UP_CODE_CFG		(0x04)

/***-------------------- MACROS --------------------***/




////////////////////////////// TYPEDEF ////////////////////////////
////
////
/***-------------------- ENUM --------------------***/


/***-------------------- STRUCT --------------------***/
typedef struct _box_request_info{
	char sn[32];   		// 车载控制器序列号
	char time[32];  	// 当前时间（Unix时间戳）or 状态时间
	char tboxId[32];	// Tbox设备ID
	char comId[32];		// 4g模块ID
	char cardId[32];	// 读卡器ID
	
	char bindStatus[2];	// 注册绑定状态
	char workMode[2];		// 工作模式
	char verifyMode[2];	// 身份认证模式
	char reportMode[2];	// 主动上报模式
	char sysFault[2];		// 系统故障
	char cardFault[2];		// 读卡器故障
	char netFault[2];		// 网络故障
	char cardStatus[2];	// 读卡器状态
	char netStatus[2];		// 网络状态
	char readStatus[2];	// 读卡状态
	
	char log[64];		// 设备日志
}box_request_info;

typedef struct _cos_up_info{
	char key[32];			// 读卡器类型
	char value[32];      	// 版本号
}cos_up_info;

typedef struct _box_up_info{
	char version[10];		// 版本号
	char url[80];      		// 下载地址
	char checksum[40];      // 校验值 MD5 32B
}box_up_info;

typedef struct _box_response_info{
	char status;			// 注册状态：0-成功
	char description[64];	// 描述：失败、错误
	char sign[64];			// 签名数据
	
	box_up_info program;	// 主程序
	box_up_info firmware;	// 固件
	box_up_info configuration;	// 配置	
}box_response_info;


/* 内部配置文件 */
typedef struct _gpb_conf_info{
	char gopanver[16];		//
	char goreadver[16];
	char tboxver[16];
	char gopansn[16];	// 车载控制器序列号
	char goreadsn[16];	// 读卡器ID
	char tboxsn[16];	// Tbox设备ID
	char netsn[16];		// 4g模块ID
} gpb_conf_info;

/* 内部状态文件 */
typedef struct _gpb_stat_info{
	char bindStatus[2];	// 注册绑定状态
	char workMode[2];		// 工作模式
	char verifyMode[2];	// 身份认证模式
	char reportMode[2];	// 主动上报模式
	char sysFault[2];		// 系统故障
	char cardFault[2];		// 读卡器故障
	char netFault[2];		// 网络故障
	char cardStatus[2];	// 读卡器状态
	char netStatus[2];		// 网络状态
	char readStatus[2];	// 读卡状态
} gpb_stat_info;



////////////////////////////// VOLATILE VARIABLE ////////////////////////////
////
////




////////////////////////////// Func_declaration //////////////////////////////
////
////




#endif /* _LIBGPBOXUPDATE_H_ */