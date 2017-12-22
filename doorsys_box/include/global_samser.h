#ifndef _GLOBAL_SAMSER_H_
#define _GLOBAL_SAMSER_H_

#include "global_sys.h"

//全局变量
#define ERR_LOG_LEN       300*1024     //客户端上报的日志大小
#define iMAXRECVBUF   10240
#define MSG_MAXLEN 		iMAXRECVBUF
#define BUF_LEN       iMAXRECVBUF
#define SAM_MAX       16
#define FILE_PATH     256           //文件名类长度
#define IP_LEN        20            //IP长度
#define PORT_LEN      8             //端口长度
#define MAC_LEN       20            //MAC长度
#define VERSION_LEN   20            //版本长度
#define SHELL_NO_LEN  20            //SAM序号长度
#define PASSWD_LEN    40            //密码长度
#define UUID_LEN      16            //UUID长度
#define SAM_PATH      16            //SAM的PATH路径
#define FTP_USER_LEN  16            //FTP用户名长度
#define FTP_PASSWD_LEN 32           //FTP密码
#define FTP_FILE_NAME_LEN 64        //FTP长度
#define TIME_LEN      8             //间隔时间长度
#define DN_MAX_LEN    100           //DN的最大长度
#define CERT_INFO_LEN 2048          //身份证文字信息长度
#define CERT_PIC_LEN  4096          //身份证图片信息长度
#define SAM_NUM_LEN   4             //SAM个数
#define CPU_INFO_LEN  8             //CPU信息长度
#define MEM_INFO_LEN  8             //MEM信息长度
#define FLASH_INFO_LEN 8            //FLASH信息长度
#define READ_SEQ_LEN  20            //Reader序号
#define MOBILE_NO_LEN 64            //手机号长度
#define SAM_SN_LEN    20            //SAM_SN长度
#define CERT_NO_LEN   18            //身份证号长度
#define MSG_BUF_LEN   1024          //MSG长度
#define DN_LEN        32            //DN长度
#define CRC_BUF_LEN   100           //CRC最大长度
#define BUF_TMP_LEN   4096          //BUF临时长度
#define READ_SN_LEN   100           //读卡器序号最大长度
#define CUST_NUM_LEN  100           //客户号长度
#define CKH_LEN       100           //参考号长度
#define MARK_LEN      600           //身份证掩码数据长度
#define CORP_NUM_LEN  100           //商户号长度
#define COS_VER_LEN   30            //COS版本号长度

#define TCP_CLT_TIME_OUT   5       //TCP通信超时时间，单位: 秒
#define REDIS_CLT_TIME_OUT   1       //REDIS超时时间，单位: 秒

#define  iSHORTLEN      2
#define  iACESERLEN     8
#define  iINTLEN        4

#define BUSI_PORT     10040      //管理进程监听端口

//错误码段
#define LOG_ERR_RCVCLT       0xE2100001  //接收客户端报文超时
#define LOG_ERR_CRCCHECK     0xE2100002  //校验CRC失败
#define LOG_ERR_LED          0xE2100003  //点灯失败
#define LOG_ERR_FIND_CARD    0xE2100004  //寻卡失败
#define LOG_ERR_READID       0xE2100005  //认证失败

//管理类错误码
#define LOG_ERR_APPSTART     0xE2000030  //系统已经启动

//系统启用标志
#define SYS_UP_FLAG_YES     1   //应用已启用
#define SYS_UP_FLAG_NO      2   //应用已关闭

//业务状态
#define BUSI_STAT_FREE  0x01   //空闲
#define BUSI_STAT_FIND  0x02   //寻卡
#define BUSI_STAT_READ  0x03   //读证

//读卡器状态
#define READER_STAT_OK   "0"   //正常
#define READER_STAT_FAIL "1"   //异常

//LED灯
#define LED_RED     0x01      //红灯
#define LED_GREEN   0x02      //绿灯

typedef struct
{
	char refnum_buf[100];   //参考号
	int  refnum_len;        //参考号长度
	char readsn_buf[100];   //读卡器序号
	int  readsn_len;        //读卡器序号长度
	char cos_version[100];  //COS版本
	int  cos_len;           //COS版本长度
	int  err_code;          //错误码
	char err_text[100*1024]; //错误日志内容
	int  err_len;            //错误长度
	char tcp_type;           //通讯协议 0x01-tcp
	char time_buf[4096];     //日志时间链
	int  time_len;           //时间链长度
}busi_log;

#endif
