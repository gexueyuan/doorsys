#ifndef __AKEY_DEF_H__
#define __AKEY_DEF_H__
//#pragma once


typedef signed char         INT8, *LPINT8;
typedef signed short        INT16, *LPINT16;
typedef signed int          INT32, *LPINT32;
typedef unsigned char       UINT8, *LPUINT8;
typedef unsigned short      UINT16, *LPUINT16;
typedef unsigned int        UINT32, *LPUINT32;

#ifdef WIN32
typedef unsigned __int64	UINT64;
#define AKEY_NODEF_BYTE
#ifndef AKEY_INTERFACE
# define AKEY_INTERFACE		interface
#endif
#else
typedef const char * 		LPCSTR;
typedef unsigned long long	UINT64;
typedef long				HRESULT;
#define AKEY_INTERFACE		struct
#endif

// �������Ҫ�������Ͷ��壬���ڰ������ļ�ǰ����AKEY_NODEF_BYTE
#ifndef AKEY_NODEF_BYTE
typedef UINT8 BYTE;
typedef LPUINT8 LPBYTE;
#endif

// ��־��
#ifndef LGNTRACE
#define LGN_TRACE_CATEGORY_DEF 0
#define LGN_TRACE_CATEGORY_ERROR_LEVEL 3
#define LGN_TRACE_CATEGORY_WARNING_LEVEL 4
#define LGN_TRACE_CATEGORY_INFO_LEVEL 6
#define LGN_TRACE_CATEGORY_DEBUG_LEVEL 7
#define LGNTRACE
#define LGNTRACE_ENTRY(cate, level, func)
#define LGNTRACE_MSG
#define LGNTRACE_HEX(msg, data, len)
#define LGNTRACE_HEX_LV(level, msg, data, len)
#define LGNTRACE_ERRORNO(err)
#endif

// ���������
#define AKEY_INDEX_ENCODE_FSK_24_36			8 // ����
#define AKEY_INDEX_ENCODE_FSK_04_10			7
#define AKEY_INDEX_ENCODE_FSK_04_08			6
#define AKEY_INDEX_ENCODE_FSK_04_06			5
#define AKEY_INDEX_ENCODE_FSK_02_04			4
//#define AKEY_INDEX_ENCODE_OPTIMIZED_LIST	"\xFF\xFF\xFF\xFF\x01\x02\x03\x04\x05\xFF\xFF\xFF\xFF\xFF\xFF\xFF" // �����б�
#define AKEY_INDEX_DECODE_2FSK_1000			8 // ����
#define AKEY_INDEX_DECODE_2FSK_5500			7
#define AKEY_INDEX_DECODE_2FSK_6000			6
#define AKEY_INDEX_DECODE_2FSK_2756			5
#define AKEY_INDEX_DECODE_2FSK_2000			4
#define AKEY_INDEX_DECODE_ASK_2000			3
#define AKEY_INDEX_DECODE_ASK_2000_WP		1
#define AKEY_INDEX_DECODE_ASK_4000			9 // ����
#define AKEY_INDEX_DECODE_ASK_11000			10
#define AKEY_INDEX_DECODE_ASK_12000			11
#define AKEY_INDEX_DECODE_OPTIMIZED_LIST	"\xFF\x08\xFF\x07\x06\x05\x03\x04\x09\xFF\x02\x01\xFF\xFF\xFF\xFF" // �����б�
#define AKEY_INDEX_ENCODER_MAX_NUMBER		5 // ֧�ֵı�������
#define AKEY_INDEX_DECODER_MAX_NUMBER		32 // ֧�ֵĽ��������������������

// ������־
#define AKEY_FLAG_ENCODE_SYNC_PLAY			0x00800000 // ͬ�����ţ����ڼ�����ճ�ʱʱ��
#define AKEY_FLAG_ENCODE_WAVE_ROTATE180		0x00010000 // ��ת180
#define AKEY_FLAG_ENCODE_WAVE_STEREO		0x00040000 // ������
#define AKEY_FLAG_ENCODE_WAVE_PRIORITY		0x01000000 // ���ȱ���
#define AKEY_FLAG_ENCODE_WAVE_SELECT		0x02000000 // ѡ�����
#define AKEY_FLAG_DECODE_WAVE_ROTATE180		0x00010000 // ��ת180
#define AKEY_FLAG_DECODE_WAVE_POLAR			0x00020000 // ָ�����μ���
#define AKEY_FLAG_DECODE_WAVE_LEFT_CHANNEL	0x00040000 // ������
#define AKEY_FLAG_DECODE_WAVE_RIGHT_CHANNEL	0x00080000 // ������
#define AKEY_FLAG_DECODE_WAVE_PRIORITY		0x01000000 // ���Ƚ���
#define AKEY_FLAG_DECODE_WAVE_SELECT		0x02000000 // ѡ�����
#define AKEY_FLAG_DECODE_WAVE_PLUGIN		0x04000000 // ʹ�ò������
//#define AKEY_FLAG_DECODE_TEST				0x80000000
#define AKEY_FLAG_WAVE_MAP_MASK				0x00007FFF
#define AKEY_FLAG_ENCODE_ECC				0x00008000
#define AKEY_FLAG_DECODE_ECC				0x00008000
#define AKEY_FLAG_ENCODE_VALUE_WITH_INDEX(n) (1<<((n)-1))
#define AKEY_FLAG_DECODE_VALUE_WITH_INDEX(n) (1<<((n)-1))


// TOKEN����
#define AKKY_FLAG_TOKEN_CHECK_DEVICE		0x00000001	// ����豸
#define AKKY_FLAG_TOKEN_NEW_DEVICE			0x00000002	// ָ��Ϊ���豸
#define AKKY_FLAG_TOKEN_CHECK_SESSION		0x00000004	// ����Ƿ�Ҫ�½�ͨ��
#define AKKY_FLAG_TOKEN_NEW_SESSION			0x00000008	// ǿ���½�ͨ��
#define AKKY_FLAG_TOKEN_PREPARE_MASK		0x000000FF	// PREPAREר��
#define AKKY_FLAG_TOKEN_PINID2				0x00010000	// ʹ��2��PIN
//#define AKKY_FLAG_TOKEN_USE_CACHE_RAND		0x00020000	// ʹ���ϴλ���������
#define AKKY_FLAG_TOKEN_SN_ENDCHAR0			0x00040000	// ��ǺŰ���������0����Ҫ���ڶ�ȡ����֤��
#define AKKY_FLAG_TOKEN_NOT_USE_CACHE		0x01000000	// ��ʹ�û��棨Ŀǰ���ڶ�������
#define AKKY_FLAG_TINIT_CHECKED_DEVICE		0x00000001	// �Ѽ���豸
#define AKKY_FLAG_TINIT_SUPPORT_ENCRYPTAPDU	0x00000100	// ֧�ּ���ָ�init
#define AKKY_FLAG_TINIT_MUST_ENCRYPTAPDU	0x00000200	// ����ʹ�ü���ָ�init
#define AKKY_FLAG_TINIT_NOT_LOAD_PAIRKEY1B	0x00000400	// ������1B����Կ��init��������ĸ�������Ҫɾ�������ļ�������
#define AKKY_FLAG_TINIT_MUST_SESSION		0x00000800	// ִ���κ�ָ��ǰ�����Ƚ���ͨ��

// ժҪ����
#define AKEY_HASH_SHA1						0x00
#define AKEY_HASH_MD5						0x01
#define AKEY_HASH_SHA256					0x02
#define AKEY_HASH_SHA384					0x03
#define AKEY_HASH_SHA512					0x04
#define AKEY_HASH_MD5SHA					0x05
#define AKEY_HASH_SM3						0x06
#define AKEY_HASH_SHA256_SM3				0x07
#define AKEY_HASH_FROM_LENGTH(l)			(((l) == 20)? AKEY_HASH_SHA1 : ((l) >> 4))

// ǩ����־��һ���ֽڵ�ժҪ���ͣ�
#define AKEY_FLAG_SIGN_HASH_SHA1			AKEY_HASH_SHA1
#define AKEY_FLAG_SIGN_HASH_MD5				AKEY_HASH_MD5
#define AKEY_FLAG_SIGN_HASH_SHA256			AKEY_HASH_SHA256
#define AKEY_FLAG_SIGN_HASH_SHA384			AKEY_HASH_SHA384
#define AKEY_FLAG_SIGN_HASH_SHA512			AKEY_HASH_SHA512
#define AKEY_FLAG_SIGN_HASH_MD5SHA			AKEY_HASH_MD5SHA
#define AKEY_FLAG_SIGN_HASH_SM3				AKEY_HASH_SM3
#define AKEY_FLAG_SIGN_HASH_SHA256_SM3		AKEY_HASH_SHA256_SM3
#define AKEY_FLAG_SIGN_HASH_MASK			0x000000FF
#define AKEY_FLAG_SIGN_HASH_FROM_LENGTH(l)	AKEY_HASH_FROM_LENGTH(l)
#define AKEY_FLAG_SIGN_NOHASHOID			0x00010000
//#define AKEY_FLAG_SIGN_NOCHECKINTEGRIRY		0x00020000	// COS�ڲ��������Կ��������
#define AKEY_FLAG_SIGN_GETRESULT_SCANKEY	0x00040000	// ȡǩ���Ľ����ͬʱ���ɨ�谴��
#define AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS	0x00080000	// ����ͨѶ��ַ
#define AKEY_FLAG_SIGN_DEVELOP_DATA			0x00100000	// ǿ��ʹ��Develop��װ����
#define AKEY_FLAG_SIGN_TYPE_MASK			0xFF000000
#define AKEY_FLAG_SIGN_TYPE_DOUBLEHASH		(0x01 << 24)	// ˫ժҪǩ��
#define AKEY_FLAG_SIGN_TYPE_SHOWDATA		(0x02 << 24)	// ֱ����ʾǩ��
#define AKEY_FLAG_SIGN_TYPE_HASHRELAY		(0x03 << 24)	// ժҪ����ǩ��
#define AKEY_FLAG_SIGN_TYPE_HASHRELAY2		(0x04 << 24)	// ժҪ����ǩ��2
// P7
#define AKEY_FLAG_P7_HASH_MASK				0x000000FF
#define AKEY_FLAG_P7_SM						0x00010000	// ����
#define AKEY_FLAG_P7_ORG_OID				0x00020000	// ԭ��OIDû��ʹ�ù���OID����ICBC
#define AKEY_FLAG_P7_ABC					0x00040000	// ǩ��OID+ǩ�����ֱ���ýṹ�洢

// �㷨��־
#define AKEY_FLAG_CIPHER_ENCRYPT			0x00000001	// ����
#define AKEY_FLAG_CIPHER_DECRYPT			0x00000000	// ����
#define AKEY_FLAG_CIPHER_NOPADDING			0x00010000	// NOPADDING
#define AKEY_FLAG_RSA_NOPADDING				0x00010000	// NOPADDING
#define AKEY_FLAG_RSA_PUBLICDECRYPT			0x00100000	// ��Կ����

// ����
#define AKEY_PARAM_DEVICE_LABLE				0x00000001 // ���
#define AKEY_PARAM_DEVICE_SERIAL_NUMBER		0x00000002 // ��Ǻţ�ֻ����
#define AKEY_PARAM_CHARSET					0x00000003 // �ַ�����
#define AKEY_PARAM_LANGID					0x00000004 // ����ID
#define AKEY_PARAM_CHARSET_LIST				0x00000005 // �ַ������б�ֻ����
#define AKEY_PARAM_LANGID_LIST				0x00000006 // ����ID�б�ֻ����
#define AKEY_PARAM_USERPIN_ATTR				0x00000007 // �û����������ԣ�ֻ������������ʹ�ã�
#define AKEY_PARAM_USERPIN_TIMES			0x00000008 // �û����������������+ʣ�����Σ��ڴ棩��ֻд���ֵ��
#define AKEY_PARAM_CERTKEYID_LIST			0x00000009 // ֤�������ԿId�б�ֻ���������棩
#define AKEY_PARAM_DEVICE_RAND				0x0000000A // ����������豸����д���棩
#define AKEY_PARAM_COSINFO					0x0000000B // COS��Ϣ��ֻ����
#define AKEY_PARAM_LANGID_CHARSET			0x0000000C // ����ID+�ַ����루ֻ����
#define AKEY_PARAM_SIGN_PACKET_SZIE			0x0000000D // ��ȡǩ�����ĵķ������С+������С��ֻ����
#define AKEY_PARAM_SCREEN_ROTATOIN			0x0000000E // ��Ļ��ת��ֻд������ʽ��00-ÿ�ζ���ת��80-��ת������81-��ת������
#define AKEY_PARAM_DEVICE_CUSTOMID			0x0000000F // �ͻ��ţ�ֻ����
#define AKEY_PARAM_CERT_INFO_LIST			0x00000010 // ֤����Ϣ�б�OID + KID + Usage + Type + Len(2) + Name����ֻ���������棩
#define AKEY_PARAM_DEVICE_BLUETOOTH_ADDR	0x00000011 // ������ַ��ֻ����
// ͬʱ֧�ֽ�����
#define AKEY_PARAM_BITMAP_INFO				0x00001001 // ��ȡCOS��Ϣ��ѯ�����Σ�FID+BITMAP
#define AKEY_PARAM_PUBLICKEY				0x00001002 // ��Կ��ֻ���������Σ�KID
#define AKEY_PARAM_CERT						0x00001003 // ֤�飨ֻ���������棩�����Σ�KID
#define AKEY_PARAM_BIRTHCERT				0x00001004 // ����֤��ֻ���������Σ�KID+[SN]
#define AKEY_PARAM_CERT_USAGE				0x00001005 // ֤����;��ֻ���������棩�����Σ�KID
#define AKEY_PARAM_KEYID_WITH_NAME			0x00001006 // ��ȡKID��ֻ���������棩�����Σ�Type + Len(2) + Name
//#define AKEY_PARAM_PUBLICKEY_NAME			0x00001007 // ��Կ���ƣ���������ֻ���������棩�����Σ�KID
//#define AKEY_PARAM_PRIVATEKEY_NAME			0x00001008 // ˽Կ���ƣ���������ֻ���������棩�����Σ�KID
#define AKEY_PARAM_PUBLICKEY_N				0x00001009 // ��ԿN��ֻ���������Σ�KID
#define AKEY_PARAM_SCREEN_ROTATOIN_CHK		0x00001021 // ��Ļ��ת��⣨���棩�����Σ���ת����(1B)��Ӧ�û���
// ͨ�õĶ��Ʋ���
#define AKEY_PARAM_EXEC_INIT_DEVICE			0x00002001 // ��ʼ���豸�����Σ��û����룬���Σ���
#define AKEY_PARAM_EXEC_CREATE_P10			0x00002002 // ����֤�����󣬽��Σ��û�����(L1V)+keyusage(1)+keysize(1)+hashtype(1)+cn(L1V)+ou(L1V)+o(L1V)�����Σ�keyid(1)+p10(der)
#define AKEY_PARAM_EXEC_WRITE_P7			0x00002003 // д��֤�飬���Σ�keyid(1)+p7(der)�����Σ���
#define AKEY_PARAM_EXEC_SAFE_EXPORT			0x00002004 // ��ȫ������Կ�����Σ�keyid+hashtype + mode(1)+exportkeyid(1)+keysize(2)+keyusage(1)+rand(4)�����Σ���ȫ��Կ����/ǩ��
#define AKEY_PARAM_EXEC_GENERATE_KEY		0x00002005 // ������Կ��p1+keyid(1)+data�����Σ���/ǩ��
// ���⣨������
#define AKEY_PARAM_HIP_VERIFY_PIN           0x00800001 // HIPУ�����루ֻд��
#define AKEY_PARAM_HIP_CHANGE_PIN			0x00800002 // HIP�޸����루ֻд��
#define AKEY_PARAM_OTP_STATUS               0x00800003 // OTP״̬��ֻ����
// ͨѶ��
#define AKEY_PARAM_COMM_FUNCTION1			0xFF000001 // ���ܱ�1
#define AKEY_PARAM_COMM_BATTERY_LEVEL		0xFF000002 // ����
#define AKEY_PARAM_COMM_SHELL_NUM			0xFF000003 // ��Ǻ�
#define AKEY_PARAM_COMM_PROTOCOL			0xFF000004 // ͨѶЭ��
// ��Ŀ���
#define AKEY_PARAM_ICBC_ENPUBLICKEY			0x93120001 // [ICBC]���ܺ�Կ(base64)��ֻ���������Σ�KID+[P2]
#define AKEY_PARAM_ICBC_ADMINKEY_TYPE		0x93120002 // [ICBC]��ȡ��Կ��ϵ���ͣ����Σ�1-����ϵ
#define AKEY_PARAM_ICBC_ENPUBLICKEY_NEW		0x93120003 // [ICBC]���ܺ�Կ(����ϵ)��ֻ���������Σ�KID+KID2+����(2)+�����(15)
#define AKEY_PARAM_SPDB_ENPIN				0x93290001 // [SPDB]��ȡ��������
#define AKEY_PARAM_CMB_PUBLICFILE			0x93270001 // [SMB]�����ļ���ֻ��������Σ�FileId
#define AKEY_PARAM_CMB_PRIVATEFILE			0x93270002 // [SMB]˽���ļ���ֻ��������Σ�FileId + Length(2) + PIN(L1V)
//���֤���
#define AKEY_PARAM_IDCARD_READERCERT        0x99050001 //������֤��(��������֤��SSL֤��)��ʽ��(����֤����[2]+����֤+SSL֤�鳤��[2]+SSL֤��)
#define AKEY_PARAM_IDCARD_COSVERSION        0x99050002 //cos�汾��
#define AKEY_PARAM_IDCARD_COSSPEED          0x99050003 //cos����


// ͨѶЭ�飨ͬ������;�������ն˹淶����
#define AKEY_COMM_PROTOCOL_USB				0
#define AKEY_COMM_PROTOCOL_AUDIO			1
#define AKEY_COMM_PROTOCOL_BLUETOOTH		2
#define AKEY_COMM_PROTOCOL_KCARD			0x81
#define AKEY_COMM_PROTOCOL_LIUMA			0x82

// ���ܱ�ͨѶ�㣩
#define AKEY_COMM_FUNCTION1_DEVICE_KCARD		(1<<0)	// UKEY����KCARDģʽ������ת��ͷͨѶ 

// ���ܱ�TOKEN�㣩
#define AKEY_TOKEN_FUNCTION1_SCREEN_ROTATOIN	(1<<0)	// COS���κ�ģʽ�¶�֧�ַ�תָ��
#define AKEY_TOKEN_FUNCTION1_RCI_USAGE			(1<<1)	// ReadCertInfo���أ�ObjId+KeyId+CertUsage
#define AKEY_TOKEN_FUNCTION1_RCI_CNAME			(1<<2)	// ReadCertInfo���أ�ObjId+KeyId+CertUsage+CType+CName(LV)����ȡ֤��ָ�������

#define AKEY_CHARSET_GBK					0 // GBK
#define AKEY_CHARSET_UTF8					1 // UTF8


#define AKEY_MAX(a, b)            (((a) > (b)) ? (a) : (b))
#define AKEY_MIN(a, b)            (((a) < (b)) ? (a) : (b))



typedef struct _AKEY_SESSION_STATUS	// �Ự״̬
{
	UINT32  bSessionActive : 1;	// ͨѶ�Ự�Ƿ񼤻�
	UINT32  bSessionEncrypt: 1;	// ͨѶ����
	UINT32  bDeviceRand: 1;		// �豸�������Ч
	UINT32  bAutoFreeProtocolObj:1;
	UINT32  u32RFU1 : 28;
	UINT16  u16MaxApdu_Le;		// ����ȡ����
	UINT16  u16MaxApdu_Lc;		// �����³���
	BYTE btSessionEncKeyId;		// ����ͨѶ�ĻỰ��ԿID
	BYTE btSessionEncType;		// ����ͨѶ�ļ������ͣ�0-DES��1-AES
	BYTE abSessionKey[16];		// ����ͨѶ�ĻỰ��Կ
	UINT32 u32PubKey1bNLen;
	BYTE abPubKey1bN[0x100];		// ��Կ
}AKEY_SESSION_STATUS, *AKEY_SESSION_STATUS_PTR;

// ͨѶ���Խ����
typedef struct _AKEY_COMM_TEST_RESULT_ITEM
{
	UINT8 btEncodeIndex; // ���б�������
	UINT8 btDecodeIndex; // ���н�������
	UINT16 unCorrectRate; // �ɹ���(����10000)
}AKEY_COMM_TEST_RESULT_ITEM, *AKEY_COMM_TEST_RESULT_ITEM_PTR;

#endif // #ifndef __AKEY_DEF_H__
