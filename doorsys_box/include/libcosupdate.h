#ifndef _LIB_BASE64_H_
#define _LIB_BASE64_H_

#define HTTPS_PRE          "https://"

#define COS_TRAN_CODE_01   0x01   //������ѯ�ӿ�
#define COS_TRAN_CODE_02   0x02   //��ȡ��Ȩǩ���ӿ�
#define COS_TRAN_CODE_03   0x03   //����������
#define COS_TRAN_CODE_04   0x04   //�����ɹ�/ʧ�ܷ����ӿ�

#define COS_TAG_57         0x57
#define COS_TAG_59         0x59

typedef struct _cos_request_info{
	char cosVersion[100];   //COS�汾
	char updateCert[4096];  //����֤��
	char cosInfo[1024];     //COS��Ϣ(�汾)
	char randomNo[100];     //�����
	char read_sn[100];      //���������
	char isSuccess[2];         //�����ɹ���־:0-�ɹ�,1-ʧ��
}cos_request_info;

typedef struct _cos_response_info{
	char isUpdate[2];          //�Ƿ�����:0-����Ҫ,1-��Ҫ
	char cosVersion[100];   //Cos�汾����
	char flag[2];              //�Ƿ�ǿ������:0-��ǿ��,1-ǿ��
	char version[100];      //����Cos�汾
	int size;               //�汾��С
	char signature[4096];   //ǩ����
	char authResult[4096];  //��Ȩ���
	char cosPacket[51200];  //cos����������
	char resultCode[100];   //������
}cos_response_info;

int cjson_request(int tran_code, cos_request_info *p_cos_req_info, char *recv_buf, int *recv_len);

#endif
