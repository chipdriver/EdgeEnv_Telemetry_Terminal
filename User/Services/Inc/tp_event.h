#ifndef __TP_EVENT_H__
#define __TP_EVENT_H__

/**
 * 头文件
 */
#include "ST6336.h"

/**
 * 结构体
 */
/* ========= DOWN 事件输出 ========= */
typedef struct
{
    uint16_t x;
    uint16_t y;
}TP_DownEvent;

/* DOWN 检测上下文（需要常驻保存） */
typedef struct{
    uint8_t was_pressed;    //上一轮是否按下
}TP_DownCtx;

/**
* 宏定义
*/

/**
 * 函数声明
 */
void TP_DOWNInit(TP_DownCtx *ctx);
uint8_t TP_PollDown(TP_DownCtx *ctx,TP_DownEvent *out);



/* ========= UP 事件输出 ========= */
typedef struct {
    uint16_t x;          // 松手时采用“最后一次有效坐标”
    uint16_t y;
} TP_UpEvent;

/* UP 检测上下文 */
typedef struct {
    uint8_t  was_pressed;     // 上一轮是否按下（用于判断 1->0）
    uint8_t  release_cnt;     // 连续“未按下”的计数（用于松手确认）
    uint16_t last_x;          // 最近一次有效坐标
    uint16_t last_y;
} TP_UpCtx;

/* 初始化 */
void TP_UpInit(TP_UpCtx *ctx);

/* 轮询检测：返回1=产生UP；0=无UP */
uint8_t TP_PollUp(TP_UpCtx *ctx, TP_UpEvent *out);


#endif  /*__TP_EVENT_H__*/