#ifndef __AKEY_HELPER_H__
#define __AKEY_HELPER_H__

#include "AKeyDef.h"
#include "AKeyError.h"
#include "AKeyInterface.h"


namespace AKey
{
	namespace Helper
	{
		// 判断系统是否为低字节序
		static bool IsLittleEndian()
		{
			union T
			{
				unsigned short s;
				unsigned char c[2];
			};

			static union T t = {1};
			return (t.c[0] == 1);
		}

		static UINT32 Min(UINT32 a, UINT32 b)
		{
			return (a<b)? a : b;
		}
		static UINT32 Max(UINT32 a, UINT32 b)
		{
			return (a>b)? a : b;
		}

		// 获取摘要名称
		static const char * HashType2Name(UINT32 dwType)
		{
#ifdef AKEY_SUPPORT_LOWERCASE_HASHNAME
			const char * names[] = {"sha1", "md5", "sha256", "sha384", "sha512", "md5sha1", "sm3", "sha256sm3"};
#else
			const char * names[] = {"SHA1", "MD5", "SHA256", "SHA384", "SHA512", "MD5SHA1", "SM3", "SHA256SM3"};
#endif
			return (dwType < (sizeof(names)/sizeof(const char*)))? names[dwType] : names[0];
		}

		// 字节反转
		static UINT16 ReverseUInt16(UINT16 nValue)
		{
			return ((nValue & 0x00FF) << 8) | ((nValue & 0xFF00) >> 8);
		}
		static UINT32 ReverseUInt32(UINT32 nValue)
		{
			return ((nValue & 0x000000FF) << 24) | ((nValue & 0x0000FF00) << 8) | ((nValue & 0x00FF0000) >> 8) | ((nValue & 0xFF000000) >> 24);
		}



		//! 低地址存放最低有效字节
		class LittleEndian
		{
		public:
			static void UInt16ToBytes(UINT16 unNum, LPBYTE pbBuff)
			{
				pbBuff[0] = (BYTE)(unNum);
				pbBuff[1] = (BYTE)(unNum >> 8);
			}

			static UINT16 UInt16FromBytes(LPBYTE pbBuff)
			{
				return (pbBuff[0]) + (pbBuff[1] << 8);
			}


			static void UInt32ToBytes(UINT32 unNum, LPBYTE pbBuff)
			{
				pbBuff[0] = (BYTE)(unNum);
				pbBuff[1] = (BYTE)(unNum >> 8);
				pbBuff[2] = (BYTE)(unNum >> 16);
				pbBuff[3] = (BYTE)(unNum >> 24);
			}

			static UINT32 UInt32FromBytes(LPBYTE pbBuff)
			{
				return (pbBuff[0]) + (pbBuff[1] << 8) + (pbBuff[2] << 16) + (pbBuff[3] << 24);
			}

		};

		//! 低地址存放最高有效字节
		class BigEndian
		{
		public:
			static void UInt16ToBytes(UINT16 unNum, LPBYTE pbBuff)
			{
				pbBuff[0] = (BYTE)(unNum >> 8);
				pbBuff[1] = (BYTE)(unNum);
			}

			static UINT16 UInt16FromBytes(LPBYTE pbBuff)
			{
				return (pbBuff[0] << 8) + (pbBuff[1]);
			}


			static void UInt32ToBytes(UINT32 unNum, LPBYTE pbBuff)
			{
				pbBuff[0] = (BYTE)(unNum >> 24);
				pbBuff[1] = (BYTE)(unNum >> 16);
				pbBuff[2] = (BYTE)(unNum >> 8);
				pbBuff[3] = (BYTE)(unNum);
			}

			static UINT32 UInt32FromBytes(LPBYTE pbBuff)
			{
				return (pbBuff[0] << 24) + (pbBuff[1] << 16) + (pbBuff[2] << 8) + (pbBuff[3]);
			}
		};

		class Convert
		{
		public:
			static INT32 StringToInt(const char * pszSrc, int base = 10)
			{
				bool bNegative =  (*pszSrc == '-')? (pszSrc ++, true) : (false);
				INT32 n = 0;
				for (; ;)
				{
					int offset = CharToInt(*(pszSrc++));
					if (offset < 0 || !(offset <  base))
						break;
					n = n * base + offset;
				}

				return (bNegative)? (0-n) : (n);
			}

			static int StringFromInt(char * pszDest, INT32 n, int length,int base = 10)
			{
				if (n < 0)
				{
					*(pszDest++) = '-';
					n = 0-n;
				}
				char * pszStart = pszDest;
				do{
					*(pszDest++) = CharFromInt( static_cast<int>(n % base) );
					n = n / base;
				}while(n);
				
				if (length <= 0)
					length = (int)(pszDest - pszStart);
				else
				{
					for (; pszDest < (pszStart + length); )
						*(pszDest++) = '0';
				}

				// StringReverse
				for (char * p= pszStart + length - 1, *q=pszStart; q < p; q++,p--)
				{
					char t = *q;
					*q = *p;
					*p = t;
				}
				return ( length );
			}

			static int BytesToHexString(LPBYTE pbBytes, int nLength, LPBYTE pbDest)
			{
				if (pbDest == NULL)
					return nLength << 1;

				for (int i=0; i<(nLength<<1); i++)
				{
					*(pbDest++) = (BYTE)CharFromInt( (i&1)? ((pbBytes[i>>1]) & 0x0F) : ((pbBytes[i>>1]) >> 4) );
				}
				return ( nLength<<1 );
			}

			static int HexStringToBytes(LPBYTE pbSrc, int nLength, LPBYTE pbBytes)
			{
				if (pbBytes == NULL)
					return ((nLength + 1) >> 1);

				for (int i=0; i<nLength; i++)
				{
					int c = CharToInt(pbSrc[i]);
					if (c < 0 || c > 16)
					{
						c = 0; // return -1;
					}
					if (i & 1)
					{
						*pbBytes += (BYTE)c;
						pbBytes ++;
					}
					else
					{
						*pbBytes = (BYTE)(c << 4);
					}
				}
				return ((nLength + 1) >> 1);
			}

		protected:
			//! 返回一个字符的进制数，不在36进制内返回-1
			static int CharToInt(int ch)
			{
				if (ch >= '0' && ch <= '9')
					return ch - '0';
				else if (ch >= 'A' && ch <= 'Z')
					return ch - 'A' + 10;
				else if (ch >= 'a' && ch <= 'z')
					return ch - 'a' + 10;
				else
					return -1;
			}
			//! 返回进制数的字符，不在36内返回0
			static int CharFromInt(int ch)
			{
				if (ch >=0 && ch <= 9)
					return ch + '0';
				else if (ch >= 10 && ch<=36)
					return ch - 10 + 'A';
				else
					return 0;
			}

		};

	}; // helper

#define __AKEY_BUFFER_H__
	class CBuffer
	{
	public:
		CBuffer(UINT32 dwAllocLength = 0) : m_dwAllocLength(dwAllocLength), m_dwLength(0) 
		{
			m_pbBuffPtr = (dwAllocLength>0)? (new BYTE[dwAllocLength]) : NULL;
		}
		CBuffer(LPBYTE pbData, UINT32 dwLength) : m_dwAllocLength(dwLength), m_dwLength(dwLength) 
		{
			m_pbBuffPtr = new BYTE[dwLength];
			if (m_pbBuffPtr != NULL)
			{
				memcpy(m_pbBuffPtr, pbData, dwLength);
			}
		}

		virtual ~CBuffer()
		{
			if (m_pbBuffPtr != NULL)
			{
				delete [] m_pbBuffPtr;
			}
		}

		LPBYTE Allocate(UINT32 dwNewAllocLength)
		{
			if (m_dwAllocLength < dwNewAllocLength)
			{
				if (m_pbBuffPtr != NULL)
				{
					delete [] m_pbBuffPtr;
				}
				m_pbBuffPtr = new BYTE[dwNewAllocLength];
				m_dwAllocLength = dwNewAllocLength;
			}
			m_dwLength = 0;
			return m_pbBuffPtr;
		}

		LPBYTE ReAllocBytes(UINT32 dwNewAllocLength)
		{
			if (m_dwAllocLength < dwNewAllocLength)
			{
				LPBYTE pbBuffPtr = new BYTE[dwNewAllocLength];
				if (pbBuffPtr != NULL)
				{
					if (m_pbBuffPtr != NULL)
					{
						if (m_dwLength > dwNewAllocLength)
							m_dwLength = dwNewAllocLength;
						memcpy(pbBuffPtr, m_pbBuffPtr, m_dwLength);
						delete [] m_pbBuffPtr;
					}
					m_pbBuffPtr = pbBuffPtr;
					m_dwAllocLength = dwNewAllocLength;
				}
			}
			return m_pbBuffPtr;
		}

		LPBYTE ReAllocBytesSetLength(UINT32 dwNewAllocLength)
		{
			if (ReAllocBytes(dwNewAllocLength) != NULL)
			{
				m_dwLength = dwNewAllocLength;
			}
			return m_pbBuffPtr;
		}


		operator LPBYTE() const
		{
			return m_pbBuffPtr;
		}

		LPBYTE GetBuffer() const
		{
			return m_pbBuffPtr;
		}

		UINT32 GetAllocLength() const
		{
			return m_dwAllocLength;
		}

		UINT32 GetLength() const
		{
			return m_dwLength;
		}

		UINT32 & GetLengthRef()
		{
			return m_dwLength;
		}

		bool SetLength(UINT32 dwNewLength)
		{
			if (dwNewLength > m_dwAllocLength)
				return false;
			m_dwLength = dwNewLength;
			return true;
		}
	
	protected:
		LPBYTE m_pbBuffPtr;
		UINT32 m_dwAllocLength;
		UINT32 m_dwLength;
	};

	class CSEBuffer : public CBuffer
	{
	public:
		CSEBuffer(UINT32 dwAllocLength = 0) : CBuffer(dwAllocLength)
		{
		}
		CSEBuffer(LPBYTE pbData, UINT32 dwLength) : CBuffer(pbData, dwLength)
		{
		}
		virtual ~CSEBuffer()
		{
			if (m_pbBuffPtr != NULL)
			{
				memset(m_pbBuffPtr, 0, m_dwAllocLength);
			}
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	// class CBufferStream
	class CBufferStream : public IStream
	{
	public:
		CBufferStream()
			:m_nAllocSize(0), m_nUsedSize(0), m_nCurrPos(0), m_pbBuffer(NULL)
		{
		}

		virtual ~CBufferStream()
		{
			if (m_pbBuffer)
				delete [] m_pbBuffer;
		}

		LPBYTE GetBuffer()
		{
			return m_pbBuffer;
		}

		// IStorage
		virtual int Read(LPBYTE pbBuffer, int nLength)
		{
			if (m_nCurrPos >= m_nUsedSize)
				return 0;

			if (nLength > (m_nUsedSize - m_nCurrPos))
				nLength = m_nUsedSize - m_nCurrPos;

			memcpy(pbBuffer+m_nCurrPos, m_pbBuffer, nLength);
			m_nCurrPos += nLength;
			return nLength;
		}

		virtual int Write(LPBYTE pbBuffer, int nLength)
		{
			if (ReAllocBytes(nLength) == NULL)
			{
				return -1;
			}

			memcpy(m_pbBuffer+m_nCurrPos, pbBuffer, nLength);
			m_nCurrPos += nLength;

			if (m_nUsedSize < m_nCurrPos)
			{
				m_nUsedSize = m_nCurrPos;
			}
			return nLength;
		}

		virtual int GetPosition()
		{
			return m_nCurrPos;
		}

		virtual int SetPosition(int nPos)
		{
			if (nPos > m_nUsedSize)
				return -1;
			int nOldPos = m_nCurrPos;
			m_nCurrPos = nPos;
			return nOldPos;
		}

		virtual int GetSize()
		{
			return m_nUsedSize;
		}

		virtual int SetSize(int nSize)
		{
			int nOldUsedSize = m_nUsedSize;
			m_nUsedSize = (nSize < m_nUsedSize)? nSize : m_nUsedSize;
			if (m_nCurrPos > m_nUsedSize)
			{
				m_nCurrPos = m_nUsedSize;
			}
			return nOldUsedSize;
		}

		int WriteZero(int nLength)
		{
			if (ReAllocBytes(nLength) == NULL)
			{
				return -1;
			}

			memset(m_pbBuffer+m_nCurrPos, 0x00, nLength);
			m_nCurrPos += nLength;

			if (m_nUsedSize < m_nCurrPos)
			{
				m_nUsedSize = m_nCurrPos;
			}
			return nLength;

		}

	protected:
		LPBYTE ReAllocBytes(int nAddLength)
		{
			if (m_nCurrPos + nAddLength > m_nAllocSize)
			{
				int nAllocSize = m_nAllocSize + ((nAddLength + 0x0FFF) & 0xFFFFF000); // 4K
				LPBYTE pbTemp = new BYTE[nAllocSize]; 
				if (pbTemp == NULL)
					return NULL;

				if (m_pbBuffer != NULL)
				{
					memcpy(pbTemp, m_pbBuffer, m_nUsedSize);
					delete[] m_pbBuffer;
				}
				m_nAllocSize = nAllocSize;
				m_pbBuffer = pbTemp;
			}
			return m_pbBuffer;
		}

	protected:
		int m_nAllocSize;
		int m_nUsedSize;
		int m_nCurrPos;
		LPBYTE m_pbBuffer;
	};


	class CCircleQueueStream : public IStream
	{
	public:
		CCircleQueueStream(int nItemSize = 256, int nItemCount = 64)
			:m_nItemSize(2+nItemSize), m_nItemCount(1+nItemCount), m_nStartPos(0), m_nEndPos(0), m_pbBuffer(NULL)
		{
			m_pbBuffer = new BYTE[(m_nItemSize) * m_nItemCount];
		}

		virtual ~CCircleQueueStream()
		{
			if (m_pbBuffer)
				delete [] m_pbBuffer;
		}

		// IStorage
		virtual int Read(LPBYTE pbBuffer, int nLength)
		{
			if (m_nStartPos == m_nEndPos)
			{
				return 0;
			}
			else
			{
				LPBYTE pbTmpBuff = m_pbBuffer+(m_nStartPos*m_nItemSize);
				int nTmpLen = (pbTmpBuff[0] << 8) + pbTmpBuff[1];
				if (nLength < nTmpLen)
					nTmpLen = nLength;

				memcpy(pbBuffer, pbTmpBuff+2, nTmpLen);

				if ((++m_nStartPos) == m_nItemCount)
					m_nStartPos = 0;

				return nTmpLen;
			}
		}

		virtual int Write(LPBYTE pbBuffer, int nLength)
		{
			if (nLength > (m_nItemSize - 2))
				nLength = m_nItemSize - 2;

			LPBYTE pbTmpBuff = m_pbBuffer+(m_nEndPos*m_nItemSize);
			pbTmpBuff[0] = (BYTE)(nLength >> 8);
			pbTmpBuff[1] = (BYTE)(nLength);
			memcpy(pbTmpBuff+2, pbBuffer, nLength);

			if ((++m_nEndPos) == m_nItemCount)
				m_nEndPos = 0;

			if (m_nStartPos == m_nEndPos)
			{
				if ((++m_nStartPos) == m_nItemCount)
					m_nStartPos = 0;
			}

			return nLength;
		}

		virtual int GetPosition()
		{
			return -1;
		}

		virtual int SetPosition(int nPos)
		{
			return -1;
		}

		virtual int GetSize()
		{
			return (m_nStartPos > m_nEndPos)? (m_nItemCount+m_nStartPos-m_nEndPos) : (m_nEndPos - m_nStartPos);
		}

		virtual int SetSize(int nSize)
		{
			if (nSize == 0)
			{
				m_nStartPos = m_nEndPos = 0;
			}
			return 0;
		}

	protected:
		const int m_nItemSize;
		const int m_nItemCount;
		int m_nStartPos;
		int m_nEndPos;
		LPBYTE m_pbBuffer;
	};

	// class CMutexLock
	class CMutexLock
	{
	public:
		CMutexLock(IMutex * pMutex) : m_pMutex(pMutex)
		{
			if (m_pMutex != NULL)
				m_pMutex->Lock();
		}
		~CMutexLock()
		{
			if (m_pMutex != NULL)
				m_pMutex->Unlock();
		}

	protected:
		IMutex * m_pMutex;
	};

	// class CAsn1Item
	class CAsn1Item
	{
	public:
		CAsn1Item(UINT32 dwAllocLength) : m_buff(16+dwAllocLength)
		{
			m_dwStart = 16;
			m_dwEnd = m_dwStart;
		}

		CAsn1Item(LPBYTE pbData, UINT32 dwLength) : m_buff(16+dwLength)
		{
			m_dwStart = 16;
			m_dwEnd = m_dwStart;
			AddData(pbData, dwLength);
		}

		CAsn1Item(UINT32 dwTag, LPBYTE pbData, UINT32 dwLength) : m_buff(16+dwLength)
		{
			m_dwStart = 16;
			m_dwEnd = m_dwStart;
			AddData(pbData, dwLength);
			Make(dwTag);
		}

		HRESULT AddData(LPBYTE pbData, UINT32 dwLength)
		{
			LPBYTE p = m_buff.ReAllocBytes (m_dwEnd + dwLength);
			if (p == NULL)
			{
				return AKEY_RV_OUT_MEMORY;
			}
			memcpy(p+m_dwEnd, pbData, dwLength);
			m_dwEnd += dwLength;
			m_buff.SetLength(m_dwEnd);
			return AKEY_RV_OK;
		}

		HRESULT AddData(const CAsn1Item & subItem)
		{
			return AddData(subItem.GetBuffer(), subItem.GetLength());
		}

		LPBYTE Make(UINT32 dwTag, LPUINT32 pdwSize = NULL)
		{
			LPBYTE ptr = m_buff.GetBuffer();
			UINT32 len = m_dwEnd - m_dwStart;
			if (((dwTag == 0x02) && (len > 0) && (m_dwStart > 0) && (ptr[m_dwStart] & 0x80)) || (dwTag == 0x03))
			{
				ptr[--m_dwStart] = 0x00;
				len += 1;
			}

			if (m_dwStart < 4) // 预留的存储tag空间不足
			{
				return NULL;
			}

			if (len < 0x80)
			{
				ptr[--m_dwStart] = (BYTE)len;
			}
			else if (len < 0x100)
			{
				ptr[--m_dwStart] = (BYTE)len;
				ptr[--m_dwStart] = 0x81;
			}
			else
			{
				ptr[--m_dwStart] = (BYTE)(len & 0xFF);
				ptr[--m_dwStart] = (BYTE)((len >> 8) & 0xFF);
				ptr[--m_dwStart] = 0x82;
			}

			ptr[--m_dwStart] = (BYTE)dwTag;;
			if (pdwSize != NULL)
			{
				*pdwSize = m_dwEnd - m_dwStart;
			}
			return (ptr + m_dwStart);
		}

		LPBYTE GetBuffer() const
		{
			return (m_buff.GetBuffer() + m_dwStart);
		}

		UINT32 GetLength() const
		{
			return m_dwEnd - m_dwStart;
		}

	public:
		static int MakeTLV(BYTE btTag, LPBYTE pbData, int nLength, LPBYTE pbOutput)
		{
			LPBYTE ptr = pbOutput;
			int len = (((btTag == 0x02) && (nLength > 0) && (pbData[0] & 0x80)) || (btTag == 0x03)) ? (nLength + 1) : nLength;

			*ptr++ = btTag;
			if (len < 0x80)
			{
				*ptr++ = (BYTE)len;
			}
			else if (len < 0x100)
			{
				*ptr++ = 0x81;
				*ptr++ = (BYTE)len;
			}
			else
			{
				*ptr++ = 0x82;
				*ptr++ = (BYTE)(len & 0xFF);
				*ptr++ = (BYTE)((len >> 8) & 0xFF);
			}

			if (len > nLength)
			{
				*ptr++ = 0x00;
			}
			memcpy(ptr, pbData, nLength);
			return (int)(ptr - pbOutput) + nLength;
		}

		static int FindTLV(LPBYTE pbInput, int nInputLen, BYTE btTag, int nNumber, int * pnLength)
		{
			for (int i = 0; i<nInputLen; )
			{
				BYTE btTag_t = pbInput[i++];
				int nLen_t = pbInput[i++];
				if (nLen_t > 0x80)
				{
					int nLenSize = nLen_t & 0x7F;
					nLen_t = 0;
					for (int j = 0; j<nLenSize; j++)
					{
						nLen_t = (nLen_t << 8) + pbInput[i++];
					}
				}

				if (btTag_t == btTag)
				{
					if ((--nNumber) <= 0)
					{
						*pnLength = nLen_t;
						return i;
					}
				}
				i += nLen_t;
			}
			return -1;
		}

		static int FindTLVChain(LPBYTE pbInput, int nInputLen, LPBYTE pbTagNumberList, int nTagNumberSize, int * pnLength)
		{
			LPBYTE ptr = pbInput;
			int len = nInputLen;
			int pos = -1;
			for (int i = 0; i < nTagNumberSize; i+=2)
			{
				pos = FindTLV(ptr, len, pbTagNumberList[i], pbTagNumberList[i + 1], &len);
				if (pos < 0)
				{
					break;
				}
				ptr += pos;
			}

			*pnLength = len;
			return (pos < 0) ? -1 : (int)(ptr - pbInput);
		}

	private:
		UINT32 m_dwStart, m_dwEnd;
		CBuffer m_buff;
	};
};

#endif
