//#include "AKeyDef.h"
//#include "AKeyHelper.h"

#include "OpenAlg.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

#ifndef AKEY_SUPPORT_OPENSSL_TDR
#define AKEY_SUPPORT_OPENSSL_TDR 1	// 默认不支持自定义的openssl算法库
#endif
#ifndef AKEY_SUPPORT_SM_PKCS7
#define AKEY_SUPPORT_SM_PKCS7 1		// 默认支持国密算法的P7
#endif

#if (AKEY_SUPPORT_OPENSSL_TDR)
#include <openssl/evp_tdr.h>
#else
#include "sm3.inc"
#include "sm4.inc"
#endif
#if (AKEY_SUPPORT_SM_PKCS7)
#include "sm_pk7_asn1.inc"
#else
#define SM_PKCS7_new() NULL
#define SM_PKCS7_free(p7) NULL
#endif
#include "KeyCode.inc"

//自动载入动态库
#ifdef WIN32
#pragma comment(lib, "libeay32.lib")
# if (AKEY_SUPPORT_OPENSSL_TDR)
#  pragma comment(lib, "tdreay32.lib")
# endif
#pragma message("Automatically linking with libeay32.lib")
#else
#define __try	// Android没有支持
#define __except(e) if(0)
#endif

// 增加OID
static int NID_sha256sm3 = OBJ_create("1.19.30.19.18.42", "SHA256SM3", "sha256sm3");


namespace OpenAlg
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class CAutoCleanup
	class CAutoCleanup
	{
	public:
		CAutoCleanup()
		{
#if 1
			OpenSSL_add_all_algorithms();
#else
			//OPENSSL_cpuid_setup();
			
#ifndef OPENSSL_NO_DES
			EVP_add_cipher(EVP_des_cfb());
			//EVP_add_cipher(EVP_des_cfb1());
			//EVP_add_cipher(EVP_des_cfb8());
			EVP_add_cipher(EVP_des_ede_cfb());
			EVP_add_cipher(EVP_des_ede3_cfb());
			//EVP_add_cipher(EVP_des_ede3_cfb1());
			//EVP_add_cipher(EVP_des_ede3_cfb8());

			EVP_add_cipher(EVP_des_ofb());
			EVP_add_cipher(EVP_des_ede_ofb());
			EVP_add_cipher(EVP_des_ede3_ofb());

			//EVP_add_cipher(EVP_desx_cbc());
			//EVP_add_cipher_alias(SN_desx_cbc,"DESX");
			//EVP_add_cipher_alias(SN_desx_cbc,"desx");

			EVP_add_cipher(EVP_des_cbc());
			//EVP_add_cipher_alias(SN_des_cbc,"DES");
			//EVP_add_cipher_alias(SN_des_cbc,"des");
			EVP_add_cipher(EVP_des_ede_cbc());
			EVP_add_cipher(EVP_des_ede3_cbc());
			//EVP_add_cipher_alias(SN_des_ede3_cbc,"DES3");
			//EVP_add_cipher_alias(SN_des_ede3_cbc,"des3");

			EVP_add_cipher(EVP_des_ecb());
			EVP_add_cipher(EVP_des_ede());
			EVP_add_cipher(EVP_des_ede3());
#endif

#ifndef OPENSSL_NO_RC4
			EVP_add_cipher(EVP_rc4());
			//EVP_add_cipher(EVP_rc4_40());
#ifndef OPENSSL_NO_MD5
			//EVP_add_cipher(EVP_rc4_hmac_md5());
#endif
#endif

#ifndef OPENSSL_NO_RC2
			EVP_add_cipher(EVP_rc2_ecb());
			EVP_add_cipher(EVP_rc2_cfb());
			EVP_add_cipher(EVP_rc2_ofb());
			EVP_add_cipher(EVP_rc2_cbc());
			//EVP_add_cipher(EVP_rc2_40_cbc());
			//EVP_add_cipher(EVP_rc2_64_cbc());
			//EVP_add_cipher_alias(SN_rc2_cbc,"RC2");
			//EVP_add_cipher_alias(SN_rc2_cbc,"rc2");
#endif

#ifndef OPENSSL_NO_AES
			EVP_add_cipher(EVP_aes_128_ecb());
			EVP_add_cipher(EVP_aes_128_cbc());
			EVP_add_cipher(EVP_aes_128_cfb());
			//EVP_add_cipher(EVP_aes_128_cfb1());
			//EVP_add_cipher(EVP_aes_128_cfb8());
			EVP_add_cipher(EVP_aes_128_ofb());
			//EVP_add_cipher(EVP_aes_128_ctr());
			//EVP_add_cipher(EVP_aes_128_gcm());
			//EVP_add_cipher(EVP_aes_128_xts());
			//EVP_add_cipher_alias(SN_aes_128_cbc,"AES128");
			//EVP_add_cipher_alias(SN_aes_128_cbc,"aes128");
			EVP_add_cipher(EVP_aes_192_ecb());
			EVP_add_cipher(EVP_aes_192_cbc());
			EVP_add_cipher(EVP_aes_192_cfb());
			//EVP_add_cipher(EVP_aes_192_cfb1());
			//EVP_add_cipher(EVP_aes_192_cfb8());
			EVP_add_cipher(EVP_aes_192_ofb());
			//EVP_add_cipher(EVP_aes_192_ctr());
			//EVP_add_cipher(EVP_aes_192_gcm());
			//EVP_add_cipher_alias(SN_aes_192_cbc,"AES192");
			//EVP_add_cipher_alias(SN_aes_192_cbc,"aes192");
			EVP_add_cipher(EVP_aes_256_ecb());
			EVP_add_cipher(EVP_aes_256_cbc());
			EVP_add_cipher(EVP_aes_256_cfb());
			//EVP_add_cipher(EVP_aes_256_cfb1());
			//EVP_add_cipher(EVP_aes_256_cfb8());
			EVP_add_cipher(EVP_aes_256_ofb());
			//EVP_add_cipher(EVP_aes_256_ctr());
			//EVP_add_cipher(EVP_aes_256_gcm());
			//EVP_add_cipher(EVP_aes_256_xts());
			//EVP_add_cipher_alias(SN_aes_256_cbc,"AES256");
			//EVP_add_cipher_alias(SN_aes_256_cbc,"aes256");
#if !defined(OPENSSL_NO_SHA) && !defined(OPENSSL_NO_SHA1)
			//EVP_add_cipher(EVP_aes_128_cbc_hmac_sha1());
			//EVP_add_cipher(EVP_aes_256_cbc_hmac_sha1());
#endif
#endif

			EVP_add_digest(EVP_md5());
			EVP_add_digest(EVP_sha1());
			EVP_add_digest(EVP_sha256());
			EVP_add_digest(EVP_sha384());
			EVP_add_digest(EVP_sha512());
								
			//ENGINE_setup_bsd_cryptodev();
			
#endif

			OpenSSL_add_sm3_digest();
			OpenSSL_add_sm4_cipher();
		}
		
		~CAutoCleanup()
		{
			EVP_cleanup();
			OBJ_cleanup();
			// 调用OpenSSL的crypto库，在退出前需要调用API "CRYPTO_cleanup_all_ex_data"，清除管理CRYPTO_EX_DATA的全局hash表中的数据，避免内存泄漏。
			CRYPTO_cleanup_all_ex_data(); 
		}
	};	
	static CAutoCleanup s_autoCleanup;

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


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// static
	int CBase64::Encode(LPBYTE pbData, int nDataLen, LPBYTE pbOut)
	{
		int nOutMaxLen = ((( nDataLen << 3 ) + 5) / 6) + 3;
		nOutMaxLen += ((nOutMaxLen + 63) / 64); // \n
		if (pbOut == NULL)
		{
			return nOutMaxLen;
		}
		
		EVP_ENCODE_CTX encodeCtx;
		EVP_EncodeInit(&encodeCtx);		
		int nOutLen1 = nOutMaxLen;
		EVP_EncodeUpdate(&encodeCtx, pbOut, &nOutLen1, pbData, nDataLen);
		int nOutLen2 = nOutMaxLen - nOutLen1;
		EVP_EncodeFinal(&encodeCtx, pbOut+nOutLen1, &nOutLen2);

		return nOutLen1 + nOutLen2;	
	}
	int CBase64::Decode(LPBYTE pbData, int nDataLen, LPBYTE pbOut)
	{
		int nOutMaxLen = ( ( nDataLen * 6 ) + 7 ) >> 3;
		if (pbOut == NULL)
		{
			return nOutMaxLen;
		}

		EVP_ENCODE_CTX encodeCtx;
		EVP_DecodeInit(&encodeCtx);		
		int nOutLen1 = nOutMaxLen;
		EVP_DecodeUpdate(&encodeCtx, pbOut, &nOutLen1, pbData, nDataLen);
		int nOutLen2 = nOutMaxLen - nOutLen1;
		EVP_DecodeFinal(&encodeCtx, pbOut+nOutLen1, &nOutLen2);

		return nOutLen1 + nOutLen2;	
	}

#ifdef __AKEY_BUFFER_H__
	int CBase64::Encode(LPBYTE pbData, int nDataLen, AKey::CBuffer & resultBuff)
	{
		int nOutMaxLen = Encode(pbData, nDataLen, (LPBYTE)NULL);
		int nOutLen = Encode(pbData, nDataLen, resultBuff.ReAllocBytes(nOutMaxLen+32));
		resultBuff.SetLength( nOutLen );
		return nOutLen;
	}
	int CBase64::Decode(LPBYTE pbData, int nDataLen, AKey::CBuffer & resultBuff)
	{
		int nOutMaxLen = Decode(pbData, nDataLen, (LPBYTE)NULL);
		int nOutLen = Decode(pbData, nDataLen, resultBuff.ReAllocBytes(nOutMaxLen+32));
		resultBuff.SetLength( nOutLen );
		return nOutLen;
	}
#endif


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// static
	int CRand::GetBytes(LPBYTE pbSeed, int nSeedLen, LPBYTE pbRand, int nRandLen)
	{
		if (pbSeed && nSeedLen > 0)
		{
			RAND_seed(pbSeed, nSeedLen);
		}

		return RAND_bytes(pbRand, nRandLen); 
	}
	
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// 摘要计算
	CDigest::CDigest() : m_pCtx(NULL)
	{
	}
	
	CDigest::~CDigest()
	{
		if (m_pCtx != NULL)
		{
			CDigest::Final(NULL, NULL);
		}
	}

	bool CDigest::Init(const char * pszAlgName)
	{
		const EVP_MD * pMD = EVP_get_digestbyname(pszAlgName);	// md5, sha1
		if (pMD == NULL)
			return false;

		EVP_MD_CTX * pMDCtx = EVP_MD_CTX_create();

		EVP_DigestInit(pMDCtx, pMD);

		if (m_pCtx != NULL)
		{
			CDigest::Final(NULL, NULL);
		}
		m_pCtx = pMDCtx;
		return true;
	}

	bool CDigest::Update(LPBYTE pbData, int nLength)
	{
		if (m_pCtx == NULL)
			return false;

		EVP_DigestUpdate((EVP_MD_CTX *)m_pCtx, pbData, nLength);
		return true;
	}

	bool CDigest::Final(LPBYTE pbOut, unsigned int * pnOutLen)
	{
		if (m_pCtx == NULL)
			return false;

		if ((pbOut != NULL) && (pnOutLen != NULL))
		{
			EVP_DigestFinal((EVP_MD_CTX *)m_pCtx, pbOut, pnOutLen);
		}
		
		EVP_MD_CTX_destroy((EVP_MD_CTX *)m_pCtx);
		m_pCtx = NULL;
		return true;
	}

	bool CDigest::GetBlockSize(int * pnBlockSize, int * pnInternalStateLen)
	{
		if (m_pCtx == NULL)
			return false;

		int nBlockSize = 4;
		int nInternalStateLen = ((EVP_MD_CTX *)m_pCtx)->digest->md_size;
		if (nInternalStateLen == 28) // SHA224
		{
			nInternalStateLen = 32;
		}
		else if (nInternalStateLen == 48) // SHA384,
		{
			SHA_LONG64 *p = (SHA_LONG64 *)(((EVP_MD_CTX *)m_pCtx)->md_data);
			nBlockSize = (p[0] == 0xcbbb9d5dc1059ed8)? 8 : 4;
			nInternalStateLen = 64; 
		}
		else if (nInternalStateLen == 64) // SHA512
		{
			SHA_LONG64 *p = (SHA_LONG64 *)(((EVP_MD_CTX *)m_pCtx)->md_data);
			nBlockSize = (p[0] == 0x6a09e667f3bcc908)? 8 : 4;
		}
	
		*pnBlockSize = nBlockSize;
		*pnInternalStateLen = nInternalStateLen;
		return true;
	}

	bool CDigest::GetInternalData(int nBlockSize, int nInternalStateLen, LPBYTE pbOut)
	{
		if (m_pCtx == NULL)
			return false;
				

		if (pbOut != NULL)
		{
			HashReverse((LPBYTE)( ((EVP_MD_CTX *)m_pCtx)->md_data), nInternalStateLen, nBlockSize,  pbOut);
		}
		return true;
	}
		

	// static 
	void CDigest::HashReverse(LPBYTE pbInBuff, int nInLength, int nBlockSize, LPBYTE pbOutBuff)
	{
#if defined(__ILP64__)
#error [HashReverse] not support ILP64.
#else
		if (nBlockSize == 8)
		{
			SHA_LONG64 *p = (SHA_LONG64 *)pbInBuff;
			for (int i=0; i<nInLength; i+=8)
			{
				pbOutBuff[i+0] = (BYTE)(((*p) >> 56) & 0xFF);
				pbOutBuff[i+1] = (BYTE)(((*p) >> 48) & 0xFF);
				pbOutBuff[i+2] = (BYTE)(((*p) >> 40) & 0xFF);
				pbOutBuff[i+3] = (BYTE)(((*p) >> 32) & 0xFF);
				pbOutBuff[i+4] = (BYTE)(((*p) >> 24) & 0xFF);
				pbOutBuff[i+5] = (BYTE)(((*p) >> 16) & 0xFF);
				pbOutBuff[i+6] = (BYTE)(((*p) >> 8) & 0xFF);
				pbOutBuff[i+7] = (BYTE)((*p) & 0xFF);
				p ++;							
			}
		}
		else
		{
			SHA_LONG *p = (SHA_LONG *)pbInBuff;
			for (int i=0; i<nInLength; i+=4)
			{
				pbOutBuff[i+0] = (BYTE)(((*p) >> 24) & 0xFF);
				pbOutBuff[i+1] = (BYTE)(((*p) >> 16) & 0xFF);
				pbOutBuff[i+2] = (BYTE)(((*p) >> 8) & 0xFF);
				pbOutBuff[i+3] = (BYTE)((*p) & 0xFF);
				p ++;
			}
		}
#endif
	}
	
	int CDigest::Digest(const char * pszAlgName, LPBYTE pbData, int nLength, LPBYTE pbOut)
	{
		const EVP_MD * pMD = EVP_get_digestbyname(pszAlgName);	// md5, sha1
		if (pMD == NULL)
			return -1;
	
		EVP_MD_CTX *ctx = EVP_MD_CTX_create();
		unsigned int nOutLen = 64;
		EVP_DigestInit(ctx, pMD);
		EVP_DigestUpdate(ctx, pbData, nLength);
		EVP_DigestFinal(ctx, pbOut, &nOutLen);
		EVP_MD_CTX_destroy(ctx);
		return nOutLen;
	}

	int CDigest::GetInternalData(const char * pszAlgName, LPBYTE pbData, int nLength, LPBYTE pbOut)
	{
		CDigest digest;
		if (!digest.Init(pszAlgName))
			return -1;
	
		int nBlockSize = 4;
		int nInternalStateLen = 0;
		digest.GetBlockSize(&nBlockSize, &nInternalStateLen);

		digest.Update(pbData, nLength);
		digest.GetInternalData(nBlockSize, nInternalStateLen, pbOut);
		digest.Final(NULL, NULL);
		return nInternalStateLen;
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
			digest.GetInternalData(4, 0x20, pbInternalResult);		
		}

		unsigned int unHashLen = 0x20;
		digest.Final(pbMdResult, (pbMdResult==NULL)? NULL : &unHashLen);
		return unHashLen;
	}

	int CDigest::HMAC(const char * pszAlgName, LPBYTE pbKey, int nKeyLen, LPBYTE pbData, int nDataLen, LPBYTE pbOut)
	{
		const EVP_MD * pMD = EVP_get_digestbyname(pszAlgName);
		if (pMD == NULL)
			return -1;

		unsigned int nOutLen = 128;
		::HMAC(pMD, pbKey, nKeyLen, pbData, nDataLen, pbOut, &nOutLen);
		return nOutLen;
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//class CCipher
	CCipher::CCipher() : m_pbCtx(NULL)
	{
	}

	CCipher::~CCipher()
	{
		if (m_pbCtx != NULL)
		{			
			CCipher::Final(NULL, NULL);
		}
	}

	bool CCipher::Init(const char * pszAlgName, LPBYTE pbKey, int nKeyLen, LPBYTE pbIv, int nIvLen, UINT32 un32Flags)
	{
		const EVP_CIPHER * pCipher = EVP_get_cipherbyname(pszAlgName);
		if (pCipher == NULL)
		{
			return false;
		}


		EVP_CIPHER_CTX * pCCtx = EVP_CIPHER_CTX_new(); // new EVP_CIPHER_CTX;
		EVP_CIPHER_CTX_init(pCCtx);

		int encType = (un32Flags & CCipher::ENCRYPT)? 1 : 0;
		if (EVP_CipherInit(pCCtx, pCipher, pbKey, pbIv, encType) < 1)
		{
			delete pCCtx;
			return false;
		}

		if (pCipher->nid == NID_rc2_cbc || pCipher->nid == NID_rc2_ecb || pCipher->nid == NID_rc2_cfb64 || pCipher->nid == NID_rc2_ofb64)
		{
		// 9.0.6
		//	EVP_CIPHER_CTX *KCtx = (EVP_CIPHER_CTX *)m_pCipherCtx;
		//	RC2_set_key(&(KCtx->c.rc2.ks), pKeyBuffer.GetLengthInt(), pKeyBuffer, pKeyBuffer.GetLengthInt() * 8);
		// 9.0.8
			EVP_CIPHER_CTX_set_key_length(pCCtx, nKeyLen);
			EVP_CIPHER_CTX_ctrl(pCCtx, EVP_CTRL_SET_RC2_KEY_BITS, nKeyLen*8, NULL);
			pCipher->init(pCCtx, pbKey, pbIv, encType);
		}

		if (un32Flags & CCipher::NOPADDING)
		{
			EVP_CIPHER_CTX_set_padding(pCCtx, 0);
		}

		if (m_pbCtx != NULL)
		{
			CCipher::Final(NULL, NULL);
		}
		m_pbCtx = (LPBYTE)pCCtx;
		return true;
	}

	bool CCipher::Update(LPBYTE pbIn, int nInLen, LPBYTE pbOut, int * pnOutLen)
	{
		if (m_pbCtx == NULL)
			return false;

		EVP_CIPHER_CTX * pCCtx = (EVP_CIPHER_CTX *)m_pbCtx;
		if (EVP_CipherUpdate(pCCtx, pbOut, pnOutLen, pbIn, nInLen) < 0)
			return false;
		return true;
	}

	bool CCipher::Final(LPBYTE pbOut, int * pnOutLen)
	{
		if (m_pbCtx == NULL)
			return false;

		EVP_CIPHER_CTX * pCCtx = (EVP_CIPHER_CTX *)m_pbCtx;
		if ((pbOut != NULL) && (pnOutLen != NULL))
		{
			if (EVP_CipherFinal(pCCtx, pbOut, pnOutLen) < 1)
				return false;
		}

		EVP_CIPHER_CTX_cleanup(pCCtx);
		
		EVP_CIPHER_CTX_free(pCCtx);
		m_pbCtx = NULL;
		return true;
	}

	// static
	int CCipher::Cipher(const char * pszAlgName, LPBYTE pbKey, LPBYTE pbIv, UINT32 un32Flags, LPBYTE pbIn, int nInLen, LPBYTE pbOut)
	{
		CCipher cipher;
		cipher.Init(pszAlgName, pbKey, 0, pbIv, (pbIv==NULL)? 0 : 16, un32Flags);
		int nOutLen1 = 0, nOutLen2 = 0;
		cipher.Update(pbIn, nInLen, pbOut, &nOutLen1);
		return cipher.Final(pbOut+nOutLen1, &nOutLen2)? (nOutLen1 + nOutLen2) : -1;
	}
	int CCipher::CipherWithKeyCode(const char * pszAlgName, LPBYTE pbIv, UINT32 un32Flags, LPBYTE pbIn, int nInLen, LPBYTE pbOut)
	{
		BYTE abKeyBuff[16];
		KeyCode::byteFiller(&abKeyBuff);
		return CCipher::Cipher(pszAlgName, abKeyBuff, pbIv, un32Flags, pbIn, nInLen, pbOut);
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////
	// RSA
	// static
	int CRsa::PrivateCrypt(LPBYTE pbDerKey, int nKeyLen, UINT32 un32Flags, LPBYTE pbInBuff, int nInLen, LPBYTE pbOutBuff)
	{
		int nRet = 0;
		__try
		{
			RSA * rsa = d2i_RSAPrivateKey(NULL, (const unsigned char **)&pbDerKey, nKeyLen);
			if (rsa != NULL)
			{
				int nPadding = (un32Flags & CRsa::NOPADDING)? RSA_NO_PADDING : RSA_PKCS1_PADDING;
				if (un32Flags & CRsa::ENCRYPT)
					nRet = RSA_private_encrypt(nInLen, pbInBuff, pbOutBuff, rsa, nPadding);
				else
					nRet = RSA_private_decrypt(nInLen, pbInBuff, pbOutBuff, rsa, nPadding);
				RSA_free(rsa);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			nRet = -1;
		}
		return nRet;
	}
	int CRsa::PublicCrypt(LPBYTE pbDerKey, int nKeyLen, UINT32 un32Flags, LPBYTE pbInBuff, int nInLen, LPBYTE pbOutBuff)
	{
		int nRet = 0;
		__try
		{
			RSA * rsa = d2i_RSAPublicKey(NULL, (const unsigned char **)&pbDerKey, nKeyLen);
			if (rsa != NULL)
			{
				int nPadding = (un32Flags & CRsa::NOPADDING)? RSA_NO_PADDING : RSA_PKCS1_PADDING;
				if (un32Flags & CRsa::ENCRYPT)
					nRet = RSA_public_encrypt(nInLen, pbInBuff, pbOutBuff, rsa, nPadding);
				else
					nRet = RSA_public_decrypt(nInLen, pbInBuff, pbOutBuff, rsa, nPadding);
				RSA_free(rsa);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			nRet = -1;
		}
		return nRet;
	}

	int CRsa::PublicCrypt(LPBYTE pbKeyN, int nKeyNLen, LPBYTE pbKeyE, int nKeyELen, UINT32 un32Flags, LPBYTE pbInBuff, int nInLen, LPBYTE pbOutBuff)
	{
		int nRet = 0;
		__try
		{
			RSA * rsa = RSA_new();
			if (rsa != NULL)
			{
				rsa->n = BN_bin2bn(pbKeyN, nKeyNLen, NULL);
				rsa->e = BN_bin2bn(pbKeyE, nKeyELen, NULL);

				int nPadding = (un32Flags & CRsa::NOPADDING)? RSA_NO_PADDING : RSA_PKCS1_PADDING;
				if (un32Flags & CRsa::ENCRYPT)
					nRet = RSA_public_encrypt(nInLen, pbInBuff, pbOutBuff, rsa, nPadding);
				else
					nRet = RSA_public_decrypt(nInLen, pbInBuff, pbOutBuff, rsa, nPadding);
				RSA_free(rsa);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			nRet = -1;
		}
		return nRet;
	}

	int CRsa::PublicCrypt(LPBYTE pbKeyN, int nKeyNLen, UINT32 un32KeyE, UINT32 un32Flags, LPBYTE pbInBuff, int nInLen, LPBYTE pbOutBuff)
	{
		BYTE abKeyE[4] = { (BYTE)(un32KeyE>>24), (BYTE)(un32KeyE>>16), (BYTE)(un32KeyE>>8), (BYTE)(un32KeyE) };
		return CRsa::PublicCrypt(pbKeyN, nKeyNLen, abKeyE, 4, un32Flags, pbInBuff, nInLen, pbOutBuff);
	}

	int CRsa::GetPubKeyN(LPBYTE pbDerKey, int nKeyLen, LPBYTE pbOutBuff)
	{
		int nRet = 0;
		__try
		{
			RSA * rsa = d2i_RSAPublicKey(NULL, (const unsigned char **)&pbDerKey, nKeyLen);
			if (rsa != NULL)
			{
				nRet = BN_bn2bin(rsa->n, pbOutBuff);
				RSA_free(rsa);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
			nRet = -1;
		}
		return nRet;
	}
	
#ifdef __AKEY_INTERFACE_H__
	static RSA_METHOD s_myRSAMeth;
	static int my_rsa_priv_crypt(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding, int flags)
	{
		AKey::IToken * pToken = NULL;
		UINT32 dwKeyID = 0;
		BN_bn2bin(rsa->d, (LPBYTE)(&pToken));
		BN_bn2bin(rsa->p, (LPBYTE)(&dwKeyID));

		HRESULT hr = 0;
		UINT32 dwOutLen = 0x100;
		if (flags & CRsa::ENCRYPT)
		{
			UINT32 dwSignFlags = AKEY_FLAG_SIGN_NOHASHOID;
			int offset = 4 + from[3];
			if (((2+from[1]) == flen) && (offset+2+from[offset+1] == flen))
			{
				flen = from[offset+1];
				from = from + offset + 2;
				dwSignFlags = 0;
			}
			dwSignFlags |= AKEY_FLAG_SIGN_HASH_FROM_LENGTH(flen);
			hr = pToken->Sign(dwKeyID, dwSignFlags, (LPBYTE)from, flen, 0, NULL, 0, to, &dwOutLen);
		}
		else
		{
			hr = pToken->Decrypt(dwKeyID, (LPBYTE)from, flen, to, &dwOutLen);
		}
		return (hr == AKEY_RV_OK)? dwOutLen : 0;
	}
	static int my_rsa_priv_enc(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
	{
		return my_rsa_priv_crypt(flen, from, to, rsa, padding, CRsa::ENCRYPT);
	}
	static int my_rsa_priv_dec(int flen, const unsigned char *from, unsigned char *to, RSA *rsa, int padding)
	{
		return my_rsa_priv_crypt(flen, from, to, rsa, padding, 0);
	}
	void * CRsa::RebuildTokenRSA(void * pRsa, AKey::IToken * pToken, UINT32 dwKeyID)
	{
		__try
		{
			memcpy(&s_myRSAMeth, RSA_get_method((RSA*)pRsa), sizeof(RSA_METHOD));
			s_myRSAMeth.rsa_priv_enc = my_rsa_priv_enc;
			s_myRSAMeth.rsa_priv_dec = my_rsa_priv_dec;
			RSA_set_method((RSA*)pRsa, &s_myRSAMeth);

			((RSA*)pRsa)->d = BN_bin2bn((LPBYTE)(&pToken), sizeof(void*), NULL);
			((RSA*)pRsa)->p = BN_bin2bn((LPBYTE)(&dwKeyID), sizeof(UINT32), NULL);
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
		return pRsa;
	}
#endif
	

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// Cert
	CCert::CCert() : m_cert(NULL)
	{
	}
	CCert::CCert(LPBYTE pbCert, int nCertLen)
	{
		m_cert = d2i_X509(NULL, (const unsigned char **)&pbCert, nCertLen);
	}
	
	CCert::~CCert()
	{
		if (m_cert != NULL)
		{
			X509_free((X509 *)m_cert);
		}
	}

	int CCert::GetPubKey(LPBYTE pbPubKey)
	{
		if (m_cert != NULL)
		{
#if 0		
			EVP_PKEY * pkey = X509_get_pubkey((X509 *)m_cert);
			if (pkey != NULL)
			{
				RSA * rsa = EVP_PKEY_get1_RSA(pkey);
				if (rsa != NULL)
				{
					return i2d_RSAPublicKey(rsa, (pbPubKey==NULL)? NULL : &pbPubKey);
				}
				// 部分android手机不支持（可能厂商把ec代码屏蔽了）
				EC_KEY *ec = EVP_PKEY_get1_EC_KEY(pkey);
				if (ec != NULL)
				{
					return i2o_ECPublicKey(ec, (pbPubKey==NULL)? NULL : &pbPubKey);
				}
			}
#else
			X509_PUBKEY * pub = ((X509 *)m_cert)->cert_info->key;
			if (pub != NULL)
			{
				//X509_PUBKEY_get0_param(NULL, &pbPubKey, &pklen, NULL, key->public_key);
				memcpy(pbPubKey, pub->public_key->data, pub->public_key->length);
				return pub->public_key->length;
			}
#endif
		}
		return -1;
	}
	
	int CCert::GetSubjectCN(LPBYTE pbCN)
	{
		if (m_cert != NULL)
		{
			X509_NAME *pX509Subject = X509_get_subject_name((X509 *)m_cert);
			if (pX509Subject != NULL)
			{
				return X509_NAME_get_text_by_NID(pX509Subject, NID_commonName, (char *)pbCN, 0x100); 
			}
		}
		return -1;
	}

	// static
	int CCert::GetPubKey(LPBYTE pbCert, int nCertLen, LPBYTE pbPubKey)
	{
		CCert cert(pbCert, nCertLen);
		return cert.GetPubKey(pbPubKey);
	}
	int CCert::GetSubjectCN(LPBYTE pbCert, int nCertLen, LPBYTE pbCN)
	{
		CCert cert(pbCert, nCertLen);
		return cert.GetSubjectCN(pbCN);
	}	
	int CCert::GetCN(LPBYTE pbCert, int nCertLen, LPBYTE pbCN)
	{
		CCert cert(pbCert, nCertLen);
		return cert.GetSubjectCN(pbCN);
	}
		
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// PKCS7
	CPkcs7::CPkcs7(CPkcs7::PKCS7TYPE type, int nFlags) : m_nFlags(nFlags), m_nSigStructSize(0)
	{
		m_p7 = (nFlags & AKEY_FLAG_P7_SM)? SM_PKCS7_new() : PKCS7_new(); // 构建PKCS对象
		if (m_p7 != NULL)
		{
			if (type == CPkcs7::DATA)
				PKCS7_set_type((PKCS7 *)m_p7, NID_pkcs7_data); 
			else if (type == CPkcs7::SIGNED)
				PKCS7_set_type((PKCS7 *)m_p7, NID_pkcs7_signed); 
			else if (type == CPkcs7::SIGNEDANDENVELOPED)
				PKCS7_set_type((PKCS7 *)m_p7, NID_pkcs7_signedAndEnveloped);

#if (AKEY_SUPPORT_SM_PKCS7)
			if ((nFlags & AKEY_FLAG_P7_SM) && (!(nFlags & AKEY_FLAG_P7_ORG_OID))) // 更改为国密的OID，必须在PKCS7_set_type之后修改
			{
				if (type == CPkcs7::DATA)
					((PKCS7 *)m_p7)->type = OBJ_nid2obj(NID_sm_pkcs7_data);
				else if (type == CPkcs7::SIGNED)
					((PKCS7 *)m_p7)->type = OBJ_nid2obj(NID_sm_pkcs7_signed);
				else if (type == CPkcs7::SIGNEDANDENVELOPED)
					((PKCS7 *)m_p7)->type = OBJ_nid2obj(NID_sm_pkcs7_signedAndEnveloped);
			}
#endif
		}
	}
	
	CPkcs7::~CPkcs7()
	{
		if (m_p7 != NULL)
		{
			(m_nFlags & AKEY_FLAG_P7_SM)? SM_PKCS7_free((SM_PKCS7 *)m_p7) : PKCS7_free((PKCS7 *)m_p7); //释放P7对象
		}
	}

	// 摘要类型
	bool CPkcs7::SetSignMdAlg(const char * pszHashName)
	{
		PKCS7 * p7 = (PKCS7 *)m_p7;
		if (p7 == NULL)
			return false;

		X509_ALGOR * x509Algor = X509_ALGOR_new(); // 构建算法对象
		
		x509Algor->algorithm = OBJ_nid2obj(  OBJ_sn2nid(pszHashName) );// 摘要类型		
		x509Algor->parameter = ASN1_TYPE_new(); 
		if (x509Algor->parameter == NULL)
		{
			X509_ALGOR_free(x509Algor);
			return false;
		}		
		x509Algor->parameter->type = V_ASN1_NULL;
		
		sk_X509_ALGOR_push(p7->d.sign->md_algs, x509Algor); // 压栈：签名数据的摘要类型
		return true;
	}

	// 数据
	bool CPkcs7::SetData(LPBYTE pbPlainData, int nPlainDataLen)
	{
		PKCS7 * p7 = (PKCS7 *)m_p7;
		if (p7 == NULL)
			return false;

		// 如果接口中传入签名原文则为Attched签名，否则为Deteched签名
		if ((pbPlainData != NULL) && (nPlainDataLen > 0))
		{
			ASN1_OCTET_STRING_set(p7->d.data,(const unsigned char *)pbPlainData, nPlainDataLen);
		}
		return true;
	}
	
	bool CPkcs7::SetSignContent(LPBYTE pbPlainData, int nPlainDataLen, int nFlags)
	{
		CPkcs7 dataP7(CPkcs7::DATA, nFlags);

		if (!dataP7.SetData(pbPlainData, nPlainDataLen))
			return false;

		PKCS7 * pSignP7 = (PKCS7 *)m_p7;
		PKCS7 * pDataP7 = (PKCS7 *)dataP7.Detach();
		//pSignP7->state = PKCS7_S_TAIL;  // 签名数据 为 尾节点
		//pDataP7->state = PKCS7_S_HEADER; // 头节点

		// PKCS7_set_content(pSignP7, pDataP7); // 根据type判断， 同以下3行代码		
		if (pSignP7->d.sign->contents != NULL) // 可改进：若是SM的P7，则调用SM_PKCS7_free； 首次构造P7其类型为UNDEF
			PKCS7_free(pSignP7->d.sign->contents);			
		pSignP7->d.sign->contents = pDataP7;

		// PKCS7_set_detached(pSignP7, (nPlainDataLen>0)? 0 : 1); // Attatch - P7 - 带签名原文的P7 // 内部判断type不可用
		if (nPlainDataLen <= 0) // detached
		{
			pSignP7->detached = 1;
			ASN1_OCTET_STRING_free(pDataP7->d.data);
			pDataP7->d.data = NULL;
		}
							
		return true;
	}

	// 证书信息
	bool CPkcs7::SetSignCert(LPBYTE pbCert, int nCertLen)
	{
		PKCS7 * p7 = (PKCS7 *)m_p7;
		if (p7 == NULL)
			return false;

		if (p7->d.sign->cert == NULL)
		{
			p7->d.sign->cert = sk_X509_new_null(); // 构建证书输入对象
			if (p7->d.sign->cert == NULL)
				return false;
		}

		X509 * x509Cert = d2i_X509(NULL, (const unsigned char **)&pbCert, nCertLen);
		if (x509Cert == NULL)
			return false;

		sk_X509_push( p7->d.sign->cert, x509Cert ); // 压栈：P7-签名数据-证书信息
		return true;
	}

	static int MakeTLV(BYTE btTag, LPBYTE pbData, int nLength, LPBYTE pbOutput)
	{
		int pos = 2;
		if (pbData[0] > 0x80)
		{
			pbOutput[pos++] = 0x00;
		}
		pbOutput[0] = btTag;
		pbOutput[1] = (BYTE)(pos-2+nLength);
		memcpy(pbOutput+pos, pbData, nLength);
		return pos+nLength;
	}
	
	// 签名者信息
	bool CPkcs7::SetSignerInfo(const char * pszHashName, LPBYTE pbSignerCert, int nSignerCertLen, LPBYTE pbSignature, int nSignatureLen)
	{
		PKCS7 * p7 = (PKCS7 *)m_p7;
		if (p7 == NULL)
			return false;

		// 签名证书信息
		CCert signerCert(pbSignerCert, nSignerCertLen);
		X509 * x509Cert = (X509 *)signerCert.GetObject();
		if (x509Cert == NULL)
			return false;

		PKCS7_SIGNER_INFO * pSignerInfo = PKCS7_SIGNER_INFO_new();// 构建签名者信息对象
		if (pSignerInfo == NULL)
			return false;
		
		ASN1_INTEGER_set(pSignerInfo->version, 1); // 设置签名者版本
		
		ASN1_INTEGER * pSerialNumber = X509_get_serialNumber( x509Cert );
		if (pSerialNumber == NULL)
			return false;
		if (pSignerInfo->issuer_and_serial->serial != NULL)
			M_ASN1_INTEGER_free(pSignerInfo->issuer_and_serial->serial);
		pSignerInfo->issuer_and_serial->serial = M_ASN1_INTEGER_dup( pSerialNumber ); // 设置颁发者序列号
		
		X509_NAME * pX509Name = X509_get_issuer_name( x509Cert );
		if (pX509Name == NULL)
			return false;	
		X509_NAME_set(&pSignerInfo->issuer_and_serial->issuer, pX509Name ); // 证书颁发者名称
				
		pSignerInfo->digest_alg->algorithm = OBJ_nid2obj( OBJ_sn2nid(pszHashName) ); // 摘要算法
		if (pSignerInfo->digest_alg->algorithm == NULL)
			return false;
		
		pSignerInfo->digest_alg->parameter = ASN1_TYPE_new();
		if (pSignerInfo->digest_alg->parameter == NULL)
			return false;
		pSignerInfo->digest_alg->parameter->type = V_ASN1_NULL;
				
		int digest_enc_alg_nid = NID_rsaEncryption;
		if (m_nFlags & AKEY_FLAG_P7_SM)
		{
			LPCSTR pszAlgName = (m_nFlags & AKEY_FLAG_P7_ABC)? "SM2-SM3" : "SM2";
			digest_enc_alg_nid = OBJ_sn2nid(pszAlgName);
		}
		pSignerInfo->digest_enc_alg->algorithm = OBJ_nid2obj( digest_enc_alg_nid ); // NID_rsaEncryption // 摘要加密算法为: RSA-私钥-签名 - PKCS1签名
		if (pSignerInfo->digest_enc_alg->algorithm == NULL)
			return false;
		pSignerInfo->digest_enc_alg->parameter = ASN1_TYPE_new();
		if (pSignerInfo->digest_enc_alg->parameter == NULL)
			return false;
		pSignerInfo->digest_enc_alg->parameter->type = V_ASN1_NULL;
		
		// SM3-with-SM2
		if ((digest_enc_alg_nid != NID_rsaEncryption) && (nSignatureLen == 0x40))
		{
			BYTE abSM2SignDer[0x48];			

			int nLen_t = MakeTLV(0x02, pbSignature, 0x20, abSM2SignDer+2); // r
			nLen_t += MakeTLV(0x02, pbSignature+0x20, 0x20, abSM2SignDer+2+nLen_t); // s
			
			if (m_nFlags & AKEY_FLAG_P7_ABC)
			{
				m_nSigStructSize = nLen_t;
				ASN1_STRING_set(pSignerInfo->enc_digest, abSM2SignDer+2, nLen_t); // 设置签名结果
			}
			else
			{
				abSM2SignDer[0] = 0x30;
				abSM2SignDer[1] = (BYTE)nLen_t;
				m_nSigStructSize = 2+nLen_t;
				ASN1_STRING_set(pSignerInfo->enc_digest, abSM2SignDer, 2+nLen_t); // 设置签名结果
			}
		}
		else
		{
			ASN1_STRING_set(pSignerInfo->enc_digest, pbSignature, nSignatureLen); // 设置签名结果
		}
		
		sk_PKCS7_SIGNER_INFO_push(p7->d.sign->signer_info, pSignerInfo);// 压栈：签名者信息
		return true;
	}

	bool CPkcs7::MakeSignedData(const char * pszHashName, LPBYTE pbPlainData, int nPlainDataLen, LPBYTE pbSignerCert, int nSignerCertLen, LPBYTE pbSignature, int nSignatureLen)
	{
		if (!CPkcs7::SetSignMdAlg(pszHashName))
			return false;

		if (!CPkcs7::SetSignContent(pbPlainData, nPlainDataLen, m_nFlags))
			return false;

		if (!CPkcs7::SetSignCert(pbSignerCert, nSignerCertLen))
			return false;

		if (!CPkcs7::SetSignerInfo(pszHashName, pbSignerCert, nSignerCertLen, pbSignature, nSignatureLen))
			return false;

		return true;
	}

	bool CPkcs7::GetBytes(LPBYTE pbResult, int * pnResult)
	{
		PKCS7 * p7 = (PKCS7 *)m_p7;
		if (p7 == NULL)
			return false;

#if (AKEY_SUPPORT_SM_PKCS7)
		if ((m_nFlags & AKEY_FLAG_P7_SM) && (!(m_nFlags & AKEY_FLAG_P7_ORG_OID))) 
		{
			* pnResult = i2d_SM_PKCS7(p7, (pbResult==NULL)? NULL : &pbResult); // 获取PKCS7数据长度
		}
		else
#endif
		{
			* pnResult = i2d_PKCS7(p7, (pbResult==NULL)? NULL : &pbResult); // 获取PKCS7数据长度
		}
		
		if ((pbResult != NULL) && (m_nFlags & AKEY_FLAG_P7_SM) && (m_nFlags & AKEY_FLAG_P7_ABC) && ((*pnResult) > (2+m_nSigStructSize)))
		{
			// 替换签名结果的数据类型04-->0x30
			pbResult[-(2+m_nSigStructSize)] = 0x30;
		}
		return (* pnResult > 0);
	}

#ifdef __AKEY_BUFFER_H__
	bool CPkcs7::GetBytes(AKey::CBuffer & resultBuff)
	{
		int nLength = 0;
		if (!GetBytes(NULL, &nLength))
		{
			return false;
		}

		bool bResult = GetBytes(resultBuff.ReAllocBytes(nLength), &nLength);
		resultBuff.SetLength(nLength);
		return bResult;
	}

	// static
	bool CPkcs7::MakeSignedData(int nFlags, LPBYTE pbPlainData, int nPlainDataLen, LPBYTE pbSignerCert, int nSignerCertLen, LPBYTE pbSignature, int nSignatureLen, AKey::CBuffer & resultBuff)
	{
		UINT32 dwHashType = nFlags & AKEY_FLAG_P7_HASH_MASK;
		if ((nSignatureLen < 0x80) && (dwHashType == AKEY_HASH_SM3))
		{
			nFlags |= AKEY_FLAG_P7_SM; // 国密
		}
	
		CPkcs7 signP7(CPkcs7::SIGNED, nFlags);
		if (!signP7.MakeSignedData(AKey::Helper::HashType2Name(dwHashType), pbPlainData, nPlainDataLen, pbSignerCert, nSignerCertLen, pbSignature, nSignatureLen))
			return false;
	
		return signP7.GetBytes(resultBuff);
	}
#endif


	//class CPkcs5
	bool CPkcs5::PasswordToKey(const char *pass, int passlen,
			   const unsigned char *salt, int saltlen, int iter,
			   const char * pszHashName,
			   int keylen, unsigned char *out)
	{
		unsigned char digtmp[EVP_MAX_MD_SIZE], *p, itmp[4];
		int cplen, j, k, tkeylen, mdlen;
		unsigned long i = 1;
		HMAC_CTX hctx;
		const EVP_MD * digest = EVP_get_digestbyname(pszHashName);
		if (digest == NULL)
			return 0;

		mdlen = EVP_MD_size(digest);
		if (mdlen < 0)
			return 0;

		HMAC_CTX_init(&hctx);
		p = out;
		tkeylen = keylen;
		while(tkeylen)
		{
			if(tkeylen > mdlen)
				cplen = mdlen;
			else
				cplen = tkeylen;
			/* We are unlikely to ever use more than 256 blocks (5120 bits!)
			* but just in case...
			*/
			itmp[0] = (unsigned char)((i >> 24) & 0xff);
			itmp[1] = (unsigned char)((i >> 16) & 0xff);
			itmp[2] = (unsigned char)((i >> 8) & 0xff);
			itmp[3] = (unsigned char)(i & 0xff);
			HMAC_Init_ex(&hctx, pass, passlen, digest, NULL);
			HMAC_Update(&hctx, salt, saltlen);
			HMAC_Update(&hctx, itmp, 4);
			HMAC_Final(&hctx, digtmp, NULL);
			memcpy(p, digtmp, cplen);
			for(j = 1; j < iter; j++)
			{
				HMAC(digest, pass, passlen,
					digtmp, mdlen, digtmp, NULL);
				for(k = 0; k < cplen; k++)
					p[k] ^= digtmp[k];
			}
			tkeylen-= cplen;
			i++;
			p+= cplen;
		}
		HMAC_CTX_cleanup(&hctx);
		return 1;
	};
};
