#ifndef SM4_DEBUG
# ifndef NDEBUG
#  define NDEBUG
# endif
#endif
//#include <assert.h>

//#include <stdlib.h>
//#include <memory.h>
//#include <openssl/sm4.h>

#define SM4_ENCRYPT	1
#define SM4_DECRYPT	0

/* Because array size can't be a const in C, the following two are macros.
   Both sizes are in bytes. */
/*#define SM4_MAXNR 14*/
#define SM4_BLOCK_SIZE 16

#ifdef  __cplusplus
extern "C" {
#endif

/* This should be a hidden type, but EVP requires that the size be known */
struct sm4_key_st {
#ifdef SM4_LONG
    unsigned long md_key[4];
    unsigned long rd_key[32];
#else
    unsigned int md_key[4];
    unsigned int rd_key[32];
#endif
    /*int rounds;*/
};
typedef struct sm4_key_st SM4_KEY;

int SM4_set_key(const unsigned char *userKey, const int bits,
	SM4_KEY *key);

void SM4_encrypt(const unsigned char *in, unsigned char *out,
	const SM4_KEY *key);
void SM4_decrypt(const unsigned char *in, unsigned char *out,
	const SM4_KEY *key);

void SM4_ecb_encrypt(const unsigned char *in, unsigned char *out,
	const SM4_KEY *key, const int enc);
void SM4_cbc_encrypt(const unsigned char *in, unsigned char *out,
	const unsigned long length, const SM4_KEY *key,
	unsigned char *ivec, const int enc);

#ifdef  __cplusplus
}
#endif


#ifdef SM4_LONG
typedef unsigned long INT32U;
#else
typedef unsigned int INT32U;
#endif
typedef unsigned char INT8U;

#if defined(_MSC_VER) && (defined(_M_IX86) || defined(_M_AMD64) || defined(_M_X64))
# define SWAP(x) (_lrotl(x, 8) & 0x00ff00ff | _lrotr(x, 8) & 0xff00ff00)
# define GETU32(p) SWAP(*((INT32U *)(p)))
# define PUTU32(ct, st) { *((INT32U *)(ct)) = SWAP((st)); }
#else
# define GETU32(pt) (((INT32U)(pt)[0] << 24) ^ ((INT32U)(pt)[1] << 16) ^ ((INT32U)(pt)[2] <<  8) ^ ((INT32U)(pt)[3]))
# define PUTU32(ct, st) { (ct)[0] = (INT8U)((st) >> 24); (ct)[1] = (INT8U)((st) >> 16); (ct)[2] = (INT8U)((st) >>  8); (ct)[3] = (INT8U)(st); }
#endif


/*
*********************************************************************************************************
*                                                 MACRO
*********************************************************************************************************
*/
#define SMS4_ROUND     32                                                    
#define ROL(x,y)       (((x) << (y)) | (((x) & 0xFFFFFFFFul) >> (32 - (y))))   


/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/ 
const static INT32U FK[4] = 
{
    0xA3B1BAC6ul, 0x56AA3350ul, 0x677D9197ul, 0xB27022DCul
};

const static INT32U CK[SMS4_ROUND] = 
{
    0x00070e15ul, 0x1c232a31ul, 0x383f464dul, 0x545b6269ul,
    0x70777e85ul, 0x8c939aa1ul, 0xa8afb6bdul, 0xc4cbd2d9ul,
    0xe0e7eef5ul, 0xfc030a11ul, 0x181f262dul, 0x343b4249ul,
    0x50575e65ul, 0x6c737a81ul, 0x888f969dul, 0xa4abb2b9ul,
    0xc0c7ced5ul, 0xdce3eaf1ul, 0xf8ff060dul, 0x141b2229ul,
    0x30373e45ul, 0x4c535a61ul, 0x686f767dul, 0x848b9299ul,
    0xa0a7aeb5ul, 0xbcc3cad1ul, 0xd8dfe6edul, 0xf4fb0209ul,
    0x10171e25ul, 0x2c333a41ul, 0x484f565dul, 0x646b7279ul
};

const static INT8U Sbox[256] =
{
    0xd6,0x90,0xe9,0xfe,0xcc,0xe1,0x3d,0xb7,0x16,0xb6,0x14,0xc2,0x28,0xfb,0x2c,0x05,
    0x2b,0x67,0x9a,0x76,0x2a,0xbe,0x04,0xc3,0xaa,0x44,0x13,0x26,0x49,0x86,0x06,0x99,
    0x9c,0x42,0x50,0xf4,0x91,0xef,0x98,0x7a,0x33,0x54,0x0b,0x43,0xed,0xcf,0xac,0x62,
    0xe4,0xb3,0x1c,0xa9,0xc9,0x08,0xe8,0x95,0x80,0xdf,0x94,0xfa,0x75,0x8f,0x3f,0xa6,
    0x47,0x07,0xa7,0xfc,0xf3,0x73,0x17,0xba,0x83,0x59,0x3c,0x19,0xe6,0x85,0x4f,0xa8,
    0x68,0x6b,0x81,0xb2,0x71,0x64,0xda,0x8b,0xf8,0xeb,0x0f,0x4b,0x70,0x56,0x9d,0x35,
    0x1e,0x24,0x0e,0x5e,0x63,0x58,0xd1,0xa2,0x25,0x22,0x7c,0x3b,0x01,0x21,0x78,0x87,
    0xd4,0x00,0x46,0x57,0x9f,0xd3,0x27,0x52,0x4c,0x36,0x02,0xe7,0xa0,0xc4,0xc8,0x9e,
    0xea,0xbf,0x8a,0xd2,0x40,0xc7,0x38,0xb5,0xa3,0xf7,0xf2,0xce,0xf9,0x61,0x15,0xa1,
    0xe0,0xae,0x5d,0xa4,0x9b,0x34,0x1a,0x55,0xad,0x93,0x32,0x30,0xf5,0x8c,0xb1,0xe3,
    0x1d,0xf6,0xe2,0x2e,0x82,0x66,0xca,0x60,0xc0,0x29,0x23,0xab,0x0d,0x53,0x4e,0x6f,
    0xd5,0xdb,0x37,0x45,0xde,0xfd,0x8e,0x2f,0x03,0xff,0x6a,0x72,0x6d,0x6c,0x5b,0x51,
    0x8d,0x1b,0xaf,0x92,0xbb,0xdd,0xbc,0x7f,0x11,0xd9,0x5c,0x41,0x1f,0x10,0x5a,0xd8,
    0x0a,0xc1,0x31,0x88,0xa5,0xcd,0x7b,0xbd,0x2d,0x74,0xd0,0x12,0xb8,0xe5,0xb4,0xb0,
    0x89,0x69,0x97,0x4a,0x0c,0x96,0x77,0x7e,0x65,0xb9,0xf1,0x09,0xc5,0x6e,0xc6,0x84,
    0x18,0xf0,0x7d,0xec,0x3a,0xdc,0x4d,0x20,0x79,0xee,0x5f,0x3e,0xd7,0xcb,0x39,0x48
};


/*
*********************************************************************************************************
*                                         Non-linear Transform
*
* Description : None
*
* Arguments   : A          input
*
*
* Returns     : B          output
*********************************************************************************************************
*/
static INT32U t(INT32U A) 
{
    INT8U  a[4] = {0};
    INT8U  b[4] = {0};
    INT32U B = 0;
    INT8U  i = 0;

    for(i=0; i<4; i++) 
    {
        a[i] = (INT8U)((A>>(i*8)) & 0x00ff);
        b[i] = Sbox[(a[i])];
        B   |= ((INT32U)(b[i]) << (i*8));
    }

    return B;
}


/*
*********************************************************************************************************
*                               Compose Transform for Encryption / Decryption
*
* Description : 
*
* Arguments   : A          input
*
*
* Returns     : C          output
*********************************************************************************************************
*/
static INT32U T1(INT32U A) 
{
    INT32U B = 0;
    INT32U C = 0;

    B = t(A);
    C = B ^ ROL(B,2) ^ ROL(B,10) ^ ROL(B,18) ^ ROL(B,24);

    return C;
}


/*
*********************************************************************************************************
*                                   Compose Transform for Key Expansion
*
* Description : 
*
* Arguments   : A          input
*
*
* Returns     : C          output
*********************************************************************************************************
*/
static INT32U T2(INT32U A) 
{
    INT32U B = 0;
    INT32U C = 0;

    B = t(A);
    /*printf ("B = %08x\n", B);*/
    C = B ^ ROL(B,13) ^ ROL(B,23);

    return C;
}


/*
*********************************************************************************************************
*                                             Key Expansion
*
* Description : None
*
* Arguments   : MK[]       Seed Key
*
*               rk[]       Round Key
* 
*
* Returns     : None
*********************************************************************************************************
*/
static void SMS4_KeyExpansion(INT32U MK[], INT32U rk[]) 
{
    INT32U K[4] = {0};
    INT8U  i = 0;
    
    for(i=0; i<4; i++)
    {
        K[i] = MK[i] ^ FK[i];
        /*printf ("K[%d] = %08x\n", i, K[i]);
        printf ("K[%d] = %02x\n", i, K[i]);*/
    }
    
    for(i=0; i<SMS4_ROUND; i++)
    {
        /*printf ("异或 %08x\n", K[(i+1)%4] ^ K[(i+2)%4] ^ K[(i+3)%4] ^ CK[i]);*/
        K[i%4] ^= T2(K[(i+1)%4] ^ K[(i+2)%4] ^ K[(i+3)%4] ^ CK[i]);
        rk[i] = K[i % 4];
    }    
}


/*
*********************************************************************************************************
*                                             SMS4 Encryption
*
* Description : None
*
* Arguments   : X[]        PlainText 
*
*               rk[]       Round Key
*
*               Y[]        CypherText
* 
*
* Returns     : None
*********************************************************************************************************
*/
static void SMS4_Encryption(INT32U X[], INT32U rk[], INT32U Y[])
{
    INT32U tempX[4] = {0};
    INT8U  i = 0;
    
    for (i=0; i<4; i++)
         tempX[i] = X[i];

    for (i=0; i<SMS4_ROUND; i++)
         tempX[i%4] ^= T1(tempX[(i+1)%4] ^ tempX[(i+2)%4] ^ tempX[(i+3)%4] ^ rk[i]);

    for (i=0; i<4; i++)
         Y[i] = tempX[3-i];
}


/*
*********************************************************************************************************
*                                             SMS4 Decryption
*
* Description : None
*`
* Arguments   : X[]        CypherText
*
*               rk[]       Round Key
*
*               Y[]        PlainText
* 
*
* Returns     : None
*********************************************************************************************************
*/
static void SMS4_Decryption(INT32U X[], INT32U rk[], INT32U Y[])
{
    INT32U tempX[4] = {0};
    INT8U  i = 0;
    
    for (i=0; i<4; i++)
         tempX[i] = X[i];

    for (i=0; i<SMS4_ROUND; i++)
         tempX[i%4] ^= T1(tempX[(i+1)%4] ^ tempX[(i+2)%4] ^ tempX[(i+3)%4] ^ rk[(31-i)]);

    for (i=0; i<4; i++)
         Y[i] = tempX[3-i];
}



/****************************************ljc being************************************************************/


int SM4_set_key(const unsigned char *userKey, const int bits,
	SM4_KEY *key)
{
	INT32U *mk;
	if (!userKey || !key)
		return -1;
	if (bits != 128)
		return -2;

	mk = key->md_key;
	mk[0] = GETU32(userKey     );
	mk[1] = GETU32(userKey +  4);
	mk[2] = GETU32(userKey +  8);
	mk[3] = GETU32(userKey + 12);
	SMS4_KeyExpansion(mk, key->rd_key);
	return 0;
}

void SM4_encrypt(const unsigned char *in, unsigned char *out,
	const SM4_KEY *key)
{
	INT32U x[4], y[4];

	x[0] = GETU32(in     );
	x[1] = GETU32(in +  4);
	x[2] = GETU32(in +  8);
	x[3] = GETU32(in + 12);
	SMS4_Encryption(x, (INT32U *)(key->rd_key), y);
	PUTU32(out, y[0]);
	PUTU32(out +  4, y[1]);
	PUTU32(out +  8, y[2]);
	PUTU32(out +  12, y[3]);
}
void SM4_decrypt(const unsigned char *in, unsigned char *out,
	const SM4_KEY *key)
{
	INT32U x[4], y[4];

	x[0] = GETU32(in     );
	x[1] = GETU32(in +  4);
	x[2] = GETU32(in +  8);
	x[3] = GETU32(in + 12);
	SMS4_Decryption(x, (INT32U *)(key->rd_key), y);
	PUTU32(out, y[0]);
	PUTU32(out +  4, y[1]);
	PUTU32(out +  8, y[2]);
	PUTU32(out +  12, y[3]);
}

void SM4_ecb_encrypt(const unsigned char *in, unsigned char *out,
	const SM4_KEY *key, const int enc)
{
    //assert(in && out && key);
	//assert((SM4_ENCRYPT == enc)||(SM4_DECRYPT == enc));

	if (SM4_ENCRYPT == enc)
		SM4_encrypt(in, out, key);
	else
		SM4_decrypt(in, out, key);
}
void SM4_cbc_encrypt(const unsigned char *in, unsigned char *out,
	const unsigned long length, const SM4_KEY *key,
	unsigned char *ivec, const int enc)
{
	unsigned long n;
	unsigned long len = length;
	unsigned char tmp[SM4_BLOCK_SIZE];
	const unsigned char *iv = ivec;

	//assert(in && out && key && ivec);
	//assert((SM4_ENCRYPT == enc)||(SM4_DECRYPT == enc));

	if (SM4_ENCRYPT == enc) {
		while (len >= SM4_BLOCK_SIZE) {
			for(n=0; n < SM4_BLOCK_SIZE; ++n)
				out[n] = in[n] ^ iv[n];
			SM4_encrypt(out, out, key);
			iv = out;
			len -= SM4_BLOCK_SIZE;
			in += SM4_BLOCK_SIZE;
			out += SM4_BLOCK_SIZE;
		}
		if (len) {
			for(n=0; n < len; ++n)
				out[n] = in[n] ^ iv[n];
			for(n=len; n < SM4_BLOCK_SIZE; ++n)
				out[n] = iv[n];
			SM4_encrypt(out, out, key);
			iv = out;
		}
		memcpy(ivec,iv,SM4_BLOCK_SIZE);
	} else if (in != out) {
		while (len >= SM4_BLOCK_SIZE) {
			SM4_decrypt(in, out, key);
			for(n=0; n < SM4_BLOCK_SIZE; ++n)
				out[n] ^= iv[n];
			iv = in;
			len -= SM4_BLOCK_SIZE;
			in  += SM4_BLOCK_SIZE;
			out += SM4_BLOCK_SIZE;
		}
		if (len) {
			SM4_decrypt(in,tmp,key);
			for(n=0; n < len; ++n)
				out[n] = tmp[n] ^ iv[n];
			iv = in;
		}
		memcpy(ivec,iv,SM4_BLOCK_SIZE);
	} else {
		while (len >= SM4_BLOCK_SIZE) {
			memcpy(tmp, in, SM4_BLOCK_SIZE);
			SM4_decrypt(in, out, key);
			for(n=0; n < SM4_BLOCK_SIZE; ++n)
				out[n] ^= ivec[n];
			memcpy(ivec, tmp, SM4_BLOCK_SIZE);
			len -= SM4_BLOCK_SIZE;
			in += SM4_BLOCK_SIZE;
			out += SM4_BLOCK_SIZE;
		}
		if (len) {
			memcpy(tmp, in, SM4_BLOCK_SIZE);
			SM4_decrypt(tmp, out, key);
			for(n=0; n < len; ++n)
				out[n] ^= ivec[n];
			for(n=len; n < SM4_BLOCK_SIZE; ++n)
				out[n] = tmp[n];
			memcpy(ivec, tmp, SM4_BLOCK_SIZE);
		}
	}
}


#ifdef HEADER_ENVELOPE_H
//-------------------------------- e_sm4 --------------------------------
static int sm4_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
					const unsigned char *iv, int enc);

typedef struct
{
	SM4_KEY ks;
} EVP_SM4_KEY;


#define BLOCK_CIPHER_ecb_loop() \
	unsigned int i, bl; \
	bl = ctx->cipher->block_size;\
	if(inl < bl) return 1;\
	inl -= bl; \
	for(i=0; i <= inl; i+=bl) 

#define BLOCK_CIPHER_func_ecb(cname, cprefix, kstruct, ksched) \
static int cname##_ecb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
	BLOCK_CIPHER_ecb_loop() \
		cprefix##_ecb_encrypt(in + i, out + i, &((kstruct *)ctx->cipher_data)->ksched, ctx->encrypt);\
	return 1;\
}

#define BLOCK_CIPHER_func_cbc(cname, cprefix, kstruct, ksched) \
static int cname##_cbc_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
	cprefix##_cbc_encrypt(in, out, (long)inl, &((kstruct *)ctx->cipher_data)->ksched, ctx->iv, ctx->encrypt);\
	return 1;\
}

BLOCK_CIPHER_func_ecb(sm4, SM4, EVP_SM4_KEY, ks);
BLOCK_CIPHER_func_cbc(sm4, SM4, EVP_SM4_KEY, ks);

static EVP_CIPHER sm4_ecb = {
	0, 16, 16, 16,
	EVP_CIPH_ECB_MODE, 
	sm4_init_key, 
	sm4_ecb_cipher,
	NULL, 
	sizeof(EVP_SM4_KEY),
	EVP_CIPHER_set_asn1_iv, EVP_CIPHER_get_asn1_iv,
	NULL,
	NULL
}; 

static EVP_CIPHER sm4_cbc = {
	0, 16, 16, 16,
	EVP_CIPH_CBC_MODE, 
	sm4_init_key, 
	sm4_cbc_cipher,
	NULL, 
	sizeof(EVP_SM4_KEY),
	EVP_CIPHER_set_asn1_iv, EVP_CIPHER_get_asn1_iv,
	NULL,
	NULL
}; 


const EVP_CIPHER *EVP_sm4_ecb(void)
{
	if (sm4_ecb.nid == 0)
	{
		sm4_ecb.nid = OBJ_create("1.2.156.7564596.1", "SM4-ECB", "sm4-ecb");
	}
	return &sm4_ecb; 
}

const EVP_CIPHER *EVP_sm4_cbc(void)
{
	if (sm4_cbc.nid == 0)
	{
		sm4_cbc.nid = OBJ_create("1.2.156.7564596.2", "SM4-CBC", "sm4-cbc");
	}
	return &sm4_cbc; 
}


static int sm4_init_key(EVP_CIPHER_CTX *ctx, const unsigned char *key,
		   const unsigned char *iv, int enc)
{
	int ret=SM4_set_key(key, ctx->key_len * 8, (SM4_KEY*)(ctx->cipher_data));

	if(ret < 0)
	{
		/*EVPerr(EVP_F_SM4_INIT_KEY,EVP_R_SM4_KEY_SETUP_FAILED);*/
		return 0;
	}

	return 1;
}


void OpenSSL_add_sm4_cipher(void)
{
	EVP_add_cipher(EVP_sm4_ecb());
	EVP_add_cipher(EVP_sm4_cbc());
}
#endif
