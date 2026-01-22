#include "tp_event.h"

/**
 * @brief 上下文初始化
 */
void TP_DOWNInit(TP_DownCtx *ctx)
{
    if(!ctx) return;    //参数保护：ctx 为NULL 直接返回，避免空指针
    ctx->was_pressed = 0;   //初始状态设为“上一轮未按下”
}

/**
 * @brief 轮询检测触摸“按下瞬间”（DOWN 事件）
 *
 * @details
 * 目标：把“持续的触摸状态(pressed/not pressed)”转换成“只触发一次的按下事件”。
 *
 * 输入：
 *   - ctx：用于保存历史状态（上一轮是否按下 was_pressed）
 *   - out：用于输出 DOWN 事件坐标（可传 NULL 表示不需要坐标）
 *
 * 依赖：
 *   - FT6336_ReadTouch_Filtered(&t)
 *       返回 1：当前检测到按下且坐标有效（t.x/t.y 可用）
 *       返回 0：当前无触摸 或 本次数据无效
 *
 * 输出/返回值：
 *   - 返回 1：本次检测到 DOWN（从“没按下”->“按下”的跳变）
 *   - 返回 0：本次没有 DOWN（要么一直按着，要么没按下）
 *
 * 行为特性：
 *   - 手指第一次落下：触发一次（返回1）
 *   - 手指一直按住不动：不会重复触发（一直返回0）
 *   - 松手后再按：会再次触发一次（返回1）
 */
uint8_t TP_PollDown(TP_DownCtx *ctx, TP_DownEvent *out)
{
    if (!ctx) return 0;                          // 保护：ctx 为空，无法保存历史状态，直接认为无事件

    FT6336_Touch_t t;                            // 临时变量：用于接收触摸坐标数据（本轮读取到的 x/y）
    uint8_t pressed = FT6336_ReadTouch_Filtered(&t); // 读取当前触摸状态：1=按下且坐标有效；0=没按下/无效

    // -------------------- 判定 DOWN：上一轮没按下 && 当前按下 --------------------
    // ctx->was_pressed 表示“上一轮”是否处于按下状态
    // pressed         表示“当前这一轮”是否处于按下状态
    // 只有从 0 -> 1 的跳变，才是“按下瞬间（DOWN）”
    if ((ctx->was_pressed == 0) && (pressed == 1))   // 条件：上一轮没按下，而这一轮按下
    {
        ctx->was_pressed = 1;                      // 更新历史：从现在开始，系统认为“已经处于按下状态”

        if (out)                                   // out 不为空才输出（允许用户只关心事件，不关心坐标）
        {
            out->x = t.x;                           // 输出 DOWN 时刻的 X 坐标
            out->y = t.y;                           // 输出 DOWN 时刻的 Y 坐标
        }

        return 1;                                   // 明确告诉上层：本轮产生了 DOWN 事件
    }

    // -------------------- 状态更新：松手后允许下一次再次产生 DOWN --------------------
    // 如果当前没按下（pressed==0），说明手指已经离开屏幕或本轮无效
    // 那么我们把历史状态清零，表示“已经松手”
    // 这样下次再按下时，就能再次满足 0->1，产生新的 DOWN
    if (pressed == 0)                               // 条件：当前没按下
    {
        ctx->was_pressed = 0;                       // 更新历史：标记为“未按下”，为下一次 DOWN 做准备
    }

    return 0;                                       // 默认情况：本轮没有 DOWN 事件（可能一直按着，也可能一直没按）
}

void TP_UpInit(TP_UpCtx *ctx)
{
    if (!ctx) return;                 // 保护：空指针直接返回

    ctx->was_pressed = 0;             // 过去是否被按下，初始认为“未按下”
    ctx->release_cnt = 0;             // 松手确认计数清零
    ctx->last_x = 0;                  // 最近坐标清零（可选）
    ctx->last_y = 0;
}

/**
 * @brief 轮询检测触摸“松开瞬间”（UP 事件）
 *
 * @details
 * 目标：把“持续的触摸状态(pressed/not pressed)”转换成“只触发一次的松开事件”。
 *
 * 依赖：
 *   - FT6336_ReadTouch_Filtered(&t)
 *       返回 1：当前按下且坐标有效
 *       返回 0：当前无触摸或本次数据无效
 *
 * 关键点（松手确认）：
 *   - 某些情况下会偶发读到 pressed=0（丢帧/干扰）
 *   - 为避免误判松手：要求连续 TP_RELEASE_CONFIRM 次 pressed=0
 *     才真正触发 UP
 *
 * 输出/返回值：
 *   - 返回 1：本轮产生 UP（确认松手成立）
 *   - 返回 0：本轮无 UP
 *
 * UP 坐标说明：
 *   - 松手时已经没有新坐标，因此输出“最后一次有效坐标(last_x/last_y)”
 */
uint8_t TP_PollUp(TP_UpCtx *ctx, TP_UpEvent *out)
{
    if (!ctx) return 0;                               // 保护：ctx为空无法保存历史状态

    FT6336_Touch_t t;                                 // 临时变量：接收本轮读取的坐标
    uint8_t pressed = FT6336_ReadTouch_Filtered(&t);  // 读取当前状态：1=按下且坐标有效；0=未按下/无效

    if (pressed)                                      // 如果当前确实按下
    {
        ctx->was_pressed = 1;                          // 标记：处于按下状态
        ctx->release_cnt = 0;                          // 清除“松手确认计数”（因为还在按着）

        ctx->last_x = t.x;                             // 记录最近一次有效坐标
        ctx->last_y = t.y;

        return 0;                                      // 按着不算 UP，本轮无事件
    }

    // 走到这里，说明 pressed == 0（当前读到未按下或无效）
    // 如果之前从未按下过（was_pressed==0），那只是一直没按，不会产生 UP
    if (ctx->was_pressed == 0)                         // 之前就没按下
    {
        ctx->release_cnt = 0;                          // 保守：计数清零
        return 0;                                      // 无 UP
    }

    // 走到这里：之前是按下状态，现在读到未按下 —— 可能是真松手，也可能是丢帧
    ctx->release_cnt++;                                // 累计连续“未按下”次数

    if (ctx->release_cnt < TP_RELEASE_CONFIRM)         // 未达到确认次数：先不触发 UP
    {
        return 0;                                      // 暂不认为松手
    }

    // 达到确认次数：正式判定 UP
    ctx->was_pressed = 0;                              // 状态回到未按下
    ctx->release_cnt = 0;                              // 计数清零，为下一次触摸准备

    if (out)                                           // 如果需要输出 UP 信息
    {
        out->x = ctx->last_x;                          // 输出最后一次有效坐标
        out->y = ctx->last_y;
    }

    return 1;                                          // 本轮产生 UP 事件
}


// 工具：int16 绝对值（MOVE 判定会用）
static int16_t TP_i16_abs(int16_t v)
{
    return (v < 0) ? (int16_t)(-v) : v;
}

/**
 * @brief 初始化 MOVE 事件检测上下文
 *
 * @details
 * 该函数用于初始化 TP_MoveCtx 结构体中的所有状态变量，
 * 必须在第一次调用 TP_PollMove() 之前调用一次。
 *
 * MOVE 事件属于“持续按下过程中的位移事件”，
 * 内部依赖历史状态（was_pressed）和参考点（last_rep_x/last_rep_y），
 * 如果不初始化，会导致第一次 MOVE 判断异常。
 *
 * @param ctx MOVE 事件检测上下文指针（需要常驻保存）
 */
void TP_MoveInit(TP_MoveCtx *ctx)
{
    if (!ctx) return;                 // 参数保护：ctx 为空则直接返回，防止空指针访问

    ctx->was_pressed = 0;             // 标记为“上一轮未按下”，为首次按下建立正确初始状态

    ctx->last_rep_x  = 0;             // 上一次“上报 MOVE”的参考点 X 坐标清零
    ctx->last_rep_y  = 0;             // 上一次“上报 MOVE”的参考点 Y 坐标清零

    ctx->last_x = 0;                  // 最近一次有效坐标清零（可选，用于记录当前触点位置）
    ctx->last_y = 0;                  // 最近一次有效坐标清零
}

/**
 * @brief 轮询检测触摸“移动事件”（MOVE）
 *
 * @details
 * 目标：只有当手指在按下状态下移动到“一定距离”，才上报 MOVE，
 *      以过滤轻微抖动，避免 UI 抖、按钮乱跳。
 *
 * 依赖：
 *   - FT6336_ReadTouch_Filtered(&t)
 *       返回 1：当前按下且坐标有效
 *       返回 0：当前无触摸或本次数据无效
 *
 * MOVE 触发条件：
 *   - 必须处于按下状态（pressed=1）
 *   - 当前坐标相对“上一次上报MOVE的坐标(last_rep)”的位移 >= TP_MOVE_TH
 *
 * 输出/返回值：
 *   - 返回 1：本轮产生 MOVE（out 中带 x/y/dx/dy）
 *   - 返回 0：本轮无 MOVE（可能没按下，或位移不足阈值）
 *
 * 注意：
 *   - 第一次按下（刚进入按下状态）不会产生 MOVE，只会初始化参考点
 *   - 松手后会复位状态，下一次按下重新开始
 */
uint8_t TP_PollMove(TP_MoveCtx *ctx, TP_MoveEvent *out)
{
    if (!ctx) return 0;                               // 保护：ctx为空无法保存状态

    FT6336_Touch_t t;                                 // 临时变量：接收本轮坐标
    uint8_t pressed = FT6336_ReadTouch_Filtered(&t);  // 读取：1=按下且有效；0=未按下/无效

    if (!pressed)                                     // 如果当前没有按下
    {
        ctx->was_pressed = 0;                         // 状态复位：标记为未按下
        return 0;                                     // 未按下不可能MOVE
    }

    // 走到这里说明 pressed == 1（当前按下且坐标有效）
    ctx->last_x = t.x;                                // 记录最近一次有效坐标（可选）
    ctx->last_y = t.y;

    if (ctx->was_pressed == 0)                        // 如果这是“刚按下后的第一轮”
    {
        ctx->was_pressed = 1;                         // 标记进入按下状态
        ctx->last_rep_x = t.x;                        // 初始化“MOVE参考点”为当前坐标
        ctx->last_rep_y = t.y;
        return 0;                                     // 刚按下不报MOVE（应该由DOWN去报）
    }

    // 走到这里：处于持续按下状态，开始判断是否“有效移动”
    int16_t dx = (int16_t)t.x - (int16_t)ctx->last_rep_x; // 相对上次MOVE参考点的X位移
    int16_t dy = (int16_t)t.y - (int16_t)ctx->last_rep_y; // 相对上次MOVE参考点的Y位移

    // 轻量距离判定（曼哈顿距离）：|dx| + |dy|
    // 优点：计算快，够用；缺点：不是严格欧式距离，但触摸阈值判断足够
    uint16_t dist = (uint16_t)(TP_i16_abs(dx) + TP_i16_abs(dy));

    if (dist < TP_MOVE_TH)                            // 位移不够：认为是抖动/微动，不上报
    {
        return 0;                                     // 无 MOVE
    }

    // 位移达到阈值：产生 MOVE
    ctx->last_rep_x = t.x;                            // 更新“MOVE参考点”为当前坐标
    ctx->last_rep_y = t.y;

    if (out)                                          // 如果需要输出事件
    {
        out->x  = t.x;                                // 输出当前坐标
        out->y  = t.y;
        out->dx = dx;                                 // 输出位移（相对上一次MOVE）
        out->dy = dy;
    }

    return 1;                                         // 本轮产生 MOVE
}

/*================================================================合并DOMW/MOVE/UP========================================================================*/

/*工具：int16绝对值*/
static int16_t TP_i16_abc(int16_t v)
{
    return (v < 0) ? (uint16_t)(-v) : v;
}

/**
 * @brief 初始化触摸曾上下文（合并版）
 * @details 将所有与DOWN/MOVE/UP相关的状态同意放到一个IP_Ctx里，避免多套Ctx之间不同步
 */
void TP_Init(TP_Ctx *ctx)
{
	if(!ctx) return;
	
	ctx->pressed = 0;				//当前是否按下（初始化为未按下）
	ctx->was_pressed = 0;		//上一轮是否按下（初始化为未按下）
	ctx->release_cnt = 0;		//松手确认计数清零
	
	ctx->start_x = 0;				//DOWN起点清零
	ctx->start_y = 0;
	
	ctx->last_x = 0;				//最近坐标清零
	ctx->last_y = 0;
	
	ctx->last_rep_x = 0;		//MOVE参考点清零
	ctx->last_rep_y = 0;		
}

/**
* @brief 轮询输出触摸事件
* @details 
*	输入：底层触摸状态（是否按下 + 坐标）
*	输出：离散事件（DOWN/MOVE/UP），并携带坐标/位移等信息
*	
*	事件判定原则：
*		- DOWN；上一轮未按下，&& 当前按下
*		- MOVE；持续按下状态，位移超过阈值 TP_MOVE_TH
*		- UP  ；上一轮按下 && 当前连续 TP_RELEASE_CONFIRM 轮未按下
*	
*	返回值：
*		-1 ：本轮产生了事件(evt->type != NONE)
*		-0 ：本轮无事件
*/
uint8_t TP_Poll(TP_Ctx *ctx,TP_Event *evt)
{
    //参数检查：空指针直接返回
    if(!ctx || !evt)    return 0;

    // ===== 1) 事件输出结构体先清为 NONE =====
    evt->type = TP_EVT_NONE;                          // 默认无事件
    evt->x = 0;                                       // 默认坐标清零（无事件时没意义）
    evt->y = 0;
    evt->dx = 0;                                      // 默认位移清零
    evt->dy = 0;

    // ===== 2) 读取底层触摸（你已有的Filtered函数） =====
    FT6336_Touch_t t;                                   //临时接收本轮坐标
    uint8_t pressed = FT6336_ReadTouch_Filtered(&t);    //1=按下且坐标有效；0=未按下/无效
    ctx->pressed = pressed;                             //保存当前是否按下到上下文

    // ===== 3) 如果当前按下：更新最近坐标，并清松手计数 =====
    if(pressed)         //当前按下
    {
        ctx->last_x = t.x;  //更新最近一次的有效坐标
        ctx->last_y = t.y;  //更新最近一次的有效坐标

        ctx->release_cnt = 0;   //清楚UP确认计数
    }

    // ===== 4) 判定 DOWN：0->1 的跳变 =====
    if((ctx->was_pressed == 0) && (pressed == 1))   //上一轮未按下 && 当前按下
    {
        ctx->was_pressed = 1;       //标记进入按下状态

        ctx->start_x = ctx->last_x;  //记录DOWN起点
        ctx->start_y = ctx->last_y;  //记录DOWN起点

        ctx->last_rep_x = ctx->last_x;  //初始化MOVE参考点（第一次按下时）
        ctx->last_rep_y = ctx->last_y;  //初始化MOVE参考点（第一次按下时）

        evt->type = TP_EVT_DOWN;        //输出事件类型DOWN
        evt->x = ctx->last_x;           //输出DOWN坐标
        evt->y = ctx->last_y;
        
        return 1;       //本轮产生事件，直接返回
    }

    // ===== 5) 持续按下状态：尝试判定 MOVE =====
    if(pressed && (ctx->pressed == 1))  //当前按下 且 已处于按下周期中
    {
        int16_t dx = (int16_t)ctx->last_x - (int16_t)ctx->last_rep_x;   //相对上次MOVE上报点的移动距离
        int16_t dy = (int16_t)ctx->last_y - (int16_t)ctx->last_rep_y;   //相对上次MOVE上报点的移动距离

        uint16_t dist = (uint16_t)(TP_i16_abs(dx) + TP_i16_abs(dy) );   //曼哈顿距离判定

        if(dist >= TP_MOVE_TH)
        {
            ctx->last_rep_x = ctx->last_x;            // 更新MOVE参考点到当前
            ctx->last_rep_y = ctx->last_y;

            evt->type = TP_EVT_MOVE;                  // 输出事件类型：MOVE
            evt->x = ctx->last_x;                     // 输出当前坐标
            evt->y = ctx->last_y;
            evt->dx = dx;                             // 输出位移（相对上次MOVE）
            evt->dy = dy;
            return 1;   //本轮产生事件，直接返回
        }
    }

    // ===== 6) 判定 UP：按下周期中，连续N轮未按下 =====
    if((ctx->was_pressed == 1) && (pressed == 0))
    {
        ctx->release_cnt++;                           // 累计连续未按下次数

        if(ctx->release_cnt >= TP_RELEASE_CONFIRM)      // 达到确认阈值：正式UP
        {
            ctx->was_pressed = 0;                     // 回到未按下状态
            ctx->release_cnt = 0;                     // 计数清零

            evt->type = TP_EVT_UP;                    // 输出事件类型：UP
            evt->x = ctx->last_x;                     // UP时输出最后一次有效坐标
            evt->y = ctx->last_y;
            return 1;       //本轮产生事件，直接返回
        }
    }

    return 0;   //本轮无事件
}
 



