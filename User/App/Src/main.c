#include "main.h"

//变量定义
TP_Ctx tp;                                             // 触摸上下文（合并版）
TP_Event e;                                             // 触摸事件输出

UI_PageId page = UI_PAGE_1;                             // 当前页面
uint16_t down_x = 0;                                    // 记录DOWN的x
uint16_t down_y = 0;                                    // 记录DOWN的y
FT6336_Touch_t t;							//第1个触摸点

// 可调参数
#define SWIPE_TH_X          60     // 超过 60px 认为想翻页
#define SWIPE_MAX_Y         40     // 竖向超过 40px 就不当水平滑动
#define ANIM_STEP           12     // 动画每步像素（越小越丝滑但更费性能）
#define ANIM_DELAY_MS       8      // 每步延时（控制动画速度）

// 把offset从当前值动画到目标值（每步重绘一次）
static void UI_AnimateTo(UI_PageId page, int16_t *offset_x, int16_t target)
{
    if (!offset_x) return;

    while (*offset_x != target)
    {
        if (*offset_x < target)
        {
            *offset_x += ANIM_STEP;
            if (*offset_x > target) *offset_x = target;
        }
        else
        {
            *offset_x -= ANIM_STEP;
            if (*offset_x < target) *offset_x = target;
        }

        UI_DrawSlide(page, *offset_x);   // 关键：每步都重绘，形成动画
        DWT_Delay_ms(ANIM_DELAY_MS);     // 控制速度（你已有DWT）
    }
}


int main(void)
{
    int16_t  offset_x = 0;           // 当前跟手偏移
    uint8_t  dragging = 0;           // 是否正在拖动
    uint8_t  drag_valid = 1;         // 是否仍然认为这是“水平滑动”（竖向大就置0）

	SystemInit();								//初始化系统
	DWT_Delay_Init();							//初始化DWT延时函数
	DHT11_Init();								//初始化DHT11
	TFT_GPIO_Init();							//初始化TFT GPIO
	SPI1_Init_Master(); 					    //初始化SPI1主机模式
	ST7789_MinInit();							//ST7789最小初始化
	ST7789_FillColor(0x0000); 		            // 黑底
	ST7789_SetRotation(0);				        //设置屏幕旋转方向
	UART1_Init();								//串口初始化
	I2C1_Init_ForTouch();					    //I2C1初始化
	FT6336_Reset();                             //复位 FT6336
	UI_ShowPage(page);                          // 先显示第一页

	while (1)
{
    // ① 每一轮都调用一次：更新 tp.pressed / tp.last_x / tp.last_y，并可能产生事件
    TP_Poll(&tp, &e);

    // ② DOWN：开始拖动，记录起点
    if (e.type == TP_EVT_DOWN)
    {
        down_x = e.x;
        down_y = e.y;

        offset_x = 0;
        dragging = 1;
        drag_valid = 1;
    }

    // ③ 跟手（关键）：不等 MOVE 事件！只要还按着，每轮都重绘
    if (dragging && tp.pressed)
    {
        int16_t dx = (int16_t)tp.last_x - (int16_t)down_x;
        int16_t dy = (int16_t)tp.last_y - (int16_t)down_y;

        if (i16_abs(dy) > SWIPE_MAX_Y) drag_valid = 0;

        if (drag_valid)
        {
            offset_x = dx;
            UI_DrawSlide(page, offset_x);
        }
    }

    // ④ UP：松手结算（翻页或回弹）
    if (e.type == TP_EVT_UP)
    {
        dragging = 0;

        if (!drag_valid)
        {
            UI_AnimateTo(page, &offset_x, 0);
            UI_ShowPage(page);
        }
        else if (offset_x <= -SWIPE_TH_X)
        {
            UI_AnimateTo(page, &offset_x, -240);  // 建议你后面换成 LCD_W
            page = (page == UI_PAGE_1) ? UI_PAGE_2 : UI_PAGE_1;
            offset_x = 0;
            UI_ShowPage(page);
        }
        else if (offset_x >= SWIPE_TH_X)
        {
            UI_AnimateTo(page, &offset_x, +240);
            page = (page == UI_PAGE_1) ? UI_PAGE_2 : UI_PAGE_1;
            offset_x = 0;
            UI_ShowPage(page);
        }
        else
        {
            UI_AnimateTo(page, &offset_x, 0);
            UI_ShowPage(page);
        }
    }

    DWT_Delay_ms(10);
}
}
	
