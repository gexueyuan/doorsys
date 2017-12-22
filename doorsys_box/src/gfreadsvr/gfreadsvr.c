#include "global_samser.h"
#include "global_sys.h"
#include "liblog.h"
#include "libgfreadpub.h"

static void gfread_exit(int SigUsr1)
{
	exit(0);
}

int main(int argc,char *argv[])
{
	int  iRet = 0;
	char sFilePath[256];
	char sLogPath[256];
	
	int ch = 0;
	int rc = 0;

	signal( SIGTERM,gfread_exit);
	signal( SIGINT,gfread_exit);
	signal( SIGQUIT,SIG_IGN);
	signal( SIGTSTP,SIG_IGN);
	signal( SIGHUP ,SIG_IGN);
	signal( SIGCHLD,SIG_IGN);
	signal( SIGPIPE,SIG_IGN);
	signal( SIGCLD ,SIG_IGN);
	
	setpgrp();

	iRet = gfread_busi_run();
//	iRet = test_gfread_run();
	return 0;
}
