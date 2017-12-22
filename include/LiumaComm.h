#ifndef __AKEY_LIUMACOMM_H__
#define __AKEY_LIUMACOMM_H__

#include "AKeyDef.h"
#include "AKeyInterface.h"
#include "AKeyHelper.h"

namespace AKey
{
	// class CLiumaComm
	class CLiumaComm : public IReader
	{
	public:
		CLiumaComm() : m_pSubProtocol(NULL)
		{
			_maxTryTimes = 20;
			_liumaToggle = 0;
		}
		virtual ~CLiumaComm()
		{
		}

		virtual HRESULT Init(IProtocol * pSubProtocol)
		{
			m_pSubProtocol = pSubProtocol;
			LGNTRACE(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "LiumaComm Init() = %08X\r\n", AKEY_RV_OK);
			return AKEY_RV_OK;
		}
		
		// IReader2 
		virtual HRESULT Final()
		{
			return AKEY_RV_OK;
		}

		virtual HRESULT Cancel()
		{
			return AKEY_RV_OK;
		}


		// IReader
		// 检测设备是否存在
		virtual HRESULT CheckDevice(bool * pbNewDevice = NULL)
		{
			HRESULT hr = AKEY_RV_OK;
			return hr;
		}

		// IReader
		// 执行指令
		virtual HRESULT Control(UINT32 unCmd, LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen, UINT32 unTimeout)
		{
			unTimeout;
			LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "Transmit(Liuma)");
			LGNTRACE_HEX("Transmit(LIUMA) Send:", pbSend, unSLen);
			HRESULT hr =  LIUMA_Transmit(pbSend, unSLen, pbRecv, punRLen, _maxTryTimes);
			if (hr != AKEY_RV_OK)
			{
				LGNTRACE_ERRORNO(hr);
				return hr;
			}
			LGNTRACE_HEX("Transmit(LIUMA) Recv:", pbRecv, *punRLen);
			return hr;
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
					pbResult[0] = AKEY_COMM_PROTOCOL_LIUMA;
				}
				break;
			}
			default:
				hr = AKEY_RV_TOKEN_INVALID_PARAM;
				break;
			}

			return hr;
		}

		// IProtocol
		virtual IProtocol * GetSubProtocol()
		{
			return m_pSubProtocol;
		}

	protected:
		IProtocol * m_pSubProtocol;
		UINT32 _maxTryTimes;
		BYTE _liumaToggle;

	protected:
#define LIUMA_STATIC

/****************************************************************
*					刘码公共代码-begin							*
****************************************************************/
typedef unsigned char uchar;
typedef UINT32 uint;

#ifndef xMemset
#define xMemset memset
#define xxMemcpy memcpy
#endif

#ifndef LIUMA_RV_OK
#define LIUMA_RV_OK					AKEY_RV_OK
#define LIUMA_RV_TIMEOUT			AKEY_RV_COMM_TIMEOUT
#define LIUMA_RV_OUTOFBUFFER		AKEY_RV_OUTOFBUFFER
#define LIUMA_RV_BUFFERTOOSMALL		AKEY_RV_BUFFERTOOSMALL
#define	LIUMA_RV_WRONG_STAT			AKEY_RV_COMM_WRONG_STAT		
#define	LIUMA_RV_WRONG_IBLOCK		AKEY_RV_COMM_WRONG_IBLOCK
#define	LIUMA_RV_COMM_INTERRUPT		AKEY_RV_APP_COMMUNICATION_INTERRUPT
#endif

#ifndef LIUMA_BLOCKSIZE
#define LIUMA_BLOCKSIZE				20 /*数据块大小: 16-两线，20-蓝牙*/
#define LIUMA_VALID_BLOCKSIZE		(LIUMA_BLOCKSIZE-2) /*一个块里的有效数据大小*/

#define LIUMA_MAX_BLOCKFLAGS_SIZE	15 /*最大块标记大小*/
#define LIUMA_MAX_BLOCKID			114 /*最大块号=LIUMA_MAX_BLOCKFLAGS_SIZE*8*/
#define LIUMA_MAX_BUFFERSIZE		2048 /*最大缓冲区大小=LIUMA_MAX_BLOCKID*18*/
#endif

typedef HRESULT LIUMA_RESULT;		/*返回值类型*/
/****************************************************************
*					刘码公共代码-end							*
****************************************************************/



/****************************************************************
*					刘码公共代码-begin							*
*					v1.0.10 at 2016-04-07						*
****************************************************************/
#define LIUMA_TOGGLE_INC()		(_liumaToggle = _liumaToggle? 0x00: 0x20)
#define LIUMA_TOGGLE_CMP(tog)	(((tog) & 0x20) == _liumaToggle)
#define LIUMA_PCB_IBLOCK		(_liumaToggle)
#define LIUMA_PCB_RBLOCK		(_liumaToggle | 0x80)
#define LIUMA_PCB_SBLOCK		(_liumaToggle | 0x40)
#define LIUMA_PCB_DLEN_FLAG		0x01
#define LIUMA_PCB_RLEN_FLAG		0x02
#define LIUMA_CHAIN_LASTBLOCK	0x80
#define LIUMA_CHAIN_NUM(n)		((n)&0x7F)

#ifndef LIUMA_STATIC
#define LIUMA_STATIC static
#endif


void LIUMA_InitContext(void)
{
	_liumaToggle = 0;
}

/*组块（LastBlock）*/
static void LIUMA_MakeBlockData(uchar * data, uint dataLen, uchar * blockBuffer, uint validBufferOffset)
{
	uint validBufferSize = LIUMA_BLOCKSIZE-validBufferOffset;
	uchar *validData = blockBuffer + validBufferOffset;
	if (dataLen == validBufferSize)
	{
		xxMemcpy(validData, data, dataLen);
	}
	else
	{
		xMemset(validData, 0, validBufferSize);
#if (LIUMA_BLOCKSIZE <= 0x80)
		blockBuffer[0] |= LIUMA_PCB_DLEN_FLAG;
		*validData++ = (uchar)dataLen;
#else
		uint vlen = 0;
		if (dataLen < (validBufferSize - 3))
		{
			blockBuffer[0] |= LIUMA_PCB_DLEN_FLAG;
			vlen = dataLen;
		}
		else
		{
			blockBuffer[0] |= LIUMA_PCB_RLEN_FLAG;
			vlen = validBufferSize - dataLen;
		}
		
		if (vlen < 0x80)
		{
			*validData++ = (uchar)vlen;
		}
		else if (vlen < 0x100)
		{
			*validData++ = 0x81;
			*validData++ = (uchar)vlen;
		}
		else
		{
			*validData++ = 0x82;
			*validData++ = (uchar)(vlen >> 8);
			*validData++ = (uchar)vlen;
		}
#endif
		xxMemcpy(validData, data, dataLen);
	}
}
/*解块（LastBlock）*/
static void LIUMA_ParseBlockData(uchar * blockBuffer, uint validBufferOffset, uint * validDataOffset, uint * validDataLength)
{
	if ((blockBuffer[0] & 0x03) == 0)
	{
		*validDataOffset = validBufferOffset;
		*validDataLength = LIUMA_BLOCKSIZE - validBufferOffset;
	}
	else
	{
#if (LIUMA_BLOCKSIZE <= 0x80)
		*validDataOffset = validBufferOffset+1;
		*validDataLength = blockBuffer[validBufferOffset];
#else
		if ((blockBuffer[validBufferOffset] & 0x80) == 0)
		{
			*validDataOffset = validBufferOffset+1;
			*validDataLength = blockBuffer[validBufferOffset];
		}
		else if (blockBuffer[validBufferOffset] == 0x81)
		{
			*validDataOffset = validBufferOffset+2;
			*validDataLength = blockBuffer[validBufferOffset+1];
		}
		else
		{
			*validDataOffset = validBufferOffset+3;
			*validDataLength = (blockBuffer[validBufferOffset+1] << 8) + blockBuffer[validBufferOffset+2];
		}

		if ((validDataOffset[0] & 0x03) == LIUMA_PCB_RLEN_FLAG)
		{
			*validDataLength = (LIUMA_BLOCKSIZE - validBufferOffset) - (*validDataLength);
		}
#endif
	}
}

#define LIUMA_MakeIBlockData(data, dataLen, blockBuffer) LIUMA_MakeBlockData(data, dataLen, blockBuffer, 2)
#define LIUMA_MakeRBlockData(data, dataLen, blockBuffer) LIUMA_MakeBlockData(data, dataLen, blockBuffer, 1)
#define LIUMA_ParseIBlockData(blockBuffer, validDataOffset, validDataLength) LIUMA_ParseBlockData(blockBuffer, 2, validDataOffset, validDataLength);
#define LIUMA_ParseRBlockData(blockBuffer, validDataOffset, validDataLength) LIUMA_ParseBlockData(blockBuffer, 1, validDataOffset, validDataLength);


/*发送I包（多块）*/
LIUMA_STATIC LIUMA_RESULT LIUMA_SendIBlocks(uchar * indata, uint dataLen)
{
	uint i, dataOffset, tmpCount;
	uchar ucBlockBuffer[LIUMA_BLOCKSIZE+1]; /*多分配1字节用于底层通讯*/

	dataOffset = 0;
	tmpCount = ((dataLen-1) / LIUMA_VALID_BLOCKSIZE);
	ucBlockBuffer[0] = LIUMA_PCB_IBLOCK;
	for (i=0; i<tmpCount; i++)
	{
		ucBlockBuffer[1] = (uchar)(i+1);
		xxMemcpy(ucBlockBuffer+2, indata+dataOffset, LIUMA_VALID_BLOCKSIZE);
		if (LIUMA_SendRawDataBlock(ucBlockBuffer) == LIUMA_RV_COMM_INTERRUPT)
		{
			return LIUMA_RV_COMM_INTERRUPT; /*只有通信中断才退出*/
		}
		
		dataOffset += LIUMA_VALID_BLOCKSIZE;
	}

	/*最后一块*/
	ucBlockBuffer[1] = (uchar)(tmpCount+1) | LIUMA_CHAIN_LASTBLOCK;
	LIUMA_MakeIBlockData(indata+dataOffset, dataLen-dataOffset, ucBlockBuffer);
	return LIUMA_SendRawDataBlock(ucBlockBuffer); 
}
/*重发送I包（多块）*/
LIUMA_STATIC LIUMA_RESULT LIUMA_ReSendIBlocks(uchar *sendFlags, uchar * indata, uint dataLen)
{
	uint i, tmpCount;
	uchar ucBlockBuffer[LIUMA_BLOCKSIZE+1]; /*多分配1字节用于底层通讯*/
		
	tmpCount = ((dataLen-1) / LIUMA_VALID_BLOCKSIZE);
	ucBlockBuffer[0] = LIUMA_PCB_IBLOCK;
	ucBlockBuffer[1] = 0;
	for (i=0; i<tmpCount; i++)
	{
		if (!(sendFlags[(i>>3)] & (1 << (i&0x07)))) /*未发送状态*/
		{
			if (ucBlockBuffer[1]) /*发送上个块*/
			{
				if (LIUMA_SendRawDataBlock(ucBlockBuffer) == LIUMA_RV_COMM_INTERRUPT)
				{
					return LIUMA_RV_COMM_INTERRUPT; /*只有通信中断才退出*/
				}
			}
			ucBlockBuffer[1] = (uchar)(i+1);
			xxMemcpy(ucBlockBuffer+2, indata+(i*LIUMA_VALID_BLOCKSIZE), LIUMA_VALID_BLOCKSIZE);
		}
	}

	/*数据的最后一块是否发送*/
	if (!(sendFlags[(tmpCount>>3)] & (1 << (tmpCount&0x07)))) /*未发送状态*/
	{
		if (ucBlockBuffer[1]) /*发送上个块*/
		{
			if (LIUMA_SendRawDataBlock(ucBlockBuffer) == LIUMA_RV_COMM_INTERRUPT)
			{
				return LIUMA_RV_COMM_INTERRUPT; /*只有通信中断才退出*/
			}
		}
		ucBlockBuffer[1] = (uchar)(tmpCount+1);
		LIUMA_MakeIBlockData(indata + (tmpCount*LIUMA_VALID_BLOCKSIZE), dataLen - (tmpCount * LIUMA_VALID_BLOCKSIZE), ucBlockBuffer);
	}

	if (ucBlockBuffer[1]) /*发送最后一块*/
	{
		ucBlockBuffer[1] |= LIUMA_CHAIN_LASTBLOCK; /*标记为最后发送一块*/
		if (LIUMA_SendRawDataBlock(ucBlockBuffer) == LIUMA_RV_COMM_INTERRUPT)
		{
			return LIUMA_RV_COMM_INTERRUPT; /*只有通信中断才退出*/
		}
	}
	return LIUMA_RV_OK;
}

/*保持接收到的I包*/
LIUMA_STATIC LIUMA_RESULT LIUMA_SaveIBlock(uchar *ucBlockBuffer, uchar *recvFlags, uint * receivedMaxBlockCount,  uint * recvBlockCount, uchar * outdata, uint * outdataLen)
{
	uint validOffset, validLen;
	uchar blockIdx = LIUMA_CHAIN_NUM(ucBlockBuffer[1]) - 1; /*包号*/
	uchar lastBlock = ucBlockBuffer[1] & LIUMA_CHAIN_LASTBLOCK;

	LIUMA_ParseIBlockData(ucBlockBuffer, &validOffset, &validLen);
	/*接收缓冲不足*/
	if (*outdataLen < (blockIdx*LIUMA_VALID_BLOCKSIZE + validLen))
	{
		return LIUMA_RV_BUFFERTOOSMALL;
	}
	/*有效包号，有效长度*/
	if ((blockIdx < LIUMA_MAX_BLOCKID) && ((!lastBlock && (validLen==LIUMA_VALID_BLOCKSIZE)) || (lastBlock && (validLen<=LIUMA_VALID_BLOCKSIZE))))
	{
		uchar idx = blockIdx>>3;
		uchar bit = (1 << (blockIdx&0x07));
		if (!(recvFlags[idx] & bit))
		{
			(*recvBlockCount) ++;
			recvFlags[idx] |= bit; /*设置为已接收状态*/
		}
		xxMemcpy(outdata+(blockIdx*LIUMA_VALID_BLOCKSIZE), ucBlockBuffer+validOffset, validLen);

		if (lastBlock && (*receivedMaxBlockCount == 0)) /*第一次收到的最后一块，即是最大块个数*/
		{
			*receivedMaxBlockCount = LIUMA_CHAIN_NUM(ucBlockBuffer[1]);
			*outdataLen = (blockIdx * LIUMA_VALID_BLOCKSIZE) + validLen;
		}
		return LIUMA_RV_OK;
	}

	return LIUMA_RV_WRONG_IBLOCK;
}


/*状态机*/
#define LIUMA_FSM_SEND				1
#define LIUMA_FSM_RECEIVE			2
/*操作*/
#define LIUMA_OPR_IDLE				0
#define LIUMA_OPR_RESEND			1
#define LIUMA_OPR_NAK				2
#define LIUMA_OPR_EMPTYNAK			3

/* 发送一包数据，并接收一包数据  */
LIUMA_RESULT LIUMA_Transmit(uchar * indata, uint indataLen, uchar * outdata, uint *outdataLen, uint maxTryTimes)
{
	uint nReceivedMaxBlockCount, nReceivedBlockCount, tryTimes;
	uchar stat, opr;
	uchar ucBlockFlags[LIUMA_MAX_BLOCKFLAGS_SIZE]; /*块使用标记列表*/
	uchar ucBlockBuffer[LIUMA_BLOCKSIZE+1]; /*多分配1字节用于底层通讯*/
	LIUMA_RESULT hr;

	if (indataLen > LIUMA_MAX_BUFFERSIZE) /*长度过大，ucBlockFlags不够存放*/
	{
		return LIUMA_RV_OUTOFBUFFER;
	}

	nReceivedMaxBlockCount = 0;
	nReceivedBlockCount = 0; /*已经接收到的块个数，可能乱序*/
	xMemset(ucBlockFlags, 0, LIUMA_MAX_BLOCKFLAGS_SIZE);

	stat = LIUMA_FSM_SEND; /*发送状态*/
	LIUMA_SendIBlocks(indata, indataLen);
	opr = LIUMA_OPR_IDLE; /*空闲操作*/
	for (tryTimes=0; tryTimes<maxTryTimes; )
	{
		switch(opr)
		{
		case LIUMA_OPR_RESEND: /*发送IBlock*/
			hr = LIUMA_ReSendIBlocks(ucBlockFlags, indata, indataLen);
			break;
		case LIUMA_OPR_NAK: /*发送NAK*/
			ucBlockBuffer[0] = LIUMA_PCB_RBLOCK;
			LIUMA_MakeRBlockData(ucBlockFlags, LIUMA_MAX_BLOCKFLAGS_SIZE, ucBlockBuffer);
			hr = LIUMA_SendRawDataBlock(ucBlockBuffer);
			break;
		case LIUMA_OPR_EMPTYNAK: /*发送空NAK*/
			ucBlockBuffer[0] = LIUMA_PCB_RBLOCK;
			LIUMA_MakeRBlockData(NULL, 0, ucBlockBuffer); /*有效长度为0*/
			hr = LIUMA_SendRawDataBlock(ucBlockBuffer);
			break;
		default:
			hr = LIUMA_RV_OK;
			break;
		};

		if (hr == LIUMA_RV_COMM_INTERRUPT)
		{
			return LIUMA_RV_COMM_INTERRUPT; /*只有通信中断才退出*/
		}
		/*接收一个块*/
		hr = LIUMA_RecvRawDataBlock(ucBlockBuffer);
		if (hr == LIUMA_RV_COMM_INTERRUPT)
		{
			return LIUMA_RV_COMM_INTERRUPT; /*只有通信中断才退出*/
		}

		if (hr != LIUMA_RV_OK)
		{
			ucBlockBuffer[0] = 0xFF; /*错误响应*/
		}
		switch(ucBlockBuffer[0] & 0xC0)
		{
		case 0x00: /*收到IBlock*/
			if (LIUMA_TOGGLE_CMP(ucBlockBuffer[0])) /*序号正确*/
			{
				if (stat != LIUMA_FSM_RECEIVE) /*切换为接收状态*/
				{
					xMemset(ucBlockFlags, 0, LIUMA_MAX_BLOCKFLAGS_SIZE);
					stat = LIUMA_FSM_RECEIVE;
					opr = LIUMA_OPR_IDLE; /*空闲操作*/
				}
				/*检测数据正确性，保存接收到数据*/
				if (LIUMA_SaveIBlock(ucBlockBuffer, ucBlockFlags, &nReceivedMaxBlockCount, &nReceivedBlockCount, outdata, outdataLen) != LIUMA_RV_OK)
				{
					LIUMA_TOGGLE_INC();
					return LIUMA_RV_WRONG_IBLOCK;
				}
				if (ucBlockBuffer[1] & LIUMA_CHAIN_LASTBLOCK) /*已经收到最后一块*/
				{
					if (nReceivedBlockCount >= nReceivedMaxBlockCount)
					{
						LIUMA_TOGGLE_INC();
						return LIUMA_RV_OK; /*接收完成*/
					}

					opr = LIUMA_OPR_NAK; /*但整包不完整，NAK操作*/
				}
				else
				{
					opr = LIUMA_OPR_IDLE; /*空闲操作*/
				}
			}
			else /*序号错误*/
			{
				opr = LIUMA_OPR_IDLE; /*空闲操作*/
			}
			break;
		case 0x80: /*收到RBlock*/
			tryTimes++;
			if ((stat == LIUMA_FSM_SEND) && LIUMA_TOGGLE_CMP(ucBlockBuffer[0])) /*发送状态，并且序号正确*/
			{
				uint tmpIdx, tmpDataOffset, tmpDataLength;
				LIUMA_ParseRBlockData(ucBlockBuffer, &tmpDataOffset, &tmpDataLength);
				for (tmpIdx=0; tmpIdx<tmpDataLength; tmpIdx++)
				{
					ucBlockFlags[tmpIdx] |= ucBlockBuffer[tmpDataOffset+tmpIdx];
				}
				opr = LIUMA_OPR_RESEND; /*发送操作*/
			}
			else /*序号错误*/
			{
				opr = LIUMA_OPR_IDLE; /*空闲操作*/
			}
			break;
		case 0x40: /*收到SBlock(WTX)*/
			tryTimes = 0;
			opr = LIUMA_OPR_IDLE; /*空闲操作*/
			break;
		default:
			tryTimes++;
			/*如果在发送状态没有收到响应，应发送空NAK*/
			opr = (stat == LIUMA_FSM_SEND)? LIUMA_OPR_EMPTYNAK : LIUMA_OPR_NAK;
			break;
		}
	}
	return LIUMA_RV_TIMEOUT;
}
/****************************************************************
*					刘码公共代码-end							*
****************************************************************/


		// 继承类来实现以下两个函数
		virtual HRESULT SendRawDataBlock(LPBYTE pbBlockBuff)
		{
			if (m_pSubProtocol != NULL)
			{
				return m_pSubProtocol->Control(AKEY_PROTOCOL_COMMAND_BULKWRITE, pbBlockBuff, LIUMA_BLOCKSIZE, NULL, NULL, 1000);
			}
			return AKEY_RV_FAIL;
		}
		virtual HRESULT RecvRawDataBlock(LPBYTE pbBlockBuff)
		{
			if (m_pSubProtocol != NULL)
			{
				UINT32 unRecvLen = LIUMA_BLOCKSIZE;
				return m_pSubProtocol->Control(AKEY_PROTOCOL_COMMAND_BULKREAD, NULL, 0, pbBlockBuff, &unRecvLen, 5000);
			}
			return AKEY_RV_FAIL;
		}

		inline LIUMA_RESULT LIUMA_SendRawDataBlock(BYTE * indata)
		{
			return SendRawDataBlock(indata);
		}
		inline LIUMA_RESULT LIUMA_RecvRawDataBlock(BYTE * outdata)
		{
			return RecvRawDataBlock(outdata);
		}


	};
};

#endif
