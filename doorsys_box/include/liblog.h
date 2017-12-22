#ifndef __LOG_H_INCLUDED__
#define __LOG_H_INCLUDED__

#define L_ALERT -3	/*!< Alert level */
#define L_CRIT  -2	/*!< Critical level */
#define L_ERR   -1	/*!< Error level */
#define L_WARN   1	/*!< Warning level */
#define L_NOTICE 2	/*!< Notice level */
#define L_INFO   3	/*!< Info level */
#define L_DBG    4	/*!< Debug level */


#ifndef NO_DEBUG
	#undef NO_LOG
#endif

void log_open(char* zmq_dest, char* module, int loglevel);
void log_close();
void log_print(int level, char *fmt,...);
void log_printex(int level, char * filename, int lineno, char *fmt,...);
void log_hex(int level,char* data,int len,char *fmt,...);
int log_server(char* zmq_dest, char * log_path);

#ifdef NO_LOG
	#define LOG_OPEN(zmq_dest, module, loglevel)
	#define LOG_CLOSE()
# ifndef WIN32
	#define LOG_PRINT(level, fmt, args...)
	#define LOG_PRINTEX(level, filename, lineno, fmt, args...)
	#define LOG_HEX(level, data, len, fmt, args...)
	#define NO_DEBUG(level, filename, lineno, fmt, args...)
# else
	#define LOG_PRINT __noop
	#define LOG_PRINT __noop
	#define LOG_HEX __noop
	#define NO_DEBUG __noop
# endif
#else /* NO_LOG */
	#define LOG_OPEN log_open
	#define LOG_CLOSE log_close
	#define LOG_PRINT log_print
	#define LOG_PRINTEX log_printex
	#define LOG_HEX log_hex

//log_printex(L_DBG, __FILE__, __LINE__, fmt, ##args)
	#ifdef NO_DEBUG
		#define LOG_DEBUG(level, filename, lineno, fmt, args...)
	#else
		#define LOG_DEBUG LOG_PRINTEX
	#endif
#endif

#endif /* __LOG_H_INCLUDED__ */
