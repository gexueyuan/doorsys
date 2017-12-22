//
//  IDCardTask.h
//  TDRToken
//
//  Created by zhaowei on 16/5/12.
//  Copyright © 2016年 lujun. All rights reserved.
//

#ifndef IDCardTask_h
#define IDCardTask_h

#include "IDCardInclude.h"
#include "IDCardManager.h"
#include "IDCardCore.h"
#include "IDCardBussiness.h"
#include "IDCardInfo.h"






namespace  IDCARD {
    class IDCardTask
    {
    public:
        IDCardTask() {}
        
        ~IDCardTask() {}
        
        const char* libVersion()
        {
            return IDCARD_SDK_VERSION;
        }
        
        /**
         *  初始化参数信息，用于读取身份证
         *
         *  @param paramContext    一般参数信息
         *  @param readerInterface 读卡器接口
         */
        void Init(TDR_PARAM_Context* paramContext,
                  IIDCardReaderInterface* readerInterface,
                  IIDCardFile* fileInterface,
                  TDR_CALLBACK_CONTEXT* callBack)
        {
            ParamManager::instance()->setParamContext(paramContext);
            ParamManager::instance()->setReaderInterface(readerInterface);
            ParamManager::instance()->setFileInterface(fileInterface);
            ParamManager::instance()->setCallBackContext(callBack);
            m_bussiness.LoadCache();
            if (paramContext) {
				IDCARD_CORE_CONTEXT coreCtx;
				memset(&coreCtx,0,sizeof(IDCARD_CORE_CONTEXT));
				coreCtx.u32ConnTime = paramContext->u32ConnTime;
				coreCtx.u32TCPSRTime = paramContext->u32TCPSRTime;
				coreCtx.u32UDPSRTime = paramContext->u32UDPSRTime;
                coreCtx.bWorkMode = (IDCARD_WORD_MODE)paramContext->bWorkMode;
                m_core.initParam(&m_bussiness,&coreCtx);
            }
        }
        
        
        /**
         *  读证初始化设置
         *
         *  @param ip   ip 地址
         *  @param port 端口号
		 *  @param pbSockCfg socket配置
		 *  @param bWorkMode 工作模式
		 *  @param u32ConnTime 连接超时（单位毫秒）
		 *  @param u32TCPSRTime TCP接收数据超时(单位毫秒)
		 *  @param u32UDPSRTime UDP接收数据超时（单位毫秒）
		 *  @param u32PerPkgTimeOut 包超时（单位毫秒）
		 *  @param nBusNoTimeOut   业务总超时(单位秒)
         */
        void setIpAndPort(const char* ip,int port,LPBYTE pbSockCfg,BYTE bWorkMode,UINT32 u32ConnTime,UINT32 u32TCPSRTime,UINT32 u32UDPSRTime,UINT32 u32PerPkgTimeOut,int nBusNoTimeOut)
        {
			ParamManager::instance()->setSocketCfg(ip, port,pbSockCfg,bWorkMode,u32ConnTime,u32TCPSRTime,u32UDPSRTime,u32PerPkgTimeOut,nBusNoTimeOut);
			IDCARD_CORE_CONTEXT coreCtx;
			memset(&coreCtx,0,sizeof(IDCARD_CORE_CONTEXT));
			coreCtx.u32ConnTime = u32ConnTime;
			coreCtx.u32TCPSRTime = u32TCPSRTime;
			coreCtx.u32UDPSRTime = u32UDPSRTime;
			coreCtx.bWorkMode = (IDCARD_WORD_MODE)bWorkMode;
			m_core.initParam(&m_bussiness,&coreCtx);
        }
        
        /**
         *  设备断开时调用
         */
        void DeviceDisConnect()
        {
            IDCardInfo::instance()->cleanAppKeyHash();
			m_bussiness.cleanCache04CMD();
        }

		//设置工作状态：[设置bEnabled为false表示取消读证]
		void SetIDWorkFlag(bool bEnabled)
		{
			ParamManager::instance()->setEnableIDWork(bEnabled);
		}

		bool GetIDWorkFlag()
		{
			return ParamManager::instance()->getIDWorkIsEnable();
		}
        
        
        /**
         设置等待超时

         @param u32TimeOut 等待超时时间(参考值:5000ms)单位ms
         @param u32TimeInterval 查询时间间隔（参考值:500ms）单位ms
         */
        void setWaitSamTimeOut(UINT32 u32TimeOut,UINT32 u32TimeInterval)
        {
            ParamManager::instance()->setSamWaitTimeOut(u32TimeOut);
            ParamManager::instance()->setSamWaitTimeInterval(u32TimeInterval);
        }
        
        
        /**
         退出等待
         */
        void quitWait()
        {
            ParamManager::instance()->setWaitFlag(true);//表示退出
        }

		HRESULT unBindDevice()
		{
			return IDCardInfo::instance()->UnBindDevice();
		}

        
        /**
         *  读卡前的准备工作(读取读卡器序列号、cos版本号、读卡器证书等信息)
         *
         *  @param snBuf  读卡器序列号
         *  @param cosBuf cos版本号
         *
         *  @return 错误码
         */
        HRESULT PrepareWork(CBuffer& snBuf,CBuffer & cosBuf)
        {
            HRESULT hr = AKEY_RV_OK;
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "PrepareWork");
            hr = IDCardInfo::instance()->PrepareWork(snBuf,cosBuf);
            LGNTRACE_ERRORNO(hr);
			if (hr == AKEY_RV_OK) {
				//设置为可读取状态
				ParamManager::instance()->setEnableIDWork(true);
				m_bussiness.cleanCache04CMD();
                IDCardInfo::instance()->cleanAppKeyHash();
			}
			
            return hr;
        }
        
        
        /**
         *  发送寻卡指令
         *
         *  @param flag 返回的寻卡标示
         *
         *  @return 错误码
         */
        HRESULT FindCard(BYTE* flag)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "FindCard");
            HRESULT hr = AKEY_RV_OK;
            hr = IDCardInfo::instance()->FindCard(flag);
            LGNTRACE_ERRORNO(hr);
            return hr;
        }
        
        /**
         *  读取电量
         *
         *  @param battery 电量数据
         *
         *  @return 错误码
         */
        HRESULT GetBattery(BYTE* battery)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "GetBattery");
            HRESULT hr = AKEY_RV_OK;
            hr = IDCardInfo::instance()->GetBatteryLevel(battery);
            LGNTRACE_ERRORNO(hr);
            return hr;
        }
        
        /**
         *  读取cos信息，主要用于上传服务器(相关字段的信息请查询COS信息查询描述表)
         *
         *  @param timeInterval   获取读卡器设备寻卡的时间间隔
         *  @param batteryHealth  电池健康信息

         *  @param readerBehavior 读卡行为信息

         *
         *  @return 错误码
         */
        HRESULT GetReaderCosInfo(CBuffer& timeInterval,CBuffer& batteryHealth,CBuffer& readerBehavior)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "GetBattery");
            HRESULT hr = AKEY_RV_OK;
            hr = IDCardInfo::instance()->GetReaderCosInfo(timeInterval,batteryHealth,readerBehavior);
            LGNTRACE_ERRORNO(hr);
            return hr;
        }
        
        
        /**
         *  读取身份证信息
         *
         *
         *  @return 错误码
         */
        
        HRESULT ReadIDCardInfo()
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ReadIDCardInfo");
			SetIDWorkFlag(true);
            ParamManager::instance()->setWaitFlag(false);
            HRESULT hr = AKEY_RV_OK;
            initLog();
            CBuffer refNum,idCardBuf;
            fnreadIDInfo idInfoCb = ParamManager::instance()->getIDCardInfoCb();
            fnreadIDImage idCardImgCb = ParamManager::instance()->getIDCardImgCb();
            fnreadCardError errCb = ParamManager::instance()->getErrCallBack();
            m_time.AllBegin();
            hr = m_core.ReadID(idCardBuf);
            m_bussiness.GetRefNumBuf(refNum);
            if (hr != AKEY_RV_OK) {
                CLog::instance()->WriteCacheToFile();
                if (errCb) {
                     errCb(hr,refNum.GetBuffer(),refNum.GetLength());
                }
                //执行一次寻卡操作，通知读卡器读证业务流程终止
                FindCard(NULL);
                return hr;
            }
            if (!ParamManager::instance()->getFinishFlag()) {
                fnreadCardFinish finish = NULL;
                finish = ParamManager::instance()->getFinishCallBack();
                if (finish) {
                    finish();
                }
            }
            if (idInfoCb) {
                idInfoCb(idCardBuf.GetBuffer(),idCardBuf.GetLength());
            }
            CBuffer imageBuf,timeBuf;
            hr = m_core.ReadImage(imageBuf);
            if (hr != AKEY_RV_OK) {
                CLog::instance()->WriteCacheToFile();
                //执行一次寻卡操作，通知读卡器读证业务流程终止
                FindCard(NULL);
                if (errCb) {
                    errCb(hr,refNum.GetBuffer(),refNum.GetLength());
                }
                return hr;
            }
            CLog::instance()->setTotalTaskTime(m_time.timeAllSpend());
            m_bussiness.IDReaderDisServer();
            m_bussiness.getTimeBuf(timeBuf);
            if (idCardImgCb) {
                idCardImgCb(imageBuf.GetBuffer(),imageBuf.GetLength(),refNum.GetBuffer(),refNum.GetLength(),timeBuf.GetBuffer(),timeBuf.GetLength());
            }
            //执行一次寻卡操作，通知读卡器读证业务流程终止
            FindCard(NULL);
            return hr;
        }
        
        
        /**
         *  与上面的接口分开
         *
         *  @param refNum    参考号
		 *  @param samNum    网卡模组编号
		 *  @param samIndex  sam索引
         *  @param idCardBuf 身份证信息（或错误码数据）
		 *  @param noteBuf   票据信息
		 *  @param imageFlag 是否应该取图片 (0x01:取照片 0x02:不取照片)
		 *  @param timeBuf   耗时日志
         *  @Note： 在票交所的读证中，idCardBuf中存储了错误码数据
         *  @return 错误码
         */
        HRESULT ReadIDCardInfo(CBuffer& refNum,CBuffer& samNum,BYTE& samIndex,CBuffer& idCardBuf,CBuffer& noteBuf,BYTE* imageFlag,CBuffer& timeBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ReadIDCardInfo");
            HRESULT hr = AKEY_RV_OK;
            initLog();
            SetIDWorkFlag(true);
            ParamManager::instance()->setWaitFlag(false);
			LGNTRACE_MSG("SDK Version:%s",IDCARD_SDK_VERSION);
			LGNTRACE_MSG("WORK MODE:%d",ParamManager::instance()->getWorkMode());
            m_time.AllBegin();
            hr = m_core.ReadID(idCardBuf);
            m_bussiness.GetRefNumBuf(refNum);
			CLog::instance()->getSamVerBuf(samNum);
			samIndex = m_bussiness.getSamIndex();
			const CBuffer& noteTmp =ParamManager::instance()->getNoteInfo();
			if (noteTmp.GetLength() > 0) {
				memcpy(noteBuf.ReAllocBytesSetLength(noteTmp.GetLength()),noteTmp.GetBuffer(),noteTmp.GetLength());
			}
			
            if (hr != AKEY_RV_OK) {
				LGNTRACE_MSG("ReadID hr:%x",hr);
                CLog::instance()->WriteCacheToFile();
                //执行一次寻卡操作，通知读卡器读证业务流程终止
                FindCard(NULL);
                return hr;
            }
            if (!ParamManager::instance()->getFinishFlag()) {
                fnreadCardFinish finish = NULL;
                finish = ParamManager::instance()->getFinishCallBack();
                if (finish) {
                    finish();
                }
            }
			if (imageFlag)
			{
				*imageFlag = ParamManager::instance()->getImageAvaiableFlag();
			}
			if (ParamManager::instance()->getImageAvaiableFlag() == 0x02)//不取照片
			{
				LGNTRACE_MSG("Total time:%f",m_time.timeAllSpend());
				CLog::instance()->setTotalTaskTime(m_time.timeAllSpend());
				m_bussiness.IDReaderDisServer();
				m_bussiness.getTimeBuf(timeBuf);
				//执行一次寻卡操作，通知读卡器读证业务流程终止
				FindCard(NULL);
				return hr;
			}
			
            m_time.Begin();
            return hr;
        }
        
        /**
         *  身份证图片的读取
         *
         *  @param imageBuf 身份证图片
         *  @param timeBuf  耗时信息
         *
         *  @return 错误码
         */
        HRESULT ReadIDCardImage(CBuffer& imageBuf,CBuffer& timeBuf)
        {
            LGNTRACE_ENTRY(LGN_TRACE_CATEGORY_DEF, LGN_TRACE_CATEGORY_INFO_LEVEL, "ReadImg");
            HRESULT hr = AKEY_RV_OK;
            CLog::instance()->setInterfaceTime(m_time.timeSpend());
            hr = m_core.ReadImage(imageBuf);
            if (hr != AKEY_RV_OK) {
				LGNTRACE_MSG("ReadImage hr:%x",hr);
                CLog::instance()->WriteCacheToFile();
				m_bussiness.IDReaderDisServer();
                //执行一次寻卡操作，通知读卡器读证业务流程终止
                FindCard(NULL);
                return hr;
            }
            LGNTRACE_MSG("Total time:%f",m_time.timeAllSpend());
            CLog::instance()->setTotalTaskTime(m_time.timeAllSpend());
            m_bussiness.IDReaderDisServer();
            m_bussiness.getTimeBuf(timeBuf);
            //执行一次寻卡操作，通知读卡器读证业务流程终止
            FindCard(NULL);
            return hr;
        }

		HRESULT CloseReader()
		{
			return	IDCardInfo::instance()->CloseReader();
		}
        
        /**
         *  读取日志
         *
         *  @param pun32NetTime  总网络通信时间
         *  @param pun32CmdTime  总指令交互时间
		 *  @param pun32TotalTime 读证总时间
         *  @param cmdBuf        每条指令的统计(格式:数据[4]+时间[4])
         *  @param netHandleTime 网卡模组应用处理时间(格式:step[2]+value[4])
         *  @param appSafeComBuf 与安全模块通讯时间(格式:step[2]+value[4])
         */
        void GetLog(UINT32* pun32NetTime,UINT32* pun32CmdTime,UINT32* pun32TotalTime,CBuffer& cmdBuf,CBuffer& netHandleTime,CBuffer& appSafeComBuf)
        {
            if (pun32NetTime) {
                *pun32NetTime = CLog::instance()->GetTotalNetTime();
            }
            if (pun32CmdTime) {
                *pun32CmdTime = CLog::instance()->GetInsTime();
            }
			if (pun32TotalTime) {
				*pun32TotalTime = CLog::instance()->getTotalTaskTime();
			}
			
            CLog::instance()->getTotalInsTime_Ex(cmdBuf);
            CLog::instance()->getNetHandleBuf_Ex(netHandleTime);
            CLog::instance()->getSafeComBuf_Ex(appSafeComBuf);
        }
        
    protected:
        
        void initLog()
        {
            CLog::instance()->CleanLog();
            CLog::instance()->initTimer();
            ParamManager::instance()->setFinishFlag(false);
			ParamManager::instance()->cleanNoteInfo();
            m_bussiness.cleanRefNum();//清除参考号
			m_bussiness.cleanSamIndex();
        }
        
    private:
        IDCardBussiness m_bussiness;
        IDCardCore      m_core;
        CTime m_time;
    };

    
};


#endif /* IDCardTask_h */
