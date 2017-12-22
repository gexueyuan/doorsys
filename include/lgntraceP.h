
#ifndef __LGNTRACEP_IDCARD_H__
#define __LGNTRACEP_IDCARD_H__


#define LGN_SUPPORT_UNICODE 0
#define LGN_SUPPORT_SELECTANY 0
#define LGN_CDECL 
#define LGN_INLINE inline
#define LGN_SELECTANY extern
#define LGN_BR "\n"
#define LGN_BR_W L"\n"
#define LGN_TEXT(quote) L##quote
#define LGN_NOOP

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <stdarg.h>
#ifndef  TARGET_OS_IOS
#include "IDCardInclude.h"
#endif
#include "IDCardUtil.h"

typedef signed char         INT8, *LPINT8;
typedef signed short        INT16, *LPINT16;
typedef signed int          INT32, *LPINT32;
typedef unsigned char       UINT8, *LPUINT8;
typedef unsigned short      UINT16, *LPUINT16;
typedef unsigned int        UINT32, *LPUINT32;

#ifdef WIN32
typedef unsigned __int64	UINT64;
#define AKEY_NODEF_BYTE
#else
typedef unsigned long long	UINT64;
typedef long				HRESULT;
#define AKeyInterface           struct
#endif

// 如果不需要以下类型定义，则在包含本文件前定义AKEY_NODEF_BYTE
#ifndef AKEY_NODEF_BYTE
typedef UINT8 BYTE;
typedef LPUINT8 LPBYTE;
#endif

#ifndef WIN32
typedef unsigned long DWORD;
typedef const char *  LPCSTR;
typedef const void * LPCVOID;
typedef const LPBYTE LPCBYTE;
typedef char CHAR, *LPSTR;
typedef LPCSTR LPCWSTR;

#ifndef LGN_SYSTEMTIME_DECLARE
#define LGN_SYSTEMTIME_DECLARE
typedef struct _SYSTEMTIME {
    UINT16 wYear;
    UINT16 wMonth;
    UINT16 wDayOfWeek;
    UINT16 wDay;
    UINT16 wHour;
    UINT16 wMinute;
    UINT16 wSecond;
    UINT16 wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;
#endif
#endif

//#define LGN_SELECTANY extern

namespace LGN
{
	namespace API
	{
		LGN_INLINE int SafeStringLength(LPCSTR pszStr)
		{
			return (pszStr == NULL)? 0 : ::strlen(pszStr);
		}
        
        LGN_INLINE void GetLocalTime(LPSYSTEMTIME lpSystemTime)
		{
			time_t now;
			struct tm *timenow;
            
			::time(&now);
			timenow = ::localtime(&now);
            
            struct timeval nowtimeval;
            gettimeofday(&nowtimeval,0);
            
			lpSystemTime->wYear = 1900 + timenow->tm_year;
			lpSystemTime->wMonth = timenow->tm_mon + 1;
			lpSystemTime->wDayOfWeek = timenow->tm_wday;
			lpSystemTime->wDay = timenow->tm_mday;
			lpSystemTime->wHour = timenow->tm_hour;
			lpSystemTime->wMinute = timenow->tm_min;
			lpSystemTime->wSecond = timenow->tm_sec;
			lpSystemTime->wMilliseconds = nowtimeval.tv_usec/1000;
		}

		LGN_INLINE DWORD GetCurrentProcessId()
		{
			return (DWORD)getpid(); // unistd.h _UNISTD_H or _UNISTD_H_
		}

		LGN_INLINE DWORD GetCurrentThreadId()
		{
#if (LGN_SUPPORT_MULTITHREAD)
			return (DWORD)pthread_self();
#else
			return (DWORD)0;
#endif

		}
	};

	class ChTraitsExA
	{
	public:
		static LPSTR StringCat(LPSTR pszDest, LPCSTR pszSrc)
		{
			::strcat(pszDest, pszSrc);
			return pszDest;
		}

		static int FormatN(LPSTR lpOut, int cchLimitIn, LPCSTR lpFmt, va_list arglist)
		{
			return ::vsprintf(lpOut, lpFmt, arglist);	// stdio.h		
		}

		static int FormatNP(LPSTR lpOut, int cchLimitIn, LPCSTR lpFmt, ...)
		{
			va_list ptr;
			va_start(ptr, lpFmt);
			int nRet = FormatN(lpOut, cchLimitIn, lpFmt, ptr);
			va_end(ptr);
			return nRet;
			return 0;
		}

		static void SOutputDebugString(LPCSTR pszOutputString)
		{
            fprintf(stderr, "%s", pszOutputString);
            UINT32 u32Length = LGN::API::SafeStringLength(pszOutputString);
            IDCARD::CLog::instance()->AppendProcessLog((LPBYTE)pszOutputString,u32Length);
		}

		//! 获取文件名
		static LPCSTR SPathFindFileName(LPCSTR pszPath)
		{
			for (LPCSTR p=(pszPath + API::SafeStringLength(pszPath) - 1); p>=pszPath; p--)
			{
				if (((*p) == '\\') || ((*p) == '/'))
				{
					return (p + 1);
				}
			}
			return pszPath;
		}

	};
    
    class CCriticalSection
	{
	public:
#if (LGN_SUPPORT_MULTITHREAD)
		CCriticalSection() throw()
		{
			int rv = pthread_mutex_init(&m_mutex, NULL);
			if (rv != 0)
				LgnThrow(API::HResultFromError(rv));
                }
		~CCriticalSection()
		{
			pthread_mutex_destroy(&m_mutex);
		}
        
		void Enter() throw()
		{
			int rv = pthread_mutex_lock(&m_mutex);
			if (rv != 0)
				LgnThrow(API::HResultFromError(rv));
                }
		void Leave() throw()
		{
			int rv = pthread_mutex_unlock(&m_mutex);
			if (rv != 0)
				LgnThrow(API::HResultFromError(rv));
                }
        
		bool TryEnter()
		{
			return( pthread_mutex_trylock(&m_mutex) == 0 );
		}
        
	protected:
		pthread_mutex_t m_mutex;
#else
		void Enter() {}			
		void Leave() {}			
#endif
	};


    //! 为了兼容ATL而重定义类型
	typedef CCriticalSection CAutoCriticalSection;
    
	//! 线程临界区锁定类
	class CCriticalSectionLock
	{
	public:
		//! 构造函数，进入临界区
		CCriticalSectionLock( CAutoCriticalSection& cs) : m_cs(cs)
		{
			m_cs.Enter();
		}
		//! 虚造函数，离开临界区
		~CCriticalSectionLock()
		{
			m_cs.Leave();
		}
        
        // Implementation
	private:
		CAutoCriticalSection& m_cs;
        
        // Private to avoid accidental use
		CCriticalSectionLock( const CCriticalSectionLock& ) throw();
		CCriticalSectionLock& operator=( const CCriticalSectionLock& ) throw();
	};
    
	LGN_SELECTANY CAutoCriticalSection g_criticalSectionForLog;
	class FileTraitsTraceA
	{
	public:
		static HRESULT Append(const char * pszFileName, LPCVOID pData, UINT32 unLength)
		{
            if(pszFileName == NULL)
                return 0;
            
			CCriticalSectionLock _lock(g_criticalSectionForLog);
            
//			FILE * fp = fopen([[[NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) objectAtIndex:0] stringByAppendingPathComponent:[NSString stringWithUTF8String:pszFileName]] UTF8String], "ab+");
//            HRESULT hr = (fwrite(pData, 1, unLength, fp) == unLength) ? 0 : -1;
//            fclose(fp);
            return 0;
		}
	};

};

#include "lgntrace.h"

#endif //__LGNTRACEP_H__
