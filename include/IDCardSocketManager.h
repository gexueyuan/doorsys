#ifndef IDCardScoketManager_h
#define IDCardScoketManager_h

#include "IDCardInclude.h"
#include "IDCardSocket.h"
#include "IDCardRetrySocket.h"
#include "IDCardUDP.h"
#include "IDCardRetryUDP.h"
#include "IDCardTCPUDP.h"
#include "IDCardRetryTCPUDP.h"

namespace IDCARD {
	class CSockManager
	{
	public:
		CSockManager(){}
		~CSockManager(){}

		static CSockManager* instance()
		{
			static  CSockManager* _instance = NULL;
			if (_instance == NULL) {
				_instance = new CSockManager();
			}
			return _instance;
		}

		void setTimeOut(UINT32 u32ConnTimeOut,UINT32 u32TCPSRTimeOut,UINT32 u32UDPSRTimeOut)
		{
			m_rawTCPSocket.setTimeOut(u32ConnTimeOut,u32TCPSRTimeOut);
			m_AppTCPSocket.setTimeOut(u32ConnTimeOut,u32TCPSRTimeOut);
			m_AppUDPSocket.setTimeOut(u32ConnTimeOut,u32UDPSRTimeOut);
			m_AppTCPUDPSocket.setTimeOut(u32ConnTimeOut,u32UDPSRTimeOut);
		}

		ISocket* getRequestPortSock()
		{
			return &m_rawTCPSocket;
		}

		ISocket* getTCPSocket()
		{
			return &m_AppTCPSocket;
		}

		ISocket* getUDPSocket()
		{
			return &m_AppUDPSocket;
		}

		ISocket* getTCPUDPSocket()
		{
			return &m_AppTCPUDPSocket;
		}

		ISocket* getSockFromStep(IDREADER_STEP step)
		{
			BYTE bCfg[4];
			IDCARD_NET_TYPE netType;
			ParamManager::instance()->getSockCfg(bCfg);
			if (bCfg[step] == NET_TYPE_DEFAULT) {//客户端没有设置网络类型，就采用默认的配置
				netType = g_NetType[step];
			}else {
				netType = (IDCARD_NET_TYPE)bCfg[step];
			}
			if (netType == NET_TYPE_TCP) {
				return &m_AppTCPSocket;
			}else if(netType == NET_TYPE_UDP){
				return &m_AppUDPSocket;
			}else if(netType == NET_TYPE_TCP_UDP) {
				return &m_AppTCPUDPSocket;
			}
			return NULL;
		}

	private:
		CSocketTCP        m_rawTCPSocket;//用于申请端口
		IDCardRetryUDP    m_AppUDPSocket;//用于读证的支持重试的TCP socket
		IDCardRetrySocket m_AppTCPSocket;//用于读证的支持重试的UDP socket
		IDCardRetryTCPUDP m_AppTCPUDPSocket;//支持tcp和udp混合
	};

};

#endif