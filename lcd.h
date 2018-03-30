/*
 * lcd.h
 *
 *  Created on: 2017年8月19日
 *      Author: wansaiyon
 *
 *     http://blog.csdn.net/hexiaolong2009
 *     正点原子NT35510 LCD模块的驱动接口，参照正点原子NT35510修改而来
 *     LCD模块：480*800电容屏
 */
#ifndef _LCD_H_
#define _LCD_H_

/*自己添加包含文件*/
#include "stm32f4xx.h"	//包含uint32_t等

/*宏定义--------------------------------------------------------------------*/
/******************************************************************************
2^26 =0X0400 0000 = 64MB,每个BANK有4*64MB = 256MB
64MB:FSMC_Bank1_NORSRAM1:0X6000 0000 ~ 0X63FF FFFF
64MB:FSMC_Bank1_NORSRAM2:0X6400 0000 ~ 0X67FF FFFF
64MB:FSMC_Bank1_NORSRAM3:0X6800 0000 ~ 0X6BFF FFFF
64MB:FSMC_Bank1_NORSRAM4:0X6C00 0000 ~ 0X6FFF FFFF

选择BANK1-BORSRAM4连接TFT,地址范围为0X6C00 0000 ~ 0X6FFF FFFF
探索者F407开发板选择FSMC-A6连接LCD的DC(寄存器/数据选择)脚
寄存器基地址 = 0X6C00 0000
RAM基地址        = 0X6C00 0080 = 0X6C00 0000+(1<<(6+1))
如果电路设计时选择不同的地址线时,地址要重新计算的
*******************************************************************************/
/******************************* NT35510显示屏的参数定义 ***************/
#define CMD_ADDR    0x6C000000
#define DATA_ADDR   0x6C000080

//画笔颜色
#define WHITE  		0xFFFF
#define BLACK       0x0000
#define BLUE        0x001F
#define BRED        0XF81F
#define GRED 		0XFFE0
#define GBLUE		0X07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define GREEN       0x07E0
#define CYAN        0x7FFF
#define YELLOW      0xFFE0
#define BROWN 		0XBC40 //棕色
#define BRRED 		0XFC07 //棕红色
#define GRAY  		0X8430 //灰色

#define BACK_COLOR	0x7FFF
//typedef enum
//{
//	WHITE	=	 0xFFFF,
//	BLACK   =    0x0000,
//	BLUE    =    0x001F,
//	BRED    =    0XF81F,
//	GRED 	=	 0XFFE0,
//	GBLUE	=	 0X07FF,
//	RED     =    0xF800,
//	MAGENTA =    0xF81F,
//	GREEN   =    0x07E0,
//	CYAN    =    0x7FFF,
//	YELLOW  =    0xFFE0,
//	BROWN 	=	 0XBC40, //棕色
//	BRRED 	=	 0XFC07, //棕红色
//	GRAY  	=	 0X8430 //灰色
//}Color_Typedef;


#define  LCD_WR_REG(cmd)   (*(volatile uint16_t *)(CMD_ADDR)) = (cmd)
#define  LCD_WR_DATA(val)   (*(volatile uint16_t *)(DATA_ADDR)) = (val)
#define  LCD_RD_DATA()   (*(volatile uint16_t *)(DATA_ADDR))


typedef enum
{
	LCD_CMD_SLEEP_IN = 0,
	LCD_CMD_SLEEP_OUT,
	LCD_CMD_SET_DIR,
}LCD_CMD;


typedef struct _LCD_DRV{
	uint32_t (*checkid)(void);
	void (*init)(void);
	void (*setpixel)(uint16_t color, uint16_t x, uint16_t y);
	void (*fillrect)(uint16_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
	void (*bitmap)(uint16_t color[], uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
	void (*clear)(uint16_t color);
	int (*ioctrl)(uint32_t cmd, uint32_t param);
	uint16_t (*getpixel)(uint16_t x, uint16_t y);
}lcd_drv_t;


extern const lcd_drv_t *lcd_drv;
extern const lcd_drv_t nt35510_module;
extern const lcd_drv_t st7789v_module;

void LCD_GPIO_Cfg(void);
void LCD_FMC_Cfg(void);
void LCD_Reset(void);
uint8_t LCD_CheckID(void);
void LCD_Init(void);
void LCD_DrawPixel(uint16_t color, uint16_t x, uint16_t y);
void LCD_FillRect(uint16_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
void LCD_DrawBitmap(uint16_t color[], uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
void LCD_Clear(uint16_t color);

#endif /* DRIVERS_BSP_LCD_LCD_H_ */
