#ifndef _DBG_H_
#define _DBG_H_

#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>

#define DBG_LVL_NONE           0
#define DBG_LVL_ERR            1
#define DBG_LVL_INFO           2
#define ERR_NO_FLAG_TRUE       1
#define ERR_NO_FLAG_FALSE      0

#define DBG_INFO(fmt, ...) \
do{ \
	printf("<func:%s, line:%d>: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}while(0)

#define DBG_ERROR(fmt, ...) \
do{ \
	printf("<func:%s, line:%d>: "fmt, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
}while(0)

void
dbg_info(int errnoflag, const char* fmt, ...);

void
dbg_error(int errnoflag, const char* fmt, ...);

#endif /* _DBG_H_ */
