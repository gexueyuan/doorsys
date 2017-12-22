#ifndef __AKEY_OPENALG_H__
#define __AKEY_OPENALG_H__
// 不要在此处包含其它头文件，便于拷贝本文件代码给其它工程使用

namespace OpenAlg
{
	// [base]
	class CRC
	{
	public:
		static unsigned int DoCRC16(LPBYTE pbData, int nLength, unsigned int nSeed);
	};

	// [base]
	class ECC
	{
	public:
		static int Encode(LPBYTE pbData, int nLength);
		static int Decode(LPBYTE pbData, int nLength);

	private:
		static BYTE edac_en(LPBYTE pbData);
		static bool edac_de(LPBYTE pbData, BYTE chkData);
	};
	
	class CBase64
	{
	public:
		static int Encode(LPBYTE pbData, int nDataLen, LPBYTE pbOut);
		static int Decode(LPBYTE pbData, int nDataLen, LPBYTE pbOut);

		// [base]
#ifdef __AKEY_BUFFER_H__
		static int Encode(LPBYTE pbData, int nDataLen, AKey::CBuffer & resultBuff);
		static int Decode(LPBYTE pbData, int nDataLen, AKey::CBuffer & resultBuff);
#endif
	};

	class CRand
	{
	public:
		static int GetBytes(LPBYTE pbSeed, int nSeedLen, LPBYTE pbRand, int nRandLen);
	};


	// [base]
	class CDigest
	{
	public:
		CDigest();
		~CDigest();

		bool Init(const char * pszAlgName);
		bool Update(LPBYTE pbData, int nLength);
		bool Final(LPBYTE pbOut, unsigned int * pnOutLen);
		bool GetBlockSize(int * pnBlockSize, int * pnInternalStateLen); // 只能在init后update前调用有效
		bool GetInternalData(int nBlockSize, int nInternalStateLen, LPBYTE pbOut);
		
		// [base]
		static void HashReverse(LPBYTE pbInBuff, int nInLength, int nBlockSize, LPBYTE pbOutBuff);
		static int Digest(const char * pszAlgName, LPBYTE pbData, int nLength, LPBYTE pbOut);
		static int GetInternalData(const char * pszAlgName, LPBYTE pbData, int nLength, LPBYTE pbOut);
		static int GetInternalData_SM3(LPBYTE pbCert, int nCerLen, LPBYTE pbData, int nLength, LPBYTE pbInternalResult, LPBYTE pbMdResult);
		static int HMAC(const char * pszAlgName, LPBYTE pbKey, int nKeyLen, LPBYTE pbData, int nDataLen, LPBYTE pbOut);

	protected:
		void * m_pCtx;
	};

	class CCipher
	{
	public:
		enum FLAG
		{
			ENCRYPT = 0x00000001,
			NOPADDING = 0x00010000
		};

		CCipher();
		~CCipher();

		bool Init(const char * pszAlgName, LPBYTE pbKey, int nKeyLen, LPBYTE pbIv, int nIvLen, UINT32 un32Flags);
		bool Update(LPBYTE pbIn, int nInLen, LPBYTE pbOut, int * pnOutLen);
		bool Final(LPBYTE pbOut, int * pnOutLen);

		static int Cipher(const char * pszAlgName, LPBYTE pbKey, LPBYTE pbIv, UINT32 un32Flags, LPBYTE pbIn, int nInLen, LPBYTE pbOut);
		// [base]
		static int CipherWithKeyCode(const char * pszAlgName, LPBYTE pbIv, UINT32 un32Flags, LPBYTE pbIn, int nInLen, LPBYTE pbOut);

	protected:
		LPBYTE m_pbCtx;
	};

	class CRsa
	{
	public:
		enum FLAG
		{
			ENCRYPT = 0x00000001,
			NOPADDING = 0x00010000
		};

		// 可选
		static int PrivateCrypt(LPBYTE pbDerKey, int nKeyLen, UINT32 un32Flags, LPBYTE pbInBuff, int nInLen, LPBYTE pbOutBuff);
		// 
		static int PublicCrypt(LPBYTE pbDerKey, int nKeyLen, UINT32 un32Flags, LPBYTE pbInBuff, int nInLen, LPBYTE pbOutBuff);
		// [base]
		static int PublicCrypt(LPBYTE pbKeyN, int nKeyNLen, LPBYTE pbKeyE, int nKeyELen, UINT32 un32Flags, LPBYTE pbInBuff, int nInLen, LPBYTE pbOutBuff);
		static int PublicCrypt(LPBYTE pbKeyN, int nKeyNLen, UINT32 un32KeyE, UINT32 un32Flags, LPBYTE pbInBuff, int nInLen, LPBYTE pbOutBuff);

		//static int GetPubKeyN(LPBYTE pbDerKey, int nKeyLen, LPBYTE pbOutBuff);

		// 扩展
#ifdef __AKEY_INTERFACE_H__
		static void * RebuildTokenRSA(void * pRsa, AKey::IToken * pToken, UINT32 dwKeyID);
#endif
	};	


	class CCert
	{
	public:
		CCert();
		CCert(LPBYTE pbCert, int nCertLen);
		~CCert();

		void * GetObject() { return m_cert; };
		void * Detach() // Attach
		{
			void * cert = m_cert;
			m_cert = NULL;
			return cert;
		}

		int GetPubKey(LPBYTE pbPubKey); // der
		int GetSubjectCN(LPBYTE pbCN);
		int GetSerialNumber(LPBYTE pbSerialNumber); // der
		int GetIssuerName(LPBYTE pbIssuerName); // der

		static int FindPubKey(LPBYTE pbCert, int nCertLen, int * pnPubKeyLen);
		static int GetPubKey(LPBYTE pbCert, int nCertLen, LPBYTE pbPubKey);
		static int GetSubjectCN(LPBYTE pbCert, int nCertLen, LPBYTE pbCN);
		static int GetSerialNumber(LPBYTE pbCert, int nCertLen, LPBYTE pbSerialNumber);
		static int GetIssuerName(LPBYTE pbCert, int nCertLen, LPBYTE pbIssuerName);
	protected:
		void * m_cert;
	};

	// [base]
	class CPkcs10
	{
	public:
		static int FindPubKey(LPBYTE pbP10, int nP10Len, int * pnPubKeyLen);
	};
	
	// [base]
	class CPkcs7
	{
	public:
		enum PKCS7TYPE { DATA, SIGNED, SIGNEDANDENVELOPED };

		CPkcs7(CPkcs7::PKCS7TYPE type, int nFlags);
		~CPkcs7();

		void * GetObject() { return m_p7; };
		void * Detach() // Attach
		{
			void * p7 = m_p7;
			m_p7 = NULL;
			return p7;
		}

		bool SetSignMdAlg(UINT32 hashType);
		bool SetData(LPBYTE pbPlainData, int nPlainDataLen);
		bool SetSignContent(LPBYTE pbPlainData, int nPlainDataLen, int nFlags);
		bool SetSignCert(LPBYTE pbCert, int nCertLen);
		bool SetSignerInfo(UINT32 hashType, LPBYTE pbSignerCert, int nSignerCertLen, LPBYTE pbSignature, int nSignatureLen);

		bool MakeSignedData(UINT32 hashType, LPBYTE pbPlainData, int nPlainDataLen, LPBYTE pbSignerCert, int nSignerCertLen, LPBYTE pbSignature, int nSignatureLen);
		bool GetBytes(LPBYTE pbResult, int * pnResult);

#ifdef __AKEY_BUFFER_H__
		bool GetBytes(AKey::CBuffer & resultBuff);
#endif
		// static
		static bool MakeSignedData(int nFlags, LPBYTE pbPlainData, int nPlainDataLen, LPBYTE pbSignerCert, int nSignerCertLen, LPBYTE pbSignature, int nSignatureLen, LPBYTE pbOutBuff, int * pnOutLen);
#ifdef __AKEY_BUFFER_H__
		static bool MakeSignedData(int nFlags, LPBYTE pbPlainData, int nPlainDataLen, LPBYTE pbSignerCert, int nSignerCertLen, LPBYTE pbSignature, int nSignatureLen, AKey::CBuffer & resultBuff);
#endif
		static int FindCert(LPBYTE pbP7, int nP7Len, LPBYTE pbPubKey, int nPubKeyLen, int * pnCertLen);

	protected:
		void * m_p7;
		int m_nFlags, m_nSigStructSize;
	};

	class CPkcs5
	{
	public:
		static bool PasswordToKey(const char *pass, int passlen,
			   const unsigned char *salt, int saltlen, int iter,
			   int keylen, unsigned char *out);
	};
};

#endif
