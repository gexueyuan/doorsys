//
//  IDCardUtil.h
//  TDRToken
//
//  Created by zhaowei on 16/5/5.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardUtil_h
#define IDCardUtil_h

#include "AKeyDef.h"
#include "AKeyHelper.h"
#include "alg/OpenAlg.h"
#include "IDCardConfig.h"
#include "IDCardManager.h"
#include "IDCardError.h"
#include "IDCardTime.h"

#ifdef _WIN32
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define EPOCHFILETIME  116444736000000000Ui64
#else
#define EPOCHFILETIME  116444736000000000ULL
#endif

struct timezone
{
    int tz_minuteswest;
    int tz_dsttime;
};

static int gettimeofday (struct timeval *tv, struct timezone *tz)
{    
    if (tv)
    {
#if 0
		 FILETIME ft;
		LARGE_INTEGER li;
		__int64 t;

		GetSystemTimeAsFileTime (&ft);
        li.LowPart = ft.dwLowDateTime;
        li.HighPart = ft.dwHighDateTime;
        t = li.QuadPart;      /* In 100-nanosecond intervals */
        t -= EPOCHFILETIME;   /* Offset to the Epoch time */
        t /= 10;          /* In microseconds */
        tv->tv_sec = (long) (t / 1000000);
        tv->tv_usec = (long) (t % 1000000);
#else
		time_t clock;
		struct tm tm;
		SYSTEMTIME wtm;

		GetLocalTime(&wtm);
		tm.tm_year     = wtm.wYear - 1900;
		tm.tm_mon     = wtm.wMonth - 1;
		tm.tm_mday     = wtm.wDay;
		tm.tm_hour     = wtm.wHour;
		tm.tm_min     = wtm.wMinute;
		tm.tm_sec     = wtm.wSecond;
		tm. tm_isdst    = -1;
		clock = mktime(&tm);
		tv->tv_sec = clock;
		tv->tv_usec = wtm.wMilliseconds * 1000;
#endif
    }
    
    if (tz)
    {
	    static int tzflag = 0;
        if (!tzflag)
        {
            _tzset ();
            tzflag++;
        }
        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }
    
    return 0;
}
#endif

using namespace AKey;

namespace IDCARD {
    
    class CPackageMaker {
    public:
        CPackageMaker(){}
        ~CPackageMaker(){}
        static UINT32 MakeKVPacket(LPBYTE pbKeyData, UINT32 dwKeyLength,LPBYTE pbValue,UINT32 dwValueLen, LPBYTE pbOutput)
        {
            memcpy(pbOutput, pbKeyData, dwKeyLength);
            memcpy(pbOutput+dwKeyLength, pbValue, dwValueLen);
            return dwKeyLength + dwValueLen;
        }
        
        static UINT32 MakeLVPacket(LPBYTE pbData, UINT32 dwLength, LPBYTE pbOutput)
        {
            Helper::BigEndian::UInt16ToBytes(dwLength, pbOutput);
            memcpy(pbOutput+2, pbData, dwLength);
            return 2 + dwLength;
        }
        
        static int MakeTLVPacket(UINT16 tag,LPBYTE pbValue,UINT32 u32ValueLen,LPBYTE pbOutput)
        {
            Helper::BigEndian::UInt16ToBytes(tag, pbOutput);
            Helper::BigEndian::UInt16ToBytes(u32ValueLen, pbOutput+2);
            memcpy(pbOutput+4, pbValue, u32ValueLen);
            return u32ValueLen+4;
        }
        
        //pu32Offset为偏移量
        static int ParseTLVPacket(UINT16 tag,LPBYTE pbValue,UINT32 u32ValueLen,UINT32* pu32Offset)
        {
            for (UINT32 i = 0;  i< u32ValueLen;) {
                int len = Helper::BigEndian::UInt16FromBytes(pbValue+i+2);
                if (tag == Helper::BigEndian::UInt16FromBytes(pbValue+i)) {
                    if (pu32Offset) {
                        *pu32Offset = i+2+2;
                        return len;
                    }
                }
                i += 2 + 2 + len;
            }
            return -1;
        }
        
        
        static LPBYTE BirthCertFindTLV(LPBYTE pbInput, int nInputLen, UINT16 u16Tag, int * pnLength)
        {
            for (UINT32 i = 0; i<nInputLen; )
            {
                UINT16 nTag_t = Helper::BigEndian::UInt16FromBytes(pbInput+i);
                int nLen_t = Helper::BigEndian::UInt16FromBytes(pbInput+i+2);
                
                if (nTag_t == u16Tag)
                {
                    if (pnLength) {
                        *pnLength = nLen_t;
                    }
                    return (pbInput+i+2+2);
                }
                i += 2 + 2 + nLen_t;
            }
            return NULL;
        }
        
        
    };
    
    class CDecryptIDCard
    {
    public:
        
        static HRESULT EncPackage(LPBYTE pbKey,UINT32 u32KeyLen,LPBYTE pbIv,UINT32 u32IvLen,LPBYTE pbIn,UINT32 u32In,CBuffer& outBuf,bool bPadding)
        {
            HRESULT hr = AKEY_RV_OK;
            CBuffer btTmpBuf(u32In+16);
            UINT32 flag = OpenAlg::CCipher::ENCRYPT;
            if (bPadding == false) {
                flag |= OpenAlg::CCipher::NOPADDING;
            }
            UINT32 EncLen2 = OpenAlg::CCipher::Cipher("SM4-CBC",pbKey,pbIv,flag,pbIn,u32In,btTmpBuf);
            if (EncLen2 <= 0) {
                return AKEY_RV_SM4_ENC_ERROR;
            }
            memcpy(outBuf.ReAllocBytesSetLength(EncLen2), btTmpBuf, EncLen2);
            return hr;
        }
        
        static HRESULT DecPackage(LPBYTE pbKey,UINT32 u32KeyLen,LPBYTE pbIv,UINT32 u32IvLen,LPBYTE pbIn,UINT32 u32In,CBuffer& outBuf)
        {
            HRESULT hr = AKEY_RV_OK;
			CBuffer btTmpBuf(u32In);
            int nTmpLen = OpenAlg::CCipher::Cipher("SM4-CBC",pbKey, pbIv, 0, pbIn, u32In, btTmpBuf);
            if (nTmpLen <=0 ) {
                return AKEY_RV_DEC_ENC_DATA_ERROR;
            }
            memcpy(outBuf.ReAllocBytesSetLength(nTmpLen), btTmpBuf, nTmpLen);
            return hr;
        }
        
        static HRESULT DecIDCardPackage(LPBYTE pbAppKey,LPBYTE pbEncKey,LPBYTE pbIDCard,UINT32 u32IDCardLen,CBuffer& outBuf)
        {
            if (pbIDCard == NULL || u32IDCardLen == 0) {
                return AKEY_RV_PARAM_ERROR;
            }
            LPBYTE pbIV          = pbIDCard+1;
            LPBYTE pbEncData     = pbIDCard+1+16;
            UINT32 u32EncData    = u32IDCardLen -1 -16;
            HRESULT hr = DecIDInfo(pbAppKey,pbEncKey, pbIV, pbEncData, u32EncData,outBuf);
            return hr;
        }
        
        /**
         *  解密图片
         *
         *  @param pbEncKey      加密密钥
         *  @param pbIv          向量
         *  @param pbEncData     加密数据
         *  @param u32EncDataLen 加密数据长度
         *  @param pbOutput      解密输出
         *  @param punOutputLen  解密输出数据长度
         *
         *  @return 错误码
         */
        static HRESULT DecIDInfo(LPBYTE pbAppKey,LPBYTE pbEncKey,LPBYTE pbIv,LPBYTE pbEncData,UINT32 u32EncDataLen,CBuffer& outBuf)
        {
            BYTE bSessionKey[300];
            int u32SessionKeyLen =32;
            LPBYTE pbOutBuf = outBuf.ReAllocBytes(u32EncDataLen);
            u32SessionKeyLen = OpenAlg::CCipher::Cipher("SM4-ECB",(LPBYTE)pbAppKey,NULL,OpenAlg::CCipher::NOPADDING,pbEncKey,16,bSessionKey);
            if (u32SessionKeyLen <= 0)
            {
                return AKEY_RV_DEC_KEY_CIPHER_ERROR;
            }
            int nTmpLen = OpenAlg::CCipher::Cipher("SM4-CBC", (LPBYTE)bSessionKey, pbIv, 0, pbEncData, u32EncDataLen, pbOutBuf);
            if (nTmpLen <=0 ) {
                return AKEY_RV_DEC_ENC_DATA_ERROR;
            }
            BYTE abHash[300];
            OpenAlg::CDigest::Digest(Helper::HashType2Name(AKEY_HASH_SM3), pbOutBuf, nTmpLen-8, abHash);
            if (memcmp(abHash, pbOutBuf+nTmpLen-8, 8) != 0) {
                return AKEY_RV_IDINFO_HASH_NO_MATCH;
            }
            outBuf.SetLength(nTmpLen - 8);
            return AKEY_RV_OK;
        }
    };
    
    
#pragma mark - CLog
    class CLog
    {
    public:
        CLog() {
            m_logBuffer.ReAllocBytes(30*DEFAULT_BUFFERLEN);
            m_netHandleBuff.ReAllocBytes(DEFAULT_BUFFERLEN);
            m_safeComBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
            m_TotalInsBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
            m_TotalNetBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
			m_samVerBuf.ReAllocBytesSetLength(IDCARD_SAM_VER_LEN);
        }
        ~CLog() {}
        
        static CLog* instance()
        {
            static CLog*  m_instance = NULL;
            if (m_instance == NULL) {
                m_instance = new CLog();
            }
            return m_instance;
        }
        
        void CleanLog() {
            m_logBuffer.SetLength(0);
            m_netHandleBuff.SetLength(0);
            m_safeComBuf.SetLength(0);
            m_TotalInsBuf.SetLength(0);
            m_TotalNetBuf.SetLength(0);
			memset(m_samVerBuf.ReAllocBytesSetLength(IDCARD_SAM_VER_LEN),0,IDCARD_SAM_VER_LEN);
        }
        
        void initTimer() {
            m_totalNetTime = 0;
            m_totalInsComTime = 0;
            m_totalTaskTime = 0;
            m_interfaceTime = 0;
			m_connectTime = 0;
			m_disConnectTime = 0;
            m_requestPortTime = 0;
            m_releasePortTime = 0;
        }
        
        void initCount()
        {
            m_u16ReaderRetryCount = 0;
            m_u16BTRetryCount = 0;
            m_u16clientTCPNetRetryCount = 0;
			m_u16clientUDPNetRetryCount = 0;
			m_u16serverTCPNetRetryCount = 0;
			m_u16serverUDPNetRetryCount = 0;
        }
        
        void AddNetTime(UINT32 u32NetTime)
        {
            m_totalNetTime += u32NetTime;
        }
        
        void AppendNetLengthAndTime(UINT16 u16SendLen,UINT16 u16RecvLen,UINT16 u16Time,IDCARD_NET_TYPE netType,IDCARD_SERVER_TYPE serverType)
        {
            if ((m_TotalNetBuf.GetLength() + 8) > m_TotalNetBuf.GetAllocLength()) {
                m_TotalNetBuf.ReAllocBytes(2*m_TotalNetBuf.GetAllocLength());
            }
            BYTE buf[2];
			UINT32 u32Offset = 0;
			LPBYTE pbTotalBuf = m_TotalNetBuf.GetBuffer() + m_TotalNetBuf.GetLength();
            Helper::BigEndian::UInt16ToBytes(u16SendLen, buf);
            memcpy(pbTotalBuf, buf, 2);
			u32Offset+=2;
            Helper::BigEndian::UInt16ToBytes(u16RecvLen, buf);
            memcpy(pbTotalBuf+u32Offset, buf, 2);
			u32Offset+=2;
            Helper::BigEndian::UInt16ToBytes(u16Time, buf);
            memcpy(pbTotalBuf+u32Offset, buf, 2);
			u32Offset +=2;
			memcpy(pbTotalBuf+u32Offset, &netType, 1);
			u32Offset+=1;
			memcpy(pbTotalBuf+u32Offset, &serverType, 1);
			u32Offset+=1;
            m_TotalNetBuf.SetLength(m_TotalNetBuf.GetLength()+u32Offset);
        }
        
        void AddInsComTime(UINT32 u32InsTime)
        {
            m_totalInsComTime += u32InsTime;
        }
        
        UINT32 GetTotalNetTime()
        {
            return m_totalNetTime;
        }
        
        UINT32 GetInsTime()
        {
            return m_totalInsComTime;
        }
        
        UINT32 getTotalTaskTime()
        {
            return m_totalTaskTime;
        }
        
        void setTotalTaskTime(UINT32 u32TotalTime)
        {
            m_totalTaskTime = u32TotalTime;
        }
        
        void setInterfaceTime(UINT32 u32InterceTime)
        {
            m_interfaceTime = u32InterceTime;
        }
        
        UINT32 getInterfaceTime()
        {
            return m_interfaceTime;
        }
        
        
//        void AppendTimeLog(PROCESS_STATUS status,UINT32 timeSpend)
//        {
//            if (m_u32Log >= LOG_BUFFERLEN) {
//                return;
//            }
//            BYTE timeBuf[4] = {0};
//            Helper::BigEndian::UInt32ToBytes(timeSpend, timeBuf);
//            LPBYTE pbLogBuf = m_logBuffer;
//            BYTE kBuff[2] = {0};
//            Helper::BigEndian::UInt16ToBytes(status, kBuff);
//            m_u32Log += CPackageMaker::MakeKVPacket((LPBYTE)kBuff, 2, (LPBYTE)timeBuf, 4, pbLogBuf+m_u32Log);
//        }
        
//        void AppendMultiTimeLog(LPBYTE pbLog,UINT32 u32LogLen)
//        {
//            if (m_u32Log >= LOG_BUFFERLEN) {
//                return;
//            }
//            memcpy(m_logBuffer+m_u32Log, pbLog, u32LogLen);
//            m_u32Log+=u32LogLen;
//        }
        
        void AppendProcessLog(LPBYTE pbData,UINT32 u32DataLen)
        {
            if ((m_logBuffer.GetLength() + u32DataLen) > m_logBuffer.GetAllocLength()) {
                UINT32 u32Offset = m_logBuffer.GetLength()/2;
                UINT32 u32Len = m_logBuffer.GetLength() - u32Offset;
                memmove(m_logBuffer.GetBuffer(), m_logBuffer.GetBuffer()+u32Offset,u32Len);
                m_logBuffer.SetLength(u32Len);
            }
            memcpy(m_logBuffer.GetBuffer() + m_logBuffer.GetLength(), pbData, u32DataLen);
            m_logBuffer.SetLength(m_logBuffer.GetLength()+u32DataLen);
        }
        
        HRESULT WriteCacheToFile()
        {
            char* errFile = ParamManager::instance()->getErrFilePath();
            if (strlen(errFile) == 0) {
                return AKEY_RV_PARAM_ERR_FILE_NAME;
            }
            IIDCardFile* fileInterface = ParamManager::instance()->getFileInterface();
            HRESULT hr = fileInterface->Write(errFile,m_logBuffer);
            return hr;
        }
        
        void AppendNetSAM(LPBYTE pbData,UINT32 u32DataLen)
        {
            if ((m_netHandleBuff.GetLength() + u32DataLen) > m_netHandleBuff.GetAllocLength()) {
                m_netHandleBuff.ReAllocBytes(2*m_netHandleBuff.GetAllocLength());
            }
            memcpy(m_netHandleBuff.GetBuffer() + m_netHandleBuff.GetLength(), pbData, u32DataLen);
            m_netHandleBuff.SetLength(m_netHandleBuff.GetLength()+u32DataLen);
        }
        
        void AppendSafeComSAM(LPBYTE pbData,UINT32 u32DataLen)
        {
            if ((m_safeComBuf.GetLength() + u32DataLen) > m_safeComBuf.GetAllocLength()) {
                m_safeComBuf.ReAllocBytes(2*m_safeComBuf.GetAllocLength());
            }
            memcpy(m_safeComBuf.GetBuffer() + m_safeComBuf.GetLength(), pbData, u32DataLen);
            m_safeComBuf.SetLength(m_safeComBuf.GetLength()+u32DataLen);
        }
    
        void addComTime(LPBYTE pbCmdData,UINT32 u32CmdLen,UINT32 u32CmdTime)
        {
            if ((m_TotalInsBuf.GetLength() + (u32CmdLen + 4)) > m_TotalInsBuf.GetAllocLength()) {
                m_TotalInsBuf.ReAllocBytes(2*m_TotalInsBuf.GetAllocLength());
            }
            BYTE btCmdTimeBuf[4];
            memcpy(m_TotalInsBuf.GetBuffer() + m_TotalInsBuf.GetLength(), pbCmdData, u32CmdLen);
            Helper::BigEndian::UInt32ToBytes(u32CmdTime, btCmdTimeBuf);
            memcpy(m_TotalInsBuf.GetBuffer() + m_TotalInsBuf.GetLength() + u32CmdLen, btCmdTimeBuf, 4);
            m_TotalInsBuf.SetLength(m_TotalInsBuf.GetLength() + u32CmdLen+ 4);
        }
        
        void setRequestPortTime(UINT16 u16ReqPortTime)
        {
            m_requestPortTime = u16ReqPortTime;
        }
        
        void setRelPortTime(UINT16 u16RelPortTime)

        {
            m_releasePortTime = u16RelPortTime;
        }
        
        UINT16 getReqPortTime()
        {
            return m_requestPortTime;
        }
        
        UINT16 getRelPortTime()
        {
            return m_releasePortTime;
        }

        void addReaderRetryCount()
        {
            m_u16ReaderRetryCount++;
        }
        
        void addBTRetryCount()
        {
            m_u16BTRetryCount++;
        }
        
        void addClientTCPNetRetryCount()
        {
            m_u16clientTCPNetRetryCount++;
        }

		void addClientUDPNetRetryCount()
		{
			m_u16clientUDPNetRetryCount++;
		}

		void addServerTCPNetRetryCount(UINT16 u16Count)
		{
			m_u16serverTCPNetRetryCount += u16Count;
		}

		void addServerUDPNetRetryCount(UINT16 u16Count)
		{
			m_u16serverUDPNetRetryCount += u16Count;
		}
        
        UINT16 getReaderRetryCount()
        {
            return m_u16ReaderRetryCount;
        }
        
        UINT16 getBTRetryCount()
        {
            return m_u16BTRetryCount;
        }
        
        UINT16 getClientTCPNetRetryCount()
        {
            return m_u16clientTCPNetRetryCount;
        }

		UINT16 getClientUDPNetRetryCount()
		{
			return m_u16clientUDPNetRetryCount;
		}

		UINT16 getServerTCPNetRetryCount()
		{
			return m_u16serverTCPNetRetryCount;
		}

		UINT16 getServerUDPNetRetryCount()
		{
			return m_u16serverUDPNetRetryCount;
		}

		void addConnectTime(UINT16 u32ConnectTime)
		{
			m_connectTime += u32ConnectTime;
		}

		void addDisConnect(UINT16 u32DisConnectTime)
		{
			m_disConnectTime += u32DisConnectTime;
		}

		UINT16 getConnectTime()
		{
			return m_connectTime;
		}

		UINT16 getDisConnectTime()
		{
			return m_disConnectTime;
		}
        
#pragma mark Buf
        
        const CBuffer& getLogBuf()
        {
            return m_logBuffer;
        }
        
        const CBuffer& getTotalInsTime()
        {
            return m_TotalInsBuf;
        }
        
        const CBuffer& getNetHandleBuf()
        {
            return m_netHandleBuff;
        }
        
        const CBuffer& getSafeComBuf()
        {
            return m_safeComBuf;
        }
        
        const CBuffer& getTotalNetTimeBuf()
        {
            return m_TotalNetBuf;
        }

		const CBuffer& getSamVerBuf()
		{
			return m_samVerBuf;
		}

		void getSamVerBuf(CBuffer& samVerBuf)
		{
			memcpy(samVerBuf.ReAllocBytesSetLength(m_samVerBuf.GetLength()),m_samVerBuf.GetBuffer(),m_samVerBuf.GetLength());
		}
        
        void getTotalInsTime_Ex(CBuffer& buf)
        {
            memcpy(buf.ReAllocBytesSetLength(m_TotalInsBuf.GetLength()), m_TotalInsBuf.GetBuffer(), m_TotalInsBuf.GetLength());
        }
        
        void getNetHandleBuf_Ex(CBuffer& buf)
        {
            memcpy(buf.ReAllocBytesSetLength(m_netHandleBuff.GetLength()), m_netHandleBuff.GetBuffer(), m_netHandleBuff.GetLength());
        }
        
        void getSafeComBuf_Ex(CBuffer& buf)
        {
            memcpy(buf.ReAllocBytesSetLength(m_safeComBuf.GetLength()), m_safeComBuf.GetBuffer(), m_safeComBuf.GetLength());
        }

		void setSamVer(LPBYTE pbSamVer,UINT32 u32SamVerLen)
		{
			memcpy(m_samVerBuf.ReAllocBytesSetLength(u32SamVerLen),pbSamVer,u32SamVerLen);
		}
        
    private:
        CBuffer m_logBuffer;
        CBuffer m_netHandleBuff;  //网卡模组应用处理时间
        CBuffer m_safeComBuf;     //与安全模块通讯时间
        CBuffer m_TotalInsBuf;    //统计指令的数据和时间
        CBuffer m_TotalNetBuf;    //统计网络的收发数据长度和时间
		CBuffer m_samVerBuf;	  //网卡模组编号
        UINT32  m_totalNetTime;   //所有网络通信时间和
        UINT32  m_totalInsComTime;//所有指令通信时间和
        UINT32  m_totalTaskTime;  //读取身份证数据总时间
        UINT32  m_interfaceTime;  //接口时间
        UINT32  m_disConnectTime; //连接服务器时间
		UINT32  m_connectTime;    //断开服务器连接时间
        
        UINT16  m_requestPortTime;//申请端口时间
        UINT16  m_releasePortTime;//释放端口时间
        
        //BYTE  m_curRetryCount;   //重试次数的计数
        UINT16  m_u16ReaderRetryCount;//读卡器重试次数
        UINT16  m_u16BTRetryCount;    //蓝牙重试次数(蓝牙超时，蓝牙通信失败)
        UINT16  m_u16clientTCPNetRetryCount;   //TCP网络重试次数(客户端)
		UINT16  m_u16clientUDPNetRetryCount;   //UDP网络重试次数（客户端）
		UINT16  m_u16serverTCPNetRetryCount;   //TCP网络重试次数（服务器端）
		UINT16  m_u16serverUDPNetRetryCount;   //UDP网络重试次数（服务器端）
    };

	class IDCardUtil {
	public:
		IDCardUtil() {}
		~IDCardUtil(){}

		static IDCardUtil* instance()
		{
			static  IDCardUtil* _instance = NULL;
			if (_instance == NULL) {
				_instance = new IDCardUtil();
			}
			return _instance;
		}

		static void CreateUniqueCode(const CBuffer& snBuf,CBuffer& resultBuf)
		{
			static BYTE bCount = 0;
			BYTE tmpBuf1[16] = {0},tmpBuf2[16] = {0},encBuf[16] = {0};
			memcpy(tmpBuf1,snBuf.GetBuffer(),snBuf.GetLength());
			Helper::Convert::HexStringToBytes(tmpBuf1,16,tmpBuf2);
			tmpBuf2[7] = bCount++;//加上计数值
			struct timeval stTimeval;
			gettimeofday(&stTimeval, NULL);
			Helper::BigEndian::UInt32ToBytes(stTimeval.tv_sec,tmpBuf2+8);
			Helper::BigEndian::UInt32ToBytes(stTimeval.tv_usec,tmpBuf2+12);
			LPBYTE pbDefKey = (LPBYTE)IDCARD_DEFAULT_ENC_KEY;
			CDecryptIDCard::EncPackage(pbDefKey, 16, (LPBYTE)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16, tmpBuf2,16, resultBuf,false);
		}


		static bool compareBuf(CBuffer& buf1,CBuffer& buf2)
		{
			if (buf1.GetLength() == buf2.GetLength()) {
				LPBYTE pbBuf1 = buf1.GetBuffer();
				LPBYTE pbBuf2 = buf2.GetBuffer();
				if (memcmp(pbBuf1, pbBuf2, buf1.GetLength()) == 0) {
					return true;
				}else {
					return false;
				}
			}else {
				return false;
			}
		}

		static bool compareBufAndData(CBuffer& buf1,LPBYTE pbData,UINT32 u32DataLen)
		{
			if (buf1.GetLength() == u32DataLen) {
				if (memcmp(buf1.GetBuffer(),pbData,u32DataLen) == 0) {
					return true;
				}else {
					return false;
				}
			}else {
				return false;
			}
		}

		bool checkTimeOut()
		{
			UINT32 u32TotalTime = IDWORK_TIMEOUT;
			int nTimeCfg = ParamManager::instance()->getBusTimeOut();
			if (nTimeCfg == -1) {//无超时
				return true;
			} else if (nTimeCfg > 0) {
				u32TotalTime = nTimeCfg*1000;//毫秒
			}
			if (m_time.timeSpend() < u32TotalTime) {
				return true;
			}
			return false;
		}

		void Timebegin()
		{
			m_time.Begin();
		}

	private:
		CTime m_time;
	};
}


#endif /* IDCardUtil_h */
