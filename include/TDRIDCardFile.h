//
//  TDRTokenStorage.h
//  TDRToken
//
//  Created by lujun on 1/26/15.
//  Copyright (c) 2015 lujun. All rights reserved.
//

#ifndef __TDRIDCard__TDRIDCardFle__
#define __TDRIDCard__TDRIDCardFle__
#include "IDCardInclude.h"

class TDRIDCardFile : public IDCARD::IIDCardFile {
public:
    virtual HRESULT Write(const char* szFileName,CBuffer& inBuf);
private:
    char m_fileName[NAME_MAX];
};

#endif /* defined(__TDRToken__TDRTokenStorage__) */
