#ifndef __AKEY_SESSION_H__
#define __AKEY_SESSION_H__

#include "AKeyDef.h"
#include "AKeyInterface.h"
#include "AKeyHelper.h"
#include "alg/MatrixCrypt.h"
#include "alg/OpenAlg.h"


namespace AKey
{
	class CSessionBase : public ISession
	{
	public:
		CSessionBase() : m_pProtocolObj(NULL), m_pReaderObj(NULL)
		{
			memset(&m_status, 0, sizeof(m_status));
		}

		virtual ~CSessionBase()
		{
			//Final();
		}

	public: // ISession
		virtual HRESULT Init(IReader *pReaderObj, IProtocol *pProtocolObj = NULL, bool bAutoFreeProtocolObj = false)
		{
			m_pReaderObj = pReaderObj;
			m_pProtocolObj = (pProtocolObj == NULL)? pReaderObj : pProtocolObj; 
			memset(&m_status, 0, sizeof(m_status));
			m_status.u16MaxApdu_Le = 0xEE; // 4个通讯包（HID）
			m_status.u16MaxApdu_Lc = 0xEB; // 4个通讯包（HID）
			m_status.bAutoFreeProtocolObj = bAutoFreeProtocolObj;
			return AKEY_RV_OK;
		}

		virtual HRESULT Final()
		{
			if (m_status.bAutoFreeProtocolObj)
			{
				while(m_pProtocolObj)
				{
					IProtocol *pProtocolObj = m_pProtocolObj;
					m_pProtocolObj = pProtocolObj->GetSubProtocol();
					delete m_pProtocolObj;
				}
				m_status.bAutoFreeProtocolObj = false;
			}
			return AKEY_RV_OK;
		}

		//virtual HRESULT Open(bool bSupportEncryptApdu)
		//{
		//	return AKEY_RV_OK;
		//}

		virtual HRESULT Close()
		{
			m_status.bSessionActive = 0;
			m_status.bSessionEncrypt = 0;
			m_status.u32PubKey1bNLen = 0;
			return AKEY_RV_OK;
		}

		virtual HRESULT Transmit(LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			if (m_status.bSessionEncrypt)
			{
				 return Transmit_Encrypt(pbSend, unSLen, pbRecv, punRLen);
			}
			else
			{
				 return Transmit_Plain(pbSend, unSLen, pbRecv, punRLen);
			}
		}

		virtual UINT32 GetFlags()
		{
			return 0;
		}

		virtual IReader * GetReader()
		{
			return m_pReaderObj;
		}

		virtual AKEY_SESSION_STATUS_PTR GetStatus()
		{
			return &m_status;
		}

	protected:
		// 执行APUD指令（明文）
		HRESULT Transmit_Plain(LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			m_status.bDeviceRand = false;
			if ((pbSend[0] == 0xE0) && (pbSend[1] == 0xFE))
			{
				return m_pProtocolObj->Control(AKEY_PROTOCOL_COMMAND_TRANSMIT, pbSend, unSLen, pbRecv, punRLen, 30000);
			}
			else
			{
				LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Transmit_Plain");
				LGNTRACE_HEX("session send data : ", pbSend, unSLen);
				HRESULT hr = m_pProtocolObj->Control(AKEY_PROTOCOL_COMMAND_TRANSMIT, pbSend, unSLen, pbRecv, punRLen, 30000);
				LGNTRACE_HEX("session recv data : ", pbRecv, (hr==AKEY_RV_OK)? *punRLen : 0);
				LGNTRACE_ERRORNO(hr);
				return hr;
			}
		}
		HRESULT Execute_Plain(LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			HRESULT hr = Transmit_Plain(pbSend, unSLen, pbRecv, punRLen);
			if (hr == AKEY_RV_OK)
			{
				UINT32 dwStatus = ((*punRLen) < 2)? 0 : ((pbRecv[(*punRLen)-2] << 8) + pbRecv[(*punRLen)-1]);
				if (dwStatus != 0x9000)
				{
					hr = AKEY_RV_SW_BASE +  dwStatus;
				}
				(*punRLen) -= 2; // 去掉SW
			}
			return hr;
		}


		// 执行APUD指令（加密）
		HRESULT Transmit_Encrypt(LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Transmit_Encrypt");
			LGNTRACE_HEX("token send data* : ", pbSend, unSLen);

			//E0FE00+ID+LC(00 + XXXX)+Data
			CBuffer sendBuff(7+unSLen+16);
			LPBYTE pbSendBuf = sendBuff;

			const char * pszAlgName = (m_status.btSessionEncType==0)? "DES-ECB" : "AES-128-ECB";
			//UINT32 dwKeyLen = (m_tokenStatus.btSessionEncType == 0)? 8 : 16;

			// 加密数据
			memcpy(pbSendBuf+7, pbSend, unSLen);
			int nTmpLen = OpenAlg::CCipher::Cipher(pszAlgName, m_status.abSessionKey, NULL, OpenAlg::CCipher::ENCRYPT, pbSend, unSLen, pbSendBuf+7);
			if (nTmpLen <= 0)
			{
				return AKEY_RV_FAIL;
			}

			// 执行加密指令
			if (nTmpLen <= 0xFF)
			{
				pbSendBuf += 2;
				pbSendBuf[4] = (BYTE)(nTmpLen & 0xFF);;
				nTmpLen += 5;
			}
			else
			{
				pbSendBuf[4] = 0x00;
				Helper::BigEndian::UInt16ToBytes((UINT16)nTmpLen, pbSendBuf+5);
				nTmpLen += 7;
			}
			memcpy(pbSendBuf, "\xE0\xFE\x00\x00", 4);
			pbSendBuf[3] = m_status.btSessionEncKeyId;

			UINT32 dwRecvLen = (*punRLen);
			HRESULT hr = Execute_Plain(pbSendBuf, nTmpLen, pbRecv, &dwRecvLen);
			if (hr == AKEY_RV_OK)
			{
				// 解密
				CBuffer recvBuff_t(dwRecvLen);
				memcpy((LPBYTE)recvBuff_t, pbRecv, dwRecvLen);
				nTmpLen = OpenAlg::CCipher::Cipher(pszAlgName, m_status.abSessionKey, NULL, 0,  (LPBYTE)recvBuff_t, dwRecvLen, pbRecv);
				if (nTmpLen > 0)
				{
					*punRLen = nTmpLen;
					LGNTRACE_HEX("token recv data* : ", pbRecv, *punRLen);
				}
				else
				{
					hr = AKEY_RV_FAIL;
				}
			}

			LGNTRACE_ERRORNO(hr);
			return hr;
		}

	protected:
		IProtocol * m_pProtocolObj;
		IReader * m_pReaderObj;
		AKEY_SESSION_STATUS m_status;
	};

	// 默认会话类
	class CDefaultSession : public CSessionBase
	{
	public: // ISession
		virtual HRESULT Open(bool bSupportEncryptApdu)
		{
			m_status.bSessionActive = 1;
			m_status.bSessionEncrypt = 0;

			// 加密通讯
			if (bSupportEncryptApdu)
			{
				if (m_status.u32PubKey1bNLen == 0)
				{
					//获取N
					UINT32 u32TmpLen = sizeof(m_status.abPubKey1bN);
					HRESULT hr = Execute_Plain((LPBYTE)"\xE0\xB4\x01\x1B\x02\x20\x00", 7, m_status.abPubKey1bN, &u32TmpLen);
					if (hr != AKEY_RV_OK)
					{
						return hr;
					}
					m_status.u32PubKey1bNLen = u32TmpLen;
				}

				// 生成主密钥
				BYTE abRandom[16];
				OpenAlg::CRand::GetBytes(NULL, 0, abRandom, 16);
				
				// 获取通信会话密钥
				BYTE abSendBuff[300], abRecvBuff[300];
				UINT32 dwEncResultLen = 0x80, dwRecvLen = sizeof(abRecvBuff);
				memcpy(abSendBuff, "\xE0\xFD\x00\x00\x82\x1B\x00", 7);
				dwEncResultLen = OpenAlg::CRsa::PublicCrypt(m_status.abPubKey1bN, m_status.u32PubKey1bNLen, 0x00010001, OpenAlg::CRsa::ENCRYPT, abRandom, 16, abSendBuff+7);
				if ((int)dwEncResultLen <= 0)
				{
					return AKEY_RV_FAIL;
				}
				HRESULT hr  = Execute_Plain(abSendBuff, 7+dwEncResultLen, abRecvBuff, &dwRecvLen);
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}

				m_status.btSessionEncKeyId = abRecvBuff[0];
				m_status.btSessionEncType = abRecvBuff[1];
				const char * pszAlgName = (m_status.btSessionEncType==0)? "DES-EDE" : "AES-128-ECB";
				int nSessionKeyLen = OpenAlg::CCipher::Cipher(pszAlgName, abRandom, NULL, OpenAlg::CCipher::NOPADDING, abRecvBuff+2, dwRecvLen-2, m_status.abSessionKey);
				if (nSessionKeyLen <= 0)
				{
					return AKEY_RV_FAIL;
				}
				m_status.bSessionEncrypt = 1;
			}
			return AKEY_RV_OK;
		}
	};

	// 蓝牙会话类
	class CBluetoothSession  : public CSessionBase
	{
	public:
		CBluetoothSession() : m_bNewDevice(false)
		{
		}

	public: // ISession
		virtual HRESULT Open(bool bSupportEncryptApdu)
		{
			m_status.bSessionActive = 1;
			m_status.bSessionEncrypt = 0;
			if (bSupportEncryptApdu)
			{
				BYTE abName[20], abLinkCode[4*8], abDeviceRand[8];
				UINT32 unNameLen = sizeof(abName);
				UINT32 unLinkCodeLen = sizeof(abLinkCode);
				UINT32 unRSSI=0;
				bool bFirst = false;

				// 获取蓝牙配对信息
				HRESULT hr = GetInfo(abName, &unNameLen, &bFirst, &unRSSI);
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}

				BYTE abSendBuff[30] = {0xE0, 0xFA, 0x00, 0x00};
				BYTE abRecvBuff[300];
				UINT32 unRecvLen = sizeof(abRecvBuff);
				abSendBuff[2] = bFirst? 0x80 : 0x00;
				abSendBuff[4] = (BYTE)unNameLen;
				memcpy(abSendBuff+5, abName, unNameLen);
				// 请求连接蓝牙设备
				hr = Execute_Plain(abSendBuff, 5+abSendBuff[4], abRecvBuff, &unRecvLen);
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
				m_bNewDevice = (abRecvBuff[0] == 0);
				m_status.btSessionEncKeyId = 0x01; // 固定
				m_status.btSessionEncType = abRecvBuff[1];
				memcpy(abDeviceRand, abRecvBuff+2, 8); // unRecvLen-2

				for (;;)
				{
					// 获取连接码 (hr = AKEY_RV_SESSION_INVALID_CODE)
					hr = GetLinkCode(hr, abLinkCode, &unLinkCodeLen);
					if (hr != AKEY_RV_OK)
					{
						return hr;
					}

					// 解密出明文PIN
					if (CMatrixCrypt::Instance()->IsValid())
					{
						BYTE abLinkCode_t[8];
						unLinkCodeLen = AKey::CMatrixCrypt::Instance()->Decrypt(abLinkCode, unLinkCodeLen, abLinkCode_t);
						memset(abLinkCode, 0, sizeof(abLinkCode));
						memcpy(abLinkCode, abLinkCode_t, unLinkCodeLen);
						memset(abLinkCode_t, 0, sizeof(abLinkCode_t));
					}

					// 计算通讯密钥
					OpenAlg::CDigest::Digest(Helper::HashType2Name(AKEY_HASH_MD5), abLinkCode, unLinkCodeLen, m_status.abSessionKey);

					if (CMatrixCrypt::Instance()->IsValid())
					{
						memset(abLinkCode, 0, unLinkCodeLen); // 清除运行时敏感数据
//						AKey::CMatrixCrypt::Instance()->Cleanup();
					}

					// 连接确认
					OpenAlg::CCipher::Cipher("DES-EDE", m_status.abSessionKey, NULL, OpenAlg::CCipher::ENCRYPT|OpenAlg::CCipher::NOPADDING, abDeviceRand, 8, abSendBuff+5);
					abSendBuff[2] = 0x01;
					abSendBuff[3] = (BYTE)unRSSI;
					abSendBuff[4] = 4;
					unRecvLen = sizeof(abRecvBuff);
					hr = Execute_Plain(abSendBuff, 5+abSendBuff[4], abRecvBuff, &unRecvLen);
					if (hr == AKEY_RV_OK)
					{
						OpenAlg::CCipher::Cipher("DES-EDE", m_status.abSessionKey, NULL, OpenAlg::CCipher::NOPADDING, abRecvBuff, 8, m_status.abSessionKey);
						m_status.bSessionEncrypt = 1;
						break;
					}
				}
				return hr;
			}
			return AKEY_RV_OK;
		}

		virtual HRESULT Close()
		{
			//if (m_status.bSessionEncrypt)
			//{
			//	BYTE abRecvBuff[300];
			//	UINT32 unRecvLen = sizeof(abRecvBuff);
			//	Execute_Plain((LPBYTE)"\xE0\xFA\x02\x00\x00", 5, 0, abRecvBuff, &unRecvLen);
			//}
			return CSessionBase::Close();
		}

		virtual UINT32 GetFlags()
		{
			return AKKY_FLAG_TINIT_MUST_SESSION;
		}
	protected:
		// 获取手机唯一名称
		virtual HRESULT GetInfo(LPBYTE pbName, LPUINT32 punNameLen, bool * pbFirst, LPUINT32 punRSSI) = 0;
		// 获取连接码
		virtual HRESULT GetLinkCode(HRESULT hr, LPBYTE pbCode, LPUINT32 punCodeLen) = 0;
		/*
			if (m_bNewDevice)
			{
				// 输入连接码(弹界面)，用户可取消
				// 保存连接码到配置
			}
			else
			{
				if (hr == AKEY_RV_OK)
					// 从配置读取连接码
				else
					// 报错，清除配置(只保留名称)
			}
		*/

	protected:
		bool m_bNewDevice;

	};
};

#endif
