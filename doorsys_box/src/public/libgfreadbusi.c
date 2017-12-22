/**********************************************************************************************
** module name: libmngcenter.c
** module action: for samser's mng service
** author: liuwenjie
** create date:20160907
** modify: 
***********************************************************************************************/
#include "global_sys.h"
#include "global_netcard.h"
#include "global_samser.h"
#include "liblog.h"
#include "libtlv.h"
#include "libzmqtools.h"
#include "libanalysisconf.h"
#include "libutil.h"
#include "libgfreadpub.h"

extern int ig_runmode;

#ifndef WIN32
#define socket_close close
#else
#define socket_close closesocket
#endif

static int busi_parse_packet(char *recv_buf, int recv_len, char *send_buf, int *send_len);
static int busi_read_id(unsigned short tran_code, char *rsp_buf, int *rsp_len);
	
typedef struct _busi_config{
	int tcp_timeout;
	int log_level;
	int find_card_timeout;
	int read_svr_port;
	char soft_version[64];  //软件版本
	//char cos_version[64];   //COS版本
	char read_sn[20];       //读卡器序号
	char busi_server[64];
	char cos_server[64];
	char log_server[64];
	char read_svr_ip[64];
	char gopanbox_server[64];  //GBOX
	//char control_server[64];   //主控
	char idcard_buf[4096];
	int  idcard_len;
	char refnum_buf[100];
	int  refnum_len;
	char time_buf[4096];
	int  time_len;
}busi_config;

static busi_config g_busi_config;
static int exit_flag = 0;
static char busi_stat;   //业务状态0x01-空闲，0x02-寻卡，0x03-读证

static void busi_sig(int SigUsr1)
{
	LOG_PRINT(L_ERR, "busi_sig begin ...");
	exit_flag = 1;
}

static void load_busi_cfg(){
	char scfg_file[128];
	char scfg_desg[64];
	memset(scfg_file,0x00,sizeof(scfg_file));
	sprintf(scfg_file,"%s/%s", GOFUN_HOME_DIR, NETCARD_CONFIG_FILE);

	memset(&g_busi_config, 0, sizeof(g_busi_config));
	strcpy(g_busi_config.log_server, "tcp://");
	strcpy(g_busi_config.busi_server, "tcp://");
	//strcpy(g_busi_config.cos_server, "tcp://");
	strcpy(g_busi_config.gopanbox_server, "tcp://");
	//strcpy(g_busi_config.control_server, "tcp://");
	g_busi_config.tcp_timeout  = conf_getint("GFREAD_INFO", "tcp_timeout", scfg_file);
	g_busi_config.log_level  = conf_getint("GFREAD_INFO", "iLogLevel", scfg_file);
	conf_getstring("GFREAD_INFO", "sLogServer", g_busi_config.log_server+6, sizeof(g_busi_config.log_server)-6, scfg_file);
	conf_getstring("GFREAD_INFO", "sBusiServer", g_busi_config.busi_server+6, sizeof(g_busi_config.busi_server)-6, scfg_file);
	//conf_getstring("GFREAD_INFO", "sCosServer", g_busi_config.cos_server+6, sizeof(g_busi_config.cos_server)-6, scfg_file);
	
	conf_getstring("GFREAD_INFO", "sGopanboxServer", g_busi_config.gopanbox_server+6, sizeof(g_busi_config.gopanbox_server)-6, scfg_file);
	//conf_getstring("GFREAD_INFO", "sControlServer", g_busi_config.control_server+6, sizeof(g_busi_config.control_server)-6, scfg_file);

	conf_getstring("GFREAD_INFO", "read_svr_ip", g_busi_config.read_svr_ip, sizeof(g_busi_config.read_svr_ip)-1, scfg_file);
	g_busi_config.read_svr_port = conf_getint("GFREAD_INFO", "read_svr_port", scfg_file);
	g_busi_config.find_card_timeout = conf_getint("GFREAD_INFO", "find_card_timeout", scfg_file);

}

int gfread_busi_comm()
{
	void *context;
	void* request;
	void* gopanbox_del;
	//void* control_del;
	char recv_buf[MAX_BUF_LEN];
	int  recv_len = 0;
	char send_buf[MAX_BUF_LEN];
	int  send_len = 0;
	int  rc = 0;
	int  tmp_len = 0;
	int  timeout = 0;
	unsigned short tran_code = 0;

	signal(SIGUSR1,busi_sig);
	
	load_busi_cfg();
	LOG_OPEN(g_busi_config.log_server, LOGTAG_BUSI, g_busi_config.log_level);

	//zmq begin
	context = zmq_ctx_new();
	request = zmq_socket_new_dealer_svr(context, g_busi_config.busi_server);
	
	gopanbox_del = zmq_socket_new_dealer(context, g_busi_config.gopanbox_server);
	//control_del = zmq_socket_new_dealer(context, g_busi_config.control_server);
	
	zmq_pollitem_t pollitems[1];
	
	memset(pollitems, 0, sizeof(pollitems));
	pollitems[0].socket = request;  
	pollitems[0].events = ZMQ_POLLIN;  

	LOG_PRINT(L_INFO, "gfread_busi_comm process start OK!");
	
		//初始化读卡器
	rc = gfread_init();
	if(rc < 0)
	{
		LOG_PRINT(L_ERR, "gfread_init error.rc:[%d]", rc);
		return -1;
	}
	
	rc = gfInitParam((const char *)g_busi_config.read_svr_ip, g_busi_config.read_svr_port);
	if(rc < 0)
	{
		LOG_PRINT(L_ERR, "gfInitParam error.rc:[%d]", rc);
		return -1;
	}
	
	rc = gfOpenDevice();
	if(rc < 0)
	{
		LOG_PRINT(L_ERR, "gfInitParam error.rc:[%d]", rc);
		return -1;
	}
	
	rc = gfPrepare(g_busi_config.read_sn, g_busi_config.cos_version);
	if(rc < 0)
	{
		LOG_PRINT(L_ERR, "gfPrepare error.rc:[%d]", rc);
		return -1;
	}
	LOG_PRINT(L_DBG, "read_sn:[%s]", g_busi_config.read_sn);
	LOG_PRINT(L_DBG, "cos_version:[%s]", g_busi_config.cos_version);
	
	//将读卡器号和版本、读卡器状态写到json配置文件中
	cjson_update_value(GPBOX_CONF_JSON_FILE, GOREADVER, g_busi_config.cos_version);
	cjson_update_value(GPBOX_CONF_JSON_FILE, GOREADSN, g_busi_config.read_sn);
	
//	rc = cjson_update_value(g_busi_config.read_sn, g_busi_config.cos_version, READER_STAT_OK);
	
	//接收消息
	timeout = -1;
	busi_stat = BUSI_STAT_FREE;
	while(1)
	{
		if(exit_flag == 1)
		{
			gfDisConnect();
			zmq_close(request); 
			zmq_term(context); 
			busi_exit();
			return 0; 
		}
		
		LOG_PRINT(L_DBG,"gfread_busi_comm zmq_poll begin...timeout:[%d]", timeout);
		
		//设置为空闲等待
		if(timeout == -1)
		{
			cjson_update_value(GPBOX_STAT_JSON_FILE, READSTATUS, READSTAT_1);
		}
		rc = zmq_poll(pollitems, 1, timeout);  
		if(rc <= 0) {
			timeout = -1;
			continue;
		}
		
		//zmq
		if(pollitems[0].revents & ZMQ_POLLIN){
			recv_len = zmq_recv(request, recv_buf,sizeof(recv_buf), 0);  
			if (recv_len <= 0){
				LOG_PRINT(L_ERR, "[mngcenter_run] zmq_recv error=[%d]", zmq_errno());
				continue;
			}
			LOG_HEX(L_INFO, recv_buf, recv_len, "tcp_recv_msg:[%d]", recv_len);

			//解析报文处理
			rc = busi_parse_packet(recv_buf, recv_len, send_buf, &send_len);
			if(send_len == 0)
			{
				//寻卡返回E0E06FF0
				if(busi_stat == BUSI_STAT_FIND)
				{
					timeout = 1*1000;
				}
				
				continue;
			}
			
			//发报文
			LOG_HEX(L_INFO, send_buf, send_len, "zmq_send:[%d]", send_len);
	
			//将参考号发送给TBOX通讯进程
			rc = zmq_send(gopanbox_del, send_buf, send_len, 0);	
		}
	}
	zmq_close(request); 
	zmq_term(context);  

	return 0;
}

//将错误日志内容读入内存
static void read_err_log(char *err_text, int *err_len)
{
	int i = 0;
	long  err_log_len = 0;
	FILE *fpFile = NULL;
	
	fpFile = fopen(CARD_ERR_FILE, "r");
	if(fpFile == NULL)
	{
		LOG_PRINT(L_ERR, "fopen %s err.", CARD_ERR_FILE);
		return;
	}
	
	fseek(fpFile,0L,SEEK_END);
	err_log_len=ftell(fpFile);
	if(err_log_len > 0xFFFF)
	{
		err_log_len = err_log_len - 0xFFFF;
		fseek(fpFile,0xFFFF,SEEK_SET);
	}
	else
	{
		*err_len = err_log_len;
		fseek(fpFile,0L,SEEK_SET);
	}
	
	
	int len = 0;
	int write_len = 0;
	char buf[4096];
	while((len = fread(buf, 1, sizeof(buf), fpFile)) > 0)
	{
		memcpy(err_text+write_len, buf, len);
		write_len += len;
	}
	fclose(fpFile);

	return;
}

static int busi_send_log(int err_code)
{
	busi_log t_busi_log;
	
	memset(&t_busi_log, 0, sizeof(busi_log));
	memcpy(t_busi_log.refnum_buf, g_busi_config.refnum_buf, g_busi_config.refnum_len);
	t_busi_log.refnum_len = g_busi_config.refnum_len;
	
	memcpy(t_busi_log.readsn_buf, g_busi_config.read_sn, strlen(g_busi_config.read_sn));
	t_busi_log.readsn_len = strlen(g_busi_config.read_sn);
	
	memcpy(t_busi_log.cos_version, g_busi_config.cos_version, strlen(g_busi_config.cos_version));
	t_busi_log.cos_len = strlen(g_busi_config.cos_version);
	t_busi_log.tcp_type = 0x01;
	t_busi_log.err_code = err_code;
	
	if(err_code != 0)
	{
		t_busi_log.err_len = 0;
		read_err_log(t_busi_log.err_text, &t_busi_log.err_len);
		gflog_send_err(&t_busi_log);
	}
	else
	{
		memcpy(t_busi_log.time_buf, g_busi_config.time_buf, g_busi_config.time_len);
		t_busi_log.time_len = g_busi_config.time_len;
		gflog_send_err(&t_busi_log);
	}
	
	return 0;
}		

static int busi_packet(unsigned short tran_code, int err_code, char *rsp_buf, int *rsp_len)
{
	int send_len = 0;
	int ret = 0;
	unsigned short usTmpCRC;
	
	//交易码
	send_len = iAddTlvList(rsp_buf, TLV_TAG_JYDM, TLV_TYPE_16, (unsigned char *)&tran_code, 2);
	
	//错误号
	send_len += iAddTlvList(rsp_buf+send_len, TLV_TAG_CWH, TLV_TYPE_32, (unsigned char *)&err_code, 4);
	
	*rsp_len = send_len;

	return 0;
}		

//读证功能
static int busi_read_id(unsigned short tran_code, char *rsp_buf, int *rsp_len)
{
	int i = 0;
	int rc = 0;
	int send_len = 0;
	int  dn_len = 0;
	char sDN[100];

	//寻卡
	LOG_PRINT(L_DBG, "gffind_card begin ...");
	
	//设置为读卡
	cjson_update_value(GPBOX_STAT_JSON_FILE, READSTATUS, READSTAT_1); 
	
	busi_stat = BUSI_STAT_FIND;
	rc =  gffind_card();
	if(rc == 0xE0E06FF0)
	{
		LOG_PRINT(L_ERR, "gffind_card error.rc:[%d]", rc);
		*rsp_len = 0;
		return rc;
	}
	else
	{
		busi_stat = BUSI_STAT_READ;
		LOG_PRINT(L_DBG, "gffind_card return rc:[%08X]", rc);
	}

	LOG_PRINT(L_DBG, "gffind_card end.");

	if(rc != 0)
	{
		LOG_PRINT(L_ERR, "gffind_card error.rc:[%d]", rc);
		busi_packet(tran_code, LOG_ERR_FIND_CARD, rsp_buf, rsp_len);
		return -1;
	}
	
	//寻到卡后点灯-读卡过程中
	rc = gfLedFindCardIng();
	if(rc < 0)
	{
		LOG_PRINT(L_ERR, "busi_read_id gfLedFindCardIng error.rc:[%d]", rc);
		busi_packet(tran_code, LOG_ERR_LED, rsp_buf, rsp_len);
		return -1;
	}
	
	if(tran_code == READ_INFO_OFFLINE)   //脱机读证
	{ 
		rc = gfOffLineReadInfo(sDN, &dn_len);
		if(rc != 0)
		{
			LOG_PRINT(L_ERR, "gfOffLineReadInfo error.rc:[%d]", rc);
			//点灯-认证失败
			gfLedReadIdFail();
			
			//设置为读卡失败
			cjson_update_value(GPBOX_STAT_JSON_FILE, READSTATUS, READSTAT_3); 
			
			busi_packet(tran_code, LOG_ERR_READID, rsp_buf, rsp_len);
			LOG_PRINT(L_DBG, "gfreadIDInfo end .");
			return -1;
		}	
		
		//点灯-认证成功
		gfLedReadIdSuc();
		
		//设置为读卡成功
		cjson_update_value(GPBOX_STAT_JSON_FILE, READSTATUS, READSTAT_2); 
		
		//DN
		send_len = iAddTlvList(rsp_buf, TLV_GOPAN_DN, TLV_TYPE_B, sDN, dn_len);
		
		*rsp_len = send_len;
	}
	else                        //联机读证
 	{	
		//读证
		char id_buf[4096];
		unsigned int id_len = 0;
		LOG_PRINT(L_DBG, "gfreadIDInfo begin ...");
		rc = gfreadIDInfo(g_busi_config.idcard_buf, &g_busi_config.idcard_len, g_busi_config.refnum_buf, &g_busi_config.refnum_len, g_busi_config.time_buf, &g_busi_config.time_len);
		if(rc != 0)
		{
			LOG_PRINT(L_ERR, "busi_read_id gfreadIDInfo error.rc:[%08X]", rc);
			//点灯-认证失败
			gfLedReadIdFail();
			
			//发送错误日志给日志服务
			busi_send_log(rc);
			
			//设置为读卡失败
			cjson_update_value(GPBOX_STAT_JSON_FILE, READSTATUS, READSTAT_3); 
			
			busi_packet(tran_code, LOG_ERR_READID, rsp_buf, rsp_len);
			LOG_PRINT(L_DBG, "gfreadIDInfo end .");
			return -1;
		}
		
		LOG_PRINT(L_DBG, "gfreadIDInfo end .");
		LOG_HEX(L_DBG, g_busi_config.idcard_buf, g_busi_config.idcard_len, "idcard:[%d]", g_busi_config.idcard_len);
		LOG_PRINT(L_DBG, "refnum:[%s]", g_busi_config.refnum_buf);
		LOG_HEX(L_DBG, g_busi_config.time_buf, g_busi_config.time_len, "timebuf:[%d]", g_busi_config.time_len);
		
		//点灯-认证成功
		gfLedReadIdSuc();
	
		//发送时间链给日志服务
		busi_send_log(0);
		
		//获取身份证密文信息
		char enc_id_data[4096];
		int  id_data_len = 0;
		rc = gfreadEncIdData(enc_id_data, &id_data_len);
		if(rc < 0)
		{
			busi_packet(tran_code, LOG_ERR_READID, rsp_buf, rsp_len);
			LOG_PRINT(L_DBG, "gfreadIDInfo end .");
			return -1;
		}
		
		//设置为读卡成功
		cjson_update_value(GPBOX_STAT_JSON_FILE, READSTATUS, READSTAT_2); 

		send_len = iAddTlvList(rsp_buf, TLV_GOPAN_JYDM, TLV_TYPE_16, (unsigned char *)&tran_code, 2);
		
		//身份证密文数据
		send_len += iAddTlvList(rsp_buf+send_len, TLV_IDINFO_ENC, TLV_TYPE_B, enc_id_data, id_data_len);

		//参考号
		send_len += iAddTlvList(rsp_buf+send_len, TLV_GOPAN_INDEX, TLV_TYPE_B, g_busi_config.refnum_buf, g_busi_config.refnum_len);

		*rsp_len = send_len;
	}
	return 0;
}	
		
static int busi_parse_packet(char *recv_buf, int recv_len, char *send_buf, int *send_len)
{		
	int  rc;
	int  tmp_len;
	int  timeout;
	int  socket_fd;
	unsigned short tran_code = 0;
	
	//获取交易码
	iFindTlvList(recv_buf, recv_len, TLV_GOPAN_JYDM, TLV_TYPE_16, (unsigned char *)&tran_code, &tmp_len);

	switch(tran_code)
	{
		case 	READ_INFO_ONLINE:  //认证模式-在线和离线
		case  READ_INFO_OFFLINE:
			LOG_PRINT(L_DBG, "busi_parse_packet begin...");
			//收到读证请求后，点亮LED灯，提示“将身份证放入读卡区域”
			rc = gfLedBegin();
			if(rc < 0)
			{
				LOG_PRINT(L_ERR, "busi_read_id gfLedBegin error.rc:[%d]", rc);
				busi_packet(0x0010, 0x0001, send_buf, send_len);
				break;
			}
			//寻卡和读证
			rc = busi_read_id(tran_code, send_buf, send_len);
			if(rc == 0xE0E06FF0) //寻卡返回6FF0
			{
				LOG_PRINT(L_DBG, "find card return E0E06FF0");
				return 0;
			}
			busi_stat = BUSI_STAT_FREE;
			break;
		default:
			break;
	}
	
END:	

	return 0;
}



static int mng_clt_to_server(void *socket, char *send_buf, int send_len, char *recv_buf, int *recv_len)
{
	int rc;
	void *context;
	void *responder;
	char zmq_dest[64];
	char scfg_file[128];
	char tmp_buf[MAX_BUF_LEN];
	int  tmp_len;
	
	memcpy(tmp_buf + 4, send_buf, send_len);
	tmp_len = send_len;
	
	util_make_crc(tmp_buf+4, tmp_len);
	tmp_len += 2;
	
	*((unsigned int *)tmp_buf) = htonl(tmp_len);
	
	zmq_send(socket,tmp_buf,tmp_len+4,0);
	
	tmp_len = sizeof(tmp_buf);
	rc = zmq_recv(socket, tmp_buf, tmp_len, 0);
	if(rc < 2)
	{
		LOG_PRINT(L_ERR, "mng_clt_to_server zmq_recv error. rc:[%d]", rc);
		return -1;
	}
	tmp_len = rc;
	LOG_HEX(L_INFO, tmp_buf, (tmp_len > 100)?100:tmp_len, "mng_zmq_recv:recv_len:[%d]", tmp_len);
	//LOG_PRINT(L_INFO, "mng_zmq_recv:recv_len:[%d]", tmp_len);
	memcpy(recv_buf, tmp_buf+4, tmp_len-4);
	*recv_len = tmp_len-4;
	
	//CRC校验
	rc = util_check_crc(recv_buf, *recv_len);
	if(rc < 0)
	{
		LOG_PRINT(L_ERR, "mng_clt_to_server util_check_crc check error.");
		return -1;
	}
	*recv_len -= 2;
	
	return 0;	
}

int gfread_busi_run(){
	pid_t pid;
	pid_t pid_busi;

	signal(SIGUSR1,busi_sig);

	pid = fork();
	if(pid < 0){
		printf("fork busi process error[%d]\n",errno);
		return -1;
	}
	else if (pid == 0){
		gfread_busi_comm();
	}
	else{
		pid_busi = pid;
		printf("busi Server start OK!pid=[%d]\n",pid);
	}
	while(1)
	{
		if(exit_flag == 1)
		{
			return 0;
		}
		int status = 0;
		/* waitpid会在这里等待 */
		if (pid_busi > 0){
			pid = waitpid(pid_busi, &status, WNOHANG); 
			if (pid < 0){
				//进程退出，重启进程
				printf("busi Server  pid %d exit, restart it.", pid_busi);
				pid = fork();
				if(pid < 0){
					printf("fork busi process error[%d]",errno);
					sleep(5);
					continue;
				}
				else if(pid == 0){
					gfread_busi_comm();
				}
				else{
					pid_busi = pid;
					printf("busi Server start, new process pid=[%d]", pid);
				}				
			}
		}

		sleep(60);
	}
	
	return 0;
}

int busi_exit(){
	LOG_CLOSE();
	return 0;
}
