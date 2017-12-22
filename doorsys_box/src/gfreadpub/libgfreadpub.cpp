#include "IDCardInclude.h"
#include "libgfreadpub.h"
#include "gfreadpub.h"
#include "IDCardGoFunParam.h"
//#include "TDRTokenStorage.h"
#include "TDRIDCardFile.h"
#include "TDRIDCardStorage.h"
#include "IDCardPR11CosUpdate.h"
#include "luareader.h"
#include "liblog.h"
#include "global_samser.h"
#include "alg/OpenAlg.h"
#include "global_netcard.h"
#include "libanalysisconf.h"

static IDCARD::IDCardPR11Update *g_idCardPR11Update = NULL;
static IDCARD::IDCardTask* g_idCardTask = NULL;
static IDCARD::IDCardGoFunParam* g_idCardParam = NULL;
static TDRIDCardStorage* g_idReaderStorage_ = NULL;
static TDRIDCardFile* g_file = NULL;
static AKey::IDCardReader*  g_idReader = NULL;
static HANDLE g_context = NULL;
static void *g_mjcon = NULL;

int gfread_init(AKey::IProtocol *s_reader);
	


static int luareader_callback(void *context, const char * command, const unsigned char * input, int input_len, unsigned char * output, int max_output_size)
{

    return 0;
}

static int gfUInt16ToBytes(int n, unsigned char * output)
{
	output[0] = (BYTE)((n >> 8) & 0xFF);
	output[1] = (BYTE)((n) & 0xFF);
	return 2;
}

static int gfLedTransmit(char led_index)
{
	int  hr = 0;
	int  send_len = 0;
	int  recv_len = 0;
	unsigned char send_buf[100];
	unsigned char recv_buf[100];
		
	
	send_buf[0] = 0x80;
	send_buf[1] = 0x35;
	send_buf[2] = 0x00;
	send_buf[3] = led_index;
	send_buf[4] = 0x00;
	send_len = 5;
	hr = luareader_transmit(g_context, send_buf, send_len, recv_buf, 100, 3000);
	//hr = gfZmqTransmit(g_context, send_buf, send_len, recv_buf, 3);
	printf("hr:[%d]\n", hr);
	if(hr > 0)
	{
		print_hex(recv_buf, hr);
	}
	
	return hr;
}

//点灯-寻卡阶段 慢闪RED (红灯慢闪，亮1s，慢1s，APP有订单时发送，提示可以刷卡)
int gfLedBegin()
{
	return 0;
}

//点灯-读卡过程中(寻卡) 0x02:红灯快闪，亮100ms，慢100ms，寻到卡片时发送
int gfLedFindCardIng()
{
	return 0;
}

//点灯-读卡成功   暂无
int gfLedFindCardSuc()
{
	return 0;
}

//点灯-读卡失败  暂无
int gfLedFindCardFail()
{
	return 0;
}

//点灯-认证成功 0x03：绿灯常亮5s，读证成功时发送
int gfLedReadIdSuc()
{
	return 0;
}

//点灯-认证失败   0x04: 红灯常亮5s，读证失败时发送；
int gfLedReadIdFail()
{
	return 0;
}

//灭灯   0x05: 灯灭
int gfLedClose()
{
	return 0;
}


//点灯-正在升级 暂无
int gfLedCosUpdate()
{
	return 0;
}

//脱机读证
int gfOffLineReadInfo(char *sDN, int *dn_len)
{
	int hr = 0;
	unsigned char tmp_buf[1024];
	hr = luareader_transmit(g_context, (unsigned char *)"\xF0\xF4\x81\x04\x00", 5, (unsigned char *)sDN, 1024, 3000);
//	hr = gfZmqTransmit(g_context, (unsigned char *)"\xF0\xF4\x81\x04\x00", 5, (unsigned char *)sDN, 3);
	if(hr < 2)
	{
		printf("gfOffLineReadInfo error.rc:[%d]\n", hr);
		return -1;
	}
	unsigned short status = (sDN[hr-2] << 8 + sDN[hr-1]);
	printf("gfOffLineReadInfo return.DN[%d]:[%02x], DN[%d]:[%02x]\n", hr-2, sDN[hr-2], hr-1, sDN[hr-1]);
	if(status != 0x9000)
	{
		printf("gfOffLineReadInfo error.status:[%02X]\n", status);
		return -1;
	}

	printf("sDN:[%d]\n", hr);
	print_hex((unsigned char *)sDN, hr);
	printf("sDN end:\n");

	*dn_len = hr-2;
	
	return 0;
}


int gfreadDevInit()
{
//	char scfg_file[128];
//	char scfg_desg[64];
//	memset(scfg_file,0x00,sizeof(scfg_file));
//	sprintf(scfg_file,"%s/%s", GOFUN_HOME_DIR, NETCARD_CONFIG_FILE);
	
//	strcpy(scfg_desg, "tcp://");
//	conf_getstring((char *)"GFREAD_INFO", (char *)"sAcsServer", scfg_desg+6, sizeof(scfg_desg)-6, scfg_file);
	
//	g_mjcon = zmq_ctx_new();
//	g_context = zmq_socket_new_dealer(g_mjcon, scfg_desg);
	int rc = 0;
	unsigned char output[1024] = {0};
	//PR11 & R0A3
	g_context = luareader_new(0,  NULL, luareader_callback); 
	if(NULL == g_context)
	{
		return -1;
	}

	rc = luareader_get_list(g_context, (char *)output, sizeof(output));
	if(rc < 0)
	{
		printf("luareader_get_list(%p)=%d(%s)\n", g_context, rc, output);
		luareader_term(g_context);
		return -2;
	}

	rc = luareader_connect(g_context, (char *)output);
	if(rc < 0)
	{
		rc = luareader_pop_value(g_context, (char *)output, sizeof(output)); 
		printf("luareader_connect(%p)=%d\n", g_context, rc);
		printf("output：[%s]\n", output);
		luareader_term(g_context);
		return -3;
	}	
	return 0;
}

//门禁项目获取身份证密文信息
int gfreadEncIdData(unsigned char *pbRecv, int *punRLen)
{
	int rc = 0;
	unsigned char send_buf[100];
	int  send_len = 0;
	
//	memcpy(send_buf, "\x00\x01\x02", 3);  --TODO
//	send_len = 3;
	rc = gfZmqTransmit(g_context, send_buf, send_len, pbRecv, 3);
	if(rc > 0)
	{
		*punRLen = rc;
		rc = 0;
	}
	
	return rc;
}

int gfreadDevRead(unsigned char *pbSend, unsigned int unSLen, unsigned char *pbRecv, unsigned int *punRLen)
{
	int rc = 0;
	
	rc = gfZmqTransmit(g_context, pbSend, unSLen, pbRecv, 3);
	if(rc > 0)
	{
		*punRLen = rc;
		rc = 0;
	}


	return rc;
}

int gfreadDevClose()
{
	int rc = 0;
	unsigned char output[1024] = {0};

	rc = luareader_disconnect(g_context);

	rc = luareader_pop_value(g_context, (char *)output, sizeof(output)); // get error message

	luareader_term(g_context);
//	zmq_close(g_context);
//	zmq_term(g_mjcon);
	
	return 0;
} 

int gfOpenDevice()
{
	int rc = 0;

	rc = gfreadDevInit();
	if(rc < 0)
	{
		return rc;
	}
	if(NULL == g_idReader)
	{
		g_idReader = new AKey::IDCardReader();
	}
	g_idReader->Init(g_context);

	g_idCardParam->setReader(g_idReader);
	
	return 0;
}

int gfPrepare(char *sn_buf, char *cos_buf)
{
	int hr = 0;
	CBuffer snBuf, cosBuf;
	
	hr = g_idCardTask->PrepareWork(snBuf, cosBuf);
	if(hr != AKEY_RV_OK)
	{
		printf("gfPrepare err.hr:[%08X]\n", hr);
		return -1;
	}
	memcpy(sn_buf, snBuf.GetBuffer(), snBuf.GetLength());
	memcpy(cos_buf, cosBuf.GetBuffer(), cosBuf.GetLength());

	return hr;
}

int gfDisConnect()
{
	int hr = 0;
	
	g_idCardTask->DeviceDisConnect();
	gfreadDevClose();
	
	return 0;
}

int gfreadIDInfo(char *idcard_buf, unsigned int *idcard_len, char *ref_buf, int *ref_len, char *time_buf, int *time_len)
{
	int hr = AKEY_RV_OK;
	CBuffer idInfoBuf, refNumBuf, samVerBuf, noteBuf, timeBuf;
	BYTE bSamIndex, imgageFlag;
	
	hr = g_idCardTask->ReadIDCardInfo(refNumBuf, samVerBuf, bSamIndex, idInfoBuf, noteBuf, &imgageFlag, timeBuf);
	if(hr != AKEY_RV_OK)
	{
		printf("ReadIDCardInfo error[%08x]",hr);
		return hr;
	}

	memcpy(idcard_buf, idInfoBuf.GetBuffer(), idInfoBuf.GetLength());
	*idcard_len = idInfoBuf.GetLength();
	
	memcpy(ref_buf, refNumBuf.GetBuffer(), refNumBuf.GetLength());
	*ref_len = refNumBuf.GetLength();
	
	memcpy(time_buf, timeBuf.GetBuffer(), timeBuf.GetLength());
	*time_len = timeBuf.GetLength();
	
	return hr;
	
}

void IDReaderFinish()
{
	//提示身份证可以拿走
	gfLedFindCardSuc();
	return;
}

void IDReaderWaitSamNum(UINT32 u32WaitSamNum)
{
	return;
}

int gfInitParam(const char *ip_addr, int ip_port)
{
	TDR_PARAM_Context paramCtx;
	
	memset(&paramCtx, 0, sizeof(TDR_PARAM_Context));
	
	strcpy(paramCtx.IpAddr, ip_addr);
	paramCtx.Port = ip_port;
	strcpy(paramCtx.businessID, "tdr_zhuhai_01");
	paramCtx.nBusinessLen = strlen(paramCtx.businessID);
	memset(paramCtx.sockCfg, 0x01, 4);

	strcpy(paramCtx.errFile, CARD_ERR_FILE);
	
	paramCtx.u32NetRetry = 3;
	paramCtx.u32ReaderRetry = 3;
	paramCtx.u32ConnTime = 2*1000;//单位毫秒
	paramCtx.u32TCPSRTime = 2*1000;//单位毫秒
	paramCtx.u32UDPSRTime = 500;	//单位毫秒
	paramCtx.nBusNoTimeOut = 60;//单位秒
	paramCtx.u32PerPkgTimeOut = 6000;//单位毫秒
	
	paramCtx.bBindMode = 0x00; 
	paramCtx.bRetryFlag = 0x00;
	paramCtx.bWorkMode = MODE_NET_GATEWAY;

	TDR_CALLBACK_CONTEXT  callBackCtx;
	memset(&callBackCtx, 0, sizeof(TDR_CALLBACK_CONTEXT));
	callBackCtx.finishCb = IDReaderFinish;
	callBackCtx.waitSamCb = IDReaderWaitSamNum;

	g_idCardTask->Init(&paramCtx, g_idCardParam, g_file, &callBackCtx);
	
	return 0;
}


int gfread_init()
{

	if(g_idCardTask == NULL)
	{
		g_idCardTask = new IDCARD::IDCardTask();
		
		if(g_idReaderStorage_ == NULL)
		{
			g_idReaderStorage_ = new TDRIDCardStorage(CARD_STORAGE_TMP_FILE);
		}
		
		if(g_file == NULL)
		{
			g_file = new TDRIDCardFile();
		}

		if(g_idCardParam == NULL)
		{
			g_idCardParam = new IDCARD::IDCardGoFunParam(NULL, NULL, g_idReaderStorage_);
		}
	}
	
	return 0;
}

//寻卡
int gffind_card()
{
	int rc = 0;

	rc = 0;   //门禁项目没有寻卡
	
	return rc;
}

#if 1
int test_gfread_run()
{
	int  rc = 0;
	char svr_ip[20];
	int  svr_port;
	
	memset(svr_ip, 0, sizeof(svr_ip));
	strcpy(svr_ip, "10.200.15.90");
	svr_port = 30000;
	
	rc = gfread_init();

	gfInitParam(svr_ip, svr_port);

	rc = gfOpenDevice();
	if(rc != AKEY_RV_OK)
	{
		return rc;
	}
	
	char sn_buf[100];
	char cos_buf[100];
	rc = gfPrepare(sn_buf, cos_buf);
	if(rc != AKEY_RV_OK)
	{
		//LOG_PRINT(L_ERR, "ReadIDCardInfo error[%08x]",rc);
		return rc;
	}
	
	do{
		 rc =  g_idCardTask->FindCard(NULL);
	}while(rc == 0xE0E06FF0);
	if(rc != AKEY_RV_OK)
	{
		return rc;
	}		

	char id_buf[4096];
	unsigned int id_len = 0;
	char refnum_buf[100];
	int  refnum_len = 0;
	char timeout_buf[100];
	int  time_len = 0;
	rc = gfreadIDInfo(id_buf, &id_len, refnum_buf, &refnum_len,timeout_buf, &time_len);
	if(rc != AKEY_RV_OK)
	{
		printf("ReadIDCardInfo error[%08x]\n",rc);
		return rc;
	}
	refnum_buf[refnum_len] = 0x00;
	
	printf("refnum_buf:[%s]\n", refnum_buf);
	print_hex((unsigned char*)timeout_buf, (UINT32)time_len);
	

	printf("gfreadIDInfo success.\n");
	print_hex((unsigned char*)id_buf, (UINT32)id_len);
	printf("gfreadIDInfo success111.\n");
	
	return 0;
}

int gfread_exit(){
	return 0;
}
#endif
