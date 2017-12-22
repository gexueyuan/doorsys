#ifndef __AKEY_COMMAND_H__
#define __AKEY_COMMAND_H__

#include "AKeyDef.h"
#include "AKeyInterface.h"
#include "AKeyHelper.h"
#include "alg/MatrixCrypt.h"
#include "alg/OpenAlg.h"

namespace AKey
{
	// class CCommand
	class CCommand
	{
	public:
		CCommand(ISession * pSessionObj) : m_pSessionObj(pSessionObj), m_pCustomCommand(NULL)
		{
		}
		virtual ~CCommand()
		{
			if (m_pCustomCommand != NULL)
			{
				delete m_pCustomCommand;
			}
		}

		inline AKEY_SESSION_STATUS_PTR GetSessionStatus()
		{
			return m_pSessionObj->GetStatus();
		}
		
		inline HRESULT CreateCustomCommand(UINT32 dwCustomId,int deviceType);

		// 执行APUD指令（根据令牌状态来判断是否采用加密方式）
		HRESULT Execute(LPBYTE pbSend, UINT32 unSLen, UINT32 unMaxLe, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			HRESULT hr = Execute(pbSend, unSLen, unMaxLe);
			if (hr == AKEY_RV_OK)
			{
				if (pbRecv != NULL)
				{
					memcpy(pbRecv, m_abRecvBuff, m_dwRecvLen);
				}
				if (punRLen != NULL)
				{
					*punRLen = m_dwRecvLen;
				}
			}
			return hr;
		}

		HRESULT Execute(LPBYTE pbSend, UINT32 unSLen, UINT32 unMaxLe=0)
		{
			m_dwRecvLen = sizeof(m_abRecvBuff);
			HRESULT hr = m_pSessionObj->Transmit(pbSend, unSLen, m_abRecvBuff, &m_dwRecvLen);
			if (hr == AKEY_RV_OK)
			{
				UINT32 dwStatus = (m_dwRecvLen < 2)? 0 : ((m_abRecvBuff[m_dwRecvLen-2] << 8) + m_abRecvBuff[m_dwRecvLen-1]);
				if (dwStatus != 0x9000)
				{
					hr = AKEY_RV_SW_BASE +  dwStatus;
				}
				m_dwRecvLen -= 2; // 去掉SW
			}
			return hr;
		}

		HRESULT MakeApduAndExcute(LPBYTE pbApduHead, LPBYTE pbDataBeginPtr, LPBYTE pbDataEndPtr)
		{
			UINT32 dwDataLen = (UINT32)(pbDataEndPtr - pbDataBeginPtr);
			if (dwDataLen > 0xFF) // 扩展指令
			{
				LPBYTE ptr = pbDataBeginPtr - 7;
				memcpy(ptr, pbApduHead, 5);
				Helper::BigEndian::UInt16ToBytes((UINT16)dwDataLen, ptr+5); //P3
				return Execute(ptr, 7+dwDataLen);
			}
			else
			{
				LPBYTE ptr = pbDataBeginPtr - 5;
				memcpy(ptr, pbApduHead, 4);
				ptr[4] = (BYTE)dwDataLen;
				return Execute(ptr, 5+dwDataLen);
			}
		}

		HRESULT DevelopData(UINT32 dwCosBuffLen, LPBYTE pbData, UINT32 dwDataLen)
		{
			HRESULT hr = AKEY_RV_OK;

			if (dwCosBuffLen == 0)
			{
				dwCosBuffLen = GetSessionStatus()->bSessionEncrypt? 144 : 0xA0 - 7; // (144 + 7) + 1 + 7 = 159 (0x9F)
				if (Execute((LPBYTE)"\xE0\xC2\x00\x00\x00", 5, 0x02) == AKEY_RV_OK) // 必须执行MSE后，执行该指令才能成功
				{
					dwCosBuffLen = (m_abRecvBuff[0]<<8) + m_abRecvBuff[1];
				}
			}

			// 如果要两个数据包，保证第一包有64字节（交行-判断编码）
			if ((dwDataLen > dwCosBuffLen) && (dwCosBuffLen > 64) && (dwDataLen - dwCosBuffLen < 64))
			{
				dwCosBuffLen = dwDataLen - 64;
			}

			CBuffer sendBuff(8+dwCosBuffLen);
			LPBYTE pbSendBuff = sendBuff;
			// put data
			memcpy(pbSendBuff, "\xE0\xC2\x00\x00\x00", 5);
			if (dwDataLen > dwCosBuffLen)
			{
				UINT32 dwDataLen_t = dwDataLen - dwCosBuffLen;
				for (UINT32 i=0; (hr == AKEY_RV_OK) && (i<dwDataLen_t); i+=dwCosBuffLen)
				{
					UINT32 dwCurrLen = ((dwDataLen_t - i) < dwCosBuffLen)? (dwDataLen_t - i) : dwCosBuffLen;
					pbSendBuff[3] = (i == 0)? 0x01: 0x00;
					UINT32 dwSendLen_t = MakeApduPacket(pbData+i, dwCurrLen, NULL, pbSendBuff);
					hr = Execute(pbSendBuff, dwSendLen_t);		
				}
			}
			if (hr == AKEY_RV_OK) // 最后一包（满包）
			{
				UINT32 dwCurrLen = (dwDataLen > dwCosBuffLen)? dwCosBuffLen : dwDataLen;
				pbSendBuff[3] = ((dwDataLen > dwCosBuffLen))? 0x02: 0x03;
				UINT32 dwSendLen_t = MakeApduPacket(pbData+dwDataLen-dwCurrLen, dwCurrLen, NULL, pbSendBuff);
				hr = Execute(pbSendBuff, dwSendLen_t, 2);		
			}
			return hr;
		}

		// 选择文件
		virtual HRESULT SelectFile(BYTE btP2, UINT32 u32FileId, LPBYTE pbRecvBuff, LPUINT32 pu32RecvLen)
		{
			BYTE abSendBuff[7] = {0x00, 0xA4, 0x00, btP2, 0x02, (BYTE)((u32FileId >> 8) & 0xFF), (BYTE)((u32FileId) & 0xFF)};
			return (pbRecvBuff != NULL)? Execute(abSendBuff, 7, 0x40, pbRecvBuff, pu32RecvLen) : Execute(abSendBuff, 7);
		}

		// 读取二进制文件
		virtual HRESULT ReadBin(UINT32 u32FileId, UINT32 u32Offset, UINT32 u32Length, LPBYTE pbData)
		{
			HRESULT hr = AKEY_RV_OK;
			if (u32FileId != 0)
			{
				if ((hr = SelectFile(0x00, u32FileId, NULL, NULL)) != AKEY_RV_OK)
				{
					return hr;
				}
			}

			UINT32 u32MaxLe = GetSessionStatus()->u16MaxApdu_Le;
			if (u32MaxLe > (sizeof(m_abRecvBuff) - 0x20)) // 考虑到加密通讯需要占用一些
			{
				u32MaxLe = sizeof(m_abRecvBuff) - 0x20;
			}

			BYTE abSendBuff[7] = {0x00, 0xB0, 0x00, 0x00, 0x00};
			for (UINT32 i=0; i<u32Length; i+=u32MaxLe)
			{
				UINT32 u32ApduLen = 5;
				UINT32 u32ReadLen = (u32Length - i < u32MaxLe)? (u32Length - i) : u32MaxLe;
				Helper::BigEndian::UInt16ToBytes(u32Offset+i, abSendBuff+2);
				if (u32ReadLen <= 0xFF)
				{
					abSendBuff[4] = (BYTE)(u32ReadLen & 0xFF);
				}
				else
				{
					Helper::BigEndian::UInt16ToBytes((UINT16)u32ReadLen, abSendBuff+5);
					u32ApduLen = 7;
				}
				if ((hr = Execute(abSendBuff, u32ApduLen, u32ReadLen)) != AKEY_RV_OK)
				{
					return hr;
				}
				memcpy(pbData+i, m_abRecvBuff, u32ReadLen);
			}
			return AKEY_RV_OK;
		}

		// 更新二进制文件
		virtual HRESULT UpdateBin(UINT32 u32FileId, UINT32 u32Offset, UINT32 u32Length, LPBYTE pbData)
		{
			HRESULT hr = AKEY_RV_OK;
			if (u32FileId != 0)
			{
				if ((hr = SelectFile(0x00, u32FileId, NULL, NULL)) != AKEY_RV_OK)
				{
					return hr;
				}
			}

			UINT32 u32MaxLc = GetSessionStatus()->u16MaxApdu_Lc;

			CBuffer sendBuff(7+u32MaxLc);
			LPBYTE pbSendBuff = sendBuff;
			memcpy(pbSendBuff, "\x00\xD6\x00\x00\x00", 5);
			for (UINT32 i=0; i<u32Length; i+=u32MaxLc)
			{
				UINT32 u32WriteLen = (u32Length - i < u32MaxLc)? (u32Length - i) : u32MaxLc;
				Helper::BigEndian::UInt16ToBytes(u32Offset+i, pbSendBuff+2);
				UINT32 dwSendLen_t = MakeApduPacket(pbData+i, u32WriteLen, NULL, pbSendBuff);
				if ((hr = Execute(pbSendBuff, dwSendLen_t)) != AKEY_RV_OK)
				{
					return hr;
				}
			}
			return AKEY_RV_OK;
		}

		// 校验密码（支持明文和RSA两种）
		virtual HRESULT VerifyPin(UINT32 un32Flags, LPBYTE pbUserPin, UINT32 dwUserPinLen)
		{
			if (dwUserPinLen == 0) // hip
			{
				return Execute((LPBYTE)"\x84\x20\x00\x00\x00", 5);  // verifypin test 不支持PINID
			}

			// 公钥加密
			BYTE abSendBuff[300];
			UINT32 dwEncResultLen = sizeof(abSendBuff)-5;
			HRESULT hr = RsaEncryptPin(pbUserPin, dwUserPinLen, abSendBuff+5, &dwEncResultLen);
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}

			// 组合指令
			memcpy(abSendBuff, (LPBYTE)"\x84\x20\x00\x00\x80", 5);
			if (un32Flags & AKKY_FLAG_TOKEN_PINID2)
			{
				abSendBuff[3] = 0x01; // P2
			}
			abSendBuff[4] = (BYTE)dwEncResultLen;
			hr = Execute(abSendBuff, 5+dwEncResultLen);  // verifypin
			memset(abSendBuff, 0, 5+dwEncResultLen);
			return hr;
		}

		// 修改密码（只支持RSA方式）
		virtual HRESULT ChangePin(UINT32 un32Flags, LPBYTE pbOldPin, UINT32 dwOldPinLen, LPBYTE pbNewPin, UINT32 dwNewPinLen)
		{
			if ((dwOldPinLen == 0) && (dwNewPinLen == 0)) // hip
			{
				return Execute((LPBYTE)"\x84\x24\x00\x00\x00", 5);  // changepin test 不支持PINID
			}

			// 公钥加密
			BYTE abSendBuff[300];
			UINT32 dwEncResultLen = sizeof(abSendBuff)-5;
			HRESULT hr = RsaEncryptPin(pbOldPin, dwOldPinLen, pbNewPin, dwNewPinLen, abSendBuff+5, &dwEncResultLen);
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}

			// 组合指令
			memcpy(abSendBuff, (LPBYTE)"\x84\x24\x00\x00\x80", 5);
			if (un32Flags & AKKY_FLAG_TOKEN_PINID2)
			{
				abSendBuff[3] = 0x01; // P2
			}
			abSendBuff[4] = (BYTE)dwEncResultLen;
			hr = Execute(abSendBuff, 5+dwEncResultLen);  // changepin
			memset(abSendBuff, 0, 5+dwEncResultLen);
			return hr;
		}

		// 私钥解密（解密）
		virtual HRESULT Decrypt(UINT32 dwKeyID, LPBYTE pbEncryptedData, UINT32 dwEncryptedDataLen, LPBYTE pbData, LPUINT32 pdwDataLen)
		{
			if ((pbEncryptedData == NULL) || (dwEncryptedDataLen > 0x100) || (pdwDataLen == NULL))
				return AKEY_RV_TOKEN_INVALID_PARAM;
			if (pbData == NULL)
			{
				*pdwDataLen = 0x100;
				return AKEY_RV_OK;
			}

			BYTE abSendBuff[300];
			memcpy(abSendBuff, "\xE0\x22\x41\xB8\x03\x00\x00\x00", 8); // V1.3
			abSendBuff[5] = (BYTE)dwKeyID;
			HRESULT hr = Execute(abSendBuff, 8);
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}

			UINT32 dwSendLen_t = MakeApduPacket(pbEncryptedData, dwEncryptedDataLen, (LPBYTE)"\xE0\x2A\x80\x86", abSendBuff);
			hr = Execute(abSendBuff, dwSendLen_t, dwEncryptedDataLen-11, pbData, pdwDataLen);
			return hr;
		}

		// 普通签名
		HRESULT SignMSE(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbPCHash, UINT32 dwPCHashLen)
		{
			if ((pbPCHash == NULL) || (dwPCHashLen > 0x40))
			{
				return AKEY_RV_TOKEN_INVALID_PARAM;
			}

			BYTE abSendBuff[300];
			memcpy(abSendBuff, (LPBYTE)"\xE0\x22\x41\xB6\x03\x00\x00\x00", 8); // mse
			abSendBuff[4] = (BYTE)(dwPCHashLen + 3);
			abSendBuff[5] = (BYTE)(dwKeyID);
            abSendBuff[6] = (BYTE)((dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID) == 0)? (dwSignFlags | 0x20) : (dwSignFlags);
			memcpy(abSendBuff + 8, pbPCHash, dwPCHashLen);
			
			HRESULT hr = Execute(abSendBuff, dwPCHashLen + 8);
			m_tmpStatus.bResponseIncludeRandom = false;
			return hr;
		}		
		HRESULT SignMSE_Develop(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbTransData, UINT32 dwTransDataLen)
		{
			if (pbTransData == NULL)
			{
				return AKEY_RV_TOKEN_INVALID_PARAM;
			}

			BYTE abSendBuff[8];
			memcpy(abSendBuff, (LPBYTE)"\xE0\x22\x41\xB6\x03\x00\x00\x00", 8); // mse
			abSendBuff[5] = (BYTE)(dwKeyID);
			abSendBuff[6] = (BYTE)((dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID) == 0)? (dwSignFlags | 0x20) : (dwSignFlags);			
			HRESULT hr = Execute(abSendBuff, 5+abSendBuff[4]);
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}

			hr = DevelopData(0, pbTransData, dwTransDataLen);
			m_tmpStatus.bResponseIncludeRandom = false;
			return hr;
		}

		// 关键签名（双摘要）
		HRESULT SignMSE_DoubleHash(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbPCHash, UINT32 dwPCHashLen, LPBYTE pbShowData, UINT32 dwShowDataLen)
		{
			if ((pbPCHash == NULL) || (dwPCHashLen > 0x40) || (pbShowData == NULL) || (dwShowDataLen>2048))
			{
				return AKEY_RV_TOKEN_INVALID_PARAM;
			}

			UINT32 dwDataLen = 3 + dwPCHashLen + dwShowDataLen;
			CBuffer sendBuff(7+dwDataLen);
			LPBYTE pbSendBuff = sendBuff;

			memcpy(pbSendBuff, (LPBYTE)"\xE0\x22\x41\xE6\x00\x00\x00\x00\x00\x00\x00", 10); // mse
			UINT32 dwDataOff = 5;
			if (dwDataLen <= 0xFF)
			{
				pbSendBuff[4] = (BYTE)(dwDataLen & 0xFF);	//P3
			}
			else
			{
				Helper::BigEndian::UInt16ToBytes((UINT16)dwDataLen, pbSendBuff+5); //P3
				dwDataOff += 2;
			}
			
			pbSendBuff[dwDataOff] = (BYTE)dwKeyID;
			pbSendBuff[dwDataOff+1] = (BYTE)dwSignFlags;
			if (!(dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID))
			{
				pbSendBuff[dwDataOff+1] |= 0x20; // HashOID
			}
			if (dwSignFlags & AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS)
			{
				pbSendBuff[dwDataOff+2] = 0x80; // 蓝牙通讯地址
			}

			memcpy(pbSendBuff + dwDataOff + 3, pbPCHash, dwPCHashLen);
			memcpy(pbSendBuff + dwDataOff + 3 + dwPCHashLen, pbShowData, dwShowDataLen);
			HRESULT hr = Execute(pbSendBuff, dwDataOff + dwDataLen);
			m_tmpStatus.bResponseIncludeRandom = false;
			return hr;
		}

		// 关键签名（显示数据）
		HRESULT SignMSE_ShowData(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbShowData, UINT32 dwShowDataLen)
		{
			if (pbShowData == NULL)
			{
				return AKEY_RV_TOKEN_INVALID_PARAM;
			}

			BYTE abSendBuff_mse[8+6] = {0xE0, 0x22, 0x41, 0xE6, 0x03, (BYTE)dwKeyID, (BYTE)(dwSignFlags), 0x00};
			if (!(dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID))
			{
				abSendBuff_mse[6] |= 0x20; // HashOID
			}
			if (dwSignFlags & AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS)
			{
				dwShowDataLen -= 6;
				abSendBuff_mse[4] += 6;
				abSendBuff_mse[7] |= 0x80; // 蓝牙通讯地址
				memcpy(abSendBuff_mse+8, pbShowData+dwShowDataLen, 6);
			}
			HRESULT hr = Execute(abSendBuff_mse, 5+abSendBuff_mse[4]); // MSE //dwHashType: 0-SHA1, 1-MD5, 2-SHA256, 3-SHA384, 4-SHA512
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}

			hr = DevelopData((m_dwRecvLen >= 2)? ((m_abRecvBuff[0]<<8) + m_abRecvBuff[1]) : 0, pbShowData, dwShowDataLen);
			m_tmpStatus.bResponseIncludeRandom = false;
			return hr;
		}

		// 关键签名（摘要接力）
		HRESULT SignMSE_HashRelay(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbPCHash, UINT32 dwPCHashLen, UINT32 dwPCDataLen, LPBYTE pbShowData, UINT32 dwShowDataLen)
		{
			if ((pbPCHash == NULL) || (dwPCHashLen > 0x40) || (pbShowData == NULL) || (dwShowDataLen>2048))
			{
				return AKEY_RV_TOKEN_INVALID_PARAM;
			}

			UINT32 dwDataLen = 3 + 4 + dwPCHashLen + dwShowDataLen;
			CBuffer sendBuff(7+dwDataLen);
			LPBYTE pbSendBuff = sendBuff;

			memcpy(pbSendBuff, (LPBYTE)"\xE0\x22\x41\xE6\x00\x00\x00\x00\x00\x00\x00", 10); // mse
			UINT32 dwDataOff = 5;
			if (dwDataLen <= 0xFF)
			{
				pbSendBuff[4] = (BYTE)(dwDataLen);	//P3
			}
			else
			{
				Helper::BigEndian::UInt16ToBytes((UINT16)dwDataLen, pbSendBuff+5); //P3
				dwDataOff += 2;
			}
			
			pbSendBuff[dwDataOff] = (BYTE)dwKeyID;
			pbSendBuff[dwDataOff+1] = (BYTE)dwSignFlags;
			if (!(dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID))
			{
				pbSendBuff[dwDataOff+1] |= 0x20; // HashOID
			}
			if (dwSignFlags & AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS)
			{
				pbSendBuff[dwDataOff+2] = 0x80; // 蓝牙通讯地址
			}

			Helper::BigEndian::UInt32ToBytes(dwPCDataLen, pbSendBuff + dwDataOff + 3);
			memcpy(pbSendBuff + dwDataOff + 3 + 4, pbPCHash, dwPCHashLen);
			memcpy(pbSendBuff + dwDataOff + 3 + 4 + dwPCHashLen, pbShowData, dwShowDataLen);

			HRESULT hr = Execute(pbSendBuff, dwDataOff + dwDataLen, 2);
			m_tmpStatus.bResponseIncludeRandom = false;
			return hr;
		}

		// 关键签名（摘要接力2）
		HRESULT SignMSE_HashRelay2(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbPCHash, UINT32 dwPCHashLen, UINT32 dwPCDataLen, LPBYTE pbShowData, UINT32 dwShowDataLen)
		{
			if ((pbPCHash == NULL) || (dwPCHashLen > (0x40+0x80)) || (pbShowData == NULL) || (dwShowDataLen>2048))
			{
				return AKEY_RV_TOKEN_INVALID_PARAM;
			}

			const UINT32 dwHashType = (dwSignFlags & 0xFF);
			const UINT32 dwHashBlockSize = ((dwHashType==AKEY_FLAG_SIGN_HASH_SHA384) || (dwHashType==AKEY_FLAG_SIGN_HASH_SHA512))? 128 : 64; // SHA384，SHA512 = 128； 其他=64

			UINT32 dwDataLen = 3 + 4 + 1 + dwPCHashLen + dwShowDataLen; // 比SignMSE_HashRelay多一个字节（剩余长度）
			CBuffer sendBuff(7+dwDataLen);
			LPBYTE pbSendBuff = sendBuff;

			memcpy(pbSendBuff, (LPBYTE)"\xE0\x22\x41\xE6\x00\x00\x00\x00\x00\x00\x00", 10); // mse
			UINT32 dwDataOff = 5;
			if (dwDataLen <= 0xFF)
			{
				pbSendBuff[4] = (BYTE)(dwDataLen);	//P3
			}
			else
			{
				Helper::BigEndian::UInt16ToBytes((UINT16)dwDataLen, pbSendBuff+5); //P3
				dwDataOff += 2;
			}
			
			pbSendBuff[dwDataOff] = (BYTE)dwKeyID;
			pbSendBuff[dwDataOff+1] = (BYTE)dwSignFlags;
			if (!(dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID))
			{
				pbSendBuff[dwDataOff+1] |= 0x20; // HashOID
			}
			if (dwSignFlags & AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS)
			{
				pbSendBuff[dwDataOff+2] = 0x80; // 蓝牙通讯地址
			}

			UINT32 dwRemainLen = dwPCDataLen % dwHashBlockSize;
			Helper::BigEndian::UInt32ToBytes(dwPCDataLen-dwRemainLen, pbSendBuff + dwDataOff + 3);
			pbSendBuff[dwDataOff + 3 + 4] = (BYTE)dwRemainLen;
			memcpy(pbSendBuff + dwDataOff + 3 + 4 + 1, pbPCHash, dwPCHashLen);
			memcpy(pbSendBuff + dwDataOff + 3 + 4 + 1 + dwPCHashLen, pbShowData, dwShowDataLen);

			HRESULT hr =  Execute(pbSendBuff, dwDataOff + dwDataLen, 2);
			m_tmpStatus.bResponseIncludeRandom = false;
			return hr;
		}

#if (AKEY_SUPPORT_QUICK_SIGN)
		// 普通签名（快速）
		HRESULT QCommonSign26(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbHashData, UINT32 dwHashDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen)
		{
			CBuffer sendBuff(16 + dwHashDataLen);
			LPBYTE pbSendBuff = sendBuff;
			memcpy(pbSendBuff, "\xE0\x26", 2);
			pbSendBuff[2] = (BYTE)(dwKeyID | 0x20);
			pbSendBuff[3] = (BYTE)dwSignFlags;
			if (!(dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID))
			{
				pbSendBuff[3] |= 0x20; // HashOID
			}

			// 需要验证PIN（必须）
			if ((pbUserPin != NULL) && (dwUserPinLen > 0))
			{
				HRESULT hr = CalcPinMac(pbUserPin, dwUserPinLen, pbSendBuff+5);
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
				memcpy(pbSendBuff+5+4, pbHashData, dwHashDataLen);
				pbSendBuff[2] |= 0x40; // 需要验证PIN
				pbSendBuff[4] = (BYTE)(4 + dwHashDataLen);
			}
			else
			{
				memcpy(pbSendBuff+5, pbHashData, dwHashDataLen);
				pbSendBuff[4] = (BYTE)(dwHashDataLen);
			}

			HRESULT hr = Execute(pbSendBuff, 5 + pbSendBuff[4], 0x80);
			memset(pbSendBuff, 0, 5 + pbSendBuff[4]); // 敏感数据清零

			m_tmpStatus.bResponseIncludeRandom = true;
			if (hr == AKEY_RV_OK)
			{ 
				hr = ParseRespond(pbResult, pdwResultLen);
			}
			return hr;
		}

		// 关键签名（快速）
		HRESULT QCrucialSign26_ShowData(UINT32 dwKeyID, UINT32 dwSignFlags, BYTE abPacketSizeList[4], LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbShowData, UINT32 dwShowDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen)
		{
			HRESULT hr = AKEY_RV_OK;
			// 每包最大值，单包最大值
			UINT32 dwSinglePacketSize = (abPacketSizeList[0] << 8) + abPacketSizeList[1];
			UINT32 dwMaxPacketSize = (abPacketSizeList[2] << 8) + abPacketSizeList[3];
			if (dwSinglePacketSize == 0)
			{
				dwSinglePacketSize = 1024;
			}
			if (dwMaxPacketSize < dwSinglePacketSize)
			{
				dwMaxPacketSize = dwSinglePacketSize;
			}

			CBuffer totalDataBuff(16 + dwShowDataLen);
			LPBYTE pbTotalDataBuff = totalDataBuff.GetBuffer() + 7;
			UINT32 dwTotalDataLen = dwShowDataLen;
			// 需要验证PIN（必须）
			if ((pbUserPin != NULL) && (dwUserPinLen > 0))
			{
				hr = CalcPinMac(pbUserPin, dwUserPinLen, pbTotalDataBuff);
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
				memcpy(pbTotalDataBuff+4, pbShowData, dwShowDataLen);
				dwTotalDataLen += 4;
			}
			else
			{
				memcpy(pbTotalDataBuff, pbShowData, dwShowDataLen);
			}

			BYTE btP1 = (BYTE)dwKeyID;
			BYTE btP2 = (BYTE)dwSignFlags;
			if (dwTotalDataLen > dwShowDataLen)
			{
				btP1 |= 0x40; // 需要验证PIN
			}
			if (!(dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID))
			{
				btP2|= 0x20; // HashOID
			}

			// 只要一包
			if (dwTotalDataLen <= dwMaxPacketSize)
			{
				LPBYTE pbSendBuff;
				UINT32 dwSendLen_t;
				if (dwTotalDataLen < 0x100)
				{
					pbSendBuff = pbTotalDataBuff - 5;
					pbSendBuff[4] = (BYTE)(dwTotalDataLen);
					dwSendLen_t = 5+dwTotalDataLen;
				}
				else
				{
					pbSendBuff = pbTotalDataBuff - 7;
					pbSendBuff[4] = 0x00;
					Helper::BigEndian::UInt16ToBytes(dwTotalDataLen, pbSendBuff+5);
					dwSendLen_t = 7+dwTotalDataLen;
				}
				memcpy(pbSendBuff, "\xE0\x26", 2);
				pbSendBuff[2] = btP1;
				pbSendBuff[3] = btP2;
				hr = Execute(pbSendBuff, dwSendLen_t, 0x80);
			}
			else // 多包
			{
				CBuffer sendBuff(16+dwSinglePacketSize);
				LPBYTE pbSendBuff = sendBuff;
				// sign
				memcpy(pbSendBuff, "\xE0\x26\x00\x00\x00", 5);
				pbSendBuff[2] = btP1 | 0x80;
				pbSendBuff[3] = btP2;

				UINT32 dwCurrLen = dwTotalDataLen % dwSinglePacketSize;
				if (dwCurrLen == 0)
				{
					dwCurrLen = dwSinglePacketSize;
				}
				UINT32 dwSendLen_t = MakeApduPacket(pbTotalDataBuff, dwCurrLen, NULL, pbSendBuff);
				hr = Execute(pbSendBuff, dwSendLen_t);

				for (UINT32 i=dwCurrLen; (hr == AKEY_RV_OK) && (i<dwTotalDataLen); i+=dwSinglePacketSize)
				{
					LPBYTE pbApduHead_t = (i+dwSinglePacketSize < dwTotalDataLen)? (LPBYTE)"\xE0\xC2\x00\x00\x00" : (LPBYTE)"\xE0\xC2\x00\x02\x00";
					UINT32 dwSendLen_t = MakeApduPacket( pbTotalDataBuff+i, dwSinglePacketSize, pbApduHead_t, pbSendBuff);
					hr = Execute(pbSendBuff, dwSendLen_t, 0x80);		
				}
				memset((LPBYTE)sendBuff, 0, sendBuff.GetAllocLength()); // 敏感数据清零
			}
			memset((LPBYTE)totalDataBuff, 0, totalDataBuff.GetAllocLength()); // 敏感数据清零

			m_tmpStatus.bResponseIncludeRandom = true;
			if (hr == AKEY_RV_OK)
			{
				hr = ParseRespond(pbResult, pdwResultLen);
			}
			return hr;
		}
#endif
#if (AKEY_SUPPORT_QUICK_SIGNEX)
		// 快速关键签名（摘要接力）
		HRESULT QCrucialSignEx_All(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbPCHash, UINT32 dwPCHashLen, UINT32 dwPCDataLen, LPBYTE pbShowData, UINT32 dwShowDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen)
		{
			if (/*(pbPCHash == NULL) ||*/ (dwPCHashLen > 0x40) || (pbShowData == NULL) || (dwShowDataLen>2048))
			{
				return AKEY_RV_TOKEN_INVALID_PARAM;
			}

			HRESULT hr = AKEY_RV_OK;
			CBuffer sendBuff(dwPCHashLen + dwShowDataLen + 32); // 分配足够大的空间
			LPBYTE pbSendDataPtr = (LPBYTE)sendBuff + 7;
			LPBYTE ptr = pbSendDataPtr;

			// 数据：KID + HashType + Flag + [PCDataLen + PCHash] + ShowData + [PinMAC] + [BLEADDR] 
			*(ptr++) = (BYTE)dwKeyID;
			*(ptr++) = (dwSignFlags & AKEY_FLAG_SIGN_NOHASHOID)? (BYTE)dwSignFlags : (BYTE)(dwSignFlags | 0x20); // HashOID
			*(ptr++) = (dwSignFlags & AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS)? 0xC0 : 0x40; //b7:表示蓝牙通讯, b6:快速签名
			if (dwPCHashLen || dwPCDataLen) // 是否有PC计算HASH部分
			{
				Helper::BigEndian::UInt32ToBytes(dwPCDataLen, ptr);
				memcpy(ptr + 4, pbPCHash, dwPCHashLen);
				ptr += (4 + dwPCHashLen);
			}
			memcpy(ptr, pbShowData, dwShowDataLen); // 显示数据（可包含6字节的蓝牙地址）
			ptr += dwShowDataLen;

			// 需要验证PIN（必须）
			if ((pbUserPin != NULL) && (dwUserPinLen > 0))
			{
				if (dwSignFlags & AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS) // 蓝牙通讯，输入的最后6字节为蓝牙地址
				{
					ptr -= 6;
				}

				// padding
				memset(ptr,  0x00, 8);
				*ptr = 0x80;
				// calc pin mac
				UINT32 dwPacketLen = (UINT32)(ptr - (pbSendDataPtr+3));
				hr = CalcPinMac(pbUserPin, dwUserPinLen, pbSendDataPtr+3, (dwPacketLen+8)&0xFFF8, ptr);
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
				ptr += 4;

				if (dwSignFlags & AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS) // 蓝牙通讯，输入的最后6字节为蓝牙地址
				{
					memcpy(ptr, pbShowData+dwShowDataLen-6, 6);
					ptr += 6;
				}
				
				pbSendDataPtr[2] |= 0x20; // 指明带口令
			}

			// 计算出有效数据长度，并组APDU
			hr = MakeApduAndExcute((LPBYTE)"\xE0\x22\x41\xE6\x00", pbSendDataPtr, ptr);
			memset((LPBYTE)sendBuff, 0, sendBuff.GetAllocLength()); // 敏感数据清零

			m_tmpStatus.bResponseIncludeRandom = true;
			if (hr == AKEY_RV_OK)
			{
				hr = ParseRespond(pbResult, pdwResultLen);
			}
			return hr;
		}
#endif

		// 取签名结果
		HRESULT GetSignResult(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbPubKey, UINT32 dwPubKeyLen, LPBYTE pbResult, LPUINT32 pdwResultLen)
		{
			if (pbResult == NULL)
			{
				*pdwResultLen = 0x100;
				return AKEY_RV_OK;
			}

			HRESULT hr = AKEY_RV_FAIL;
			if (dwSignFlags & AKEY_FLAG_SIGN_GETRESULT_SCANKEY) // 取签名的结果，同时兼顾扫描按键
			{
				hr = Execute((LPBYTE)"\x80\xC0\x00\x00\x00", 5, 0x100);
				if (hr == AKEY_RV_OK)
				{
					hr = ParseRespond(pbResult, pdwResultLen);
				}
			}
			else
			{
				BYTE abApdu[5] = {0xE0, 0x2A, (((dwSignFlags & AKEY_FLAG_SIGN_TYPE_MASK) == 0)? 0x9E : 0xEE), 0x9A, 0x00};
				//for (int i=0; i<5; i++)
				{
					hr = Execute(abApdu, 5, 0x100, pbResult, pdwResultLen);
					//if ((hr == AKEY_RV_OK) && (dwSignFlags & AKEY_FLAG_SIGN_NOCHECKINTEGRIRY) && (dwPubKeyLen > 0) && ((*pdwResultLen) > 0x80)) // 2048需要验证
					//{
					//	if (OpenAlg::CRsa::PublicCrypt(pbPubKey, dwPubKeyLen, 0, pbResult, *pdwResultLen, m_abRecvBuff) <= 0)
					//	{
					//		hr = AKEY_RV_TOKEN_INVALID_PAIRKEY;
					//		continue;
					//	}
					//}
					//break;
				}
			}
			return hr;			
		}

		// 扫描按键
		HRESULT ScanPressKey()
		{
			HRESULT hr = Execute((LPBYTE)"\xFD\x00\x00\x00\x00", 5);
			return hr;
		}

		// 用户取消
		HRESULT Cancel()
		{
			return Execute((LPBYTE)"\xFD\x09\x1B\x00\x00", 5);
		}


		// 获取证书和其关联的密钥ID列表；不同产品返回格式不一致，只能在TokenStorage层调用
		HRESULT GetCertInfoList_Quick(LPBYTE pbCertInfoList, LPUINT32 pdwLength, UINT32 dwFuncFlags)
		{
			HRESULT hr = Execute((LPBYTE)"\xFC\xB8\x00\x00\x00", 5, 10);
			// oid + kid + usage + type + name(LV)
			if (hr == AKEY_RV_OK)
			{
				UINT32 dwValidDataLen = (dwFuncFlags & AKEY_TOKEN_FUNCTION1_RCI_USAGE)? 3 : 2;
				*pdwLength = (m_dwRecvLen / dwValidDataLen) * 6;
				if (pbCertInfoList != NULL)
				{
					for (UINT32 i=0,j=0; i<m_dwRecvLen; i+=dwValidDataLen,j+=6)
					{
						memcpy(pbCertInfoList+j, m_abRecvBuff+i, dwValidDataLen);
						memset(pbCertInfoList+j+dwValidDataLen, 0, 6-dwValidDataLen);
					}
				}
			}
			return hr;
		}

		// 快读证书数据
		HRESULT GetCertData_Quick(BYTE btObjId, LPBYTE pbCert, LPUINT32 pdwCertLen)
		{
			BYTE abSendBuff[5] = {0xFC, 0xB8, 0x01, btObjId, 0x00};
			HRESULT hr = AKEY_RV_OK;
			for (UINT32 i=0; ;)
			{
				hr = Execute(abSendBuff, 5, 0x400);
				if ((hr != AKEY_RV_OK) && (hr != (AKEY_RV_SW_BASE + 0x6100)))
				{
					return hr;
				}

				if (pbCert != NULL)
				{
					memcpy(pbCert+i,  m_abRecvBuff, m_dwRecvLen);
				}
				i += (m_dwRecvLen);

				if (hr == AKEY_RV_OK)
				{
					*pdwCertLen = i;
					break;
				}
				abSendBuff[2] = 0x02; // 下一包
			}
			return hr;
		}

		// 读取参数
		virtual HRESULT ReadParam(UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen)
		{
			HRESULT hr = AKEY_RV_OK;
			switch(dwParamID)
			{
			case AKEY_PARAM_DEVICE_LABLE: // 卷标
				*pdwResultLen = 32;
				if (pbResult != NULL)
				{
					hr = ReadBin(0xA311, 4, 32, pbResult); // 要去空格？
				}
				break;

			case AKEY_PARAM_CHARSET: // 字符编码
			case AKEY_PARAM_LANGID: // 语言ID
				*pdwResultLen = 1;
				if (pbResult != NULL)
				{
					hr = Execute((LPBYTE)"\xF0\xFD\x00\x00\x02", 5, 0x02);
					if (hr == AKEY_RV_OK)
					{
						pbResult[0] = (dwParamID == AKEY_PARAM_CHARSET)? m_abRecvBuff[1] : m_abRecvBuff[0];
					}
				}
				break;
			case AKEY_PARAM_LANGID_CHARSET: // 语言ID+字符编码
				*pdwResultLen = 2;
				if (pbResult != NULL)
				{
					hr = Execute((LPBYTE)"\xF0\xFD\x00\x00\x02", 5, 0x02, pbResult, pdwResultLen);
				}
				break;

			case AKEY_PARAM_CHARSET_LIST: // 字符编码列表
			case AKEY_PARAM_LANGID_LIST: // 语言ID列表
				*pdwResultLen = 2;
				if (pbResult != NULL)
				{
					hr = Execute((dwParamID==AKEY_PARAM_CHARSET_LIST)? (LPBYTE)"\xF0\xFD\x01\x02\x02" : (LPBYTE)"\xF0\xFD\x01\x01\x02", 5, 0x02, pbResult, pdwResultLen);
				}
				break;

			case AKEY_PARAM_USERPIN_ATTR: // 用户的密码属性
				*pdwResultLen = 16;
				if (pbResult != NULL)
				{
					hr = Execute((LPBYTE)"\x10\xF6\x00\x10\x04", 5, 0x04, pbResult, pdwResultLen);
				}
				break;
				
			case AKEY_PARAM_DEVICE_RAND: // 随机数据，进参：[长度(1B)]
				*pdwResultLen = ((dwDataLen>0)? pbData[0] : 0x08);
				if (pbResult != NULL)
				{
					BYTE abSendBuff[5] = {0x00, 0x84, 0x00, 0x00, (*pdwResultLen)};
					hr = Execute(abSendBuff, 5, abSendBuff[4], pbResult, pdwResultLen);
					GetSessionStatus()->bDeviceRand = (hr == AKEY_RV_OK);
				}
				break;

			case AKEY_PARAM_COSINFO: // COS信息
				*pdwResultLen = 0x80;
				if (pbResult != NULL)
				{
					hr = Execute((LPBYTE)"\xF0\xF6\x00\x00\x00", 5, 0x80, pbResult, pdwResultLen);
				}
				break;

			case AKEY_PARAM_SIGN_PACKET_SZIE: // 获取签名报文的分组包大小+最大包大小
				*pdwResultLen = 0x04;
				if (pbResult != NULL)
				{
					hr = Execute((LPBYTE)"\xE0\xC2\x00\x00\x00", 5, 0x04, pbResult, pdwResultLen);
				}
				break;
			
			case AKEY_PARAM_BITMAP_INFO: // 获取COS信息查询表，进参：FID+BITMAP
				*pdwResultLen = 0x80;
				if (pbResult != NULL)
				{
					BYTE abSendBuff[300] = {0xF0, 0xF8, 0x00, 0x00, (BYTE)dwDataLen};
					memcpy(abSendBuff+5, pbData, dwDataLen);

					hr = Execute(abSendBuff, 5+abSendBuff[4], 0x80, pbResult, pdwResultLen);
				}
				break;

			case AKEY_PARAM_PUBLICKEY: // 公钥（只读），进参：KID
				*pdwResultLen = 0x140;
				if (pbResult != NULL)
				{
					hr = ReadPubKey(pbData[0], pbResult, pdwResultLen);
				}
				break;

			case AKEY_PARAM_PUBLICKEY_N: // 公钥N（只读），进参：KID
				*pdwResultLen = 0x100;
				if (pbResult != NULL)
				{
					BYTE abSendBuff[7] = {0xE0, 0xB4, 0x01, pbData[0], 0x02, 0x20, 0x00};
					hr = Execute(abSendBuff, 7, 0x80, pbResult, pdwResultLen);
				}
				break;

			case AKEY_PARAM_BIRTHCERT: // 出生证（只读）
				*pdwResultLen = 0x800;
				if (pbResult != NULL)
				{
					hr = ReadBirthCert(pbData[0], pbData+1, dwDataLen-1, pbResult, pdwResultLen);
				}
				break;

            case AKEY_PARAM_OTP_STATUS: //OTP状态
                *pdwResultLen = 0x01;
				if (pbResult != NULL)
                {
					hr = Execute((LPBYTE)"\xE0\x24\x00\x00\x00", 5, 0x01, pbResult, pdwResultLen);
                }
				break;

			default:
				hr = AKEY_RV_TOKEN_INVALID_PARAM;
				if (m_pCustomCommand != NULL)
				{
					hr = m_pCustomCommand->OnReadParam(this, dwParamID, pbData, dwDataLen, pbResult, pdwResultLen);
				}

				if (hr == AKEY_RV_TOKEN_INVALID_PARAM)
				{
					hr = m_pSessionObj->GetReader()->ReadParam(dwParamID, pbData, dwDataLen, pbResult, pdwResultLen);
				}
				break;
			}

			return hr;
		}

		// 写入参数
		virtual HRESULT WriteParam(UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen)
		{
			HRESULT hr = AKEY_RV_OK;
			switch(dwParamID)
			{
			case AKEY_PARAM_DEVICE_LABLE: // 卷标
				hr = (dwDataLen >= 32)? UpdateBin(0xA311, 4, 32, pbData) : AKEY_RV_FAIL;
				break;
			case AKEY_PARAM_CHARSET: // 字符编码
				{
					BYTE abSendBuff[] = {0xF0, 0xFE, 0x00, 0x02, 0x01, pbData[0]};
					hr = Execute(abSendBuff, 6);
				}
				break;

			case AKEY_PARAM_LANGID: // 语言ID
				{
					BYTE abSendBuff[] = {0xF0, 0xFE, 0x00, 0x01, 0x01, pbData[0]};
					hr = Execute(abSendBuff, 6);
				}
				break;

			case AKEY_PARAM_SCREEN_ROTATOIN: // 屏幕翻转
				{
					BYTE abSendBuff[] = {0xEB, 0x1E, pbData[0], 0xD0, 0x00}; // P1: 00-每次都翻转，80-翻转到正向，81-翻转到反向
					hr = Execute(abSendBuff, 5);
				}
				break;

			case AKEY_PARAM_DEVICE_RAND: // 随机数据
				{
					m_dwRecvLen = dwDataLen;
					memmove(m_abRecvBuff, pbData, dwDataLen);
					GetSessionStatus()->bDeviceRand = true;
				}
				break;

			case AKEY_PARAM_HIP_VERIFY_PIN: // HIP输入密码
                {
                    BYTE abSendBuff[] = {0xEB, 0x1E, 0x01, 0xE8, 0x00};
                    hr = Execute(abSendBuff, 5);
				}
                break;
 
            case AKEY_PARAM_HIP_CHANGE_PIN: // HIP修改密码
				{
                    BYTE abSendBuff[] = {0xE0, 0x24, 0x01, 0x00, 0x00, 0x00, 0x00};
                    hr = Execute(abSendBuff, 7);
				}
				break;

			default:
				hr = AKEY_RV_TOKEN_INVALID_PARAM;
				break;
			}

			return hr;
		}

	protected:
		// 解析响应（可能保护随机数）
		HRESULT ParseRespond(LPBYTE pbResult, LPUINT32 pdwResultLen)
		{
			if (m_tmpStatus.bResponseIncludeRandom)
			{
				if (m_dwRecvLen < 8) // signRet + rand
				{
					*pdwResultLen = 0;
					return AKEY_RV_FAIL;
				}
				*pdwResultLen = m_dwRecvLen - 8;
				if (pbResult != NULL)
				{
					memcpy(pbResult, m_abRecvBuff, m_dwRecvLen-8);
					WriteParam(AKEY_PARAM_DEVICE_RAND, m_abRecvBuff+m_dwRecvLen-8, 8); // 执行后会影响m_abRecvBuff，必须在最后执行
				}
			}
			else
			{
				if (pbResult != NULL)
				{
					memcpy(pbResult, m_abRecvBuff, m_dwRecvLen);
				}
				*pdwResultLen = m_dwRecvLen;
			}
			return AKEY_RV_OK;
		}

		// RSA加密PIN
		HRESULT RsaEncryptPin(LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbEncResultPtr, LPUINT32 pdwEncResultLen)
		{
			return RsaEncryptPin(pbUserPin, dwUserPinLen, NULL, 0, pbEncResultPtr, pdwEncResultLen);
		}
		HRESULT RsaEncryptPin(LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbNewPin, UINT32 dwNewPinLen, LPBYTE pbEncResultPtr, LPUINT32 pdwEncResultLen)
		{
			if (!(GetSessionStatus()->bDeviceRand && m_dwRecvLen == 8))
			{
				HRESULT hr = Execute((LPBYTE)"\x00\x84\x00\x00\x08", 5, 0x08);  // rand
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
			}

			// 解密出明文PIN
			CBuffer buffPin, buffNewPin;
			if (CMatrixCrypt::Instance()->IsValid())
			{
				dwUserPinLen = AKey::CMatrixCrypt::Instance()->Decrypt(pbUserPin, dwUserPinLen, buffPin.ReAllocBytes(32));
				pbUserPin = buffPin;
				if ((pbNewPin != NULL) && (dwNewPinLen>0))
				{
					dwNewPinLen = AKey::CMatrixCrypt::Instance()->Decrypt(pbNewPin, dwNewPinLen, buffNewPin.ReAllocBytes(32));
					pbNewPin = buffNewPin;
				}
			}

			// 公钥加密
			BYTE abInputData[0x100];
			UINT32 dwInputLen = dwUserPinLen;
			memcpy(abInputData, pbUserPin, dwUserPinLen);
			if ((pbNewPin != NULL) && (dwNewPinLen>0))
			{
				memcpy(abInputData+dwInputLen, "\x00", 1);
				memcpy(abInputData+dwInputLen+1, pbNewPin, dwNewPinLen);
				dwInputLen += (1 + dwNewPinLen);
			}
			memcpy(abInputData+dwInputLen, m_abRecvBuff, 8);
			dwInputLen += 8;
			*pdwEncResultLen = OpenAlg::CRsa::PublicCrypt(GetSessionStatus()->abPubKey1bN, GetSessionStatus()->u32PubKey1bNLen, 0x00010001, OpenAlg::CRsa::ENCRYPT, abInputData, dwInputLen, pbEncResultPtr);
			if (CMatrixCrypt::Instance()->IsValid())
			{
				memset(pbUserPin, 0, dwUserPinLen); // 清除运行时敏感数据
				memset(pbNewPin, 0, dwNewPinLen);
			}
			memset(abInputData, 0, sizeof(abInputData)); // 清除运行时敏感数据
			if ((int)(*pdwEncResultLen) <= 0)
			{
				return AKEY_RV_FAIL;
			}
			return AKEY_RV_OK;
		}

		// 计算PIN-MAC
		HRESULT CalcPinMac(LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbPinMac)
		{
			return CalcPinMac(pbUserPin, dwUserPinLen, NULL, 0, pbPinMac);
		}
		HRESULT CalcPinMac(LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbDataPtr, UINT32 dwDataLen, LPBYTE pbPinMac)
		{
			BYTE abRandBuff[8];
			if (!(GetSessionStatus()->bDeviceRand && m_dwRecvLen == 8)) // 重新取随机数
			{
				HRESULT hr = Execute((LPBYTE)"\x00\x84\x00\x00\x08", 5, 0x08);  // rand
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
			}
			memcpy(abRandBuff, m_abRecvBuff, 8);

			// 解密出明文PIN
			CBuffer buffPin;
			if (CMatrixCrypt::Instance()->IsValid())
			{
				dwUserPinLen = AKey::CMatrixCrypt::Instance()->Decrypt(pbUserPin, dwUserPinLen, buffPin.ReAllocBytes(32));
				pbUserPin = buffPin;
			}

			BYTE abKey[16];
			OpenAlg::CDigest::Digest(Helper::HashType2Name(AKEY_HASH_MD5), pbUserPin, dwUserPinLen, abKey);
			if (CMatrixCrypt::Instance()->IsValid())
			{
				memset(pbUserPin, 0, dwUserPinLen); // 清除运行时敏感数据
			}

			if ((pbDataPtr != NULL) && (dwDataLen > 0))
			{
				OpenAlg::CCipher::Cipher("DES-EDE-CBC", abKey, abRandBuff, OpenAlg::CCipher::ENCRYPT|OpenAlg::CCipher::NOPADDING, pbDataPtr, dwDataLen, m_abRecvBuff);
				memcpy(pbPinMac, m_abRecvBuff+dwDataLen-8, 4);
			}
			else
			{
				OpenAlg::CCipher::Cipher("DES-EDE", abKey, NULL, OpenAlg::CCipher::ENCRYPT|OpenAlg::CCipher::NOPADDING, abRandBuff, 8, m_abRecvBuff);
				memcpy(pbPinMac, m_abRecvBuff, 4);
			}
			memset(abKey, 0, sizeof(abKey)); // 清除运行时敏感数据
			return AKEY_RV_OK;
		}

		// 读取公钥
		HRESULT ReadPubKey(UINT32 dwKeyId, LPBYTE pbPubKey, LPUINT32 pdwPubKeyLen)
		{
			BYTE abSendBuff[7] = {0xE0, 0xB4, 0x01, (BYTE)dwKeyId, 0x02, 0x20, 0x00}; // n; // e(01->02)
			HRESULT hr = Execute(abSendBuff, 7, 0x100);  
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}
			UINT32 nNLen = m_dwRecvLen;
			if (m_dwRecvLen == 0x41) // SM2
			{
				if (pbPubKey != NULL)
				{
					memcpy(pbPubKey, m_abRecvBuff, m_dwRecvLen);
				}
				*pdwPubKeyLen = m_dwRecvLen;
			}
			else
			{
				CAsn1Item der(m_dwRecvLen + 16);
				der.AddData(m_abRecvBuff, m_dwRecvLen);
				der.Make(0x02);
				der.AddData((LPBYTE)"\x02\x03\x01\x00\x01", 5);
				der.Make(0x30, pdwPubKeyLen);
				if (pbPubKey != NULL)
				{
					memcpy(pbPubKey, der.GetBuffer(), der.GetLength());
				}
			}
			//else if (nNLen <= 0x80)
			//{
			//	if (pbPubKey != NULL)
			//	{
			//		memcpy(pbPubKey, "\x30\x81\x89\x02\x81\x81\x00", 7);
			//		pbPubKey[2] = nNLen+9;
			//		pbPubKey[5] = nNLen+1;
			//		memcpy(pbPubKey+7, m_abRecvBuff, nNLen);
			//		memcpy(pbPubKey+7+nNLen, "\x02\x03\x01\x00\x01", 5);
			//	}
			//	*pdwPubKeyLen = 7+nNLen+5;
			//}
			//else
			//{
			//	if (pbPubKey != NULL)
			//	{
			//		memcpy(pbPubKey, "\x30\x82\x01\x0A\x02\x82\x01\x01\x00", 9);
			//		pbPubKey[2] = (BYTE)((nNLen+10) >> 8);
			//		pbPubKey[3] = (BYTE)(nNLen+10);
			//		pbPubKey[6] = (BYTE)((nNLen+1) >> 8);
			//		pbPubKey[7] = (BYTE)(nNLen+1);
			//		memcpy(pbPubKey+9, m_abRecvBuff, nNLen);
			//		memcpy(pbPubKey+9+nNLen, "\x02\x03\x01\x00\x01", 5);
			//	}
			//	*pdwPubKeyLen = 9+nNLen+5;
			//}
			return AKEY_RV_OK;
		}

		// 读出生证
		HRESULT ReadBirthCert(UINT32 dwKeyId, LPBYTE pbSN, UINT32 dwSNLen, LPBYTE pbBirthCert, LPUINT32 pdwBirthCertLen)
		{
			BYTE abRecvBuff[1024];
			HRESULT hr = ReadBin(0xA316, 6, 23, abRecvBuff); // AuthKeyOffset(2) + AuthKeyInfoLength(2) + BirthCertLevel(1) + BirthCertVersion(2) + BirthCertTime(14) + BirthCertSignLength(2)
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}

			UINT32 dwAuthKeyOffset = (abRecvBuff[0] << 8) + abRecvBuff[1];
			UINT32 dwAuthKeyInfoLength = (abRecvBuff[2] << 8) + abRecvBuff[3];
			UINT32 dwBirthCertSignLength = (abRecvBuff[21] << 8) + abRecvBuff[22];

			UINT32 dwPacketOffet = 0;
			dwPacketOffet += MakeTLVPacket(0x0001, pbSN, 2, pbBirthCert+dwPacketOffet); // type
			dwPacketOffet += MakeTLVPacket(0x0002, abRecvBuff+5, 2, pbBirthCert+dwPacketOffet); // Version
			dwPacketOffet += MakeTLVPacket(0x0003, abRecvBuff+7, 14, pbBirthCert+dwPacketOffet); // Time
			dwPacketOffet += MakeTLVPacket(0x0004, pbSN, dwSNLen, pbBirthCert+dwPacketOffet); // SN

			BYTE abSendBuff[7] = {0xE0, 0xB4, 0x01, (BYTE)dwKeyId, 0x02, 0x20, 0x00}; // n; // e(01->02)
			hr = Execute(abSendBuff, 7, 0x100);  
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}
			if (m_dwRecvLen == 0x41) // SM2
			{
				dwPacketOffet += MakeTLVPacket(0x0005, m_abRecvBuff+0x01, 0x20, pbBirthCert+dwPacketOffet); // proPubKeyX
				dwPacketOffet += MakeTLVPacket(0x0006, m_abRecvBuff+0x21, 0x20, pbBirthCert+dwPacketOffet); // proPubKeyY
			}
			else
			{
				dwPacketOffet += MakeTLVPacket(0x0005, m_abRecvBuff, m_dwRecvLen, pbBirthCert+dwPacketOffet); // proPubKeyN

				abSendBuff[2] = 0x02;
				hr = Execute(abSendBuff, 7, 0x100);  
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
				dwPacketOffet += MakeTLVPacket(0x0006, m_abRecvBuff, m_dwRecvLen, pbBirthCert+dwPacketOffet); // proPubKeyE
			}

			hr = ReadBin(0, 6+23, dwBirthCertSignLength, abRecvBuff);
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}
			dwPacketOffet += MakeTLVPacket(0x0007, abRecvBuff, dwBirthCertSignLength, pbBirthCert+dwPacketOffet); // Sign

			if (dwAuthKeyInfoLength > 0)
			{
				hr = ReadBin(0, dwAuthKeyOffset, dwAuthKeyInfoLength, abRecvBuff);
				if (hr != AKEY_RV_OK)
				{
					return hr;
				}
				memcpy(pbBirthCert+dwPacketOffet, abRecvBuff, dwAuthKeyInfoLength); // AuthInfo
				dwPacketOffet += dwAuthKeyInfoLength;
			}

			*pdwBirthCertLen = dwPacketOffet;
			return AKEY_RV_OK;
		}

		UINT32 MakeTLVPacket(UINT32 dwTag, LPBYTE pbData, UINT32 dwLength, LPBYTE pbPacket)
		{
			Helper::BigEndian::UInt16ToBytes(dwTag, pbPacket);
			Helper::BigEndian::UInt16ToBytes(dwLength, pbPacket+2);
			memcpy(pbPacket+4, pbData, dwLength);
			return 4 + dwLength;
		}

		UINT32 MakeApduPacket(LPBYTE pbData, UINT32 dwLength, LPBYTE pbApduHead, LPBYTE pbPacket)
		{
			if (pbApduHead != NULL)
			{
				memcpy(pbPacket, pbApduHead, 4);
			}
			if (dwLength <= 0xFF)
			{
				pbPacket[4] = (BYTE)(dwLength & 0xFF);
				memcpy(pbPacket+5, pbData, dwLength);
				return 5+dwLength;
			}
			else
			{
				pbPacket[4] = 0;
				Helper::BigEndian::UInt16ToBytes((UINT16)dwLength, pbPacket+5);
				memcpy(pbPacket+7, pbData, dwLength);
				return 7+dwLength;
			}
		}

	protected:
		ISession * m_pSessionObj; // 会话类对象实例
		ICustomCommand * m_pCustomCommand; // 制定命令对象
		BYTE m_abRecvBuff[0x1000]; // COS的通讯缓冲区不超过4K
		UINT32 m_dwRecvLen;
		struct _TEMPORARY_STATUS
		{
			UINT32 bResponseIncludeRandom : 1; // bit0, 响应值包含随机数
			UINT32 ruf:31;
		}m_tmpStatus; // 临时状态

		friend class CICBCCommand;
		friend class CABCCommand;
		friend class CCMBCommand;
		friend class CSPDBCommand;
        friend class CIDReaderCommand;
	};
};

// 包含定制对象文件，并根据客户号创建其对象
#include "custom/ICBCCommand.h"
#include "custom/ABCCommand.h"
#include "custom/CMBCommand.h"
#include "custom/SPDBCommand.h"
#include "custom/IDReaderCommand.h"
namespace AKey
{
	HRESULT CCommand::CreateCustomCommand(UINT32 dwCustomId,int deviceType)
	{
		if (m_pCustomCommand != NULL)
		{
			delete m_pCustomCommand;
			m_pCustomCommand = NULL;
		}

		switch(dwCustomId)
		{
		case 0x39333132: // 工商银行(默认)
		case 0x39333436: // 工商银行(SM2默认)
			m_pCustomCommand = new CICBCCommand();
			break;
		case 0x39333231: // 农业银行(默认)
		case 0x39333530: // 农业银行(SM2默认)
			m_pCustomCommand = new CABCCommand();
			break;
		case 0x39333237: // 招商银行(默认)
		case 0x39333437: // 招商银行(二合一)
		case 0x39333434: // 招商银行(企业)
		case 0x39333536: // 招商银行(通用key)
			m_pCustomCommand = new CCMBCommand();
			break;
		case 0x39333239: // 上海浦发(二代默认/三代默认)
		case 0x39333433: // 上海浦发(柜员项目)
		case 0x39333432: // 上海浦发(浦发读卡器)
			m_pCustomCommand = new CSPDBCommand();
			break;
        case 0x39393035: //身份证阅读器
            m_pCustomCommand = new CIDReaderCommand(deviceType);
            break;
                
		default:
			break;		
		};
		return AKEY_RV_OK;
	}
}

#endif
