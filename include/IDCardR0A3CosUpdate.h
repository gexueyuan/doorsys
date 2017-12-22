//
//  IDCardCosUpdate.h
//  TDRToken
//
//  Created by zhaowei on 2016/10/21.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardCosUpdate_h
#define IDCardCosUpdate_h

#include "AKeyBase.h"
#include "AKeyDef.h"
#include "AKeyHelper.h"
#include "AKeyError.h"
#include "AKeyInterface.h"

using namespace AKey;

namespace IDCARD {
    class IDCardR0A3Update
    {
    public:
        IDCardR0A3Update(IProtocol * reader):m_Reader(reader) {
            
        }
		IDCardR0A3Update() {
			m_Reader = NULL;
		}

        ~IDCardR0A3Update(){}

		void InitReader(IProtocol* reader) {
			m_Reader = reader;
		}
        
        HRESULT ReadBaseInfo(CBuffer& cosSignBuf) {
            HRESULT hr = AKEY_RV_OK;
            CBuffer recvBuf;
            hr = Reader_Execute((LPBYTE)"\x00\xA4\x04\x00\x08\xA0\x00\x00\x01\x03\x53\x50\x42", 13, recvBuf);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            hr = Reader_Execute((LPBYTE)"\x80\xCA\xCE\x24\x01\x01", 6, cosSignBuf);
            if (hr != AKEY_RV_OK) {
                return hr;
            }
            return hr;
        }

		HRESULT ReadUpdateCert(CBuffer& certBuf)
		{
			HRESULT hr = AKEY_RV_OK;
			hr = Reader_Execute((LPBYTE)"\x80\xCA\xCE\x12\x04\x00\x00\x04\x00", 9, certBuf);
			return hr;
		}

		HRESULT OpenSwith(LPBYTE pbSwithData,UINT32 u32SwithLen)
		{
			HRESULT hr = AKEY_RV_OK;
			UINT32 u32TmpLen = (u32SwithLen < 0x100)?5:6;
			CBuffer sendBuf(u32TmpLen+u32SwithLen);
			CBuffer recvBuf;
			memcpy(sendBuf.GetBuffer(),(LPBYTE)"\x80\xDE\x00\x00\x00",5);
			if (u32SwithLen < 0x100)
			{
				sendBuf.GetBuffer()[4] = u32SwithLen;
				memcpy(sendBuf.GetBuffer()+5,pbSwithData,u32SwithLen);
				sendBuf.SetLength(5+u32SwithLen);
			}
			else 
			{
				Helper::BigEndian::UInt16ToBytes(u32SwithLen,sendBuf.GetBuffer()+5);
				memcpy(sendBuf.GetBuffer()+7,pbSwithData,u32SwithLen);
				sendBuf.SetLength(7+u32SwithLen);
			}
			hr = Reader_Execute(sendBuf.GetBuffer(),sendBuf.GetLength(),recvBuf);
			return hr;
		}

		HRESULT ScanDevice()
		{
			HRESULT hr = AKEY_RV_OK;
			CBuffer recvBuf;
			hr = Reader_Execute((LPBYTE)"\xFD\x00\x00\x00\x00",5,recvBuf);
			return hr;
		}

		
		HRESULT WriteEncProKey(LPBYTE pbEncProKey,UINT32 u32EncProKeyLen)
		{
			HRESULT hr = AKEY_RV_OK;
			CBuffer sendBuf(9+u32EncProKeyLen);
			CBuffer recvBuf;
			memcpy(sendBuf.GetBuffer(),(LPBYTE)"\x80\xE2\x80\x00\x00\xCE\x02\x00\x00",9);
			sendBuf.GetBuffer()[4] = (4+u32EncProKeyLen);
			sendBuf.GetBuffer()[8] = u32EncProKeyLen;
			memcpy(sendBuf.GetBuffer()+9,pbEncProKey,u32EncProKeyLen);
			sendBuf.SetLength(9+u32EncProKeyLen);
			hr = Reader_Execute(sendBuf.GetBuffer(),sendBuf.GetLength(),recvBuf);
			return hr;
		}

		HRESULT WriteCosData(LPBYTE pbCosData,UINT32 u32CosDataLen,UINT32 u32PerPkgSize)
		{
			if (pbCosData == NULL || u32CosDataLen == 0)
			{
				return AKEY_RV_TOKEN_INVALID_PARAM;
			}
			HRESULT hr = AKEY_RV_OK;
			CBuffer sendBuf,recvBuf;
			LPBYTE pbSend = sendBuf.ReAllocBytes(7 + u32PerPkgSize);
			for (int i=0,j=0; i< u32CosDataLen ; i+=u32PerPkgSize,j++){
				UINT32 u32SendPkgLen = u32PerPkgSize;
				if ((i + u32PerPkgSize) >= u32CosDataLen){
					j += 0x8000; // last
					u32SendPkgLen =  u32CosDataLen - i;
				}
				memcpy(pbSend,(LPBYTE)"\x80\x22",2);
				Helper::BigEndian::UInt16ToBytes(j,pbSend+2);
				pbSend[4] = 0x00;
				Helper::BigEndian::UInt16ToBytes(u32SendPkgLen,pbSend+5);

				memcpy(pbSend+7,pbCosData+i,u32SendPkgLen);
				hr = Reader_Execute(pbSend,7+u32SendPkgLen,recvBuf);
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
			}
			return hr;
		}
        
                
    protected:
        HRESULT Reader_Execute(LPBYTE pbSend, UINT32 unSLen,CBuffer& outBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Cos Reader_Execute");
            LGNTRACE_HEX("Reader Send:", pbSend, unSLen);
            LPBYTE pbRecvBuf = outBuf.ReAllocBytes(DEFAULT_BUFFERLEN);
            UINT32 u32RecvLen = DEFAULT_BUFFERLEN;
            HRESULT hr = Transmit(pbSend, unSLen, 0, pbRecvBuf, &u32RecvLen);
            if (hr != AKEY_RV_OK)
            {
                LGNTRACE_ERRORNO(hr);
                return hr;
            }
            LGNTRACE_HEX("Reader Recv:", pbRecvBuf, u32RecvLen);
            UINT32 status = (u32RecvLen < 2)? 0xFFFF : ((pbRecvBuf[u32RecvLen-2] << 8) + pbRecvBuf[u32RecvLen-1]);
            if (status != 0x9000)
            {
                LGNTRACE_ERRORNO(status);
                return AKEY_RV_SW_BASE + status;
            }
            u32RecvLen -= 2;
            outBuf.SetLength(u32RecvLen);
            return AKEY_RV_OK;
        }
        
        HRESULT Transmit(LPBYTE pbSend, UINT32 unSLen,UINT32 unMaxLe,LPBYTE pbRecv,LPUINT32 punRLen)
        {
            if (m_Reader == NULL) {
                return AKEY_RV_FAIL;
            }
            return m_Reader->Control(AKEY_PROTOCOL_COMMAND_TRANSMIT,pbSend, unSLen, pbRecv, punRLen,0);
        }
        
        int ParseTLVPacket(BYTE tag,LPBYTE pbValue,UINT32 u32ValueLen,UINT32* pu32Offset)
        {
            for (UINT32 i = 0;  i< u32ValueLen;) {
                int len = pbValue[i+1];
                if (tag == pbValue[i]) {
                    if (pu32Offset) {
                        *pu32Offset = i+2;
                        return len;
                    }
                }
                i += 1 + 1 + len;
            }
            return -1;
        }
        
    private:
        IProtocol* m_Reader;
        CBuffer m_snBuf;
        CBuffer m_cosVerBuf;
    };
};


#endif /* IDCardCosUpdate_h */
