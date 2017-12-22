// 不要在此处包含其它头文件，便于拷贝本文件代码给其它工程使用
#include "AKeyBase.h"
#include "OpenAlg.h"

#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/hmac.h>

#ifndef AKEY_SUPPORT_OPENSSL_MIN
#define AKEY_SUPPORT_OPENSSL_MIN 0 // mini版算法库
#endif
#ifndef AKEY_SUPPORT_OPENSSL_TDR
#define AKEY_SUPPORT_OPENSSL_TDR 0	// 默认不支持自定义的openssl算法库
#endif

#if !(AKEY_SUPPORT_OPENSSL_MIN)
#if (AKEY_SUPPORT_OPENSSL_TDR)
#include <tdrssl/sm_evp.h>
#else
#include <openssl/ec.h>
#include "alg/dgst/sm3.c"
#include "sm4.inc"
#endif
#endif

//自动载入动态库
#ifdef WIN32
# if (AKEY_SUPPORT_OPENSSL_MIN)
#pragma comment(lib, "mcrypto.lib")
# else
#pragma comment(lib, "libeay32.lib")
#  if (AKEY_SUPPORT_OPENSSL_TDR)
#   pragma comment(lib, "tdreay32.lib")
#  endif
# endif
#pragma message("Automatically linking with libeay32.lib")
#else
#define __try	// Android没有支持
#define __except(e) if(0)
#endif



namespace OpenAlg
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class CAutoCleanup
	class CAutoCleanup
	{
	public:
		CAutoCleanup()
		{
#if !AKEY_SUPPORT_OPENSSL_MIN
#if 0
			OpenSSL_add_all_algorithms();
#else
			//OPENSSL_cpuid_setup();
			
#ifndef OPENSSL_NO_DES
			EVP_add_cipher(EVP_des_cbc());
			EVP_add_cipher(EVP_des_ede_cbc());
			EVP_add_cipher(EVP_des_ede3_cbc());

			EVP_add_cipher(EVP_des_ecb());
			EVP_add_cipher(EVP_des_ede());
			EVP_add_cipher(EVP_des_ede3());
#endif


#ifndef OPENSSL_NO_AES
			EVP_add_cipher(EVP_aes_128_ecb());
			EVP_add_cipher(EVP_aes_128_cbc());
			//EVP_add_cipher(EVP_aes_192_ecb());
			//EVP_add_cipher(EVP_aes_192_cbc());
			//EVP_add_cipher(EVP_aes_256_ecb());
			//EVP_add_cipher(EVP_aes_256_cbc());
#endif
#if (!OPENALG_SUPPORT_DIGEST)
			EVP_add_digest(EVP_md5());
			EVP_add_digest(EVP_sha1());
			EVP_add_digest(EVP_sha256());
			EVP_add_digest(EVP_sha384());
			EVP_add_digest(EVP_sha512());
#endif								
			//ENGINE_setup_bsd_cryptodev();			
#endif

			OpenSSL_add_sm3_digest();
			OpenSSL_add_sm4_cipher();
#endif
		}
		
		~CAutoCleanup()
		{
#if !AKEY_SUPPORT_OPENSSL_MIN
			EVP_cleanup();
			OBJ_cleanup();
			// 调用OpenSSL的crypto库，在退出前需要调用API "CRYPTO_cleanup_all_ex_data"，清除管理CRYPTO_EX_DATA的全局hash表中的数据，避免内存泄漏。
			CRYPTO_cleanup_all_ex_data(); 
#endif
		}
	};	
	static CAutoCleanup s_autoCleanup;



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
	

#if (!OPENALG_SUPPORT_DIGEST)
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

#endif // #if (!OPENALG_SUPPORT_DIGEST)

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
};
