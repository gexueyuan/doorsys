//
//  lgntrace.cpp
//  iOSToken
//
//  Created by lujun on 13-4-16.
//  Copyright (c) 2013年 lujun. All rights reserved.
//

#include "IDCardInclude.h"
#include "lgntraceP.h"
#if (LGN_SUPPORT_TRACE)
namespace LGN
{
    CAutoCriticalSection g_criticalSectionForLog;
    CTraceCategory lgnTraceGeneral(LGN_TRACE_CATEGORY_DEF_FLAGS, LGN::TraceLevel::Debug, (LPCSTR)LGN_TRACE_CATEGORY_DEF_NAME,LGN_TRACE_CATEGORY_DEF_FILE);
};
#endif
