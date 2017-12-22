#ifndef _LIB_ZMQ_TOOLS_H_
#define _LIB_ZMQ_TOOLS_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern void *zmq_socket_new_sub(void *context,char *dest);
extern void *zmq_socket_new_pub(void *context,char *dest);

extern void *zmq_socket_new_rep(void *context,char *dest);
extern void *zmq_socket_new_req(void *context,char *dest);

extern void *zmq_socket_new_router(void *context,char *dest);
extern void *zmq_socket_new_dealer(void *context,char *dest);
extern void *zmq_socket_new_dealer_identity(void *context,char *dest, char *identity);
extern void *zmq_socket_new_dealer_svr(void *context,char *dest);

extern void *zmq_socket_new_pull(void *context,char *dest);
extern void *zmq_socket_new_push(void *context,char *dest);


extern int zmq_poll_recv(void *socket, char *pBuff, int iMaxLen, int iTimeOut);
extern int zmq_socket_send(void *socket, char *pBuff, int iSendLen);

#ifdef __cplusplus
}
#endif

#endif
