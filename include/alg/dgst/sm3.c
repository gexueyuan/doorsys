/*
 * SM3 Hash alogrith 
 * thanks to Xyssl
 * author:goldboar
 * email:goldboar@163.com
 * 2011-10-26
 */
/*
//Testing data from SM3 Standards
//http://www.oscca.gov.cn/News/201012/News_1199.htm 
// Sample 1
// Input:"abc"  
// Output:66c7f0f4 62eeedd9 d1f2d46b dc10e4e2 4167c487 5cf2f7a2 297da02b 8f4ba8e0

// Sample 2 
// Input:"abcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcdabcd"
// Outpuf:debe9ff9 2275b8a1 38604889 c18e5a4d 6fdb70e5 387e5765 293dcba3 9c0c5732
*/

#include "sm3.h"
#include <memory.h>


/*
 * 32-bit integer manipulation macros (big endian)
 */
#ifndef GET_ULONG_BE
#define GET_ULONG_BE(n,b,i)                             \
{                                                       \
    (n) = ( (SM3_LONG) (b)[(i)    ] << 24 )        \
        | ( (SM3_LONG) (b)[(i) + 1] << 16 )        \
        | ( (SM3_LONG) (b)[(i) + 2] <<  8 )        \
        | ( (SM3_LONG) (b)[(i) + 3]       );       \
}
#endif

#ifndef PUT_ULONG_BE
#define PUT_ULONG_BE(n,b,i)                             \
{                                                       \
    (b)[(i)    ] = (unsigned char) ( ((n) >> 24) & 0xFF );       \
    (b)[(i) + 1] = (unsigned char) ( ((n) >> 16) & 0xFF );       \
    (b)[(i) + 2] = (unsigned char) ( ((n) >>  8) & 0xFF );       \
    (b)[(i) + 3] = (unsigned char) ( (n) & 0xFF       );       \
}
#endif

/*
 * SM3 context setup
 */
void SM3_Init( SM3_CTX *ctx )
{
    ctx->total[0] = 0;
    ctx->total[1] = 0;

    ctx->state[0] = 0x7380166F;
    ctx->state[1] = 0x4914B2B9;
    ctx->state[2] = 0x172442D7;
    ctx->state[3] = 0xDA8A0600;
    ctx->state[4] = 0xA96F30BC;
    ctx->state[5] = 0x163138AA;
    ctx->state[6] = 0xE38DEE4D;
    ctx->state[7] = 0xB0FB0E4E;

}

static void sm3_process( SM3_CTX *ctx, unsigned char data[64] )
{
    SM3_LONG SS1, SS2, TT1, TT2, W[68],W1[64];
    SM3_LONG A, B, C, D, E, F, G, H;
	SM3_LONG T[64];
	//SM3_LONG Temp1,Temp2,Temp3,Temp4,Temp5;
	int j;
#if defined(SM3_DEBUG)
	int i;
#endif

/* 	for(j=0; j < 68; j++)
// 		W[j] = 0;
// 	for(j=0; j < 64; j++)
// 		W1[j] = 0;
*/
	
	for(j = 0; j < 16; j++)
		T[j] = 0x79CC4519;
	for(j =16; j < 64; j++)
		T[j] = 0x7A879D8A;

    GET_ULONG_BE( W[ 0], data,  0 );
    GET_ULONG_BE( W[ 1], data,  4 );
    GET_ULONG_BE( W[ 2], data,  8 );
    GET_ULONG_BE( W[ 3], data, 12 );
    GET_ULONG_BE( W[ 4], data, 16 );
    GET_ULONG_BE( W[ 5], data, 20 );
    GET_ULONG_BE( W[ 6], data, 24 );
    GET_ULONG_BE( W[ 7], data, 28 );
    GET_ULONG_BE( W[ 8], data, 32 );
    GET_ULONG_BE( W[ 9], data, 36 );
    GET_ULONG_BE( W[10], data, 40 );
    GET_ULONG_BE( W[11], data, 44 );
    GET_ULONG_BE( W[12], data, 48 );
    GET_ULONG_BE( W[13], data, 52 );
    GET_ULONG_BE( W[14], data, 56 );
    GET_ULONG_BE( W[15], data, 60 );

#if defined(SM3_DEBUG)
	printf("Message with padding:\n");
	for(i=0; i< 8; i++)
		printf("%08x ",W[i]);
	printf("\n");
	for(i=8; i< 16; i++)
		printf("%08x ",W[i]);
	printf("\n");
#endif

#define SM3_FF0(x,y,z) ( (x) ^ (y) ^ (z)) 
#define SM3_FF1(x,y,z) (((x) & (y)) | ( (x) & (z)) | ( (y) & (z)))

#define SM3_GG0(x,y,z) ( (x) ^ (y) ^ (z)) 
#define SM3_GG1(x,y,z) (((x) & (y)) | ( (~(x)) & (z)) )

#define SM3_ROTL(x,n) (((x) << (n)) | ((x) >> (32 - (n))))

#define SM3_P0(x) ((x) ^  SM3_ROTL((x),9) ^ SM3_ROTL((x),17)) 
#define SM3_P1(x) ((x) ^  SM3_ROTL((x),15) ^ SM3_ROTL((x),23)) 

	for(j = 16; j < 68; j++ )
	{
		W[j] = SM3_P1( W[j-16] ^ W[j-9] ^ SM3_ROTL(W[j-3],15)) ^ SM3_ROTL(W[j - 13],7 ) ^ W[j-6];
		//Why thd release's result is different with the debug's ?
		//Below is okay. Interesting, Perhaps VC6 has a bug of Optimizaiton.
/*		
		Temp1 = W[j-16] ^ W[j-9];
		Temp2 = SM3_ROTL(W[j-3],15);
		Temp3 = Temp1 ^ Temp2;
		Temp4 = SM3_P1(Temp3);
		Temp5 =  SM3_ROTL(W[j - 13],7 ) ^ W[j-6];
		W[j] = Temp4 ^ Temp5;*/
	}

#if defined(SM3_DEBUG)
	printf("Expanding message W0-67:\n");
	for(i=0; i<68; i++)
	{
		printf("%08x ",W[i]);
		if(((i+1) % 8) == 0) printf("\n");
	}
	printf("\n");
#endif

	for(j =  0; j < 64; j++)
	{
        W1[j] = W[j] ^ W[j+4];
	}

#if defined(SM3_DEBUG)
	printf("Expanding message W'0-63:\n");
	for(i=0; i<64; i++)
	{
		printf("%08x ",W1[i]);
		if(((i+1) % 8) == 0) printf("\n");
	}
	printf("\n");
#endif

    A = ctx->state[0];
    B = ctx->state[1];
    C = ctx->state[2];
    D = ctx->state[3];
    E = ctx->state[4];
    F = ctx->state[5];
    G = ctx->state[6];
    H = ctx->state[7];
#if defined(SM3_DEBUG)
	printf("j     A       B        C         D         E        F        G       H\n");
	printf("   %08x %08x %08x %08x %08x %08x %08x %08x\n",A,B,C,D,E,F,G,H);
#endif

	for(j =0; j < 16; j++)
	{
		SS1 = SM3_ROTL((SM3_ROTL(A,12) + E + SM3_ROTL(T[j],j)), 7); 
		SS2 = SS1 ^ SM3_ROTL(A,12);
		TT1 = SM3_FF0(A,B,C) + D + SS2 + W1[j];
		TT2 = SM3_GG0(E,F,G) + H + SS1 + W[j];
		D = C;
		C = SM3_ROTL(B,9);
		B = A;
		A = TT1;
		H = G;
		G = SM3_ROTL(F,19);
		F = E;
		E = SM3_P0(TT2);
#if defined(SM3_DEBUG)
		printf("%02d %08x %08x %08x %08x %08x %08x %08x %08x\n",j,A,B,C,D,E,F,G,H);
#endif
	}
	
	for(j =16; j < 64; j++)
	{
		SS1 = SM3_ROTL((SM3_ROTL(A,12) + E + SM3_ROTL(T[j],j&0x1F)), 7);  // 不是所有CPU支持大于字长的位移
		SS2 = SS1 ^ SM3_ROTL(A,12);
		TT1 = SM3_FF1(A,B,C) + D + SS2 + W1[j];
		TT2 = SM3_GG1(E,F,G) + H + SS1 + W[j];
		D = C;
		C = SM3_ROTL(B,9);
		B = A;
		A = TT1;
		H = G;
		G = SM3_ROTL(F,19);
		F = E;
		E = SM3_P0(TT2);
#if defined(SM3_DEBUG)
		printf("%02d %08x %08x %08x %08x %08x %08x %08x %08x\n",j,A,B,C,D,E,F,G,H);
#endif	
	}

    ctx->state[0] ^= A;
    ctx->state[1] ^= B;
    ctx->state[2] ^= C;
    ctx->state[3] ^= D;
    ctx->state[4] ^= E;
    ctx->state[5] ^= F;
    ctx->state[6] ^= G;
    ctx->state[7] ^= H;
#if defined(SM3_DEBUG)
	   printf("   %08x %08x %08x %08x %08x %08x %08x %08x\n",ctx->state[0],ctx->state[1],ctx->state[2],
		                          ctx->state[3],ctx->state[4],ctx->state[5],ctx->state[6],ctx->state[7]);
#endif
}

/*
 * SM3 process buffer
 */
void SM3_Update( SM3_CTX *ctx, unsigned char *input, int ilen )
{
    int fill;
    SM3_LONG left;

    if( ilen <= 0 )
        return;

    left = ctx->total[0] & 0x3F;
    fill = 64 - left;

    ctx->total[0] += ilen;
    //ctx->total[0] &= 0xFFFFFFFF;

    if( ctx->total[0] < (SM3_LONG) ilen )
        ctx->total[1]++;

    if( left && ilen >= fill )
    {
        memcpy( (void *) (ctx->buffer + left), (void *) input, fill );
        sm3_process( ctx, ctx->buffer );
        input += fill;
        ilen  -= fill;
        left = 0;
    }

    while( ilen >= 64 )
    {
        sm3_process( ctx, input );
        input += 64;
        ilen  -= 64;
    }

    if( ilen > 0 )
    {
        memcpy( (void *) (ctx->buffer + left),
                (void *) input, ilen );
    }
}

static const unsigned char sm3_padding[64] =
{
 0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*
 * SM3 final digest
 */
void SM3_Final( unsigned char output[32], SM3_CTX *ctx )
{
    SM3_LONG last, padn;
    SM3_LONG high, low;
    unsigned char msglen[8];

    high = ( ctx->total[0] >> 29 )
         | ( ctx->total[1] <<  3 );
    low  = ( ctx->total[0] <<  3 );

    PUT_ULONG_BE( high, msglen, 0 );
    PUT_ULONG_BE( low,  msglen, 4 );

    last = ctx->total[0] & 0x3F;
    padn = ( last < 56 ) ? ( 56 - last ) : ( 120 - last );

    SM3_Update( ctx, (unsigned char *) sm3_padding, padn );
    SM3_Update( ctx, msglen, 8 );

    PUT_ULONG_BE( ctx->state[0], output,  0 );
    PUT_ULONG_BE( ctx->state[1], output,  4 );
    PUT_ULONG_BE( ctx->state[2], output,  8 );
    PUT_ULONG_BE( ctx->state[3], output, 12 );
    PUT_ULONG_BE( ctx->state[4], output, 16 );
    PUT_ULONG_BE( ctx->state[5], output, 20 );
    PUT_ULONG_BE( ctx->state[6], output, 24 );
    PUT_ULONG_BE( ctx->state[7], output, 28 );
}

/*
 * output = SM3( input buffer )
 */
void SM3( unsigned char *input, int ilen, unsigned char output[32] )
{
    SM3_CTX ctx;

    SM3_Init( &ctx );
    SM3_Update( &ctx, input, ilen );
    SM3_Final( output, &ctx );

    memset( &ctx, 0, sizeof( SM3_CTX ) );
}


#ifdef HEADER_ENVELOPE_H
//-------------------------------- m_sm3.c --------------------------------
static int sm2_verify(int type, const unsigned char *dgst, int dgst_len,
					  const unsigned char *sigbuf, int sig_len, EC_KEY *eckey)
{
	return(0);
}
static int sm2_sign(int type, const unsigned char *dgst, int dgstlen, 
					unsigned char *sig, unsigned int *siglen, EC_KEY *eckey)
{
	return(0);
}


static int init(EVP_MD_CTX *ctx)
{ 
	SM3_Init((SM3_CTX *)(ctx->md_data)); 
	return 1;
}

static int update(EVP_MD_CTX *ctx, const void *data, size_t count)
{ 
	SM3_Update((SM3_CTX *)(ctx->md_data),(unsigned char *)data,(unsigned int)count); 
	return 1;
}

static int final(EVP_MD_CTX *ctx, unsigned char *md)
{ 
	SM3_Final( md, (SM3_CTX *)(ctx->md_data)); 
	return 1;
}

#define EVP_PKEY_SM2_method		(evp_sign_method *)sm2_sign, \
								(evp_verify_method *)sm2_verify, \
								{EVP_PKEY_EC,0,0,0}

static /*const*/ EVP_MD sm3_md=
{
	0, /* NID_SM2_with_SM3*/
	0, /* NID_SM2_with_SM3*/
	SM3_DIGEST_LENGTH,
	0,
	init,
	update,
	final,
	NULL,
	NULL,
	EVP_PKEY_SM2_method,
	SM3_CBLOCK,
	sizeof(EVP_MD *) + sizeof(SM3_CTX),
};

const EVP_MD *EVP_sm3(void)
{
	if (sm3_md.type == 0)
	{
		OBJ_create("1.2.156.10197.1.301", "SM2", "sm2");	/*SM2椭圆曲线公钥密码算法*/
		sm3_md.type = OBJ_create("1.2.156.10197.1.401", "SM3", "sm3");	/*SM3密码杂凑算法*/
		sm3_md.pkey_type = OBJ_create("1.2.156.10197.1.501", "SM2-SM3", "sm3WithSM2");	/*签名类型*/
	}
	return(&sm3_md);
}

void OpenSSL_add_sm3_digest(void)
{
	EVP_add_digest(EVP_sm3());
}
#endif
