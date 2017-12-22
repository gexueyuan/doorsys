#include "global_sys.h"
#include "global_netcard.h"
#include "global_samser.h"
#include "liblog.h"
#include "libtlv.h"
#include "libzmqtools.h"
#include "libgfreadpub.h"

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

int main(int argc, char *argv[])
{
	int rc = 0;
	int  req_len = 0;
	int  rsp_len = 0;
	char req_buf[1024];
	char rsp_buf[1024];

	if(argc != 2)
	{
		printf("Useage: %s + apdu\n", argv[0]);
		return -1;
	}

	rc = gfreadDevInit();
	if(rc < 0)
	{
		printf("gfreadDevInit error.rc:[%d]\n", rc);
		return rc;
	}

	req_len = strlen(argv[1]);
	util_asc_to_hex(argv[1], req_buf, &req_len);
	printf("req_len:[%d]\n", req_len);
	rc = gfreadDevRead(req_buf, req_len, rsp_buf, &rsp_len);
	if(rc != 0)
	{
		printf("gfreadDevRead error.rc:[%d]\n", rc);
		return rc;
	}
	printf("rsp:");
	print_hex(rsp_buf, rsp_len);

	gfreadDevClose();
	
	return 0;
}
