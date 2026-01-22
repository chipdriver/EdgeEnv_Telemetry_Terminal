#ifndef __UI_PAGES_H__
#define __UI_PAGES_H__

#include <stdint.h>
#include "st7789.h"
#ifdef __cplusplus
extern "C" {
#endif

// 页面编号
typedef enum {
    UI_PAGE_1 = 0,
    UI_PAGE_2 = 1,
} UI_PageId;

/**
 * 宏定义
 */
#define SWIPE_TH_X      60      // 横向超过60像素，认为是滑动
#define SWIPE_MAX_Y     40      // 竖向移动超过40像素，认为不是水平滑动
#define LCD_W 240
#define LCD_H 320
/**
 * 函数声明
 */
int16_t i16_abs(int16_t v);         //绝对值
void UI_Init(void);                 //初始化UI（如果需要初始化字体/背景等，可放这里）
void UI_ShowPage(UI_PageId page);   //切换并绘制页面
void UI_DrawSlide(UI_PageId page, int16_t offset_x);// 跟手滑动渲染：page=当前页，offset_x=当前横向偏移（可正可负）
#ifdef __cplusplus
}
#endif

#endif
