///////////////////////// File State /////////////////////////
////    #include "zmqsoc.h"
////    2017/7/12	Created by Hyper @ TendyRon
////
//////////////////////////////////////////////////////////////


#ifndef  _ZMQSOC_H_
#define  _ZMQSOC_H_


////////////////////////////// INCLUDE //////////////////////////////
////
////


////////////////////////////// DEFINE ////////////////////////////
////
////
/* GPBOX_ZMQ_TLV */
//tag
#define ZMQ_TLV_TAG_WORKMODE	(0x0001)	// 工作模式
#define ZMQ_TLV_TAG_IDCHKMODE	(0x0002)	// 身份认证模式
#define ZMQ_TLV_TAG_CTRLCMD		(0x0003)	// 控制指令

//val
#define ZMQ_TLV_VAL_WKMODE_IDLE			(0x0001)	// 休眠待机
#define ZMQ_TLV_VAL_WKMODE_NET			(0x0002)	// 联网模式
#define ZMQ_TLV_VAL_WKMODE_RDCARD		(0x0003)	// 读证模式
#define ZMQ_TLV_VAL_WKMODE_RDPASS		(0x0004)	// 认证通过
#define ZMQ_TLV_VAL_WKMODE_RDFAIL		(0x0005)	// 认证失败
#define ZMQ_TLV_VAL_WKMODE_RUN			(0x0006)	// 驾驶场景
#define ZMQ_TLV_VAL_WKMODE_PAUSE		(0x0007)	// 临时停车

#define ZMQ_TLV_VAL_CTRL_COSUPD			(0x0003)	// cos升级指令
#define ZMQ_TLV_VAL_CTRL_COSDEL			(0x0004)	// 取消cos升级
/***-------------------- MACROS --------------------***/




////////////////////////////// TYPEDEF ////////////////////////////
////
////
/***-------------------- ENUM --------------------***/


/***-------------------- STRUCT --------------------***/




////////////////////////////// VOLATILE VARIABLE ////////////////////////////
////
////




////////////////////////////// Func_declaration //////////////////////////////
////
////




#endif /* _ZMQSOC_H_ */
