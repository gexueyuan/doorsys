#ifndef _GLOBAL_SAMSER_H_
#define _GLOBAL_SAMSER_H_

#include "global_sys.h"

//ȫ�ֱ���
#define ERR_LOG_LEN       300*1024     //�ͻ����ϱ�����־��С
#define iMAXRECVBUF   10240
#define MSG_MAXLEN 		iMAXRECVBUF
#define BUF_LEN       iMAXRECVBUF
#define SAM_MAX       16
#define FILE_PATH     256           //�ļ����೤��
#define IP_LEN        20            //IP����
#define PORT_LEN      8             //�˿ڳ���
#define MAC_LEN       20            //MAC����
#define VERSION_LEN   20            //�汾����
#define SHELL_NO_LEN  20            //SAM��ų���
#define PASSWD_LEN    40            //���볤��
#define UUID_LEN      16            //UUID����
#define SAM_PATH      16            //SAM��PATH·��
#define FTP_USER_LEN  16            //FTP�û�������
#define FTP_PASSWD_LEN 32           //FTP����
#define FTP_FILE_NAME_LEN 64        //FTP����
#define TIME_LEN      8             //���ʱ�䳤��
#define DN_MAX_LEN    100           //DN����󳤶�
#define CERT_INFO_LEN 2048          //���֤������Ϣ����
#define CERT_PIC_LEN  4096          //���֤ͼƬ��Ϣ����
#define SAM_NUM_LEN   4             //SAM����
#define CPU_INFO_LEN  8             //CPU��Ϣ����
#define MEM_INFO_LEN  8             //MEM��Ϣ����
#define FLASH_INFO_LEN 8            //FLASH��Ϣ����
#define READ_SEQ_LEN  20            //Reader���
#define MOBILE_NO_LEN 64            //�ֻ��ų���
#define SAM_SN_LEN    20            //SAM_SN����
#define CERT_NO_LEN   18            //���֤�ų���
#define MSG_BUF_LEN   1024          //MSG����
#define DN_LEN        32            //DN����
#define CRC_BUF_LEN   100           //CRC��󳤶�
#define BUF_TMP_LEN   4096          //BUF��ʱ����
#define READ_SN_LEN   100           //�����������󳤶�
#define CUST_NUM_LEN  100           //�ͻ��ų���
#define CKH_LEN       100           //�ο��ų���
#define MARK_LEN      600           //���֤�������ݳ���
#define CORP_NUM_LEN  100           //�̻��ų���
#define COS_VER_LEN   30            //COS�汾�ų���

#define TCP_CLT_TIME_OUT   5       //TCPͨ�ų�ʱʱ�䣬��λ: ��
#define REDIS_CLT_TIME_OUT   1       //REDIS��ʱʱ�䣬��λ: ��

#define  iSHORTLEN      2
#define  iACESERLEN     8
#define  iINTLEN        4

#define BUSI_PORT     10040      //������̼����˿�

//�������
#define LOG_ERR_RCVCLT       0xE2100001  //���տͻ��˱��ĳ�ʱ
#define LOG_ERR_CRCCHECK     0xE2100002  //У��CRCʧ��
#define LOG_ERR_LED          0xE2100003  //���ʧ��
#define LOG_ERR_FIND_CARD    0xE2100004  //Ѱ��ʧ��
#define LOG_ERR_READID       0xE2100005  //��֤ʧ��

//�����������
#define LOG_ERR_APPSTART     0xE2000030  //ϵͳ�Ѿ�����

//ϵͳ���ñ�־
#define SYS_UP_FLAG_YES     1   //Ӧ��������
#define SYS_UP_FLAG_NO      2   //Ӧ���ѹر�

//ҵ��״̬
#define BUSI_STAT_FREE  0x01   //����
#define BUSI_STAT_FIND  0x02   //Ѱ��
#define BUSI_STAT_READ  0x03   //��֤

//������״̬
#define READER_STAT_OK   "0"   //����
#define READER_STAT_FAIL "1"   //�쳣

//LED��
#define LED_RED     0x01      //���
#define LED_GREEN   0x02      //�̵�

typedef struct
{
	char refnum_buf[100];   //�ο���
	int  refnum_len;        //�ο��ų���
	char readsn_buf[100];   //���������
	int  readsn_len;        //��������ų���
	char cos_version[100];  //COS�汾
	int  cos_len;           //COS�汾����
	int  err_code;          //������
	char err_text[100*1024]; //������־����
	int  err_len;            //���󳤶�
	char tcp_type;           //ͨѶЭ�� 0x01-tcp
	char time_buf[4096];     //��־ʱ����
	int  time_len;           //ʱ��������
}busi_log;

#endif
