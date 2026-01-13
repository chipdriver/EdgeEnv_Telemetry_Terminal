#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>
/**
 * 每个字符 16 行，每行 8 个像素 = 1 字节
 * 共 16 字节表示一个字符（从上到下）
 *
 * bit7 是最左边的像素，bit0 是最右边的像素（常见约定）
 */
extern const uint8_t Font8x16[96][16]; // ASCII 0x20~0x7F
extern const uint8_t A_8x16[16];
#endif //__FONT_H__