//
//  IDCardUDP.h
//  TDRToken
//
//  Created by zhaowei on 2016/8/8.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardUDP_h
#define IDCardUDP_h

#include "IDCardInclude.h"
#include "IDCardConfig.h"
#include "IDCardTime.h"
#include "IDCardInterface.h"

#ifndef _WIN32
#define INVALID_SOCKET            (-1)
#endif

namespace IDCARD {
    
    class CUDPRaw : public ISocket
    {
    public:
        CUDPRaw():m_socketHandle(INVALID_SOCKET) {
            m_connTimeOut = 2;
            m_recvTimeOut = 2;
			m_netType = NET_TYPE_UDP;
        }
        
        ~CUDPRaw(){}
        
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

		virtual bool    IsSocketValid()
		{
			return (m_socketHandle != INVALID_SOCKET);
		}
        
        virtual HRESULT ConnectServer()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP ConnectServer");
			if (strlen(m_ipBuffer) == 0 || m_port == 0) {
				return AKEY_RV_SOCKET_IP_PORT_PARAM_ERROR;
			}
            int sd = socket(AF_INET, SOCK_DGRAM, 0);
            if (sd == INVALID_SOCKET) { /* calls socket() */
                LGNTRACE_MSG("socket err:%x",AKEY_RV_SERVER_CONNECT_ERROR);
                return AKEY_RV_SERVER_CONNECT_ERROR;
            }
            m_socketHandle = sd;
            memset(&m_sa,0,sizeof(m_sa));
            m_sa.sin_family = AF_INET;
            m_sa.sin_addr.s_addr = inet_addr(m_ipBuffer); /* Server IP */
            m_sa.sin_port = htons(m_port); /* Server Port number */
            setConnAndSRTimeOut(m_socketHandle);
            return AKEY_RV_OK;
        }
        
        virtual HRESULT DisconnectServer()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP DisconnectServer");
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
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP Socket Write");
            if (m_socketHandle == INVALID_SOCKET) {
                return AKEY_RV_FAIL;
            }
            int iSendCnt = sendBuf.GetLength();
            LPBYTE pbSend = sendBuf.GetBuffer();
			int addr_len = sizeof(struct sockaddr_in);
            int iSendSz = sendto(m_socketHandle, (const char*)sendBuf.GetBuffer(),sendBuf.GetLength(), 0, (struct sockaddr *)&m_sa, addr_len);
            if (iSendSz != iSendCnt)
            {
                return AKEY_RV_SERVER_SEND_ERROR;
            }
            LGNTRACE_HEX("UDP Socket Send:", sendBuf.GetBuffer(), sendBuf.GetLength());
            return AKEY_RV_OK;
        }
        
        HRESULT Read(CBuffer& recvBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP Socket Read");
            UINT32 unDataLen_t = 0;
			UINT32 unRecvBufLen = 2*DEFAULT_BUFFERLEN;
			LPBYTE pbRecvBuf = recvBuf.ReAllocBytes(unRecvBufLen);
            socklen_t addr_len = sizeof(struct sockaddr_in);
            for (int offset = 0; offset < (int) (4+ unDataLen_t);)
            {
				//struct sockaddr_in addr_t;
				//memset(&addr_t,0, sizeof(addr_t));
				int recvLen = recvfrom(m_socketHandle,(char*) (pbRecvBuf + offset), unRecvBufLen - offset, 0,NULL, NULL);
                if (recvLen <= 0)
                {
#ifdef WIN32
					if( (recvLen == SOCKET_ERROR) && WSAGetLastError()==WSAETIMEDOUT) {
						LGNTRACE_MSG("windows UDP time out");
						return AKEY_RV_SOCKET_COMM_TIMEOUT;
					}
#else 
					if ( (recvLen == -1) && (errno == EWOULDBLOCK)) {  
						LGNTRACE_MSG("UDP time out");
						return AKEY_RV_SOCKET_COMM_TIMEOUT;  
					}

#endif // _DEBUG
                    LGNTRACE_MSG("UDP recv error:%x errno:%d",AKEY_RV_SERVER_RECV_ERROR,errno);
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
            LGNTRACE_HEX("UDP Socket Recv:", recvBuf.GetBuffer(), unDataLen_t);
            return AKEY_RV_OK;
        }
        
        virtual HRESULT SendAndRecv(CBuffer& sendBuf,CBuffer& recvBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP SendAndRecv");
            HRESULT hr = AKEY_RV_OK;
            hr = Write(sendBuf);
            if (hr != AKEY_RV_OK) {
                LGNTRACE_MSG("UDP write err:%x",hr);
                return hr;
            }
            hr = Read(recvBuf);
            return hr;
        }
        
        virtual HRESULT OnSendAndRecvFail()
        {
            return AKEY_RV_OK;
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
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP setConnAndSRTimeOut");
			int rv;
			LGNTRACE_MSG("UDP m_recvTimeOut:%d",m_recvTimeOut);
#ifdef WIN32 //
			rv = ::setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (char *)&m_recvTimeOut, sizeof(m_recvTimeOut));
			if (rv != 0)
			{
				LGNTRACE_MSG("setsockopt SO_RCVTIMEO rv:%d",rv);
			}
			rv = ::setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, (char *)&m_recvTimeOut, sizeof(m_recvTimeOut));
			if (rv != 0)
			{
				LGNTRACE_MSG("setsockopt SO_SNDTIMEO rv:%d",rv);
			}
#else
			long tv_sec = m_recvTimeOut/1000;
			long tv_usec = (m_recvTimeOut - (tv_sec*1000))*1000;
            struct timeval timeout =
            { tv_sec, tv_usec }; // 10S
            rv = setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (void *) &timeout,
                       sizeof(timeout));
			if (rv != 0)
			{
				LGNTRACE_MSG("setsockopt SO_RCVTIMEO rv:%d",rv);
			}
            rv = setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, (void *) &timeout,
                       sizeof(timeout));
			if (rv != 0)
			{
				LGNTRACE_MSG("setsockopt SO_SNDTIMEO rv:%d",rv);
			}
#endif
            
#if  (TARGET_OS_IOS)
            int value = 1;
            setsockopt(sd, SOL_SOCKET, SO_NOSIGPIPE, (void *) &value,
                       sizeof(value));
#endif
        }
        
    private:
        int m_connTimeOut;
        int m_recvTimeOut;
		char m_ipBuffer[50];
		int m_port;
        struct sockaddr_in m_sa;
        int m_socketHandle;
		IDCARD_NET_TYPE m_netType;
    };
}

#endif /* IDCardUDP_h */
