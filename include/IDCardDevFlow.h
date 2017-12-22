//
//  IDCardDevCmd.h
//  TDRToken
//
//  Created by zhaowei on 16/5/5.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardDevCmd_h
#define IDCardDevCmd_h
#include "IDCardConfig.h"
#include "IDCardInfo.h"

#define  IDCARD_DEVCMD_VERSION  0x0302

typedef enum EXCHANGE_CMD_TYPE
{
    CMD_TYPE_SINGLE =0x01,//单指令
    CMD_TYPE_MUL    =0x02,//多指令
}EXCHANGE_CMD_TYPE;

namespace IDCARD {
#pragma mark - CIDDevFlow
    class CIDDevFlow
    {
    public:
        CIDDevFlow(){
            m_sessionValid = false;
            
        }
        ~CIDDevFlow(){}
        
        void LoadCache()
        {
            IDCardInfo::instance()->LoadSamCertAndVer(m_samCertVerBuf,m_samCertBuf);
        }
        
        void initParam()
        {
            m_error = AKEY_RV_OK;
            m_proRequst = PROGRESS_BUILD_SAFECHANNEL1;
            m_status[2] = 0x00;
            m_status[3] = 0x00;//默认输出照片
            
        }
        
        /**
         *  组设备指令流包
         *
         *
         *  @return 错误码
         */
        HRESULT MakeDevFlow(IDREADER_PROCESS_STATUS progress,CBuffer& outBuf) {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "MakeDevFlow");
            LPBYTE pbDevFlowBuf = m_abFlowBuf;
            UINT32 uOffset = 0;
            m_proRequst = progress;
            LGNTRACE_MSG("m_proRequst:%d",m_proRequst);
            switch (m_proRequst) {
                case PROGRESS_BUILD_SAFECHANNEL1:
                {
                    getDeviceStatus();
                    const CBuffer& appHashKey = IDCardInfo::instance()->getAppKeyHash();
                    LGNTRACE_HEX("appHashKey:", appHashKey.GetBuffer(), appHashKey.GetLength());
                    if (appHashKey.GetLength()) {
                        CBuffer EncResultBuf;
                        LPBYTE pbEncIv = (LPBYTE)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
                        CDecryptIDCard::EncPackage(appHashKey.GetBuffer(), 16, pbEncIv, 16, pbEncIv, 16, EncResultBuf,false);
                        uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_CHANNEL_VALID_VERIFY_CODE,EncResultBuf.GetBuffer(),EncResultBuf.GetLength(), pbDevFlowBuf+uOffset);
                    }
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_STATUS,m_status, 4, pbDevFlowBuf+uOffset);
                    BYTE btVer[2] = {0};
                    if (m_samCertVerBuf.GetLength()) {
                        memcpy(btVer, m_samCertVerBuf.GetBuffer(), 2);
                    }else {
                        Helper::BigEndian::UInt16ToBytes(0x0000, btVer);
                    }
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_SAM_CERT_VERSION,btVer,2, pbDevFlowBuf+uOffset);
                    const CBuffer& SNBuf = IDCardInfo::instance()->getSN();
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_SERIAL_NUMBER,SNBuf.GetBuffer(),SNBuf.GetLength(), pbDevFlowBuf+uOffset);
                }
                    break;
                case PROGRESS_BUILD_SAFECHANNEL2:
                {
                    getDeviceStatus();
					cleanCacheBuf();//新建立通道清除缓存
                    const CBuffer& certBuff = IDCardInfo::instance()->getReaderCert();
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_READER_CERT,certBuff.GetBuffer(),certBuff.GetLength(), pbDevFlowBuf+uOffset);
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_STATUS,m_status, 4, pbDevFlowBuf+uOffset);
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_ENC_SIGN_DATA,m_devEncSignData.GetBuffer(),m_devEncSignData.GetLength(), pbDevFlowBuf+uOffset);
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_BEGIN:
                {
                    getDeviceStatus();
                    ParamManager::instance()->setFinishFlag(false);
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_STATUS,m_status, 4, pbDevFlowBuf+uOffset);
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_SINGLE_INS_RESPONSE,m_signalResponse.GetBuffer(),m_signalResponse.GetLength(), pbDevFlowBuf+uOffset);
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_SINGLE:
                {
                    getDeviceStatus();
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_STATUS,m_status, 4, pbDevFlowBuf+uOffset);
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_SINGLE_INS_RESPONSE,m_signalResponse.GetBuffer(),m_signalResponse.GetLength(), pbDevFlowBuf+uOffset);
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_MUL:
                {
                    getDeviceStatus();
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_STATUS,m_status, 4, pbDevFlowBuf+uOffset);
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_MUL_INS_RESPONSE,m_mulResponse.GetBuffer(),m_mulResponse.GetLength(), pbDevFlowBuf+uOffset);
                }
                    break;
                case PROGRESS_CMD_ERROR:
                {
                    getDeviceStatus();
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_STATUS,m_status, 4, pbDevFlowBuf+uOffset);
                    BYTE errorCode[4];
                    Helper::BigEndian::UInt32ToBytes((UINT32)m_error, errorCode);
                    uOffset+=CPackageMaker::MakeTLVPacket(TAG_DEV_ERROR_CODE,errorCode, 4, pbDevFlowBuf+uOffset);
                }
                    break;
                    
                default:
                    break;
            }
            memcpy(outBuf.ReAllocBytesSetLength(uOffset), pbDevFlowBuf, uOffset);
            return AKEY_RV_OK;
        }
        /**
         *  解析设备指令流
         *
         *  @param pbRecv  输出数据到缓冲区
         *  @param punRecv 输出数据长度
         *  @param clone   是否输出到缓冲区(YES:输出 NO:不输出)
         *
         *  @return 错误码
         */
        HRESULT ParseDevFlow(CBuffer& inBuf,CBuffer& outBuf){
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ParseDevFlow");
            LPBYTE pbInBuf = inBuf.GetBuffer();
            UINT32 u32InLen = inBuf.GetLength();
            UINT32 u32Offset = 0;
            HRESULT hr = AKEY_RV_OK;
            int u16TagLen ;
            u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_STATUS, pbInBuf, u32InLen, &u32Offset);
            if (u16TagLen == -1) {
                return AKEY_RV_INS_EXCHANGE_NO_DEVICE_STATUS;
            }
            m_proResponse = (IDREADER_PROCESS_STATUS)pbInBuf[u32Offset];
            m_status[2] = pbInBuf[u32Offset+2];
            switch (m_proResponse) {
                case PROGRESS_BUILD_SAFECHANNEL1:
                {
                    u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_SAM_RANDOM, pbInBuf, u32InLen, &u32Offset);
                    if (u16TagLen == -1) {
                        return AKEY_RV_INS_EXCHANGE_NO_SAM_RANDOM;
                    }
                    CBuffer sendBuf,resultBuf;
                    
                    LPBYTE pbSendBuf = sendBuf.ReAllocBytesSetLength(5+u16TagLen);
                    memcpy(pbSendBuf, "\x00\x9E\x20\x11\x08", 5);
                    memcpy(pbSendBuf+5, pbInBuf+u32Offset, u16TagLen);
                    pbSendBuf[4] = (BYTE)u16TagLen;
                    hr = IDCardInfo::instance()->Reader_Execute(pbSendBuf, 5+u16TagLen, resultBuf);
                    if (AKEY_RV_OK != hr) {
                        LGNTRACE_ERRORNO(hr);
                        return hr;
                    }
                    u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_SAM_CERT, pbInBuf, u32InLen, &u32Offset);
                    if (u16TagLen > 0) {
                        memcpy(m_samCertBuf.ReAllocBytesSetLength(u16TagLen), pbInBuf+u32Offset, u16TagLen);
                        int u32CertVerLen = 0;
                        LPBYTE pbCertVer = CPackageMaker::BirthCertFindTLV(pbInBuf+u32Offset, u16TagLen, 0x0002, &u32CertVerLen);
                        memcpy(m_samCertVerBuf.ReAllocBytesSetLength(u32CertVerLen), pbCertVer, u32CertVerLen);
                        IDCardInfo::instance()->SaveCertVer(m_samCertVerBuf,m_samCertBuf);
                        //return AKEY_RV_INS_EXCHANGE_NO_SAM_CERT;
                    }
                    
                    pbSendBuf = sendBuf.ReAllocBytesSetLength(7+m_samCertBuf.GetLength());
                    memcpy(pbSendBuf, "\x00\x9E\x00\x11\x00", 5);
                    Helper::BigEndian::UInt16ToBytes(m_samCertBuf.GetLength(), pbSendBuf+5);
                    memcpy(pbSendBuf+7, m_samCertBuf.GetBuffer(), m_samCertBuf.GetLength());
                    hr = IDCardInfo::instance()->Reader_Execute(pbSendBuf,7+m_samCertBuf.GetLength(), resultBuf);
                    if (AKEY_RV_OK != hr) {
                        LGNTRACE_ERRORNO(hr);
                        return hr;
                    }
                    if (resultBuf.GetLength() < 17) {
						LGNTRACE_HEX("resultBuf error:",resultBuf.GetBuffer(),resultBuf.GetLength());
                        return AKEY_RV_INS_EXCHANGE_APPKEY_LENGTH_ERROR;
                    }
                    IDCardInfo::instance()->setAppKey(resultBuf.GetBuffer()+1, 16);
                    BYTE btHash[0x20];
                    OpenAlg::CDigest::Digest(Helper::HashType2Name(AKEY_HASH_SHA1),resultBuf.GetBuffer()+1, 16, btHash);
                    IDCardInfo::instance()->setAppKeyHash(btHash, 16);
                    
                    m_proRequst = PROGRESS_BUILD_SAFECHANNEL2;
                    memcpy(m_devEncSignData.ReAllocBytesSetLength(resultBuf.GetLength()-17), resultBuf.GetBuffer()+17, resultBuf.GetLength()-17);
                    return AKEY_RV_OK;
                }
                    break;
                case PROGRESS_BUILD_SAFECHANNEL2:
                {
                    m_proRequst = PROGRESS_CMD_EXCHANGE_BEGIN;
                    return AKEY_RV_OK;
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_BEGIN:
                {
                    m_proRequst = PROGRESS_CMD_EXCHANGE_SINGLE;
                    u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_SINGLE_INS, pbInBuf, u32InLen, &u32Offset);
                    if (u16TagLen > 0) {
						memcpy(outBuf.ReAllocBytesSetLength(u16TagLen), pbInBuf+u32Offset, u16TagLen);
						memcpy(m_cmd04Buf.ReAllocBytesSetLength(u16TagLen), pbInBuf+u32Offset, u16TagLen);
                        m_cmdType = CMD_TYPE_SINGLE;
                    }
                    return AKEY_RV_OK;
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_SINGLE:
                {
                    m_proRequst = PROGRESS_CMD_EXCHANGE_SINGLE;
                    u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_SINGLE_INS, pbInBuf, u32InLen, &u32Offset);
                    if (u16TagLen == -1) {
                        LGNTRACE_ERRORNO(AKEY_RV_INS_EXCHANGE_NO_SINGLE_CMD);
                        return AKEY_RV_INS_EXCHANGE_NO_SINGLE_CMD;
                    }
                    Execute_single(pbInBuf+u32Offset,u16TagLen);
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_MUL:
                {
                    m_proRequst = PROGRESS_CMD_EXCHANGE_MUL;
                    u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_MUL_INS, pbInBuf, u32InLen, &u32Offset);
                    if (u16TagLen == -1) {
                        LGNTRACE_ERRORNO(AKEY_RV_INS_EXCHANGE_NO_MUL_CMD);
                        return AKEY_RV_INS_EXCHANGE_NO_MUL_CMD;
                    }
                    memcpy(outBuf.ReAllocBytesSetLength(u16TagLen), pbInBuf+u32Offset, u16TagLen);
                    return AKEY_RV_OK;
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_END:
                {
                    LGNTRACE_MSG("ParseDevFlow EXCHANGE_END");
					UINT32 u32CmdOffset;
					u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_SINGLE_INS, pbInBuf, u32InLen, &u32CmdOffset);
					if (u16TagLen >0 ) {
						CBuffer resultBuf;
						hr = IDCardInfo::instance()->Transmit(pbInBuf+u32CmdOffset,u16TagLen, resultBuf);
						if (hr == AKEY_RV_OK && ParamManager::instance()->getResumeFlag()) {
							cleanCacheBuf();
						}
						memcpy(outBuf.ReAllocBytesSetLength(resultBuf.GetLength()),resultBuf.GetBuffer(),resultBuf.GetLength());
						return hr;
					}else {
						UINT32 u32EncKeyOffset;
						u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_PROCESS_ENC_KEY, pbInBuf, u32InLen, &u32EncKeyOffset);
						if (u16TagLen == -1) {
							return AKEY_RV_INS_EXCHANGE_NO_ENC_ID_INFO;
						}
						u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_ID_CIPHER_TEXT, pbInBuf, u32InLen, &u32Offset);
						if (u16TagLen == -1) {
							return AKEY_RV_INS_EXCHANGE_NO_ENC_ID_INFO;
						}
						const CBuffer& AppKey = IDCardInfo::instance()->getAppKey();
						hr = CDecryptIDCard::DecIDCardPackage(AppKey.GetBuffer(),pbInBuf+u32EncKeyOffset,pbInBuf+u32Offset,u16TagLen,outBuf);
						if (hr == AKEY_RV_OK && ParamManager::instance()->getResumeFlag()) {
							cleanCacheBuf();
						}
					}
                    return hr;
                }
                    break;
                case PROGRESS_CMD_ERROR:
                {
                    LGNTRACE_MSG("ParseDevFlow CMD_ERROR");
                    u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEV_ERROR_CODE, pbInBuf, u32InLen, &u32Offset);
                    if (u16TagLen > 0) {
                        if((m_error=Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset)) != AKEY_RV_OK)
                        {
                            LGNTRACE_ERRORNO(m_error);
                            return m_error;
                        }
                    }else {
                        return AKEY_RV_INS_EXCHANGE_DEV_FLOW_NO_ERR;
                    }
                }
                    break;
                    
                default:
                    return AKEY_RV_INS_EXCHANGE_PROGRESS_STATUS_ERROR;
                    break;
            }
            
            return AKEY_RV_OK;
        }
        
        /**
         *  获取错误码
         *
         *  @return 错误码
         */
        HRESULT getError() {
            return m_error; //错误码
        }
        
        LPBYTE getDevFlowPtr()
        {
            return m_abFlowBuf;
        }
        
        UINT32 getDevFlowLen()
        {
            return m_devFlowLen;
        }
        
        bool getCache04CMD(CBuffer& cmdBuf)
        {
            if (m_cmd04Buf.GetLength()) {
                memcpy(cmdBuf.ReAllocBytesSetLength(m_cmd04Buf.GetLength()), m_cmd04Buf.GetBuffer(), m_cmd04Buf.GetLength());
                return true;
            }
            return false;
        }

		void cleanCache04CMD()
		{
			m_cmd04Buf.SetLength(0);
		}
        
		//是否认证结束
        bool isAuthenDone()
        {
            return (m_proResponse == PROGRESS_CMD_EXCHANGE_MUL);
        }
        
		//是否读取身份证文本信息结束
        bool isReadIDDone()
        {
            return (m_proResponse == PROGRESS_CMD_EXCHANGE_END);
        }
        
        IDREADER_PROCESS_STATUS getNextProgressStatus()
        {
            return m_proRequst;
        }
        
        bool IsSWError(HRESULT hr)
        {
            return ((hr & 0xFFFF0000) == AKEY_RV_SW_BASE);
        }
        
        bool IsClientError(HRESULT hr)
        {
            return (((hr & 0xFFFF0000) == 0xE0100000) ||
                    ((hr & 0xFFFF0000) == 0xE0200000) ||
                    ((hr & 0xFFFF0000) == 0xE0E00000));
        }
        
        HRESULT Execute_Mul(LPBYTE pbSend,UINT32 u32SLen)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Execute_Mul");
            LGNTRACE_MSG("resume flag:%d",ParamManager::instance()->getResumeFlag());
            HRESULT hr = AKEY_RV_OK;
            if (ParamManager::instance()->getResumeFlag()) {
                hr = IDReader_Execute_Mul(pbSend, u32SLen, m_mulResponse);
                if (hr != AKEY_RV_OK) {
                    LGNTRACE_ERRORNO(hr);
					m_error = hr;
					m_proRequst = PROGRESS_CMD_ERROR;
					//该错误无法恢复,直接返回错误，在业务层清除AppKeyHash，然后重建通道
                    return hr;
                }
            }else {
                hr = IDCardInfo::instance()->Reader_Execute_Mul(pbSend, u32SLen, m_mulResponse);
                if (hr != AKEY_RV_OK) {
                    LGNTRACE_ERRORNO(hr);
                    m_error = hr;
                    m_proRequst = PROGRESS_CMD_ERROR;
                    return hr;
                }
            }
            return hr;
        }
        
        HRESULT Execute_single(LPBYTE pbSend,UINT32 u32SLen)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "IDCardDevFlow Execute_single");
            HRESULT hr = AKEY_RV_OK;
            hr = IDCardInfo::instance()->Reader_Execute(pbSend, u32SLen, m_signalResponse);
            if (hr != AKEY_RV_OK) {
                LGNTRACE_ERRORNO(hr);
				m_error = hr;
				m_proRequst = PROGRESS_CMD_ERROR;
                return hr;
            }
            if (hr == AKEY_RV_OK && ParamManager::instance()->getResumeFlag()) {
                LPBYTE pbCmd = pbSend;
                if (pbCmd[0] == 0xF0 && pbCmd[1] == 0xF4 && pbCmd[3] == 0x04) {
                    if (m_signalResponse.GetLength() > 32) {
                        CBuffer recvDN;
                        LPBYTE pbRecvDNLen = m_signalResponse.GetBuffer() + (m_signalResponse.GetLength() - 32);
                        memcpy(recvDN.ReAllocBytesSetLength(32), pbRecvDNLen, 32);
                        if (m_dnBuf.GetLength()) {
                            LGNTRACE_HEX("dn local:", m_dnBuf.GetBuffer(), m_dnBuf.GetLength());
                            if (!IDCardUtil::compareBuf(recvDN, m_dnBuf)) {
                                LGNTRACE_HEX("dn server:", recvDN.GetBuffer(), recvDN.GetLength());
                                cleanCacheBuf();
                                memcpy(m_dnBuf.ReAllocBytesSetLength(32), recvDN.GetBuffer(), recvDN.GetLength());
                            }
                        }else {
                            LGNTRACE_MSG("No dn");
                            memcpy(m_dnBuf.ReAllocBytesSetLength(32), recvDN.GetBuffer(), recvDN.GetLength());
                            LGNTRACE_HEX("dn:", m_dnBuf.GetBuffer(), m_dnBuf.GetLength());
                        }
                        m_signalResponse.SetLength(m_signalResponse.GetLength()-32);
                    }
                }
            }
			BYTE bBindMode = ParamManager::instance()->getBindMode();
			if (bBindMode == 0x01 && pbSend[0] == 0xF0 && pbSend[1] == 0xF4 && pbSend[3] == 0x07)
			{
				CBuffer tmpRecvBuf;
				IDCardInfo::instance()->Reader_Execute((LPBYTE)"\xF0\xF4\x81\x22\x00", 5, tmpRecvBuf);
			}
            m_cmdType = CMD_TYPE_SINGLE;
            return AKEY_RV_OK;
        }
        
        
        HRESULT checkCard()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "IDCardDevFlow checkCard");
            HRESULT hr = AKEY_RV_OK;
            int count = 3;
            while (count--) {
                hr = IDCardInfo::instance()->FindCard(NULL);
                if (hr == AKEY_RV_OK) {
                    break;
                }
            }
            return hr;
        }
        
    protected:
        void getDeviceStatus()
        {
            m_status[0] = m_proRequst;
            m_status[1] = 0x00;
            m_status[2] = 0x00;
            m_status[3] = 0x00;
        }
        
        //len[2] + CMD[len] (LV结构)
        HRESULT IDReader_Execute_Mul(LPBYTE pbSend, UINT32 unSLen,CBuffer& outBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "IDCardDev Flow Reader_Execute_Mul");
            HRESULT hr = AKEY_RV_OK;
            UINT32 u32LC = 0;
            LPBYTE pbBuf = outBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
            UINT32 u32Offset = 0;
            CBuffer cmdResultBuf;
            for (int i = 0 ; i < unSLen; i+=2+u32LC) {
                u32LC = Helper::BigEndian::UInt16FromBytes(pbSend+i);
                LGNTRACE_MSG("pbSend[3]:%d",pbSend[3]);
                LPBYTE pbTmp = pbSend+i+2;
                if (pbTmp[3] == 0x09 && m_buf09.GetLength()) {
                    LGNTRACE_MSG("get 09 cache");
                    Helper::BigEndian::UInt16ToBytes(m_buf09.GetLength(), pbBuf+u32Offset);
                    u32Offset+=2;
                    memcpy(pbBuf+u32Offset, m_buf09.GetBuffer(), m_buf09.GetLength());
                    u32Offset+=m_buf09.GetLength();
                }else if(pbTmp[3] == 0x0A && m_buf0A.GetLength()){
                    LGNTRACE_MSG("get 0A cache");
                    Helper::BigEndian::UInt16ToBytes(m_buf0A.GetLength(), pbBuf+u32Offset);
                    u32Offset+=2;
                    memcpy(pbBuf+u32Offset, m_buf0A.GetBuffer(), m_buf0A.GetLength());
                    u32Offset+=m_buf0A.GetLength();
                }else if(pbTmp[3] == 0x0B && m_buf0B.GetLength()){
                    LGNTRACE_MSG("get 0B cache");
                    Helper::BigEndian::UInt16ToBytes(m_buf0B.GetLength(), pbBuf+u32Offset);
                    u32Offset+=2;
                    memcpy(pbBuf+u32Offset, m_buf0B.GetBuffer(), m_buf0B.GetLength());
                    u32Offset+=m_buf0B.GetLength();
                }else {
                    
                    hr = IDCardInfo::instance()->Reader_Execute(pbSend+i+2, u32LC,cmdResultBuf);
                    if (hr != AKEY_RV_OK) {
						m_error = hr;
						m_proRequst = PROGRESS_CMD_ERROR;
                        return hr;
                    }
                    fnreadCmdInfo cmdCb = ParamManager::instance()->getCmdCallBack();
                    if (cmdCb) {
                        cmdCb(pbTmp[3]);
                    }
                    if(pbTmp[3] == 0x09) {
                        LGNTRACE_MSG("save 09 cache");
                        memcpy(m_buf09.ReAllocBytesSetLength(cmdResultBuf.GetLength()), cmdResultBuf.GetBuffer(), cmdResultBuf.GetLength());
                    }else if(pbTmp[3] == 0x0A) {
                        LGNTRACE_MSG("save 0A cache");
                        memcpy(m_buf0A.ReAllocBytesSetLength(cmdResultBuf.GetLength()), cmdResultBuf.GetBuffer(), cmdResultBuf.GetLength());
                    }else if(pbTmp[3] == 0x0B) {
                        LGNTRACE_MSG("save 0B cache");
                        memcpy(m_buf0B.ReAllocBytesSetLength(cmdResultBuf.GetLength()), cmdResultBuf.GetBuffer(), cmdResultBuf.GetLength());
                    }
                    Helper::BigEndian::UInt16ToBytes(cmdResultBuf.GetLength(), pbBuf+u32Offset);
                    u32Offset+=2;
                    memcpy(pbBuf+u32Offset, cmdResultBuf.GetBuffer(), cmdResultBuf.GetLength());
                    u32Offset+=cmdResultBuf.GetLength();
                }
            }
            outBuf.SetLength(u32Offset);
            return hr;
        }
        
        bool checkCacheBufReady()
        {
            return ((m_buf09.GetLength() > 0) ||
                    (m_buf0A.GetLength() > 0) ||
                    (m_buf0B.GetLength() > 0));
        }
        
        void resumeReadID()
        {
            m_proRequst = PROGRESS_CMD_EXCHANGE_BEGIN;
        }
       
        void cleanCacheBuf()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "cleanCacheBuf");
            m_buf09.SetLength(0);
            m_buf0A.SetLength(0);
            m_buf0B.SetLength(0);
        }
        
    private:
        BYTE m_abFlowBuf[DEFAULT_BUFFERLEN];            //设备指令流
        UINT32 m_devFlowLen;                            //设备指令流长度
        HRESULT m_error;                                //错误码
        IDREADER_PROCESS_STATUS m_proRequst;            //请求
        IDREADER_PROCESS_STATUS m_proResponse;          //应答
        CBuffer                 m_samCertBuf;           //控制器证书
        CBuffer                 m_samCertVerBuf;        //控制器证书版本号
        bool   m_sessionValid;                          //会话是否有效
        BYTE   m_status[4];
        bool   m_bStop;
        CBuffer m_devEncSignData;                       //设备加密签名数据
        CBuffer m_signalResponse;                       //单指令响应结果
        CBuffer m_mulResponse;                          //多指令响应结果
        
        CBuffer m_dnBuf;                                //缓存DN
        CBuffer m_buf09;                                //09缓存
        CBuffer m_buf0A;                                //0A缓存
        CBuffer m_buf0B;                                //0B缓存
        CBuffer m_cmd04Buf;                             //指令04的缓存
        EXCHANGE_CMD_TYPE m_cmdType;
        
        
        
    };
#pragma mark - CIDInfoPackage end
}


#endif /* IDCardDevCmd_h */
