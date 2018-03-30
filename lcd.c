/*
 * lcd.c
 *
 *  Created on: 2017年8月19日
 *      Author: wansaiyon
 *
 *     http://blog.csdn.net/hexiaolong2009
 *     LCD驱动模块的抽象层，负责调用实际LCD模块的功能函数
 *
 */
/*原始移植包含*/
#include "lcd.h"
#include "stm32f4xx.h"

/*自己添加包含文件*/
#include "delay_us.h"	//用于调用延时函数
#include <stdio.h>		//用于调用printf
#include "usart.h"		//用于调用HAL_UART_Transmit


const lcd_drv_t* lcd_module[] = {
	&st7789v_module,
	&nt35510_module,
	/*any other lcd module*/
};

const lcd_drv_t *lcd_drv = 0;



void LCD_GPIO_Cfg(void)
{
	/*由于FSMC的GPIO引脚已经由CUBEMX生成，所以这里只设置RESET引脚*/
	GPIO_InitTypeDef GPIO_InitStruct;

	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void LCD_FMC_Cfg(void)
{
	/*
	 * STM32 CUBEMX已经生成FSMC的初始化代码，这里无需额外配置
	 * 如果你是使用的正点原子的模板或是自己搭建的模板，则需要实现LCD_GPIO_Cfg()及LCD_FMC_Cfg()两个函数
	*/
}

void LCD_Reset(void)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_ms(100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	delay_ms(100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	delay_ms(100);
}

uint8_t LCD_CheckID(void)
{
	uint32_t id;
	int i;

	for(i = 0; i < (sizeof(lcd_module) / sizeof(lcd_module[0])); i++)
	{
		id = lcd_module[i]->checkid();
		if(0 != id)
		{
//			printf("Found ID: %x\r\n", id);
			HAL_UART_Transmit(&huart1,(uint8_t *)&id,sizeof(id),10);
			lcd_drv = lcd_module[i];
			return 1;
		}
	}

	return 0;
}

void LCD_Init(void)
{
	LCD_GPIO_Cfg();
	LCD_FMC_Cfg();
	LCD_Reset();
	if(LCD_CheckID())
	{
		lcd_drv->init();
	}
}

void LCD_DrawPixel(uint16_t color, uint16_t x, uint16_t y)
{
	if(lcd_drv)
		lcd_drv->setpixel(color, x, y);
}

void LCD_FillRect(uint16_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	if(lcd_drv)
		lcd_drv->fillrect(color, x0, y0, x1, y1);
}

void LCD_DrawBitmap(uint16_t color[], uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{
	if(lcd_drv)
		lcd_drv->bitmap(color, x0, y0, width, height);
}

void LCD_Clear(uint16_t color)
{
	if(lcd_drv)
		lcd_drv->clear(color);
}
