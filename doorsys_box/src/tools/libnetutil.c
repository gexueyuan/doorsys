#include "global_sys.h"
#include "libutil.h"
#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32") 
#endif

void util_get_utc_time(time_t *pTime)
{
	time_t tNowTime;
	time(&tNowTime);
	*pTime = tNowTime;
	return;
}

unsigned long long util_get_time_stamp(unsigned long long *ull_stamp)
{
#ifndef WIN32
	unsigned long long m_stamp = 0;
	struct timeval stTimeval;

	gettimeofday(&stTimeval, NULL);
	m_stamp = stTimeval.tv_sec;
	*ull_stamp = m_stamp*1000 + stTimeval.tv_usec/1000;
#else
	SYSTEMTIME st;
	GetLocalTime(&st);
	*ull_stamp = st.wSecond*1000+st.wSecond,st.wMilliseconds;
#endif

	return *ull_stamp;
}

static unsigned int util_do_CRC16(unsigned char *pbData, int nLength)
{
	int i = 0; 
	int j = 0;
	unsigned char ch;
	unsigned char bit;
	unsigned int crc = CRC_SEED;
	for (i=0; i<nLength; i++)
	{
		ch= pbData[i];
		for (j=0; j<8; j++)
		{
			bit = (unsigned char)(crc & 1);
			if (ch & 1)
			{
				bit ^= 1;
			}
			if (bit)
			{
				crc ^= 0x4002;
			}
			crc >>= 1;
			if (bit)
			{
				crc |= 0x8000;
			}
			ch >>= 1;
		}
	}
	return crc;
}

int util_check_crc(char *in_buf, int in_len)
{
	unsigned short usCRC;
	unsigned short usTmpCRC;
	
	memcpy(&usCRC, in_buf+in_len-2, 2);
	usCRC = ntohs(usCRC);
	
	usTmpCRC = util_do_CRC16(in_buf, in_len-2);
	if(usTmpCRC != usCRC)
	{
		return -1;
	}
	
	return 0;
}

int util_make_crc(char *in_buf, int in_len)
{
		*((unsigned short *)(in_buf+in_len)) = htons(util_do_CRC16(in_buf, in_len));
		return 0;
}

int util_hex_to_asc(unsigned char *pbyInBuffer ,int iInBufLen ,unsigned char *pbyOutBuffer )
{

	int i,j;

	for (i=0,j=0; j<iInBufLen; j++, i=i+2)
	{
		if ( (pbyInBuffer[j]/16)>9 )
			pbyOutBuffer[i] = (pbyInBuffer[j]/16)+'0'+7;
		else    
			pbyOutBuffer[i] = (pbyInBuffer[j]/16)+'0';

		if ( (pbyInBuffer[j]%16)>9 )
			pbyOutBuffer[i+1] = (pbyInBuffer[j]%16)+'0'+7;
		else
			pbyOutBuffer[i+1] = (pbyInBuffer[j]%16)+'0';
	}
	
	return(0);
}

int util_asc_to_hex(char *pbyInBuffer ,unsigned char *pbyOutBuffer , int *iInBuffLen)
{               
	unsigned char n;
	int iLen,i;
	char cTmp;

	if(*iInBuffLen%2)
		iLen=*iInBuffLen+1;
	else
		iLen=*iInBuffLen;

	for (i=0; i<*iInBuffLen; i++)
	{
		cTmp=(unsigned char) toupper(pbyInBuffer[i]);

		if (i%2!=0)/* if odd number */
		{
			n =(unsigned char) (cTmp - 0x30);
			/*
			**  Note: 'A' = 65, 'F'= 70.  65-48 = 17, 17-7=10.
			**  For example, this will convert 'A' to value 10.
			*/
			if (n>9)
				n = n-7;
			pbyOutBuffer[i/2] = pbyOutBuffer[i/2] | n;
		}
		else
		{
			pbyOutBuffer[i/2] = ( (n=cTmp-0x30)>9 ? n-7:n ) << 4;
		}
	}
	*iInBuffLen = iLen/2;
	return(iLen/2);
}

static int util_tool_rtrim_ch(char *str, char *dest, char ch)
{
	int i,len;
	
	len=strlen(str);
	if (dest != str)
	{
		memcpy(dest, str, len+1);
	}
	
	for(i=len-1;i>=0;i--)
	{
		if(dest[i]==ch)
			dest[i]=0;
		else
			return (i+1);
	}
	return 0;
}

int util_tool_rtrim(char *str, char *dest)
{
	return util_tool_rtrim_ch(str,dest,' ');
}
