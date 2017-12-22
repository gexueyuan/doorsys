#ifndef __MATRIX_CRYPT_H__
#define __MATRIX_CRYPT_H__

#include "AKeyDef.h"
#include "alg/OpenAlg.h"


namespace AKey
{

	class CMatrixCrypt
	{
	public:
		static CMatrixCrypt * Instance()
		{
			static CMatrixCrypt s_mc;
			return &s_mc;
		}

		int GenerateRand(LPBYTE rnd)
		{
			m_rnd[0] = 0x00;
			for (; m_rnd[0] == 0x00; )
			{
				OpenAlg::CRand::GetBytes(NULL, 0, m_rnd, 8);
			}
			if (rnd != NULL)
			{
				memcpy(rnd, m_rnd, 8);
			}
			return 8;
		}

		void Cleanup()
		{
			memset(m_rnd, 0, 8);
		}

		bool IsValid() const
		{
			return (m_rnd[0] != 0x00);
		}

		int Encrypt(LPBYTE data, int length, LPBYTE output)
		{
			if (!IsValid())
			{
				GenerateRand(NULL);
			}
			return CMatrixCrypt::Encrypt(m_rnd, data, length, output);
		}
		int Decrypt(LPBYTE data, int length, LPBYTE output)
		{
			return CMatrixCrypt::Decrypt(m_rnd, data, length, output);
		}

	private:
		CMatrixCrypt()
		{
			memset(m_rnd, 0, sizeof(m_rnd));
		}
		BYTE m_rnd[8];
		
	public:
		static int Encrypt(BYTE rnd[8], LPBYTE data, int length, LPBYTE output)
		{
			for (int i=0, j=0; i<length; i++,j+=4)
			{
				EncryptByte(data[i], rnd, output+j);
			}
			return (length * 4);
		}

		static int Decrypt(BYTE rnd[8], LPBYTE data, int length, LPBYTE output)
		{
			for (int i=0, j=0; i<length; i+=4,j++)
			{
				output[j] = DecryptByte(data+i, rnd);
			}
			return (length / 4);
		}

	protected:
		static  BYTE GMul(BYTE a, BYTE b)
		{
			BYTE p = 0;
			for(int counter = 0; counter < 8; counter++) 
			{
				if((b & 1) == 1) 
					p ^= a;
				BYTE hi_bit_set = (a & 0x80);
				a <<= 1;
				if(hi_bit_set == 0x80) 
					a ^= 0x1b;		
				b >>= 1;
			}
			return p;
		}

		// GF(2^8)有限域矩陣乘法
		static void GMix_Column(BYTE r[4]) 
		{
			BYTE a[4] = {r[0], r[1], r[2], r[3]};
			r[0] = GMul(a[0],2) ^ GMul(a[3],1) ^ GMul(a[2],1) ^ GMul(a[1],3);
			r[1] = GMul(a[1],2) ^ GMul(a[0],1) ^ GMul(a[3],1) ^ GMul(a[2],3);
			r[2] = GMul(a[2],2) ^ GMul(a[1],1) ^ GMul(a[0],1) ^ GMul(a[3],3);
			r[3] = GMul(a[3],2) ^ GMul(a[2],1) ^ GMul(a[1],1) ^ GMul(a[0],3);
			memset(a, 0, sizeof(a));
		}

		//GF (2^8)有限域矩陣乘法
		static void Inv_Mix_Column(BYTE r[4]) 
		{
			BYTE a[4] = {r[0], r[1], r[2], r[3]};
			r[0] = GMul(a[0],14) ^ GMul(a[3],9) ^ GMul(a[2],13) ^ GMul(a[1],11);
			r[1] = GMul(a[1],14) ^ GMul(a[0],9) ^ GMul(a[3],13) ^ GMul(a[2],11);
			r[2] = GMul(a[2],14) ^ GMul(a[1],9) ^ GMul(a[0],13) ^ GMul(a[3],11);
			r[3] = GMul(a[3],14) ^ GMul(a[2],9) ^ GMul(a[1],13) ^ GMul(a[0],11);
			memset(a, 0, sizeof(a));
		}

		static void EncryptByte(BYTE c, BYTE rnd[8], BYTE r[4])
		{
			r[0] = rnd[3]^(c&0x03)^rnd[5];
			r[1] = rnd[7]^(c&0x0c)^rnd[0];
			r[2] = rnd[4]^(c&0x30)^rnd[2];
			r[3] = rnd[6]^(c&0xc0)^rnd[1];
			Inv_Mix_Column(r);
		}

		static BYTE DecryptByte(BYTE en[4], BYTE rnd[8])
		{
			BYTE buff[4] = {en[0], en[1], en[2], en[3]};
			GMix_Column(buff);
			BYTE r  = (buff[0]^rnd[3]^rnd[5]);
			r += (buff[1]^rnd[7]^rnd[0]);
			r += (buff[2]^rnd[4]^rnd[2]);
			r += (buff[3]^rnd[6]^rnd[1]);
			memset(buff, 0, sizeof(buff));
			return r;
		}
	};

};

#endif
