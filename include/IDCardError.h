//
//  IDReaderError.h
//  TDRToken
//
//  Created by zhaowei on 2016/7/12.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDReaderError_h
#define IDReaderError_h

//###############################身份证错误码###############################

#define AKEY_RV_SERVER_CONNECT_ERROR            0xE0200001 //连接服务器失败
#define AKEY_RV_SERVER_SEND_ERROR               0xE0200002 //往服务器发送数据失败
#define AKEY_RV_SERVER_RECV_ERROR               0xE0200003 //从服务器接收数据失败
#define AKEY_RV_MAC_VERIFY_ERROR                0xE0200004 //数据包mac校验错误
#define AKEY_RV_NO_INIT                         0xE0200005 //没有初始化参数
#define AKEY_RV_BITMAP_LENGTH_ERROR             0xE0200006 //bitmap返回的数据长度错误
#define AKEY_RV_PARSE_PROCESS_STATUS_ERROR      0xE0200007 //解析的过程状态码错误
#define AKEY_RV_PACKAGE_PROCESS_STATUS_ERROR    0xE0200008 //组包的过程状态码错误
#define AKEY_RV_CACHE_NO_SAM_CERT               0xE0200009 //缓存中获取控制器证书错误
#define AKEY_RV_CACHE_NO_SAM_CERT_VERSION       0xE020000A //缓存中获取控制器证书版本号错误
#define AKEY_RV_DEC_KEY_CIPHER_ERROR            0xE020000B //解密加密密文错误
#define AKEY_RV_OPEN_FILE_ERROR                 0xE020000C //打开文件失败
#define AKEY_RV_SM4_ENC_ERROR                   0xE020000D //sm4加密失败
#define AKEY_RV_DEC_ENC_DATA_ERROR              0xE020000E //解密加密数据失败
#define AKEY_RV_IDINFO_HASH_NO_MATCH            0xE020000F //身份证信息的hash值不匹配
#define AKEY_RV_PARAM_ERROR                     0xE0200010 //参数错误
#define AKEY_RV_READING_IN_PROGRESS             0xE0200011 //读证正在进行，需等待
#define AKEY_RV_PARAM_IP_PORT_ERROR             0xE0200012 //ip地址或端口号参数错误
#define AKEY_RV_PARAM_ERR_FILE_NAME             0xE0200013 //错误日志文件名称为空
#define AKEY_RV_SERVER_RECV_TIMEOUT             0xE0200014 //接收服务器数据超时
#define AKEY_RV_DN_NO_COMPARE                   0xE0200015 //接收的DN与缓存不匹配
#define AKEY_RV_NOT_SUPPORT_RESUME				0xE0200016 //不支持断点续传
#define AKEY_RV_NOT_SUPPORT_BATTERY				0xE0200017 //不支持电量获取
#define AKEY_RV_SOCKET_RECONNECT_INVALID        0xE0200018 //网络重建失败
#define AKEY_RV_SOCKET_RECV_RETRY_EXCEED        0xE0200019 //网络重试次数超出(接收数据)
#define AKEY_RV_SOCKET_CONN_RETRY_EXCEED        0xE0200020 //网络重试次数超出(连接服务器)
#define AKEY_RV_READER_RETRY_EXCEED             0xE0200021 //读卡器重试次数超出
#define AKEY_RV_CONNECT_DISPORT_SERVER_ERROR    0xE0200022 //连接调度服务器失败
#define AKEY_RV_CONNECT_SAM_SERVER_ERROR        0xE0200023 //连接网卡模组服务器失败
#define AKEY_RV_UDP_CHECK_TIME_OUT				0xE0200024 //UDP超时检查
#define AKEY_RV_SOCKET_IP_PORT_PARAM_ERROR      0xE0200025 //设置socket的参数错误
#define AKEY_RV_UNIQUENUM_NOT_MATCH				0xE0200026 //唯一码不匹配
#define AKEY_RV_IDWORK_CANCELED					0xE0200027 //读证任务被用户取消
#define AKEY_RV_IDWORK_TIMEOUT					0xE0200028 //读证任务超时
#define AKEY_RV_PACKAGE_COMM_TIMEOUT			0xE0200029 //包通信的超时时间，包括udp和tcp
#define AKEY_RV_SOCKET_COMM_TIMEOUT				0xE0200030 //socket通信超时
#define AKEY_RV_APP_HASH_KEY_UNAVAIABLE			0xE0200031 //AppHashKey被清除
#define AKEY_RV_WAITSAM_TIMEOUT                 0xE0200032 //Sam排队超时
#define AKEY_RV_WAITSAM_NOCARD                  0xE0200033 //Sam排队中无卡


#define AKEY_RV_BUILD_SAFECHANNEL_NO_TRADECODE          0xE0200101 //建立安全通道1的交易码不存在
#define AKEY_RV_BUILD_SAFECHANNEL_TRADECODE_ERROR       0xE0200102 //建立安全通道1的交易码错误
#define AKEY_RV_BUILD_SAFECHANNEL_NO_BATCH              0xE0200103 //建立安全通道1的批次号不存在
#define AKEY_RV_BUILD_SAFECHANNEL_NO_TRANSTION_NUM      0xE0200104 //建立安全通道1的参考号不存在
#define AKEY_RV_BUILD_SAFECHANNEL_NO_SEQ_NUM            0xE0200105 //建立安全通道1的流水号不存在
#define AKEY_RV_BUILD_SAFECHANNEL_SEQ_NUM_ERROR         0xE0200106 //建立安全通道1的流水号错误
#define AKEY_RV_BUILD_SAFECHANNEL_NO_NET_MODULE         0xE0200107 //建立安全通道1的网卡模组编号不存在
#define AKEY_RV_BUILD_SAFECHANNEL_NO_DEVICE_FLOW        0xE0200108 //建立安全通道1的设备指令流不存在
#define AKEY_RV_BUILD_SAFECHANNEL_NO_TIME_LOG           0xE0200109 //建立安全通道1的时间统计不存在
#define AKEY_RV_BUILD_SAFECHANNEL_NO_NET_TIME_LOG       0xE020010A //建立安全通道1的网卡模组应用处理时间统计不存在
#define AKEY_RV_BUILD_SAFECHANNEL_NO_COM_TIME_LOG       0xE020010B //建立安全通道1的与安全模块通讯时间统计不存在
#define AKEY_RV_BUILD_SAFECHANNEL_NO_ERRORCODE          0xE020010C //建立安全通道1的错误码不存在

#define AKEY_RV_BUILD2_SAFECHANNEL_NO_TRADECODE         0xE0200201 //建立安全通道2的交易码不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_TRADECODE_ERROR      0xE0200202 //建立安全通道2的交易码错误
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_BATCH             0xE0200203 //建立安全通道2的批次号不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_TRANSTION_NUM     0xE0200204 //建立安全通道2的参考号不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_SEQ_NUM           0xE0200205 //建立安全通道2的顺序号不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_SEQ_NUM_ERROR        0xE0200206 //建立安全通道2的顺序号错误
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_NET_MODULE        0xE0200207 //建立安全通道2的网卡模组编号不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_DEVICE_FLOW       0xE0200208 //建立安全通道2的设备指令流不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_TIME_LOG          0xE0200209 //建立安全通道2的时间统计不存在
#define AKey_RV_BUILD2_SAFECHANNEL_NO_ENC_TEXT          0xE020020A //建立安全通道2的加密数据不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_DEC_TEXT_ERROR       0xE020020B //建立安全通道2的解密数据失败
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_NET_TIME_LOG      0xE020020C //建立安全通道2的网卡模组应用处理时间统计不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_COM_TIME_LOG      0xE020020D //建立安全通道2的与安全模块通讯时间统计不存在
#define AKEY_RV_BUILD2_SAFECHANNEL_NO_ERRORCODE         0xE020020E //建立安全通道2的错误码不存在


#define AKEY_RV_INS_EXCHANGE_NO_TRADECODE               0xE0200301 //指令交互的交易码不存在
#define AKEY_RV_INS_EXCHANGE_TRADECODE_ERROR            0xE0200302 //指令交互的交易码错误
#define AKEY_RV_INS_EXCHANGE_NO_BATCH                   0xE0200303 //指令交互的批次号不存在
#define AKEY_RV_INS_EXCHANGE_NO_TRANSTION_NUM           0xE0200304 //指令交互的流水号不存在
#define AKEY_RV_INS_EXCHANGE_NO_SEQ_NUM                 0xE0200305 //指令交互的顺序号不存在
#define AKEY_RV_INS_EXCHANGE_SEQ_NUM_ERROR              0xE0200306 //指令交互的顺序号错误
#define AKEY_RV_INS_EXCHANGE_NO_NET_MODULE              0xE0200307 //指令交互的网卡模组编号不存在
#define AKEY_RV_INS_EXCHANGE_NO_DEVICE_FLOW             0xE0200308 //指令交互的设备指令流不存在
#define AKEY_RV_INS_EXCHANGE_NO_TIME_LOG                0xE0200309 //指令交互的时间统计不存在
#define AKey_RV_INS_EXCHANGE_NO_ENC_TEXT                0xE020030A //指令交互的加密数据不存在
#define AKEY_RV_INS_EXCHANGE_DEC_TEXT_ERROR             0xE020030B //指令交互的解密数据失败
#define AKEY_RV_INS_EXCHANGE_NO_NET_TIME_LOG            0xE020030C //指令交互的网卡模组应用处理时间统计不存在
#define AKEY_RV_INS_EXCHANGE_NO_COM_TIME_LOG            0xE020030D //指令交互的与安全模块通讯时间统计不存在
#define AKEY_RV_INS_EXCHANGE_NO_DEVICE_STATUS           0xE020030E //指令交互的设备指令流中状态不存在
#define AKEY_RV_INS_EXCHANGE_NO_SAM_CERT                0xE020030F //指令交互的设备指令流中控制器证书不存在
#define AKEY_RV_INS_EXCHANGE_NO_SAM_RANDOM              0xE0200310 //指令交互的设备指令流中控制器随机数不存在
#define AKEY_RV_INS_EXCHANGE_APPKEY_LENGTH_ERROR        0xE0200311 //指令交互的设备指令流中AppKey数据长度错误
#define AKEY_RV_INS_EXCHANGE_PROGRESS_STATUS_ERROR      0xE0200312 //指令交互的设备指令流中状态值错误
#define AKEY_RV_INS_EXCHANGE_NO_ERROR_CODE              0xE0200313 //指令交互的设备指令流中没有错误码
#define AKEY_RV_INS_EXCHANGE_NO_ENC_ID_INFO             0xE0200314 //身份证的加密文本数据不存在
#define AKEY_RV_INS_EXCHANGE_NO_SINGLE_CMD              0xE0200315 //指令交互的设备指令流中没有单指令数据
#define AKEY_RV_INS_EXCHANGE_NO_MUL_CMD                 0xE0200316 //指令交互的设备指令流中没有多指令数据
#define AKEY_RV_INS_EXCHANGE_DEV_FLOW_NO_ERR            0xE0200317 //指令交互的设备指令流中没有错误码的标示

#define AKEY_RV_IMAGE_NO_TRADECODE                      0xE0200401 //请求图片的交易码不存在
#define AKEY_RV_IMAGE_TRADECODE_ERROR                   0xE0200402 //请求图片的交易码错误
#define AKEY_RV_IMAGE_NO_BATCH                          0xE0200403 //请求图片的批次号不存在
#define AKEY_RV_IMAGE_NO_TRANSTION_NUM                  0xE0200404 //请求图片的流水号不存在
#define AKEY_RV_IMAGE_NO_SEQ_NUM                        0xE0200405 //请求图片的顺序号不存在
#define AKEY_RV_IMAGE_SEQ_NUM_ERROR                     0xE0200406 //请求图片的顺序号错误
#define AKEY_RV_IMAGE_NO_NET_MODULE                     0xE0200407 //请求图片的网卡模组编号不存在
#define AKEY_RV_IMAGE_NO_DEVICE_FLOW                    0xE0200408 //请求图片的设备指令流不存在
#define AKEY_RV_IMAGE_NO_TIME_LOG                       0xE0200409 //请求图片的时间统计不存在
#define AKey_RV_IMAGE_NO_ENC_TEXT                       0xE020040A //请求图片的加密数据不存在
#define AKEY_RV_IMAGE_DEC_TEXT_ERROR                    0xE020040B //请求图片的解密数据失败
#define AKEY_RV_IMAGE_NO_ERROR_CODE                     0xE020040C //请求图片的中没有错误码
#define AKEY_RV_IMAGE_NO_PROGRESS_KEY                   0xE020040D //请求图片中没有过程密钥
#define AKEY_RV_IMAGE_NO_ENC_IMAGE_DATA                 0xE020040E //请求图片中没有过程图片密文数据

#define AKEY_RV_REQUEST_PORT_TRADECODE_ERROR            0xE0200501 //申请端口的交易码错误
#define AKEY_RV_REQUEST_PORT_NO_SAM_INDEX               0xE0200502 //申请端口的交易没有sam索引
#define AKEY_RV_REQUEST_PORT_NO_SAM_IP                  0xE0200503 //申请端口没有返回IP地址
#define AKEY_RV_REQUEST_PORT_NO_SAM_PORT                0xE0200504 //申请端口没有返回端口号

#define AKEY_RV_RELEASE_PORT_TRADECODE_ERROR            0xE0200601 //释放端口的交易码错误


#endif /* IDReaderError_h */
