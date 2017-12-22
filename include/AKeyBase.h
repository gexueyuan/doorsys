#ifndef __AKEY_BASE_H__
#define __AKEY_BASE_H__
//#pragma once

// 系统头文件
#ifdef WIN32
# include <windows.h>
# define OPENALG_SUPPORT_DIGEST 0 // 启用OpenAlg_dgst的摘要代码
#elif ANDROID
#include <string.h>
#include <stddef.h>
#include "log.h"
#include <android/log.h>
#else
#include <stdio.h>
#include <string.h>	
#endif


//
#include "AKeyDef.h"

// 错误号定义
#include "AKeyError.h"

#include "AKeyHelper.h"

#endif // #ifndef __AKEY_DEF_H__
