//
//  IDCardTrade.h
//  TDRToken
//
//  Created by zhaowei on 16/5/8.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardBussiness_h
#define IDCardBussiness_h

#include <time.h>
#include "IDCardInclude.h"
#include "IDCardPackage.h"
#include "IDCardManager.h"
#include "IDCardDevFlow.h"
#include "IDCardSocketManager.h"

using namespace AKey;

namespace IDCARD {
    class IDCardBussiness: public IIDCardIOInterface
    {
    public:
        IDCardBussiness()
        {
            m_u16SendLen = 0;
            m_u16RecvLen = 0;
        }
        
        ~IDCardBussiness(){}
        
        void setSocket(ISocket* socket)
        {
            m_socket = socket;
        }
        
        virtual HRESULT WriteAndRead(IDREADER_PROCESS_STATUS proStatus,CBuffer& inBuf,CBuffer& outBuf)
        {
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "WriteAndRead");
            HRESULT hr = AKEY_RV_OK;
            CBuffer appBuf,shellBuf,recvBuf;
            hr = m_appPackage.MakePackage(proStatus,inBuf,appBuf);
            if (AKEY_RV_OK != hr) {
                return hr;
            }
			//m_shellPackage.setSeqNum(m_appPackage.getCurrentReqNum());
			//m_shellPackage.setBatch(m_appPackage.getBatchNum());
			
			m_shellPackage.setStepNum(proStatus);
            hr = m_shellPackage.MakePackage(appBuf,shellBuf);
            if (AKEY_RV_OK != hr) {
                return hr;
            }
            time.Begin();
            hr = m_socket->SendAndRecv(shellBuf, recvBuf);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
			UINT32 u32Time = time.timeSpend();
            CLog::instance()->AddNetTime(u32Time);
			LGNTRACE_MSG("SendLen:%d RecvLen:%d Time:%d ms",shellBuf.GetLength(),recvBuf.GetLength(),u32Time);
			IDCARD_SERVER_TYPE serverType;
			if (proStatus == PROGRESS_GET_PORT || 
				proStatus == PROGRESS_RELEASE_PORT) {
				serverType = SERVER_TYPE_REQUEST;
			}else {
				serverType = SERVER_TYPE_SAM;
			}
			
            CLog::instance()->AppendNetLengthAndTime(shellBuf.GetLength(),recvBuf.GetLength(),u32Time,m_socket->GetNetType(),serverType);
            hr = m_shellPackage.ParsePackage(recvBuf,appBuf);
            if ( AKEY_RV_OK != hr) {
                return hr;
            }
            hr = m_appPackage.ParsePackage(proStatus,appBuf,outBuf);
            return hr;
        }
        
        HRESULT WriteAndRead(IPackage* pbShellPackage,IDREADER_PROCESS_STATUS proStatus,CBuffer& inBuf,CBuffer& outBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            CBuffer appBuf,shellBuf,recvBuf;
            hr = m_appPackage.MakePackage(proStatus,inBuf,appBuf);
            if (AKEY_RV_OK != hr) {
                return hr;
            }
            hr = pbShellPackage->MakePackage(appBuf,shellBuf);
            if (AKEY_RV_OK != hr) {
                return hr;
            }
            time.Begin();
            hr = m_socket->SendAndRecv(shellBuf, recvBuf);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
			IDCARD_SERVER_TYPE serverType;
			if (proStatus == PROGRESS_GET_PORT || 
				proStatus == PROGRESS_RELEASE_PORT) {
				serverType = SERVER_TYPE_REQUEST;
			}else {
				serverType = SERVER_TYPE_SAM;
			}
            CLog::instance()->AddNetTime(time.timeSpend());
            CLog::instance()->AppendNetLengthAndTime(shellBuf.GetLength(),recvBuf.GetLength(),time.timeSpend(),m_socket->GetNetType(),serverType);
            hr = pbShellPackage->ParsePackage(recvBuf,appBuf);
            if ( AKEY_RV_OK != hr) {
                return hr;
            }
            hr = m_appPackage.ParsePackage(proStatus,appBuf,outBuf);
            return hr;
        }
        
        void IDReaderInit()
        {
            m_devFlow.initParam();
        }
        
        HRESULT IDReaderConnectServer()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ConnectServer");
            time.Begin();
            HRESULT hr = AKEY_RV_OK;
			LGNTRACE_MSG("****Connect Server ip:%s port:%d",m_socket->GetIp(),m_socket->GetPort());
            hr = m_socket->ConnectServer();
			if (hr == AKEY_RV_OK)
			{ 
				UINT16 timeTmp = time.timeSpend();
				CLog::instance()->AddNetTime(timeTmp);
				CLog::instance()->addConnectTime(timeTmp);
				LGNTRACE_MSG("****Connect Success time:%d ms",timeTmp);
			}else {
				LGNTRACE_MSG("****Connect Server Error :%x",hr);
				m_socket->DisconnectServer();
			}
            return hr;
        }
        
        HRESULT IDReaderDisServer()
        {
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "DisServer");
            time.Begin();
			LGNTRACE_MSG("&&&&DisConnect Server ip:%s port:%d",m_socket->GetIp(),m_socket->GetPort());
            m_socket->DisconnectServer();
			LGNTRACE_MSG("&&&&DisConnect Server Success");
			UINT32 timeTmp = (UINT32)(time.timeSpend());
            CLog::instance()->AddNetTime(timeTmp);
			CLog::instance()->addDisConnect(timeTmp);
            return AKEY_RV_OK;
        }
        
        void GetRefNumBuf(CBuffer& refNumBuf)
        {
            const CBuffer& buf = m_appPackage.getTransBuf();
            if (buf.GetLength()) {
                memcpy(refNumBuf.ReAllocBytesSetLength(buf.GetLength()), buf.GetBuffer(), buf.GetLength());
            }else {
                GetDefaultRefNum(refNumBuf);
            }
        }

		BYTE getSamIndex()
		{
			return m_appPackage.getSamIndex();
		}
        
        void cleanRefNum()
        {
            m_appPackage.setTranstion(NULL, 0);
        }
        
        void cleanSamIndex()
        {
            m_appPackage.setSamIndex(0);
			if(ParamManager::instance()->getWorkMode() == MODE_NET_GATEWAY)
			{
				m_shellPackage.cleanSamIndexAndAddr();
			}
        }

		void cleanCache04CMD()
		{
			m_devFlow.cleanCache04CMD();
		}
        
        HRESULT GetDefaultRefNum(CBuffer& refNumBuf)
        {
            struct timeval stTimeval;
            gettimeofday(&stTimeval, NULL);
            char btBuf[21];
            memcpy(btBuf, "Default", 7);
            sprintf(btBuf+7, "%010ld", stTimeval.tv_sec);
            sprintf(btBuf+17, "%03d", stTimeval.tv_usec/1000);
            memcpy(refNumBuf.ReAllocBytesSetLength(20), btBuf, 20);
            return AKEY_RV_OK;
        }
        
        HRESULT IDReaderWriteAndRead(IDREADER_PROCESS_STATUS proStatus,CBuffer& inBuf,CBuffer& outBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            if (proStatus == PROGRESS_BUILD_SAFECHANNEL1 ||
                proStatus == PROGRESS_BUILD_SAFECHANNEL2) {
                m_shellPackage.setDefaultKey();
            }else {
                const CBuffer& appKeyHash = IDCardInfo::instance()->getAppKeyHash();
                m_shellPackage.setKey(appKeyHash);
            }
            m_shellPackage.setTradeCode(proStatus);
			if (ParamManager::instance()->getWorkMode() != MODE_NET_GATEWAY)
			{
				m_shellPackage.setSamIndex(m_appPackage.getSamIndex());
			}
			
            CBuffer devOutBuf,recvBuf;
            hr = m_devFlow.MakeDevFlow(proStatus, devOutBuf);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            hr = WriteAndRead(proStatus, devOutBuf, recvBuf);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            hr = m_devFlow.ParseDevFlow(recvBuf, outBuf);
            return hr;
        }
        
        HRESULT isReadIDDone()
        {
            return m_devFlow.isReadIDDone();
        }
        
        IDREADER_PROCESS_STATUS getNextProcess()
        {
            return m_devFlow.getNextProgressStatus();
        }
       
        
        HRESULT GetPort()
        {
            HRESULT hr = AKEY_RV_OK;
            CBuffer inBuf,outBuf;
            hr = IDReaderConnectServer();
            if (hr != AKEY_RV_OK) {
                return AKEY_RV_CONNECT_DISPORT_SERVER_ERROR;
            }
            hr = WriteAndRead(&m_NoEncShellPackage, PROGRESS_GET_PORT, inBuf, outBuf);
            IDReaderDisServer();
            return hr;
        }
        
        HRESULT ReleasePort()
        {
            HRESULT hr = AKEY_RV_OK;
            CBuffer inBuf,outBuf;
            hr = IDReaderConnectServer();
            if (hr != AKEY_RV_OK) {
                return AKEY_RV_CONNECT_DISPORT_SERVER_ERROR;
            }
            hr = WriteAndRead(&m_NoEncShellPackage, PROGRESS_RELEASE_PORT, inBuf, outBuf);
            IDReaderDisServer();
            return hr;
        }
        
        /**
         *  建立安全通道
         *
         *  @param outBuf 返回数据
         *
         *  @return 错误码
         */
        HRESULT buildSafeChannel(CBuffer& outBuf)
        {
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "buildSafeChannel");
            HRESULT hr = AKEY_RV_OK;
			cleanCache04CMD();
            CBuffer inBuf;
			ISocket* pbSocket = CSockManager::instance()->getSockFromStep(STEP_BUILD_SAFECHANNEL);
			char szIp[50] = {0};
			int port = 0;
			ParamManager::instance()->getRequestIpAddress(szIp);
			ParamManager::instance()->getRequestPort(&port);
			pbSocket->setConnParam(szIp,port);
            setSocket(pbSocket);
			hr = IDReaderConnectServer();
			if (hr != AKEY_RV_OK) {
				LGNTRACE_MSG("buildSafeChannel ConnectServer hr:%x",hr);
				return AKEY_RV_CONNECT_SAM_SERVER_ERROR;
			}
			
            hr = IDReaderWriteAndRead(PROGRESS_BUILD_SAFECHANNEL1,inBuf,outBuf);
            if (hr != AKEY_RV_OK) {
				IDReaderDisServer();
                return hr;
            }
            hr = IDReaderWriteAndRead(PROGRESS_BUILD_SAFECHANNEL2,inBuf,outBuf);
			if (hr == AKEY_RV_OK) {
				if (checkNeedCloseSock(STEP_BUILD_SAFECHANNEL)) {
					IDReaderDisServer();
				}
			}else {
				IDReaderDisServer();
			}
            return hr;
        }
        
        /**
         *  身份证认证
         *
         *  @param inBuf  输入数据(04,07指令,udp通信)
         *  @param outBuf 输出数据
         *  @param pbIsAuthOrReadDone:(0:表示认证结束，1：表示读证结束)
		 *  
         *  @return 错误码
         */
        HRESULT Authentication(CBuffer& inBuf,CBuffer& outBuf,BYTE* pbIsAuthOrReadDone)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Authentication");
            HRESULT hr = AKEY_RV_OK;
			bool isCacheCMD = m_devFlow.getCache04CMD(inBuf);
            hr = m_devFlow.Execute_single(inBuf.GetBuffer(), inBuf.GetLength());
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            IDREADER_PROCESS_STATUS progress = (isCacheCMD)?PROGRESS_CMD_EXCHANGE_BEGIN:PROGRESS_CMD_EXCHANGE_SINGLE;

			ISocket* pbSocket = CSockManager::instance()->getSockFromStep(STEP_AUTHENTICATION);
			setSocket(pbSocket);
			if (checkNeedReConnSock(STEP_AUTHENTICATION) || (pbSocket->IsSocketValid() == false))
			{
				char szIp[50] = {0};
				int port = 0;
				ParamManager::instance()->getRequestIpAddress(szIp);
				ParamManager::instance()->getRequestPort(&port);
				pbSocket->setConnParam(szIp,port);
				hr = IDReaderConnectServer();
				if (hr != AKEY_RV_OK) {
					return AKEY_RV_CONNECT_SAM_SERVER_ERROR;
				}
			}
            while (1) {
                LGNTRACE_MSG("ReadID progress:%d",progress);
                hr = IDReaderWriteAndRead(progress,inBuf,outBuf);
                if (hr != AKEY_RV_OK) {
					IDReaderDisServer();
                    return hr;
                }
                if (m_devFlow.isAuthenDone()) {
					if (pbIsAuthOrReadDone){
						*pbIsAuthOrReadDone = 0;
					}
					if (checkNeedCloseSock(STEP_AUTHENTICATION)) {
						IDReaderDisServer();
					}
                    return AKEY_RV_OK;
                }
				if (m_devFlow.isReadIDDone()) {
					if (pbIsAuthOrReadDone){
						*pbIsAuthOrReadDone = 1;
					}
					if (checkNeedCloseSock(STEP_AUTHENTICATION)) {
						IDReaderDisServer();
					}
					return AKEY_RV_OK;
				}
				
                progress = getNextProcess();
            }
            return hr;
        }
        
        /**
         *  读取身份证
         *
         *  @param inBuf  输入数据
         *  @param outBuf 输出数据
         *
         *  @return 错误码
         */
        HRESULT ReadIDInfo(CBuffer& inBuf,CBuffer& outBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ReadIDInfo");
            HRESULT hr = AKEY_RV_OK;
            CBuffer cmdRecvBuf;
            hr = m_devFlow.Execute_Mul(inBuf.GetBuffer(), inBuf.GetLength());
            if (hr != AKEY_RV_OK) {
				CBuffer tmpSend,tmpRecv;
				IDReaderWriteAndRead(PROGRESS_CMD_ERROR, tmpSend, tmpRecv);
                return hr;
            }
            CBuffer tmpInBuf;
			ISocket* pbSocket = CSockManager::instance()->getSockFromStep(STEP_READ_IDINFO);
			setSocket(pbSocket);
			if (pbSocket->IsSocketValid() == false)
			{
				char szIp[50] = {0};
				int port = 0;
				ParamManager::instance()->getRequestIpAddress(szIp);
				ParamManager::instance()->getRequestPort(&port);
				pbSocket->setConnParam(szIp,port);
				hr = IDReaderConnectServer();
				if (hr != AKEY_RV_OK) {
					return AKEY_RV_CONNECT_SAM_SERVER_ERROR;
				}
			}
            hr = IDReaderWriteAndRead(PROGRESS_CMD_EXCHANGE_MUL, tmpInBuf, outBuf);
            return hr;
        }

		HRESULT ReadImage(CBuffer& outBuf)
		{
			HRESULT hr = AKEY_RV_OK;
			CBuffer inBuf;
			ISocket* pbSocket = CSockManager::instance()->getSockFromStep(STEP_READ_IMAGE);
			setSocket(pbSocket);
			if (!pbSocket->IsSocketValid())
			{
				char szIp[50] = {0};
				int port = 0;
				ParamManager::instance()->getRequestIpAddress(szIp);
				ParamManager::instance()->getRequestPort(&port);
				pbSocket->setConnParam(szIp,port);
				hr = IDReaderConnectServer();
				if (hr != AKEY_RV_OK) {
					return AKEY_RV_CONNECT_SAM_SERVER_ERROR;
				}
			}
			m_shellPackage.setTradeCode(PROGRESS_REQUEST_IMAGE);
			hr = WriteAndRead(PROGRESS_REQUEST_IMAGE,inBuf, outBuf);
			pbSocket->DisconnectServer();
			return hr;
		}
        
        HRESULT DoIDTask(CBuffer& idBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "DoIDTask");
            HRESULT hr = AKEY_RV_OK;
            IDREADER_STEP step = STEP_BUILD_SAFECHANNEL;
            CBuffer sigCMDBuf,mulCMDBufer;
			IDCARD_WORD_MODE workMode = ParamManager::instance()->getWorkMode();
			m_appPackage.setRequestNum(0);
            if (m_devFlow.getCache04CMD(sigCMDBuf)) { //指令有缓存，则进入认证过程
                step = STEP_AUTHENTICATION;
            }
            IDCardUtil::instance()->Timebegin();
            for (; ; ) {
                switch (step) {
                    case STEP_BUILD_SAFECHANNEL:
                    {
                        hr = buildSafeChannel(sigCMDBuf);
                        if (hr == AKEY_RV_OK) {
                            step = STEP_AUTHENTICATION;
                            continue;
                        }else {
                            return hr;
                        }
                    }
                        break;
                    case STEP_AUTHENTICATION:
                    {
						BYTE bAuthOrReadDone;
                        hr = Authentication(sigCMDBuf,mulCMDBufer,&bAuthOrReadDone);
                        if (hr == AKEY_RV_OK) {
							if (bAuthOrReadDone == 0x00)//认证结束
							{
								step = STEP_READ_IDINFO;
								continue;
							}else {//读证结束
								memcpy(idBuf.ReAllocBytesSetLength(mulCMDBufer.GetLength()),mulCMDBufer.GetBuffer(),mulCMDBufer.GetLength());
								return AKEY_RV_OK;
							}
                        }else {
                            if (!IDCardUtil::instance()->checkTimeOut()) {
                                LGNTRACE_MSG("Authentication time out");
                                return hr;
                            }
							if (hr == AKEY_RV_PACKAGE_COMM_TIMEOUT)//通信包超时为6秒
							{
								hr = m_devFlow.checkCard();
								if (hr != AKEY_RV_OK) {
									return hr; //无卡，退出
								}else {
									step = STEP_AUTHENTICATION;//有卡，重新认证
									if (workMode == MODE_NET_GATEWAY || 
										workMode == MODE_NET_NO_REQ_PORT)
									{
										cleanSamIndex();
									}
									continue;
								}
							}
                            //非客户端错误
                            if(!m_devFlow.IsClientError(hr))
                            {
                                LGNTRACE_MSG("no client error:%x",hr);
                                //清除与服务器相关缓存数据
                                cleanSamIndex();
                                return hr;
                            }
                            if(hr == AKEY_RV_DEC_ENC_DATA_ERROR) {
								IDCardInfo::instance()->cleanAppKeyHash();
                                step = STEP_BUILD_SAFECHANNEL; //重建通道
								if (workMode == MODE_NET_GATEWAY || 
									workMode == MODE_NET_NO_REQ_PORT)
								{
									cleanSamIndex();
								}
                                continue;
                            }else  if(hr == 0xE0E06FF0) {
								hr = m_devFlow.checkCard();
								if (hr != AKEY_RV_OK) {
									return hr; //无卡，退出
								}else {
									step = STEP_AUTHENTICATION;//有卡，重新认证
									if (workMode == MODE_NET_GATEWAY || 
										workMode == MODE_NET_NO_REQ_PORT)
									{
										cleanSamIndex();
									}
									continue;
								}
							} else {
								return hr;
                            }
                        }
                    }
                        break;
                    case STEP_READ_IDINFO:
                    {
                        hr = ReadIDInfo(mulCMDBufer,idBuf);//读证
                        if (hr == AKEY_RV_OK) {
                            return hr;
                        }else {
                            if (!IDCardUtil::instance()->checkTimeOut()) {
                                LGNTRACE_MSG("Authentication time out");
                                return hr;
                            }
							if (hr == AKEY_RV_PACKAGE_COMM_TIMEOUT)//通信包超时为6秒
							{
								hr = m_devFlow.checkCard();
								if (hr != AKEY_RV_OK) {
									return hr; //无卡，退出
								}else {
									step = STEP_AUTHENTICATION;//有卡，重新认证
									if (workMode == MODE_NET_GATEWAY || 
										workMode == MODE_NET_NO_REQ_PORT)
									{
										cleanSamIndex();
									}
									continue;
								}
							}
                            if(hr == 0xE0E06FF0)
							{
								hr = m_devFlow.checkCard();
								if (hr != AKEY_RV_OK) {
									return hr; //无卡，退出
								}else {
									step = STEP_AUTHENTICATION;//有卡，重新认证
									if (workMode == MODE_NET_GATEWAY || 
										workMode == MODE_NET_NO_REQ_PORT)
									{
										cleanSamIndex();
									}
									continue;
								}
							}else if(m_devFlow.IsSWError(hr) ||
								hr == AKEY_RV_DEC_ENC_DATA_ERROR){//sw错误或者解密失败,重建通道
								LGNTRACE_ERRORNO(hr);
                                step = STEP_BUILD_SAFECHANNEL;
								if (workMode == MODE_NET_GATEWAY || 
									workMode == MODE_NET_NO_REQ_PORT)
								{
									cleanSamIndex();
								}
                                continue;
                            }else{ //其他退出
								return hr;
							}
                        }
                    }
                        break;
                        
                    default:
                        break;
                }
            }
            return hr;
        }
        
        void LoadCache()
        {
            m_devFlow.LoadCache();
        }
        
        void getTimeBuf(CBuffer& timeBuf)
        {
            BYTE btLogBuf[DEFAULT_BUFFERLEN];
            UINT32 u32LogLen = 0;
            const CBuffer& totalNetTimeBuf = CLog::instance()->getTotalNetTimeBuf();
			CBuffer totalNetAndVerBuf;
			LPBYTE pbTotalNetAndVer = totalNetAndVerBuf.ReAllocBytesSetLength(totalNetTimeBuf.GetLength() + strlen(IDCARD_NET_DATA_VERSION));
			memcpy(pbTotalNetAndVer,IDCARD_NET_DATA_VERSION,strlen(IDCARD_NET_DATA_VERSION));
			memcpy(pbTotalNetAndVer+strlen(IDCARD_NET_DATA_VERSION),totalNetTimeBuf.GetBuffer(),totalNetTimeBuf.GetLength());
            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_NETWORK_TIME,totalNetAndVerBuf.GetBuffer(),totalNetAndVerBuf.GetLength(), btLogBuf + u32LogLen);

			BYTE bSamIndex = m_appPackage.getSamIndex();
			u32LogLen += CPackageMaker::MakeTLVPacket(TAG_SAM_INDEX,&bSamIndex,1, btLogBuf + u32LogLen);
			
			u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_SDK_VERSION,(LPBYTE)IDCARD_SDK_VERSION,strlen(IDCARD_SDK_VERSION), btLogBuf + u32LogLen);

			const CBuffer& samVerBuf = CLog::instance()->getSamVerBuf();
			u32LogLen += CPackageMaker::MakeTLVPacket(TAG_SAM_CONTROLLER_VERSION,samVerBuf.GetBuffer(),samVerBuf.GetLength(), btLogBuf + u32LogLen);
            
            BYTE btNetTime[2];
            UINT16 u16TotalTaskTime = CLog::instance()->getTotalTaskTime();
            Helper::BigEndian::UInt16ToBytes(u16TotalTaskTime, btNetTime);
            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_TOTAL_TASK_TIME,btNetTime,2, btLogBuf + u32LogLen);
            
            const CBuffer& totalCmdBuf = CLog::instance()->getTotalInsTime();
            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_READER_CMD_TIME,totalCmdBuf.GetBuffer(),totalCmdBuf.GetLength(), btLogBuf + u32LogLen);
            
            CBuffer netHandleTime,appSafeComBuf;
            CLog::instance()->getNetHandleBuf_Ex(netHandleTime);
            CLog::instance()->getSafeComBuf_Ex(appSafeComBuf);
            
            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_SAFE_COM,appSafeComBuf.GetBuffer(),appSafeComBuf.GetLength(), btLogBuf + u32LogLen);
            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_NET_APP_HANDLE,netHandleTime.GetBuffer(),netHandleTime.GetLength(), btLogBuf + u32LogLen);
            
            u16TotalTaskTime = CLog::instance()->getInterfaceTime();
            Helper::BigEndian::UInt16ToBytes(u16TotalTaskTime, btNetTime);
            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_APP_INTERFACE_TIME,btNetTime,2, btLogBuf + u32LogLen);


			UINT16 u16ConnectTime = CLog::instance()->getConnectTime();
			Helper::BigEndian::UInt16ToBytes(u16ConnectTime, btNetTime);
			u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_APP_CONNECT_TIME,btNetTime,2, btLogBuf + u32LogLen);


			UINT16 u16DisConnectTime = CLog::instance()->getDisConnectTime();
			Helper::BigEndian::UInt16ToBytes(u16DisConnectTime, btNetTime);
			u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_APP_DISCONNECT_TIME,btNetTime,2, btLogBuf + u32LogLen);
            
            UINT16 u16ReqPortTime = CLog::instance()->getReqPortTime();
            Helper::BigEndian::UInt16ToBytes(u16ReqPortTime, btNetTime);
            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_GETPORT_GET_TIME,btNetTime,2, btLogBuf + u32LogLen);
            
            UINT16 u16RelPortTime = CLog::instance()->getRelPortTime();
            Helper::BigEndian::UInt16ToBytes(u16RelPortTime, btNetTime);
            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_GETPORT_RELEASE_TIME,btNetTime,2, btLogBuf + u32LogLen);
            
            
            BYTE btRetryBuf[8+4+7*2];
			UINT32 u32RetryOffset = 0;
			memcpy(btRetryBuf,IDCARD_TIME_DATA_VERSION,strlen(IDCARD_TIME_DATA_VERSION));
			u32RetryOffset+=strlen(IDCARD_TIME_DATA_VERSION);
			BYTE bSockCfg[4];
			ParamManager::instance()->getSockCfg(bSockCfg);
			memcpy(btRetryBuf+u32RetryOffset,bSockCfg,4);
			u32RetryOffset += 4;
			Helper::BigEndian::UInt16ToBytes(ParamManager::instance()->getTotalNetRetry(),btRetryBuf+u32RetryOffset);//总网络重试次数
			u32RetryOffset+=2;
			Helper::BigEndian::UInt16ToBytes(ParamManager::instance()->getTotalReaderRetry(),btRetryBuf+u32RetryOffset);//总读卡器重试次数
			u32RetryOffset+=2;
			Helper::BigEndian::UInt16ToBytes(CLog::instance()->getReaderRetryCount(),btRetryBuf+u32RetryOffset);//读卡器重试次数
			u32RetryOffset+=2;
			Helper::BigEndian::UInt16ToBytes(CLog::instance()->getClientTCPNetRetryCount(),btRetryBuf+u32RetryOffset);//客户端TCP网络重试次数
			u32RetryOffset+=2;
			Helper::BigEndian::UInt16ToBytes(CLog::instance()->getClientUDPNetRetryCount(),btRetryBuf+u32RetryOffset);//客户端UDP网络重试次数
			u32RetryOffset+=2;
			Helper::BigEndian::UInt16ToBytes(CLog::instance()->getServerTCPNetRetryCount(),btRetryBuf+u32RetryOffset);//服务器端TCP网络重试次数
			u32RetryOffset+=2;
			Helper::BigEndian::UInt16ToBytes(CLog::instance()->getServerUDPNetRetryCount(),btRetryBuf+u32RetryOffset);//服务器端UDP网络重试次数
			u32RetryOffset+=2;

            u32LogLen += CPackageMaker::MakeTLVPacket(TAG_LOG_APP_RETRY_COUNT,btRetryBuf,u32RetryOffset, btLogBuf + u32LogLen);
            
            memcpy(timeBuf.ReAllocBytesSetLength(u32LogLen), btLogBuf, u32LogLen);

        }
       
		//查询标志
		bool IsEnabledIDWork()
		{
			return ParamManager::instance()->getIDWorkIsEnable();
		}

		void EnableIDWork()
		{
			ParamManager::instance()->setEnableIDWork(true);
		}

		void DisableIDWork()
		{
			ParamManager::instance()->setEnableIDWork(false);
		}

	protected:
		bool checkNeedCloseSock(IDREADER_STEP curStep)
		{
			BYTE bCfg[4];
			ParamManager::instance()->getSockCfg(bCfg);
			if (curStep < STEP_READ_IMAGE) {
				IDCARD_NET_TYPE curStepNetType,nextStepNetType;
				if (bCfg[curStep] == NET_TYPE_DEFAULT) {
					curStepNetType = g_NetType[curStep];
				}else {
					curStepNetType = (IDCARD_NET_TYPE)bCfg[curStep];
				}

				if (bCfg[curStep+1] == NET_TYPE_DEFAULT) {
					nextStepNetType = g_NetType[curStep+1];
				}else {
					nextStepNetType = (IDCARD_NET_TYPE)bCfg[curStep+1];
				}
				if (curStepNetType != nextStepNetType) {//当前的网络类型和下一步的网络类型不同时，需要关闭当前的sock,否则不用关闭
					return true;
				}else {
					return false;
				}
			}else {
				return true;
			}
			
		}
		bool checkNeedReConnSock(IDREADER_STEP curStep)
		{
			BYTE bCfg[4];
			ParamManager::instance()->getSockCfg(bCfg);
			if (curStep > STEP_BUILD_SAFECHANNEL) {
				IDCARD_NET_TYPE curStepNetType,PreStepNetType;
				if (bCfg[curStep] == NET_TYPE_DEFAULT) {
					curStepNetType = g_NetType[curStep];
				}else {
					curStepNetType = (IDCARD_NET_TYPE)bCfg[curStep];
				}

				if (bCfg[curStep-1] == NET_TYPE_DEFAULT) {
					PreStepNetType = g_NetType[curStep-1];
				}else {
					PreStepNetType = (IDCARD_NET_TYPE)bCfg[curStep-1];
				}
				if (curStepNetType != PreStepNetType) {//当前的网络类型和下一步的网络类型不同时，需要关闭当前的sock,否则不用关闭
					return true;
				}else {
					return false;
				}
			}else {
				return true;
			}
		}

        
    private:
        ISocket* m_socket;
        AppPackage m_appPackage;
        ShellPackage m_NoEncShellPackage;
        EncShellPackage m_shellPackage;
        CIDDevFlow m_devFlow;
        CTime time;
        UINT16 m_u16SendLen;
        UINT16 m_u16RecvLen;
    };
};


#endif /* IDCardTrade_h */
