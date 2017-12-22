//
//  TDRTokenStorage.cpp
//  TDRToken
//
//  Created by lujun on 1/26/15.
//  Copyright (c) 2015 lujun. All rights reserved.
//

#include "TDRIDCardFile.h"
#include <stdio.h>

HRESULT TDRIDCardFile::Write(const char* szFileName,CBuffer& inBuf)
{
    FILE* hFile = fopen(szFileName, "wb+");
    if (hFile == NULL) {
        return AKEY_RV_FAIL;
    }
    fseek(hFile, 0, SEEK_SET);
    HRESULT hr = (fwrite(inBuf.GetBuffer(), 1, inBuf.GetLength(), hFile) == inBuf.GetLength())? AKEY_RV_OK : AKEY_RV_FAIL;
    fclose(hFile);
    return hr;
}

//#include "OpenAlg.inc"
