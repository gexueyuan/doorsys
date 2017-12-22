#ifndef _GLOBAL_NETCARD_H_
#define _GLOBAL_NETCARD_H_

//log module tag define.BEGIN
#define LOGTAG_MNG      "MNG"
#define LOGTAG_BUSI  "GFREADBUSI"
#define LOGTAG_GFLOG "GFLOG"
#define LOGTAG_GFCOSUPDATE "COSUPDATE"
#define LOGTAG_GPBOXUPDATE "BOXUPDATE"
//log module tag define.END

//netcard server base define.BEGIN
#define NETCARD_CONFIG_FILE   "etc/id_gofun.conf"
#define CARD_STORAGE_TMP_FILE "/tmp/card_storage.txt"
#define CARD_ERR_FILE         "/tmp/idreaderr.log"
#define READER_INFO_CONF_FILE    "etc/reader_info.json"  //��������Ϣ�洢�����ļ�
#define HTTPS_CERT_FILE        "cert.pem"         //https�ͻ���ʹ�õ�֤��
#define GPBOX_CTRL_CONF_FILE   "etc/gpbox_ctrl.conf"
#define GOFUN_HOME_DIR        ""         //������Ŀ¼

#define GPBOX_CONF_JSON_FILE   "etc/gpbox_conf.json"   //�ڲ������ļ�
#define GPBOX_STAT_JSON_FILE   "etc/gpbox_stat.json"   //�ڲ�״̬�ļ�
//netcard server base define.END

//JSON����
#define GOPANSN         "gopansn"    //��ƷSN ��gopan��
#define GOREADVER       "goreadver"  //�������汾��
#define GOREADSN        "goreadsn "  //������SN
#define WORKMODE        "workmode"   //��ǰ����ģʽ��0-6��
#define READSTATUS      "readstatus" //����״̬ 0�����д���,1�����ڶ���,2�������ɹ�,3������ʧ��

#define READSTAT_0      "0"      //����״̬ 0�����д���
#define READSTAT_1      "1"      //����״̬ 1�����ڶ���
#define READSTAT_2      "2"      //����״̬ 2�������ɹ�
#define READSTAT_3      "3"      //����״̬ 3������ʧ��

#define MAX_BUF_LEN    10240

//TRANS_CODE define.begin
#define TRANS_CODE_01      0x0001   //����ģʽ
#define TRANS_CODE_02      0x0002   //����ģʽ
#define TRANS_CODE_03      0x0003   //��֤ģʽ-����
#define TRANS_CODE_04      0x0004   //��֤ģʽ-����
#define TRANS_CODE_05      0x0005   //��֤ͨ��
#define TRANS_CODE_06      0x0006   //���ģʽ
#define TRANS_CODE_07      0x0007   //Ϩ��ģʽ
#define TRANS_CODE_08      0x0008   //COS����
#define TRANS_CODE_09      0x0009   //��������
#define TRANS_CODE_0A      0x000A   //����DN

//TRANS_CODE define.end

//LOG_CODE define.begin
#define LOG_CODE_01        0x0403   //���ʹ�����־
#define LOG_CODE_02        0X0405   //������־ʱ����
//LOG_CODE define.end

//TLV define.BEGIN
#define TLV_TAG_JYDM   0x0001   //������
#define TLV_TAG_PCH    0x0002   //���κ�
#define TLV_TAG_CKH    0x0003   //�ο���
#define TLV_TAG_DKQXH  0x0004   //���������
#define TLV_TAG_KHH    0x0005   //�ͻ���
#define TLV_TAG_CWH    0x0006   //�����
#define TLV_TAG_SXH    0x0007   //��ˮ��
#define TLV_TAG_WKDKH  0x0008   //�����˿ں�
#define TLV_TAG_WZBH   0x0009   //λ�ñ��
#define TLV_TAG_KHDXX  0x000A   //�ͻ���ѡ��
#define TLV_TAG_FWDXX  0x000B   //�����ѡ��
#define TLV_TAG_FBBZ   0x000C   //�ְ���־
#define TLV_TAG_KHDZS  0x0011   //����ͨ���ͻ���֤��
#define TLV_TAG_FWDZS  0x0012   //����ͨ�������֤��
#define TLV_TAG_DKQZS  0x0013	  //������֤��
#define TLV_TAG_KZQZS  0x0014   //������֤��
#define TLV_TAG_FWDZSBB 0x0015  //�����֤��汾
#define TLV_TAG_FWDSJS 0x0016   //����������
#define TLV_TAG_MYMW   0x0017   //����ͨ����Կ����
#define TLV_TAG_QMJG   0x0018   //����ͨ��ǩ�����
#define TLV_TAG_WKMZBH 0x0019   //����ģ����
#define TLV_TAG_COSBB  0x001A   //COS�汾��
#define TLV_TAG_WYLH   0x001B   //Ψһ����
#define TLV_TAG_CFCS   0x001C   //�ط�����
#define TLV_TAG_SBL    0x0081   //�豸������
#define TLV_TAG_ZPSJ   0x0082   //��Ƭ����
#define TLV_TAG_SJTJ   0x0083   //ʱ��ͳ��
#define TLV_TAG_RZSJ   0x0084   //��־����
#define TLV_TAG_MW     0x8001   //����:������Tag����ܵĽ��	

//��Ƭ����tag
#define TLV_TAG_PIC_GCMY1 0x0203 //������Կ1
#define TLV_TAG_PIC_ZPMW1 0x0204 //��Ƭ����1
#define TLV_TAG_PIC_ZPMW2 0x0205 //��Ƭ����2
#define TLV_TAG_PIC    0x0207    //��ҵ��Ƭ����
#define TLV_TAG_BUSI   0x0208    //��ҵ������Ϣ
#define TLV_TAG_PIC_FLAG 0x0209  //�Ƿ�����Ƭ��־
#define TLV_TAG_TPK5  0x0211     //�̻�ͨ��-������Կ����(����������Ϣ)
#define TLV_TAG_TPK2  0x0212     //�̻�ͨ��-������Կ����(������Ƭ)
#define TLV_TAG_TPK3  0x0213     //�̻�ͨ��-������Կ����(����ָ��)

//ʱ��ͳ�Ʊ�ǩ
#define TLV_TIME_WKMZ 0x0304     //����ģ�鴦��ʱ��

//����Ӧ�ñ�ǩ

//��־ʹ�ñ�ǩ
#define TLV_LOG_RZKG     0x0701   //��־����-0x01-��,0x02-��
#define TLV_LOG_READSN   0x0702   //READ ���
#define TLV_LOG_SJWYBH   0x0703   //�ֻ�Ψһ���
#define TLV_LOG_HZXH     0x0704   //�������
#define TLV_LOG_SMSN     0x0705   //SAM_SN
#define TLV_LOG_COSV     0x0708   //COS�汾
#define TLV_LOG_HZLX     0x0709   //��������
#define TLV_LOG_CWM      0x0712   //������
#define TLV_LOG_CWNR     0x0713   //��������
#define TLV_LOG_HZYYBB   0x070A   //����Ӧ������汾��
#define TLV_LOG_WKMZYYSJ 0x071B   //����ģ��Ӧ�ô���ʱ��
#define TLV_LOG_AQMOSJ   0x071C   //�밲ȫģ��ͨѶʱ��
#define TLV_LOG_GETPIC   0x071D   //��ȡ��Ƭ��ʱ��
#define TLV_LOG_CLT      0x071F   //ͨѶ���̶��ͻ��˱�����ʱ��
#define TLV_LOG_SAM      0x071E   //��ȫģ�鴦��ʱ��
#define TLV_LOG_TCPTYPE  0x0727   //ͨѶģʽ

//TLV define.END

//GOPAN_BOX tag
#define TLV_GOPAN_JYDM     0x0001     //���״���
#define TLV_IDINFO_ENC     0x0002     //���֤��������
#define TLV_GOPAN_INDEX    0x0003     //���֤����
#define TLV_GOPAN_DN       0x0004     //���֤DN


#define READ_INFO_ONLINE   0x0101     //���߶�֤
#define READ_INFO_OFFLINE  0x0102     //���߶�֤

//GOPAN_BOX define.END

#define NC_CHAIN_ID_LEN 16
#define NC_IDENTITY_LEN 16
#define NC_PACKET_HEAD_SISE 24 // (((comm_packet_head*)NULL)->app_data - ((comm_packet_head*)NULL)->mark)

typedef struct _comm_packet_head{
	char len_buf[4];
	char mark[2];
	char chain_id[NC_CHAIN_ID_LEN];		//Ψһ����
	char sam_index;						//SAM����
	char retry_times;					//���Դ���
	char err_num;
	char rfu;
	char sam_addr1;
	char sam_addr2;
	char app_data[4]; // 0
}comm_packet_head;
typedef struct _comm_msg_head{
	char comm_type;        //ͨ������ 1-TCP  2-UDP
	char src_type;         //��Դ��־ 1-ͨ�� 2-ҵ��
	char end_flag;         //ҵ����̽�����־,1-����
	char ruf[1];           //���ڶ��루8�ֽڣ�
	char src_identity[NC_IDENTITY_LEN];  //��Ϣ��Դid
	int  src_identity_len; //��Ϣ��Դid����
	uint64_t time_stamp;   //���յ�����ʱ�������λ:ms
	struct _comm_info_st{
		int index;	// tcp(array index)
		int socket_fd; // tcp
		int client_addr_len; // udp
		struct sockaddr_in client_addr;  //udp �ͻ���IP��Ϣ
	}comm_info;
	int pack_len; // NC_PACKET_HEAD_SISE + len(app_data)
	comm_packet_head packet;
}comm_msg_head;  //ϵͳ�ڲ�,ͨ�ű���ͷ


#endif
