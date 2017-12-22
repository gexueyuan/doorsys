//
//  IDCardRetrySocket.h
//  TDRToken
//
//  Created by zhaowei on 2016/8/4.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardRetrySocket_h
#define IDCardRetrySocket_h

#define SOCKET_RETRY_CONNECT 3
#define TCP_RECV_TIMEOUT   2000 //TCP 默认接收超时时间

#include "IDCardInclude.h"
#include "IDCardUtil.h"

namespace IDCARD {
    class IDCardRetrySocket : public CSocketTCP
    {
    public:
        IDCardRetrySocket(){
			setTimeOut(TCP_RECV_TIMEOUT,TCP_RECV_TIMEOUT);
		}
        ~IDCardRetrySocket(){}

        virtual HRESULT OnSendAndRecvFail()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "TCP OnSendAndRecvFail");
            HRESULT hr = AKEY_RV_OK;
			if (m_u32CurRetryTime>= m_u32RetryCountPerPkg)
			{
				LGNTRACE_MSG("CurRetry:%d RetryCount:%d",m_u32CurRetryTime,m_u32RetryCountPerPkg);
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
			m_u32CurRetryTime = 0;
			int writeFlag = 1;
            UINT32 u32TimeWait = ParamManager::instance()->getSamWaitTimeOut();
            UINT32 u32BeginTime = 0;
            UINT32 u32TimeInterval = IDWORK_SAM_WAITTIME;
            if (ParamManager::instance()->getSamWaitTimeInterval() > 0) {
                u32TimeInterval = ParamManager::instance()->getSamWaitTimeInterval();
            }
            fnWaitSamNum waitNumCb = ParamManager::instance()->getWaitNumCb();
            for(;;)
            {
				if (ParamManager::instance()->getIDWorkIsEnable() == false) {
					LGNTRACE_MSG("User Cancel");
					return AKEY_RV_IDWORK_CANCELED;
				}
                if (ParamManager::instance()->getWaitFlag() == true) {
                    return AKEY_RV_IDWORK_CANCELED;
                }
				if (writeFlag == 1 || writeFlag == 2)
				{
                    if (writeFlag == 2) {
                        if (u32BeginTime >= u32TimeWait) {
                            return AKEY_RV_WAITSAM_TIMEOUT;
                        }
						BYTE bFlag;
						HRESULT hr2 = IDCardInfo::instance()->FindCard(&bFlag);
						if (hr2 == 0xE0E06FF0) {
							return AKEY_RV_WAITSAM_NOCARD;
						}else if(hr2 != AKEY_RV_OK) {
							return hr2;
						}
						
                        if (waitNumCb) {
                            waitNumCb(ParamManager::instance()->getWaitSamNum());
                        }
                        CTime::SleepTime(u32TimeInterval);
                    }
                    
					hr = Write(sendBuf);
					if (hr != AKEY_RV_OK) {
						LGNTRACE_MSG("TCP Write Error hr:%x",hr);
						DisconnectServer();
						hr = OnSendAndRecvFail();
						if (hr != AKEY_RV_OK) {
							return hr;
						}
						if (!IsSocketValid()) {
							LGNTRACE_MSG("TCP Socket is valid");
							return AKEY_RV_SOCKET_RECONNECT_INVALID;
						}
						addRetryCount(sendBuf);
						continue;
					}
				}
                hr = Read(recvBuf);
                if (hr != AKEY_RV_OK) {
					LGNTRACE_MSG("TCP Read Error hr:%x",hr);
                    DisconnectServer();
                    hr = OnSendAndRecvFail();
                    if (hr != AKEY_RV_OK) {
                        return hr;
                    }
                    if (!IsSocketValid()) {
                        LGNTRACE_MSG("Socket is valid");
                        return AKEY_RV_SOCKET_RECONNECT_INVALID;
                    }
					addRetryCount(sendBuf);
                    continue;
                }
				hr = checkRecvBuf(recvBuf);
                if (hr == 0xE1CC0011) {
                    writeFlag = 2;
                    u32BeginTime += u32TimeInterval;
                    continue;
                }
				if ((hr & 0xFFFF0000) == 0xE1CC0000) {
					LGNTRACE_MSG("TCP checkRecvBuf  Error:hr:%x",hr);
					return hr;
				}
				if (hr != AKEY_RV_OK) {
					LGNTRACE_MSG("TCP checkRecvBuf  Continue:hr:%x",hr);
					writeFlag = 0;//数据错误，重新读取
					continue;
				}else {
					writeFlag = 1;
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
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "TCP checkRecvBuf");
			HRESULT hr = AKEY_RV_OK;
			LPBYTE pbInBuf = recvBuf.GetBuffer();
			int inLen = recvBuf.GetLength();
			UINT16 mac = OpenAlg::CRC::DoCRC16(pbInBuf,inLen-2, 0x3E8C);//
			if (mac != Helper::BigEndian::UInt16FromBytes(pbInBuf+inLen-2)) {
				LGNTRACE_MSG("TCP mac not match");
				return AKEY_RV_MAC_VERIFY_ERROR;
			}
			PTR_IDCARD_PACKAGE_HEADER pbHeaderPkg = (PTR_IDCARD_PACKAGE_HEADER)(pbInBuf);
			if((pbHeaderPkg->errCode) != 0) {
                if (pbHeaderPkg->errCode == 0x0011) {
                    hr = getWaitSamNum(recvBuf);
                    if (hr != AKEY_RV_OK) {
                        return hr;
                    }
                }
				return (0xE1CC0000 + (pbHeaderPkg->errCode));
			}
			if (m_uniqueNumBuf.GetLength() && IDCardUtil::compareBufAndData(m_uniqueNumBuf,pbHeaderPkg->ChainNum,16) == false)
			{
				LGNTRACE_MSG("TCP m_uniqueNumBuf not match");
				LGNTRACE_HEX("localNum:",m_uniqueNumBuf.GetBuffer(),m_uniqueNumBuf.GetLength());
				LGNTRACE_HEX("serverNum:",pbHeaderPkg->ChainNum,16);
				return AKEY_RV_UNIQUENUM_NOT_MATCH;
			}
			CLog::instance()->addServerTCPNetRetryCount(pbHeaderPkg->retryTime);
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
				u32TimeOut = 6000;
			}
			m_u32RetryCountPerPkg = u32TimeOut/TCP_RECV_TIMEOUT;
		}
        
        HRESULT getWaitSamNum(CBuffer& recvBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "TCP getWaitSamNum");
            HRESULT hr = AKEY_RV_OK;
            LPBYTE pbInBuf = recvBuf.GetBuffer();
            int inLen = recvBuf.GetLength();
            CBuffer outBuf;
            UINT32 u32Offset = 0 ,u16TagLen = 0;
            pbInBuf = pbInBuf + sizeof(IDCARD_PACKAGE_HEADER);
            inLen = inLen -sizeof(IDCARD_PACKAGE_HEADER) - 2;
            u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ENCRYPT_TEXT, pbInBuf, inLen, &u32Offset);
            LPBYTE pbKey = (LPBYTE)IDCARD_DEFAULT_ENC_KEY;
            hr = CDecryptIDCard::DecPackage(pbKey, 16, (LPBYTE)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16, pbInBuf+u32Offset, u16TagLen, outBuf);
            if (hr != AKEY_RV_OK) {
                LGNTRACE_MSG("m_key Dec error:%x",hr);
                //解密失败，需要清除AppKeyHash
                return hr;
            }
            
            pbInBuf = outBuf.GetBuffer();
            inLen = outBuf.GetLength();
            u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SAM_WAITNUM, pbInBuf, inLen, &u32Offset);
            if (u16TagLen > 0 ) {
                UINT16 u16WaitNum = Helper::BigEndian::UInt16FromBytes(pbInBuf + u32Offset);
                ParamManager::instance()->setWaitSamNum(u16WaitNum);
            }else {
                ParamManager::instance()->setWaitSamNum(0);
            }
            
            return hr;

        }
        
    private:
		UINT32 m_u32CurRetryTime;
		CBuffer m_uniqueNumBuf;
		UINT32 m_u32RetryCountPerPkg;
    };
};


#endif /* IDCardAppSocket_h */
