//
//  IDCardTags.h
//  TDRToken
//
//  Created by zhaowei on 16/6/5.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardTags_h
#define IDCardTags_h

//主标签
#define     TAG_TRADE_CODE                          0x0001   //交易码
#define     TAG_BATCH_NUMBER                        0x0002   //批次号
#define     TAG_TRANSACTION_NUMBER                  0x0003   //参考号
#define     TAG_READER_SERIAL_NUMBER                0x0004   //读卡器序号
#define     TAG_CUSTOM_ID                           0x0005   //客户号
#define     TAG_ERROR_CODE                          0x0006   //错误码
#define     TAG_SEQ_NUMBER                          0x0007   //流水号
#define     TAG_SAM_PORT                            0x0008   //网卡端口号
#define     TAG_SAM_INDEX                           0x0009   //位置编号 sam索引
#define     TAG_CLIENT_OPTION                       0x000A   //客户端选项
#define     TAG_SERVER_OPTION                       0x000B   //服务端选项

#define     TAG_SAM_CONTROLLER_VERSION				0x0019   //网卡模组编号
#define		TAG_UNIQUE_CODE							0x001B   //唯一链号
#define		TAG_NETCOMM_RETRY_TIME					0x001C	 //重发次数 （双向）

#define     TAG_DEVICE_FLOW                         0x0081   //设备指令流
#define     TAG_IMAGE                               0x0082   //图片密文数据
#define     TAG_TIME_LOG                            0x0083   //时间统计
#define     TAG_LOG_DATA                            0x0084   //日志数据
#define		TAG_ENC_READER_SN						0x0085	 //加密读卡器序列号


#define     TAG_DEV_STATUS                          0x0101   //设备状态
#define     TAG_DEV_SERIAL_NUMBER                   0x0102   //设备序列号(同读卡器序号)
#define     TAG_DEV_READER_CERT                     0x0103   //同读卡器证书
#define     TAG_DEV_SAM_CERT                        0x0104   //控制器证书
#define     TAG_DEV_SAM_RANDOM                      0x0105   //控制器随机数
#define     TAG_DEV_ENC_SIGN_DATA                   0x0106   //设备加密签名数据
#define     TAG_DEV_SINGLE_INS                      0x0107   //单设备指令
#define     TAG_DEV_SINGLE_INS_RESPONSE             0x0108   //单指令响应
#define     TAG_DEV_MUL_INS                         0x0109   //多设备指令
#define     TAG_DEV_MUL_INS_RESPONSE                0x010A   //多设别指令响应
#define     TAG_DEV_PROCESS_ENC_KEY                 0x010B   //过程密钥密文
#define     TAG_DEV_ID_CIPHER_TEXT                  0x010C   //身份证信息密文
#define     TAG_DEV_ERROR_CODE                      0x010D   //设备错误码
#define     TAG_DEV_SAM_CERT_VERSION                0X010E   //控制器证书版本号
#define     TAG_DEV_READER_CERT_VERSION             0x010F   //读卡器证书版本号
#define     TAG_DEV_CHANNEL_VALID_VERIFY_CODE       0x0110   //通道有效验证码


#define     TAG_IMAGE_PROGRESS_KEY                  0x0203   //过程密钥
#define     TAG_IMAGE_ENCED_IMAGE_DATA              0x0204   //加密照片

#define		TAG_IMAGE_AVAIABLE						0x0209   //是否有照片

//日志数据标签
#define     TAG_LOG_SWITCH                          0x0701   //日志开关
#define     TAG_LOG_READER_SERIAL_NUMBER            0x0702   //Reader 序号
#define     TAG_LOG_MOBILE_UNIQUE_IDENTIFY          0x0703   //手机唯一编号
#define     TAG_LOG_HEZI_SERIAL_NUMBER              0x0704   //盒子序号
#define     TAG_LOG_SAM_SERIAL_NUMBER               0x0705   //SAM的序列号
#define     TAG_LOG_ID_NUMBER                       0x0706   //身份证号
#define     TAG_LOG_READER_HARDWARE_VERSION         0x0707   //reader的硬件版本号
#define     TAG_LOG_READER_SOFEWARE_VERSION         0x0708   //reader的软件版本号

#define     TAG_LOG_APP_VERSION                     0x070B   //App软件版本号
#define     TAG_LOG_SYSTEM_INFO                     0x070C   //手机操作系统
#define     TAG_LOG_PHONE_MODEL                     0x070D   //手机型号
#define     TAG_LOG_PHONE_MAC                       0x070E   //手机MAC
#define     TAG_LOG_PHONE_IP_ADDRESS                0x070F   //手机ip地址

#define     TAG_LOG_NET_APP_HANDLE                  0x071B   //网卡模组应用处理的时间
#define     TAG_LOG_SAFE_COM                        0x071C   //与安全模块通讯时间



//时间统计
#define     TAG_LOG_READER_CMD_TIME                 0x0301   //读卡器指令时间
#define     TAG_LOG_NETWORK_TIME                    0x0302   //客户端与服务器的网络时间
#define     TAG_LOG_TOTAL_TASK_TIME                 0x0303   //总时间
#define     TAG_LOG_SAM_TIME                        0x0304   //网卡模组时间日志
#define     TAG_LOG_APP_INTERFACE_TIME              0x0305   //App客户端接口间时间
#define     TAG_LOG_APP_CONNECT_TIME				0x0306   //连接服务器时间
#define	    TAG_LOG_APP_DISCONNECT_TIME				0x0307   //断开与服务器连接时间
#define     TAG_LOG_APP_RETRY_COUNT                 0x0308   //重试次数统计[格式:通信类型1+总网络重试次数次数+读卡器重试次数1+网络重试次数1+蓝牙重试次数1][共4字节]
#define		TAG_LOG_SDK_VERSION						0x0309   //SDK版本号


#define     TAG_ENCRYPT_TEXT                        0x8001   //密文数据
#define		TAG_ENCRYPT_TEXT2						0x8002   //密文数据2

#define     TAG_GETPORT_READER_SN                   0x0C01   //Reader序号
#define     TAG_GETPORT_GET_TIME                    0x0C02   //申请端口时间
#define     TAG_GETPORT_NEW_IP                      0x0C04   //申请到的网卡模组ip
#define     TAG_GETPORT_NEW_PORT                    0x0C03   //申请到的网卡模组端口
#define     TAG_GETPORT_RELEASE_TIME                0x0C05   //释放端口时间

#define		TAG_IDCARD_NOTE_INFO					0x0D02   //身份证票据信息

#define     TAG_SAM_WAITNUM                         0x0E01   //等待Sam的排队人数

#define     TRADECODE_BUILDSAFE       0x0101


#endif /* IDCardTags_h */
