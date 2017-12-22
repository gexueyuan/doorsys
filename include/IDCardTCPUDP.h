//
//  IDCardSocket.h
//  TDRToken
//
//  Created by zhaowei on 16/5/5.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardSocketTCPUDP_h
#define IDCardSocketTCPUDP_h

#include "IDCardInclude.h"
#include "IDCardConfig.h"
#include "IDCardTime.h"
#include "IDCardInterface.h"

#ifndef _WIN32
#define INVALID_SOCKET            (-1)
#endif

namespace IDCARD {
    class CSocketTCPUDP : public ISocket
    {
    public:
        CSocketTCPUDP():m_tcpHandle(INVALID_SOCKET),m_udpHandle(INVALID_SOCKET)
        {
            memset(m_ipBuffer, 0, sizeof(m_ipBuffer));
			m_port = 0;
			m_netType = NET_TYPE_TCP_UDP;

#ifdef _WIN32
			WSADATA wsaData;
			::WSAStartup(WINSOCK_VERSION , &wsaData);
#endif

        }

        ~CSocketTCPUDP()
        {
            DisconnectServer();
#ifdef _WIN32
			WSACleanup();
#endif
        }

		virtual void    setTimeOut(UINT32 u32ConnTime,UINT32 u32SRTime)
		{
			m_connTimeOut = u32ConnTime;
			m_recvTimeOut = u32SRTime;

		}
		virtual void    setConnParam(const char * pszHostIP, int nPort)
		{
			strcpy(m_ipBuffer, pszHostIP);
			m_port = nPort;
		}
        
        void setSocketNonBlock(int sd)
        {
#ifdef _WIN32
            int flag = 1;
            ioctlsocket(sd, FIONBIO, (unsigned long *) &flag);
#else
            int x;
            x=fcntl(sd,F_GETFL,0);
            fcntl(sd,F_SETFL, x | O_NONBLOCK);//设置非阻塞
#endif
        }
        
        void setSocketBlock(int sd)
        {
#ifdef _WIN32
            int flag = 0;
            ioctlsocket(sd, FIONBIO, (unsigned long *) &flag);
#else
            int flags = fcntl(sd,F_GETFL,0);
            flags &= ~O_NONBLOCK;
            fcntl(sd,F_SETFL,flags);
#endif
        }
        
        
        virtual HRESULT ConnectServer()
        {
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "TCPUDP ConnectServer");
			HRESULT hr = AKEY_RV_OK;
			hr = ConnectTCP();
			if (hr != AKEY_RV_OK) {
				m_tcpHandle = INVALID_SOCKET;
			}
			ConnectUDP();
            return AKEY_RV_OK;
                
        }
            
         virtual   HRESULT DisconnectServer()
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "DisconnectServer");
                if (m_tcpHandle != INVALID_SOCKET)
                {
                    LGNTRACE_MSG("close m_socketHandle:%d",m_tcpHandle);
					closeHanlde(m_tcpHandle);
                    m_tcpHandle = INVALID_SOCKET;
                }
				if (m_udpHandle != INVALID_SOCKET)
				{
					closeHanlde(m_udpHandle);
					m_udpHandle = INVALID_SOCKET;
				}
				
                return AKEY_RV_OK;
            }

            HRESULT Write(CBuffer& sendBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "TCPUDP Socket Write");
				HRESULT hr = AKEY_RV_OK;
				if (m_tcpHandle != INVALID_SOCKET)
				{
					hr = writeTCP(sendBuf);
					if (hr != AKEY_RV_OK) {
						return hr;
					}
					
				}
				if (m_udpHandle != INVALID_SOCKET)
				{
					hr = writeUDP(sendBuf);
					if (hr != AKEY_RV_OK) {
						return hr;
					}
				}
                return AKEY_RV_OK;
            }
            
            HRESULT Read(CBuffer& recvBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Socket Read");
				long tv_sec = m_recvTimeOut/1000;
				long tv_usec = (m_recvTimeOut - (tv_sec*1000))*1000;
                struct timeval timeout;
                timeout.tv_sec  = tv_sec;
                timeout.tv_usec = tv_usec;
				HRESULT hr = AKEY_RV_OK;
                fd_set rds;
                FD_ZERO(&rds);
				if (m_tcpHandle != INVALID_SOCKET) {
					 FD_SET(m_tcpHandle, &rds);
				}
				if (m_udpHandle != INVALID_SOCKET) {
					FD_SET(m_udpHandle, &rds);
				}
				int maxFd = (m_tcpHandle > m_udpHandle)?m_tcpHandle:m_udpHandle;
                switch (select(maxFd + 1, &rds, NULL, NULL, &timeout))
                {
                    case -1:
                    {
                        LGNTRACE_MSG("select -1 error :%d",errno);
                        return AKEY_RV_SERVER_RECV_ERROR;
                    }
                        break;
                    case 0:
                    {
                        LGNTRACE_MSG("select 0 error :%d",errno);
                        return AKEY_RV_SERVER_RECV_TIMEOUT;
                    }
                        break;
                    default:
                        if (FD_ISSET(m_tcpHandle,&rds)) {
                            hr = recvWithHandle(m_tcpHandle,recvBuf);
                            return hr;
                        }
						if (FD_ISSET(m_udpHandle,&rds)) {
							hr = recvWithHandle(m_udpHandle,recvBuf);
							return hr;
						}
                        break;
                        
                }
                return AKEY_RV_SERVER_RECV_ERROR;
            }
            
            virtual HRESULT SendAndRecv(CBuffer& sendBuf,CBuffer& recvBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "SendAndRecv");
                HRESULT hr = AKEY_RV_OK;
                hr = Write(sendBuf);
                if (hr != AKEY_RV_OK) {
                    LGNTRACE_MSG("TCP write err:%x",hr);
                    return hr;
                }
                hr = Read(recvBuf);
                return hr;
            }
            
            
            //继承，完成socket重连
            virtual HRESULT OnSendAndRecvFail()
            {
                return AKEY_RV_OK;
            }
            
            virtual bool IsSocketValid()
            {
                return ((m_tcpHandle != INVALID_SOCKET) || 
					m_udpHandle != INVALID_SOCKET);
            }

			virtual IDCARD_NET_TYPE GetNetType()
			{
				return m_netType;
			}

			virtual const char* GetIp()
			{
				return m_ipBuffer;
			}

			virtual int GetPort()
			{
				return m_port;
			}

		protected:
			void setConnAndSRTimeOut(int sd)
			{
#ifdef _WIN32 // 用于win下代码测试
				::setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&m_recvTimeOut, sizeof(m_recvTimeOut));
				::setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&m_recvTimeOut, sizeof(m_recvTimeOut));
#else
				long tv_sec = m_recvTimeOut/1000;
				long tv_usec = (m_recvTimeOut - (tv_sec*1000))*1000;
				struct timeval timeout =
				{ tv_sec, tv_usec }; 
				setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (void *) &timeout,
					sizeof(timeout));
				setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, (void *) &timeout,
					sizeof(timeout));
#endif
			}

			HRESULT ConnectTCP()
			{
				LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "TCP ConnectServer");
				if (strlen(m_ipBuffer) == 0 || m_port == 0) {
					LGNTRACE_MSG("TCP Connect Param Error");
					return AKEY_RV_SOCKET_IP_PORT_PARAM_ERROR;
				}
				int sd = (int) socket(AF_INET,SOCK_STREAM, 0);
				if (sd == INVALID_SOCKET)
				{
					return AKEY_RV_SERVER_CONNECT_ERROR;
				}
# if(SUPPORT_UDP)
				m_socketUDPHandle = socket(AF_INET,SOCK_DGRAM, 0);
#endif
				//设置为非阻塞
				setSocketNonBlock(sd);
				struct sockaddr_in sa;
				memset(&sa, 0, sizeof(sa));
				sa.sin_family = AF_INET;
				sa.sin_addr.s_addr = inet_addr(m_ipBuffer); /* Server IP */
				sa.sin_port = htons(m_port); /* Server Port number */
				if (connect(sd, (struct sockaddr*) &sa, sizeof(sa)) == INVALID_SOCKET)
				{
					bool ret;

#ifdef _WIN32
					if (WSAGetLastError() == WSAEWOULDBLOCK){// it is in the connect process
#else
					if (errno == EINPROGRESS){// it is in the connect process
#endif
						struct timeval tv;
						fd_set writefds;
						int error;
						long tv_sec = m_connTimeOut/1000;
						long tv_usec = (m_connTimeOut - (tv_sec*1000))*1000;
						tv.tv_sec = tv_sec;
						tv.tv_usec = tv_usec;
						FD_ZERO(&writefds);
						FD_SET(sd, &writefds);
						if(select(sd+1,NULL,&writefds,NULL,&tv)>0){
							int len=sizeof(int);
							//下面的一句一定要，主要针对防火墙
#ifdef _WIN32
							getsockopt(sd, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t *)&len);
#else
							getsockopt(sd, SOL_SOCKET, SO_ERROR, (void*)&error, (socklen_t *)&len);
#endif
							if(error==0) {
								ret=true;
							}
							else {
								ret=false;
							}
						}else {
							ret=false;
						}//timeout or error happen
					}else{
						ret=false;
					}
					if (ret == false) {
						return AKEY_RV_SERVER_CONNECT_ERROR;
					}
				}
				setSocketBlock(sd);
				DisconnectServer();
				setConnAndSRTimeOut(sd);                
#if  (TARGET_OS_IOS)
				int value = 1;
				setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *) &value,
					sizeof(value));
#endif

				m_tcpHandle = sd;
				return AKEY_RV_OK;
			}

			HRESULT ConnectUDP()
			{
				HRESULT hr = AKEY_RV_OK;
				LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP ConnectServer");
				if (strlen(m_ipBuffer) == 0 || m_port == 0) {
					return AKEY_RV_SOCKET_IP_PORT_PARAM_ERROR;
				}
				int sd = socket(AF_INET, SOCK_DGRAM, 0);
				if (sd == INVALID_SOCKET) { /* calls socket() */
					LGNTRACE_MSG("socket err:%x",AKEY_RV_SERVER_CONNECT_ERROR);
					return AKEY_RV_SERVER_CONNECT_ERROR;
				}
				m_udpHandle = sd;
				memset(&m_sa,0,sizeof(m_sa));
				m_sa.sin_family = AF_INET;
				m_sa.sin_addr.s_addr = inet_addr(m_ipBuffer); /* Server IP */
				m_sa.sin_port = htons(m_port); /* Server Port number */
				setConnAndSRTimeOut(m_udpHandle);
				return AKEY_RV_OK;
			}

			void closeHanlde(int sd)
			{
#ifdef WIN32
				shutdown(sd, 2);
				closesocket(sd);
#else
				shutdown(sd, 2);
				close(sd);
#endif
			}

			HRESULT writeTCP(CBuffer& sendBuf)
			{
				LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Socket Write");
				if (m_tcpHandle == INVALID_SOCKET) {
					return AKEY_RV_FAIL;
				}
				int iSendCnt = sendBuf.GetLength();
				LPBYTE pbSend = sendBuf.GetBuffer();
				while (iSendCnt > 0)
				{
					int iSendSz = send(m_tcpHandle, (const char *)pbSend, iSendCnt, 0);
					if (iSendSz <= 0)
					{
						return AKEY_RV_SERVER_SEND_ERROR;
					}
					pbSend += iSendSz;
					iSendCnt -= iSendSz;
				}
				LGNTRACE_HEX("Socket Send:", sendBuf.GetBuffer(), sendBuf.GetLength());
				return AKEY_RV_OK;
			}

			HRESULT writeUDP(CBuffer& sendBuf)
			{
				LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP Socket Write");
				if (m_udpHandle == INVALID_SOCKET) {
					return AKEY_RV_FAIL;
				}
				int iSendCnt = sendBuf.GetLength();
				LPBYTE pbSend = sendBuf.GetBuffer();
				int addr_len = sizeof(struct sockaddr_in);
				int iSendSz = sendto(m_udpHandle, (const char*)sendBuf.GetBuffer(),sendBuf.GetLength(), 0, (struct sockaddr *)&m_sa, addr_len);
				if (iSendSz != iSendCnt)
				{
					return AKEY_RV_SERVER_SEND_ERROR;
				}
				LGNTRACE_HEX("UDP Socket Send:", sendBuf.GetBuffer(), sendBuf.GetLength());
				return AKEY_RV_OK;
			}

			HRESULT recvWithHandle(int sd,CBuffer& recvBuf)
			{
				LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Socket recvWithHandle");
				HRESULT hr = AKEY_RV_OK;
				UINT32 unRecvBufLen = 2*DEFAULT_BUFFERLEN;
				LPBYTE pbRecvBuf = recvBuf.ReAllocBytes(unRecvBufLen);
				UINT32 unDataLen_t = 0;
				for (int offset = 0; offset < (int) (4+ unDataLen_t);)
				{
					int recvLen = recv(sd, (char*) (pbRecvBuf + offset),
						unRecvBufLen - offset, 0);
					if (recvLen <= 0)
					{
						LGNTRACE_MSG("recv error :%d",errno);
						return AKEY_RV_SERVER_RECV_ERROR;
					}
					if ((offset < 4) && ((offset + recvLen) >= 4))
					{
						unDataLen_t = Helper::BigEndian::UInt32FromBytes(pbRecvBuf);
						if (unDataLen_t + 4 > 2*DEFAULT_BUFFERLEN) {
							unRecvBufLen = unDataLen_t + 4;
							recvBuf.SetLength(offset + recvLen);
							pbRecvBuf = recvBuf.ReAllocBytes(unRecvBufLen);
						}
					}
					offset += recvLen;
				}
				if (pbRecvBuf)
				{
					memmove(pbRecvBuf, pbRecvBuf + 4 , unDataLen_t );
				}
				recvBuf.SetLength(unDataLen_t);
				LGNTRACE_HEX("Socket Recv:", pbRecvBuf , unDataLen_t);
				return AKEY_RV_OK;
			}
            
        private:
            int m_tcpHandle;
			int m_udpHandle;
            char m_ipBuffer[50];
            int m_port;
            int m_connTimeOut; //连接超时时间，单位毫秒
            int m_recvTimeOut; //发送和接收数据超时，单位毫秒
			IDCARD_NET_TYPE m_netType;
			struct sockaddr_in m_sa;
        };
};


#endif /* IDCardSocket_h */
