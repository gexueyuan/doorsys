/**
 * \file sm3.h
 * thanks to Xyssl
 * SM3 standards:http://www.oscca.gov.cn/News/201012/News_1199.htm
 * author:goldboar
 * email:goldboar@163.com
 * 2011-10-26
 */
#ifndef XYSSL_SM3_H
#define XYSSL_SM3_H


#define SM3_DIGEST_LENGTH	32
#define SM3_CBLOCK			64


#ifdef __cplusplus
extern "C" {
#endif

#if defined(__LP32__)
#define SM3_LONG unsigned long
#elif defined(__ILP64__)
#define SM3_LONG unsigned long
#else
#define SM3_LONG unsigned int
#endif

/**
 * \brief          SM3 context structure
 */
typedef struct sm3_context
{
    SM3_LONG state[8];     /*!< intermediate digest state  */
    SM3_LONG total[2];     /*!< number of bytes processed  */
    unsigned char buffer[64];   /*!< data block being processed */

    unsigned char ipad[64];     /*!< HMAC: inner padding        */
    unsigned char opad[64];     /*!< HMAC: outer padding        */

}SM3_CTX;


/**
 * \brief          SM3 context setup
 *
 * \param ctx      context to be initialized
 */
void SM3_Init( SM3_CTX *ctx );

/**
 * \brief          SM3 process buffer
 *
 * \param ctx      SM3 context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void SM3_Update( SM3_CTX *ctx, unsigned char *input, int ilen );

/**
 * \brief          SM3 final digest
 *
 * \param ctx      SM3 context
 */
void SM3_Final( unsigned char output[32], SM3_CTX *ctx );

/**
 * \brief          Output = SM3( input buffer )
 *
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   SM3 checksum result
 */
void SM3( unsigned char *input, int ilen, unsigned char output[32]);

/**
 * \brief          Output = SM3( file contents )
 *
 * \param path     input file name
 * \param output   SM3 checksum result
 *
 * \return         0 if successful, 1 if fopen failed,
 *                 or 2 if fread failed
 */
int SM3_File( char *path, unsigned char output[32] );

/**
 * \brief          SM3 HMAC context setup
 *
 * \param ctx      HMAC context to be initialized
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 */
void SM3_HMAC_Init( SM3_CTX *ctx, unsigned char *key, int keylen);

/**
 * \brief          SM3 HMAC process buffer
 *
 * \param ctx      HMAC context
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 */
void SM3_HMAC_Update( SM3_CTX *ctx, unsigned char *input, int ilen );

/**
 * \brief          SM3 HMAC final digest
 *
 * \param ctx      HMAC context
 * \param output   SM3 HMAC checksum result
 */
void SM3_HMAC_Finish( SM3_CTX *ctx, unsigned char output[32] );

/**
 * \brief          Output = HMAC-SM3( hmac key, input buffer )
 *
 * \param key      HMAC secret key
 * \param keylen   length of the HMAC key
 * \param input    buffer holding the  data
 * \param ilen     length of the input data
 * \param output   HMAC-SM3 result
 */
void SM3_HMA( unsigned char *key, int keylen, unsigned char *input, int ilen, unsigned char output[32] );


#ifdef __cplusplus
}
#endif

#endif /* sm3.h */
