#include "global_sys.h"
#include "global_netcard.h"
#include "global_samser.h"
#include "liblog.h"
#include "libtlv.h"
#include "libzmqtools.h"

int main(int argc, char *argv[])
{
	void * _context = NULL;
	void * _sender = NULL;
	void * _request = NULL;
	
	char request_path[64];
	char dest_path[64];
	char scfg_file[256];
	memset(scfg_file, 0, sizeof(scfg_file));
	sprintf(scfg_file,"%s/%s", GOFUN_HOME_DIR, NETCARD_CONFIG_FILE);
	
	strcpy(request_path, "tcp://");
	strcpy(dest_path, "tcp://");
	conf_getstring("GFREAD_INFO", "sGopanboxServer", request_path+6, sizeof(request_path)-6, scfg_file);
	conf_getstring("GFREAD_INFO", "sBusiServer", dest_path+6, sizeof(dest_path)-6, scfg_file);
	
	printf("request_path:[%s]\n", request_path);
	printf("dest_path:[%s]\n", dest_path);

	printf("argc:[%d]\n", argc);

	if(argc != 2)
	{
		printf("Useage:%s + flag[1-认证on,2-认证off]\n", argv[0]);
		return -1;
	}
	
	_context = zmq_ctx_new();
	
	_sender = zmq_socket_new_dealer(_context, dest_path);
	_request = zmq_socket_new_dealer_svr(_context, request_path);

	
	//组包
	char tmp_buf[4096];
	int tmp_len = 0;
	unsigned short tran_code = 0;
	
	if(atoi(argv[1]) == 1)  //在线读证
	{
		tran_code = 0x0101;
		tmp_len = iAddTlvList(tmp_buf, TLV_GOPAN_JYDM, TLV_TYPE_16, (unsigned char *)&tran_code, 2);
	} 
	else if(atoi(argv[1]) == 2)  //离线读证
	{
		tran_code = 0x0102;
		tmp_len = iAddTlvList(tmp_buf, TLV_GOPAN_JYDM, TLV_TYPE_16, (unsigned char *)&tran_code, 2);
	}

	//发送
	zmq_send(_sender,tmp_buf,tmp_len,0);
	
	//接收
	if(atoi(argv[1]) == 1 || atoi(argv[1]) == 2)
	{
		char recv_buf[4096];
		int recv_len  = 0;
		recv_len = zmq_recv (_request, recv_buf,4096, 0);
		printf("recv_len:[%d]\n", recv_len);
		zmq_close(_request); 
	}
	else
	{
		printf("tran_code:[%d]\n", tran_code);
		sleep(1);
		zmq_close(_request); 
	}
	
//	curl_send(NULL, NULL, 0);
	
	zmq_close(_sender); 
	zmq_term(_context);  
	
	return 0;
}
