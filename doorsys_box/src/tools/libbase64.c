#include "global_sys.h"

static const char base64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
/**************************************************************
** ������  : base64_encode
** ��  ��  : BASE64����
** ��  ��  : ���Ľ�
** ��������: 2015/12/9
** ��������:
**     ����:
**	    bin_data       ���뱨��
**          bin_size       ���뱨�ĳ���
**     ���:
**          base64_data    ���ܺ���
**          *base64_size   ���ܺ��ĳ���
** ����ֵ  : 0-��ȷ ����:����
**************************************************************/
int base64_encode(char *bin_data,int bin_size,char *base64_data,int *base64_size)
{
	int i,j,k;
	int blk_size,remain_size;
	unsigned char * p;
	unsigned char left[3];
	int i64;

	blk_size = bin_size / 3;
	remain_size = bin_size % 3;

	p =(unsigned char *) bin_data;
	j = 0;
	i64=0;

	for(i=0;i<blk_size;i++)
	{
		k = (p[0] & 0xFC) >> 2;
		base64_data[j++] = base64_table[k];

		k = ((p[0] & 0x03) << 4) | (p[1] >> 4);
		base64_data[j++] = base64_table[k];

		k = ((p[1] & 0x0F) << 2) | (p[2] >> 6);
		base64_data[j++] = base64_table[k];

		k = p[2] & 0x3F;
		base64_data[j++] = base64_table[k];

		i64++;i64++;i64++;i64++;

		p += 3;
	}

	switch(remain_size)
	{
	case 0:
		break;

	case 1:
		left[0] = p[0];
		left[1] = 0;
		p = left;

		k = (p[0] & 0xFC) >> 2;
		base64_data[j++] = base64_table[k];
		k = ((p[0] & 0x03) << 4) | (p[1] >> 4);
		base64_data[j++] = base64_table[k];

		base64_data[j++] = '=';
		base64_data[j++] = '=';
		break;

	case 2:
		left[0] = p[0];
		left[1] = p[1];
		left[2] = 0;
		p = left;

		k = (p[0] & 0xFC) >> 2;
		base64_data[j++] = base64_table[k];
		k = ((p[0] & 0x03) << 4) | (p[1] >> 4);
		base64_data[j++] = base64_table[k];
		k = ((p[1] & 0x0F) << 2) | (p[2] >> 6);
		base64_data[j++] = base64_table[k];
		base64_data[j++] = '=';
		break;

	default:
		break;
	}

	base64_data[j] = 0;
	*base64_size=j;
	return *base64_size;
}

/**************************************************************
** ������  : base64_decode
** ��  ��  : BASE64����
** ��  ��  : ���Ľ�
** ��������: 2015/12/9
** ��������:
**     ����:
**	    base64_data       ���뱨��
**          base64_size       ���뱨�ĳ���
**     ���:
**          bin_data          ���ܺ���
**          *bin_size         ���ܺ��ĳ���
** ����ֵ  : 0-��ȷ ����:����
**************************************************************/
int base64_decode(const char *base64_data,int base64_size,char *bin_data,int *bin_size)
{
	int i,j,k,m,n,l;
	unsigned char four_bin[4];
	char four_char[4];
	char c;

	j = base64_size;
	i = 0;
	l = 0;

	for(;;)
	{
		if((i+4) > j)
		{
			break;
		}

		k=0;
		while(k<4)
		{
			if(i == j)
			{
				break;
			}

			c = base64_data[i++];
			if((c == '+') || (c == '/') || (c == '=') ||
				((c >= '0') && (c <= '9')) ||
				((c >= 'A') && (c <= 'Z')) ||
				((c >= 'a') && (c <= 'z')))
			{
				four_char[k++] = c;
			}
		}

		if(k != 4)
		{
			return -1;
		}

		n = 0;
		for(k=0;k<4;k++)
		{
			if(four_char[k] != '=')
			{
				for(m=0;m<64;m++)
				{
					if(base64_table[m] == four_char[k])
					{
						four_bin[k] = (unsigned char)m;
						break;
					}
				}
			}
			else
			{
				n++;
			}
		}

		switch(n)
		{
		case 0:
			bin_data[l++] = (four_bin[0] << 2) | (four_bin[1] >> 4);
			bin_data[l++] = (four_bin[1] << 4) | (four_bin[2] >> 2);
			bin_data[l++] = (four_bin[2] << 6) | four_bin[3]; 
			break;

		case 1:
			bin_data[l++] = (four_bin[0] << 2) | (four_bin[1] >> 4);
			bin_data[l++] = (four_bin[1] << 4) | (four_bin[2] >> 2);
			break;

		case 2:
			bin_data[l++] = (four_bin[0] << 2) | (four_bin[1] >> 4);
			break;

		default:
			break;
		}

		if(n != 0)
		{
			break;
		}
	}

	*bin_size = l;
	return 0;
}
