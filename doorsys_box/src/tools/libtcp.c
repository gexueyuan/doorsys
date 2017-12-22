#include "global_sys.h"
#include "liblog.h"
#include "libtcp.h"


#ifndef WIN32
#define socket_read read
#define socket_write write
#define socket_close close
#else
#define socket_read(s,buf,len) recv(s, buf, len, 0)
#define socket_write(s,buf,len) send(s, buf, len, 0)
#define socket_close closesocket
#endif


int tcp_server(int port){
	int sockfd;
	struct sockaddr_in local_addr;
	int optval;

	memset((char *)&local_addr, 0, sizeof(local_addr));
	
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		LOG_PRINT(L_ERR,"create socket fail,errno=[%d]",errno);
		return -1;
	}
	
	optval = 1;
	if(setsockopt(sockfd , SOL_SOCKET, SO_REUSEADDR,( char *)&optval, sizeof( optval ) ) < 0 ){
		LOG_PRINT(L_ERR,"set socket SO_REUSEADDR error,errno=[%d]",errno);
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port	= htons(port);
	
	if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0){
		LOG_PRINT(L_ERR,"bind socket fail,errno=[%d]\n",errno);
		if (errno==EADDRINUSE){
			LOG_PRINT(L_ERR,"Address already in use\n");
		}
		socket_close(sockfd);
		return -1;
	}

	if (listen(sockfd, 1000) < 0){
		LOG_PRINT(L_ERR,"listen socket fail,errno=[%d]",errno);
		socket_close(sockfd);
		return -1;
	}
	return sockfd;
}

int udp_server(int port){
	int sockfd;
	struct sockaddr_in local_addr;
	int optval;
	
	memset((char *)&local_addr, 0, sizeof(local_addr));
	
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		LOG_PRINT(L_ERR,"create socket fail,errno=[%d]\n",errno);
		return -1;
	}
	
	
	optval = 1;
	if(setsockopt(sockfd , SOL_SOCKET, SO_REUSEADDR,( char *)&optval, sizeof( optval ) ) < 0 ){
		LOG_PRINT(L_ERR," set socket SO_REUSEADDR error,errno=[%d]\n",errno);
	}

	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port	= htons(port);
	
	if (bind(sockfd, (struct sockaddr *)&local_addr, sizeof(local_addr)) < 0){
		LOG_PRINT(L_ERR,"bind socket fail,errno=[%d]\n",errno);
		if (errno==EADDRINUSE){
			LOG_PRINT(L_ERR,"Address already in use\n");
		}
		socket_close(sockfd);
		return -1;
	}
	
	return sockfd;
}

int tcp_accept(int sockfd,char *client_ip, int *client_port){
	socklen_t len;
	
	int newsockfd = -1;
	struct sockaddr_in addr;
	
	memset((char *) &addr, 0, sizeof(struct sockaddr_in));
	len = sizeof(struct sockaddr_in);
	
	newsockfd = accept(sockfd, (struct sockaddr *)&addr, &len);
	if (newsockfd < 0){
		LOG_PRINT(L_ERR,"errno[%d]",errno);
		return -1;
	}
	strcpy(client_ip , inet_ntoa(((struct sockaddr_in *)&addr)->sin_addr));
	*client_port = ntohs(((struct sockaddr_in *)&addr)->sin_port);
	
	return newsockfd;
}

static int _read_fixed_length_data(int sockfd, char * buffer, int length)
{
	int i, readlen;
	for (i=0; i<length; ){
		readlen = socket_read(sockfd, buffer+i, length-i);
		if (readlen < 0){
			if (errno == EINTR)
				continue;
			else{
				LOG_PRINT(L_ERR,"readlen =[%d] error=[%s]", readlen, strerror(errno));
				return -1;
			}
		}
		else{
			if (readlen == 0 ){
				//client close socket
				//LOG_PRINT(L_ERR,"ret =[%d] error=[%s]", readlen, strerror(errno));
				return i; // 0
			}
			i+= readlen;
		}
	}
	return length;
	
}

static int _read_fixed_length_data_poll(int sockfd, char * buffer, int length, int timeout)
{
	int rc;
	int i, readlen;
	
#ifndef WIN32	

	struct pollfd  pfds[1];
	nfds_t npfds;

	npfds = 1;
	pfds[0].fd = sockfd;
	pfds[0].events = POLLIN;
#else
	fd_set fds;
	struct timeval  st_timeout;
	st_timeout.tv_sec = 0;
	st_timeout.tv_usec = timeout*1000;  //微妙
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);

	if (timeout > 0){
		rc = select(sockfd+1, &fds, NULL, NULL, &st_timeout);
		if (rc == -1){
			LOG_PRINT(L_ERR,"tcp_read select error.[%d]",rc);
			return -1;
		}
		else if(rc == 0){
			LOG_PRINT(L_INFO,"tcp_read timeout[%d]",rc);
			return -1;
		}
	}
#endif

#ifndef WIN32
	rc = poll(pfds, npfds, timeout); //单位：毫秒
	if(rc < 0)
	{
		LOG_PRINT(L_ERR,"tcp_read_poll poll error.rc:[%d]", rc);
		return -1;
	}
	if(rc == 0)
	{
		LOG_PRINT(L_ERR,"tcp_read_poll poll timeout.timeout:[%d]", timeout);
		return -1;
	}
	if(POLLERR & pfds[0].revents)
	{
		LOG_PRINT(L_ERR,"tcp_read_poll poll error.");
		return -1;
	}
	if(POLLIN & pfds[0].revents)
	{
#endif
		for (i=0; i<length; ){
			readlen = socket_read(sockfd, buffer+i, length-i);
			if (readlen < 0){
				if (errno == EINTR)
					continue;
				else{
					LOG_PRINT(L_ERR,"readlen =[%d] error=[%s]", readlen, strerror(errno));
					return -1;
				}
			}
			else{
				if (readlen == 0 ){
					//client close socket
					//LOG_PRINT(L_ERR,"ret =[%d] error=[%s]", readlen, strerror(errno));
					return i; // 0
				}
				i+= readlen;
			}
			
		}
#ifndef WIN32
	}
#endif
	return length;
}

int tcp_read(int type,int sockfd,char *p_readbuf,int max_len,int *p_readlen,int sec_time, int mic_time)
{
	int rc = 0;
	char  msghead_buf[10];
	int   readlen;
	unsigned int msg_len;

	fd_set fds;
	struct timeval  st_timeout;
	st_timeout.tv_sec = sec_time;
	st_timeout.tv_usec = mic_time*1000;  //微妙
	FD_ZERO(&fds);
	FD_SET(sockfd, &fds);
	
	if (sec_time != 0 || mic_time != 0){
		rc = select(sockfd+1, &fds, NULL, NULL, &st_timeout);
		if (rc == -1){
			LOG_PRINT(L_ERR,"tcp_read select error.[%d]",rc);
			return -1;
		}
		else if(rc == 0){
			LOG_PRINT(L_INFO,"tcp_read timeout[%d]",rc);
			return -1;
		}
	}

	memset(msghead_buf, 0x00, sizeof(msghead_buf));
	switch (type){
		case iSHORTLEN:
			readlen = _read_fixed_length_data(sockfd, msghead_buf, 2);
			if (readlen < 2){
				return -1;
			}

			msg_len = ntohs(*((uint16_t*)msghead_buf));
			break;
		case iINTLEN:			
			readlen = _read_fixed_length_data(sockfd, msghead_buf, 4);
			if (readlen < 4){
				return -1;
			}

			msg_len = ntohl(*((uint32_t*)msghead_buf));
			break;
	}

	if (msg_len > (unsigned int)max_len){
		LOG_PRINT(L_ERR,"msg_len =[%d] over!", msg_len);
		return -2;
	}

	readlen = _read_fixed_length_data(sockfd, p_readbuf, msg_len);
	if (readlen < (int)msg_len){
		return -1;
	}
	*p_readlen = readlen;
	return readlen;
}

int tcp_read_poll(int type,int sockfd,char *p_readbuf,int max_len,int *p_readlen,int mill_time)  //毫秒
{
	int rc = 0;

	char  msghead_buf[10];
	int   readlen;
	unsigned int msg_len;

	memset(msghead_buf, 0x00, sizeof(msghead_buf));
	switch (type){
		case iSHORTLEN:
			readlen = _read_fixed_length_data_poll(sockfd, msghead_buf, 2, mill_time);
			if (readlen < 2){
				return -1;
			}

			msg_len = ntohs(*((uint16_t*)msghead_buf));
			break;
		case iINTLEN:			
			readlen = _read_fixed_length_data_poll(sockfd, msghead_buf, 4, mill_time);
			if (readlen < 4){
				return -1;
			}

			msg_len = ntohl(*((uint32_t*)msghead_buf));
			break;
	}

	if (msg_len > (unsigned int)max_len){
		LOG_PRINT(L_ERR,"msg_len =[%d] over!", msg_len);
		return -2;
	}

	readlen = _read_fixed_length_data_poll(sockfd, p_readbuf, msg_len, mill_time);
	if (readlen < (int)msg_len){
		return -1;
	}
	*p_readlen = readlen;
	return readlen;

}


int tcp_write(int type,int sockfd, char *p_send_buf, int send_len)
{
	char msghead_buf[10];
	char send_buf[MSG_MAXLEN];
	int  posi;
	int  tmp_len;
	int  ret_len;
	unsigned short msghead_len;
	int msghead_len4;
	
	memset(msghead_buf,0x00,sizeof(msghead_buf));
	memset(send_buf,0x00,sizeof(send_buf));
	switch(type){
		case iSHORTLEN:
			msghead_len = htons(send_len);
			memcpy(send_buf,&msghead_len,sizeof(short));
			memcpy(send_buf+sizeof(short),p_send_buf,send_len);
			send_len += sizeof(short);
			break;
		case iINTLEN:
			msghead_len4 = htonl(send_len);
			memcpy(send_buf, &msghead_len4, sizeof(int));
			memcpy(send_buf+sizeof(int), p_send_buf, send_len);
			send_len += sizeof(int);
			break;
	}
	
	posi = 0;
	tmp_len = 0;
	do{
		if(send_len > 4096){
			posi = 4096;
		}
		else{
			posi = send_len;
		}
		ret_len = socket_write(sockfd, send_buf+tmp_len,posi);
		if (ret_len < 0 ){
			if ( errno == EINTR ){
				continue;
			}
			else{
				LOG_PRINT(L_ERR,"ret =[%d] error=[%d]", ret_len,errno);
				return -1;
			}
		}
		send_len -= ret_len;
		tmp_len += ret_len;
	}while (send_len > 0);
	return tmp_len;
}


int tcp_connect(char *svr_ip, int svr_port)
{
	int sockfd = 0;
	struct sockaddr_in addr;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(svr_port);

	if((addr.sin_addr.s_addr = inet_addr(svr_ip)) == INADDR_NONE) 
	{
		LOG_PRINT(L_ERR,"tcp_connect:inet_addr err.ip[%s] ", svr_ip);
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		LOG_PRINT(L_ERR, "tcp_connect create socket fail");
		return -1;
	}

	if(connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0)
	{
		LOG_PRINT(L_ERR,"svr_ip:[%s],svr_port:[%d]", svr_ip, svr_port);
		LOG_PRINT(L_ERR,"tcp_connect connect socket fail.errno:[%d][%s]", errno, strerror(errno));
		socket_close(sockfd);
		return -1;
	}
	
	return sockfd;
}

//无阻塞
int tcp_no_block_connect(char *svr_ip, int svr_port)
{
	int iRet = 0;
	int sockfd = 0;
	int    iFlag, iErr;
	socklen_t  iLen;
	struct sockaddr_in addr;
	struct timeval     tval;
	fd_set wset;

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(svr_port);

	if((addr.sin_addr.s_addr = inet_addr(svr_ip)) == INADDR_NONE)
	{
		LOG_PRINT(L_ERR,"tcp_no_block_connect inet_addr err.ip[%s] \n", svr_ip);
		return -1;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		LOG_PRINT(L_ERR,"tcp_no_block_connect create socket fail");
		return -1;
	}

#ifndef WIN32
	if ((iFlag = fcntl(sockfd, F_GETFL, 0)) < 0) 
	{	
		close(sockfd);
		return -1;
	}
	if (fcntl(sockfd, F_SETFL, iFlag | O_NONBLOCK) < 0)
	{
		close(sockfd);
		return -1;
	}
#endif
    
	iRet = connect(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
	if(iRet < 0)
	{
		if (errno != EINPROGRESS)
		{
			socket_close(sockfd);
			return -1;
		}
	}
	if(iRet != 0)
	{
		FD_ZERO(&wset);
		FD_SET(sockfd, &wset);
		tval.tv_sec = 2;
		tval.tv_usec = 0;
		iRet = select(sockfd + 1, NULL, &wset, NULL, &tval);
		if (iRet == 0) 
		{
			socket_close(sockfd);
			return -1;
		}
		if (FD_ISSET(sockfd, &wset)) 
		{
			iLen = sizeof(iErr);
			if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&iErr, &iLen) < 0) 
			{
				socket_close(sockfd);
				return -1;
			}
			if (iErr != 0)
			{
				socket_close(sockfd);
				return -1;
			}
		}
		else
		{
			socket_close(sockfd);
			return -1;
		}
	}
#ifndef WIN32
	if (fcntl(sockfd, F_SETFL, iFlag) < 0) 
	{
		socket_close(sockfd);
		return -1;
	}
#endif

    return sockfd;
}

//不指定长度头的读
int tcp_nohead_read_poll(int sockfd,char *p_readbuf,int max_len,int *p_readlen,int mill_time) {//毫秒
	int rc;
	int readlen;
#ifndef WIN32
	struct pollfd  pfds[1];
	nfds_t npfds;

	npfds = 1;
	pfds[0].fd = sockfd;
	pfds[0].events = POLLIN;

	rc = poll(pfds, npfds, mill_time);
	if(rc < 0) {
		LOG_PRINT(L_ERR,"tcp_nohead_read_poll poll error.rc:[%d]", rc);
		return -1;
	}
	if(rc == 0){
		LOG_PRINT(L_ERR,"tcp_nohead_read_poll poll timeout.timeout:[%d]", mill_time);
		return -1;
	}
	if(POLLIN & pfds[0].revents){
#endif
		rc = recv(sockfd, p_readbuf, max_len,0);
		if(rc <= 0)
		{
			LOG_PRINT(L_INFO, "tcp_nohead_read_poll recv error.rc[%d]", rc);
			return -1;
		}
		*p_readlen = rc;

		return rc;

#ifndef WIN32
	}
#endif

	return 0;
}

int tcp_get_local_ip(char *outip, char *ethname)
{
#ifndef WIN32
	int sock;
	struct sockaddr_in sin;
	struct ifreq ifr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == -1)
	{
		LOG_PRINT(L_ERR,"socket");
		return -1;	
	}

	strncpy(ifr.ifr_name, ethname, IFNAMSIZ);
	ifr.ifr_name[IFNAMSIZ - 1] = 0;

	if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
	{
		LOG_PRINT(L_ERR,"ioctl");
		close(sock);
		return -1;
	}

	memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
	strcpy(outip, inet_ntoa(sin.sin_addr));
	//*outip = ntohl(sin.sin_addr.s_addr);

	close(sock);
#endif
	return 0;

}
int tcp_get_local_mac(char *outMac, char *ethname)
{
 #ifndef WIN32
       int sock;
        struct ifreq ifr;

        sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (sock == -1)
        {
                LOG_PRINT(L_ERR,"socket");
                return -1;
        }

        strncpy(ifr.ifr_name, ethname, IFNAMSIZ);
        ifr.ifr_name[IFNAMSIZ - 1] = 0;

        if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
        {
                LOG_PRINT(L_ERR,"ioctl");
		close(sock);
                return -1;
        }
        snprintf(outMac, 18, "%02X:%02X:%02X:%02X:%02X:%02X",
                 (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                 (unsigned char)ifr.ifr_hwaddr.sa_data[5]);


	close(sock);
#endif
        return 0;
}
