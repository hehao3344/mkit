#include "time1.h"

static u32 timing_delay0 = 0; //
static u32 timing_delay1 = 0; //
static u32 timing_delay2 = 0; //
static u32 timing_delay3 = 0; //
//
void time1_init(void)
{
    //TIM1_TimeBaseInit(16, TIM1_COUNTERMODE_UP, 1000, 0);
    TIM1_TimeBaseInit(8, TIM1_COUNTERMODE_UP, 1000, 0);
    TIM1_ARRPreloadConfig(ENABLE);
    TIM1_ITConfig(TIM1_IT_UPDATE, ENABLE);
    TIM1_Cmd(ENABLE);
}

// 每1ms加1 在定时器中 完成
void time1_increment(void)
{
    // 暂时没考虑数据溢出
    timing_delay0++;
    timing_delay1++;
    timing_delay2++;
    timing_delay3++;
}

void time1_set_value(u32 index, u32 value)
{
    if (0 == index)
    {
        timing_delay0 = value;
    }
    else if (1 == index)
    {
        timing_delay1 = value;
    }
    else if (2 == index)
    {
        /* 按键使用 */
        timing_delay2 = value;
    }
    else if (3 == index)
    {
        timing_delay3 = value;
    }    
}

u32 time1_get_value(u32 index)
{
    u32 ret = 0;
    if (0 == index)
    {
        ret = timing_delay0;
    }
    else if (1 == index)
    {
        ret = timing_delay1;
    }
    else if (2 == index)
    {
        ret = timing_delay2;
    }    
    else
    {
        ret = timing_delay3;
    }

    return ret;
}
