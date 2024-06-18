#ifndef _U_BS_SWAP_H_
#define _U_BS_SWAP_H_

#include <stdint.h>

/**
在计算机领域中，大小端（Endian）指的是数据在内存中存储的字节序（顺序）方式。有两种常见的字节序：

大端序（Big-Endian）：
数据的高位字节存储在低地址，低位字节存储在高地址。
例如，十六进制数 0x12345678 在大端序下的存储顺序为 12 34 56 78。

小端序（Little-Endian）：
数据的低位字节存储在低地址，高位字节存储在高地址。
例如，十六进制数 0x12345678 在小端序下的存储顺序为 78 56 34 12。
在编程中，可以通过以下方法来判断当前系统的字节序是大端序还是小端序：

#include <stdio.h>

int main() {
    unsigned int num = 0x12345678;
    unsigned char *ptr = (unsigned char *)&num;

    if (*ptr == 0x12) {
        printf("Big-Endian\n");
    } else if (*ptr == 0x78) {
        printf("Little-Endian\n");
    } else {
        printf("Unknown Endian\n");
    }

    return 0;
}

说明：
这段程序利用了指针 ptr 将 num 的地址强制转换为 unsigned char * 类型，这样就可以逐个字节访问 num 的内容。
通过检查 *ptr 的值来判断字节序：如果 *ptr 的值是 0x12，则是大端序；如果是 0x78，则是小端序。
注意：这种方法要求确保 unsigned int 的大小是 4 个字节，且符合系统的内存布局。

实例程序和结果打印：
实例：
#include <stdio.h>

#define U_B_SWAP16C(x) (((x) << 8 & 0xff00)  | ((x) >> 8 & 0x00ff))
#define U_B_SWAP32C(x) (U_B_SWAP16C(x) << 16 | U_B_SWAP16C((x) >> 16))
#define U_B_SWAP64C(x) (U_B_SWAP32C(x) << 32 | U_B_SWAP32C((x) >> 32))

int main(int argc, char *argv[])
{
  uint16_t a = 0x1234, bs_a = U_B_SWAP16C(a);
  uint8_t *pa = (uint8_t*)&a, *p_bs_a = (uint8_t*)&bs_a;
  uint32_t b = 0x12345678, bs_b = U_B_SWAP32C(b);
  uint8_t *pb = (uint8_t*)&b, *p_bs_b = (uint8_t*)&bs_b;
  uint64_t c = 0x1020304050607080, bs_c = U_B_SWAP64C(c);
  uint8_t *pc = (uint8_t*)&c, *p_bs_c = (uint8_t*)&bs_c;

  printf("%#x %#x\n", *pa, *(pa+1)); //a在内存中存储顺序，可以用来看系统的大小端。大端：数据的高位字节存储在低地址，低位字节存储在高地址;小端：数据的低位字节存储在低地址，高位字节存储在高地址。
  printf("%p %p\n", pa, (pa+1)); //地址pa小于地址pa+1
  printf("%#x %#x\n\n", *p_bs_a, *(p_bs_a+1));

  printf("%#x %#x %#x %#x\n", *pb, *(pb+1), *(pb+2), *(pb+3));
  printf("%p %p %p %p\n", pb, (pb+1), (pb+2), (pb+3));
  printf("%#x %#x %#x %#x\n\n", *p_bs_b, *(p_bs_b+1), *(p_bs_b+2), *(p_bs_b+3));

  printf("%#x %#x %#x %#x %#x %#x %#x %#x\n", *pc, *(pc+1), *(pc+2), *(pc+3), *(pc+4), *(pc+5), *(pc+6), *(pc+7));
  printf("%#x %#x %#x %#x %#x %#x %#x %#x\n", *p_bs_c, *(p_bs_c+1), *(p_bs_c+2), *(p_bs_c+3), *(p_bs_c+4), *(p_bs_c+5), *(p_bs_c+6), *(p_bs_c+7));

	return 0;
}

结果打印：
0x34           0x12 //变量a=0x1234按照字节打印，高位字节12存在高地址0x7ffd392a416d，低位字节34存在低地址0x7ffd392a416c，表明该系统为小端字节序
0x7ffd392a416c 0x7ffd392a416d
0x12           0x34

0x78           0x56           0x34           0x12
0x7ffd392a4170 0x7ffd392a4171 0x7ffd392a4172 0x7ffd392a4173
0x12           0x34           0x56           0x78

0x80 0x70 0x60 0x50 0x40 0x30 0x20 0x10
0x10 0x20 0x30 0x40 0x50 0x60 0x70 0x80

*/
/**
 * 运算符优先级参考：https://blog.csdn.net/yuliying/article/details/72898132
 * 位运算符优先级：从上到下以此减小
 * ~(按位取反运算符)
 * <<(左移)
 * >>(右移)
 * &(按位与)
 * ^(按位异或)
 * |(按位或)
*/

#define U_B_SWAP16C(x) (((x) << 8 & 0xff00)  | ((x) >> 8 & 0x00ff)) //双字节的大小端转换宏
#define U_B_SWAP32C(x) (U_B_SWAP16C(x) << 16 | U_B_SWAP16C((x) >> 16)) //四字节的大小端转换宏
#define U_B_SWAP64C(x) (U_B_SWAP32C(x) << 32 | U_B_SWAP32C((x) >> 32)) //八字节的大小端转换宏

#define U_B_SWAPC(s, x) U_B_SWAP##s##C(x)

static inline const uint16_t u_b_swap16(uint16_t x)
{
    x= (x>>8) | (x<<8);
    return x;
}

static inline const uint32_t u_b_swap32(uint32_t x)
{
    return U_B_SWAP32C(x);
}

static inline const uint64_t u_b_swap64(uint64_t x)
{
    return (uint64_t)u_b_swap32(x) << 32 | u_b_swap32(x >> 32);
}

// be2ne ... big-endian to native-endian
// le2ne ... little-endian to native-endian

#if U_HAVE_BIGENDIAN
#define u_be2ne16(x) (x)
#define u_be2ne32(x) (x)
#define u_be2ne64(x) (x)
#define u_le2ne16(x) u_bswap16(x)
#define u_le2ne32(x) u_bswap32(x)
#define u_le2ne64(x) u_bswap64(x)
#define U_BE2NEC(s, x) (x)
#define U_LE2NEC(s, x) U_BSWAPC(s, x)
#else
#define u_be2ne16(x) u_bswap16(x)
#define u_be2ne32(x) u_bswap32(x)
#define u_be2ne64(x) u_bswap64(x)
#define u_le2ne16(x) (x)
#define u_le2ne32(x) (x)
#define u_le2ne64(x) (x)
#define U_BE2NEC(s, x) U_BSWAPC(s, x)
#define U_LE2NEC(s, x) (x)
#endif

#define U_BE2NE16C(x) U_BE2NEC(16, x)
#define U_BE2NE32C(x) U_BE2NEC(32, x)
#define U_BE2NE64C(x) U_BE2NEC(64, x)
#define U_LE2NE16C(x) U_LE2NEC(16, x)
#define U_LE2NE32C(x) U_LE2NEC(32, x)
#define U_LE2NE64C(x) U_LE2NEC(64, x)

#endif //_U_BS_SWAP_H_