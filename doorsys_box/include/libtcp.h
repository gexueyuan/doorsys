#ifndef _LIB_TCP_H_
#define _LIB_TCP_H_

#define iSHORTLEN   2
#define iINTLEN     4

#define MSG_MAXLEN   10240

extern int tcp_server(int port);
extern int udp_server(int port);
extern int tcp_accept(int sockfd,char *client_ip, int *client_port);
extern int tcp_read(int type,int sockfd,char *p_readbuf,int max_len,int *p_readlen,int sec_time, int mic_time);
extern int tcp_read_poll(int type,int sockfd,char *p_readbuf,int max_len,int *p_readlen,int mill_time);
extern int tcp_write(int type,int sockfd, char *send_buf, int send_len);
extern int tcp_connect(char *svr_ip, int svr_port);
extern int tcp_no_block_connect(char *svr_ip, int svr_port);
extern int tcp_get_local_ip(char *outip, char *ethname);
extern int tcp_get_local_mac(char *outMac, char *ethname);

#endif
