#include "global_sys.h"
#include "global_samser.h"
#include "global_netcard.h"
#include "liblog.h"
#include "libcosupdate.h"
#include "cJSON.h"

//根据请求元素组成json格式
//req: {"header":{"Locale":"ZH_cn"},"request":[{"id":"001","method":"transfer","params":{"cosInfo":"00000001D69..","updateCert":"00010006..."}}]}
int cjson_packet(int tran_code, cos_request_info *p_cos_req_info, char *recv_buf, int *recv_len)
{
	cJSON * pJsonRoot = NULL;
	cJSON * pJsonHead = NULL;
	cJSON * pJsonArrayReq = NULL;
	cJSON * pJsonArrRoot = NULL;
	cJSON * pJsonParamsObj = NULL;
	
	pJsonRoot = cJSON_CreateObject();
	if(NULL == pJsonRoot)
	{
		return -1;
	}

	pJsonHead = cJSON_CreateObject();
	if(NULL == pJsonHead)
	{
		cJSON_Delete(pJsonRoot);
		return -1;
	}

	pJsonArrayReq = cJSON_CreateArray();
	if(NULL == pJsonArrayReq)
	{
		cJSON_Delete(pJsonRoot);
		cJSON_Delete(pJsonHead);
		return -1;
	}
	
	pJsonArrRoot = cJSON_CreateObject();
	if(NULL == pJsonArrRoot)
	{
		cJSON_Delete(pJsonRoot);
		cJSON_Delete(pJsonHead);
		cJSON_Delete(pJsonArrayReq);
		return -1;
	}
	
	pJsonParamsObj = cJSON_CreateObject();
	if(NULL == pJsonParamsObj)
	{
		cJSON_Delete(pJsonRoot);
		cJSON_Delete(pJsonHead);
		cJSON_Delete(pJsonArrayReq);
		cJSON_Delete(pJsonArrRoot);
		return -1;
	}
	
	cJSON_AddStringToObject(pJsonHead, "Locale", "ZH_cn");
	
	if(tran_code == COS_TRAN_CODE_01)
	{
		cJSON_AddStringToObject(pJsonParamsObj, "cosVersion", p_cos_req_info->cosVersion);
		cJSON_AddStringToObject(pJsonParamsObj, "updateCert", p_cos_req_info->updateCert);
	}
	else if(tran_code == COS_TRAN_CODE_02)
	{
		cJSON_AddStringToObject(pJsonParamsObj, "randomNo", p_cos_req_info->randomNo);
		cJSON_AddStringToObject(pJsonParamsObj, "updateCert", p_cos_req_info->updateCert);
	}
	else if(tran_code == COS_TRAN_CODE_03)
	{
		cJSON_AddStringToObject(pJsonParamsObj, "updateCert", p_cos_req_info->updateCert);
	}
	else if(tran_code == COS_TRAN_CODE_04)
	{
//		cJSON_AddStringToObject(pJsonParamsObj, "readerSN", p_cos_req_info->read_sn);
		cJSON_AddStringToObject(pJsonParamsObj, "isSuccess", p_cos_req_info->isSuccess);
		cJSON_AddStringToObject(pJsonParamsObj, "updateCert", p_cos_req_info->updateCert);
	}

	cJSON_AddStringToObject(pJsonArrRoot, "id", "001");
	cJSON_AddStringToObject(pJsonArrRoot, "method", "transfer");
	cJSON_AddItemToObject(pJsonArrRoot, "params", pJsonParamsObj);
	
	cJSON_AddItemToArray(pJsonArrayReq, pJsonArrRoot); 
		
	cJSON_AddItemToObject(pJsonRoot, "header", pJsonHead);
	cJSON_AddItemToObject(pJsonRoot, "request", pJsonArrayReq);

	char *pJsonBuf = cJSON_PrintUnformatted(pJsonRoot);
	memcpy(recv_buf, pJsonBuf, strlen(pJsonBuf));
	*recv_len = strlen(pJsonBuf);
	
	cJSON_Delete(pJsonRoot);
	free(pJsonBuf);
	
	return 0;
}

//解析json格式
int cjson_parse_packet(int tran_code, cos_response_info *p_cos_res_info, char *recv_buf, int recv_len)
{
	cJSON * pJsonRoot = NULL;
	cJSON * pJsonRequest = NULL;
	
	if(NULL == recv_buf)
	{
		return -1;
	}
	
	pJsonRoot = cJSON_Parse(recv_buf);
	if(NULL == pJsonRoot)
	{
		return -2;
	}
	
	//只解析params的元素
	pJsonRequest = cJSON_GetObjectItem(pJsonRoot, "resultList");
	if(NULL == pJsonRequest)
	{
		printf("cJSON_GetObjectItem has not item:resultList.\n");
		cJSON_Delete(pJsonRoot);
		return -1;
	}
	
	int i = 0;
	int icount = cJSON_GetArraySize(pJsonRequest);
	printf("icount:[%d]\n", icount);
	for(i = 0; i < icount; i++)
	{
		cJSON *request = cJSON_GetArrayItem(pJsonRequest, i);
		if(NULL != request)
		{
			cJSON *pJsonParam = cJSON_GetObjectItem(request, "result"); 
			if(NULL != pJsonParam)
			{
				if(tran_code == COS_TRAN_CODE_01)
				{
					cJSON *pJsonUpdFlag = cJSON_GetObjectItem(pJsonParam, "isUpdate"); 
					if(NULL == pJsonUpdFlag)
					{
						printf("json have no flag:isUpdate\n");
						break;
					}
					p_cos_res_info->isUpdate[0] = pJsonUpdFlag->valuestring[0];
					printf("isUpdate:[%s]\n", p_cos_res_info->isUpdate);
					
					cJSON *pJsonSign = cJSON_GetObjectItem(pJsonParam, "signature"); 
					if(NULL == pJsonSign)
					{
						printf("json have no flag:signature\n");
						break;
					}
					strcpy(p_cos_res_info->signature, pJsonSign->valuestring);
				}
				else if(tran_code == COS_TRAN_CODE_02)
				{
					cJSON *pJsonAuthRes = cJSON_GetObjectItem(pJsonParam, "authResult"); 
					if(NULL == pJsonAuthRes)
					{
						printf("json have no flag:authResult\n");
						break;
					}
					strcpy(p_cos_res_info->authResult, pJsonAuthRes->valuestring);
				}
				else if(tran_code == COS_TRAN_CODE_04)
				{
					cJSON *pJsonResCode = cJSON_GetObjectItem(pJsonParam, "resultCode"); 
					if(NULL == pJsonResCode)
					{
						printf("json have no flag:resultCode\n");
						break;
					}
					strcpy(p_cos_res_info->resultCode, pJsonResCode->valuestring);
				}
				break;
			}
		}
	}
	
	cJSON_Delete(pJsonRoot);
	
	return 0;
}

static int cjson_write_to_file(char *file_name, char *buf, int buf_len)
{
	FILE *fp_file = NULL;
	
	fp_file = fopen(file_name, "w");
	if(NULL == fp_file)
	{
		return -1;
	}
	
	fwrite(buf, 1, buf_len, fp_file);
	fclose(fp_file);
	
	return 0;
}

static int cjson_read_from_file(char *file_name, char *buf, int *buf_len)
{
	FILE *fp_file = NULL;
	
	fp_file = fopen(file_name, "r");
	if(NULL == fp_file)
	{
		return -1;
	}
	
	*buf_len = fread(buf, 1, 4096, fp_file);
	fclose(fp_file);
	
	return 0;
}

//读json文件  [{"readSn":"123456789012","cosVersion":"PR11-V1.0.2.3-170623-TEST-KEY", "cosStat":"0"},{"readSn1":"123456789012","cosVersion1":"PR11-V1.0.2.3-170623-TEST-KEY", "cosStat1":"0"}]
//从配置文件获取key
int cjson_get_value(char *file_name, char *key_name, char *value)
{
	cJSON * pJsonRoot = NULL;
	char recv_buf[4096];
	int  recv_len = 0;
	int  upd_flag = 0;
	int  i = 0;
	
	//检查配置文件是否存在
	char scfg_file[128];
	memset(scfg_file,0x00,sizeof(scfg_file));
	sprintf(scfg_file,"%s/%s", GOFUN_HOME_DIR, file_name);

#if 0
	if(access(scfg_file, F_OK) != 0)
	{
		pJsonRoot = cJSON_CreateObject();
		if(NULL == pJsonRoot)
		{
			return -1;
		}
		cJSON_AddStringToObject(pJsonRoot, "readSn", read_sn);
		cJSON_AddStringToObject(pJsonRoot, "cosVersion", cos_version);
		cJSON_AddStringToObject(pJsonRoot, "cosStat", cos_stat);
		
		char *pJsonBuf = cJSON_Print(pJsonRoot);
		cjson_write_to_file(scfg_file, pJsonBuf, strlen(pJsonBuf));
		
		cJSON_Delete(pJsonRoot);
		free(pJsonBuf);
	}
	else
#endif	
	{
		memset(recv_buf, 0, sizeof(recv_buf));
		cjson_read_from_file(scfg_file, recv_buf, &recv_len);
		pJsonRoot = cJSON_Parse(recv_buf);
		if(NULL == pJsonRoot)
		{
			printf("cjson_update_conf:cJSON_Parse error.\n");
			return -1;
		}
		
		int icount = cJSON_GetArraySize(pJsonRoot);

		for(i = 0; i < icount; i++)
		{
			cJSON *pJsonItem = cJSON_GetArrayItem(pJsonRoot, i);
			if(NULL != pJsonItem)
			{
				cJSON *pJsonKey = cJSON_GetObjectItem(pJsonItem, key_name); 
				if(NULL == pJsonKey)
				{
					continue;
				}
				if(pJsonKey->type == cJSON_String)
				{
					strcpy(value, pJsonKey->valuestring);
				}
				else if(pJsonKey->type == cJSON_Number)
				{
					sprintf(value, "%d", pJsonKey->valueint);
				}
				break;			
			}
		}
		if(i == icount)
		{
			printf("do not found %s\n", key_name);
			cJSON_Delete(pJsonRoot);
			return -1;
		}

		cJSON_Delete(pJsonRoot);
	}	
	
	return 0;
}

//从配置文件获取key,并更新
int cjson_update_value(char *file_name, char *key_name, char *value)
{
	cJSON * pJsonRoot = NULL;
	char recv_buf[4096];
	int  recv_len = 0;
	int  upd_flag = 0;
	int  i = 0;
	
	//检查配置文件是否存在
	char scfg_file[128];
	memset(scfg_file,0x00,sizeof(scfg_file));
	sprintf(scfg_file,"%s/%s", GOFUN_HOME_DIR, file_name);

	memset(recv_buf, 0, sizeof(recv_buf));
	cjson_read_from_file(scfg_file, recv_buf, &recv_len);
	pJsonRoot = cJSON_Parse(recv_buf);
	if(NULL == pJsonRoot)
	{
		printf("cjson_update_conf:cJSON_Parse error.\n");
		return -1;
	}
	
	int icount = cJSON_GetArraySize(pJsonRoot);
	for(i = 0; i < icount; i++)
	{
		cJSON *pJsonItem = cJSON_GetArrayItem(pJsonRoot, i);
		if(NULL != pJsonItem)
		{
			cJSON *pJsonKey = cJSON_GetObjectItem(pJsonItem, key_name); 
			if(NULL == pJsonKey)
			{
				continue;
			}
			if(pJsonKey->type == cJSON_String)
			{
				strcpy(pJsonKey->valuestring, value);
			}
			else if(pJsonKey->type == cJSON_Number)
			{
				
				pJsonKey->valueint = atoi(value);
				pJsonKey->valuedouble = atoi(value);
			}
			break;			
		}
	}
	if(i == icount)
	{
		printf("do not found %s\n", key_name);
		cJSON_Delete(pJsonRoot);
		return -1;
	}

	//更新到文件中
	char *pJsonBuf = cJSON_PrintUnformatted(pJsonRoot);
	cjson_write_to_file(scfg_file, pJsonBuf, strlen(pJsonBuf));
		
	cJSON_Delete(pJsonRoot);
	free(pJsonBuf);

	return 0;
}