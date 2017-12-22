//
//  IDCardInfo.h
//  TDRToken
//
//  Created by zhaowei on 16/5/16.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardInfo_h
#define IDCardInfo_h

#include "IDCardTime.h"

typedef enum IDCARD_DEVICE_TYPE
{
    DEVICE_TYPE_NATIVE = 0,
    DEVICE_TYPE_JAVA = 1
}IDCARD_DEVICE_TYPE;

namespace IDCARD {
    class IDCardInfo
    {
    public:
        IDCardInfo(){}
        ~IDCardInfo(){}
        
        static IDCardInfo* instance()
        {
            static IDCardInfo* _instance = NULL;
            if (_instance == NULL) {
                _instance = new IDCardInfo();
            }
            return _instance;
        }
        
        HRESULT PrepareWork(CBuffer& snBuf,CBuffer& cosBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            hr = ReaderCardCert(m_readerCertBuf);
            if(hr != AKEY_RV_OK) {
                return hr;
            }
            hr = ReadSNAndCosVer(snBuf,cosBuf);
            return hr;
        }
        
        HRESULT GetBatteryLevel(BYTE* bBattery)
        {
            HRESULT hr = AKEY_RV_OK;
            CBuffer resultBuf;
            hr = Read_BITMAP((LPBYTE)"\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00", 10, resultBuf);
            if (hr == AKEY_RV_OK && resultBuf.GetLength() > 8) {
                if (bBattery) {
                    LPBYTE pbResult = resultBuf.GetBuffer();
                    UINT32 v1 = Helper::LittleEndian::UInt32FromBytes(pbResult);
                    if (v1 & (1<<14))
                    {
                        *bBattery = pbResult[8];
                        return AKEY_RV_OK;
                    }else {
                        return AKEY_RV_NOT_SUPPORT_BATTERY;
                    }
                }
            }else  {
                return AKEY_RV_NOT_SUPPORT_BATTERY;
            }
            return hr;

        }
        
        HRESULT ReadCardInfo(CBuffer& snBuf,CBuffer& cosBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            IIDCardReaderInterface* pInterface = ParamManager::instance()->getReaderInterface();
            if (!pInterface) {
                return AKEY_RV_TOKEN_INVALID_PARAM;
            }
            hr = pInterface->Prepare();
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            BYTE bRetBuf[DEFAULT_BUFFERLEN];
            UINT32 u32RetLen = DEFAULT_BUFFERLEN;
            hr = pInterface->GetReaderCert(bRetBuf, &u32RetLen);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            int readerCertLen = bRetBuf[1];
            memcpy(m_readerCertBuf.ReAllocBytesSetLength(readerCertLen),bRetBuf+2,readerCertLen);
            int sslCertLen = bRetBuf[2+readerCertLen+1];
            memcpy(m_sslCertBuf.ReAllocBytesSetLength(sslCertLen),bRetBuf+2+readerCertLen+2,sslCertLen);
            
            BYTE bCosVer[200]={0};
            UINT32 u32CosVerLen = 200;
            hr = pInterface->GetReaderInfo(bCosVer, &u32CosVerLen);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            UINT32 u32Offset = 0;
            int nLen = CPackageMaker::ParseTLVPacket(0x0004,m_readerCertBuf.GetBuffer(),m_readerCertBuf.GetLength(),&u32Offset);
            if (nLen <= 0)
            {
                return AKEY_RV_TOKEN_INVALID_PARAM;
            }
            memcpy(m_snBuf.ReAllocBytesSetLength(nLen),m_readerCertBuf.GetBuffer()+u32Offset,nLen);
            memcpy(m_cosVerBuf.ReAllocBytesSetLength(u32CosVerLen),bCosVer,u32CosVerLen);
            
            memcpy(snBuf.ReAllocBytesSetLength(nLen),m_readerCertBuf.GetBuffer()+u32Offset,nLen);
            memcpy(cosBuf.ReAllocBytesSetLength(u32CosVerLen),bCosVer,u32CosVerLen);
            
            BYTE bSupportResume = 0;
            hr = pInterface->SupportResume(&bSupportResume);
            if (hr == AKEY_RV_OK) {
                if (bSupportResume == 0x01) {
                    ParamManager::instance()->setResumeFlag(true);
                }else {
                    ParamManager::instance()->setResumeFlag(false);
                }
            }else {
                ParamManager::instance()->setResumeFlag(false);
            }
            return hr;
        }
        
        HRESULT ReadCosLog(CBuffer& logBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            hr = Reader_Execute((LPBYTE)"\xF0\xF4\x01\x11\x00", 5,logBuf);
            return hr;
        }
        
        HRESULT CloseReader()
        {
            CBuffer recvBuf;
            return Reader_Execute((LPBYTE)"\x00\x9A\x04\x00\x00", 5, recvBuf);
        }
        
        HRESULT FindCard(BYTE* flag)
        {
            HRESULT hr = AKEY_RV_OK;
            CBuffer recvBuf;
            hr = Reader_Execute((LPBYTE)"\xF0\xF4\x01\x01\x00", 5,recvBuf);
            if (hr == AKEY_RV_OK) {
                if (flag)
                {
                    LPBYTE pbBuf = recvBuf.GetBuffer();
                    *flag = pbBuf[0];
                }
            }
            return hr;
        }
        
        HRESULT GetBattery(BYTE* battery)
        {
            HRESULT hr = AKEY_RV_OK;
            IIDCardReaderInterface* pInterface = ParamManager::instance()->getReaderInterface();
            if (pInterface == NULL) {
                return AKEY_RV_NO_INIT;
            }
            hr = pInterface->GetBatteryLevel(battery);
            return hr;
        }

		HRESULT UnBindDevice()
		{
			HRESULT hr = AKEY_RV_OK;
			CBuffer recvBuf;
			hr = Reader_Execute((LPBYTE)"\xF0\xF4\x81\x21\x00", 5,recvBuf);
			return hr;
		}
        
        HRESULT GetReaderCosInfo(CBuffer& timeInterval,CBuffer& batteryHealth,CBuffer& readerBehavior)
        {
            HRESULT hr = AKEY_RV_OK;
            IIDCardReaderInterface* pInterface = ParamManager::instance()->getReaderInterface();
            if (pInterface == NULL) {
                return AKEY_RV_NO_INIT;
            }
            hr = pInterface->GetReaderCosInfo(timeInterval,batteryHealth,readerBehavior);
            return hr;
        }
        
        HRESULT Transmit(LPBYTE pbSend, UINT32 unSLen,CBuffer& outBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "IDCardInfo Transmit");
            CTime cmdTime;
            cmdTime.Begin();
            IIDCardReaderInterface* pInterface = ParamManager::instance()->getReaderInterface();
            if (pInterface == NULL)
            {
                return AKEY_RV_NO_INIT;
            }
            LGNTRACE_HEX("Transmit Send:", pbSend, unSLen);
            LPBYTE pbRecvBuf = outBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
            UINT32 u32RecvLen = DEFAULT_BUFFERLEN;
            HRESULT hr = pInterface->Transmit(pbSend, unSLen, 0, pbRecvBuf, &u32RecvLen);
            if (hr != AKEY_RV_OK)
            {
                LGNTRACE_ERRORNO(hr);
                return hr;
            }
            LGNTRACE_HEX("Transmit Recv:", pbRecvBuf, u32RecvLen);
            outBuf.SetLength(u32RecvLen);
            CLog::instance()->AddInsComTime(cmdTime.timeSpend());
            CLog::instance()->addComTime(pbSend, (unSLen>4)?4:unSLen, cmdTime.timeSpend());
            return hr;
        }
        
        
        HRESULT Reader_Execute(LPBYTE pbSend, UINT32 unSLen,CBuffer& outBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Reader_Execute");
            CTime cmdTime;
            cmdTime.Begin();
            IIDCardReaderInterface* pInterface = ParamManager::instance()->getReaderInterface();
            if (pInterface == NULL)
            {
                return AKEY_RV_NO_INIT;
            }
            LGNTRACE_HEX("Reader Send:", pbSend, unSLen);
            LPBYTE pbRecvBuf = outBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
            UINT32 u32RecvLen = DEFAULT_BUFFERLEN;
            HRESULT hr = pInterface->Transmit(pbSend, unSLen, 0, pbRecvBuf, &u32RecvLen);
            if (hr != AKEY_RV_OK)
            {
                LGNTRACE_ERRORNO(hr);
                return hr;
            }
            LGNTRACE_HEX("Reader Recv:", pbRecvBuf, u32RecvLen);
            UINT32 status = (u32RecvLen < 2)? 0xFFFF : ((pbRecvBuf[u32RecvLen-2] << 8) + pbRecvBuf[u32RecvLen-1]);
            if (status != 0x9000 && status != 0x900A)
            {
                LGNTRACE_ERRORNO(status);
                return AKEY_RV_SW_BASE + status;
            }
            u32RecvLen -= 2;
            outBuf.SetLength(u32RecvLen);
            CLog::instance()->AddInsComTime(cmdTime.timeSpend());
            CLog::instance()->addComTime(pbSend, (unSLen>4)?4:unSLen, cmdTime.timeSpend());
            if (status == 0x900A) {
                if (ParamManager::instance()->getFinishFlag() == false) {
                    fnreadCardFinish finishCb = NULL;
                    finishCb = ParamManager::instance()->getFinishCallBack();
                    if (finishCb) {
                        finishCb();
                    }
                    ParamManager::instance()->setFinishFlag(true);
                }
            }
            return AKEY_RV_OK;
        }
        
        //len[2] + CMD[len] (LV结构)
        HRESULT Reader_Execute_Mul(LPBYTE pbSend, UINT32 unSLen,CBuffer& outBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Reader_Execute_Mul");
            HRESULT hr = AKEY_RV_OK;
            UINT32 u32LC = 0;
            LPBYTE pbBuf = outBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
            UINT32 u32Offset = 0;
            CBuffer cmdResultBuf;
            for (int i = 0 ; i < unSLen; i+=2+u32LC) {
                u32LC = Helper::BigEndian::UInt16FromBytes(pbSend+i);
                hr = Reader_Execute(pbSend+i+2, u32LC,cmdResultBuf);
                if (hr != AKEY_RV_OK) {
                    return hr;
                }
                Helper::BigEndian::UInt16ToBytes(cmdResultBuf.GetLength(), pbBuf+u32Offset);
                u32Offset+=2;
                memcpy(pbBuf+u32Offset, cmdResultBuf.GetBuffer(), cmdResultBuf.GetLength());
                u32Offset+=cmdResultBuf.GetLength();
            }
            outBuf.SetLength(u32Offset);
            return hr;
        }
        
        const CBuffer& getReaderCert()
        {
            return m_readerCertBuf;
        }
        
        HRESULT LoadSamCertAndVer(CBuffer& samVersion,CBuffer& samCert)
        {
            HRESULT hr = AKEY_RV_OK;
            IIDCardReaderInterface* pInterface = ParamManager::instance()->getReaderInterface();
            if (pInterface == NULL)
            {
                return AKEY_RV_NO_INIT;
            }
            hr = pInterface->LoadSamCertAndVersion(samVersion,samCert);
            return hr;
        }
        
        HRESULT SaveCertVer(CBuffer& samVersion,CBuffer& samCert)
        {
            HRESULT hr = AKEY_RV_OK;
            IIDCardReaderInterface* pInterface = ParamManager::instance()->getReaderInterface();
            if (pInterface == NULL)
            {
                return AKEY_RV_NO_INIT;
            }
            hr = pInterface->SaveSamCertAndVersion(samVersion,samCert);
            return hr;
        }
        
        void getReaderCert(CBuffer& certBuf)
        {
            memcpy(certBuf.ReAllocBytesSetLength(m_readerCertBuf.GetLength()), m_readerCertBuf.GetBuffer(), m_readerCertBuf.GetLength());
        }
        
        const CBuffer& getSSLCert()
        {
            return m_sslCertBuf;
        }
        
        const CBuffer& getSN()
        {
            return m_snBuf;
        }
        
        const CBuffer& getCosVer()
        {
            return m_cosVerBuf;
        }
        
        const CBuffer& getAppKey()
        {
            return m_appKey;
        }
        
        const CBuffer& getAppKeyHash()
        {
            return m_appKeyHash;
        }
        
        void setAppKey(LPBYTE pbAppKey,UINT32 u32AppKeyLen)
        {
            memcpy(m_appKey.ReAllocBytesSetLength(u32AppKeyLen), pbAppKey, u32AppKeyLen);
        }
        
        void setAppKeyHash(LPBYTE pbAppKeyHash,UINT32 u32AppKeyHashLen)
        {
            memcpy(m_appKeyHash.ReAllocBytesSetLength(u32AppKeyHashLen), pbAppKeyHash, u32AppKeyHashLen);
        }
        
        void cleanAppKeyHash()
        {
            m_appKeyHash.SetLength(0);
        }
        
    protected:
        HRESULT ReaderCardCert(CBuffer& readerCertBuf)
        {
            HRESULT hr = AKEY_RV_OK;
			IIDCardReaderInterface* pInterface = ParamManager::instance()->getReaderInterface();
			if (pInterface == NULL)
			{
				return AKEY_RV_NO_INIT;
			}
            CBuffer resultBuf;
            hr = pInterface->ChooseApp();
            if (hr != AKEY_RV_OK) {
                m_deviceType = DEVICE_TYPE_NATIVE;
            }else {
                m_deviceType = DEVICE_TYPE_JAVA;
            }
            switch (m_deviceType) {
                case DEVICE_TYPE_NATIVE:
                {
					hr = Reader_Execute((LPBYTE)"\x00\xA4\x00\x00\x02\xDF\x20", 7,resultBuf);
                    hr = Reader_Execute((LPBYTE)"\x00\xA4\x00\x00\x02\xA3\x16", 7,resultBuf);
                    if (hr != AKEY_RV_OK) {
                        return hr;
                    }
                    hr =  Reader_Execute((LPBYTE)"\x00\xB0\x02\x00\xD0", 5,resultBuf);
                    if (hr != AKEY_RV_OK) {
                        return hr;
                    }
                    if (hr != AKEY_RV_OK) {
                        return hr;
                    }
                    LPBYTE pbResult = resultBuf.GetBuffer();
                    memcpy(readerCertBuf.ReAllocBytesSetLength(pbResult[1]), pbResult+2, pbResult[1]);
                    break;
                }
                case DEVICE_TYPE_JAVA:
                {
                    hr = Reader_Execute((LPBYTE)"\x80\xCA\xCE\x41\x04\x00\x00\x00\xD0", 9,readerCertBuf);
                    if (hr != AKEY_RV_OK) {
                        return hr;
                    }
                    break;
                }
                default:
                    hr = AKEY_RV_TOKEN_INVALID_PARAM;
                    break;
            }
            return hr;
        }
        
        HRESULT ReadSNAndCosVer(CBuffer& snBuf,CBuffer& cosBuf)
        {
            HRESULT hr = AKEY_RV_OK; 
            CBuffer resultBuf;
            hr = Reader_Execute((LPBYTE)"\xF0\xF6\x02\x00\x00", 5, resultBuf);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            if (m_deviceType == DEVICE_TYPE_NATIVE) {
                memcpy(m_cosVerBuf.ReAllocBytesSetLength(resultBuf.GetLength()),resultBuf.GetBuffer(),resultBuf.GetLength());
                memcpy(cosBuf.ReAllocBytesSetLength(resultBuf.GetLength()),resultBuf.GetBuffer(),resultBuf.GetLength());
            }else {
				LPBYTE btVersion = resultBuf.GetBuffer();
				UINT32 u32Version = resultBuf.GetLength();
                int nCount = 0;
                LPBYTE pbBegin = NULL;
                LPBYTE pbEnd = NULL;
                for (unsigned int i = 0; i < u32Version; i++) {
                    if (btVersion[i] == 0x2C) {
                        nCount++;
                        if (nCount == 2) {
                            pbBegin = btVersion + i + 1;
                        }
                        if (nCount == 3) {
                            pbEnd = btVersion + i;
                            break;
                        }
                    }
                }
                if (nCount == 3) {
                    memcpy(m_cosVerBuf.ReAllocBytesSetLength(pbEnd - pbBegin),pbBegin,pbEnd - pbBegin);
                    memcpy(cosBuf.ReAllocBytesSetLength(pbEnd - pbBegin),pbBegin,pbEnd - pbBegin);
                }
            }
            UINT32 u32Offset = 0;
            int nLen = CPackageMaker::ParseTLVPacket(0x0004,m_readerCertBuf.GetBuffer(),m_readerCertBuf.GetLength(),&u32Offset);
            if (nLen <= 0)
            {
                return AKEY_RV_TOKEN_INVALID_PARAM;
            }
            memcpy(m_snBuf.ReAllocBytesSetLength(nLen),m_readerCertBuf.GetBuffer()+u32Offset,nLen);
            memcpy(snBuf.ReAllocBytesSetLength(nLen),m_readerCertBuf.GetBuffer()+u32Offset,nLen);
            
            BYTE bSupportResume = 0;
            hr = SupportResume(&bSupportResume);
            if (hr == AKEY_RV_OK) {
                if (bSupportResume == 0x01) {
                    ParamManager::instance()->setResumeFlag(true);
                }else {
                    ParamManager::instance()->setResumeFlag(false);
                }
            }else {
                ParamManager::instance()->setResumeFlag(false);
            }
            return AKEY_RV_OK;
        }
        
        HRESULT Read_BITMAP(LPBYTE pbData,UINT32 u32DataLen,CBuffer& resultBuf)
        {
            BYTE abSendBuff[300] = {0xF0, 0xF8, 0x00, 0x00, (BYTE)u32DataLen};
            memcpy(abSendBuff+5, pbData, u32DataLen);
            HRESULT hr = Reader_Execute(abSendBuff, 5+abSendBuff[4], resultBuf);
            return hr;
        }
        
        HRESULT SupportResume(BYTE* bSupportResume)
        {
            HRESULT hr = AKEY_RV_OK;
            CBuffer resultBuf;
            hr = Read_BITMAP( (LPBYTE)"\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00", 10, resultBuf);
            if (hr == AKEY_RV_OK && resultBuf.GetLength() > 8) {
                if (bSupportResume) {
                    LPBYTE pbResult = resultBuf.GetBuffer();
                    UINT32 v1 = Helper::LittleEndian::UInt32FromBytes(pbResult);
                    if (v1 & (1<<23))
                    {
                        *bSupportResume = pbResult[8];
                        return AKEY_RV_OK;
                    }else {
                        return AKEY_RV_NOT_SUPPORT_RESUME;
                    }
                }
            }
            return hr;
        }

        
    private:
        CBuffer m_readerCertBuf;
        CBuffer m_sslCertBuf;
        CBuffer m_snBuf;
        CBuffer m_cosVerBuf;
        CBuffer m_appKey;
        CBuffer m_appKeyHash;
        IDCARD_DEVICE_TYPE m_deviceType;
    };
    
}


#endif /* IDCardInfo_h */
