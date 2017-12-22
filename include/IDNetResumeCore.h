//
//  IDNetResume.h
//  TDRToken
//
//  Created by zhaowei on 2016/8/4.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDNetResumeCore_h
#define IDNetResumeCore_h

/**
 *  该文件的工作模式:当网络出现错误的情况下，会把当前发送的包重新发送，直到成功接收服务器数据为止，重试次数由参数控制。
 */

#include "IDCardInclude.h"
#include "IDCardSocket.h"
#include "IDCardRetrySocket.h"
#include "IDCardUDP.h"
#include "IDCardRetryUDP.h"
#include "IDCardSocketManager.h"

namespace IDCARD {

    class IDNetResumeCore :public IIDCore
    {
    public:
        IDNetResumeCore(){}
        
        ~IDNetResumeCore(){}
        
        void setBussiness(IDCardBussiness* bussiness)
        {
            m_bussiness = bussiness;
        }
        
        virtual HRESULT RequestPort()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "RequestPort");
            //连接服务器，申请端口
			LGNTRACE_MSG("work mode:%d",m_ctx.bWorkMode);
            if (m_ctx.bWorkMode == MODE_NET_REQ_PORT) {
                HRESULT hr = AKEY_RV_OK;
				ISocket* pbSocket = CSockManager::instance()->getRequestPortSock();
                m_bussiness->setSocket(pbSocket);
                char szIp[50] = {0};
				int port = 0;
                ParamManager::instance()->getSamIpAddress(szIp);
                ParamManager::instance()->getSamPort(&port);
				pbSocket->setConnParam(szIp,port);
                hr = m_bussiness->GetPort();
                if (hr != AKEY_RV_OK) {
                    return hr;
                }
                return AKEY_RV_OK;
            }else {
                char szIp[50] = {0};
                ParamManager::instance()->getSamIpAddress(szIp);
                int port = 0;
                ParamManager::instance()->getSamPort(&port);
                ParamManager::instance()->setRequestIp((LPBYTE)szIp, (UINT32)strlen(szIp));
                ParamManager::instance()->setRequestPort(port);
                return AKEY_RV_OK;
            }
            return AKEY_RV_OK;
        }
        
        virtual HRESULT Init(IDCARD_CORE_CONTEXT* ctx)
        {
			if (ctx) {
				memcpy(&m_ctx,ctx,sizeof(IDCARD_CORE_CONTEXT));
				CSockManager::instance()->setTimeOut(ctx->u32ConnTime,ctx->u32TCPSRTime,ctx->u32UDPSRTime);
			}
            return AKEY_RV_OK;
        }
        
        virtual HRESULT Begin()
        {
            CLog::instance()->initCount();
            if (m_ctx.bWorkMode == MODE_NET_NO_REQ_PORT ||m_ctx.bWorkMode == MODE_NET_GATEWAY) {
                m_bussiness->cleanSamIndex();
            }
            return AKEY_RV_OK;
        }
        
        virtual HRESULT ReadIDInfo(CBuffer& IDInfoBufer)
        {
            if (m_bussiness == NULL) {
                return AKEY_RV_NO_INIT;
            }
            HRESULT hr = AKEY_RV_OK;
			hr = m_bussiness->DoIDTask(IDInfoBufer);
			if (hr != AKEY_RV_OK) {
				m_bussiness->IDReaderDisServer();
				ReleasePort();
			}
            return hr;
        }
        
        virtual HRESULT ReadImage(CBuffer& imgBufer)
        {
            if (m_bussiness == NULL) {
                return AKEY_RV_NO_INIT;
            }
            HRESULT hr = AKEY_RV_OK;
            hr = m_bussiness->ReadImage(imgBufer);
			m_bussiness->IDReaderDisServer();
			ReleasePort();
            return hr;
        }
        
        virtual HRESULT Finial()
        {
            return AKEY_RV_OK;
        }
    protected:
        
        HRESULT ReleasePort()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ReleasePort");
			LGNTRACE_MSG("word mode:%d",m_ctx.bWorkMode);
			if (m_ctx.bWorkMode == MODE_NET_NO_REQ_PORT || m_ctx.bWorkMode == MODE_NET_GATEWAY) {
				return AKEY_RV_OK;
			}
			
            HRESULT hr = AKEY_RV_OK;
            char szIp[50] = {0};
            ParamManager::instance()->getSamIpAddress(szIp);
            int port = 0;
            ParamManager::instance()->getSamPort(&port);
			ISocket* pbSocket = CSockManager::instance()->getRequestPortSock();
			pbSocket->setConnParam(szIp,port);
			m_bussiness->setSocket(pbSocket);
            hr = m_bussiness->ReleasePort();
            return hr;
        }
        
    private:
        IDCardBussiness* m_bussiness;
		IDCARD_CORE_CONTEXT m_ctx;
    };

	
};


#endif /* IDNetResume_h */
