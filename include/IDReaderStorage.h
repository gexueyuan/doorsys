//
//  TDRTokenStorage.h
//  TDRToken
//
//  Created by lujun on 1/26/15.
//  Copyright (c) 2015 lujun. All rights reserved.
//

#ifndef __IDReader__TDRIDReaderStorage__
#define __IDReader__TDRIDReaderStorage__
#include "AKeyBase.h"
#include "AKeyStorageBase.h"
#include "AKeyHelper.h"
#include "alg/OpenAlg.h"
#include "IDCardConfig.h"
#include "IDCardError.h"

namespace AKey {
    class IDReaderStorage : public AKey::CStorageBase {
    public:
        IDReaderStorage()
        {
        };
        HRESULT SaveSamCertAndVersion(CBuffer& samVerBuf,CBuffer& samCert)
        {
            HRESULT hr = AKEY_RV_OK;
            AppendTLV_cache(TAG_GROUP_SAM_CERT_VERSION, samVerBuf.GetLength(),samVerBuf.GetBuffer());
            AppendTLV_cache(TAG_GROUP_SAM_CERT, samCert.GetLength(),samCert.GetBuffer());
            SaveCache();
            return hr;
            
        }
        HRESULT LoadSamCertAndVersion(CBuffer& samVerBuf,CBuffer& samCert)
        {
            HRESULT hr = AKEY_RV_OK;
            LoadCache();
            int nPos = FindTLV_cache(TAG_GROUP_SAM_CERT_VERSION);
            if (nPos < 0)
            {
                return AKEY_RV_CACHE_NO_SAM_CERT;
            }
            memcpy(samVerBuf.ReAllocBytesSetLength(GetTLV_Length_cache(nPos)),GetTLV_Value_cache(nPos),GetTLV_Length_cache(nPos));
            
            nPos = FindTLV_cache(TAG_GROUP_SAM_CERT);
            if (nPos < 0)
            {
                return AKEY_RV_CACHE_NO_SAM_CERT;
            }
            memcpy(samCert.ReAllocBytesSetLength(GetTLV_Length_cache(nPos)),GetTLV_Value_cache(nPos),GetTLV_Length_cache(nPos));
            return hr;
        }
    protected:
        virtual HRESULT ReadConfig(UINT32 dwOffset, UINT32 dwLength, LPBYTE pbData)
        {
            return AKEY_RV_OK;
            
        }
        virtual HRESULT WriteConfig(UINT32 dwOffset, UINT32 dwLength, LPBYTE pbData)
        {
            return AKEY_RV_OK;
        }
    private:
    };
}


#endif /* defined(__TDRToken__TDRTokenStorage__) */
