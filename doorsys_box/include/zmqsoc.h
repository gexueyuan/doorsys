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
#define ZMQ_TLV_TAG_WORKMODE	(0x0001)	// ����ģʽ
#define ZMQ_TLV_TAG_IDCHKMODE	(0x0002)	// �����֤ģʽ
#define ZMQ_TLV_TAG_CTRLCMD		(0x0003)	// ����ָ��

//val
#define ZMQ_TLV_VAL_WKMODE_IDLE			(0x0001)	// ���ߴ���
#define ZMQ_TLV_VAL_WKMODE_NET			(0x0002)	// ����ģʽ
#define ZMQ_TLV_VAL_WKMODE_RDCARD		(0x0003)	// ��֤ģʽ
#define ZMQ_TLV_VAL_WKMODE_RDPASS		(0x0004)	// ��֤ͨ��
#define ZMQ_TLV_VAL_WKMODE_RDFAIL		(0x0005)	// ��֤ʧ��
#define ZMQ_TLV_VAL_WKMODE_RUN			(0x0006)	// ��ʻ����
#define ZMQ_TLV_VAL_WKMODE_PAUSE		(0x0007)	// ��ʱͣ��

#define ZMQ_TLV_VAL_CTRL_COSUPD			(0x0003)	// cos����ָ��
#define ZMQ_TLV_VAL_CTRL_COSDEL			(0x0004)	// ȡ��cos����
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
