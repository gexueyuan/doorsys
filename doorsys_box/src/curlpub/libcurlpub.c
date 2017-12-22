#include "global_sys.h"
#include "global_samser.h"
#include "global_netcard.h"
#include "curl/curl.h"
#include <syslog.h>

static char userAgent[]="user-agent:Android_6.0/TDRMPC_Phone_1.3.6.0/SM-G9300/samsung";
static char di[]="di:xxxx/1.0/xxxx/0/xxxx/2560*1440";

struct data {
  char trace_ascii; /* 1 or 0 */ 
};

typedef struct _send_data_info{
	char send_data[ERR_LOG_LEN];
	int  send_len;
}send_data_info;
 
static
void dump(const char *text,
		  FILE *stream, unsigned char *ptr, size_t size,
		  char nohex)
{
  size_t i;
  size_t c;
 
  unsigned int width=0x10;
 
  if(nohex)
	/* without the hex output, we can fit more on screen */ 
	width = 0x40;
 
  fprintf(stream, "%s, %10.10ld bytes (0x%8.8lx)\n",
		  text, (long)size, (long)size);
 
  for(i=0; i<size; i+= width) {
 
	fprintf(stream, "%4.4lx: ", (long)i);
 
	if(!nohex) {
	  /* hex not disabled, show it */ 
	  for(c = 0; c < width; c++)
		if(i+c < size)
		  fprintf(stream, "%02x ", ptr[i+c]);
		else
		  fputs("   ", stream);
	}
 
	for(c = 0; (c < width) && (i+c < size); c++) {
	  /* check for 0D0A; if found, skip past and start a new line of output */ 
	  if(nohex && (i+c+1 < size) && ptr[i+c]==0x0D && ptr[i+c+1]==0x0A) {
		i+=(c+2-width);
		break;
	  }
	  fprintf(stream, "%c",
			  (ptr[i+c]>=0x20) && (ptr[i+c]<0x80)?ptr[i+c]:'.');
	  /* check again for 0D0A, to avoid an extra \n if it's at width */ 
	  if(nohex && (i+c+2 < size) && ptr[i+c+1]==0x0D && ptr[i+c+2]==0x0A) {
		i+=(c+3-width);
		break;
	  }
	}
	fputc('\n', stream); /* newline */ 
  }
  fflush(stream);
}
 
static
int my_trace(CURL *handle, curl_infotype type,
			 char *data, size_t size,
			 void *userp)
{
  struct data *config = (struct data *)userp;
  const char *text;
  (void)handle; /* prevent compiler warning */ 
 
  switch (type) {
  case CURLINFO_TEXT:
	fprintf(stderr, "== Info: %s", data);
  default: /* in case a new one is introduced to shock us */ 
	return 0;
 
  case CURLINFO_HEADER_OUT:
	text = "=> Send header";
	break;
  case CURLINFO_DATA_OUT:
	text = "=> Send data";
	break;
  case CURLINFO_SSL_DATA_OUT:
	text = "=> Send SSL data";
	break;
  case CURLINFO_HEADER_IN:
	text = "<= Recv header";
	break;
  case CURLINFO_DATA_IN:
	text = "<= Recv data";
	break;
  case CURLINFO_SSL_DATA_IN:
	text = "<= Recv SSL data";
	break;
  }

#if 0
   log_Print(DEBUG, "%s", text);
   if(size < 4096)
   {
	   log_Print(DEBUG, "%s", data);
   }
#endif
 
  dump(text, stderr, (unsigned char *)data, size, config->trace_ascii);
  return 0;
}

static size_t write_data(void* buffer,size_t size,size_t nmemb,void *userdata)
{
	send_data_info *p_send_data = (send_data_info *)userdata;
	memcpy(p_send_data->send_data+p_send_data->send_len, buffer, size*nmemb); //最后一个字符是换行符
	p_send_data->send_len += size*nmemb;
	
	return size*nmemb;
}

int curl_send(char *http_url, char *send_buf, int send_len)
{
	CURL *curl;
	CURLcode res;
	struct data config;
	config.trace_ascii = 1; /* enable ascii tracing */
	send_data_info t_send_data;
	

	curl = curl_easy_init();
	if(!curl)
	{
		syslog(LOG_USER|LOG_INFO, "curl_easy_init error.");
		return -1;
	}
	printf("http_url:[%s]\n", http_url);

//	curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
//	curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl,CURLOPT_URL,http_url); 
//	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POSTFIELDS);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
#if 0
	curl_easy_setopt(curl,CURLOPT_READFUNCTION,read_data); 
	curl_easy_setopt(curl,CURLOPT_READDATA,(void *)&t_send_data);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,t_send_data.send_len);
#else
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,send_buf);
#endif

	memset(&t_send_data, 0, sizeof(send_data_info));
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void *)&t_send_data);

	curl_easy_setopt(curl,CURLOPT_POST,1); 

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		switch(res)
		{
			case CURLE_UNSUPPORTED_PROTOCOL:
				syslog(LOG_USER|LOG_INFO,"do not support protocol");
			case CURLE_COULDNT_CONNECT:
				syslog(LOG_USER|LOG_INFO,"can not connect host %s", http_url);
			case CURLE_HTTP_RETURNED_ERROR:
				syslog(LOG_USER|LOG_INFO,"http return error.");
			case CURLE_READ_ERROR:
				syslog(LOG_USER|LOG_INFO,"read file error");
			default:
				syslog(LOG_USER|LOG_INFO,"res:[%d]",res);
   		}
		return -1;
	}
	if(t_send_data.send_len < 1024)
	{
		printf("send_data:[%s]\n", t_send_data.send_data);
	}
	printf("send_len:[%d]\n", t_send_data.send_len);

	curl_easy_cleanup(curl);
	
	return 0;
}



int curl_https_send(char *https_url, char *send_buf, int send_len, char *recv_buf, int *recv_len, char *gopan_box_sn)
{
	CURL *curl;
	CURLcode res;
	struct data config;
	config.trace_ascii = 1; /* enable ascii tracing */
	send_data_info t_send_data;
	struct curl_slist *http_header = NULL;
	char cert_name[256];
	char tmp_buf[256];

	curl = curl_easy_init();
	if(!curl)
	{
		syslog(LOG_USER|LOG_INFO, "curl_easy_init error.");
		return -1;
	}
	printf("https_url:[%s]\n", https_url);

	//curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, my_trace);
	//curl_easy_setopt(curl, CURLOPT_DEBUGDATA, &config);
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl,CURLOPT_URL,https_url); 
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
#if 0
	curl_easy_setopt(curl,CURLOPT_READFUNCTION,read_data); 
	curl_easy_setopt(curl,CURLOPT_READDATA,(void *)&t_send_data);
	curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,t_send_data.send_len);
#else
	curl_easy_setopt(curl,CURLOPT_POSTFIELDS,send_buf);
#endif

	memset(cert_name, 0, sizeof(cert_name));
	sprintf(cert_name, "%s/etc/%s", GOFUN_HOME_DIR, HTTPS_CERT_FILE);
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0L);
//	curl_easy_setopt(curl,CURLOPT_CAPATH, cert_name);
//	curl_easy_setopt(curl,CURLOPT_CAINFO, cert_name);
	curl_easy_setopt(curl,CURLOPT_SSLCERT,cert_name);
	curl_easy_setopt(curl,CURLOPT_SSLCERTTYPE,"PEM"); 
	curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0L);  

	http_header = curl_slist_append(http_header, "Content-Type: application/json");
	http_header = curl_slist_append(http_header, userAgent);
	http_header = curl_slist_append(http_header, di);
	
	memset(tmp_buf, 0, sizeof(tmp_buf));
	sprintf(tmp_buf, "sn:%s", gopan_box_sn);
	http_header = curl_slist_append(http_header, tmp_buf);
	
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, http_header);

	memset(&t_send_data, 0, sizeof(send_data_info));
	curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,write_data);
	curl_easy_setopt(curl,CURLOPT_WRITEDATA,(void *)&t_send_data);

	curl_easy_setopt(curl,CURLOPT_POST,1); 

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		switch(res)
		{
			case CURLE_UNSUPPORTED_PROTOCOL:
				syslog(LOG_USER|LOG_INFO,"do not support protocol");
			case CURLE_COULDNT_CONNECT:
				syslog(LOG_USER|LOG_INFO,"can not connect host %s", https_url);
			case CURLE_HTTP_RETURNED_ERROR:
				syslog(LOG_USER|LOG_INFO,"http return error.");
			case CURLE_READ_ERROR:
				syslog(LOG_USER|LOG_INFO,"read file error");
			default:
				syslog(LOG_USER|LOG_INFO,"res:[%d]",res);
   		}
   	printf("https return res:[%d]\n", res);
		return -1;
	}
	if(t_send_data.send_len < 1024)
	{
		printf("send_data:[%s]\n", t_send_data.send_data);
	}
	printf("send_len:[%d]\n", t_send_data.send_len);
	
	memcpy(recv_buf, t_send_data.send_data, t_send_data.send_len);
	*recv_len = t_send_data.send_len;

	curl_slist_free_all(http_header);
	curl_easy_cleanup(curl);
	
	return 0;
}
