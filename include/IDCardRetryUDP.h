//
//  IDCardRetryUDP.h
//  TDRToken
//
//  Created by zhaowei on 2016/8/10.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardRetryUDP_h
#define IDCardRetryUDP_h

#include "IDCardInclude.h"

#define UDP_RECV_TIMEOUT   100		//接收超时(单位毫秒)
#define UDP_RECV_TIMEOUT_CB 20		//20次接收超时回调
#define UDP_PER_PKG_TIMEOUT	6000	//每一包默认超时(单位毫秒)

namespace IDCARD {
    class IDCardRetryUDP : public CUDPRaw {

    public:
		IDCardRetryUDP(){
			setTimeOut(0,UDP_RECV_TIMEOUT);
		}
		~IDCardRetryUDP(){}
        
        virtual HRESULT OnSendAndRecvFail()
        {
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP OnSendAndRecvFail");
			HRESULT hr = AKEY_RV_OK;
			if (m_time.timeSpend() >= m_u32PerPkgTimeOut)//超时
			{
				LGNTRACE_MSG("CurRetry:%d timeSpend:%d m_u32PerPkgTimeOut:%d",m_u32CurRetryTime,m_time.timeSpend(),m_u32PerPkgTimeOut);
				return AKEY_RV_PACKAGE_COMM_TIMEOUT;//重试次数超出
			}
			if(IDCardUtil::instance()->checkTimeOut() == false){
				return AKEY_RV_IDWORK_TIMEOUT;
			}
			return AKEY_RV_OK;
        }
        
        HRESULT checkRecvBuf(CBuffer& recvBuf)
        {
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP checkRecvBuf");
			HRESULT hr = AKEY_RV_OK;
			LPBYTE pbInBuf = recvBuf.GetBuffer();
			int inLen = recvBuf.GetLength();
			UINT16 mac = OpenAlg::CRC::DoCRC16(pbInBuf,inLen-2, 0x3E8C);//
			if (mac != Helper::BigEndian::UInt16FromBytes(pbInBuf+inLen-2)) {
				LGNTRACE_MSG("UDP mac not match");
				return AKEY_RV_MAC_VERIFY_ERROR;
			}
			PTR_IDCARD_PACKAGE_HEADER pbHeaderPkg = (PTR_IDCARD_PACKAGE_HEADER)(pbInBuf);
			if((pbHeaderPkg->errCode) != 0) {
				return (0xE1CC0000 + (pbHeaderPkg->errCode));
			}
			if (m_uniqueNumBuf.GetLength() && IDCardUtil::compareBufAndData(m_uniqueNumBuf,pbHeaderPkg->ChainNum,16) == false)
			{
				LGNTRACE_MSG("UDP m_uniqueNumBuf not match");
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

        virtual HRESULT SendAndRecv(CBuffer& sendBuf,CBuffer& recvBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "UDP SendAndRecv");
			if (ParamManager::instance()->getIDWorkIsEnable() == false) {
				LGNTRACE_MSG("User Cancel");
				return AKEY_RV_IDWORK_CANCELED;
			}
            HRESULT hr = AKEY_RV_OK;
			saveUniqueNum(sendBuf);
			calPkgTimeout();
			m_time.Begin();
			m_u32CurRetryTime = 0;
			int writeFlag = 1;
            for(int index = 100;;index += 100)
            {
				if (ParamManager::instance()->getIDWorkIsEnable() == false) {
					LGNTRACE_MSG("User Cancel");
					return AKEY_RV_IDWORK_CANCELED;
				}
				if (writeFlag == 1)
				{
					hr = Write(sendBuf);
					if (hr != AKEY_RV_OK) {
						hr = OnSendAndRecvFail();
						if (hr != AKEY_RV_OK) {
							return hr;
						}
						addRetryCount(sendBuf);
						LGNTRACE_MSG("UDP m_u32CurRetryTime:%d",m_u32CurRetryTime);
						continue;
					}
				}
                
                hr = Read(recvBuf);
                if (hr != AKEY_RV_OK) {
					if (hr == AKEY_RV_SOCKET_COMM_TIMEOUT) {
						hr = OnSendAndRecvFail();
						if (hr != AKEY_RV_OK) {
							return hr;
						}
						addRetryCount(sendBuf);
						LGNTRACE_MSG("UDP m_u32CurRetryTime:%d",m_u32CurRetryTime);
						continue;
					}else {
						return hr;
					}
                }
				hr = checkRecvBuf(recvBuf);
				if ((hr & 0xFFFF0000) == 0xE1CC0000) {
					LGNTRACE_MSG("UDP checkRecvBuf  Error:hr:%x",hr);
					return hr;
				}
				if (hr != AKEY_RV_OK) {
					hr = OnSendAndRecvFail();
					if (hr != AKEY_RV_OK) {
						return hr;
					}
					LGNTRACE_MSG("UDP checkRecvBuf  Continue:hr:%x",hr);
					writeFlag = 0;//数据错误，重新读取
					continue;
				}else {
					writeFlag = 1;
				}
                return hr;
            }
            return hr;
        }

		void addRetryCount(CBuffer& sendBuf)
		{
			++m_u32CurRetryTime;
			if ((m_u32CurRetryTime % 4) == 0)
			{
				fnNetRetryTime netRetry = ParamManager::instance()->getNetRetryCb();
				if (netRetry) {
					netRetry(NET_TYPE_UDP,m_u32CurRetryTime);
				}
			}
			CLog::instance()->addClientUDPNetRetryCount();
			LPBYTE pbInBuf = sendBuf.GetBuffer() + 4;
			PTR_IDCARD_PACKAGE_HEADER pbHeaderPkg = (PTR_IDCARD_PACKAGE_HEADER)(pbInBuf);
			UINT32 inLen = sendBuf.GetLength()-6;
			UINT16 mac = 0;
			pbHeaderPkg->retryTime = (BYTE)m_u32CurRetryTime;
			mac = OpenAlg::CRC::DoCRC16(pbInBuf,inLen, 0x3E8C);//
			Helper::BigEndian::UInt16ToBytes(mac,pbInBuf+inLen);
		}
	protected:
		void calPkgTimeout()
		{
			UINT32 u32TimeOut = ParamManager::instance()->getPerPkgTimeOut();
			if (u32TimeOut == 0)
			{
				u32TimeOut = UDP_PER_PKG_TIMEOUT;
			}
			m_u32PerPkgTimeOut = u32TimeOut;
		}

	private:
		CBuffer m_uniqueNumBuf;
		UINT32 m_u32CurRetryTime;
		UINT32 m_u32PerPkgTimeOut;
		CTime m_time;
    };
};


#endif /* IDCardRetryUDP_h */
