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

/* ========= MOVE 事件输出 ========= */
typedef struct {
    uint16_t x;     // 当前坐标
    uint16_t y;
    int16_t  dx;    // 相对“上一次上报MOVE”的位移
    int16_t  dy;
} TP_MoveEvent;

/* MOVE 检测上下文 */
typedef struct {
    uint8_t  was_pressed;     // 上一轮是否按下（只有按下状态才可能MOVE）
    uint16_t last_rep_x;      // 上一次“触发MOVE时”的坐标（rep=reported）
    uint16_t last_rep_y;

    uint16_t last_x;          // 最近一次有效坐标（可选：调试/备用）
    uint16_t last_y;
} TP_MoveCtx;

/**
* 宏定义
*/
#define TP_RELEASE_CONFIRM    2     //连续多少次按下才算真的”松手“
#define TP_MOVE_TH    3             //位移超过多少像素才认为“有效移动”
/**
 * 函数声明
 */
void TP_DOWNInit(TP_DownCtx *ctx);                          //Down初始化       
uint8_t TP_PollDown(TP_DownCtx *ctx,TP_DownEvent *out);     //轮询检测：返回1=产生DOWN；0=无DOWN

void TP_UpInit(TP_UpCtx *ctx);                              //UP初始化
uint8_t TP_PollUp(TP_UpCtx *ctx, TP_UpEvent *out);          //轮询检测：返回1=产生UP；0=无UP

void TP_MoveInit(TP_MoveCtx *ctx);                          //MOVE初始化
uint8_t TP_PollMove(TP_MoveCtx *ctx, TP_MoveEvent *out);    //轮询检测：返回1=产生MOVE；0=无MOVE


/*================================================================合并DOMW/MOVE/UP========================================================================*/


/*  结构体  */
/* ========= 统一事件类型 ========= */
typedef enum {
    TP_EVT_NONE = 0,
    TP_EVT_DOWN,
    TP_EVT_MOVE,
    TP_EVT_UP,
} TP_EventType;

/* ========= 统一事件输出 ========= */
typedef struct {
    TP_EventType type;   // 本次事件类型：DOWN/MOVE/UP/NONE

    uint16_t x;          // 当前/最后坐标
    uint16_t y;

    int16_t  dx;         // 仅 MOVE 有意义：相对上一次MOVE上报点的位移
    int16_t  dy;

} TP_Event;

/* ========= 统一上下文（合并版） ========= */
typedef struct 
{
    uint8_t pressed;            //check whether it is currently pressed 检查当前是否按下
    uint8_t was_pressed;        //上一轮是否按下（用于DOWN/UP判断）

    uint8_t release_cnt;        //UP确认：连续未按下计数

    uint16_t start_x;           //DOWN起点
    uint16_t start_y;

    uint16_t last_x;            //最后一次有效坐标x
    uint16_t last_y;            //最后一次有效坐标y

    uint16_t last_rep_x;        //上一次MOVE上报的参考点
		uint16_t last_rep_y;
}TP_Ctx;

/* 初始化合并上下文 */
void TP_Init(TP_Ctx *ctx);

/* 轮询产生事件：返回1=产生事件；0=无事件 */
uint8_t TP_Poll(TP_Ctx *ctx, TP_Event *evt);

#endif  /*__TP_EVENT_H__*/