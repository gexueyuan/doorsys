//
//  IDCardInterface.h
//  TDRToken
//
//  Created by wz on 15/9/24.
//  Copyright © 2015年 lujun. All rights reserved.
//

#ifndef IDCardInterface_h
#define IDCardInterface_h

#include "AKeyDef.h"
#include "AKeyInterface.h"
#include "AKeyHelper.h"

using namespace AKey;

namespace IDCARD
{
    AKEY_INTERFACE IIDCardReaderInterface
    {
        virtual HRESULT Prepare() = 0;
		virtual HRESULT ChooseApp() = 0;
        //读卡器证书长度[2]+读卡器证书数据+SSL证书长度[2]+SSL证书数据
        virtual HRESULT GetReaderCert(LPBYTE pbRecv,LPUINT32 punRLen) = 0;
        virtual HRESULT GetReaderInfo(LPBYTE pbCosVer,LPUINT32 punCosVerLen  ) = 0;
        virtual HRESULT GetCosSpeedFlag(bool* bSupportSpeed,BYTE* bSpeed) = 0;
        virtual HRESULT GetReaderPuk(BYTE btKeyID, LPBYTE pbOut,LPUINT32 punOutResult) = 0;
        virtual HRESULT GetBatteryLevel(BYTE* bBattery) = 0;
        virtual HRESULT SupportResume(BYTE* bSupportResume) = 0;
        virtual HRESULT GetReaderCosInfo(CBuffer& timeInterval,CBuffer& batteryHealth,CBuffer& readerBehavior) = 0;
        virtual HRESULT Transmit(LPBYTE pbSend, UINT32 unSLen,UINT32 unMaxLe,LPBYTE pbRecv,LPUINT32 punRLen) = 0;
        virtual HRESULT SM2Sign(BYTE keyID,LPBYTE pbPuK,UINT16 u16PukLen,LPBYTE pbInput,UINT32 u32InputLen,LPBYTE pbOutput,LPUINT32 punOutput) = 0;
        virtual HRESULT SM2Encrypt(LPBYTE pbInput,UINT32 u32InputLen,LPBYTE pbKey,UINT32 u32KeyLen,LPBYTE pbOutput,LPUINT32 punOutput) = 0;
        virtual HRESULT SaveSamCertAndVersion(CBuffer& samVersion,CBuffer& samCert) = 0;
        virtual HRESULT LoadSamCertAndVersion(CBuffer& samVersion,CBuffer& samCert) = 0;
    };
    
    AKEY_INTERFACE IIDCardBusInterface
    {
        virtual HRESULT GetRandomNum(LPBYTE pbSN,UINT32 u32SNLen,LPBYTE pbCustomID,UINT32 u32CustomIDLen) = 0;
        //virtual HRESULT
    };
    
    AKEY_INTERFACE IIDCardIOInterface
    {
        virtual HRESULT WriteAndRead(IDREADER_PROCESS_STATUS proStatus,CBuffer& inBuf,CBuffer& outBuf) = 0;
        
    };
    
    AKEY_INTERFACE IIDCardFile
    {
        virtual HRESULT Write(const char* szFileName,CBuffer& inBuf) = 0;
    };
    
    AKEY_INTERFACE IPackage
    {
        virtual HRESULT MakePackage(CBuffer& inBuf,CBuffer& outBuf) = 0;
        virtual HRESULT ParsePackage(CBuffer& inBuf,CBuffer& outBuf)= 0 ;
    };
    
    AKEY_INTERFACE ISocket
    {
        virtual void    setTimeOut(UINT32 u32ConnTime,UINT32 u32SRTime) = 0;
		virtual void    setConnParam(const char * pszHostIP, int nPort) = 0;
        virtual HRESULT ConnectServer() = 0;
		virtual bool    IsSocketValid() = 0;
		virtual IDCARD_NET_TYPE GetNetType() = 0;
		virtual const char* GetIp() = 0;
		virtual int GetPort() = 0;
        virtual HRESULT DisconnectServer() = 0;
        virtual HRESULT SendAndRecv(CBuffer& sendBuf,CBuffer& recvBuf) = 0;
    };
    
    AKEY_INTERFACE IIDCore
    {
		virtual HRESULT Init(IDCARD_CORE_CONTEXT* ctx) = 0;
		virtual HRESULT Begin() = 0;
        virtual HRESULT RequestPort() = 0;
        virtual HRESULT ReadIDInfo(CBuffer& IDInfoBufer) = 0;
        virtual HRESULT ReadImage(CBuffer& imgBufer) = 0;
        virtual HRESULT Finial() = 0;
    };
};

#endif /* IDCardInterface_h */
