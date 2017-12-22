#include "global_sys.h"
#include "libanalysisconf.h"

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#define CFG_LINE_LEN   128


static void ClearBlank(char * line)
{
    int i = 0, j, k, len;
    char buf[CFG_LINE_LEN + 1];

    while (line[i] != 0)
    {
        if (line[i] == ';' || line[i] == '\n' || (line[i] == '/' && line[i+1] == '/'))
        {
            line[i] = 0;
            for (j = i - 1; line[j] == ' '; line[j--] = 0);
            break;
        }
        i++;
    }

    i = 0;
	len = (int)strlen(line);
    while ((line[i] != '=') && (i < len)) i++;
    if (i == len) return;

    for (j = i - 1; line[j] == ' '; j--);
    for (k = i + 1; line[k] == ' '; k++);

    memset(buf, 0, sizeof(buf));
    memcpy(buf, line, j + 1);
    buf[j + 1] = '=';
    strcat(buf + j + 2, line + k);

    strcpy(line, buf);
}

static int IsSection(char * line)
{
    return line[0] == '[';
}

static int IsThisSection(char * line, char * pszSection)
{
    return !memcmp(line + 1, pszSection, strlen(pszSection));
}

static int IsThisEntry(char * line, char * pszEntry)
{
    return (!memcmp(line, pszEntry, strlen(pszEntry)) &&
           line[strlen(pszEntry)] == '=') ;
}

static int CutContent(char * line, char * pszDestination, int cbReturnBuf)
{
    int i = 0;

    while (line[i++] != '=');

    strncpy(pszDestination, line + i, cbReturnBuf);
    return 0;

}

int conf_getstring(char *pszSection,char *pszEntry,char *pszDestination,int cbReturnBuf, char *pszFileName)
{
    FILE * fp;
    char line[CFG_LINE_LEN];
    int cbNum = -1;
    int InThisSection = FALSE;
    int iRet;

    fp = fopen(pszFileName, "r");
    if (fp == NULL)
    {
        /*OnLog(9, "Read [%s]INI fail!", pszFileName);  */
        return -1;
    }

    while ( NULL != fgets(line, CFG_LINE_LEN, fp) )
    {
        ClearBlank(line);

        if (IsSection(line))
        {
            InThisSection = IsThisSection(line, pszSection);
            continue;
        }
        if (InThisSection == FALSE) continue;


        iRet=IsThisEntry(line, pszEntry);
        if (iRet==1)
        {
            cbNum = CutContent(line, pszDestination, cbReturnBuf);
            break;
        }
    }

    fclose(fp);

    if (cbNum == -1)
    {
        /*OnLog(9, "Read INI fail!,pszEntry=[%s],line=[%s]", pszEntry, line); */
    }

    return cbNum;

}

int conf_getint(char * pszSection, char * pszEntry, char * pszFileName)
{
    char buf[80];
    int  i;
    
     memset(buf, 0, sizeof(buf));
    if (conf_getstring(pszSection, pszEntry, buf, 80, pszFileName) < 0) {
        return -1;
    } else {
        sscanf(buf, "%i", &i);
        return i;
/*        return (int)atol(buf);*/
    }
}
