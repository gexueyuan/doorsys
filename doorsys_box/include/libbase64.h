#ifndef _LIB_BASE64_H_
#define _LIB_BASE64_H_

extern int base64_encode(char *bin_data,int bin_size,char *base64_data,int *base64_size);
extern int base64_decode(const char *base64_data,int base64_size,char *bin_data,int *bin_size);

#endif
