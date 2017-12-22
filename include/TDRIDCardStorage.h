//
//  TDRTokenStorage.h
//  TDRToken
//
//  Created by lujun on 1/26/15.
//  Copyright (c) 2015 lujun. All rights reserved.
//

#ifndef __TDRIDCard__TDRIDCardStorage__
#define __TDRIDCard__TDRIDCardStorage__
#include "IDReaderStorage.h"
#include <limits.h>

class TDRIDCardStorage : public AKey::IDReaderStorage {
public:
	TDRIDCardStorage(const char* szFilePath);
    virtual HRESULT ReadConfig(UINT32 dwOffset, UINT32 dwLength, LPBYTE pbData);
    virtual HRESULT WriteConfig(UINT32 dwOffset, UINT32 dwLength, LPBYTE pbData);
private:
    char m_filePath[NAME_MAX];
};

#endif /* defined(__TDRToken__TDRTokenStorage__) */
