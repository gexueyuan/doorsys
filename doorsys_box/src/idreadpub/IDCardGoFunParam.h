//
//  IDCardParam.cpp
//  TDRToken
//
//  Created by zhaowei on 15/12/4.
//  Copyright © 2015年 lujun. All rights reserved.
//

#include "IDCardInterface.h"
#include "IDCardConfig.h"
#include <stddef.h>
#include "IDCardError.h"
#include "IDReaderStorage.h"

namespace IDCARD {
    
    class IDCardGoFunParam : public IIDCardReaderInterface
    {
    public:
        IDCardGoFunParam(IProtocol * reader,IToken* token,IDReaderStorage* idReaderStorage)
        {
            m_Reader = reader;
            m_token = token;
            m_idReaderStorage = idReaderStorage;
        }
        ~IDCardGoFunParam(){}

		void setReader(IProtocol* reader)
		{
			m_Reader = reader;
		}

        
        virtual HRESULT Prepare()
        {
            HRESULT hr = AKEY_RV_OK;
            hr = m_token->Prepare(AKKY_FLAG_TOKEN_NEW_DEVICE|AKKY_FLAG_TOKEN_CHECK_DEVICE);
            if(hr != AKEY_RV_OK){
                return hr;
            }
            return hr;
        }

		//select your app id 
		virtual HRESULT ChooseApp()
		{
			HRESULT hr = AKEY_RV_OK;
			CBuffer resultBuf;
			//gofun
			hr = Reader_Execute((LPBYTE)IDCARD_SELECT_IDCARD_AID,16,resultBuf);
			//doorsys
			//hr = Reader_Execute((LPBYTE)IDCARD_SELECT_GOFUN_AID,16,resultBuf);
			return hr;
		}
        
        virtual HRESULT GetReaderCert(LPBYTE pbRecv,LPUINT32 punRLen)
        {
            HRESULT hr = AKEY_RV_OK;
            BYTE btParamID = IDCARD_PARAM_READCERT;
            hr = m_token->ReadParam(0, AKEY_PARAM_IDCARD_READERCERT, &btParamID, 1, pbRecv,punRLen);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            return hr;
        }
        
        /**
         *  GetCosSpeedFlag 查询cos是否支持加速，以及如果支持加速，查询是否处于加速状态
         *
         *  @param pbReady 就绪标示(0:表示未就绪，1:表示就绪)
         *
         *  @return 错误码
         */
        virtual HRESULT GetCosSpeedFlag(bool* bSupportSpeed,BYTE* bSpeed)
        {
            HRESULT hr = AKEY_RV_OK;
            BYTE btParamID = IDCARD_PARAM_COSSPEED;
            BYTE btCosSpeed[2];
            UINT32 u32Len = 2;
            hr = m_token->ReadParam(0, AKEY_PARAM_IDCARD_COSSPEED, &btParamID, 1, btCosSpeed,&u32Len);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            if (bSupportSpeed) {
                *bSupportSpeed = (bool)btCosSpeed[0];
                if (*bSupportSpeed) {
                    *bSpeed = btCosSpeed[1];
                }
            }
            return hr;
        }
        
        
        virtual HRESULT GetReaderInfo(LPBYTE pbCosVer,LPUINT32 punCosVerLen)
        {
            HRESULT hr = AKEY_RV_OK;
            BYTE btParamID = IDCARD_PARAM_COSVERSION;
            hr = m_token->ReadParam(0, AKEY_PARAM_IDCARD_COSVERSION, &btParamID, 1, pbCosVer, punCosVerLen);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            return hr;
        }
        
        virtual HRESULT GetReaderPuk(BYTE btKeyID, LPBYTE pbOut,LPUINT32 punOutResult)
        {
            HRESULT hr = AKEY_RV_OK;
            hr = m_token->ReadParam(0, AKEY_PARAM_PUBLICKEY, &btKeyID, 1, pbOut, punOutResult);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            return hr;
        }
        
        virtual HRESULT GetBatteryLevel(BYTE* bBattery)
        {
            HRESULT hr = AKEY_RV_OK;
            BYTE abCosInfo[0x80];
            UINT32 u32CosInfoLen = sizeof(abCosInfo);
            hr = m_token->ReadParam(0, AKEY_PARAM_BITMAP_INFO, (LPBYTE)"\x00\x00\x00\x40\x00\x00\x00\x00\x00\x00", 10, abCosInfo, &u32CosInfoLen);
			if (hr == AKEY_RV_OK && u32CosInfoLen > 8) {
				if (bBattery) {
					UINT32 v1 = Helper::LittleEndian::UInt32FromBytes(abCosInfo);
					if (v1 & (1<<14))
					{
						*bBattery = abCosInfo[8];
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
        
        virtual HRESULT SupportResume(BYTE* bSupportResume)
        {
            HRESULT hr = AKEY_RV_OK;
            BYTE abCosInfo[0x80];
            UINT32 u32CosInfoLen = sizeof(abCosInfo);
            hr = m_token->ReadParam(0, AKEY_PARAM_BITMAP_INFO, (LPBYTE)"\x00\x00\x00\x00\x80\x00\x00\x00\x00\x00", 10, abCosInfo, &u32CosInfoLen);
            if (hr == AKEY_RV_OK && u32CosInfoLen > 8) {
                if (bSupportResume) {
					UINT32 v1 = Helper::LittleEndian::UInt32FromBytes(abCosInfo);
					if (v1 & (1<<23))
					{
						*bSupportResume = abCosInfo[8];
						return AKEY_RV_OK;
					}else {
						return AKEY_RV_NOT_SUPPORT_RESUME;
					}
                }
            }
            return hr;
        }
        
        virtual HRESULT GetReaderCosInfo(CBuffer& timeInterval,CBuffer& batteryHealth,CBuffer& readerBehavior)
        {
            HRESULT hr = AKEY_RV_OK;
            
            BYTE abCosInfo[0x80];
            UINT32 u32CosInfoLen = sizeof(abCosInfo);
            UINT32 u32Offset = 8;
            hr = m_token->ReadParam(0,AKEY_PARAM_BITMAP_INFO, (LPBYTE)"\x00\x00\x00\x00\x00\x07\x00\x00\x00\x00", 10, abCosInfo, &u32CosInfoLen);
            if ((hr == AKEY_RV_OK) && (u32CosInfoLen > 8))
            {
                memcpy(timeInterval.ReAllocBytesSetLength(4), (abCosInfo+u32Offset), 4);
                u32Offset+=4;
                memcpy(batteryHealth.ReAllocBytesSetLength(29), abCosInfo+u32Offset, 29);
                u32Offset+=29;
                memcpy(readerBehavior.ReAllocBytesSetLength(14), abCosInfo+u32Offset, 14);
            }else if(hr == AKEY_RV_OK) {
                return AKEY_RV_BITMAP_LENGTH_ERROR;
            }
            return hr;
        }
        
        /**
         *  发送指令
         *
         *  @param pbSend  数据内容
         *  @param unSLen  数据长度
         *  @param unMaxLe 最大长度
         *  @param pbRecv  接收缓冲区
         *  @param punRLen 接收长度
         *
         *  @return 错误码
         */
        virtual HRESULT Transmit(LPBYTE pbSend, UINT32 unSLen,UINT32 unMaxLe,LPBYTE pbRecv,LPUINT32 punRLen)
        {
            if (m_Reader == NULL) {
                return AKEY_RV_FAIL;
            }
            return m_Reader->Control(AKEY_PROTOCOL_COMMAND_TRANSMIT,pbSend, unSLen, pbRecv, punRLen,0);
        }

        
        /**
         *  SM2签名
         *
         *  @param keyID       密钥ID
         *  @param pbInput     需要签名的数据
         *  @param u32InputLen 数据长度
         *  @param pbOutput    签名结果
         *  @param punOutput   签名结果的长度
         *
         *  @return 错误码
         */
        virtual HRESULT SM2Sign(BYTE keyID,LPBYTE pbPuK,UINT16 u16PukLen,LPBYTE pbInput,UINT32 u32InputLen,LPBYTE pbOutput,LPUINT32 punOutput)
        {
            HRESULT hr = AKEY_RV_OK;
            BYTE btSignResult[0x100] = {0};
            UINT32 u32SignResultLen = 0x100;
            BYTE abKeyID[1];
            abKeyID[0] = keyID;
            CBuffer hashBuff;
            int nHashLen = 0x80;
            if (u16PukLen == 65) {
                nHashLen = OpenAlg::CDigest::GetInternalData_SM3(pbPuK,u16PukLen, pbInput, u32InputLen, NULL, hashBuff.ReAllocBytes(nHashLen));
                if (nHashLen <= 0) {
                    hr = AKEY_RV_TOKEN_INVALID_DATA;
                    return hr;
                }
                hr = m_token->Sign(keyID, AKEY_FLAG_SIGN_HASH_SM3, hashBuff.GetBuffer(), nHashLen, 0, NULL, 0, btSignResult, &u32SignResultLen);
            }else{
                nHashLen = OpenAlg::CDigest::Digest(Helper::HashType2Name(AKEY_FLAG_SIGN_HASH_SHA1 & 0xFF), pbInput, u32InputLen, hashBuff.ReAllocBytes(nHashLen));
                hr = m_token->Sign(keyID, AKEY_FLAG_SIGN_HASH_SHA1, hashBuff.GetBuffer(),nHashLen, 0, NULL, 0, btSignResult, &u32SignResultLen);
            }
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            if (pbOutput) {
                memcpy(pbOutput, btSignResult, u32SignResultLen);
            }
            if (punOutput) {
                *punOutput = u32SignResultLen;
            }
            return hr;
        }
        virtual HRESULT SM2Encrypt(LPBYTE pbInput,UINT32 u32InputLen,LPBYTE pbKey,UINT32 u32KeyLen,LPBYTE pbOutput,LPUINT32 punOutput)
        {
            HRESULT hr = AKEY_RV_OK;
            return hr;
        }
        
        HRESULT SaveSamCertAndVersion(CBuffer& samVersion,CBuffer& samCert)
        {
            HRESULT hr = AKEY_RV_OK;
            hr = m_idReaderStorage->SaveSamCertAndVersion(samVersion,samCert);
            return hr;
        }
        
        HRESULT LoadSamCertAndVersion(CBuffer& samVersion,CBuffer& samCert)
        {
            HRESULT hr = AKEY_RV_OK;
            hr = m_idReaderStorage->LoadSamCertAndVersion(samVersion,samCert);
            return hr;
        }
	protected:
		HRESULT Reader_Execute(LPBYTE pbSend, UINT32 unSLen,CBuffer& outBuf)
		{
			LPBYTE pbRecvBuf = outBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
			UINT32 u32RecvLen = DEFAULT_BUFFERLEN;
			HRESULT hr = Transmit(pbSend, unSLen, 0, pbRecvBuf, &u32RecvLen);
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}
			UINT32 status = (u32RecvLen < 2)? 0xFFFF : ((pbRecvBuf[u32RecvLen-2] << 8) + pbRecvBuf[u32RecvLen-1]);
			if (status != 0x9000)
			{
				return AKEY_RV_SW_BASE + status;
			}
			u32RecvLen -= 2;
			return AKEY_RV_OK;
		}
        
        
    private:
        IProtocol* m_Reader;
        IToken*  m_token;
        IDReaderStorage* m_idReaderStorage;
    };
    
};
