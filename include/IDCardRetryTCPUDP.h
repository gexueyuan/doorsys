//
//  IDCardRetrySocket.h
//  TDRToken
//
//  Created by zhaowei on 2016/8/4.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardRetryTCPUDP_h
#define IDCardRetryTCPUDP_h

#define SOCKET_RETRY_CONNECT 3
#define TCP_RECV_TIMEOUT   2000 //TCP 默认接收超时时间

#include "IDCardInclude.h"

namespace IDCARD {
    class IDCardRetryTCPUDP : public CSocketTCPUDP
    {
    public:
        IDCardRetryTCPUDP(){
			setTimeOut(TCP_RECV_TIMEOUT,TCP_RECV_TIMEOUT);
		}
        ~IDCardRetryTCPUDP(){}

        virtual HRESULT OnSendAndRecvFail()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "TCP OnSendAndRecvFail");
            HRESULT hr = AKEY_RV_OK;
			if (m_time.timeSpend() >= m_u32PerPkgTimeOut)//超时
			{
				LGNTRACE_MSG("CurRetry:%d timeSpend:%d m_u32PerPkgTimeOut:%d",m_u32CurRetryTime,m_time.timeSpend(),m_u32PerPkgTimeOut);
				return AKEY_RV_PACKAGE_COMM_TIMEOUT;//重试次数超出
			}
			if(IDCardUtil::instance()->checkTimeOut() == false){
				return AKEY_RV_IDWORK_TIMEOUT;
			}
            UINT32 u32Cur = CLog::instance()->getClientTCPNetRetryCount();
            LGNTRACE_MSG("TCP Cur:%d",u32Cur);
            CLog::instance()->addClientTCPNetRetryCount();
            int index = 0;
            for (; index<SOCKET_RETRY_CONNECT; index++) {
                LGNTRACE_MSG("TCP Retry Connect :%d",index);
                hr = ConnectServer();
                if (hr == AKEY_RV_OK) {
                    break;
                }else {
                    hr = AKEY_RV_CONNECT_SAM_SERVER_ERROR;
                    DisconnectServer();
                }
            }
            if (index == SOCKET_RETRY_CONNECT) {
                return AKEY_RV_SOCKET_CONN_RETRY_EXCEED;
            }
            return hr;
        }
        
        virtual HRESULT SendAndRecv(CBuffer& sendBuf,CBuffer& recvBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "SendAndRecv");
			if (ParamManager::instance()->getIDWorkIsEnable() == false) {
				LGNTRACE_MSG("User Cancel");
				return AKEY_RV_IDWORK_CANCELED;
			}
            HRESULT hr = AKEY_RV_OK;
			saveUniqueNum(sendBuf);
			getRetryCount();
			m_time.Begin();
			m_u32CurRetryTime = 0;
			int writeFlag = 1;//write tcp-udp:1 udp:2 
            for(;;)
            {
				if (ParamManager::instance()->getIDWorkIsEnable() == false) {
					LGNTRACE_MSG("User Cancel");
					return AKEY_RV_IDWORK_CANCELED;
				}
				if (writeFlag == 1)
				{
					hr = Write(sendBuf);
				}else if(writeFlag == 2) { // udp
					hr = writeUDP(sendBuf);
				}
				if (writeFlag > 0 && hr != AKEY_RV_OK) {
					LGNTRACE_MSG("TCP Write Error hr:%x",hr);
					hr = OnSendAndRecvFail();
					if (hr != AKEY_RV_OK) {
						return hr;
					}
					writeFlag = 2;
					addRetryCount(sendBuf);
					continue;
				}
                hr = Read(recvBuf);
                if (hr != AKEY_RV_OK) {
					LGNTRACE_MSG("TCP Read Error hr:%x",hr);
                    hr = OnSendAndRecvFail();
                    if (hr != AKEY_RV_OK) {
                        return hr;
                    }
					writeFlag = 2;
					addRetryCount(sendBuf);
                    continue;
                }
				hr = checkRecvBuf(recvBuf);
				if ((hr & 0xFFFF0000) == 0xE1CC0000) {
					LGNTRACE_MSG("TCPUDP checkRecvBuf  Error:hr:%x",hr);
					return hr;
				}
				if (hr != AKEY_RV_OK) {
					LGNTRACE_MSG("TCPUDP checkRecvBuf  Continue:hr:%x",hr);
					writeFlag = 0;//数据错误，重新读取
					continue;
				}else {
					writeFlag = 2;
				}
                return hr;
            }
            return hr;// can not reach
        }

		void addRetryCount(CBuffer& sendBuf)
		{
			++m_u32CurRetryTime;
			fnNetRetryTime netRetry = ParamManager::instance()->getNetRetryCb();
			if (netRetry) {
				netRetry(NET_TYPE_TCP,m_u32CurRetryTime);
			}
			LPBYTE pbInBuf = sendBuf.GetBuffer() + 4;
			PTR_IDCARD_PACKAGE_HEADER pbHeaderPkg = (PTR_IDCARD_PACKAGE_HEADER)(pbInBuf);
			UINT32 inLen = sendBuf.GetLength()-6;
			UINT16 mac = 0;
			pbHeaderPkg->retryTime = (BYTE)m_u32CurRetryTime;
			mac = OpenAlg::CRC::DoCRC16(pbInBuf,inLen, 0x3E8C);//
			Helper::BigEndian::UInt16ToBytes(mac,pbInBuf+inLen);
		}

		HRESULT checkRecvBuf(CBuffer& recvBuf)
		{
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "TCPUDP checkRecvBuf");
			HRESULT hr = AKEY_RV_OK;
			LPBYTE pbInBuf = recvBuf.GetBuffer();
			int inLen = recvBuf.GetLength();
			UINT16 mac = OpenAlg::CRC::DoCRC16(pbInBuf,inLen-2, 0x3E8C);//
			if (mac != Helper::BigEndian::UInt16FromBytes(pbInBuf+inLen-2)) {
				LGNTRACE_MSG("TCP-UDP mac not match");
				return AKEY_RV_MAC_VERIFY_ERROR;
			}
			PTR_IDCARD_PACKAGE_HEADER pbHeaderPkg = (PTR_IDCARD_PACKAGE_HEADER)(pbInBuf);
			if((pbHeaderPkg->errCode) != 0) {
				return (0xE1CC0000 + (pbHeaderPkg->errCode));
			}
			if (m_uniqueNumBuf.GetLength() && IDCardUtil::compareBufAndData(m_uniqueNumBuf,pbHeaderPkg->ChainNum,16) == false)
			{
				LGNTRACE_MSG("TCP-UDP m_uniqueNumBuf not match");
				LGNTRACE_HEX("localNum:",m_uniqueNumBuf.GetBuffer(),m_uniqueNumBuf.GetLength());
				LGNTRACE_HEX("serverNum:",pbHeaderPkg->ChainNum,16);
				return AKEY_RV_UNIQUENUM_NOT_MATCH;
			}
			CLog::instance()->addServerUDPNetRetryCount(pbHeaderPkg->retryTime);
			return hr;
		}

		HRESULT saveUniqueNum(CBuffer& sendBuf)
		{
			LPBYTE pbInBuf = sendBuf.GetBuffer() + 4;
			PTR_IDCARD_PACKAGE_HEADER pbHeaderPkg = (PTR_IDCARD_PACKAGE_HEADER)(pbInBuf);
			memcpy(m_uniqueNumBuf.ReAllocBytesSetLength(16),pbHeaderPkg->ChainNum,16);
			return AKEY_RV_OK;
		}
	protected:
		void getRetryCount()
		{
			UINT32 u32TimeOut = ParamManager::instance()->getPerPkgTimeOut();
			if (u32TimeOut == 0)
			{
				u32TimeOut = UDP_PER_PKG_TIMEOUT;
			}
			m_u32PerPkgTimeOut = u32TimeOut;
		}
        
    private:
		UINT32 m_u32CurRetryTime;
		CBuffer m_uniqueNumBuf;
		UINT32 m_u32PerPkgTimeOut;
		CTime m_time;
    };
};


#endif /* IDCardAppSocket_h */
