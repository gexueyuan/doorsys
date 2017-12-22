#ifndef __IDCARDCOMPL_H__
#define __IDCARDCOMPL_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef WIN32
#include <winsock2.h>
#include <initguid.h>
#include <MMSystem.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mswsock.h>
#include <direct.h>
#include <winbase.h>
#pragma comment(lib, "Ws2_32.lib")

#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#endif

#include "AKeyBase.h"
#include "AKeyDef.h"
#include "AKeyHelper.h"
#include "AKeyError.h"
#include "lgntraceP.h"
#include "IDCardError.h"
#include "alg/OpenAlg.h"
#include "IDCardConfig.h"
#include "IDCardTask.h"
#include "IDCardUtil.h"
#include "IDCardInterface.h"
#include "IDCardTags.h"


#endif

