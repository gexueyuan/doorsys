#ifndef _LIB_BASE64_H_
#define _LIB_BASE64_H_

#define HTTPS_PRE          "https://"

#define COS_TRAN_CODE_01   0x01   //升级查询接口
#define COS_TRAN_CODE_02   0x02   //获取授权签名接口
#define COS_TRAN_CODE_03   0x03   //下载升级包
#define COS_TRAN_CODE_04   0x04   //升级成功/失败反馈接口

#define COS_TAG_57         0x57
#define COS_TAG_59         0x59

typedef struct _cos_request_info{
	char cosVersion[100];   //COS版本
	char updateCert[4096];  //升级证书
	char cosInfo[1024];     //COS信息(版本)
	char randomNo[100];     //随机数
	char read_sn[100];      //读卡器序号
	char isSuccess[2];         //升级成功标志:0-成功,1-失败
}cos_request_info;

typedef struct _cos_response_info{
	char isUpdate[2];          //是否升级:0-不需要,1-需要
	char cosVersion[100];   //Cos版本对象
	char flag[2];              //是否强制升级:0-非强制,1-强制
	char version[100];      //最新Cos版本
	int size;               //版本大小
	char signature[4096];   //签名包
	char authResult[4096];  //授权结果
	char cosPacket[51200];  //cos升级包内容
	char resultCode[100];   //返回码
}cos_response_info;

int cjson_request(int tran_code, cos_request_info *p_cos_req_info, char *recv_buf, int *recv_len);

#endif
