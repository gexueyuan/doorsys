//
//  IDCardCore.h
//  TDRToken
//
//  Created by zhaowei on 16/5/6.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardCore_h
#define IDCardCore_h

#include "IDCardInclude.h"
#include "IDCardManager.h"
#include "IDCardBussiness.h"
#include "IDCardInfo.h"
#include "IDCardDevFlow.h"
#include "IDNetResumeCore.h"

#define IDCARD_DEV_COMMADN_VERSION  0x0302

namespace IDCARD {
   class IDCardCore
    {
    public:
        IDCardCore(IIDCore* core ):m_core(core)
        {
            
        }
        
        IDCardCore()
        {
            m_core = NULL;
        }
        
        ~IDCardCore(){}
        
        void setIDCardCore(IIDCore* core)
        {
            m_core = core;
        }
        
        void initParam(IDCardBussiness* business,IDCARD_CORE_CONTEXT* coreCtx)
        {
            m_ResumeCore.setBussiness(business);
            m_core = &m_ResumeCore;
			m_core->Init(coreCtx);
        }
        
        HRESULT ReadID(CBuffer& IDInfoBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ReadID");
            HRESULT hr = AKEY_RV_OK;
            if (m_core == NULL) {
                return AKEY_RV_NO_INIT;
            }
            m_core->Begin();
            hr = m_core->RequestPort();
            if (hr != AKEY_RV_OK) {
                LGNTRACE_MSG("ReadID RequestPort Error hr:%08x",hr);
                m_core->Finial();
                return hr;
            }
            hr = m_core->ReadIDInfo(IDInfoBuf);
            if (hr != AKEY_RV_OK) {
                LGNTRACE_MSG("ReadID DoTask Error hr:%08x",hr);
            }
            m_core->Finial();
            return hr;
        }
        
        HRESULT ReadImage(CBuffer& imgBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            hr = m_core->ReadImage(imgBuf);
            return hr;
        }
        
    private:
        IIDCore* m_core;
        IDNetResumeCore m_ResumeCore;
    };
};


#endif /* IDCardCore_h */
