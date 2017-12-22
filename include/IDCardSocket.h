//
//  IDCardSocket.h
//  TDRToken
//
//  Created by zhaowei on 16/5/5.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardSocket_h
#define IDCardSocket_h

#include "IDCardInclude.h"
#include "IDCardConfig.h"
#include "IDCardTime.h"
#include "IDCardInterface.h"

#ifndef _WIN32
#define INVALID_SOCKET            (-1)
#endif

namespace IDCARD {
    class CSocketTCP : public ISocket
    {
    public:
        CSocketTCP():m_socketHandle(INVALID_SOCKET)
        {
            memset(m_ipBuffer, 0, sizeof(m_ipBuffer));
			m_port = 0;
			m_netType = NET_TYPE_TCP;

#ifdef WIN32
			WSADATA wsaData;
			::WSAStartup(WINSOCK_VERSION , &wsaData);
#endif

        }

        ~CSocketTCP()
        {
            if (m_socketHandle != INVALID_SOCKET) {
                shutdown(m_socketHandle, 2);
#ifdef WIN32
				closesocket(m_socketHandle);
#else
				close(m_socketHandle);
#endif
                m_socketHandle = INVALID_SOCKET;
            }
#ifdef WIN32
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
#ifdef WIN32
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
#ifdef WIN32
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
                
#ifdef WIN32
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
#ifdef WIN32
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
						CTime::SleepTime(200);//连接失败，休眠200毫秒
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
                LGNTRACE_MSG("NetType :%d TCP ip:%s port:%d",m_netType,m_ipBuffer,m_port);
                m_socketHandle = sd;
                return AKEY_RV_OK;
                
            }
            
            HRESULT DisconnectServer()
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "DisconnectServer");
                if (m_socketHandle != INVALID_SOCKET)
                {
                    LGNTRACE_MSG("close m_socketHandle:%d",m_socketHandle);
#ifdef WIN32
					shutdown(m_socketHandle, 2);
                    closesocket(m_socketHandle);
#else
                    shutdown(m_socketHandle, 2);
                    close(m_socketHandle);
#endif
                    m_socketHandle = INVALID_SOCKET;
                }
                return AKEY_RV_OK;
            }
            
            HRESULT Write(CBuffer& sendBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Socket Write");
                if (m_socketHandle == INVALID_SOCKET) {
                    return AKEY_RV_FAIL;
                }
                int iSendCnt = sendBuf.GetLength();
                LPBYTE pbSend = sendBuf.GetBuffer();
                while (iSendCnt > 0)
                {
                    int iSendSz = send(m_socketHandle, (const char *)pbSend, iSendCnt, 0);
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
            
            HRESULT Read(CBuffer& recvBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Socket Read");
				long tv_sec = m_recvTimeOut/1000;
				long tv_usec = (m_recvTimeOut - (tv_sec*1000))*1000;
                struct timeval timeout;
                timeout.tv_sec  = tv_sec;
                timeout.tv_usec = tv_usec;
				UINT32 unRecvBufLen = DEFAULT_BUFFERLEN;
                LPBYTE pbRecvBuf = recvBuf.ReAllocBytes(unRecvBufLen);
                fd_set rds;
                FD_ZERO(&rds);
                FD_SET(m_socketHandle, &rds);
                
                switch (select(m_socketHandle + 1, &rds, NULL, NULL, &timeout))
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
                        if (FD_ISSET(m_socketHandle,&rds)) {
							UINT32 unDataLen_t = 0;
                            for (int offset = 0; offset < (int) (4+ unDataLen_t);)
                            {
								int recvLen = recv(m_socketHandle, (char*) (pbRecvBuf + offset),
									unRecvBufLen - offset, 0);
                                if (recvLen <= 0)
                                {
                                    LGNTRACE_MSG("recv error :%d",errno);
                                    return AKEY_RV_SERVER_RECV_ERROR;
                                }
                                if ((offset < 4) && ((offset + recvLen) >= 4))
                                {
                                    unDataLen_t = Helper::BigEndian::UInt32FromBytes(pbRecvBuf);
                                    if (unDataLen_t + 4 > DEFAULT_BUFFERLEN) {
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
                            LGNTRACE_HEX("Socket Recv:", pbRecvBuf, unDataLen_t);
                            return AKEY_RV_OK;
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
                return (m_socketHandle != INVALID_SOCKET);
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
#ifdef WIN32 // 用于win下代码测试
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
            
        private:
            int m_socketHandle;
            char m_ipBuffer[50];
            int m_port;
            int m_connTimeOut; //连接超时时间，单位毫秒
            int m_recvTimeOut; //发送和接收数据超时，单位毫秒
			IDCARD_NET_TYPE m_netType;
        };
}


#endif /* IDCardSocket_h */
