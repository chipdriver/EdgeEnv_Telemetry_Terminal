#include "ui.pages.h"

// 屏幕尺寸
#define LCD_W 240
#define LCD_H 320

int16_t i16_abs(int16_t v) { return (v < 0) ? -v : v; }

void UI_Init(void)
{
    // 目前不需要做任何事，留空也行
}

static void UI_DrawPage1(void)
{
    ST7789_FillColor(COLOR_BLACK);                  // 清屏：黑底
    ST7789_DrawTriangle(20,30,80,30,50,90,0xF800);  //画一个红色三角形
}

static void UI_DrawPage2(void)
{
    ST7789_FillColor(COLOR_BLACK);                          // 清屏：黑底
    ST7789_DrawTriangle(160,200,220,200,190,260,0x001F);    //画一个蓝色三角形
}

void UI_ShowPage(UI_PageId page)
{
    if (page == UI_PAGE_1)                                             // 如果是第一页
        UI_DrawPage1();                                                // 画一个红色三角形
    else                                                               // 否则
        UI_DrawPage2();                                                // 画一个蓝色三角形
}

// 把一个值限制在[min,max]
static int16_t clamp_i16(int16_t v, int16_t minv, int16_t maxv)
{
    if (v < minv) return minv;
    if (v > maxv) return maxv;
    return v;
}

// 画一个“页面内容”（目前只画数字），center_x = 页面中心X坐标
static void UI_DrawPageNumAtCenterX(int16_t center_x, const char *num_str)
{
    // 文字左上角坐标（简单粗暴居中，后面你换成真正页面内容再精调）
    int16_t x = center_x - 10;
    int16_t y = (LCD_H / 2) - 10;

    // 如果中心完全在屏幕外，也没必要画（省点性能）
    if (center_x < -LCD_W || center_x > (LCD_W * 2)) return;

    // 边界保护：防止出现负坐标传给LCD（你LCD函数可能不支持负数）
    if (x < 0) x = 0;
    if (y < 0) y = 0;

    ST7789_DrawCircle(20,20,30,0x001F);    //画一个蓝色三角形
}

/**
 * @brief 跟手滑动绘制（同时画两页）
 *
 * @param page     当前页（UI_PAGE_1 或 UI_PAGE_2）
 * @param offset_x 横向偏移：手指向右拖为正，向左拖为负
 *
 * @details
 * 画法：清屏后同时把“当前页”和“邻页”按偏移量画出来。
 * 例如：当前是Page1
 *   - Page1中心 = 屏幕中心 + offset_x
 *   - Page2中心 = Page1中心 + 屏宽
 *
 * 这样 offset_x 变化时，就形成“页面跟手移动”的视觉效果。
 */
void UI_DrawSlide(UI_PageId page, int16_t offset_x)
{
    // 把偏移限制在[-屏宽, +屏宽]，避免画飞
    offset_x = clamp_i16(offset_x, -LCD_W, LCD_W);

    // 清屏
    ST7789_FillColor(COLOR_BLACK);                          // 清屏：黑底

    // 屏幕中心X
    int16_t cx = LCD_W / 2;

    if (page == UI_PAGE_1)
    {
        // Page1 跟手移动
        int16_t p1_cx = cx + offset_x;
        // Page2 在 Page1 的右边一个屏宽
        int16_t p2_cx = p1_cx + LCD_W;

        UI_DrawPageNumAtCenterX(p1_cx, "1");
        UI_DrawPageNumAtCenterX(p2_cx, "2");
    }
    else // UI_PAGE_2
    {
        // Page2 跟手移动
        int16_t p2_cx = cx + offset_x;
        // Page1 在 Page2 的左边一个屏宽
        int16_t p1_cx = p2_cx - LCD_W;

        UI_DrawPageNumAtCenterX(p1_cx, "1");
        UI_DrawPageNumAtCenterX(p2_cx, "2");
    }
}