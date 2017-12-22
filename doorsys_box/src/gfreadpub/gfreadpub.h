#ifndef _GFREADPUB_IDCARDREADER_H_
#define _GFREADPUB_IDCARDREADER_H_

#include "AKeyInterface.h"
#include "global_sys.h"
#include "libzmqtools.h"
#include "libgfreadpub.h"
#include "luareader.h"

static int print_hex(LPBYTE buf, UINT32 unSLen)
{
	int i = 0;
	
	printf("[");
	for(i = 0; i < unSLen; i++)
	{
		printf("%02X", buf[i]);
	}
	printf("]\n");
	
	return 0;
}

//int gfZmqTransmit(void *context, unsigned char *send_buf, int send_len, unsigned char *recv_buf, int time_out)
//{
//	int rc = 0;
	
	//·¢ËÍ
//	zmq_send(context, send_buf, send_len, 0);
	
	//½ÓÊÕ
//	rc = zmq_poll_recv(context, (char *)recv_buf, 8192, time_out);
//	if(rc < 0)
//	{
//		printf("gfZmqTransmit:zmq_poll_recv err.rc:[%d]\n", rc);
//		return -1;
//	}

//	return rc;
//}


namespace AKey
{
	class IDCardReader : public IProtocol
	{
	public:
		HRESULT Init(HANDLE handle)
		{
			m_handle = handle;
		}
		
		virtual HRESULT Control(UINT32 unCmd, LPBYTE pbSend, UINT32 unSLen, LPBYTE pbRecv, LPUINT32 punRLen, UINT32 unTimeout)
		{
			HRESULT hr = 0;

			hr = luareader_transmit(m_handle, pbSend, unSLen, pbRecv, 10240, 3000);		
		//	hr = gfZmqTransmit(m_handle, pbSend, unSLen, pbRecv, 3);
			if(hr > 0)
			{
				*punRLen = hr;
				hr = 0;
			}
			else
			{
				hr = AKEY_RV_FAIL;
			}
			
			return hr;
		}
		virtual IProtocol * GetSubProtocol()
		{
			return NULL;
		}
		
	private:
		HANDLE  m_handle;
		
	};
};

#endif
