#ifndef __AKEY_ERROR_H__
#define __AKEY_ERROR_H__
//#pragma once


// 返回值定义
//###############################以下为通用错误###############################
#define	AKEY_RV_OK								0x00000000
#define	AKEY_RV_FAIL							0xE010FFFF
#define AKEY_RV_OUT_MEMORY						0xE0100001 // 内存不足
#define AKEY_RV_USER_CANCEL						0xE0100002 // 用户取消
#define AKEY_RV_OUTOFBUFFER						0xE0100003 // 内部缓冲区不够
#define AKEY_RV_BUFFERTOOSMALL					0xE0100004 // 传入缓冲区不足
//###############################以下为通讯层错误###############################
#define	AKEY_RV_COMM_TIMEOUT					0xE0101001 // 超时
#define	AKEY_RV_COMM_WRONG_STAT					0xE0101002 // 错误状态
#define	AKEY_RV_COMM_WRONG_IBLOCK				0xE0101003 // 错误I包数据
#define AKEY_RV_COMM_PLAY_ERROR					0xE0101101 // 播放错误
#define	AKEY_RV_COMM_NO_RECORD_DATA				0xE0101201 // 没有录音数据
#define AKEY_RV_COMM_NO_MORE_RECORD_DATA		0xE0101202 // 录音数据不足（实际应用中不会出现错误）
#define AKEY_RV_COMM_DECODE_STATE_0				0xE0101300 // 解码状态（+1同步域，+2起始域，+4首字节）
#define AKEY_RV_COMM_DECODE_STATE_1				0xE0101301
#define AKEY_RV_COMM_DECODE_STATE_3				0xE0101303
#define AKEY_RV_COMM_DECODE_STATE_7				0xE0101307
#define AKEY_RV_COMM_INCORRECT_DATA				0xE0101401 // 数据错误（数据不足）
#define	AKEY_RV_COMM_INCORRECT_CRC				0xE0101402 // 校验位错误
#define AKEY_RV_COMM_INCORRECT_DSN				0xE0101403 // 不正确的设备序号
#define AKEY_RV_COMM_INCORRECT_HSN				0xE0101404 // 不正确的主机序号
#define AKEY_RV_COMM_INCORRECT_PID				0xE0101405 // 不正确的PID或PORT
#define AKEY_RV_COMM_RECVICE_OK					0xE010140F // 上行通讯OK
#define AKEY_RV_COMM_DECODE_BETWEEN(hr)			((hr) >= AKEY_RV_COMM_DECODE_STATE_7) && ((hr) <= AKEY_RV_COMM_INCORRECT_CRC)
#define AKEY_RV_COMM_CHECK_STAGE_FLAG			0x00010000 // 检测阶段错误

//###############################以下为会话层错误###############################
#define	AKEY_RV_SESSION_INVALID_CODE			0xE0E06A80 // 连接码错(蓝牙)

//###############################以下为Token及存储层错误###############################
#define	AKEY_RV_TOKEN_NOT_FOUND_DEVICE			0xE0102001 // 没有找到设备
#define	AKEY_RV_TOKEN_NOT_MATCH_DEVICE			0xE0102002 // 不匹配设备（外壳号不同）
#define AKEY_RV_TOKEN_NOT_FOUND_CERT			0xE0102003 // 没有证书
#define	AKEY_RV_TOKEN_NOT_MATCH_CERT			0xE0102004 // 不匹配证书（CN不同）
#define AKEY_RV_TOKEN_INVALID_PARAM				0xE0102005 // 无效参数
#define AKEY_RV_TOKEN_INVALID_PAIRKEY			0xE0102006 // 无效密钥
#define AKEY_RV_TOKEN_INVALID_CUSTOMID			0xE0102007 // 无效客户ID号
#define AKEY_RV_TOKEN_INVALID_DATA				0xE0102008 // 无效数据(应用)
#define AKEY_RV_TOKEN_INVALID_CACHE				0xE0102009 // 存储数据被破坏
#define AKEY_RV_TOKEN_NOT_FOUND_CERTINFO		0xE010200A // 没有证书信息
#define AKEY_RV_TOKEN_NOT_FOUND_OBJECT			0xE010200B // 没有对象（P11）
#define AKEY_RV_TOKEN_NOF_FOUND_FILE			0xE010200C // 没有文件（User）
#define AKEY_RV_TOKEN_DIRTY_DATA				0xE0103001 // 脏数据（需要重新加载）
#define AKEY_RV_TOKEN_OUT_MEMORY				0xE0103002 // 存储空间不足

//###############################以下为指令错误###############################
#define	AKEY_RV_SW_BASE							0xE0E00000 // 执行指令的SW错误
#define	AKEY_RV_SW_CARD_BASE					0xE0E10000 // 执行指令的SW错误（卡片）
#define AKEY_RV_SW_PENDING						0xE0E09001 // 需要用户交互
#define	AKEY_RV_SW_CONFIRM						0xE0E0900A // 用户确认
#define	AKEY_RV_SW_CANCEL						0xE0E0900C // 用户取消
#define	AKEY_RV_SW_TIMEOUT						0xE0E0900F // 等待按键超时
#define AKEY_RV_SW_INCORRECT_PIN_BASE			0xE0E063C0 // 密钥错误, &0F=剩余密码尝试次数
#define AKEY_RV_SW_INCORRECT_PIN_MASK			0xFFFFFFF0 // 密码错误掩码
#define AKEY_RV_SW_PIN_LOCK						0xE0E06983 // 密码锁定
#define AKEY_RV_SW_DEFAULT_PIN					0xE0E06982 // 条件不满足（默认密码）
//#define AKEY_RV_SW_DIRTY_DATA					0xE0E06926 // 对公钥加密结果解码失败（公钥不对）
#define AKEY_RV_SW_READER_TIMEOUT				0xE0E06FFD // 内置读卡器操作超时




//###############################以下为应用层错误###############################
#define AKEY_RV_APP_COMMUNICATION_INTERRUPT		0xE0201001 // 通信中断
#define AKEY_RV_APP_SIGN_FAILED                 0xE0201002 // 签名失败
#define AKEY_RV_APP_RECORDPERMISSION            0xE0201004 // 麦克风权限 (不要修改)

///////格式：时间数据（以ms为单位16）
// 0000 + 00000000
// 0001 -----硬件返回的检测到蓝牙设备的时间
// 0002 -----手机蓝牙checkcard 时间 -- ms
// 0003 -----连接server的时间
// 0004 -----获取随机数的时间(0x01的app 0x01)
// 0005 -----获取端口号的时间(0x01的app 0x02)
// 0006 -----连接端口服务器时间（以新的port连接的服务器ip地址不变）
// 0007 -----释放端口的时间(0x01的app 0x03)
// 0008 -----建立安全通道时间（0x02的app 0x01）
// 0009 -----验证签名时间（0x02的app 0x02）
// 000A -----总的指令交互时间（包括网络时间和与读卡器交互时间，0x02的app 0x03）
// 000B -----手机与读卡器指令交互时间（所有的指令）
//

#endif // #ifndef __AKEY_ERROR_H__
