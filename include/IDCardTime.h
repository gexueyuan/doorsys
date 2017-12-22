//
//  IDCardTime.h
//  TDRToken
//
//  Created by zhaowei on 16/5/5.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardTime_h
#define IDCardTime_h

#ifdef _WIN32
#include <MMSystem.h>
#include "IDCardUtil.h"
#else
#include <sys/time.h>
#endif

namespace IDCARD {
#ifdef WIN32
	class CTime
	{
	public:
		CTime()
		{
			QueryPerformanceFrequency(&m_nFreq);
		}

		void Begin()
		{
			QueryPerformanceCounter(&m_nBeginTime); 
		}

		void AllBegin()
		{
			QueryPerformanceCounter(&m_nAllBeginTime); 
		}

		double timeSpend()
		{
			LARGE_INTEGER nEndTime;
			QueryPerformanceCounter(&nEndTime);
			double time=(double)((nEndTime.QuadPart-m_nBeginTime.QuadPart) * 1000) / (double)m_nFreq.QuadPart;
			return time;//(long)time
		}

// 		double timeSpend(timeval time1,timeval time2)
// 		{
// 			double timeuse;
// 			timeuse=1000000*(time2.tv_sec-time1.tv_sec)+time2.tv_usec-time1.tv_usec;  // 计算执行时间，以微秒为单位进行计算
// 			timeuse/=1000;
// 			return timeuse;
// 		}

		//sleep 毫秒
		static void SleepTime(UINT32 u32MilTime)
		{
			Sleep(u32MilTime);
		}

		double timeAllSpend()
		{
			LARGE_INTEGER nEndTime;
			QueryPerformanceCounter(&nEndTime);
			double time=(double)((nEndTime.QuadPart-m_nAllBeginTime.QuadPart) * 1000) / (double)m_nFreq.QuadPart;
			return time;//(long)time
		}

	private:
		LARGE_INTEGER m_nFreq;
		LARGE_INTEGER m_nBeginTime, m_nAllBeginTime;
	};
#else
    class CTime
    {
    public:
        CTime()
        {
            
        }
        void Begin()
        {
            gettimeofday(&m_tpstart,NULL); // 开始时间
        }
        
        void AllBegin()
        {
            gettimeofday(&m_tAllStart,NULL); // 开始时间
        }
        
        double timeSpend()
        {
            double timeuse;
            timeval t_end;
            gettimeofday(&t_end,NULL);   // 结束时间
            timeuse=1000000*(t_end.tv_sec-m_tpstart.tv_sec)+t_end.tv_usec-m_tpstart.tv_usec;  // 计算执行时间，以微秒为单位进行计算
            timeuse/=1000;
            return timeuse;
        }
        
        double timeSpend(timeval time1,timeval time2)
        {
            double timeuse;
            timeuse=1000000*(time2.tv_sec-time1.tv_sec)+time2.tv_usec-time1.tv_usec;  // 计算执行时间，以微秒为单位进行计算
            timeuse/=1000;
            return timeuse;
        }
        
        double timeAllSpend()
        {
            double timeuse;
            timeval t_end;
            gettimeofday(&t_end,NULL);   // 结束时间
            timeuse=1000000*(t_end.tv_sec-m_tAllStart.tv_sec)+t_end.tv_usec-m_tAllStart.tv_usec;  // 计算执行时间，以微秒为单位进行计算
            timeuse/=1000;
            return timeuse;
        }

		static void SleepTime(UINT32 u32MilTime)
		{
			usleep(u32MilTime*1000);
		}
        
    private:
        struct timeval m_tpstart;
        struct timeval m_tAllStart;
    };
#endif
    
    
    class CTimeCount
    {
    public:
        CTimeCount()
        {
            m_pbBuf = m_buff.ReAllocBytes(LOG_BUFFERLEN);
            m_u32BufLen = 0;
        }
        ~CTimeCount()
        {
            
        }
        
        void AppendLog(LPBYTE pbData,UINT32 u32DataLen)
        {
            if (m_u32BufLen + u32DataLen > LOG_BUFFERLEN) {
                return;
            }
            for (int j = 0;  j < u32DataLen; j+=8) {
                int i = 0;
                UINT16 tag1 = Helper::BigEndian::UInt16FromBytes(pbData + j);
                for (;i < m_u32BufLen; i+=6) {
                    UINT16 tag2 = Helper::BigEndian::UInt16FromBytes(m_pbBuf+i);
                    if (tag1 == tag2) {
                        UINT32 value1 = Helper::BigEndian::UInt32FromBytes(pbData+j+4);
                        UINT32 value2 = Helper::BigEndian::UInt32FromBytes(m_pbBuf+i+2);
                        Helper::BigEndian::UInt32ToBytes((value1+value2), m_pbBuf+i+2);
                        break;
                    }
                }
                if (i == m_u32BufLen) {
                    memcpy(m_pbBuf+m_u32BufLen, pbData +j, 2);
                    memcpy(m_pbBuf+m_u32BufLen+2, pbData+ j + 4, 4);
                    m_u32BufLen+=6;
                }
            }
        }
        void CleanLog()
        {
            m_u32BufLen = 0;
        }
        
        LPBYTE getBuf(UINT32* punBufLen)
        {
            if (punBufLen) {
                *punBufLen = m_u32BufLen;
            }
            return m_pbBuf;
        }
    private:
        CBuffer m_buff;
        LPBYTE m_pbBuf;
        UINT32 m_u32BufLen;
    };
}


#endif /* IDCardTime_h */
