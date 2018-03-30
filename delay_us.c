/*
 * delay_us.c
 *
 *  Created on: 2017��8��20��
 *      Author: wansaiyon
 */

#include "delay_us.h"

/************************************	delay_us ˵��*********************************************
 *
 * ����cortex system timer�Ƿ��Ƶ,�δ�ʱ��SYSTICK��ʱ��Դ��Դ��������֮һ��
 *
 * ����Ƶ��
 * 1)	SysTick->CTRL��λ��2��1��ʹ���ں���������ʱ��FCLK��Ϊ�δ�ʱ����ʱ��Դ
 * 2)	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);��Ӧ16K������16K��Systickʱ�����ڲ���һ���ж�
 *
 * 8��Ƶ��
 * 1)	SysTick->CTRL��λ��2��0��ʹ���ⲿʱ��ԴHCLK/8��Ϊ�δ�ʱ����ʱ��Դ
 * 2)	HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/8000);��Ӧ2K������2K��Systickʱ�����ڲ���һ���ж�
 *
 *
 ***********************************************************************************************/

/**
 * ��ʱ��ص���Ҫ����
 */
static uint8_t  fac_us=0;           //��ʱ΢���Ƶ��
static uint16_t fac_ms=0;           //��ʱ�����Ƶ��

/**
 * ��ʱ��ʼ��
 */
void delay_init(void)
{
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8); 		//ѡ��ʱ��Դ-�ⲿʱ��-HCLK/8
    fac_us=SystemCoreClock/8000000; 								// 72/8 ��ʱ1΢��9��ʱ������
    fac_ms=(uint16_t)fac_us*1000;   								// ��ʱ1����9000��Cysticʱ������
}

/**
 * nus : ��ʱ����΢��
 */
void delay_us(uint32_t nus)
{
	uint32_t temp;
    //nus*fac_usֵ����ܳ���SysTick->LOAD(24λ)-1
    SysTick->LOAD=nus*fac_us;    									// ��������ֵ:n(us)*��ʱ1us��Ҫ���ٸ�SysTickʱ������
    SysTick->VAL=0x00;                       						// VAL��ʼ��Ϊ0
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; 						// ʹ��SysTick��ʱ��
    do
    {
    	temp=SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));    						// �ȴ�����ʱ�䵽��(λ16)
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;						// �ر�ʹ��
    SysTick->VAL =0X00;                      						// ����VAL
}

/**
 * nms : ��ʱ���ٺ���
 * @ע������ ����Ҫ����700ms���ٽ�ֵδʵ�ʲ����������׼
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
 * ns : ��500ms����ʱ������
 */
void delay_s(uint16_t ns)
{
	for (int var = 0; var < ns; ++var)
	{
		delay_ms(500);
		delay_ms(500);
	}
}
