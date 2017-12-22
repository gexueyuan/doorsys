//
//  IDCardManager.h
//  TDRToken
//
//  Created by zhaowei on 16/5/11.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardManager_h
#define IDCardManager_h

#include "AKeyDef.h"
#include "IDCardConfig.h"
#include "IDCardInterface.h"

using namespace AKey;

namespace IDCARD {
    
    class ParamManager
    {
    public:
        ParamManager() {
            memset(&m_cbContext, 0, sizeof(TDR_CALLBACK_CONTEXT));
            m_bReadFinish = false;
			m_enabledReadWork = true;
            m_u32SamWaitTimeOut = 5;//默认是5s
            m_u32SamWaitInterval = 500;
            m_bWaitStop = false;
        }
        ~ParamManager(){}
        
        static ParamManager* instance()
        {
            static  ParamManager* _instance = NULL;
            if (_instance == NULL) {
                _instance = new ParamManager();
            }
            return _instance;
        }
        
        void setParamContext(TDR_PARAM_Context* context) {
            if (context == NULL) {
                return ;
            }
            memcpy(&m_paramContext,context,sizeof(TDR_PARAM_Context));
        }
        
        void setCallBackContext(TDR_CALLBACK_CONTEXT* callBack)
        {
            if (callBack) {
                m_cbContext.finishCb = callBack->finishCb;
                m_cbContext.IDInfoCb = callBack->IDInfoCb;
                m_cbContext.IDImageCb = callBack->IDImageCb;
                m_cbContext.cmdCb = callBack->cmdCb;
                m_cbContext.errCb = callBack->errCb;
				m_cbContext.netRetryCb = callBack->netRetryCb;
                m_cbContext.waitSamCb = callBack->waitSamCb;
            }
        }
        
        void setReaderInterface(IIDCardReaderInterface* pInterface)
        {
            m_interface = pInterface;
        }
        
        void setFileInterface(IIDCardFile* pInterface)
        {
            m_fileInterface = pInterface;
        }
        
        void setFinishFlag(bool bFinishFlag)
        {
            m_bReadFinish = bFinishFlag;
        }
        
        void setResumeFlag(bool bResume)
        {
            m_bSupportResume = bResume;
        }
        
        void setRequestIp(LPBYTE szIp,UINT32 u32IpLen)
        {
            memcpy(m_requestIpBuf.ReAllocBytesSetLength(u32IpLen), szIp, u32IpLen);
        }
        
        void setRequestPort(int port)
        {
            m_requestPort = port;
        }

		void setNoteInfo(LPBYTE pbNoteInfo,UINT32 u32NoteInfoLen)
		{
			memcpy(m_idNoteInfo.ReAllocBytesSetLength(u32NoteInfoLen),pbNoteInfo,u32NoteInfoLen);
		}

		void setImageAvaiableFlag(BYTE flag)
		{
			m_imageAvaiableFlag = flag;
		}
        
        void setSamWaitTimeOut(UINT32 u32TimeOut)
        {
            m_u32SamWaitTimeOut = u32TimeOut;
        }
        
        void setSamWaitTimeInterval(UINT32 timeInterval)
        {
            m_u32SamWaitInterval = timeInterval;
        }
        
        void setWaitFlag(bool bFlag)
        {
            m_bWaitStop = bFlag;
        }
        
        void setWaitSamNum(UINT32 u32WaitSamNum)
        {
            m_u32WaitSamNum = u32WaitSamNum;
        }

		void cleanNoteInfo()
		{
			m_idNoteInfo.SetLength(0);
		}

		void cleanImageAvaiableFlag()
		{
			m_imageAvaiableFlag = 0x01;
		}

		const CBuffer& getNoteInfo()
		{
			return m_idNoteInfo;
		}
        
        bool getFinishFlag()
        {
            return m_bReadFinish;
        }
        
        bool getResumeFlag()
        {
            return m_bSupportResume;
        }
        
        BYTE getRetryFlag()
        {
            return m_paramContext.bRetryFlag;
        }

		int getBusTimeOut()
		{
			return m_paramContext.nBusNoTimeOut;
		}
        
        UINT32 getTotalNetRetry()
        {
            return m_paramContext.u32NetRetry;
        }
        
        UINT32 getTotalReaderRetry()
        {
            return m_paramContext.u32ReaderRetry;
        }
        
        IIDCardReaderInterface* getReaderInterface()
        {
            return m_interface;
        }
        
        IIDCardFile* getFileInterface()
        {
            return m_fileInterface;
        }
        
        BYTE getDeviceType()
        {
            return m_paramContext.bDeviceType;
        }
        
        IDCARD_WORD_MODE getWorkMode()
        {
            return (IDCARD_WORD_MODE)m_paramContext.bWorkMode;
        }

		BYTE getImageAvaiableFlag()
		{
			return m_imageAvaiableFlag;
		}
        
        void setUUID(LPBYTE pbUUID,UINT32 u32UUIDLen)
        {
            memcpy(m_UUID,pbUUID,u32UUIDLen);
            m_u32UUIDLen = u32UUIDLen;
        }

        void getUUID(CBuffer& uuidBuf)
        {
            memcpy(uuidBuf.ReAllocBytesSetLength(m_u32UUIDLen),m_UUID,m_u32UUIDLen);
        }
        
        char* getErrFilePath()
        {
            return m_paramContext.errFile;
        }
        
        UINT32 getSamWaitTimeOut()
        {
            return m_u32SamWaitTimeOut;
        }
        
        UINT32 getSamWaitTimeInterval()
        {
            return m_u32SamWaitInterval;
        }
        
        bool getWaitFlag()
        {
            return m_bWaitStop;
        }

		void setSocketCfg(const char* szIp,int port, LPBYTE sockCfg,BYTE workMode,UINT32 u32ConnTimeOut,UINT32 u32TCPSRTime,UINT32 u32UDPSRTime,UINT32 u32PerPkgTimeOut,int nBusNoTimeOut)
		{
			if (szIp == NULL || strlen(szIp) == 0 || port == 0) {
				return;
			}
			strcpy(m_paramContext.IpAddr, szIp);
			m_paramContext.Port = port;
			m_paramContext.bWorkMode = workMode;
			if (sockCfg) {
				memcpy(m_paramContext.sockCfg,sockCfg,4);
			}
			m_paramContext.u32ConnTime = u32ConnTimeOut;
			m_paramContext.u32TCPSRTime = u32TCPSRTime;
			m_paramContext.u32UDPSRTime = u32UDPSRTime;
			m_paramContext.u32PerPkgTimeOut = u32PerPkgTimeOut;
			m_paramContext.nBusNoTimeOut = nBusNoTimeOut;
		}
        
        void getSamIpAddress(char* szIp)
        {
            if (szIp == NULL) {
                return;
            }
            strcpy(szIp,m_paramContext.IpAddr);
        }
        
        //获取调度服务器的端口号
        void getSamPort(int* port)
        {
            *port = m_paramContext.Port;
        }
        
        //申请到的ip地址
        void getRequestIpAddress(char* szIp)
        {
            memcpy(szIp, m_requestIpBuf.GetBuffer(), m_requestIpBuf.GetLength());
        }
        
        //申请到的port
        void getRequestPort(int* port)
        {
            *port = m_requestPort;
        }
    
        //获取客户号
        void getBusinessID(CBuffer& businessIDBuf)
        {
            memcpy(businessIDBuf.ReAllocBytesSetLength(m_paramContext.nBusinessLen),m_paramContext.businessID,m_paramContext.nBusinessLen);
        }

		UINT32 getConnTime()
		{
			return m_paramContext.u32ConnTime;//与服务器连接超时
		}

		UINT32 getTCPSRTime()
		{
			return m_paramContext.u32TCPSRTime;//向TCP服务器发送数据和接收数据超时
		}

		UINT32 getUDPSRTime()
		{
			return m_paramContext.u32UDPSRTime;//向TCP服务器发送数据和接收数据超时
		}

		UINT32 getPerPkgTimeOut()
		{
			return m_paramContext.u32PerPkgTimeOut;
		}

		void getSockCfg(LPBYTE pbSockCfg)
		{
			if (pbSockCfg) {
				memcpy(pbSockCfg,m_paramContext.sockCfg,4);
			}
		}

		BYTE getBindMode()
		{
			return m_paramContext.bBindMode;
		}

		void setEnableIDWork(bool bEnable)
		{
			m_enabledReadWork = bEnable;
		}

		bool getIDWorkIsEnable()
		{
			return m_enabledReadWork;
		}
        
        fnreadCardFinish getFinishCallBack()
        {
            return m_cbContext.finishCb;
        }
        
        fnreadCmdInfo getCmdCallBack()
        {
            return m_cbContext.cmdCb;
        }
        
        fnreadCardError getErrCallBack()
        {
            return m_cbContext.errCb;
        }
        
        fnreadIDInfo getIDCardInfoCb()
        {
            return m_cbContext.IDInfoCb;
        }
        
        fnreadIDImage getIDCardImgCb()
        {
            return m_cbContext.IDImageCb;
        }

		fnNetRetryTime getNetRetryCb()
		{
			return m_cbContext.netRetryCb;
		}
        
        fnWaitSamNum getWaitNumCb()
        {
            return m_cbContext.waitSamCb;
        }
        
        UINT32 getWaitSamNum()
        {
            return m_u32WaitSamNum;
        }

        
    private:
        TDR_PARAM_Context      m_paramContext;
        TDR_CALLBACK_CONTEXT    m_cbContext;
        IIDCardReaderInterface *m_interface;
        IIDCardFile *m_fileInterface;
        bool        m_bReadFinish;
        BYTE		m_UUID[16];
        UINT32		m_u32UUIDLen;
        bool  m_bSupportResume;//是否支持断点续传
        
        CBuffer m_requestIpBuf;//申请到的ip
		CBuffer m_idNoteInfo;  //身份证票据信息
        int     m_requestPort;    //申请到的端口号
		bool    m_enabledReadWork;//读证过程是否继续 true:继续读取 false:停止
		BYTE	m_imageAvaiableFlag;//0x01:取照片 0x02:不取照片
        
        UINT32  m_u32SamWaitTimeOut;
        UINT32  m_u32SamWaitInterval;
        bool    m_bWaitStop;
        UINT32  m_u32WaitSamNum;
    };
   };


#endif /* IDCardManager_h */
