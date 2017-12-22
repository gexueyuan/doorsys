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

// 如果不需要以下类型定义，则在包含本文件前定义AKEY_NODEF_BYTE
#ifndef AKEY_NODEF_BYTE
typedef UINT8 BYTE;
typedef LPUINT8 LPBYTE;
#endif

// 日志宏
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

// 编解码索引
#define AKEY_INDEX_ENCODE_FSK_24_36			8 // 编码
#define AKEY_INDEX_ENCODE_FSK_04_10			7
#define AKEY_INDEX_ENCODE_FSK_04_08			6
#define AKEY_INDEX_ENCODE_FSK_04_06			5
#define AKEY_INDEX_ENCODE_FSK_02_04			4
//#define AKEY_INDEX_ENCODE_OPTIMIZED_LIST	"\xFF\xFF\xFF\xFF\x01\x02\x03\x04\x05\xFF\xFF\xFF\xFF\xFF\xFF\xFF" // 优先列表
#define AKEY_INDEX_DECODE_2FSK_1000			8 // 解码
#define AKEY_INDEX_DECODE_2FSK_5500			7
#define AKEY_INDEX_DECODE_2FSK_6000			6
#define AKEY_INDEX_DECODE_2FSK_2756			5
#define AKEY_INDEX_DECODE_2FSK_2000			4
#define AKEY_INDEX_DECODE_ASK_2000			3
#define AKEY_INDEX_DECODE_ASK_2000_WP		1
#define AKEY_INDEX_DECODE_ASK_4000			9 // 保留
#define AKEY_INDEX_DECODE_ASK_11000			10
#define AKEY_INDEX_DECODE_ASK_12000			11
#define AKEY_INDEX_DECODE_OPTIMIZED_LIST	"\xFF\x08\xFF\x07\x06\x05\x03\x04\x09\xFF\x02\x01\xFF\xFF\xFF\xFF" // 优先列表
#define AKEY_INDEX_ENCODER_MAX_NUMBER		5 // 支持的编码数量
#define AKEY_INDEX_DECODER_MAX_NUMBER		32 // 支持的解码器数量（多解码器）

// 编解码标志
#define AKEY_FLAG_ENCODE_SYNC_PLAY			0x00800000 // 同步播放，用于计算接收超时时间
#define AKEY_FLAG_ENCODE_WAVE_ROTATE180		0x00010000 // 翻转180
#define AKEY_FLAG_ENCODE_WAVE_STEREO		0x00040000 // 立体声
#define AKEY_FLAG_ENCODE_WAVE_PRIORITY		0x01000000 // 优先编码
#define AKEY_FLAG_ENCODE_WAVE_SELECT		0x02000000 // 选择编码
#define AKEY_FLAG_DECODE_WAVE_ROTATE180		0x00010000 // 翻转180
#define AKEY_FLAG_DECODE_WAVE_POLAR			0x00020000 // 指定波形极性
#define AKEY_FLAG_DECODE_WAVE_LEFT_CHANNEL	0x00040000 // 左声道
#define AKEY_FLAG_DECODE_WAVE_RIGHT_CHANNEL	0x00080000 // 右声道
#define AKEY_FLAG_DECODE_WAVE_PRIORITY		0x01000000 // 优先解码
#define AKEY_FLAG_DECODE_WAVE_SELECT		0x02000000 // 选择解码
#define AKEY_FLAG_DECODE_WAVE_PLUGIN		0x04000000 // 使用插件解码
//#define AKEY_FLAG_DECODE_TEST				0x80000000
#define AKEY_FLAG_WAVE_MAP_MASK				0x00007FFF
#define AKEY_FLAG_ENCODE_ECC				0x00008000
#define AKEY_FLAG_DECODE_ECC				0x00008000
#define AKEY_FLAG_ENCODE_VALUE_WITH_INDEX(n) (1<<((n)-1))
#define AKEY_FLAG_DECODE_VALUE_WITH_INDEX(n) (1<<((n)-1))


// TOKEN操作
#define AKKY_FLAG_TOKEN_CHECK_DEVICE		0x00000001	// 检测设备
#define AKKY_FLAG_TOKEN_NEW_DEVICE			0x00000002	// 指定为新设备
#define AKKY_FLAG_TOKEN_CHECK_SESSION		0x00000004	// 检测是否要新建通道
#define AKKY_FLAG_TOKEN_NEW_SESSION			0x00000008	// 强制新建通道
#define AKKY_FLAG_TOKEN_PREPARE_MASK		0x000000FF	// PREPARE专有
#define AKKY_FLAG_TOKEN_PINID2				0x00010000	// 使用2号PIN
//#define AKKY_FLAG_TOKEN_USE_CACHE_RAND		0x00020000	// 使用上次缓存的随机数
#define AKKY_FLAG_TOKEN_SN_ENDCHAR0			0x00040000	// 外壳号包含结束符0（主要用于读取出生证）
#define AKKY_FLAG_TOKEN_NOT_USE_CACHE		0x01000000	// 不使用缓存（目前用于读参数）
#define AKKY_FLAG_TINIT_CHECKED_DEVICE		0x00000001	// 已检测设备
#define AKKY_FLAG_TINIT_SUPPORT_ENCRYPTAPDU	0x00000100	// 支持加密指令，init
#define AKKY_FLAG_TINIT_MUST_ENCRYPTAPDU	0x00000200	// 必须使用加密指令，init
#define AKKY_FLAG_TINIT_NOT_LOAD_PAIRKEY1B	0x00000400	// 不加载1B号密钥，init，如果更改该配置需要删除缓存文件才作用
#define AKKY_FLAG_TINIT_MUST_SESSION		0x00000800	// 执行任何指令前必须先建立通道

// 摘要类型
#define AKEY_HASH_SHA1						0x00
#define AKEY_HASH_MD5						0x01
#define AKEY_HASH_SHA256					0x02
#define AKEY_HASH_SHA384					0x03
#define AKEY_HASH_SHA512					0x04
#define AKEY_HASH_MD5SHA					0x05
#define AKEY_HASH_SM3						0x06
#define AKEY_HASH_SHA256_SM3				0x07
#define AKEY_HASH_FROM_LENGTH(l)			(((l) == 20)? AKEY_HASH_SHA1 : ((l) >> 4))

// 签名标志（一个字节的摘要类型）
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
//#define AKEY_FLAG_SIGN_NOCHECKINTEGRIRY		0x00020000	// COS内部不检测密钥对完整性
#define AKEY_FLAG_SIGN_GETRESULT_SCANKEY	0x00040000	// 取签名的结果，同时兼顾扫描按键
#define AKEY_FLAG_SIGN_BLUETOOTH_ADDRESS	0x00080000	// 蓝牙通讯地址
#define AKEY_FLAG_SIGN_DEVELOP_DATA			0x00100000	// 强制使用Develop封装数据
#define AKEY_FLAG_SIGN_TYPE_MASK			0xFF000000
#define AKEY_FLAG_SIGN_TYPE_DOUBLEHASH		(0x01 << 24)	// 双摘要签名
#define AKEY_FLAG_SIGN_TYPE_SHOWDATA		(0x02 << 24)	// 直接显示签名
#define AKEY_FLAG_SIGN_TYPE_HASHRELAY		(0x03 << 24)	// 摘要接力签名
#define AKEY_FLAG_SIGN_TYPE_HASHRELAY2		(0x04 << 24)	// 摘要接力签名2
// P7
#define AKEY_FLAG_P7_HASH_MASK				0x000000FF
#define AKEY_FLAG_P7_SM						0x00010000	// 国密
#define AKEY_FLAG_P7_ORG_OID				0x00020000	// 原有OID没有使用国密OID）：ICBC
#define AKEY_FLAG_P7_ABC					0x00040000	// 签名OID+签名结果直接用结构存储

// 算法标志
#define AKEY_FLAG_CIPHER_ENCRYPT			0x00000001	// 加密
#define AKEY_FLAG_CIPHER_DECRYPT			0x00000000	// 解密
#define AKEY_FLAG_CIPHER_NOPADDING			0x00010000	// NOPADDING
#define AKEY_FLAG_RSA_NOPADDING				0x00010000	// NOPADDING
#define AKEY_FLAG_RSA_PUBLICDECRYPT			0x00100000	// 公钥解密

// 参数
#define AKEY_PARAM_DEVICE_LABLE				0x00000001 // 卷标
#define AKEY_PARAM_DEVICE_SERIAL_NUMBER		0x00000002 // 外壳号（只读）
#define AKEY_PARAM_CHARSET					0x00000003 // 字符编码
#define AKEY_PARAM_LANGID					0x00000004 // 语言ID
#define AKEY_PARAM_CHARSET_LIST				0x00000005 // 字符编码列表（只读）
#define AKEY_PARAM_LANGID_LIST				0x00000006 // 语言ID列表（只读）
#define AKEY_PARAM_USERPIN_ATTR				0x00000007 // 用户的密码属性（只读）（不建议使用）
#define AKEY_PARAM_USERPIN_TIMES			0x00000008 // 用户的密码最大尝试数次+剩余数次（内存）（只写最大值）
#define AKEY_PARAM_CERTKEYID_LIST			0x00000009 // 证书关联密钥Id列表（只读）（缓存）
#define AKEY_PARAM_DEVICE_RAND				0x0000000A // 随机数（从设备读，写缓存）
#define AKEY_PARAM_COSINFO					0x0000000B // COS信息（只读）
#define AKEY_PARAM_LANGID_CHARSET			0x0000000C // 语言ID+字符编码（只读）
#define AKEY_PARAM_SIGN_PACKET_SZIE			0x0000000D // 获取签名报文的分组包大小+最大包大小（只读）
#define AKEY_PARAM_SCREEN_ROTATOIN			0x0000000E // 屏幕翻转（只写），格式：00-每次都翻转，80-翻转到正向，81-翻转到反向
#define AKEY_PARAM_DEVICE_CUSTOMID			0x0000000F // 客户号（只读）
#define AKEY_PARAM_CERT_INFO_LIST			0x00000010 // 证书信息列表（OID + KID + Usage + Type + Len(2) + Name）（只读）（缓存）
#define AKEY_PARAM_DEVICE_BLUETOOTH_ADDR	0x00000011 // 蓝牙地址（只读）
// 同时支持进出参
#define AKEY_PARAM_BITMAP_INFO				0x00001001 // 获取COS信息查询表，进参：FID+BITMAP
#define AKEY_PARAM_PUBLICKEY				0x00001002 // 公钥（只读），进参：KID
#define AKEY_PARAM_CERT						0x00001003 // 证书（只读）（缓存），进参：KID
#define AKEY_PARAM_BIRTHCERT				0x00001004 // 出生证（只读），进参：KID+[SN]
#define AKEY_PARAM_CERT_USAGE				0x00001005 // 证书用途（只读）（缓存），进参：KID
#define AKEY_PARAM_KEYID_WITH_NAME			0x00001006 // 获取KID（只读）（缓存），进参：Type + Len(2) + Name
//#define AKEY_PARAM_PUBLICKEY_NAME			0x00001007 // 公钥名称（容器）（只读）（缓存），进参：KID
//#define AKEY_PARAM_PRIVATEKEY_NAME			0x00001008 // 私钥名称（容器）（只读）（缓存），进参：KID
#define AKEY_PARAM_PUBLICKEY_N				0x00001009 // 公钥N（只读），进参：KID
#define AKEY_PARAM_SCREEN_ROTATOIN_CHK		0x00001021 // 屏幕翻转检测（缓存），进参：翻转类型(1B)，应用缓存
// 通用的定制参数
#define AKEY_PARAM_EXEC_INIT_DEVICE			0x00002001 // 初始化设备，进参：用户密码，出参：无
#define AKEY_PARAM_EXEC_CREATE_P10			0x00002002 // 创建证书请求，进参：用户密码(L1V)+keyusage(1)+keysize(1)+hashtype(1)+cn(L1V)+ou(L1V)+o(L1V)，出参：keyid(1)+p10(der)
#define AKEY_PARAM_EXEC_WRITE_P7			0x00002003 // 写入证书，进参：keyid(1)+p7(der)，出参：无
#define AKEY_PARAM_EXEC_SAFE_EXPORT			0x00002004 // 安全导出公钥，进参：keyid+hashtype + mode(1)+exportkeyid(1)+keysize(2)+keyusage(1)+rand(4)，出参：安全公钥数据/签名
#define AKEY_PARAM_EXEC_GENERATE_KEY		0x00002005 // 生成密钥：p1+keyid(1)+data，出参：无/签名
// 特殊（待整理）
#define AKEY_PARAM_HIP_VERIFY_PIN           0x00800001 // HIP校验密码（只写）
#define AKEY_PARAM_HIP_CHANGE_PIN			0x00800002 // HIP修改密码（只写）
#define AKEY_PARAM_OTP_STATUS               0x00800003 // OTP状态（只读）
// 通讯层
#define AKEY_PARAM_COMM_FUNCTION1			0xFF000001 // 功能表1
#define AKEY_PARAM_COMM_BATTERY_LEVEL		0xFF000002 // 电量
#define AKEY_PARAM_COMM_SHELL_NUM			0xFF000003 // 外壳号
#define AKEY_PARAM_COMM_PROTOCOL			0xFF000004 // 通讯协议
// 项目相关
#define AKEY_PARAM_ICBC_ENPUBLICKEY			0x93120001 // [ICBC]加密后公钥(base64)（只读），进参：KID+[P2]
#define AKEY_PARAM_ICBC_ADMINKEY_TYPE		0x93120002 // [ICBC]获取密钥体系类型；出参：1-新体系
#define AKEY_PARAM_ICBC_ENPUBLICKEY_NEW		0x93120003 // [ICBC]加密后公钥(新体系)（只读），进参：KID+KID2+类型(2)+随机数(15)
#define AKEY_PARAM_SPDB_ENPIN				0x93290001 // [SPDB]获取密码密文
#define AKEY_PARAM_CMB_PUBLICFILE			0x93270001 // [SMB]公有文件（只读），入参：FileId
#define AKEY_PARAM_CMB_PRIVATEFILE			0x93270002 // [SMB]私有文件（只读），入参：FileId + Length(2) + PIN(L1V)
//身份证相关
#define AKEY_PARAM_IDCARD_READERCERT        0x99050001 //读卡器证书(包括出生证和SSL证书)格式：(出生证长度[2]+出生证+SSL证书长度[2]+SSL证书)
#define AKEY_PARAM_IDCARD_COSVERSION        0x99050002 //cos版本号
#define AKEY_PARAM_IDCARD_COSSPEED          0x99050003 //cos加速


// 通讯协议（同《多用途互联网终端规范》）
#define AKEY_COMM_PROTOCOL_USB				0
#define AKEY_COMM_PROTOCOL_AUDIO			1
#define AKEY_COMM_PROTOCOL_BLUETOOTH		2
#define AKEY_COMM_PROTOCOL_KCARD			0x81
#define AKEY_COMM_PROTOCOL_LIUMA			0x82

// 功能表（通讯层）
#define AKEY_COMM_FUNCTION1_DEVICE_KCARD		(1<<0)	// UKEY采用KCARD模式与音码转接头通讯 

// 功能表（TOKEN层）
#define AKEY_TOKEN_FUNCTION1_SCREEN_ROTATOIN	(1<<0)	// COS在任何模式下都支持翻转指令
#define AKEY_TOKEN_FUNCTION1_RCI_USAGE			(1<<1)	// ReadCertInfo返回：ObjId+KeyId+CertUsage
#define AKEY_TOKEN_FUNCTION1_RCI_CNAME			(1<<2)	// ReadCertInfo返回：ObjId+KeyId+CertUsage+CType+CName(LV)，读取证书指令不依赖它

#define AKEY_CHARSET_GBK					0 // GBK
#define AKEY_CHARSET_UTF8					1 // UTF8


#define AKEY_MAX(a, b)            (((a) > (b)) ? (a) : (b))
#define AKEY_MIN(a, b)            (((a) < (b)) ? (a) : (b))



typedef struct _AKEY_SESSION_STATUS	// 会话状态
{
	UINT32  bSessionActive : 1;	// 通讯会话是否激活
	UINT32  bSessionEncrypt: 1;	// 通讯加密
	UINT32  bDeviceRand: 1;		// 设备随机数有效
	UINT32  bAutoFreeProtocolObj:1;
	UINT32  u32RFU1 : 28;
	UINT16  u16MaxApdu_Le;		// 最大读取长度
	UINT16  u16MaxApdu_Lc;		// 最大更新长度
	BYTE btSessionEncKeyId;		// 加密通讯的会话密钥ID
	BYTE btSessionEncType;		// 加密通讯的加密类型；0-DES，1-AES
	BYTE abSessionKey[16];		// 加密通讯的会话密钥
	UINT32 u32PubKey1bNLen;
	BYTE abPubKey1bN[0x100];		// 公钥
}AKEY_SESSION_STATUS, *AKEY_SESSION_STATUS_PTR;

// 通讯测试结果项
typedef struct _AKEY_COMM_TEST_RESULT_ITEM
{
	UINT8 btEncodeIndex; // 下行编码索引
	UINT8 btDecodeIndex; // 上行解码索引
	UINT16 unCorrectRate; // 成功率(满分10000)
}AKEY_COMM_TEST_RESULT_ITEM, *AKEY_COMM_TEST_RESULT_ITEM_PTR;

#endif // #ifndef __AKEY_DEF_H__
