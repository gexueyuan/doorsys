#ifndef __AKEY_STORAGEBASE_H__
#define __AKEY_STORAGEBASE_H__

#include "AKeyDef.h"
#include "AKeyHelper.h"
#include "alg/OpenAlg.h"


namespace AKey
{
	class CStorageBase
	{
	public:
		HRESULT LoadCache()
		{
			BYTE abBuffer[8] = {0};
			ReadConfig(0, 8, abBuffer);

			// 检测标志位
			if (!(abBuffer[0] == 0x41 && abBuffer[1] == 0x51))  // Mark
			{
				m_cacheBuff.SetLength(0);
				return AKEY_RV_TOKEN_INVALID_CACHE;
			}

			UINT32 dwLength = Helper::LittleEndian::UInt32FromBytes(abBuffer+4);

			// 读取所有数据
			CBuffer buff;
			if ((buff.ReAllocBytes(dwLength) == NULL) || (m_cacheBuff.ReAllocBytes(dwLength) == NULL))
			{
				m_cacheBuff.SetLength(0);
				return AKEY_RV_OUT_MEMORY;
			}
			HRESULT hr = ReadConfig(8, dwLength, (LPBYTE)buff);
			if (hr != AKEY_RV_OK)
			{
				m_cacheBuff.SetLength(0);
				return hr;
			}

			// 解密
			int nDecLen = OpenAlg::CCipher::CipherWithKeyCode("DES-EDE-CBC", (LPBYTE)("AKEYSTORAGE"+2), 0, (LPBYTE)buff, dwLength, (LPBYTE)m_cacheBuff);
			if (nDecLen <= 0)
			{
				m_cacheBuff.SetLength(0);
				return AKEY_RV_FAIL;
			}

			// 验证CRC
			UINT32 dwCrc = OpenAlg::CRC::DoCRC16((LPBYTE)m_cacheBuff, nDecLen, 0x3E8C);
			if (((abBuffer[2] << 8) + abBuffer[3]) != dwCrc)
			{
				m_cacheBuff.SetLength(0);
				return AKEY_RV_TOKEN_INVALID_CACHE;
			}

			m_cacheBuff.SetLength((UINT32)nDecLen);
			return AKEY_RV_OK;
		}

		HRESULT SaveCache()
		{
			BYTE abBuffer[8] = {0x41, 0x51}; // Mark
			UINT32 dwCrc = OpenAlg::CRC::DoCRC16((LPBYTE)m_cacheBuff, m_cacheBuff.GetLength(), 0x3E8C);
			abBuffer[2] = (BYTE)(dwCrc >> 8);
			abBuffer[3] = (BYTE)(dwCrc);

			// 加密
			CBuffer buff;
			if (buff.ReAllocBytes(m_cacheBuff.GetLength()+16) == NULL)
			{
				m_cacheBuff.SetLength(0);
				return AKEY_RV_OUT_MEMORY;
			}
			int nEncLen = OpenAlg::CCipher::CipherWithKeyCode("DES-EDE-CBC", (LPBYTE)("AKEYSTORAGE"+2), OpenAlg::CCipher::ENCRYPT, (LPBYTE)m_cacheBuff, m_cacheBuff.GetLength(), (LPBYTE)buff);
			if (nEncLen <= 0)
			{
				return AKEY_RV_FAIL;
			}

			Helper::LittleEndian::UInt32ToBytes((UINT32)nEncLen, abBuffer+4);
			HRESULT hr = WriteConfig(0, 8, abBuffer);
			if (hr != AKEY_RV_OK)
			{
				return hr;
			}

			hr = WriteConfig(8, (UINT32)nEncLen, (LPBYTE)buff);
			return hr;
		}

		CBuffer & GetCacheBuffer()
		{
			return m_cacheBuff;
		}

		// 读配置
		virtual HRESULT ReadConfig(UINT32 dwOffset, UINT32 dwLength, LPBYTE pbData)
		{
			return AKEY_RV_FAIL;
		}
		// 写配置
		virtual HRESULT WriteConfig(UINT32 dwOffset, UINT32 dwLength, LPBYTE pbData)
		{
			return AKEY_RV_FAIL;
		}

	protected:
		int FindTLV_cache(UINT32 dwTag)
		{
			return FindTLV(m_cacheBuff, m_cacheBuff.GetLength(), dwTag);
		}

		UINT32 GetTLV_Length_cache(int nPos)
		{
			return Helper::LittleEndian::UInt32FromBytes(m_cacheBuff.GetBuffer() + nPos + 4);
		}

		LPBYTE GetTLV_Value_cache(int nPos)
		{
			return m_cacheBuff.GetBuffer() + nPos + 8;
		}

		HRESULT AppendTLV_cache(UINT32 dwTag, UINT32 dwLength, LPBYTE pbData)
		{
			if ((m_cacheBuff.GetAllocLength() - m_cacheBuff.GetLength()) < (8 + dwLength))
			{
				if (m_cacheBuff.ReAllocBytes(m_cacheBuff.GetAllocLength() + 0x4000) == NULL) // +16K
				{
					return AKEY_RV_OUT_MEMORY;
				}
			}
			AppendTLV(dwTag, dwLength, pbData,  m_cacheBuff.GetBuffer() + m_cacheBuff.GetLength());
			m_cacheBuff.SetLength(m_cacheBuff.GetLength() + 8 + dwLength);
			return AKEY_RV_OK;
		}

		HRESULT RemoveTLV_cache(UINT32 dwTag)
		{
			LPBYTE pb = m_cacheBuff;
			LPBYTE pb_end = pb + m_cacheBuff.GetLength();
			for ( ;pb < pb_end; )
			{
				UINT32 dwTag_t = Helper::LittleEndian::UInt32FromBytes(pb);
				UINT32 dwLen = Helper::LittleEndian::UInt32FromBytes(pb+4);
				if ((pb+8+dwLen) > pb_end)
				{
					return AKEY_RV_TOKEN_INVALID_CACHE;
				}

				if ((dwTag_t == dwTag) || (((dwTag & 0x0000FFFF) == 0) && (dwTag_t & 0xFFFF0000) == dwTag))
				{
					m_cacheBuff.SetLength(m_cacheBuff.GetLength() - (8+dwLen));
					pb_end -= (8+dwLen);
					memmove(pb, pb+8+dwLen, pb_end-pb);
				}
				else
				{
					pb += (8+dwLen);
				}
			}
			return AKEY_RV_OK;
		}

		HRESULT UpdateTLV_cache(UINT32 dwTag, UINT32 dwLength, LPBYTE pbData)
		{
			int nPos = FindTLV_cache(dwTag);
			if (nPos < 0)
			{
				return AppendTLV_cache(dwTag, dwLength, pbData);
			}
			else
			{
				if (GetTLV_Length_cache(nPos) == dwLength)
				{
					memcpy(GetTLV_Value_cache(nPos), pbData, dwLength);
					return AKEY_RV_OK;
				}
				else
				{
					RemoveTLV_cache(dwTag);
					return AppendTLV_cache(dwTag, dwLength, pbData);
				}
			}
		}

	protected:
		static int FindTLV(LPBYTE pbObject, UINT32 dwObjectLen, UINT32 dwTag)
		{
			for (UINT32 i=0; i<dwObjectLen; )
			{
				if (Helper::LittleEndian::UInt32FromBytes(pbObject+i) == dwTag)
				{
					return i;
				}
				
				UINT32 dwLen = Helper::LittleEndian::UInt32FromBytes(pbObject+i+4);
				if ((i+8+dwLen+8) > dwObjectLen)
				{
					return -1;
				}
				i += (8+dwLen);
			}
			return -1;
		}

		static UINT32 GetTLV_Length(LPBYTE pbObject, int nPos)
		{
			return Helper::LittleEndian::UInt32FromBytes(pbObject + nPos + 4);
		}

		static LPBYTE GetTLV_Value(LPBYTE pbObject, int nPos)
		{
			return pbObject + nPos + 8;
		}

		static UINT32 AppendTLV(UINT32 dwTag, UINT32 dwLength, LPBYTE pbData, LPBYTE pbOutput)
		{
			Helper::LittleEndian::UInt32ToBytes(dwTag, pbOutput);
			Helper::LittleEndian::UInt32ToBytes(dwLength, pbOutput+4);
			memcpy(pbOutput+8, pbData, dwLength);
			return 8+dwLength;
		}

	protected:
		CBuffer m_cacheBuff;
	};

};

#endif
