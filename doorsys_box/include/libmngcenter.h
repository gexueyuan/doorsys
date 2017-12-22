#ifndef _LIB_MNGCENTER_H_
#define _LIB_MNGCENTER_H_


extern int mng_stat_get_base_info(void *socket, mng_base_info *p_mng_base_info);
extern int mng_send_sam_stat(void *socket, int sam_index, int sam_error, mng_base_info *p_mng_base_info);
extern int mng_busi_get_base_info(void *socket, int sam_index, int sam_error, char *sam_sn, char *sam_cos, mng_base_info *p_mng_base_info);

#endif
