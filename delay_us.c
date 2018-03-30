/*
 * delay_us.c
 *
 *  Created on: 2017年8月20日
 *      Author: wansaiyon
 */

#include "delay_us.h"

/************************************	delay_us 说明*********************************************
 *
 * 根据cortex system timer是否分频,滴答定时器SYSTICK的时钟源来源如下两者之一：
 *
 * 不分频：
 * 1)	SysTick->CTRL的位段2置1，使用内核自由运行时钟FCLK作为滴答定时器的时钟源
 * 2)	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);对应16K，设置16K个Systick时钟周期产生一次中断
 *
 * 8分频：
 * 1)	SysTick->CTRL的位段2置0，使用外部时钟源HCLK/8作为滴答定时器的时钟源
 * 2)	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/8000);对应2K，设置2K个Systick时钟周期产生一次中断
 *
 *
 ***********************************************************************************************/

/**
 * 延时相关的重要变量
 */
static uint8_t  fac_us=0;           //延时微秒的频率
static uint16_t fac_ms=0;           //延时毫秒的频率

/**
 * 延时初始化
 */
void delay_init(void)
{
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8); 		//选择时钟源-外部时钟-HCLK/8
    fac_us=SystemCoreClock/8000000; 								// 72/8 延时1微秒9个时钟周期
    fac_ms=(uint16_t)fac_us*1000;   								// 延时1毫秒9000个Cystic时钟周期
}

/**
 * nus : 延时多少微秒
 */
void delay_us(uint32_t nus)
{
	uint32_t temp;
    //nus*fac_us值最大不能超过SysTick->LOAD(24位)-1
    SysTick->LOAD=nus*fac_us;    									// 设置重载值:n(us)*延时1us需要多少个SysTick时钟周期
    SysTick->VAL=0x00;                       						// VAL初始化为0
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; 						// 使能SysTick定时器
    do
    {
    	temp=SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));    						// 等待计数时间到达(位16)
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;						// 关闭使能
    SysTick->VAL =0X00;                      						// 重置VAL
}

/**
 * nms : 延时多少毫秒
 * @注意事项 ：不要超过700ms（临界值未实际测出），否则不准
 */
void delay_ms(uint16_t nms)
{
	uint32_t temp;
    SysTick->LOAD=(uint32_t)nms*fac_ms;
    SysTick->VAL =0x00;
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ;
    do
    {
        temp=SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;
    SysTick->VAL =0X00;
}

/**
 * ns : 用500ms来延时多少秒
 */
void delay_s(uint16_t ns)
{
	for (int var = 0; var < ns; ++var)
	{
		delay_ms(500);
		delay_ms(500);
	}
}
