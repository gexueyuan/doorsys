// 不要在此处包含其它头文件，便于拷贝本文件代码给其它工程使用
#include "AKeyBase.h"
#include "OpenAlg.h"
#include "KeyCode.inc"


namespace OpenAlg
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// static
	unsigned int CRC::DoCRC16(LPBYTE pbData, int nLength, unsigned int nSeed)
	{
		unsigned int crc = nSeed;
		for (int i=0; i<nLength; i++)
		{
			BYTE ch = pbData[i];
			for (int j=0; j<8; j++)
			{
				BYTE bit = (BYTE)(crc & 1);
				if (ch & 1)
				{
					bit ^= 1;
				}
				if (bit)
				{
					crc ^= 0x4002;
				}
				crc >>= 1;
				if (bit)
				{
					crc |= 0x8000;
				}
				ch >>= 1;
			}
		}
		return crc;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// static
	int ECC::Encode(LPBYTE pbData, int nLength)
	{
		int j = nLength;
		for (int i=0; i<nLength; i+=4)
		{
			BYTE btChk = 0;
			if ((nLength-i) < 4)
			{
				BYTE abTmp[4] = {0,0,0,0};
				memcpy(abTmp, pbData+i, nLength-i);
				btChk = edac_en(abTmp);
			}
			else
			{
				btChk = edac_en(pbData+i);
			}
			pbData[j++] = btChk;
		}
		return j;
	}
	int ECC::Decode(LPBYTE pbData, int nLength)
	{
		int j = nLength;
		for (int i=0; i<nLength; i+=4)
		{
			if ((nLength-i) < 4)
			{
				BYTE abTmp[4] = {0,0,0,0};
				memcpy(abTmp, pbData+i, nLength-i);
				edac_de(abTmp, pbData[j++]);
				memcpy(pbData+i, abTmp, nLength-i);
			}
			else
			{
				edac_de(pbData+i, pbData[j++]);
			}
		}
		return j;
	}

	BYTE ECC::edac_en(LPBYTE pbData)
	{
		//var gM1 = "FF02128309FF24841010FF36222580FF65490F68868EF808D8F04151";
		const BYTE gM1[4*7] = {0xFF,0x02,0x12,0x83,0x09,0xFF,0x24,0x84,0x10,0x10,0xFF,0x36,0x22,0x25,\
									0x80,0xFF,0x65,0x49,0x0F,0x68,0x86,0x8E,0xF8,0x08,0xD8,0xF0,0x41,0x51};
		BYTE chkdata = 0;
		BYTE i,j;
		BYTE edc_mask = 0x01;
		const BYTE *gMatrix = gM1;
		for(i=0;i<7;i++)
		{
			BYTE edc_bit = 0;
			for(j=0;j<4;j++)
			{
				//var tmp = Def.Hex2Int(gM1.substr(2*(i*4+j),2)) & indata[j];
				//uchar tmp = gM1[i*4+j] & indata[j];
				BYTE tmp = *(gMatrix++) & pbData[j];
				if(tmp&0x80) edc_bit ^= 1;
				if(tmp&0x40) edc_bit ^= 1;
				if(tmp&0x20) edc_bit ^= 1;
				if(tmp&0x10) edc_bit ^= 1;
				if(tmp&0x08) edc_bit ^= 1;
				if(tmp&0x04) edc_bit ^= 1;
				if(tmp&0x02) edc_bit ^= 1;
				if(tmp&0x01) edc_bit ^= 1;
			}
			if(edc_bit)
				chkdata += edc_mask;
			edc_mask <<= 1;
		}
		return chkdata;
	}

	bool ECC::edac_de(LPBYTE pbData, BYTE chkData)
	{
		const BYTE gM1[4*7] = {0xFF,0x02,0x12,0x83,0x09,0xFF,0x24,0x84,0x10,0x10,0xFF,0x36,0x22,0x25,\
									0x80,0xFF,0x65,0x49,0x0F,0x68,0x86,0x8E,0xF8,0x08,0xD8,0xF0,0x41,0x51};
		const BYTE status1_list[32] = {0x61,0x51,0x19,0x45,0x43,0x31,0x29,0x13,0x62,0x52,0x4A,0x46,0x32,0x2A,0x23,0x1A,\
									0x2C,0x64,0x26,0x25,0x34,0x16,0x15,0x54,0x0B,0x58,0x1C,0x4C,0x38,0x0E,0x0D,0x49};

		BYTE syndrome = 0;
		BYTE i,j;
		BYTE edc_mask = 0x01;
		const BYTE *gMatrix = gM1;
		for(i=0;i<7;i++)
		{		
				BYTE edc_bit = 0;
				for(j=0;j<4;j++)
				{
					BYTE tmp = *(gMatrix++) & pbData[j];
					if(tmp&0x80) edc_bit ^= 1;
					if(tmp&0x40) edc_bit ^= 1;
					if(tmp&0x20) edc_bit ^= 1;
					if(tmp&0x10) edc_bit ^= 1;
					if(tmp&0x08) edc_bit ^= 1;
					if(tmp&0x04) edc_bit ^= 1;
					if(tmp&0x02) edc_bit ^= 1;
					if(tmp&0x01) edc_bit ^= 1;
				}
				if(edc_bit)
					syndrome += edc_mask;
				edc_mask <<= 1;
		}	
		syndrome ^= chkData;
		
		if(syndrome == 0)
		{
			return true;
		}
		else
		{
			for(i=0;i<32;++i)
			{
				if(status1_list[i] == syndrome)
				{
					pbData[i/8] ^= (0x01<<(7-(i%8)));
					return true;
				}
			}
		}
		return false;
	}

	
	int CDigest::Digest(const char * pszAlgName, LPBYTE pbData, int nLength, LPBYTE pbOut)
	{
		CDigest dgst;
		if (!dgst.Init(pszAlgName))
		{
			return -1;
		}

		unsigned int nOutLen = 64;
		dgst.Update(pbData, nLength);
		dgst.Final(pbOut, &nOutLen);
		return nOutLen;
	}
	int CDigest::GetInternalData_SM3(LPBYTE pbPubKey, int nPubKeyLen, LPBYTE pbData, int nLength, LPBYTE pbInternalResult, LPBYTE pbMdResult)
	{
        BYTE abTmpBuf[256+256], abMd[32];
        memcpy(abTmpBuf, "\x00\x80\x31\x32\x33\x34\x35\x36\x37\x38\x31\x32\x33\x34\x35\x36\x37\x38\xFF\xFF\xFF\xFE\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF\x00\x00\x00\x00\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFC\x28\xE9\xFA\x9E\x9D\x9F\x5E\x34\x4D\x5A\x9E\x4B\xCF\x65\x09\xA7\xF3\x97\x89\xF5\x15\xAB\x8F\x92\xDD\xBC\xBD\x41\x4D\x94\x0E\x93\x32\xC4\xAE\x2C\x1F\x19\x81\x19\x5F\x99\x04\x46\x6A\x39\xC9\x94\x8F\xE3\x0B\xBF\xF2\x66\x0B\xE1\x71\x5A\x45\x89\x33\x4C\x74\xC7\xBC\x37\x36\xA2\xF4\xF6\x77\x9C\x59\xBD\xCE\xE3\x6B\x69\x21\x53\xD0\xA9\x87\x7C\xC6\x2A\x47\x40\x02\xDF\x32\xE5\x21\x39\xF0\xA0", 210-64);
        memcpy(abTmpBuf+210-64, pbPubKey+1, nPubKeyLen-1);
        OpenAlg::CDigest::Digest("SM3", abTmpBuf, 210, abMd);
        
        // 取摘要中间结果
        OpenAlg::CDigest digest;
        digest.Init("SM3");
        digest.Update(abMd, 0x20);
        digest.Update(pbData, nLength);
        if (pbInternalResult != NULL)
        {
            //digest.GetInternalData(4, 0x20, pbInternalResult);
        }
        
        unsigned int unHashLen = 0x20;
        digest.Final(pbMdResult, (pbMdResult==NULL)? NULL : &unHashLen);
        return unHashLen;
    }
    
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////
	// static CCipher
	int CCipher::CipherWithKeyCode(const char * pszAlgName, LPBYTE pbIv, UINT32 un32Flags, LPBYTE pbIn, int nInLen, LPBYTE pbOut)
	{
		BYTE abKeyBuff[16];
		KeyCode::byteFiller(&abKeyBuff);
		return CCipher::Cipher(pszAlgName, abKeyBuff, pbIv, un32Flags, pbIn, nInLen, pbOut);
	}
};
