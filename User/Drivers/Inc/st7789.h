#ifndef __ST7789_H__
#define __ST7789_H__

/********************************
 *             头文件
 ********************************/
#include "spi.h"
#include "tft_gpio.h"
#include "dwt_delay.h"
#include "stdlib.h"
#include "font.h"
/********************************
 *          宏定义
 ********************************/
//屏幕分辨率（240 * 320）
#define LCD_W 240  
#define LCD_H 320 

//当前”逻辑坐标系“的宽高，会随着SetRotation改变
extern uint16_t lcd_width;
extern uint16_t lcd_height;

//在当前的XY偏移（大部分模组先设为0，后面如果发现由偏移再调）
extern uint16_t lcd_x_offset;
extern uint16_t lcd_y_offset;

//颜色
#define COLOR_BLACK   0x0000
#define COLOR_WHITE   0xFFFF
#define COLOR_RED     0xF800
#define COLOR_GREEN   0x07E0
#define COLOR_BLUE    0x001F
#define COLOR_YELLOW  0xFFE0

/********************************
 *          头文件声明
 ********************************/
void ST7789_MinInit(void); //ST7789最小初始化
void ST7789_FillColor(uint16_t rgb565); //填充纯色
void ST7789_SetRotation(uint8_t r); //设置屏幕旋转方向（0~3）
void ST7789_DrawPixel(uint16_t x,uint16_t y,uint16_t color); //画一个像素点
void ST7789_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color); //画一条线（水平）
void ST7789_DrawVLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color); //画一条线（垂直）
void ST7789_FilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);//实心矩形
void ST7789_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);//空心矩形
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);//通用直线
void ST7789_DrawCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color); //圆
void ST7789_DrawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color);//三角形
void ST7789_DrawImage565(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t *img565); //图片
void ST7789_DrawChar(uint16_t x,uint16_t y,char ch,uint16_t fg,uint16_t bg);//字体
#endif // __ST7789_H__
