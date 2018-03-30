/*
 * st7789v.c
 *
 *  Created on: 2017年8月19日
 *      Author: wansaiyon
 *
 *     http://blog.csdn.net/hexiaolong2009
 *     正点原子NT35510 LCD模块的驱动接口，参照正点原子NT35510修改而来
 *     LCD模块：480*800电容屏
 */

/*原始移植包含*/
#include "stm32f4xx.h"
#include "lcd.h"
#include <stdio.h>

/*自己添加包含文件*/
#include "delay_us.h"	//用于调用延时函数



uint32_t ST7789V_CheckID(void)
{
	uint8_t id1, id2, id3, id4;

	LCD_WR_REG(0XBF);
	LCD_RD_DATA();//dummy read

	id1 = LCD_RD_DATA();//0X01
	id2 = LCD_RD_DATA();//0XD0
	id3 = LCD_RD_DATA();//0X68
	id4 = LCD_RD_DATA();//0X04

// 	printf(" Read ID:%x, %x, %x, %x\r\n", id1, id2, id3, id4);

	if((id1 == 0x01) && (id2 == 0xD0) && (id3 == 0x68) && (id4 == 0x04))
		return 0x7789;
	else
		return 0;
}

////////VCC=2.8V  IOVCC=1.8V//////////////////////
static void ST7789V_Init(void)
{
	int i;

	//---------------------------------------------------------------------------------------------------//
	LCD_WR_REG(0x11);
	delay_ms(120); //Delay 120ms
	//--------------------------------ST7789VS Frame rate setting----------------------------------//
	LCD_WR_REG(0xb2);
	LCD_WR_DATA(0x0c);
	LCD_WR_DATA(0x0c);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x33);
	LCD_WR_DATA(0x33);
	LCD_WR_REG(0xb7);
	LCD_WR_DATA(0x35);
	//---------------------------------ST7789VS Power setting--------------------------------------//
	LCD_WR_REG(0xbb);
	LCD_WR_DATA(0x35);//0x35
	LCD_WR_REG(0xc0);
	LCD_WR_DATA(0x2c);
	LCD_WR_REG(0xc2);
	LCD_WR_DATA(0x01);

	LCD_WR_REG(0xc3);///
	LCD_WR_DATA(0x18);///0x0b//0x15

	LCD_WR_REG(0xc4);
	LCD_WR_DATA(0x20);
	LCD_WR_REG(0xc6);//inversion
	LCD_WR_DATA(0x0f);
	LCD_WR_REG(0xca);
	LCD_WR_DATA(0x0f);
	LCD_WR_REG(0xc8);
	LCD_WR_DATA(0x08);
	LCD_WR_REG(0x55);
	LCD_WR_DATA(0x90);
	LCD_WR_REG(0xd0);
	LCD_WR_DATA(0xa4);
	LCD_WR_DATA(0xa1);//a1
	//--------------------------------ST7789VS gamma setting---------------------------------------//
	LCD_WR_REG(0xe0);
	LCD_WR_DATA(0x00);//D0
	LCD_WR_DATA(0x02);
	LCD_WR_DATA(0x04);
	LCD_WR_DATA(0x06);
	LCD_WR_DATA(0x0d);
	LCD_WR_DATA(0x2a);
	LCD_WR_DATA(0x38);//20  [6:0]
	LCD_WR_DATA(0x44);//36  [2:0]
	LCD_WR_DATA(0x4a);//43  [6:0]
	LCD_WR_DATA(0x1a);
	LCD_WR_DATA(0x16);
	LCD_WR_DATA(0x15);
	LCD_WR_DATA(0x1c);
	LCD_WR_DATA(0x1b);

	LCD_WR_REG(0xe1);
	LCD_WR_DATA(0x00);//D0
	LCD_WR_DATA(0x02);
	LCD_WR_DATA(0x04);
	LCD_WR_DATA(0x06);
	LCD_WR_DATA(0x0d);
	LCD_WR_DATA(0x2a);
	LCD_WR_DATA(0x38);//20  [6:0]
	LCD_WR_DATA(0x44);//36 27
	LCD_WR_DATA(0x4a);//43  [6:0]
	LCD_WR_DATA(0x1a);
	LCD_WR_DATA(0x16);
	LCD_WR_DATA(0x15);
	LCD_WR_DATA(0x1c);
	LCD_WR_DATA(0x1b);

	/* Set address mode */
	LCD_WR_REG(0x36);
	LCD_WR_DATA(0x00);

	/* Set pixel format */
	LCD_WR_REG(0x3a);
	LCD_WR_DATA(0x55);

	/* Set column address */
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0xEf);

	/* Set page address */
	LCD_WR_REG(0x2b);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x00);
	LCD_WR_DATA(0x01);
	LCD_WR_DATA(0x3f);

	LCD_WR_REG(0x29);
	LCD_WR_REG(0x2C);

	for(i = 0; i < 240 * 320; i++)
	{
		LCD_WR_DATA(0);
	}
}

static void ST7789V_SetPixel(uint16_t color, uint16_t x, uint16_t y)
{
	LCD_WR_REG(0x2a);
	LCD_WR_DATA(x >> 8);//start x
	LCD_WR_DATA(x & 0xff);
	LCD_WR_DATA(x >> 8);//end x
	LCD_WR_DATA(x & 0xff);

	LCD_WR_REG(0x2b);
	LCD_WR_DATA(y >> 8);//start x
	LCD_WR_DATA(y & 0xff);
	LCD_WR_DATA(y >> 8);//end x
	LCD_WR_DATA(y & 0xff);

	LCD_WR_REG(0x2c);//transfor begin

	LCD_WR_DATA(color);
}

static void ST7789V_FillRect(uint16_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	int i, w, h;

	w = x1 - x0 + 1;
	h = y1 - y0 + 1;

	LCD_WR_REG(0x2a);
	LCD_WR_DATA(x0 >> 8);//start x
	LCD_WR_DATA(x0 & 0xff);
	LCD_WR_DATA(x1 >> 8);//end x
	LCD_WR_DATA(x1 & 0xff);

	LCD_WR_REG(0x2b);
	LCD_WR_DATA(y0 >> 8);//start x
	LCD_WR_DATA(y0 & 0xff);
	LCD_WR_DATA(y1 >> 8);//end x
	LCD_WR_DATA(y1 & 0xff);

	LCD_WR_REG(0x2c);//transfor begin
	for(i = 0; i < w*h; i++)
	{
		LCD_WR_DATA(color);
	}
}

static void ST7789V_Clear(uint16_t color)
{
	ST7789V_FillRect(color, 0, 0, 239, 319);
}

static void ST7789V_DrawBitmap(uint16_t color[], uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{
	int i;
	uint16_t x1, y1;

	x1 = x0 + width - 1;
	y1 = y0 + height - 1;

	LCD_WR_REG(0x2a);
	LCD_WR_DATA(x0 >> 8);//start x
	LCD_WR_DATA(x0 & 0xff);
	LCD_WR_DATA(x1 >> 8);//end x
	LCD_WR_DATA(x1 & 0xff);

	LCD_WR_REG(0x2b);
	LCD_WR_DATA(y0 >> 8);//start x
	LCD_WR_DATA(y0 & 0xff);
	LCD_WR_DATA(y1 >> 8);//end x
	LCD_WR_DATA(y1 & 0xff);

	LCD_WR_REG(0x2c);//transfor begin
	for(i = 0; i < width * height; i++)
	{
		LCD_WR_DATA(color[i]);
	}
}

static int ST7789V_IOCtrl(uint32_t cmd, uint32_t param)
{
	return 0;
}

static uint16_t ST7789V_GetPixel(uint16_t x, uint16_t y)
{
	uint16_t color_16;
	uint16_t color_r, color_g, color_b;
	int i;

	LCD_WR_REG(0x2a);
	LCD_WR_DATA(x >> 8);//start x
	LCD_WR_DATA(x & 0xff);
	LCD_WR_DATA(x >> 8);//end x
	LCD_WR_DATA(x & 0xff);

	LCD_WR_REG(0x2b);
	LCD_WR_DATA(y >> 8);//start x
	LCD_WR_DATA(y & 0xff);
	LCD_WR_DATA(y >> 8);//end x
	LCD_WR_DATA(y & 0xff);


	LCD_WR_REG(0x2e);//read begin

	//dummy read
	LCD_RD_DATA();

	for(i = 0; i < 0xff; i++);

	//active read
	color_r = LCD_RD_DATA();
	color_g = LCD_RD_DATA();
	color_b = LCD_RD_DATA();

	color_16 = (color_r << 8) + (color_g << 3) + (color_b >> 3);

	return color_16;
}

const lcd_drv_t st7789v_module = {
	ST7789V_CheckID,
	ST7789V_Init,
	ST7789V_SetPixel,
	ST7789V_FillRect,
	ST7789V_DrawBitmap,
	ST7789V_Clear,
	ST7789V_IOCtrl,
	ST7789V_GetPixel
};
