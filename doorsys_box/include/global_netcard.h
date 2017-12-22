#ifndef _GLOBAL_NETCARD_H_
#define _GLOBAL_NETCARD_H_

//log module tag define.BEGIN
#define LOGTAG_MNG      "MNG"
#define LOGTAG_BUSI  "GFREADBUSI"
#define LOGTAG_GFLOG "GFLOG"
#define LOGTAG_GFCOSUPDATE "COSUPDATE"
#define LOGTAG_GPBOXUPDATE "BOXUPDATE"
//log module tag define.END

//netcard server base define.BEGIN
#define NETCARD_CONFIG_FILE   "etc/id_gofun.conf"
#define CARD_STORAGE_TMP_FILE "/tmp/card_storage.txt"
#define CARD_ERR_FILE         "/tmp/idreaderr.log"
#define READER_INFO_CONF_FILE    "etc/reader_info.json"  //读卡器信息存储配置文件
#define HTTPS_CERT_FILE        "cert.pem"         //https客户端使用的证书
#define GPBOX_CTRL_CONF_FILE   "etc/gpbox_ctrl.conf"
#define GOFUN_HOME_DIR        ""         //工作主目录

#define GPBOX_CONF_JSON_FILE   "etc/gpbox_conf.json"   //内部配置文件
#define GPBOX_STAT_JSON_FILE   "etc/gpbox_stat.json"   //内部状态文件
//netcard server base define.END

//JSON名称
#define GOPANSN         "gopansn"    //产品SN （gopan）
#define GOREADVER       "goreadver"  //读卡器版本号
#define GOREADSN        "goreadsn "  //读卡器SN
#define WORKMODE        "workmode"   //当前工作模式（0-6）
#define READSTATUS      "readstatus" //读卡状态 0：空闲待机,1：正在读卡,2：读卡成功,3：读卡失败

#define READSTAT_0      "0"      //读卡状态 0：空闲待机
#define READSTAT_1      "1"      //读卡状态 1：正在读卡
#define READSTAT_2      "2"      //读卡状态 2：读卡成功
#define READSTAT_3      "3"      //读卡状态 3：读卡失败

#define MAX_BUF_LEN    10240

//TRANS_CODE define.begin
#define TRANS_CODE_01      0x0001   //待机模式
#define TRANS_CODE_02      0x0002   //联网模式
#define TRANS_CODE_03      0x0003   //认证模式-在线
#define TRANS_CODE_04      0x0004   //认证模式-离线
#define TRANS_CODE_05      0x0005   //认证通过
#define TRANS_CODE_06      0x0006   //点火模式
#define TRANS_CODE_07      0x0007   //熄火模式
#define TRANS_CODE_08      0x0008   //COS升级
#define TRANS_CODE_09      0x0009   //返回索引
#define TRANS_CODE_0A      0x000A   //返回DN

//TRANS_CODE define.end

//LOG_CODE define.begin
#define LOG_CODE_01        0x0403   //发送错误日志
#define LOG_CODE_02        0X0405   //发送日志时间链
//LOG_CODE define.end

//TLV define.BEGIN
#define TLV_TAG_JYDM   0x0001   //交易码
#define TLV_TAG_PCH    0x0002   //批次号
#define TLV_TAG_CKH    0x0003   //参考号
#define TLV_TAG_DKQXH  0x0004   //读卡器序号
#define TLV_TAG_KHH    0x0005   //客户号
#define TLV_TAG_CWH    0x0006   //错误号
#define TLV_TAG_SXH    0x0007   //流水号
#define TLV_TAG_WKDKH  0x0008   //网卡端口号
#define TLV_TAG_WZBH   0x0009   //位置编号
#define TLV_TAG_KHDXX  0x000A   //客户端选项
#define TLV_TAG_FWDXX  0x000B   //服务端选项
#define TLV_TAG_FBBZ   0x000C   //分包标志
#define TLV_TAG_KHDZS  0x0011   //传输通道客户端证书
#define TLV_TAG_FWDZS  0x0012   //传输通道服务端证书
#define TLV_TAG_DKQZS  0x0013	  //读卡器证书
#define TLV_TAG_KZQZS  0x0014   //控制器证书
#define TLV_TAG_FWDZSBB 0x0015  //服务端证书版本
#define TLV_TAG_FWDSJS 0x0016   //服务端随机数
#define TLV_TAG_MYMW   0x0017   //传输通道秘钥密文
#define TLV_TAG_QMJG   0x0018   //传输通道签名结果
#define TLV_TAG_WKMZBH 0x0019   //网卡模组编号
#define TLV_TAG_COSBB  0x001A   //COS版本号
#define TLV_TAG_WYLH   0x001B   //唯一链号
#define TLV_TAG_CFCS   0x001C   //重发次数
#define TLV_TAG_SBL    0x0081   //设备流数据
#define TLV_TAG_ZPSJ   0x0082   //照片数据
#define TLV_TAG_SJTJ   0x0083   //时间统计
#define TLV_TAG_RZSJ   0x0084   //日志数据
#define TLV_TAG_MW     0x8001   //密文:对明文Tag组加密的结果	

//照片解码tag
#define TLV_TAG_PIC_GCMY1 0x0203 //过程秘钥1
#define TLV_TAG_PIC_ZPMW1 0x0204 //照片密文1
#define TLV_TAG_PIC_ZPMW2 0x0205 //照片密文2
#define TLV_TAG_PIC    0x0207    //企业照片数据
#define TLV_TAG_BUSI   0x0208    //企业基本信息
#define TLV_TAG_PIC_FLAG 0x0209  //是否有照片标志
#define TLV_TAG_TPK5  0x0211     //商户通道-过程密钥密文(保护基本信息)
#define TLV_TAG_TPK2  0x0212     //商户通道-过程密钥密文(保护照片)
#define TLV_TAG_TPK3  0x0213     //商户通道-过程密钥密文(保护指纹)

//时间统计标签
#define TLV_TIME_WKMZ 0x0304     //网卡模组处理时间

//管理应用标签

//日志使用标签
#define TLV_LOG_RZKG     0x0701   //日志开关-0x01-开,0x02-关
#define TLV_LOG_READSN   0x0702   //READ 序号
#define TLV_LOG_SJWYBH   0x0703   //手机唯一编号
#define TLV_LOG_HZXH     0x0704   //盒子序号
#define TLV_LOG_SMSN     0x0705   //SAM_SN
#define TLV_LOG_COSV     0x0708   //COS版本
#define TLV_LOG_HZLX     0x0709   //盒子类型
#define TLV_LOG_CWM      0x0712   //错误码
#define TLV_LOG_CWNR     0x0713   //错误内容
#define TLV_LOG_HZYYBB   0x070A   //盒子应用软件版本号
#define TLV_LOG_WKMZYYSJ 0x071B   //网卡模组应用处理时间
#define TLV_LOG_AQMOSJ   0x071C   //与安全模块通讯时间
#define TLV_LOG_GETPIC   0x071D   //获取照片的时间
#define TLV_LOG_CLT      0x071F   //通讯进程读客户端报文总时间
#define TLV_LOG_SAM      0x071E   //安全模块处理时间
#define TLV_LOG_TCPTYPE  0x0727   //通讯模式

//TLV define.END

//GOPAN_BOX tag
#define TLV_GOPAN_JYDM     0x0001     //交易代码
#define TLV_IDINFO_ENC     0x0002     //身份证密文数据
#define TLV_GOPAN_INDEX    0x0003     //身份证索引
#define TLV_GOPAN_DN       0x0004     //身份证DN


#define READ_INFO_ONLINE   0x0101     //在线读证
#define READ_INFO_OFFLINE  0x0102     //离线读证

//GOPAN_BOX define.END

#define NC_CHAIN_ID_LEN 16
#define NC_IDENTITY_LEN 16
#define NC_PACKET_HEAD_SISE 24 // (((comm_packet_head*)NULL)->app_data - ((comm_packet_head*)NULL)->mark)

typedef struct _comm_packet_head{
	char len_buf[4];
	char mark[2];
	char chain_id[NC_CHAIN_ID_LEN];		//唯一链号
	char sam_index;						//SAM索引
	char retry_times;					//重试次数
	char err_num;
	char rfu;
	char sam_addr1;
	char sam_addr2;
	char app_data[4]; // 0
}comm_packet_head;
typedef struct _comm_msg_head{
	char comm_type;        //通信类型 1-TCP  2-UDP
	char src_type;         //来源标志 1-通信 2-业务
	char end_flag;         //业务进程结束标志,1-结束
	char ruf[1];           //用于对齐（8字节）
	char src_identity[NC_IDENTITY_LEN];  //消息来源id
	int  src_identity_len; //消息来源id长度
	uint64_t time_stamp;   //接收到请求时间戳，单位:ms
	struct _comm_info_st{
		int index;	// tcp(array index)
		int socket_fd; // tcp
		int client_addr_len; // udp
		struct sockaddr_in client_addr;  //udp 客户端IP信息
	}comm_info;
	int pack_len; // NC_PACKET_HEAD_SISE + len(app_data)
	comm_packet_head packet;
}comm_msg_head;  //系统内部,通信报文头


#endif
