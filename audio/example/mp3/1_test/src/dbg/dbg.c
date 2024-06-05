#include"dbg.h"
#include<errno.h>
#include<stdarg.h>

#ifdef MAX_LINE_LEN
#undef MAX_LINE_LEN
#endif

#define MAX_LINE_LEN 1024

static int _dbg_level = DBG_LVL_ERR;
static void dbg_print(int level, int errnoflag, const char* fmt, va_list vp);

void
dbg_info(int errnoflag, const char* fmt, ...)
{
	va_list vp;

	va_start(vp, fmt);
	dbg_print(DBG_LVL_INFO, errnoflag, fmt, vp);
	va_end(vp);

	return;
}

void
dbg_error(int errnoflag, const char* fmt, ...)
{
	va_list vp;

	va_start(vp, fmt);
	dbg_print(DBG_LVL_ERR, errnoflag, fmt, vp);
	va_end(vp);

	return;
}

static void
dbg_print(int level, int errnoflag, const char* fmt, va_list vp)
{
	if(level < _dbg_level)
		return;

	char msg[MAX_LINE_LEN] = {0};
	int len = 0;

	vsnprintf(msg, MAX_LINE_LEN - 2, fmt, vp);

	len = strlen(msg);

	if(errnoflag != 0)
	{
		snprintf(msg + len, MAX_LINE_LEN - len - 2, ", msg: %s\n", strerror(errno));
	}

	strcat(msg, "\n");
	fflush(stdout);
	fputs(msg, stdout);
	fflush(NULL);

	return;
}