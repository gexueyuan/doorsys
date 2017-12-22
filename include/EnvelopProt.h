#ifndef __AKEY_ENVELOPCOMM_H__
#define __AKEY_ENVELOPCOMM_H__

#include "AKeyDef.h"
#include "AKeyInterface.h"
#include "AKeyHelper.h"


namespace AKey
{
	// class CEnvelopComm
	class CEnvelopComm : public IProtocol
	{
	public:
		CEnvelopComm() : m_pSubProtocol(NULL)
		{
		}

		virtual HRESULT Init(IProtocol * pSubProtocol)
		{
			m_pSubProtocol = pSubProtocol;

			return AKEY_RV_OK;
		}
		
		virtual HRESULT Final()
		{
			return AKEY_RV_OK;
		}

		static HRESULT DoTransmit(IProtocol * pProtocolObj, LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			const UINT32 blockSize = 0xF7; // 18*14=252(0xFC = 5+0xF7)
			BYTE sendBuff[260], recvBuff[260];
			UINT32 offset=0, recvLen;
			memcpy(sendBuff, "\x80\xC2\x00\x01", 4);
			if (unSLen > blockSize)
			{
				sendBuff[4] = (BYTE)blockSize;
				for(; offset < (unSLen-blockSize);  offset+=blockSize)
				{
					memcpy(sendBuff+5, pbSend+offset, blockSize);
					recvLen = sizeof(recvBuff);
					HRESULT hr = Execute_Reader(pProtocolObj, sendBuff, 5+sendBuff[4], recvBuff, &recvLen);
					if (hr != AKEY_RV_OK)
					{
						return hr;
					}
					sendBuff[3] = 0x00;
				}
			}

			// last block
			sendBuff[3] |= 0x02;
			sendBuff[4] = (BYTE)(unSLen - offset);
			memcpy(sendBuff+5, pbSend+offset, sendBuff[4]);
			recvLen = sizeof(recvBuff);
			HRESULT hr = pProtocolObj->Control(AKEY_PROTOCOL_COMMAND_TRANSMIT, sendBuff, 5+sendBuff[4], recvBuff, &recvLen, 30000); // SW：6B00/900X

			offset = 0;
			if ((hr == AKEY_RV_OK) && (recvLen >= 2) && (recvBuff[recvLen-2] == 0x6B)) // 多包应答
			{
				recvLen -= 2; // 去掉SW
				for (;;)
				{
					memcpy(pbRecv+offset, recvBuff, recvLen);
					offset += recvLen;

					recvLen = sizeof(recvBuff);
					hr = Execute_Reader(pProtocolObj, (LPBYTE)"\x80\xC1\x00\x00\xFA", 5, recvBuff, &recvLen); // 判断SW, 18*14=252(0xFC = 0xFA+2)
					if (hr != (AKEY_RV_SW_BASE + 0x6B00))
					{
						break;
					}
				}
			}

			if (hr != AKEY_RV_OK)
			{
				return hr;
			}

			memcpy(pbRecv+offset, recvBuff, recvLen);
			*punRLen = offset + recvLen;
			return AKEY_RV_OK;
		}
			

	public: // IProtocol
		virtual HRESULT Control(UINT32 unCmd, LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen, UINT32 unTimeout)
		{
			if (m_pSubProtocol != NULL)
			{
				return DoTransmit(m_pSubProtocol, pbSend, unSLen, pbRecv, punRLen);
			}
			else
			{
				return AKEY_RV_FAIL;
			}
		}

		virtual IProtocol * GetSubProtocol()
		{
			return m_pSubProtocol;
		}

	protected:
		static HRESULT Execute_Reader(IProtocol * pProtocolObj, LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			HRESULT hr = pProtocolObj->Control(AKEY_PROTOCOL_COMMAND_TRANSMIT, pbSend, unSLen, pbRecv, punRLen, 30000);			
			if (hr == AKEY_RV_OK)
			{
				UINT32 dwStatus = 0xFFFF;
				if ((*punRLen) >= 2)
				{
					(*punRLen) -= 2;
					dwStatus = (pbRecv[*punRLen] << 8) + pbRecv[(*punRLen)+1];
				}

				if (dwStatus != 0x9000)
				{
					hr = AKEY_RV_SW_BASE +  dwStatus;
				}
			}
			return hr;
		}

	protected:
		IProtocol * m_pSubProtocol;
	};

};

#endif
