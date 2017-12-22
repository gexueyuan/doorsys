#ifndef __AKEY_KCARDCOMM_H__
#define __AKEY_KCARDCOMM_H__

#include "AKeyDef.h"
#include "AKeyInterface.h"
#include "AKeyHelper.h"

#define BOTREADER_FLAG_CLEARNO_BEFORE_TRANSMIT		0x01 // 每次执行指令前序号清零
#define BOTREADER_FALG_RAWCOMM_TEST					0x80 //  T1裸通讯测试

namespace AKey
{
	// class CKCardComm
	class CKCardComm : public IReader
	{
	public:
		typedef struct _CONFIG
		{
			BYTE btFlags;
			BYTE btBlockSize;
			BYTE btRetryTimes;
			BYTE btRFU;
			BYTE btSendNo;
			BYTE btRecvNo;
		}CONFIG, *CONFIG_PTR;

	public:
		CKCardComm() : m_pSubProtocol(NULL)
		{			
			m_config.btFlags = 0;
			m_config.btBlockSize = 0x80;
			m_config.btRetryTimes = 3;
			m_config.btSendNo = 0;
			m_config.btRecvNo = 0;
		}
		virtual ~CKCardComm()
		{
		}

		void SetConfig(CONFIG_PTR pConfig)
		{
			m_config.btFlags = pConfig->btFlags;
			if (pConfig->btBlockSize != 0)
				m_config.btBlockSize = pConfig->btBlockSize;
			if (pConfig->btRetryTimes != 0)
				m_config.btRetryTimes = pConfig->btRetryTimes;
		}
		const CONFIG_PTR GetConfig()
		{
			return &m_config;
		}

		virtual HRESULT Init(IProtocol * pSubProtocol)
		{
			m_pSubProtocol = pSubProtocol;
			LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "KCardComm Init() = %08X\r\n", AKEY_RV_OK);
			return AKEY_RV_OK;
		}
		
		virtual HRESULT Final()
		{
			return AKEY_RV_OK;
		}
		// IReader
		// 检测设备是否存在
		virtual HRESULT CheckDevice(bool * pbNewDevice = NULL)
		{
			CONFIG_PTR pCommParam = &m_config;

			HRESULT hr = AKEY_RV_OK;
			{
				// 序号清零
				pCommParam->btSendNo = 0;
				pCommParam->btRecvNo = 0;
				hr = Transmit_T1_SBlock(0xC0, (LPBYTE)"",  0, NULL, NULL);

				// 设置块大小
				if (hr == AKEY_RV_OK)
				{
					hr = Transmit_T1_SBlock(0xC1, &(pCommParam->btBlockSize), 1, NULL, NULL);
				}

				//BYTE abRecvBuff[300];
				//UINT32 unRecvLen = sizeof(abRecvBuff);
				//hr = Transmit_T1_IBlock((LPBYTE)"\x00\xA4\x00\x00\x02\xA3\x12", 7, abRecvBuff, &unRecvLen);
				//if ((hr == AKEY_RV_OK) && (abRecvBuff[0] != 0x90) && (pbNewDevice != NULL))
				//{
				//	*pbNewDevice = true;
				//}		
			}
			return hr;
		}

		// IReader
		// 执行指令
		virtual HRESULT Control(UINT32 unCmd, LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen, UINT32 unTimeout)
		{
			CONFIG_PTR pCommParam = &m_config;

			// 清除序号
			if (pCommParam->btFlags & BOTREADER_FLAG_CLEARNO_BEFORE_TRANSMIT)
			{
				HRESULT hr = Transmit_T1_SBlock(0xC0, (LPBYTE)"",  0, NULL, NULL);
				if (hr != AKEY_RV_OK)
					return hr;
			}

			return Transmit_T1_IBlock(pbSend,  unSLen, pbRecv, punRLen);
		}

		// IReader
		virtual HRESULT ReadParam(UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen)
		{
			HRESULT hr = AKEY_RV_OK;
			switch(dwParamID)
			{
			case AKEY_PARAM_COMM_PROTOCOL:
			{
				*pdwResultLen = 1;
				if (pbData != NULL)
				{
					pbResult[0] = AKEY_COMM_PROTOCOL_KCARD;
				}
				break;
			}
			case AKEY_PARAM_COMM_BATTERY_LEVEL: // 电量1（实时）
				*pdwResultLen = 1;
				if (pbResult != NULL)
				{
					BYTE abRecvBuff[32];
					UINT32 dwRecvLen = sizeof(abRecvBuff);
					hr = Transmit_T1_SBlock(0xC6, (LPBYTE)"",  0, abRecvBuff, &dwRecvLen);
					if (hr == AKEY_RV_OK)
					{
						pbResult[0] = abRecvBuff[0];
					}
				}
				break;				
			default:
				hr = AKEY_RV_TOKEN_INVALID_PARAM;
				break;
			}

			return hr;
		}

		// IProtocol
		virtual IProtocol * GetSubProtocol()
		{
			return NULL;
		}

#ifdef AKEY_COMM_DEBUG
#endif

	protected:
		static BYTE X_XOR(LPBYTE pbData, UINT32 dwLen)
		{
			BYTE bt = 0;
			for (UINT32 i=0; i<dwLen; i++)
			{
				bt ^= pbData[i];
			}
			return bt;
		}

		HRESULT Transmit_T1_Raw(LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "SendAndRecvFrame");
			LGNTRACE_HEX("KCardComm send data : ", pbSend, unSLen);

			HRESULT hr = SendAndRecvFrame(pbSend, unSLen, pbRecv, punRLen);

			if (hr == AKEY_RV_OK)
			{
				LGNTRACE_HEX("KCardComm recv data : ", pbRecv, *punRLen);
			}
			
			LGNTRACE_ERRORNO(hr);
			return hr;
		}

		HRESULT Transmit_T1_SBlock(BYTE btPCB, LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			BYTE abSendBuff[300], abRecvBuff[300];
			UINT32 unRecvLen = 0;
			CONFIG_PTR pCommParam = &m_config;

			HRESULT hr = AKEY_RV_OK;

			abSendBuff[0] = 0; // NAD
			abSendBuff[1] = btPCB; // PCB
			abSendBuff[2] = (BYTE)unSLen;
			memcpy(abSendBuff+3, pbSend, unSLen);
			abSendBuff[3+unSLen] = X_XOR(abSendBuff, 3+unSLen);
			for (int nTryTime=0; nTryTime<pCommParam->btRetryTimes; )
			{
				unRecvLen = sizeof(abRecvBuff);
				hr = Transmit_T1_Raw(abSendBuff, 4+unSLen, abRecvBuff, &unRecvLen);
				if (hr != AKEY_RV_OK)
				{
					LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_SBlock::Transmit_T1_Raw=%X\r\n", hr);
					if ((pCommParam->btFlags & BOTREADER_FALG_RAWCOMM_TEST) || (hr != AKEY_RV_SW_READER_TIMEOUT)) // 超时错误
						return hr;

					nTryTime ++;
					continue; // 重发
				}

				if (X_XOR(abRecvBuff, unRecvLen) != 0)
				{
					LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_SBlock Error: R-XOR!\r\n");
					if (pCommParam->btFlags & BOTREADER_FALG_RAWCOMM_TEST)
						return AKEY_RV_COMM_INCORRECT_CRC;

					nTryTime ++;
					continue; // 重发
				}
				nTryTime = 0;

				if ( ((abRecvBuff[1] & 0x20) == 0x20) && ((abRecvBuff[1] & 0xDF) == abSendBuff[1]) ) // R-SBlock
				{
					if (unRecvLen>3 && pbRecv && punRLen)
					{
						memcpy(pbRecv, abRecvBuff+3, abRecvBuff[2]);
						*punRLen = abRecvBuff[2];
					}
					return AKEY_RV_OK;
				}
				else
				{
					LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_SBlock Error: R-SBlock!\r\n");
					if (pCommParam->btFlags & BOTREADER_FALG_RAWCOMM_TEST)
						return AKEY_RV_COMM_INCORRECT_DSN;

					continue; // 重发
				}
			} // for end

			return AKEY_RV_FAIL;
		}

		HRESULT Transmit_T1_IBlock(LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			BYTE abSendBuff[300], abRecvBuff[300];
			UINT32 unCurrSendLen = 0, unRecvLen = 0, unRetryTimes = 0;
			CONFIG_PTR pCommParam = &m_config;

			HRESULT hr = AKEY_RV_OK;
			UINT32 unOperType = 1; // 1-发送I包，2-发送R包，3-发送响应（WTX），4-接收I包，5-发送响应（E5）
			*punRLen = 0;
			for (int nTryTime=0; nTryTime<pCommParam->btRetryTimes; ) // 发送过程
			{
				if (unOperType == 1) // 发送I包
				{
					abSendBuff[0] = 0; // NAD
					abSendBuff[1] = (pCommParam->btSendNo & 0x01)? 0x40 : 0x00; // PCB
					if (unSLen > pCommParam->btBlockSize)
					{
						abSendBuff[1] |= 0x20; // 连续
						unCurrSendLen = pCommParam->btBlockSize;
					}
					else
					{
						abSendBuff[1] &= (~0x20); // 结束
						unCurrSendLen = (unSLen);
					}
					abSendBuff[2] = (BYTE)unCurrSendLen;
					memcpy(abSendBuff+3, pbSend, unCurrSendLen);
					abSendBuff[3+unCurrSendLen] = X_XOR(abSendBuff, 3+unCurrSendLen);
				}
				else if (unOperType == 2) // 发送R包（NAK）
				{
					abSendBuff[0] = 0; // NAD
					abSendBuff[1] = 0x80 + ((pCommParam->btRecvNo & 0x01)? 0x10 : 0x00);
					abSendBuff[2] = 0;
					abSendBuff[3] = abSendBuff[1];
				}
				else if (unOperType == 3) // WTX
				{
					abSendBuff[0] = 0; // NAD
					abSendBuff[1] = 0xE3; // abRecvBuff[1] | 0x20; // 0xC3 | 0x20
					abSendBuff[2] = 1;
					abSendBuff[3] = 0x01; //abRecvBuff[3];
					abSendBuff[4] = X_XOR(abSendBuff, 4);
				}
				else if (unOperType == 4) // 接收I包
				{
					if ( (abRecvBuff[1] & 0x40) != ((pCommParam->btRecvNo & 0x01)? 0x40 : 0x00) ) // 接收序号错误
					{
						//// 序号清零
						//hr = Transmit_T1_SBlock(pContext, 0xC0, (LPBYTE)"",  0, NULL, NULL))
						//if (hr != AKEY_RV_OK)
						//	return hr;
						//pCommParam->btSendNo = 0;
						//pCommParam->btRecvNo = 0;
						LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_IBlock Error: RecvNo!\r\n");
						pCommParam->btRecvNo ++; // 恢复序号后继续（COS-BUG）
						//return AKEY_RV_COMM_INCORRECT_HSN;
					}
					//else
					{
						memcpy(pbRecv, abRecvBuff+3, abRecvBuff[2]);
						pbRecv += abRecvBuff[2];
						*punRLen += abRecvBuff[2];

						pCommParam->btRecvNo ++;
						if ((abRecvBuff[1] & 0x20) == 0) // 不连续（结束）
						{
							return AKEY_RV_OK;
						}
					}

					// ACK
					abSendBuff[0] = 0;
					abSendBuff[1] = 0x80 + ((pCommParam->btRecvNo & 0x01)? 0x10 : 0x00); // PCB
					abSendBuff[2] = 0;
					abSendBuff[3] = abSendBuff[1]; // XOR
					unRetryTimes = 0;
				}
				else if (unOperType == 5) // 发送响应（E5）
				{
					abSendBuff[0] = 0; // NAD
					abSendBuff[1] = 0xE5;
					//abSendBuff[2] = 1;
					//abSendBuff[3] = 0x01;
					abSendBuff[3+abSendBuff[2]] = X_XOR(abSendBuff, 3+abSendBuff[2]);
				}

				// 执行单指令
				unRecvLen = sizeof(abRecvBuff);
				hr = Transmit_T1_Raw(abSendBuff, 4+abSendBuff[2], abRecvBuff, &unRecvLen);
				if (hr != AKEY_RV_OK)
				{
					LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_IBlock Error: Transmit_T1_Raw=%X!\r\n", hr);
					if ((pCommParam->btFlags & BOTREADER_FALG_RAWCOMM_TEST) || (hr != AKEY_RV_SW_READER_TIMEOUT)) // 超时错误
						return hr;

					unOperType = 2; // 发送R包（发送超时）
					nTryTime ++;
					continue; // 重发
				}
				if (X_XOR(abRecvBuff, unRecvLen) != 0) // 校验位错误，发送R包
				{
					LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_IBlock Error: R-XOR!\r\n");
					if (pCommParam->btFlags & BOTREADER_FALG_RAWCOMM_TEST)
						return AKEY_RV_COMM_INCORRECT_CRC;

					unOperType = 2; // 发送R包（校验错误）
					nTryTime ++;
					continue; // 重发
				}

				if ((abRecvBuff[1] & 0xC0) == 0xC0) // S: WTX
				{
					if (abRecvBuff[1] == 0xC3)
					{
						unOperType = 3; // 发送响应（WTX）
					}
					else if (abRecvBuff[1] == 0xC5) // 有数据交换且过程维护包（E3）的SBLOCK
					{
						BYTE abSBlockE3[5] = {0x00, 0xE3, 0x01, 0x01, 0x00};
						abSBlockE3[4] = X_XOR(abSBlockE3, 4);

						UINT32 unExchangeResultLen = 0;
						hr = SBlockDataExchange(abSBlockE3, 5, abRecvBuff+3, abRecvBuff[2], abSendBuff+3, &unExchangeResultLen);
						if (hr != AKEY_RV_OK)
						{
							return hr; // 用户取消
						}
						abSendBuff[2] = (BYTE)unExchangeResultLen;
						unOperType = 5; // 发送响应（E5）
					}
					else
					{
						LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_IBlock Error: Unknown SBlock(%02X)!\r\n", abRecvBuff[1]);
						return  AKEY_RV_COMM_INCORRECT_DATA;
					}

					// 发送后收到WTX说明发送成功，需要增加计数器
					if (unSLen > 0)
					{
						pCommParam->btSendNo ++;
						unSLen = 0;
					}

					nTryTime = 0;
					continue;
				}
				else if ((abRecvBuff[1] & 0xC0) == 0x80) // R:
				{
					if (abRecvBuff[1] & 0x03) //  NAK
					{
						LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_IBlock Error: NAK!\r\n");
						if (pCommParam->btFlags & BOTREADER_FALG_RAWCOMM_TEST) // T1裸通讯测试
							return  AKEY_RV_COMM_INCORRECT_DATA;

						unOperType = 0;
						nTryTime ++;
						continue; // 重发
					}

					if ( (abRecvBuff[1] & 0x10) != ((pCommParam->btSendNo & 0x01)? 0x00 : 0x10) ) // 发送序号错误
					{
						LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_ERROR_LEVEL, "Transmit_T1_IBlock Error: SendNo!\r\n");
						if (pCommParam->btFlags & BOTREADER_FALG_RAWCOMM_TEST) // T1裸通讯测试
							return  AKEY_RV_COMM_INCORRECT_DATA;

						unOperType = (unSLen > 0)? 1 : 2; // 发送I包（处于发送过程），发送R包（处于接收过程）
						if (unOperType == 2)
						{
							if ((++unRetryTimes) >= (pCommParam->btRetryTimes))
								return AKEY_RV_COMM_TIMEOUT;
						}
						continue; // 重发
					}					

					if (unSLen > 0)
					{
						pCommParam->btSendNo ++;
						pbSend += unCurrSendLen;
						unSLen -= unCurrSendLen;
					}
					unOperType = (unSLen > 0)? 1 : 2; // 发送I包（处于发送过程），发送R包（处于接收过程）
					nTryTime = 0;
					continue;
				}
				else //if ((abRecvBuff[1] & 0x80) == 0) // I:
				{
					if (abRecvBuff[2] == 0) // 长度为零当R包处理
					{
						pCommParam->btRecvNo ++;
						unOperType = (unSLen > 0)? 1 : 2; // 发送I包（处于发送过程），发送R包（处于接收过程）
						nTryTime = 0;
						continue; // 重发
					}

					if (unSLen > 0) // 第一个接收I包
					{
						pCommParam->btSendNo ++; // 如果序号不对在接收过程会重置序号
						pbSend += unCurrSendLen;
						unSLen -= unCurrSendLen;
					}
					unOperType = 4; // 接收I包
					nTryTime = 0;
					continue;
				}
			}

			return AKEY_RV_FAIL;
		}

	protected:
		// 继承类必须实现以下接口
		virtual HRESULT SendAndRecvFrame(LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen)
		{
			HRESULT hr = SendFrameData(pbSend, unSLen);
			if (hr == AKEY_RV_OK)
			{
				UINT32 unRLen_back = *punRLen;
				hr = RecvFrameData(pbRecv, punRLen);
				while ((hr == AKEY_RV_OK) && ((*punRLen) < (UINT32)(pbRecv[2]+4)))
				{
					UINT32 unRLen_t = unRLen_back - (*punRLen);
					hr = RecvFrameData(pbRecv+(*punRLen), &unRLen_t);
					if (hr == AKEY_RV_OK)
					{
						*punRLen += unRLen_t;
					}
				}
			}
			return hr;
		}

		virtual HRESULT SendFrameData(LPBYTE pbSend, UINT32 unSLen)
		{
			if (m_pSubProtocol != NULL)
			{
				return m_pSubProtocol->Control(AKEY_PROTOCOL_COMMAND_BULKWRITE, pbSend, unSLen, NULL, NULL, 1000);
			}
			return AKEY_RV_FAIL;
		}

		virtual HRESULT RecvFrameData(LPBYTE pbRecv, LPUINT32 punRLen)
		{
			if (m_pSubProtocol != NULL)
			{
				return m_pSubProtocol->Control(AKEY_PROTOCOL_COMMAND_BULKREAD, NULL, 0, pbRecv, punRLen, 5000);
			}
			return AKEY_RV_FAIL;
		}

		// pbHeartbeat: 心跳包，每隔500ms发送一次
		virtual HRESULT SBlockDataExchange(LPBYTE pbHeartbeat, UINT32 unHeartbeatLen, LPBYTE pbData, UINT32 unDataLen, LPBYTE pbResult, LPUINT32 punResultLen)
		{
			if (m_pSubProtocol != NULL)
			{
				CBuffer buff;
				LPBYTE pbSendPtr = buff.ReAllocBytes(unHeartbeatLen+unDataLen);
				memcpy(pbSendPtr, pbHeartbeat, unHeartbeatLen);
				memcpy(pbSendPtr+unHeartbeatLen, pbData, unDataLen);
				return m_pSubProtocol->Control(AKEY_PROTOCOL_COMMAND_DATAEXCHANGE, pbSendPtr, unHeartbeatLen+unDataLen, pbResult, punResultLen, 5000);
			}
			return AKEY_RV_FAIL;
		}

	protected:
		IProtocol * m_pSubProtocol;
		CONFIG m_config;
#ifdef AKEY_COMM_DEBUG
#endif
	};
};

#endif
