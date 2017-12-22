#include "global_sys.h"
#include "libzmqtools.h"
#include "liblog.h"


static char _module[128] = {0};
static int _loglevel = -5;
static void * _context = NULL;
static void * _sender = NULL;


static unsigned int _UInt16FromBytes(const unsigned char * input)
{
	return ((input[0] << 8) | input[1]);
}

static int _UInt16ToBytes(unsigned int n, unsigned char * output)
{
	output[0] = (BYTE)((n >> 8) & 0xFF);
	output[1] = (BYTE)((n) & 0xFF);
	return 2;
}

static const char * _get_log_level_name(int level){
	const char * name_list[] = {"ALERT", "CRIT", "ERR", "NONE", "WARN", "NOTICE", "INFO", "DBG"};
	return ((level >= L_ALERT) && (level <= L_DBG))? name_list[level - L_ALERT] : "NONE";
}

#ifdef WIN32  // test
void log_open(char* zmq_dest, char* module, int loglevel){
	strcpy(_module, module);
	_loglevel = loglevel;
}
void log_close(){
}

static void log_vprint(int level, char * filename, int lineno, char *fmt, va_list args){
#ifndef WIN32
	struct timeval tvl;
	struct tm * local_t;

	gettimeofday(&tvl, NULL);
	local_t = localtime(&tvl.tv_sec);

	printf("%s.%4d%02d%02d", _module,  local_t->tm_year+1900,local_t->tm_mon+1,local_t->tm_mday);
	printf("[%02d:%02d:%02d.%03d][%s]", local_t->tm_hour,local_t->tm_min,local_t->tm_sec,(int)(tvl.tv_usec/1000),_get_log_level_name(level));
#else
	SYSTEMTIME st;
	GetLocalTime(&st);

	printf("%s.%4d%02d%02d", _module, st.wYear,st.wMonth,st.wDay);
	printf("[%02d:%02d:%02d.%03d][%s]", st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,_get_log_level_name(level));
#endif
	if (filename != NULL){
		printf(" [%s(%d)]",filename,lineno);
	}

	vprintf(fmt, args);
	printf("\n");
}

static void log_vhex(int level, char* data,int len,char *fmt,va_list args){
	int i;

	log_vprint(level, NULL, 0, fmt, args);

	printf("\t");
	for (i=0; i<len; i++){
		printf("%02X", (unsigned char)data[i]);
	}
	printf("\n");
}

#else
void log_open(char* zmq_dest, char* module, int loglevel){
	char sLocalIp[100];
	memset(sLocalIp, 0, sizeof(sLocalIp));
	tcp_get_local_ip(sLocalIp, "eth0");
	strcpy(_module, sLocalIp);
	sprintf(_module+strlen(sLocalIp),"/%s",module);
	_loglevel = loglevel;
	_context = zmq_ctx_new();
	_sender = zmq_socket_new_push(_context,zmq_dest);
}

void log_close(){
	zmq_close(_sender);
	zmq_ctx_destroy(_context);
}

static void log_vprint(int level, char * filename, int lineno, char *fmt, va_list args){
	char buffer[2048];
	int offset;
	int rc,tmplen;
#ifndef WIN32
	struct timeval tvl;
	struct tm * local_t;

	gettimeofday(&tvl, NULL);
	local_t = localtime(&tvl.tv_sec);
#else
	SYSTEMTIME st;
	GetLocalTime(&st);
#endif

	// module
	offset = 0;
#ifndef WIN32
	tmplen = sprintf(buffer+offset+2, "%s.%4d%02d%02d", _module,  local_t->tm_year+1900,local_t->tm_mon+1,local_t->tm_mday);
#else
	tmplen = sprintf(buffer+offset+2, "%s.%4d%02d%02d", _module, st.wYear,st.wMonth,st.wDay);
#endif
	_UInt16ToBytes(tmplen, buffer+offset);
	offset += (2+tmplen);

	// level
	tmplen = 1;
	buffer[offset+2] = (char)level;
	_UInt16ToBytes(tmplen, buffer+offset);
	offset += (2+tmplen);

	// message
#ifndef WIN32
	tmplen = sprintf(buffer+offset+2,"[%02d:%02d:%02d.%03d][%s]", local_t->tm_hour,local_t->tm_min,local_t->tm_sec,(int)(tvl.tv_usec/1000),_get_log_level_name(level));
#else
	tmplen = sprintf(buffer+offset+2,"[%02d:%02d:%02d.%03d][%s]", st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,_get_log_level_name(level));
#endif
	if (filename != NULL){
		tmplen += sprintf(buffer+offset+2+tmplen," [%s(%d)]",filename,lineno);
	}
	tmplen += vsprintf(buffer+offset+2+tmplen, fmt, args);
	_UInt16ToBytes(tmplen, buffer+offset);
	offset += (2 + tmplen);

	// data
	_UInt16ToBytes(0, buffer+offset);
	offset += 2;

	rc = zmq_send(_sender, buffer, offset, ZMQ_DONTWAIT);
	if(rc < 0){
		printf("[log] zmq_send err=[%d]!",rc);
	}
}

static void log_vhex(int level, char* data,int len,char *fmt,va_list args){
	char buffer[0x4000];//16K
	int offset;
	int rc,tmplen;
#ifndef WIN32
	struct timeval tvl;
	struct tm * local_t;

	gettimeofday(&tvl, NULL);
	local_t = localtime(&tvl.tv_sec);
#else
	SYSTEMTIME st;
	GetLocalTime(&st);
#endif

	// module
	offset = 0;
#ifndef WIN32
	tmplen = sprintf(buffer+offset+2, "%s.%4d%02d%02d", _module,  local_t->tm_year+1900,local_t->tm_mon+1,local_t->tm_mday);
#else
	tmplen = sprintf(buffer+offset+2, "%s.%4d%02d%02d", _module, st.wYear,st.wMonth,st.wDay);
#endif
	_UInt16ToBytes(tmplen, buffer+offset);
	offset += (2+tmplen);

	// level
	tmplen = 1;
	buffer[offset+2] = (char)level;
	_UInt16ToBytes(tmplen, buffer+offset);
	offset += (2+tmplen);

	// message
#ifndef WIN32
	tmplen = sprintf(buffer+offset+2,"[%02d:%02d:%02d.%03d][%s]", local_t->tm_hour,local_t->tm_min,local_t->tm_sec,(int)(tvl.tv_usec/1000),_get_log_level_name(level));
#else
	tmplen = sprintf(buffer+offset+2,"[%02d:%02d:%02d.%03d][%s]", st.wHour,st.wMinute,st.wSecond,st.wMilliseconds,_get_log_level_name(level));
#endif
	tmplen += vsprintf(buffer+offset+2+tmplen,fmt,args);
	_UInt16ToBytes(tmplen, buffer+offset);
	offset += (2 + tmplen);

	// data
	tmplen = sizeof(buffer) - offset - 2;
	if (tmplen > len){
		tmplen = len;
	}
	_UInt16ToBytes(tmplen, buffer+offset);
	memcpy(buffer+offset+2, data, tmplen);
	offset += (2+tmplen);

	rc = zmq_send(_sender, buffer, offset, ZMQ_DONTWAIT);
	if(rc < 0){
		printf("[log] zmq_send err=[%d]!",rc);
	}
}
#endif

void log_print(int level, char *fmt,...){
	if (_loglevel >= level){
		va_list args;
		va_start(args,fmt);
		log_vprint(level, NULL, 0, fmt, args);
		va_end(args);
	}
}

void log_printex(int level, char * filename, int lineno, char *fmt,...){
	if (_loglevel >= level){
		va_list args;
		va_start(args,fmt);
		log_vprint(level, filename, lineno, fmt, args);
		va_end(args);
	}
}

void log_hex(int level, char* data,int len,char *fmt,...){
	if (_loglevel >= level){
		va_list args;
		va_start(args,fmt);
		log_vhex(level, data, len, fmt, args);
		va_end(args);
	}
}

static int log_creatdir(char *muldir)
{
#ifndef WIN32
	int    i,len;
    char   str[512 + 1];
    int    j=0;

    strncpy( str, muldir, 512 );
    len=strlen(str);
    for( i=1; i<len; i++ )
    {
        if( str[i]=='/')
        {
            str[i] = '\0';
            j++;
            /*判断此目录是否存在,不存在则创建*/
            if( access(str, F_OK)!=0 )
            {
                    mkdir( str, 0755 );
            }
            str[i]='/';
        }
    }
#endif
    return 0;
}


static int _writelog2file(char * log_path, char * buff, int len){
	char filename[260];
	char* module, *config, *message, *data;
	int i, offset, modlen, cfglen, msglen, dlen;
	FILE * fp;

	offset = 0;
	// module
	modlen = _UInt16FromBytes(buff+offset);
	if (modlen > (len-offset-2)){
		return -1;
	}
	module = buff+offset+2;
	offset += (2+modlen);

	// config
	cfglen = _UInt16FromBytes(buff+offset);
	if (cfglen > (len-offset-2)){
		return -1;
	}
	config = buff+offset+2;
	offset += (2+cfglen);

	// message
	msglen = _UInt16FromBytes(buff+offset);
	if (msglen > (len-offset-2)){
		return -1;
	}
	message = buff+offset+2;
	offset += (2+msglen);

	// data
	dlen = _UInt16FromBytes(buff+offset);
	if (dlen > (len-offset-2)){
		return -1;
	}
	data = buff+offset+2;
	offset += (2+dlen);

	// filename
	memset(filename, 0, sizeof(filename));
	strcpy(filename, log_path);
	memcpy(filename+strlen(filename), module, modlen);	

	// write file
	fp = fopen(filename, "a+");
	if (fp == NULL){
		log_creatdir(filename);
		fp = fopen(filename, "a+");
	}

	if (fp != NULL){
		fwrite(message, 1, msglen, fp);
		if (dlen > 0){
			for (i=0; i<dlen; i++){
				fprintf(fp, "%02X", (unsigned char)data[i]);
			}
		}
		fwrite("\n", 1, 1, fp);
		fclose(fp);
	}
	if (config[0] <= L_ERR){
		strcat(filename, ".err");
		fp = fopen(filename, "a+");
		if (fp != NULL){
			fwrite(message, 1, msglen, fp);
			fwrite("\n", 1, 1, fp);
			fclose(fp);
		}
	}
	return (msglen+dlen);
}

int log_server(char* zmq_dest, char * log_path){
	char recv_buf[0x4000]; // 16K
	int  recv_len;
	void* context;
	void* collecter;

	context = zmq_ctx_new();	
	collecter = zmq_socket_new_pull(context,zmq_dest);

	while(1){
		recv_len = zmq_recv(collecter, recv_buf, sizeof(recv_buf),0);
		if(recv_len < 0){
			printf("[log_server]zmq_recv error[%d]",recv_len);
			break;
		}

		_writelog2file(log_path, recv_buf, recv_len);
	}

	zmq_close (collecter);
	zmq_ctx_destroy (context);
	return 0;
}
