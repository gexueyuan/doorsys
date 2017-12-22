#ifndef _LIB_ANALYSYSCONF_H_
#define _LIB_ANALYSYSCONF_H_

#ifdef __cplusplus
extern "C"
{
#endif

extern int conf_getstring(char *section,char *entry,char *value,int maxsize, char *filename);
extern int conf_getint(char *section,char *entry,char *filename);

#ifdef __cplusplus
}
#endif

#endif
