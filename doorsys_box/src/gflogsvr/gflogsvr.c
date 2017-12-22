#include "global_samser.h"
#include "global_netcard.h"
#include "liblog.h"

int ig_loglevel;
int ig_runmode;
void log_exit(int SigUsr1)
{
	LOG_CLOSE();
	exit(0);
}

int main(int argc,char **argv)
{
	int iRet = 0;

	
	signal( SIGTERM,log_exit );
	signal( SIGINT, log_exit );
	signal( SIGQUIT,SIG_IGN );
	signal( SIGTSTP,SIG_IGN );
	signal( SIGHUP,SIG_IGN );
	signal( SIGCHLD,SIG_IGN);
	signal(SIGPIPE, SIG_IGN);

	gflog_run();

	LOG_CLOSE();
	exit(0);
}
