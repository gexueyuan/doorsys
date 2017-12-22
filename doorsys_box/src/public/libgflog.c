#include "global_samser.h"
#include "global_netcard.h"
#include "global_sys.h"
#include "liblog.h"
#include "libtlv.h"
#include "libutil.h"
#include "libzmqtools.h"

static void * _context = NULL;
static void * _sender = NULL;

typedef struct _log_svr_info{
	char log_svr_ip[IP_LEN];
	int  log_svr_port;
	int log_level;
	char log_server[64];
	char read_log_server[64];
	char log_svr_url[256];
}log_svr_info;

static log_svr_info g_log_svr;

static int exit_flag;

static void gflog_sig(int SigUsr1)
{
	LOG_PRINT(L_ERR, "gflog_sig begin ...");
	exit_flag = 1;
}

static void load_log_cfg(){
	char scfg_file[128];
	char scfg_desg[64];
	memset(scfg_file,0x00,sizeof(scfg_file));
	sprintf(scfg_file,"%s/%s", GOFUN_HOME_DIR, NETCARD_CONFIG_FILE);

	memset(&g_log_svr, 0, sizeof(log_svr_info));
	strcpy(g_log_svr.log_svr_url, "http://");
	strcpy(g_log_svr.log_server, "tcp://");
	strcpy(g_log_svr.read_log_server, "tcp://");
	g_log_svr.log_level  = conf_getint("GFREAD_INFO", "iLogLevel", scfg_file);
	conf_getstring("GFREAD_INFO", "sLogServer", g_log_svr.log_server+6, sizeof(g_log_svr.log_server)-6, scfg_file);
	conf_getstring("GFREAD_INFO", "sReadLogServer", g_log_svr.read_log_server+6, sizeof(g_log_svr.read_log_server)-6, scfg_file);
	conf_getstring("GFREAD_INFO", "log_svr_url", g_log_svr.log_svr_url+7, sizeof(g_log_svr.log_svr_url)-7, scfg_file);
}

int gflog_send_tosvr(char *pSndBuf, int iSendLen)
{
	int iRet = 0;
	int iCltSock = 0; 
	int iRcvLen = 0;
	char sRcvBuf[ERR_LOG_LEN];

	//转成16进制字符串
	memset(sRcvBuf, 0, sizeof(sRcvBuf));
	util_hex_to_asc(pSndBuf, iSendLen, sRcvBuf);
	iRet = curl_send(g_log_svr.log_svr_url, sRcvBuf, 2*iSendLen);
	if(iRet < 0)
	{
		LOG_PRINT(L_ERR,"curl_send error,rc:[%d]", iRet);
		return -1;
	}

#if 0	
	iCltSock = tcp_no_block_connect(pSvrIp, iSvrPort);
	if(iCltSock < 0)
	{
		LOG_PRINT(L_ERR, "tcp_no_block_connect error.ip:[%s],port:[%d]", pSvrIp, iSvrPort);
		return -1;
	}
	
	LOG_HEX(L_INFO,pSndBuf, iSendLen, "LOGTCPSND:[%d]", iSendLen);
	iRet = tcp_write(iINTLEN, iCltSock, pSndBuf, iSendLen);
	if(iRet == -1)
	{
		LOG_PRINT(L_ERR, "sendLogToSvr tcp_write error.ip:[%s],port:[%d]", pSvrIp, iSvrPort);
		close(iCltSock);
		return -1;	
	}

	iRet = tcp_read(iINTLEN,iCltSock, sRcvBuf, MSG_MAXLEN, &iRcvLen, 2, 0);
	if(iRet == -1)
	{
		LOG_PRINT(L_ERR, "sendLogToSvr tcp_read error.ip:[%s],port:[%d]", pSvrIp, iSvrPort);
		close(iCltSock);
		return -1;
	}
	LOG_HEX(L_INFO, sRcvBuf, iRcvLen, "LOGTCPRCV:[%d]", iRcvLen);

	close(iCltSock);
#endif	

	return 0;
}

int gflog_send_err(busi_log *p_busi_log)
{
	int iTmpLen = 0;
	int iTmpSndLen = 0;
	int iErrMsgLen = 0;
	unsigned int uiErrCode = 0;
	unsigned short usErrMsgLen = 0;
	unsigned char ucTranCode;
	char sSndBuff[ERR_LOG_LEN];
	unsigned char ucLogSrc;
	unsigned short usExtInfoTotLen=0; 	
	char sVersion[VERSION_LEN];
	int iUUidLen = 0;
	char sSamSn[3];
	int  iMsgLogLen = 0;
	unsigned short uiJydm = 0;
	
	//组报文
	memset(sSndBuff, 0, sizeof(sSndBuff));
	
	//交易码
	if(p_busi_log->err_code != 0)
	{
		uiJydm = LOG_CODE_01;
	}
	else
	{
		uiJydm = LOG_CODE_02;
	}
	iTmpSndLen = iAddTlvList(sSndBuff, TLV_TAG_JYDM, TLV_TYPE_16, (char *)&uiJydm, 2);
	
	//参考号
	iTmpSndLen += iAddTlvList(sSndBuff+iTmpSndLen, TLV_TAG_CKH, TLV_TYPE_B, p_busi_log->refnum_buf, p_busi_log->refnum_len);
	
	//reader序号
	iTmpSndLen += iAddTlvList(sSndBuff+iTmpSndLen, TLV_LOG_READSN, TLV_TYPE_B, p_busi_log->readsn_buf, p_busi_log->readsn_len);
	
	//手机唯一编号
	iTmpSndLen += iAddTlvList(sSndBuff+iTmpSndLen, TLV_LOG_SJWYBH, TLV_TYPE_B, p_busi_log->readsn_buf, p_busi_log->readsn_len);
	
	//COS版本
	iTmpSndLen += iAddTlvList(sSndBuff+iTmpSndLen, TLV_LOG_COSV, TLV_TYPE_B, p_busi_log->cos_version, p_busi_log->cos_len);
	
	//通讯模式
	iTmpSndLen += iAddTlvList(sSndBuff+iTmpSndLen, TLV_LOG_TCPTYPE, TLV_TYPE_B, (unsigned char *)&p_busi_log->tcp_type, 1);

	//错误码
	uiErrCode = p_busi_log->err_code;
	iTmpSndLen += iAddTlvList(sSndBuff+iTmpSndLen,TLV_LOG_CWM, TLV_TYPE_32, (char *)&uiErrCode, 4);
	

	if(p_busi_log->err_code != 0)
	{
		//错误内容
		iTmpSndLen += iAddTlvList(sSndBuff+iTmpSndLen,TLV_LOG_CWNR, TLV_TYPE_B, p_busi_log->err_text, p_busi_log->err_len);
	}
	else
	{
		//时间链
		iTmpSndLen += iAddTlvList(sSndBuff+iTmpSndLen,TLV_TAG_SJTJ, TLV_TYPE_B, p_busi_log->time_buf, p_busi_log->time_len);
	}
	
#if 0
	//计算CRC
	util_make_crc(sSndBuff, iTmpSndLen);
	iTmpSndLen += 2;
#endif
		
	//发送报文
	gflog_clt(sSndBuff, iTmpSndLen);
	
	return 0;
}


int gflog_clt(char *recv_buf, int recv_len)
{
	int rc;
	char tmp_buf[ERR_LOG_LEN];
	int tmp_len;
	
	if(NULL == _context)
	{
		_context = zmq_ctx_new();
	}
	if(NULL == _sender)
	{
		char dest_path[64];
		char scfg_file[256];
		
		memset(scfg_file, 0, sizeof(scfg_file));
		sprintf(scfg_file,"%s/%s", GOFUN_HOME_DIR, NETCARD_CONFIG_FILE);
		
		memset(dest_path, 0, sizeof(dest_path));
		strcpy(dest_path, "tcp://");
		conf_getstring("GFREAD_INFO", "sReadLogServer", dest_path+6, sizeof(dest_path)-6, scfg_file);

		_sender = zmq_socket_new_push(_context,dest_path);
	}

#if 0	
	log_svr_info t_log_svr_info;
	
	memset(&t_log_svr_info, 0, sizeof(log_svr_info));
	memcpy(t_log_svr_info.log_svr_ip, svr_ip, strlen(svr_ip));
	t_log_svr_info.log_svr_port = svr_port;
	
	memcpy(tmp_buf, &t_log_svr_info, sizeof(log_svr_info));
	tmp_len = sizeof(log_svr_info);
	memcpy(tmp_buf+tmp_len, recv_buf, recv_len);
	tmp_len += recv_len;
#endif

	memcpy(tmp_buf, recv_buf, recv_len);
	tmp_len = recv_len;
	
	LOG_HEX(L_INFO, tmp_buf, (tmp_len > 1024 )?1024:tmp_len,"sam_log_send:[%d]", tmp_len);
	rc = zmq_send(_sender, tmp_buf, tmp_len, ZMQ_DONTWAIT);
	if(rc < 0)
	{
		LOG_PRINT(L_ERR,"zmq_send error,rc:[%d]", rc);
		return -1;
	}
	
	return 0;
}


int gflog_server(){
	char recv_buf[ERR_LOG_LEN];
	int  recv_len;
	void* context;
	void* collecter;
	log_svr_info *p_log_svr_info;
	
	signal(SIGUSR1,gflog_sig);
	chdir("/");
	
	load_log_cfg();
	LOG_OPEN(g_log_svr.log_server, LOGTAG_GFLOG, g_log_svr.log_level);

	
	LOG_PRINT(L_INFO, "gflog_server process start OK!");

	context = zmq_ctx_new();	
	collecter = zmq_socket_new_pull(context,g_log_svr.read_log_server);

	while(1){
		if(exit_flag == 1)
		{
			zmq_close (collecter);
			zmq_ctx_destroy (context);
			samlog_exit();
			return 0;
		}
		
		recv_len = zmq_recv(collecter, recv_buf, sizeof(recv_buf),0);
		if(recv_len < 0){
			LOG_PRINT(L_ERR,"zmq_recv error,recv_len:[%d]", recv_len);
			break;
		}

		LOG_HEX(L_INFO, recv_buf, (recv_len > 1024 )?1024:recv_len,"sam_log_recv:[%d]", recv_len);
		gflog_send_tosvr(recv_buf, recv_len);
	}

	zmq_close (collecter);
	zmq_ctx_destroy (context);
	return 0;
}

int gflog_run(){
	pid_t pid;
	pid_t pid_log;

	signal(SIGUSR1,gflog_sig);

	pid = fork();
	if(pid < 0){
		printf("fork gflog process error[%d]\n",errno);
		return -1;
	}
	else if (pid == 0){
		gflog_server();
	}
	else{
		pid_log = pid;
		printf("gflog Server start OK!pid=[%d]\n",pid);
	}
	
	while(1)
	{
		if(exit_flag == 1)
		{
			return 0;
		}
		int status = 0;
		/* waitpid会在这里等待 */
		if (pid_log > 0){
			pid = waitpid(pid_log, &status, WNOHANG); 
			if (pid < 0){
				//进程退出，重启进程
				printf("gflog Server  pid %d exit, restart it.", pid_log);
				pid = fork();
				if(pid < 0){
					printf("fork gflog process error[%d]",errno);
					sleep(5);
					continue;
				}
				else if(pid == 0){
					gflog_server();
				}
				else{
					pid_log = pid;
					printf("busi Server start, new process pid=[%d]", pid);
				}				
			}
		}

		sleep(60);
	}
	
	return 0;
}

int samlog_exit(){
	LOG_CLOSE();
	return 0;
}

