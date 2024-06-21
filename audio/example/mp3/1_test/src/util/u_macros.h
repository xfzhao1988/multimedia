//来自ffmpeg libutil模块的macros.h文件
/**
 * UM: 表示util macro的缩写
*/
#ifndef _U_MACROS_H_
#define _U_MACROS_H_

#define UM_MAX(a,b) ((a) > (b) ? (a) : (b))
#define UM_MAX3(a,b,c) UM_MAX(UM_MAX(a,b),c)

#define UM_MIN(a,b) ((a) > (b) ? (b) : (a))
#define UM_MIN3(a,b,c) UM_MIN(UM_MIN(a,b),c)

#define UM_SWAP(type,a,b) do{type SWAP_tmp= b; b= a; a= SWAP_tmp;}while(0)

#endif // end of #ifndef _U_MACROS_H_