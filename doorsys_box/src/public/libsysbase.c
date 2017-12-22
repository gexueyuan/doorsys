/***********************************************************
**文件名称: libsysbase.c
**作	者: 柳文杰
**日	期: 2015/08/11
**文件说明: 公共函数接口
**修改记录: 
************************************************************/
#include "global_samser.h"
#include "global_netcard.h"
#include "liblog.h"

unsigned long long getTimeStampStr(char *pTime);

int ProcessMac(char *pSrcMac, char *pDesMac)
{
	int i = 0;
	int j = 0;
	int iLen = 0;

	if(NULL == pSrcMac || NULL == pDesMac)
	{
		return -1;
	} 

	iLen = strlen(pDesMac);
	for(i = 0; i < iLen; i++)
	{
		if(pDesMac[i] == ':')
		{
			continue;	
		}
		else
		{
			pSrcMac[j] = pDesMac[i];
			j++;
		}
	}

	return 0;
}

unsigned long long getTimeStampStr(char *pTime)
{
	unsigned int  msStamp = 0;
	struct timeval stTimeval;
	gettimeofday(&stTimeval, NULL);
	sprintf(pTime, "%010u", stTimeval.tv_sec);
	sprintf(pTime+10, "%03d", stTimeval.tv_usec/1000);
	msStamp = stTimeval.tv_sec*1000 + stTimeval.tv_usec/1000;
	return msStamp;
}

int createJyls(char *pNetcardSn, int iNetcardSnLen, char *pJyls, int *iJylsLen)
{
	int  iTmpLen = 0;
	char sUTCTime[BUF_TMP_LEN];
	
	memset(sUTCTime, 0, sizeof(sUTCTime));
	getTimeStampStr(sUTCTime);
	
	memcpy(pJyls, pNetcardSn, iNetcardSnLen);
	iTmpLen = iNetcardSnLen;
	
	memcpy(pJyls+iTmpLen, sUTCTime, 13);
	iTmpLen += 13;

	*iJylsLen = iTmpLen;
	
	return 0;
}

int transUUid(char *pUUid, int iUidLen, char *pOutUUid, int *iOutLen)
{
	int i = 0;
	int iLen = 0;
	unsigned long long ullTmpLen = 0;
	char sBuf[100];
	char sTmpBuf[8];
	
	memset(sBuf, 0, sizeof(sBuf));
	memcpy(sBuf, pUUid, 8);
	util_tool_rtrim(sBuf, sBuf);
	iLen = strlen(sBuf);
	memcpy(sBuf+iLen, pUUid+8, 2);
	util_tool_rtrim(sBuf, sBuf);
	iLen = strlen(sBuf);
	
	memset(sTmpBuf, 0, sizeof(sTmpBuf));
	memcpy(sTmpBuf+2, pUUid+10, iUidLen-10);
	memcpy(&ullTmpLen, sTmpBuf, iUidLen-10+2);

	
#if 0
	for(i = 0; i < iUidLen-10; i++)
	{
		sTmpBuf[i] = pUUid[iUidLen-1-i];
	}
	memcpy(&ullTmpLen, sTmpBuf, iUidLen-10);
#endif

	sprintf(sBuf+iLen, "%llu", ullTmpLen);
	
	memcpy(pOutUUid, sBuf, strlen(sBuf));
	*iOutLen = strlen(sBuf);
	
	return 0;
}

int compareAppVersion(char *sVerSion, char *pFileName)
{
	char sTmpBuf[128];
	char sFileVer[20];
	char sTmp1[20];
	char sTmp2[20];
	char *pBegin=NULL;
	char *pEnd=NULL;

	int iSrcVice1  = 0;	 //副版本 1级
	int iSrcVice2  = 0;	 //副版本 2级
	int iSrcVice3  = 0;	 //副版本 3级
	int iSrcVice4  = 0;	 //副版本 4级
	//pFileName中的各位
	int iDstVice1  = 0;	 //副版本 1级
	int iDstVice2  = 0;	 //副版本 2级
	int iDstVice3  = 0;	 //副版本 3级
	int iDstVice4  = 0;	 //副版本 4级
	
	strcpy(sTmpBuf, pFileName);
	pBegin = strchr(sTmpBuf, '-');
	if(NULL == pBegin)
	{
		return -2;
	}

	pEnd = strrchr(sTmpBuf, '-');
	if(NULL == pEnd)
	{
		return -2;
	}
	
	memset(sFileVer, 0, sizeof(sFileVer));
	memcpy(sFileVer, pBegin+1, pEnd - pBegin - 1);
	
	if(strlen(sVerSion) == 1)
	{
		//初始版本，默认更新
		strcpy(sVerSion, sFileVer);
		return 1;
	}

	sscanf(sVerSion, "%d.%d.%d.%d", &iSrcVice1, &iSrcVice2, &iSrcVice3, &iSrcVice4);

	sscanf(sFileVer, "%d.%d.%d.%d", &iDstVice1, &iDstVice2, &iDstVice3, &iDstVice4);
	
	//比较版本号
	if(iSrcVice1 > iDstVice1)
	{
		return -1;
	}
	else if(iSrcVice1 < iDstVice1)
	{
		strcpy(sVerSion, sFileVer);
		return 1;
	}
	else
	{
		if(iSrcVice2 > iDstVice2)
		{
			return -1;
		}
		else if(iSrcVice2 < iDstVice2)
		{
			strcpy(sVerSion, sFileVer);
			return 1;
		}
		else
		{
			if(iSrcVice3 > iDstVice3)
			{
				return -1;
			}
			else if(iSrcVice3 < iDstVice3)
			{
				strcpy(sVerSion, sFileVer);
				return 1;
			}
			else
			{
				if(iSrcVice4 > iDstVice4)
				{
					return -1;
				}
				else if(iSrcVice4 < iDstVice4)
				{
					strcpy(sVerSion, sFileVer);
					return 1;
				}
				else
				{
					return 0;
				}
			}
		}
	}
}


/**************************************************************
** 函数名  : modSamConfig
** 功  能  : 修改SAM配置文件的版本号
** 作  者  : 柳文杰
** 建立日期: 2015/08/31
** 参数含义: pVerSion	  格式:0.0.0.3
** 返回值  : 0:成功 其它:失败
**************************************************************/
int modSamConfig(char *pVerSion)
{
	char sFilePath[256];
	char sBuff[1024];
	char sTmpBuff[1024];
	int  iLen = 0;
	char *pBegin = NULL;
	char *pEnd = NULL;
	char *pNext = NULL;
	FILE *fpFile = NULL;
	
	memset(sFilePath, 0x00,sizeof(sFilePath));
	sprintf(sFilePath, "%s/%s", GOFUN_HOME_DIR, NETCARD_CONFIG_FILE);
	
	fpFile = fopen(sFilePath, "rb+");
	if(fpFile == NULL)
	{
		return -1;
	}
	
	memset(sBuff, 0, sizeof(sBuff));
	fread(sBuff, 1, 1024, fpFile);
	pBegin = sBuff;

	pEnd = strchr(pBegin, '\n');
	if(pEnd == NULL)
	{
		return -1;
	}
	
	iLen = pEnd - pBegin+1;
	fseek(fpFile, iLen, SEEK_SET); 
	
	pNext = strchr(pEnd+1, '\n');
	if(pNext == NULL)
	{
		return -1;
	}
	//memset(pEnd+1, 0x00,pBegin-pEnd-1);
	iLen = pNext - pEnd-1;
	printf("iLen[%d]\n", iLen);
	memset(sTmpBuff, ' ', iLen-1);
	memcpy(sTmpBuff, "version=", 8);
	memcpy(sTmpBuff+8, pVerSion, strlen(pVerSion));

	if(strlen(sTmpBuff) > iLen)
	{
		fwrite(sTmpBuff, 1, iLen, fpFile);
	}
	else
	{
		fwrite(sTmpBuff, 1, strlen(sTmpBuff), fpFile);
	}
	fclose(fpFile);
	
	return 0;
}

/**************************************************************
** 函数名  : systemCmd
** 功  能  : 调用系统system
** 作  者  : 柳文杰
** 建立日期: 2015/08/18
** 参数含义: pCmd 命令行
** 返回值  : 0:成功 <0 失败
**************************************************************/
int systemCmd(char *pCmd)
{
	pid_t status = 0;
	status = system(pCmd);
	if(status == -1)
	{
		if(errno == 10)
		{
			return 0;
		}
		return -1;
	}
	else
	{
		if (WIFEXITED(status))
		{
			if (0 == WEXITSTATUS(status))
			{
				return 0;
			}
			else
			{
				return -1;
			}
		}
	}

	return 0;
}

