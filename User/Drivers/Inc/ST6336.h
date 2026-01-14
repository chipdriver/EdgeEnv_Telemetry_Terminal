#ifndef ST6336_H_
#define ST6336_H_

/**
 * @头文件引用
 */
#include "i2c.h"        //I2C
#include "dwt_delay.h"  //延时函数

/**
 * 结构体
 */
typedef struct
{
    uint8_t touched;    //1=有触摸 0 = 无触摸
    uint8_t points;     //触摸点个数
    uint16_t x;         //第1点x
    uint16_t y;         //第1点y
    uint8_t raw[6];     //原始6字节（调式用，可不需要）
}FT6336_Touch_t;
/**
 * 宏定义
 */
#define  TP_RST_HIGH() do{GPIO_SetBits(GPIOB,GPIO_Pin_2);}while(0)      //复位引脚高
#define  TP_RST_LOW()  do{GPIO_ResetBits(GPIOB,GPIO_Pin_2);}while(0)    //复位引脚低
#define FT6336_ADDR  0x38                                               // 7位地址

/**
 * 函数声明
 */
uint8_t FT6336_ReadReg(uint8_t reg);    
void FT6336_Reset(void);                        //复位
void FT6336_ReadMulti(uint8_t reg,uint8_t *buf,uint8_t len); 
uint8_t FT6336_ReadTouch(FT6336_Touch_t *t);    //读取FT6336第一个触摸点
#endif /*ST6336_H_*/