#ifndef __AKEY_INTERFACE_H__
#define __AKEY_INTERFACE_H__

#include "AKeyDef.h"


namespace AKey
{
	// 跟平台相关（API）接口
	AKEY_INTERFACE IMutex
	{
		virtual HRESULT Lock() = 0;
		virtual HRESULT Unlock() = 0;
	};
	AKEY_INTERFACE IEvent
	{
		virtual HRESULT Set() = 0;
		virtual HRESULT WaitFor(UINT32 dwMilliseconds) = 0;
	};


	// 协议-命令
#define AKEY_PROTOCOL_COMMAND_TRANSMIT		0
#define AKEY_PROTOCOL_COMMAND_BULKWRITE		1
#define AKEY_PROTOCOL_COMMAND_BULKREAD		2
#define AKEY_PROTOCOL_COMMAND_DATAEXCHANGE	3
	AKEY_INTERFACE IProtocol
	{
		virtual HRESULT Control(UINT32 unCmd, LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen, UINT32 unTimeout) = 0;
		virtual IProtocol * GetSubProtocol() = 0;
	};

	AKEY_INTERFACE IReader : public IProtocol
	{
		virtual HRESULT CheckDevice(bool * pbNewDevice) = 0;
		virtual HRESULT ReadParam(UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
	};

	AKEY_INTERFACE ISession
	{
		virtual HRESULT Init(IReader *pReaderObj, IProtocol *pProtocolObj, bool bAutoFreeProtocolObj) = 0;
		virtual HRESULT Final() = 0;
		virtual HRESULT Open(UINT32 unFlags, LPBYTE pbResp) = 0;
		virtual HRESULT Close() = 0;
		virtual HRESULT Transmit(LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen) = 0;
		virtual UINT32 GetTokenInitFlags() = 0;
		virtual IReader * GetReader() = 0;
		virtual AKEY_SESSION_STATUS_PTR GetStatus() = 0;
	};

	AKEY_INTERFACE IToken
	{
		virtual HRESULT Init(ISession * pSessionObj, UINT32 un32InitFlags) = 0;
		virtual HRESULT Final() = 0;
		virtual HRESULT ClearCache() = 0;

		virtual HRESULT CheckDevice(LPUINT32 pun32Flags) = 0;

		virtual HRESULT Prepare(UINT32 un32Flags) = 0;
		virtual HRESULT VerifyPin(UINT32 un32Flags, LPBYTE pbUserPin, UINT32 dwUserPinLen) = 0;
		virtual HRESULT ChangePin(UINT32 un32Flags, LPBYTE pbOldPin, UINT32 dwOldPinLen, LPBYTE pbNewPin, UINT32 dwNewPinLen) = 0;
		virtual HRESULT Decrypt(UINT32 dwKeyID, LPBYTE pbEncryptedData, UINT32 dwEncryptedDataLen, LPBYTE pbData, LPUINT32 pdwDataLen) = 0;
		virtual HRESULT Sign(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbPCHash, UINT32 dwPCHashLen, UINT32 dwPCDataLen, LPBYTE pbShowData, UINT32 dwShowDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
		virtual HRESULT SignEx(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbTransData, UINT32 dwTransDataLen, LPBYTE pbShowData, UINT32 dwShowDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
		virtual HRESULT QSign(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbSignData, UINT32 dwSignDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
		virtual HRESULT QSignEx(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbUserPin, UINT32 dwUserPinLen, LPBYTE pbTransData, UINT32 dwTransDataLen, LPBYTE pbShowData, UINT32 dwShowDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
		virtual HRESULT GetSignResult(UINT32 dwKeyID, UINT32 dwSignFlags, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
		virtual HRESULT ScanPressKey() = 0;
		virtual HRESULT Cancel() = 0;

		virtual HRESULT ReadParam(UINT32 un32Flags, UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
		virtual HRESULT WriteParam(UINT32 un32Flags, UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen) = 0;
		virtual HRESULT ExecParam(UINT32 un32Flags, UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
	};

	AKEY_INTERFACE ICustomCommand
	{
		virtual HRESULT OnReadParam(void * pCommand, UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
		//virtual HRESULT OnWriteParam(void * pCommand, UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen) = 0;
	};

	AKEY_INTERFACE ICustomToken
	{
		virtual HRESULT OnReadParam(void * pToken, UINT32 un32Flags, UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen, LPBYTE pbResult, LPUINT32 pdwResultLen) = 0;
		//virtual HRESULT OnWriteParam(void * pToken, UINT32 un32Flags, UINT32 dwParamID, LPBYTE pbData, UINT32 dwDataLen) = 0;
	};

	AKEY_INTERFACE IStream
	{
		virtual int Read(LPBYTE pbBuffer, int nLength) = 0; 
		virtual int Write(LPBYTE pbBuffer, int nLength) = 0; 
		virtual int GetPosition() = 0; 
		virtual int SetPosition(int nPos) = 0; 
		virtual int GetSize() = 0;
		virtual int SetSize(int nSize) // 可以不实现
		{
			return -1;
		}
	};

};

#endif
