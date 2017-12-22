//
//  TDRTokenStorage.cpp
//  TDRToken
//
//  Created by lujun on 1/26/15.
//  Copyright (c) 2015 lujun. All rights reserved.
//

#include "TDRIDCardStorage.h"
#include <stddef.h>
#include <stdio.h>

TDRIDCardStorage::TDRIDCardStorage(const char* szFilePath)
{
	strcpy(m_filePath,szFilePath);
}

// 读配置
HRESULT TDRIDCardStorage::ReadConfig(UINT32 dwOffset, UINT32 dwLength, LPBYTE pbData)
{
    FILE *fileCfg = fopen(m_filePath, "rb+");
    if(fileCfg == NULL)
        return AKEY_RV_OPEN_FILE_ERROR;
    
    fseek(fileCfg, dwOffset, SEEK_SET);
    HRESULT hr = (fread(pbData, 1, dwLength, fileCfg) == dwLength)? AKEY_RV_OK : AKEY_RV_FAIL;
    fclose(fileCfg);
    return  hr;
}

// 写配置
HRESULT TDRIDCardStorage::WriteConfig(UINT32 dwOffset, UINT32 dwLength, LPBYTE pbData)
{
    FILE *fileCfg = fopen(m_filePath, "rb+");
    if(fileCfg == NULL)
    {
        fileCfg = fopen(m_filePath, "wb+");
        if(fileCfg == NULL)
            return AKEY_RV_FAIL;
    }
    
    fseek(fileCfg, dwOffset, SEEK_SET);
    HRESULT hr = (fwrite(pbData, 1, dwLength, fileCfg) == dwLength)? AKEY_RV_OK : AKEY_RV_FAIL;
    fclose(fileCfg);
    return  hr;
}

//#include "OpenAlg.inc"
