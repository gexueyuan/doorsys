#ifndef _LIB_GFREADPUB_H_
#define _LIB_GFREADPUB_H_

#define RUN_MODE_MJ   0     //运行模式:0-gopan, 1-门禁

#ifdef __cplusplus
extern "C"
{
#endif

	int gfInitParam(const char *ip_addr, int ip_port);
	int gfread_init();
	int gfOpenDevice();
	int gfLedBegin();
	int gfLedFindCardIng();
	int gfLedFindCardSuc();
	int gfLedFindCardFail();
	int gfLedReadIdSuc();
	int gfLedReadIdFail();
	int gfLedClose();
	int gffind_card();
	int gfPrepare(char *sn_buf, char *cos_buf);
	int gfreadIDInfo(char *idcard_buf, unsigned int *idcard_len, char *ref_buf, int *ref_len, char *time_buf, int *time_len);
	int test_gfread_run();
	int gfDisConnect();
	int gfreadDevInit();
	int gfreadDevRead(unsigned char *pbSend, unsigned int unSLen, unsigned char *pbRecv, unsigned int *punRLen);
	int gfreadDevClose();
	int gfLedCosUpdate();
	int gfreadEncIdData(unsigned char *pbRecv, int *punRLen);
	int gfOffLineReadInfo(char *sDN, int *dn_len);

#ifdef __cplusplus
}
#endif
#endif
