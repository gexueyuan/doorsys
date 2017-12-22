//
//  IDCardPackage.h
//  TDRToken
//
//  Created by zhaowei on 16/5/9.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardPackage_h
#define IDCardPackage_h

#include "AKeyDef.h"
#include "alg/OpenAlg.h"
#include "AKeyHelper.h"
#include "IDCardUtil.h"
#include "IDCardInfo.h"
#include "IDCardTags.h"

using namespace AKey;

namespace IDCARD {
    
    class ShellPackage : public IPackage
    {
    public:
        ShellPackage() {}
        ~ShellPackage() {}
        
        virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ShellPackage MakePackage");
            int inLen = inBuf.GetLength();
            LPBYTE pbOutBuf = NULL;
            UINT32 u32Offset = 0;
            pbOutBuf = outBuf.ReAllocBytesSetLength(4 + inLen + 2);
            Helper::BigEndian::UInt32ToBytes(inBuf.GetLength() + 2, pbOutBuf);
            u32Offset += 4;
            memcpy(pbOutBuf+u32Offset, inBuf.GetBuffer(), inBuf.GetLength());
            u32Offset += inBuf.GetLength();
            UINT16 mac = OpenAlg::CRC::DoCRC16(inBuf.GetBuffer(), inBuf.GetLength(), 0x3E8C);
            Helper::BigEndian::UInt16ToBytes(mac, pbOutBuf + u32Offset);
            return AKEY_RV_OK;
        }
        
        virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
        {
            LPBYTE pbInBuf = inBuf.GetBuffer();
            int inLen = inBuf.GetLength();
            UINT16 mac = OpenAlg::CRC::DoCRC16(pbInBuf,inLen-2, 0x3E8C);//两字节mac
            if (mac != Helper::BigEndian::UInt16FromBytes(pbInBuf+inLen-2)) {
                return AKEY_RV_MAC_VERIFY_ERROR;
            }
            memcpy(outBuf.ReAllocBytesSetLength(inLen-2), pbInBuf, inLen-2);
            return AKEY_RV_OK;
        }
    };
    
    class EncShellPackage : public IPackage
    {
    public:
        EncShellPackage() {
            memcpy(m_key,IDCARD_DEFAULT_ENC_KEY, 16);
			m_bstep = 0;
			m_samIndex = 0;
			m_addr1 = 0;
			m_addr2 = 0;
        }
        ~EncShellPackage() {}
        
        void setKey(const CBuffer& keyBuf)
        {
            memcpy(m_key, keyBuf.GetBuffer(), 16);
        }
        
        void setDefaultKey()
        {
            memcpy(m_key,IDCARD_DEFAULT_ENC_KEY, 16);
        }

		void setStepNum(BYTE step)
		{
			m_bstep = step;
		}

		void setSamIndex(BYTE samIndex)
		{
			m_samIndex = samIndex;
		}

		void cleanSamIndexAndAddr()
		{
			m_samIndex = 0;
			m_addr1 = 0;
			m_addr2 = 0;
		}

        void setTradeCode(IDREADER_PROCESS_STATUS status)
        {
            if (status == PROGRESS_BUILD_SAFECHANNEL1) {
                memcpy(m_tradeCode, "\x01\x01", 2);
            }else if(status == PROGRESS_BUILD_SAFECHANNEL2) {
                memcpy(m_tradeCode, "\x01\x02", 2);
            }else if(status == PROGRESS_REQUEST_IMAGE){
                memcpy(m_tradeCode, "\x01\x04", 2);
            }else {
                memcpy(m_tradeCode, "\x01\x03", 2);
            }
        }
        
        virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "EncShellPackage MakePackage");
			LGNTRACE_HEX("EncShellPackage MakePackage Before Enc:",inBuf.GetBuffer(),inBuf.GetLength());
            CBuffer encBuf;
            LPBYTE pbPackageBuf = encBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
            UINT32 u32PkgLen = 0;
            u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_TRADE_CODE, m_tradeCode,2, pbPackageBuf + u32PkgLen);
            //const CBuffer& SNBuf = IDCardInfo::instance()->getSN();
			//序列号
            //u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_READER_SERIAL_NUMBER, SNBuf.GetBuffer(),SNBuf.GetLength(), pbPackageBuf + u32PkgLen);
			//SAM索引
            //u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_SAM_INDEX, &m_samIndex,1, pbPackageBuf + u32PkgLen);

			//顺序号
			//BYTE btBuf[2];
			//Helper::BigEndian::UInt16ToBytes(m_u16RequestNum,btBuf);
			//u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_SEQ_NUMBER,btBuf,2, pbPackageBuf + u32PkgLen);
			//
			//
			////批次号
			//Helper::BigEndian::UInt16ToBytes(m_u16BatchNum,btBuf);
			//u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_BATCH_NUMBER,btBuf,2, pbPackageBuf + u32PkgLen);
			//加密字段1
            CBuffer resultBuf;
            HRESULT hr = CDecryptIDCard::EncPackage(m_key, 16, (LPBYTE)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16, inBuf.GetBuffer(), inBuf.GetLength(), resultBuf,true);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_ENCRYPT_TEXT, resultBuf.GetBuffer(),resultBuf.GetLength(), pbPackageBuf + u32PkgLen);

			const CBuffer& SNBuf = IDCardInfo::instance()->getSN();

			//唯一链号
			//CBuffer uuidBuf;
			//IDCardUtil::CreateUniqueCode(SNBuf,uuidBuf);
			//u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_UNIQUE_CODE, uuidBuf.GetBuffer(),uuidBuf.GetLength(), pbPackageBuf + u32PkgLen);

			//网络重发次数
			//BYTE btRetryCount[2] = {0};
			//u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_NETCOMM_RETRY_TIME, btRetryCount,2, pbPackageBuf + u32PkgLen);

			//加密字段2
			LGNTRACE_MSG("EncShellPackage MakePackage m_bstep:%d",m_bstep);
			if (m_bstep == PROGRESS_CMD_EXCHANGE_BEGIN)
			{
				LPBYTE pbDefKey = (LPBYTE)IDCARD_DEFAULT_ENC_KEY;
				CBuffer enc2Buf;
				UINT32 u32Enc2Len = 0;
				LPBYTE pbEnc2Buf = enc2Buf.ReAllocBytes(DEFAULT_BUFFERLEN);
				//加密字段2中步骤号（设备状态）
				u32Enc2Len += CPackageMaker::MakeTLVPacket(TAG_DEV_STATUS,&m_bstep,1, pbEnc2Buf + u32Enc2Len);
				//加密字段2中读卡器序列号
				u32Enc2Len += CPackageMaker::MakeTLVPacket(TAG_READER_SERIAL_NUMBER,SNBuf.GetBuffer(),SNBuf.GetLength(), pbEnc2Buf + u32Enc2Len);
				//加密字段2中 通道有效认证码
				const CBuffer& appHashKey = IDCardInfo::instance()->getAppKeyHash();
				LGNTRACE_HEX("appHashKey:", appHashKey.GetBuffer(), appHashKey.GetLength());
				if (appHashKey.GetLength()) {
					CBuffer EncResultBuf;
					LPBYTE pbEncIv = (LPBYTE)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";
					CDecryptIDCard::EncPackage(appHashKey.GetBuffer(), 16, pbEncIv, 16, pbEncIv, 16, EncResultBuf,false);
					u32Enc2Len+=CPackageMaker::MakeTLVPacket(TAG_DEV_CHANNEL_VALID_VERIFY_CODE,EncResultBuf.GetBuffer(),EncResultBuf.GetLength(), pbEnc2Buf+u32Enc2Len);
				}else {
					return AKEY_RV_APP_HASH_KEY_UNAVAIABLE;
				}
				enc2Buf.SetLength(u32Enc2Len);
				hr = CDecryptIDCard::EncPackage(pbDefKey, 16, (LPBYTE)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16, enc2Buf.GetBuffer(),enc2Buf.GetLength(), resultBuf,true);
				if (hr != AKEY_RV_OK) {
					return hr;
				}
				u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_ENCRYPT_TEXT2, resultBuf.GetBuffer(),resultBuf.GetLength(), pbPackageBuf + u32PkgLen);
			}
			

			//结束组包
            encBuf.SetLength(u32PkgLen);
            
            int inLen = encBuf.GetLength();
            LPBYTE pbOutBuf = NULL;
            UINT32 u32Offset = 0;
			UINT32 u32HeaderLen = sizeof(IDCARD_PACKAGE_HEADER);
            pbOutBuf = outBuf.ReAllocBytesSetLength(4 + u32HeaderLen + inLen + 2);
            Helper::BigEndian::UInt32ToBytes(inLen + u32HeaderLen + 2, pbOutBuf);
            u32Offset += 4;

			PTR_IDCARD_PACKAGE_HEADER pbHeaderPkg = (PTR_IDCARD_PACKAGE_HEADER)(pbOutBuf+u32Offset);
			//报文版本号[2]
			memcpy(pbHeaderPkg->version,IDCARD_PACKAGE_HEADER_VERSION,2);
			//唯一链号[16]
			CBuffer uuidBuf;
			IDCardUtil::CreateUniqueCode(SNBuf,uuidBuf);
			memcpy(pbHeaderPkg->ChainNum,uuidBuf.GetBuffer(),uuidBuf.GetLength());
			//sam 索引[1]
			pbHeaderPkg->samIndex = m_samIndex;
			//重试次数[1]
			pbHeaderPkg->retryTime = 0x00;
			//B20-B21 保留[2]
			pbHeaderPkg->errCode = 0x00;
			pbHeaderPkg->bRFU = 0x00;
			//PCB B22[1]
			pbHeaderPkg->addr1 = m_addr1;
			//B23 包号[1]
			pbHeaderPkg->addr2 = m_addr2;
			u32Offset += u32HeaderLen;

            memcpy(pbOutBuf+u32Offset, encBuf.GetBuffer(), encBuf.GetLength());
            u32Offset += encBuf.GetLength();
            UINT16 mac = OpenAlg::CRC::DoCRC16(pbOutBuf + 4,u32HeaderLen + encBuf.GetLength(), 0x3E8C);
            Helper::BigEndian::UInt16ToBytes(mac, pbOutBuf + u32Offset);
            return AKEY_RV_OK;
        }
        
        virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
        {
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "EncShellPackage ParsePackage");
            LPBYTE pbInBuf = inBuf.GetBuffer();
            int inLen = inBuf.GetLength();
            UINT16 mac = OpenAlg::CRC::DoCRC16(pbInBuf,inLen-2, 0x3E8C);//两字节mac
            if (mac != Helper::BigEndian::UInt16FromBytes(pbInBuf+inLen-2)) {
                return AKEY_RV_MAC_VERIFY_ERROR;
            }
            
            UINT32 u32Offset = 0;
			int u16TagLen = 0;
			HRESULT hr = AKEY_RV_OK;
			PTR_IDCARD_PACKAGE_HEADER pbHeaderPkg = (PTR_IDCARD_PACKAGE_HEADER)pbInBuf;
			m_samIndex = pbHeaderPkg->samIndex;
			m_addr1 = pbHeaderPkg->addr1;
			m_addr2 = pbHeaderPkg->addr2;
			//
			pbInBuf = pbInBuf + sizeof(IDCARD_PACKAGE_HEADER);
			inLen = inLen -sizeof(IDCARD_PACKAGE_HEADER);
			u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ERROR_CODE, pbInBuf, inLen-2, &u32Offset);
			if (u16TagLen > 0) {
				if ((hr = Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset)) != AKEY_RV_OK) {
					LGNTRACE_ERRORNO(hr);
					return hr;
				}
			}

            u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ENCRYPT_TEXT, pbInBuf, inLen-2, &u32Offset);
            hr = CDecryptIDCard::DecPackage(m_key, 16, (LPBYTE)"\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 16, pbInBuf+u32Offset, u16TagLen, outBuf);
            if (hr != AKEY_RV_OK) {
				LGNTRACE_HEX("Enc Data :",pbInBuf+u32Offset,u16TagLen);
				LGNTRACE_MSG("m_key Dec error:%x",hr);
				//解密失败，需要清除AppKeyHash
                return hr;
            }
            return AKEY_RV_OK;
        }
    private:
        BYTE m_tradeCode[2];
		UINT16 m_u16BatchNum;
        BYTE m_key[16];
        BYTE m_samIndex;
		BYTE m_bstep;
		UINT16 m_u16RequestNum;
		BYTE m_addr1;
		BYTE m_addr2;
    };
    
    class AppPackage
    {
    public:
        AppPackage()
        {
			m_requestNum = 0;
			m_batchNum = 0;
			m_samIndex = 0;
        }
        
        ~AppPackage(){}
        
        class CRequestPort : public IPackage
        {
            
        public:
            CRequestPort(AppPackage* pAppPkg)
            {
                m_pAppPkg = pAppPkg;
            }
            
            virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CRequestPort MakePackage");
                UINT32 u32PkgLen = 0;
                BYTE btBuffer[DEFAULT_BUFFERLEN] = {0};
                LPBYTE pbPackageBuf = btBuffer;
                const CBuffer& SNBuf = IDCardInfo::instance()->getSN();
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_TRADE_CODE, (LPBYTE)"\x08\x01",2, pbPackageBuf + u32PkgLen);
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_GETPORT_READER_SN, SNBuf.GetBuffer(),SNBuf.GetLength(), pbPackageBuf + u32PkgLen);
                memcpy(outBuf.ReAllocBytesSetLength(u32PkgLen), btBuffer, u32PkgLen);
                outBuf.SetLength(u32PkgLen);
                return AKEY_RV_OK;
            }
            
            virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CRequestPort ParsePackage");
                HRESULT hr = AKEY_RV_OK;
                LPBYTE pbInBuf = inBuf.GetBuffer();
                UINT32 u32InLen = inBuf.GetLength();
                UINT32 u32Offset = 0;
                int u16TagLen = 0;
                
                //
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRANSACTION_NUMBER, pbInBuf, u32InLen, &u32Offset);
                LGNTRACE_HEX("transaction num:", pbInBuf+u32Offset, u16TagLen);
                if (u16TagLen > 0) {
                    m_pAppPkg->setTranstion(pbInBuf+u32Offset, u16TagLen);
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRADE_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0 ) {
                    if (Helper::BigEndian::UInt16FromBytes(pbInBuf + u32Offset) != 0x0801 || u16TagLen != 2) {
                        LGNTRACE_ERRORNO(AKEY_RV_REQUEST_PORT_TRADECODE_ERROR);
                        return AKEY_RV_REQUEST_PORT_TRADECODE_ERROR;
                    }
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ERROR_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if ((hr = Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset)) != AKEY_RV_OK) {
                        LGNTRACE_ERRORNO(hr);
                        return hr;
                    }
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SAM_INDEX, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    m_pAppPkg->setSamIndex(pbInBuf[u32Offset]);
                }else {
                    return AKEY_RV_REQUEST_PORT_NO_SAM_INDEX;
                }

				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SAM_CONTROLLER_VERSION, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					CLog::instance()->setSamVer(pbInBuf+u32Offset,u16TagLen);
					//return AKEY_RV_BUILD_SAFECHANNEL_NO_TRANSTION_NUM;
				}
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_GETPORT_NEW_IP, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    ParamManager::instance()->setRequestIp(pbInBuf+u32Offset, u16TagLen);
                }else {
                    return AKEY_RV_REQUEST_PORT_NO_SAM_IP;
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_GETPORT_NEW_PORT, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    LGNTRACE_MSG("request port:%d",Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset));
                    ParamManager::instance()->setRequestPort(Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset));
                }else {
                    return AKEY_RV_REQUEST_PORT_NO_SAM_PORT;
                }

                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_GETPORT_GET_TIME, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->setRequestPortTime(Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset));
                }
                return hr;
            }
            
        private:
            AppPackage* m_pAppPkg;
        };
        
        class CReleasePort : public IPackage
        {
        public:
            CReleasePort(AppPackage* pAppPkg)
            {
                m_pAppPkg = pAppPkg;
            }
            
            virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CReleasePort MakePackage");
                UINT32 u32PkgLen = 0;
                BYTE btBuffer[DEFAULT_BUFFERLEN] = {0};
                LPBYTE pbPackageBuf = btBuffer;
                const CBuffer& SNBuf = IDCardInfo::instance()->getSN();
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_TRADE_CODE, (LPBYTE)"\x08\x02",2, pbPackageBuf + u32PkgLen);
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_GETPORT_READER_SN, SNBuf.GetBuffer(),SNBuf.GetLength(), pbPackageBuf + u32PkgLen);
                const CBuffer& transBuf = m_pAppPkg->getTransBuf();
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_TRANSACTION_NUMBER, transBuf.GetBuffer(),transBuf.GetLength(), pbPackageBuf + u32PkgLen);
                memcpy(outBuf.ReAllocBytesSetLength(u32PkgLen), btBuffer, u32PkgLen);
                outBuf.SetLength(u32PkgLen);
                return AKEY_RV_OK;
            }
            
            virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CReleasePort ParsePackage");
                HRESULT hr = AKEY_RV_OK;
                LPBYTE pbInBuf = inBuf.GetBuffer();
                UINT32 u32InLen = inBuf.GetLength();
                UINT32 u32Offset = 0;
                int u16TagLen = 0;
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRADE_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0 ) {
                    if (Helper::BigEndian::UInt16FromBytes(pbInBuf + u32Offset) != 0x0802 || u16TagLen != 2) {
                        LGNTRACE_ERRORNO(AKEY_RV_RELEASE_PORT_TRADECODE_ERROR);
                        return AKEY_RV_RELEASE_PORT_TRADECODE_ERROR;
                    }
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ERROR_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if ((hr = Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset)) != AKEY_RV_OK) {
                        LGNTRACE_ERRORNO(hr);
                        return hr;
                    }
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_GETPORT_RELEASE_TIME, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->setRelPortTime(Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset));
                }

                return hr;
            }
        private:
            AppPackage* m_pAppPkg;
        };
        
        
        class CBuildSafeChannel : public IPackage
        {
        public:
            
            CBuildSafeChannel(AppPackage* pAppPkg)
            {
                m_pAppPkg = pAppPkg;
            }
            
            virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CBuildSafeChannel MakePackage");
                UINT32 u32PkgLen = 0;
                BYTE btBuffer[DEFAULT_BUFFERLEN] = {0};
                LPBYTE pbPackageBuf = btBuffer;
                m_pAppPkg->setRequestNum(0);
                const CBuffer& SNBuf = IDCardInfo::instance()->getSN();
                CBuffer customIDBuf;
                ParamManager::instance()->getBusinessID(customIDBuf);
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_TRADE_CODE, (LPBYTE)"\x01\x01",2, pbPackageBuf + u32PkgLen);
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_READER_SERIAL_NUMBER, SNBuf.GetBuffer(),SNBuf.GetLength(), pbPackageBuf + u32PkgLen);
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_CUSTOM_ID, customIDBuf.GetBuffer(),customIDBuf.GetLength(), pbPackageBuf + u32PkgLen);
                const CBuffer& transBuf = m_pAppPkg->getTransBuf();//流水号
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_TRANSACTION_NUMBER, transBuf.GetBuffer(),transBuf.GetLength(), pbPackageBuf + u32PkgLen);
                m_u16SeqNum = m_pAppPkg->getRequestNum();
                BYTE btReqNum[2] = {0};
                Helper::BigEndian::UInt16ToBytes(m_u16SeqNum, btReqNum);
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_SEQ_NUMBER, btReqNum,2, pbPackageBuf + u32PkgLen);
                
                u32PkgLen += CPackageMaker::MakeTLVPacket(TAG_DEVICE_FLOW, inBuf.GetBuffer(),inBuf.GetLength(), pbPackageBuf + u32PkgLen);
                
                memcpy(outBuf.ReAllocBytesSetLength(u32PkgLen), btBuffer, u32PkgLen);
                outBuf.SetLength(u32PkgLen);
                return AKEY_RV_OK;
            }
            
            virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CBuildSafeChannel ParsePackage");
                LPBYTE pbInBuf = inBuf.GetBuffer();
                UINT32 u32InLen = inBuf.GetLength();
                UINT32 u32Offset = 0;
                HRESULT hr = AKEY_RV_OK;
                int u16TagLen = 0;
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRADE_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0 ) {
                    if (Helper::BigEndian::UInt16FromBytes(pbInBuf + u32Offset) != 0x0101 || u16TagLen != 2) {
                        LGNTRACE_ERRORNO(AKEY_RV_BUILD_SAFECHANNEL_TRADECODE_ERROR);
                        return AKEY_RV_BUILD_SAFECHANNEL_TRADECODE_ERROR;
                    }
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_TRADECODE;
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_BATCH_NUMBER, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    UINT16 u16BatchNum = Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset);
                    m_pAppPkg->setBatchNum(u16BatchNum);
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_BATCH;
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_SWITCH, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    //日志开关
                    m_pAppPkg->setLogFlag(pbInBuf[u32Offset]);
                }else {
                    m_pAppPkg->setLogFlag(0);//如果没有发送开关，默认不上传
                }
                
                //
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRANSACTION_NUMBER, pbInBuf, u32InLen, &u32Offset);
                LGNTRACE_HEX("transaction num:", pbInBuf+u32Offset, u16TagLen);
                if (u16TagLen > 0) {
                    m_pAppPkg->setTranstion(pbInBuf+u32Offset, u16TagLen);
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_TRANSTION_NUM;
                }

				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SAM_CONTROLLER_VERSION, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					IDCARD_WORD_MODE mode = ParamManager::instance()->getWorkMode();
					if (mode == MODE_NET_NO_REQ_PORT || 
						mode == MODE_NET_GATEWAY) {
						CLog::instance()->setSamVer(pbInBuf+u32Offset,u16TagLen);
					}
				}
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ERROR_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if ((hr = Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset)) != AKEY_RV_OK) {
                        LGNTRACE_ERRORNO(hr);
                        return hr;
                    }
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SEQ_NUMBER, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    UINT16 u16CurSeqNum = m_pAppPkg->getCurrentReqNum();
                    UINT16 u16SeqNum = Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset);
                    if (u16SeqNum != u16CurSeqNum) {
                        LGNTRACE_ERRORNO(AKEY_RV_BUILD_SAFECHANNEL_SEQ_NUM_ERROR);
                        return AKEY_RV_BUILD_SAFECHANNEL_SEQ_NUM_ERROR;
                    }
                }

                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEVICE_FLOW, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen == -1) {
                    LGNTRACE_ERRORNO(AKEY_RV_BUILD_SAFECHANNEL_NO_DEVICE_FLOW);
                    return AKEY_RV_BUILD_SAFECHANNEL_NO_DEVICE_FLOW;
                }
                memcpy(outBuf.ReAllocBytesSetLength(u16TagLen), pbInBuf+u32Offset, u16TagLen);
                
                //
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_NET_APP_HANDLE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->AppendNetSAM(pbInBuf+u32Offset, u16TagLen);
                }
                
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_SAFE_COM, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->AppendSafeComSAM(pbInBuf+u32Offset, u16TagLen);
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_COM_TIME_LOG;
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SAM_INDEX, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    m_pAppPkg->setSamIndex(pbInBuf[u32Offset]);
                }
                
                return AKEY_RV_OK;
            }
        private:
            AppPackage* m_pAppPkg;
            UINT16 m_u16SeqNum;
        };
        
        //验签
        class CVerifySign : public IPackage
        {
        public:
            
            CVerifySign(AppPackage* pAppPkg)
            {
                m_pAppPkg = pAppPkg;
            }
            
            virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                BYTE btBuffer[DEFAULT_BUFFERLEN];
                UINT32 u32Len = 0;
                u32Len += CPackageMaker::MakeTLVPacket(TAG_TRADE_CODE, (LPBYTE)"\x01\x02",2, btBuffer + u32Len);
                UINT16 u16BatchNum = m_pAppPkg->getBatchNum();
                BYTE baBatch[2];
                Helper::BigEndian::UInt16ToBytes(u16BatchNum, baBatch);
                u32Len += CPackageMaker::MakeTLVPacket(TAG_BATCH_NUMBER, baBatch,2, btBuffer + u32Len);
                
                const CBuffer& transBuf = m_pAppPkg->getTransBuf();
                u32Len += CPackageMaker::MakeTLVPacket(TAG_TRANSACTION_NUMBER, transBuf.GetBuffer(),transBuf.GetLength(), btBuffer + u32Len);
                
                UINT16 u16ReqNum = m_pAppPkg->getRequestNum();
                BYTE baReq[2];
                Helper::BigEndian::UInt16ToBytes(u16ReqNum, baReq);
                u32Len += CPackageMaker::MakeTLVPacket(TAG_SEQ_NUMBER,baReq,2, btBuffer + u32Len);
                
                u32Len += CPackageMaker::MakeTLVPacket(TAG_DEVICE_FLOW,inBuf.GetBuffer(),inBuf.GetLength(), btBuffer + u32Len);
				const CBuffer& SNBuf = IDCardInfo::instance()->getSN();
                u32Len += CPackageMaker::MakeTLVPacket(TAG_READER_SERIAL_NUMBER, SNBuf.GetBuffer(),SNBuf.GetLength(), btBuffer + u32Len);

				CBuffer customIDBuf;
				ParamManager::instance()->getBusinessID(customIDBuf);
                u32Len += CPackageMaker::MakeTLVPacket(TAG_CUSTOM_ID, customIDBuf.GetBuffer(),customIDBuf.GetLength(), btBuffer + u32Len);
                memcpy(outBuf.ReAllocBytesSetLength(u32Len), btBuffer, u32Len);
                return AKEY_RV_OK;
            }
            
            
            virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CVerifySign ParsePackage");
                HRESULT hr = AKEY_RV_OK;
                LPBYTE pbInBuf = inBuf.GetBuffer();
                UINT32 u32InLen = inBuf.GetLength();
                UINT32 u32Offset = 0;
                int u16TagLen = 0;
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRADE_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if (0x0102 != Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset)) {
                        LGNTRACE_ERRORNO(AKEY_RV_BUILD2_SAFECHANNEL_TRADECODE_ERROR);
                        return AKEY_RV_BUILD2_SAFECHANNEL_TRADECODE_ERROR;
                    }
                    //return AKEY_RV_BUILD2_SAFECHANNEL_NO_TRADECODE;
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ERROR_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if ((hr = Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset)) != AKEY_RV_OK) {
                        LGNTRACE_ERRORNO(hr);
                        return hr;
                    }
                    //return AKEY_RV_BUILD2_SAFECHANNEL_NO_ERRORCODE;
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRANSACTION_NUMBER, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    const CBuffer& transBuf = m_pAppPkg->getTransBuf();
                    if ((memcmp(transBuf.GetBuffer(), pbInBuf+u32Offset, u16TagLen) != 0) ||
                        (u16TagLen != transBuf.GetLength())) {
                        LGNTRACE_HEX("CVerifySign local transNum:", transBuf.GetBuffer(), transBuf.GetLength());
                        LGNTRACE_HEX("CVerifySign server transNum:", pbInBuf+u32Offset, u16TagLen);
                    }
                }

				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SEQ_NUMBER, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					UINT16 u16CurSeqNum = m_pAppPkg->getCurrentReqNum();
					UINT16 u16SeqNum = Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset);
					if (u16SeqNum != u16CurSeqNum) {
						LGNTRACE_ERRORNO(AKEY_RV_BUILD_SAFECHANNEL_SEQ_NUM_ERROR);
						return AKEY_RV_BUILD2_SAFECHANNEL_SEQ_NUM_ERROR;
					}
				}
                
                //
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEVICE_FLOW, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen == -1) {
                    LGNTRACE_ERRORNO(AKEY_RV_BUILD2_SAFECHANNEL_NO_DEVICE_FLOW);
                    return AKEY_RV_BUILD2_SAFECHANNEL_NO_DEVICE_FLOW;
                }
                
                memcpy(outBuf.ReAllocBytesSetLength(u16TagLen), pbInBuf+u32Offset, u16TagLen);
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_NET_APP_HANDLE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->AppendNetSAM(pbInBuf+u32Offset, u16TagLen);
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_NET_TIME_LOG;
                }
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_SAFE_COM, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->AppendSafeComSAM(pbInBuf+u32Offset, u16TagLen);
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_COM_TIME_LOG;
                }
                return AKEY_RV_OK;
            }
        private:
            AppPackage* m_pAppPkg;
        };
        
        
        //指令交互
        class CCommandExchange : public IPackage
        {
        public:
            
            CCommandExchange(AppPackage* pAppPkg)
            {
                m_pAppPkg = pAppPkg;
            }
            
            //入参：设备指令流,出参：交易报文
            virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                BYTE btBuffer[DEFAULT_BUFFERLEN];
                UINT32 u32Len = 0;
                
                u32Len += CPackageMaker::MakeTLVPacket(TAG_TRADE_CODE, (LPBYTE)"\x01\x03",2, btBuffer + u32Len);
                UINT16 u16BatchNum = m_pAppPkg->getBatchNum();
                BYTE baBatch[2];
                Helper::BigEndian::UInt16ToBytes(u16BatchNum, baBatch);//批次号
                u32Len += CPackageMaker::MakeTLVPacket(TAG_BATCH_NUMBER, baBatch,2, btBuffer + u32Len);
                
                const CBuffer& transBuf = m_pAppPkg->getTransBuf();//流水号
                u32Len += CPackageMaker::MakeTLVPacket(TAG_TRANSACTION_NUMBER, transBuf.GetBuffer(),transBuf.GetLength(), btBuffer + u32Len);
                
                UINT16 u16ReqNum = m_pAppPkg->getRequestNum();
                BYTE baReq[2];
                Helper::BigEndian::UInt16ToBytes(u16ReqNum, baReq);//顺序号
                u32Len += CPackageMaker::MakeTLVPacket(TAG_SEQ_NUMBER,baReq,2, btBuffer + u32Len);
                
                u32Len += CPackageMaker::MakeTLVPacket(TAG_DEVICE_FLOW,inBuf.GetBuffer(),inBuf.GetLength(), btBuffer + u32Len);

				const CBuffer& SNBuf = IDCardInfo::instance()->getSN();
				u32Len += CPackageMaker::MakeTLVPacket(TAG_READER_SERIAL_NUMBER, SNBuf.GetBuffer(),SNBuf.GetLength(), btBuffer + u32Len);

				CBuffer customIDBuf;
				ParamManager::instance()->getBusinessID(customIDBuf);
				u32Len += CPackageMaker::MakeTLVPacket(TAG_CUSTOM_ID, customIDBuf.GetBuffer(),customIDBuf.GetLength(), btBuffer + u32Len);

                memcpy(outBuf.ReAllocBytesSetLength(u32Len), btBuffer, u32Len);
                return AKEY_RV_OK;
            }
            
            //入参：交易报文 出参：设备指令流
            virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CCommandExchange ParsePackage");
                HRESULT hr = AKEY_RV_OK;
                LPBYTE pbInBuf = inBuf.GetBuffer();
                UINT32 u32InLen = inBuf.GetLength();
                UINT32 u32Offset = 0;
                int u16TagLen = 0;
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRADE_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if (0x0103 != Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset)) {
                        LGNTRACE_ERRORNO(AKEY_RV_INS_EXCHANGE_TRADECODE_ERROR);
                        return AKEY_RV_INS_EXCHANGE_TRADECODE_ERROR;
                    }
                    //return AKEY_RV_INS_EXCHANGE_NO_TRADECODE;
                }
                //
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ERROR_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if ((hr = Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset)) != AKEY_RV_OK) {
                        LGNTRACE_ERRORNO(hr);
                        return hr;
                    }
                }

				//u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SEQ_NUMBER, pbInBuf, u32InLen, &u32Offset);
				//if (u16TagLen > 0) {
				//	UINT16 u16CurSeqNum = m_pAppPkg->getCurrentReqNum();
				//	UINT16 u16SeqNum = Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset);
				//	if (u16SeqNum != u16CurSeqNum) {
				//		LGNTRACE_ERRORNO(AKEY_RV_BUILD_SAFECHANNEL_SEQ_NUM_ERROR);
				//		return AKEY_RV_INS_EXCHANGE_SEQ_NUM_ERROR;
				//	}
				//}

				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_BATCH_NUMBER, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					UINT16 u16BatchNum = Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset);
					m_pAppPkg->setBatchNum(u16BatchNum);
					//return AKEY_RV_BUILD_SAFECHANNEL_NO_BATCH;
				}
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRANSACTION_NUMBER, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    //const CBuffer& transBuf = m_pAppPkg->getTransBuf();
                    //if ((memcmp(transBuf.GetBuffer(), pbInBuf+u32Offset, u16TagLen) != 0) ||
                    //    (u16TagLen != transBuf.GetLength())) {
                    //    LGNTRACE_HEX("CCommandExchange local transNum:", transBuf.GetBuffer(), transBuf.GetLength());
                    //    LGNTRACE_HEX("CCommandExchange server transNum:", pbInBuf+u32Offset, u16TagLen);
                    //}
					m_pAppPkg->setTranstion(pbInBuf+u32Offset,u16TagLen);
                }


				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SAM_CONTROLLER_VERSION, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					IDCARD_WORD_MODE mode = ParamManager::instance()->getWorkMode();
					if (mode == MODE_NET_NO_REQ_PORT || 
						mode == MODE_NET_GATEWAY) {
						CLog::instance()->setSamVer(pbInBuf+u32Offset,u16TagLen);
					}
				}
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_DEVICE_FLOW, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen == -1) {
                    LGNTRACE_ERRORNO(AKEY_RV_INS_EXCHANGE_NO_DEVICE_FLOW);
                    return AKEY_RV_INS_EXCHANGE_NO_DEVICE_FLOW;
                }
                
                memcpy(outBuf.ReAllocBytesSetLength(u16TagLen), pbInBuf+u32Offset, u16TagLen);
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_NET_APP_HANDLE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->AppendNetSAM(pbInBuf+u32Offset, u16TagLen);
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_NET_TIME_LOG;
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_SAFE_COM, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->AppendSafeComSAM(pbInBuf+u32Offset, u16TagLen);
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_COM_TIME_LOG;
                }
				//如果UDP认证时，返回sam索引
				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_SAM_INDEX, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					m_pAppPkg->setSamIndex(pbInBuf[u32Offset]);
				}

				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_IDCARD_NOTE_INFO, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					ParamManager::instance()->setNoteInfo(pbInBuf+u32Offset,u16TagLen);
				}

				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_IMAGE_AVAIABLE, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					ParamManager::instance()->setImageAvaiableFlag(pbInBuf[u32Offset]);
				}
                return hr;
            }
        private:
            AppPackage* m_pAppPkg;
        };
        
        class CReadImage : public IPackage
        {
        public:
            
            CReadImage(AppPackage* pAppPkg)
            {
                m_pAppPkg = pAppPkg;
            }
            
            virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
				LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CReadImage MakePackage");
                BYTE btEncBuf[DEFAULT_BUFFERLEN];
                UINT32 u32EncLen = 0;
                CBuffer encBuf;
                UINT16 u16BatchNum = m_pAppPkg->getBatchNum();
                BYTE baBatch[2];
                Helper::BigEndian::UInt16ToBytes(u16BatchNum, baBatch); //批次号
                u32EncLen += CPackageMaker::MakeTLVPacket(TAG_BATCH_NUMBER, baBatch,2, btEncBuf + u32EncLen);
                
                const CBuffer& transBuf = m_pAppPkg->getTransBuf();     //流水号
                u32EncLen += CPackageMaker::MakeTLVPacket(TAG_TRANSACTION_NUMBER, transBuf.GetBuffer(),transBuf.GetLength(), btEncBuf + u32EncLen);
                
                UINT16 u16ReqNum = m_pAppPkg->getRequestNum();          //顺序号
                BYTE baReq[2];
                Helper::BigEndian::UInt16ToBytes(u16ReqNum, baReq);
                u32EncLen += CPackageMaker::MakeTLVPacket(TAG_SEQ_NUMBER,baReq,2, btEncBuf + u32EncLen);
                u32EncLen += CPackageMaker::MakeTLVPacket(TAG_TRADE_CODE,(LPBYTE)"\x01\x04",2, btEncBuf + u32EncLen);

				CBuffer customIDBuf;
				ParamManager::instance()->getBusinessID(customIDBuf);
				u32EncLen += CPackageMaker::MakeTLVPacket(TAG_CUSTOM_ID, customIDBuf.GetBuffer(),customIDBuf.GetLength(), btEncBuf + u32EncLen);

				const CBuffer& SNBuf = IDCardInfo::instance()->getSN();
				u32EncLen += CPackageMaker::MakeTLVPacket(TAG_READER_SERIAL_NUMBER, SNBuf.GetBuffer(),SNBuf.GetLength(), btEncBuf + u32EncLen);
                memcpy(outBuf.ReAllocBytesSetLength(u32EncLen), btEncBuf, u32EncLen);
    
                return AKEY_RV_OK;
            }
            
            virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "CReadImage ParsePackage");
                HRESULT hr = AKEY_RV_OK;
                LPBYTE pbInBuf = inBuf.GetBuffer();
                UINT32 u32InLen = inBuf.GetLength();
                UINT32 u32Offset = 0;
                int u16TagLen = 0;
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRADE_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if (0x0104 != Helper::BigEndian::UInt16FromBytes(pbInBuf+u32Offset)) {
                        LGNTRACE_ERRORNO(AKEY_RV_IMAGE_TRADECODE_ERROR);
                        return AKEY_RV_IMAGE_TRADECODE_ERROR;
                    }
                }
                
                //
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_ERROR_CODE, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    if ((hr = Helper::BigEndian::UInt32FromBytes(pbInBuf+u32Offset)) != AKEY_RV_OK) {
                        LGNTRACE_ERRORNO(hr);
                        return hr;
                    }
                }
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_TRANSACTION_NUMBER, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    const CBuffer& transBuf = m_pAppPkg->getTransBuf();
                    if ((memcmp(transBuf.GetBuffer(), pbInBuf+u32Offset, u16TagLen) != 0) ||
                        (u16TagLen != transBuf.GetLength())) {
                        LGNTRACE_HEX("CReadImage local transNum:", transBuf.GetBuffer(), transBuf.GetLength());
                        LGNTRACE_HEX("CReadImage server transNum:", pbInBuf+u32Offset, u16TagLen);
                    }
                }
                
                //
                UINT32 u32KeyOffset = 0;
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_IMAGE_PROGRESS_KEY, pbInBuf, u32InLen, &u32KeyOffset);
                if (u16TagLen == -1) {
                    LGNTRACE_ERRORNO(AKEY_RV_IMAGE_NO_PROGRESS_KEY);
                    return AKEY_RV_IMAGE_NO_PROGRESS_KEY;
                }
                UINT32 u32ImageOffset = 0;
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_IMAGE_ENCED_IMAGE_DATA, pbInBuf, u32InLen, &u32ImageOffset);
                if (u16TagLen == -1) {
                    LGNTRACE_ERRORNO(AKEY_RV_IMAGE_NO_ENC_IMAGE_DATA);
                    return AKEY_RV_IMAGE_NO_ENC_IMAGE_DATA;
                }
                const CBuffer& AppKey = IDCardInfo::instance()->getAppKey();
                hr = CDecryptIDCard::DecIDCardPackage(AppKey.GetBuffer(),pbInBuf + u32KeyOffset,pbInBuf+u32ImageOffset,u16TagLen,outBuf);
                if (hr != AKEY_RV_OK) {
                    return hr;
                } 

				u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_NET_APP_HANDLE, pbInBuf, u32InLen, &u32Offset);
				if (u16TagLen > 0) {
					CLog::instance()->AppendNetSAM(pbInBuf+u32Offset, u16TagLen);
					//return AKEY_RV_BUILD_SAFECHANNEL_NO_NET_TIME_LOG;
				}
                
                u16TagLen = CPackageMaker::ParseTLVPacket(TAG_LOG_SAFE_COM, pbInBuf, u32InLen, &u32Offset);
                if (u16TagLen > 0) {
                    CLog::instance()->AppendSafeComSAM(pbInBuf+u32Offset, u16TagLen);
                    //return AKEY_RV_BUILD_SAFECHANNEL_NO_COM_TIME_LOG;
                }
                return hr;
            }
        private:
            AppPackage* m_pAppPkg;
        };
        
        class CVersionManager : public IPackage
        {
        public:
            
            CVersionManager(AppPackage* pAppPkg)
            {
                m_pAppPkg = pAppPkg;
            }
            
            virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                return AKEY_RV_OK;
            }
            
            virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)
            {
                return AKEY_RV_OK;
            }
        private:
            AppPackage* m_pAppPkg;
        };
        
    public:
        HRESULT MakePackage(IDREADER_PROCESS_STATUS proStatus,CBuffer& inBuf,CBuffer& outBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            switch (proStatus) {
                case PROGRESS_BUILD_SAFECHANNEL1:
                {
                    setRequestNum(0);
                    CBuildSafeChannel    m_BSCPkg(this);
                    hr = m_BSCPkg.MakePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_BUILD_SAFECHANNEL2:
                {
                    CVerifySign          m_VerSignPkg(this);
                    hr = m_VerSignPkg.MakePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_BEGIN:
                case PROGRESS_CMD_EXCHANGE_SINGLE:
                case PROGRESS_CMD_EXCHANGE_MUL:
                case PROGRESS_CMD_ERROR:
                {
                    CCommandExchange     m_cmdExcPkg(this);
                    hr = m_cmdExcPkg.MakePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_REQUEST_IMAGE:
                {
                    CReadImage           m_imagePkg(this);
                    hr = m_imagePkg.MakePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_VERSION_MANAGER:
                {
                    CVersionManager      m_VersionPkg(this);
                    hr = m_VersionPkg.MakePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_GET_PORT:
                {
                    CRequestPort    m_ReqPortPkg(this);
                    hr = m_ReqPortPkg.MakePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_RELEASE_PORT:
                {
                    CReleasePort m_ResPortPkg(this);
                    hr = m_ResPortPkg.MakePackage(inBuf, outBuf);
                }
                    break;
                default:
                    hr = AKEY_RV_PACKAGE_PROCESS_STATUS_ERROR;
                    break;
            }
            return hr;
        }
        
        
        HRESULT ParsePackage(IDREADER_PROCESS_STATUS proStatus,CBuffer& inBuf,CBuffer& outBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            switch (proStatus) {
                case PROGRESS_BUILD_SAFECHANNEL1:
                {
                    CBuildSafeChannel    m_BSCPkg(this);
                    hr = m_BSCPkg.ParsePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_BUILD_SAFECHANNEL2:
                {
                    CVerifySign          m_VerSignPkg(this);
                    hr = m_VerSignPkg.ParsePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_CMD_EXCHANGE_BEGIN:
                case PROGRESS_CMD_EXCHANGE_SINGLE:
                case PROGRESS_CMD_EXCHANGE_MUL:
                case PROGRESS_CMD_ERROR:
                {
                    CCommandExchange     m_cmdExcPkg(this);
                    hr = m_cmdExcPkg.ParsePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_REQUEST_IMAGE:
                {
                    CReadImage           m_imagePkg(this);
                    hr = m_imagePkg.ParsePackage(inBuf,outBuf);
                }
                    break;
                    
                case PROGRESS_VERSION_MANAGER:
                {
                    CVersionManager      m_VersionPkg(this);
                    hr = m_VersionPkg.ParsePackage(inBuf,outBuf);
                }
                    break;
                case PROGRESS_GET_PORT:
                {
                    CRequestPort    m_ReqPortPkg(this);
                    hr = m_ReqPortPkg.ParsePackage(inBuf, outBuf);
                }
                    break;
                case PROGRESS_RELEASE_PORT:
                {
                    CReleasePort m_ResPortPkg(this);
                    hr = m_ResPortPkg.ParsePackage(inBuf, outBuf);
                }
                    break;
                    
                default:
                    hr = AKEY_RV_PARSE_PROCESS_STATUS_ERROR;
                    break;
            }
            return hr;
        }
        
        void setSamIndex(BYTE samIndex)
        {
            m_samIndex = samIndex;
        }
        
        void setRequestNum(BYTE requestNum)
        {
            m_requestNum = requestNum;
        }
        
        void setBatchNum(UINT16 u16BatchNum)
        {
            m_batchNum = u16BatchNum;
        }
        
        void setLogFlag(BYTE logFlag)
        {
            m_logFlag = logFlag;
        }
        
        void setTranstion(LPBYTE pbTranstion,UINT32 u32TranstionLen)
        {
            memcpy(m_TransBuf.ReAllocBytesSetLength(u32TranstionLen), pbTranstion, u32TranstionLen);
        }
        
        void setError(UINT32 error)
        {
            m_error = error;
        }
        
        void setSamPort(UINT32 u32SamPort)
        {
            m_samPort = u32SamPort;
        }
        
        BYTE getSamIndex()
        {
            return m_samIndex;
        }
        
        BYTE getRequestNum()
        {
            return (++m_requestNum);
        }

		BYTE getCurrentReqNum()
		{
			return m_requestNum;
		}
        
        const CBuffer& getTransBuf()
        {
            return m_TransBuf;
        }
        
        UINT16 getBatchNum()
        {
            return m_batchNum;
        }
        
        BYTE getLogFlag()
        {
            return m_logFlag;
        }
        
        UINT32 getSamPort()
        {
            return m_samPort;
        }
        
    private:
        CBuffer                    m_TransBuf;      //参考号
        CBuffer                    m_netModule;     //网卡模组编号
        BYTE                       m_samIndex;      //sam索引
        UINT16                     m_requestNum;    //顺序号
        UINT16                     m_batchNum;      //批次号
        BYTE                       m_logFlag;       //是否开启日志的开关
        CLog                       m_log;           //客户端日志管理类
        UINT32                     m_error;
        UINT32                     m_samPort;
        friend class CBuildSafeChannel;
        friend class CVerifySign;
        friend class CCommandExchange;
        friend class CVersionManager;
        friend class CReadImage;
    };
};


#endif /* IDCardPackage_h */
