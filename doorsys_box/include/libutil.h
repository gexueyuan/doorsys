#ifndef _LIB_UTIL_H_
#define _LIB_UTIL_H_

#define CRC_SEED           0x3E8C   //º∆À„CRC÷÷◊”

extern void util_get_utc_time(time_t *pTime);
extern unsigned long long util_get_time_stamp(unsigned long long *ull_stamp);
extern int util_check_crc(char *in_buf, int in_len);
extern int util_make_crc(char *in_buf, int in_len);
extern int util_hex_to_asc(unsigned char *pbyInBuffer ,int iInBufLen ,unsigned char *pbyOutBuffer );
extern int util_asc_to_hex(char *pbyInBuffer ,unsigned char *pbyOutBuffer , int *iInBuffLen);
extern int util_tool_rtrim(char *str, char *dest);


#endif
