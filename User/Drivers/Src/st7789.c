#include "st7789.h"

uint16_t lcd_width = LCD_W;
uint16_t lcd_height = LCD_H;

uint16_t lcd_x_offset = 0;
uint16_t lcd_y_offset = 0;

/**
 * @brief 写命令
 * @param cmd 命令
 */
void LCD_WriteCmd(uint8_t cmd)
{
    LCD_CS_LOW();   //选中屏
    LCD_DC_LOW();   //命令模式
    SPI_SendByte(cmd); //发送命令
    LCD_CS_HIGH();  //取消选中屏
}

/**
 * @brief 写1字节数据
 * @param data 数据
 */
void LCD_WriteData8(uint8_t data)
{
    LCD_CS_LOW();   //选中屏
    LCD_DC_HIGH();  //数据模式
    SPI_SendByte(data); //发送数据
    LCD_CS_HIGH();  //取消选中屏
}

/**
 * @brief 写多个字节数据
 * @param buf 数据指针
 * @param len 数据大小
 */
void LCD_WriteDataBuf(uint8_t *buf, uint32_t len)
{
    LCD_CS_LOW();       //选中屏
    LCD_DC_HIGH();      //数据模式
    SPI_SendBuffer(buf, len); //发送数据
    LCD_CS_HIGH();      //取消选中屏
}

/**
 * @brief 复位LCD屏幕
 * @note 通过RST引脚硬件复位ST7789
 *       复位时序: 高→低(20ms)→高(120ms)
 */
void LCD_Reset(void)
{
    // 1. 拉高RST引脚 (确保初始状态)
    LCD_RST_HIGH();
    DWT_Delay_ms(1);        // 稳定1ms

    // 2. 拉低RST引脚 (触发复位)
    LCD_RST_LOW();
    DWT_Delay_ms(20);       // 保持低电平20ms (复位脉冲)

    // 3. 拉高RST引脚 (退出复位状态)
    LCD_RST_HIGH();
    DWT_Delay_ms(120);      // 等待120ms让LCD完成内部初始化
}

/**
 * @brief ST7789最小初始化
 * @note 目的：让屏从休眠退出、设置色深为 RGB565、打开显示。
 */
void ST7789_MinInit(void)
{
    LCD_Reset(); //硬件复位

    //1）Sleep Out 退出休眠
    LCD_WriteCmd(0x11);
    DWT_Delay_ms(120); //等待120ms

    //2）Color mode
    LCD_WriteCmd(0x3A); //设置色深命令
    LCD_WriteData8(0x55); //设置为16位色 (RGB565)

    //3) Display ON 打开显示
    LCD_WriteCmd(0x29);
    DWT_Delay_ms(20); //等待20ms

    //设定默认方向（竖屏）
    ST7789_SetRotation(0);
}

/**
 * @brief 设置显存窗口
 * @param x0 起始X坐标
 * @param y0 起始Y坐标
 * @param x1 结束X坐标
 * @param y1 结束Y坐标
 * @note 定义一个矩形区域，后续写入的像素数据将填充到这个区域
 */
void LCD_SetWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t d[4];  // 数据缓冲区，用于存储4字节坐标参数
    
    // ========== 设置列地址范围 (X坐标) ==========
    LCD_WriteCmd(0x2A);  // CASET命令：列地址设置
    
    // 拆分起始列地址x0为高低字节 (16位 → 2个8位)
    d[0] = (x0 >> 8) & 0xFF;  // x0高字节 (bit15-8)
    d[1] = x0 & 0xFF;         // x0低字节 (bit7-0)
    
    // 拆分结束列地址x1为高低字节
    d[2] = (x1 >> 8) & 0xFF;  // x1高字节 (bit15-8)
    d[3] = x1 & 0xFF;         // x1低字节 (bit7-0)
    
    LCD_WriteDataBuf(d, 4);   // 发送4字节列地址参数：[x0_H, x0_L, x1_H, x1_L]

    // ========== 设置行地址范围 (Y坐标) ==========
    LCD_WriteCmd(0x2B);  // RASET命令：行地址设置
    
    // 拆分起始行地址y0为高低字节
    d[0] = (y0 >> 8) & 0xFF;  // y0高字节 (bit15-8)
    d[1] = y0 & 0xFF;         // y0低字节 (bit7-0)
    
    // 拆分结束行地址y1为高低字节
    d[2] = (y1 >> 8) & 0xFF;  // y1高字节 (bit15-8)
    d[3] = y1 & 0xFF;         // y1低字节 (bit7-0)
    
    LCD_WriteDataBuf(d, 4);   // 发送4字节行地址参数：[y0_H, y0_L, y1_H, y1_L]

    // ========== 准备写入显存 ==========
    LCD_WriteCmd(0x2C);  // RAMWR命令：后续发送的数据将写入上述窗口区域
}

/**
 * @brief 全屏填充纯色（RGB565）
 * @param color 要填充的颜色（RGB565格式）
 * @note 原理：
 *  1）先用LCD_SetWindow(0,0,W-1,H-1)设定写入区域为全屏
 *  2）发送RAMWR后，持续写入像素数据（每个像素2个字节：高字节在前）
 *  3) 用小缓冲区循环发送，避免一次性占用大量RAM
 */
void ST7789_FillColor(uint16_t rgb565)
{
    /*1)设置写入窗口为全屏*/
    LCD_SetWindow(0,0,lcd_width-1,lcd_height-1);

    /*2)准备颜色数据缓冲区(512字节 = 256个像素)
    * 每个像素2字节，512字节可存256个像素数据
    * buf[i]  = 高字节
    * buf[i+1]= 低字节
    */
   uint8_t buf[512];
   for(uint16_t i = 0; i < 512; i += 2 )
   {
        buf[i] = (uint8_t)(rgb565 >> 8);     //高字节
        buf[i+1] = (uint8_t)(rgb565 & 0xFF); //低字节
   }

   /* 3)总像素数 = 宽 * 高
   * 核心目的：计算LCD屏幕的总像素数量，用于后续循环控制。
   */
   uint32_t total_pixels = (uint32_t)LCD_W * (uint32_t)LCD_H;

   /* 4) 循环写像素
   *   每轮发送chunk_pixels个像素数据(最多256像素)
   */
  while(total_pixels > 0 )
  {
    uint32_t chunk_pixels = sizeof(buf) / 2; //每次发送的像素数(256)
    if(total_pixels < chunk_pixels)
        chunk_pixels = total_pixels; //最后一轮可能不足256像素
    
    /*写入像素数据*/
    LCD_WriteDataBuf(buf, (uint32_t)chunk_pixels * 2); //每个像素2字节

    total_pixels -= chunk_pixels; //更新剩余像素数
  }
}

/**
 * @brief 设置屏幕旋转方向（0~3）
 * @param r 0:竖屏(默认) 1:横屏 2:竖屏180° 3:横屏180°
 *
 * 原理：写 ST7789 的 MADCTL(0x36) 寄存器来控制行列扫描方向
 * 注意：不同模组的 X/Y 偏移可能不同；如果后面发现画面偏移，再加 x/y offset 修正。
 */

void ST7789_SetRotation(uint8_t r)
{
    uint8_t madctl = 0x00;
    switch (r & 0x03)
    {
        case 0: //竖屏：默认
            madctl = 0x00;
            lcd_width = LCD_W;  // 240
            lcd_height = LCD_H; // 320
            break;
        case 1: //横屏
            madctl = 0x60;          // MX | MV
            lcd_width  = LCD_H;     // 320
            lcd_height = LCD_W;     // 240
            break;
        case 2://竖屏180：X镜像 + Y镜像
            madctl = 0xC0; //MX|MY
            lcd_width  = LCD_W;
            lcd_height = LCD_H;
            break;
        case 3://横屏180：交换XY + X镜像
            madctl = 0xA0; //MV|MX
            lcd_width  = LCD_H;
            lcd_height = LCD_W;
            break;
        default:
            break;
    }

    LCD_WriteCmd(0x36);      // MADCTL命令
    LCD_WriteData8(madctl);

    //方向改变后，把显存窗口重设位”整屏“【0,0,lcd_width - 1,lcd_height - 1】
    LCD_SetWindow(0, 0, lcd_width - 1, lcd_height - 1);
}

/**
 * @brief 写16位颜色数据的函数
 * @param data 16位颜色数据（RGB565格式）
 */
void LCD_WriteData16(uint16_t data)
{
    uint8_t d[2];
    d[0] = (data >> 8) & 0xFF;  // 高字节
    d[1] = data & 0xFF;         // 低字节
    LCD_WriteDataBuf(d, 2);
}

/**
 * @brief 画一个像素点
 * @param x X坐标
 * @param y Y坐标
 * @param color 颜色值（RGB565格式）
 */
void ST7789_DrawPixel(uint16_t x,uint16_t y,uint16_t color)
{
    if (x >= lcd_width || y >= lcd_height) return;  //越界保护（用逻辑宽高）

    LCD_SetWindow(x, y, x, y); //窗口设到一个点
    LCD_WriteData16(color);    //写入一个像素

}

/**
 * @brief 画线（水平）
 * @param x 起点
 * @param y 起点
 * @param len 线长
 * @param color 颜色
 */
void ST7789_DrawHLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    //参数检查
    if ( y >= lcd_height || x >= lcd_width ) return; //起点越界
    if (x + len  > lcd_width ) len = lcd_width - x; //长度修正，防止越界    

    //把窗口设置为：从(x,y) 到 (x + len -1,y)
    LCD_SetWindow(x, y, x + len - 1, y);

    // 连续写 len 个相同颜色的像素
    while( len-- )
    {
        LCD_WriteData16(color);
    }
}

/**
 * @brief 画线（垂直）
 * @param x 起点
 * @param y 起点
 * @param len 线长
 * @param color 颜色
 */
void ST7789_DrawVLine(uint16_t x, uint16_t y, uint16_t len, uint16_t color)
{
    //参数检查
    if(x >= lcd_width || x >= lcd_height) return; //起点越界
    if(y + len > lcd_height) len = lcd_height - y;

    //窗口：从（x,y) 到 （x,y + len - 1）
    LCD_SetWindow(x, y, x, y + len - 1);

    // 连续写 len 个相同颜色的像素
    while( len-- )
    {
        LCD_WriteData16(color);
    }

}


/**
 * @brief 实心矩形
 * @param x 起点X
 * @param y 起点Y
 * @param w 宽度
 * @param h 高度
 * @param color 颜色
 */
void ST7789_FilledRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    //参数检查
    if(x >= lcd_width || y >= lcd_height) return; //起点越界
    if(x + w > lcd_width) w = lcd_width - x; //矩形修正
    if(y + h > lcd_height) h = lcd_height - y;//矩形修正

    //设置显存窗口
    LCD_SetWindow(x,y,x+w-1,y+h-1);

    //一共要写w*h个像素
    uint32_t count = (uint32_t)w*h;

    //连续写count个相同颜色的像素
    while(count--)
    {
        LCD_WriteData16(color);
    }
}

/**
 * @brief 空心矩形
 * @param x 起点X
 * @param y 起点Y
 * @param w 宽度
 * @param h 高度
 * @param color 颜色
 */
void ST7789_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    //参数检查
    if(x >= lcd_width || y >= lcd_height || x == 0 || y == 0) return; //起点越界
    if(x + w >= lcd_width) w = lcd_width - x - 1; //矩形修正
    if(y + h >= lcd_height) h = lcd_height - y - 1;//矩形修正

    //上边
    ST7789_DrawHLine(x, y, w, color);
    //下边
    ST7789_DrawHLine(x, y + h - 1, w, color);
    //左边
    ST7789_DrawVLine(x, y, h, color);
    //右边
    ST7789_DrawVLine(x + w - 1, y, h, color);
}

/**
 * @brief 画一条任意方向的直线（使用 Bresenham 算法）
 * @param x0 起点X坐标
 * @param y0 起点Y坐标
 * @param x1 终点X坐标
 * @param y1 终点Y坐标
 * @param color 颜色
 */
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color)
{
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1; //x方向步进：+1 或 -1
    int dy = -abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1; //y方向步进：+1 或 -1
    int err = dx + dy; //误差值

    while(1)
    {
        ST7789_DrawPixel(x0,y0,color); //画点

        if(x0 == x1 && y0 == y1) break; //到达终点，退出

        int16_t e2 = 2 * err;

        if(e2 >= dy)
        {
            err += dy;
            x0 += sx;
        }

        if(e2 <= dx)
        {
            err += dx;
            y0 += sy;
        }
    }
}


/**
 * @brief 圆
 * @param x0 圆心X
 * @param y0 圆心Y
 * @param r 半径
 * @param color 颜色
 */
void ST7789_DrawCircle(uint16_t x0,uint16_t y0,uint16_t r,uint16_t color)
{
	int16_t x = 0;
	int16_t y = r;
	int16_t d = 1 - r;//初始决策参数
    int16_t cx = 0, cy = 0,c = 0;
	//画出圆上八个对称点的宏
	#define DRAW_CRICLE_POINTS(cx,cy,x,y,c) do {\
        ST7789_DrawPixel((cx) + (x), (cy) + (y), (c));                        \
        ST7789_DrawPixel((cx) - (x), (cy) + (y), (c));                        \
        ST7789_DrawPixel((cx) + (x), (cy) - (y), (c));                        \
        ST7789_DrawPixel((cx) - (x), (cy) - (y), (c));                        \
        ST7789_DrawPixel((cx) + (y), (cy) + (x), (c));                        \
        ST7789_DrawPixel((cx) - (y), (cy) + (x), (c));                        \
        ST7789_DrawPixel((cx) + (y), (cy) - (x), (c));                        \
        ST7789_DrawPixel((cx) - (y), (cy) - (x), (c));                        \
	}while(0)

	//初始化八个点
	DRAW_CRICLE_POINTS(x0,y0,x,y,color);
	
	while(x < y)
	{
		x++;
		if(d < 0)
		{
			//选择右边的点
			d += 2 * x + 1;
		}
		else
		{
			//选择右下的点
			y--;
			d += 2 * (x - y) + 1;
		}

		DRAW_CRICLE_POINTS(x0,y0,x,y,color);
	}

	#undef DRAW_CRICLE_POINTS //取消/删除宏DRAW_CRICLE_POINTS
}

/**
 * @brief 画一个空心三角形
 * @param x0,y0 顶点 1
 * @param x1,y1 顶点 2
 * @param x2,y2 顶点 3
 * @param color 颜色
 */
void ST7789_DrawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    //画三条边
    ST7789_DrawLine(x0, y0, x1, y1, color);
    ST7789_DrawLine(x1, y1, x2, y2, color);
    ST7789_DrawLine(x2, y2, x0, y0, color);
}

/**
 * @brief 在屏幕指定位置显示一张 RGB565 图片
 * @param x      左上角 X（逻辑坐标）
 * @param y      左上角 Y（逻辑坐标）
 * @param w      图片宽度（像素）
 * @param h      图片高度（像素）
 * @param img565 指向 RGB565 像素数组（长度 = w*h）
 *
 * 注意：
 * 1) img565 每个元素就是一个像素点的 RGB565 颜色值
 * 2) 数组顺序：从左到右、从上到下（行优先）
 */
void ST7789_DrawImage565(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t *img565)
{
    //1.越界保护：如果起点就在屏幕外，直接返回
    if(x >= lcd_width || y >= lcd_height )return;

    //2.裁剪宽高，防止越界写屏
    if ( x + w > lcd_width ) w = lcd_width -x;
    if ( y + h > lcd_width ) h = lcd_height - y;

    //3.设置写入窗口（目标区域）
    LCD_SetWindow(x,y,x + w - 1,y + h -1);

    //4.连续写 w*h个像素
    uint32_t count = (uint32_t)w*h;
    while(count--)
    {
        LCD_WriteData16(*img565++);
    }
}


/**
 * @brief 显示一个 8x16 ASCII 字符
 * @param x      左上角X（逻辑坐标）
 * @param y      左上角Y（逻辑坐标）
 * @param ch     字符（ASCII）
 * @param fg     前景色（字颜色）
 * @param bg     背景色（底色）
 */
void ST7789_DrawChar(uint16_t x,uint16_t y,char ch,uint16_t fg,uint16_t bg)
{
    //1.字符范围限制：字库只包含0x20~0x7F
    if((uint8_t)ch < 0x20 || (uint8_t)ch > 0x7F)
        ch = '?';

    //2.8*16的字符区域越界就不画（简单处理）
    if(x + 8 > lcd_width) return;
    if(y + 16 > lcd_height) return;
    //3.取出字模（16行，每行1字节）
    const uint8_t *bitmap = A_8x16;

    //4.逐行逐列画点
    for(uint8_t row = 0; row < 16;row++)
    {
        uint8_t bits = bitmap[row];//当前行8个像素点

        for(uint8_t col = 0; col < 8; col++)
        {
            //约定：bit7是最左边，bit0是最右边
            if(bits & (0x80 >> col ))
                ST7789_DrawPixel(x + col,y + row, fg);
            else
                ST7789_DrawPixel(x + col,y + row, bg);
        }
    }
}


