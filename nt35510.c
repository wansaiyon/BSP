/*
 * nt35510.c
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
#include <stdio.h>		//用于调用printf

/*自己添加包含文件*/
#include "delay_us.h"	//用于调用延时函数
#include "font.h"		//用于调用ASCII字符集


static uint32_t Width =	480;
static uint32_t Height = 800;

void NT35510_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue)
{
	LCD_WR_REG(LCD_Reg);
	LCD_WR_DATA(LCD_RegValue);
}

uint16_t NT35510_ReadReg(uint16_t LCD_Reg)
{
	LCD_WR_REG(LCD_Reg);		//写入要读的寄存器序号
	delay_us(5);
	return LCD_RD_DATA();		//返回读到的值
}

//开始写GRAM
void NT35510_WriteRAM_Prepare(void)
{
	LCD_WR_REG(0X2C00);
}

void NT35510_DisplayOn(void)
{
	LCD_WR_REG(0X2900);	//开启显示
}

void NT35510_DisplayOff(void)
{
	LCD_WR_REG(0X2800);	//关闭显示
}

void NT35510_SetCursor(uint16_t x,uint16_t y)
{
	NT35510_WriteReg(0x2A00, x >> 8);
	NT35510_WriteReg(0x2A01, x & 0XFF);
	NT35510_WriteReg(0x2B00, y >> 8);
	NT35510_WriteReg(0x2B01, y & 0XFF);
}

void NT35510_SetWindow(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1)
{
	NT35510_WriteReg(0x2A00, x0 >> 8);
	NT35510_WriteReg(0x2A01, x0 & 0XFF);
	NT35510_WriteReg(0x2A02, x1 >> 8);
	NT35510_WriteReg(0x2A03, x1 & 0XFF);
	NT35510_WriteReg(0x2B00, y0 >> 8);
	NT35510_WriteReg(0x2B01, y0 & 0XFF);
	NT35510_WriteReg(0x2B02, y1 >> 8);
	NT35510_WriteReg(0x2B03, y1 & 0XFF);
}

void NT35510_DrawPixel(uint16_t color, uint16_t x, uint16_t y)
{
	NT35510_SetWindow(x, y, x, y);		//设置光标位置
	NT35510_WriteRAM_Prepare();			//开始写入GRAM
	LCD_WR_DATA(color);
}

void NT35510_Fast_DrawPoint(uint16_t color,uint16_t x,uint16_t y)
{
	NT35510_SetCursor(x,y);				//画点
	NT35510_WriteRAM_Prepare();			//开始写入GRAM
	LCD_WR_DATA(color);
}

//mode:叠加方式:1,非叠加方式：0
//mode=0时，黑色背景，随机颜色的字体,不会覆盖原来的内容
//mode=1时，随机颜色的字体，会覆盖原来的字体
void NT35510_ShowChar(uint16_t color,uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode)
{
	uint8_t temp,t1,t;
    uint16_t y0=y;
    uint8_t csize=(size/8+((size%8)?1:0))*(size/2);		//得到字体一个字符对应的点阵集所占的字节数
 	num=num-' ';//偏移地址
	for(t=0;t<csize;t++)
	{
		if(size==12)temp=asc2_1206[num][t]; 	 	//调用1206字体
		else if(size==16)temp=asc2_1608[num][t];	//调用1608字体
		else if(size==24)temp=asc2_2412[num][t];	//调用2412字体
		else return;
		for(t1=0;t1<8;t1++)
		{
			if(temp&0x80)NT35510_Fast_DrawPoint(color,x,y);			//随机颜色的字体
			else if(mode==0)NT35510_Fast_DrawPoint(BACK_COLOR,x,y);		//黑色背景
			temp<<=1;
			y++;
			if(y>=Height)return;		//超区域了
			if((y-y0)==size)
			{
				y=y0;
				x++;
				if(x>=Width)return;	//超区域了
				break;
			}
		}
	}
}

void NT35510_ShowString(uint16_t color,uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p,uint8_t mode)
{
	uint8_t x0=x;
	width+=x;
	height+=y;
    while((*p<='~')&&(*p>=' '))//判断是不是非法字符
    {
        if(x>=width){x=x0;y+=size;}
        if(y>=height)break;//退出
        NT35510_ShowChar(color,x,y,*p,size,mode);
        x+=size/2;
        p++;
    }
}

uint32_t NT35510_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)result*=m;
	return result;
}

void NT35510_ShowNum(uint16_t color,uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{
	uint8_t t,temp;
	uint8_t enshow=0;
	for(t=0;t<len;t++)
	{
		temp=(num/NT35510_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				NT35510_ShowChar(color,x+(size/2)*t,y,' ',size,mode);
				continue;
			}else enshow=1;
		}
		NT35510_ShowChar(color,x+(size/2)*t,y,temp+'0',size,mode);
	}
}

void NT35510_ShowxNum(uint8_t color,uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode)
{
	uint8_t t,temp;
	uint8_t enshow=0;
	for(t=0;t<len;t++)
	{
		temp=(num/NT35510_Pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				if(mode&0X80)NT35510_ShowChar(color,x+(size/2)*t,y,'0',size,mode&0x01);
				else NT35510_ShowChar(color,x+(size/2)*t,y,' ',size,mode&0x01);
 				continue;
			}else enshow=1;

		}
		NT35510_ShowChar(color,x+(size/2)*t,y,temp+'0',size,mode&0x01);
	}
}


//清屏函数
void NT35510_Clear(uint16_t color)
{
	uint32_t i;

	NT35510_SetWindow(0, 0, Width - 1, Height - 1);
	NT35510_WriteRAM_Prepare();
	for(i = 0; i < Width * Height; i++)
	{
		LCD_WR_DATA(color);
	}
}

uint32_t NT35510_CheckID(void)
{
	uint8_t id1, id2, id3;

	id1 = NT35510_ReadReg(0XDA00);	//should be 0x00
	id2 = NT35510_ReadReg(0XDB00);	//should be 0x80
	id3 = NT35510_ReadReg(0XDC00);	//should be 0x00
 	printf(" Read ID:%x, %x, %x\r\n", id1, id2, id3);

	if((id1 == 0x00) && (id2 == 0x80) && (id3 == 0x00))
		return 0x35510;
	else
		return 0;
}

void NT35510_Init(void)
{
 	delay_ms(50);
 	NT35510_WriteReg(0x0000,0x0001);
	delay_ms(50);

	NT35510_WriteReg(0xF000,0x55);
	NT35510_WriteReg(0xF001,0xAA);
	NT35510_WriteReg(0xF002,0x52);
	NT35510_WriteReg(0xF003,0x08);
	NT35510_WriteReg(0xF004,0x01);

	//AVDD Set AVDD 5.2V
	NT35510_WriteReg(0xB000,0x0D);
	NT35510_WriteReg(0xB001,0x0D);
	NT35510_WriteReg(0xB002,0x0D);

	//AVDD ratio
	NT35510_WriteReg(0xB600,0x34);
	NT35510_WriteReg(0xB601,0x34);
	NT35510_WriteReg(0xB602,0x34);

	//AVEE -5.2V
	NT35510_WriteReg(0xB100,0x0D);
	NT35510_WriteReg(0xB101,0x0D);
	NT35510_WriteReg(0xB102,0x0D);

	//AVEE ratio
	NT35510_WriteReg(0xB700,0x34);
	NT35510_WriteReg(0xB701,0x34);
	NT35510_WriteReg(0xB702,0x34);

	//VCL -2.5V
	NT35510_WriteReg(0xB200,0x00);
	NT35510_WriteReg(0xB201,0x00);
	NT35510_WriteReg(0xB202,0x00);

	//VCL ratio
	NT35510_WriteReg(0xB800,0x24);
	NT35510_WriteReg(0xB801,0x24);
	NT35510_WriteReg(0xB802,0x24);

	//VGH 15V (Free pump)
	NT35510_WriteReg(0xBF00,0x01);
	NT35510_WriteReg(0xB300,0x0F);
	NT35510_WriteReg(0xB301,0x0F);
	NT35510_WriteReg(0xB302,0x0F);

	//VGH ratio
	NT35510_WriteReg(0xB900,0x34);
	NT35510_WriteReg(0xB901,0x34);
	NT35510_WriteReg(0xB902,0x34);

	//VGL_REG -10V
	NT35510_WriteReg(0xB500,0x08);
	NT35510_WriteReg(0xB501,0x08);
	NT35510_WriteReg(0xB502,0x08);
	NT35510_WriteReg(0xC200,0x03);

	//VGLX ratio
	NT35510_WriteReg(0xBA00,0x24);
	NT35510_WriteReg(0xBA01,0x24);
	NT35510_WriteReg(0xBA02,0x24);

	//VGMP/VGSP 4.5V/0V
	NT35510_WriteReg(0xBC00,0x00);
	NT35510_WriteReg(0xBC01,0x78);
	NT35510_WriteReg(0xBC02,0x00);

	//VGMN/VGSN -4.5V/0V
	NT35510_WriteReg(0xBD00,0x00);
	NT35510_WriteReg(0xBD01,0x78);
	NT35510_WriteReg(0xBD02,0x00);

	//VCOM
	NT35510_WriteReg(0xBE00,0x00);
	NT35510_WriteReg(0xBE01,0x64);

	//Gamma Setting
	NT35510_WriteReg(0xD100,0x00);
	NT35510_WriteReg(0xD101,0x33);
	NT35510_WriteReg(0xD102,0x00);
	NT35510_WriteReg(0xD103,0x34);
	NT35510_WriteReg(0xD104,0x00);
	NT35510_WriteReg(0xD105,0x3A);
	NT35510_WriteReg(0xD106,0x00);
	NT35510_WriteReg(0xD107,0x4A);
	NT35510_WriteReg(0xD108,0x00);
	NT35510_WriteReg(0xD109,0x5C);
	NT35510_WriteReg(0xD10A,0x00);
	NT35510_WriteReg(0xD10B,0x81);
	NT35510_WriteReg(0xD10C,0x00);
	NT35510_WriteReg(0xD10D,0xA6);
	NT35510_WriteReg(0xD10E,0x00);
	NT35510_WriteReg(0xD10F,0xE5);
	NT35510_WriteReg(0xD110,0x01);
	NT35510_WriteReg(0xD111,0x13);
	NT35510_WriteReg(0xD112,0x01);
	NT35510_WriteReg(0xD113,0x54);
	NT35510_WriteReg(0xD114,0x01);
	NT35510_WriteReg(0xD115,0x82);
	NT35510_WriteReg(0xD116,0x01);
	NT35510_WriteReg(0xD117,0xCA);
	NT35510_WriteReg(0xD118,0x02);
	NT35510_WriteReg(0xD119,0x00);
	NT35510_WriteReg(0xD11A,0x02);
	NT35510_WriteReg(0xD11B,0x01);
	NT35510_WriteReg(0xD11C,0x02);
	NT35510_WriteReg(0xD11D,0x34);
	NT35510_WriteReg(0xD11E,0x02);
	NT35510_WriteReg(0xD11F,0x67);
	NT35510_WriteReg(0xD120,0x02);
	NT35510_WriteReg(0xD121,0x84);
	NT35510_WriteReg(0xD122,0x02);
	NT35510_WriteReg(0xD123,0xA4);
	NT35510_WriteReg(0xD124,0x02);
	NT35510_WriteReg(0xD125,0xB7);
	NT35510_WriteReg(0xD126,0x02);
	NT35510_WriteReg(0xD127,0xCF);
	NT35510_WriteReg(0xD128,0x02);
	NT35510_WriteReg(0xD129,0xDE);
	NT35510_WriteReg(0xD12A,0x02);
	NT35510_WriteReg(0xD12B,0xF2);
	NT35510_WriteReg(0xD12C,0x02);
	NT35510_WriteReg(0xD12D,0xFE);
	NT35510_WriteReg(0xD12E,0x03);
	NT35510_WriteReg(0xD12F,0x10);
	NT35510_WriteReg(0xD130,0x03);
	NT35510_WriteReg(0xD131,0x33);
	NT35510_WriteReg(0xD132,0x03);
	NT35510_WriteReg(0xD133,0x6D);
	NT35510_WriteReg(0xD200,0x00);
	NT35510_WriteReg(0xD201,0x33);
	NT35510_WriteReg(0xD202,0x00);
	NT35510_WriteReg(0xD203,0x34);
	NT35510_WriteReg(0xD204,0x00);
	NT35510_WriteReg(0xD205,0x3A);
	NT35510_WriteReg(0xD206,0x00);
	NT35510_WriteReg(0xD207,0x4A);
	NT35510_WriteReg(0xD208,0x00);
	NT35510_WriteReg(0xD209,0x5C);
	NT35510_WriteReg(0xD20A,0x00);

	NT35510_WriteReg(0xD20B,0x81);
	NT35510_WriteReg(0xD20C,0x00);
	NT35510_WriteReg(0xD20D,0xA6);
	NT35510_WriteReg(0xD20E,0x00);
	NT35510_WriteReg(0xD20F,0xE5);
	NT35510_WriteReg(0xD210,0x01);
	NT35510_WriteReg(0xD211,0x13);
	NT35510_WriteReg(0xD212,0x01);
	NT35510_WriteReg(0xD213,0x54);
	NT35510_WriteReg(0xD214,0x01);
	NT35510_WriteReg(0xD215,0x82);
	NT35510_WriteReg(0xD216,0x01);
	NT35510_WriteReg(0xD217,0xCA);
	NT35510_WriteReg(0xD218,0x02);
	NT35510_WriteReg(0xD219,0x00);
	NT35510_WriteReg(0xD21A,0x02);
	NT35510_WriteReg(0xD21B,0x01);
	NT35510_WriteReg(0xD21C,0x02);
	NT35510_WriteReg(0xD21D,0x34);
	NT35510_WriteReg(0xD21E,0x02);
	NT35510_WriteReg(0xD21F,0x67);
	NT35510_WriteReg(0xD220,0x02);
	NT35510_WriteReg(0xD221,0x84);
	NT35510_WriteReg(0xD222,0x02);
	NT35510_WriteReg(0xD223,0xA4);
	NT35510_WriteReg(0xD224,0x02);
	NT35510_WriteReg(0xD225,0xB7);
	NT35510_WriteReg(0xD226,0x02);
	NT35510_WriteReg(0xD227,0xCF);
	NT35510_WriteReg(0xD228,0x02);
	NT35510_WriteReg(0xD229,0xDE);
	NT35510_WriteReg(0xD22A,0x02);
	NT35510_WriteReg(0xD22B,0xF2);
	NT35510_WriteReg(0xD22C,0x02);
	NT35510_WriteReg(0xD22D,0xFE);
	NT35510_WriteReg(0xD22E,0x03);
	NT35510_WriteReg(0xD22F,0x10);
	NT35510_WriteReg(0xD230,0x03);
	NT35510_WriteReg(0xD231,0x33);
	NT35510_WriteReg(0xD232,0x03);
	NT35510_WriteReg(0xD233,0x6D);
	NT35510_WriteReg(0xD300,0x00);
	NT35510_WriteReg(0xD301,0x33);
	NT35510_WriteReg(0xD302,0x00);
	NT35510_WriteReg(0xD303,0x34);
	NT35510_WriteReg(0xD304,0x00);
	NT35510_WriteReg(0xD305,0x3A);
	NT35510_WriteReg(0xD306,0x00);
	NT35510_WriteReg(0xD307,0x4A);
	NT35510_WriteReg(0xD308,0x00);
	NT35510_WriteReg(0xD309,0x5C);
	NT35510_WriteReg(0xD30A,0x00);

	NT35510_WriteReg(0xD30B,0x81);
	NT35510_WriteReg(0xD30C,0x00);
	NT35510_WriteReg(0xD30D,0xA6);
	NT35510_WriteReg(0xD30E,0x00);
	NT35510_WriteReg(0xD30F,0xE5);
	NT35510_WriteReg(0xD310,0x01);
	NT35510_WriteReg(0xD311,0x13);
	NT35510_WriteReg(0xD312,0x01);
	NT35510_WriteReg(0xD313,0x54);
	NT35510_WriteReg(0xD314,0x01);
	NT35510_WriteReg(0xD315,0x82);
	NT35510_WriteReg(0xD316,0x01);
	NT35510_WriteReg(0xD317,0xCA);
	NT35510_WriteReg(0xD318,0x02);
	NT35510_WriteReg(0xD319,0x00);
	NT35510_WriteReg(0xD31A,0x02);
	NT35510_WriteReg(0xD31B,0x01);
	NT35510_WriteReg(0xD31C,0x02);
	NT35510_WriteReg(0xD31D,0x34);
	NT35510_WriteReg(0xD31E,0x02);
	NT35510_WriteReg(0xD31F,0x67);
	NT35510_WriteReg(0xD320,0x02);
	NT35510_WriteReg(0xD321,0x84);
	NT35510_WriteReg(0xD322,0x02);
	NT35510_WriteReg(0xD323,0xA4);
	NT35510_WriteReg(0xD324,0x02);
	NT35510_WriteReg(0xD325,0xB7);
	NT35510_WriteReg(0xD326,0x02);
	NT35510_WriteReg(0xD327,0xCF);
	NT35510_WriteReg(0xD328,0x02);
	NT35510_WriteReg(0xD329,0xDE);
	NT35510_WriteReg(0xD32A,0x02);
	NT35510_WriteReg(0xD32B,0xF2);
	NT35510_WriteReg(0xD32C,0x02);
	NT35510_WriteReg(0xD32D,0xFE);
	NT35510_WriteReg(0xD32E,0x03);
	NT35510_WriteReg(0xD32F,0x10);
	NT35510_WriteReg(0xD330,0x03);
	NT35510_WriteReg(0xD331,0x33);
	NT35510_WriteReg(0xD332,0x03);
	NT35510_WriteReg(0xD333,0x6D);
	NT35510_WriteReg(0xD400,0x00);
	NT35510_WriteReg(0xD401,0x33);
	NT35510_WriteReg(0xD402,0x00);
	NT35510_WriteReg(0xD403,0x34);
	NT35510_WriteReg(0xD404,0x00);
	NT35510_WriteReg(0xD405,0x3A);
	NT35510_WriteReg(0xD406,0x00);
	NT35510_WriteReg(0xD407,0x4A);
	NT35510_WriteReg(0xD408,0x00);
	NT35510_WriteReg(0xD409,0x5C);
	NT35510_WriteReg(0xD40A,0x00);
	NT35510_WriteReg(0xD40B,0x81);

	NT35510_WriteReg(0xD40C,0x00);
	NT35510_WriteReg(0xD40D,0xA6);
	NT35510_WriteReg(0xD40E,0x00);
	NT35510_WriteReg(0xD40F,0xE5);
	NT35510_WriteReg(0xD410,0x01);
	NT35510_WriteReg(0xD411,0x13);
	NT35510_WriteReg(0xD412,0x01);
	NT35510_WriteReg(0xD413,0x54);
	NT35510_WriteReg(0xD414,0x01);
	NT35510_WriteReg(0xD415,0x82);
	NT35510_WriteReg(0xD416,0x01);
	NT35510_WriteReg(0xD417,0xCA);
	NT35510_WriteReg(0xD418,0x02);
	NT35510_WriteReg(0xD419,0x00);
	NT35510_WriteReg(0xD41A,0x02);
	NT35510_WriteReg(0xD41B,0x01);
	NT35510_WriteReg(0xD41C,0x02);
	NT35510_WriteReg(0xD41D,0x34);
	NT35510_WriteReg(0xD41E,0x02);
	NT35510_WriteReg(0xD41F,0x67);
	NT35510_WriteReg(0xD420,0x02);
	NT35510_WriteReg(0xD421,0x84);
	NT35510_WriteReg(0xD422,0x02);
	NT35510_WriteReg(0xD423,0xA4);
	NT35510_WriteReg(0xD424,0x02);
	NT35510_WriteReg(0xD425,0xB7);
	NT35510_WriteReg(0xD426,0x02);
	NT35510_WriteReg(0xD427,0xCF);
	NT35510_WriteReg(0xD428,0x02);
	NT35510_WriteReg(0xD429,0xDE);
	NT35510_WriteReg(0xD42A,0x02);
	NT35510_WriteReg(0xD42B,0xF2);
	NT35510_WriteReg(0xD42C,0x02);
	NT35510_WriteReg(0xD42D,0xFE);
	NT35510_WriteReg(0xD42E,0x03);
	NT35510_WriteReg(0xD42F,0x10);
	NT35510_WriteReg(0xD430,0x03);
	NT35510_WriteReg(0xD431,0x33);
	NT35510_WriteReg(0xD432,0x03);
	NT35510_WriteReg(0xD433,0x6D);
	NT35510_WriteReg(0xD500,0x00);
	NT35510_WriteReg(0xD501,0x33);
	NT35510_WriteReg(0xD502,0x00);
	NT35510_WriteReg(0xD503,0x34);
	NT35510_WriteReg(0xD504,0x00);
	NT35510_WriteReg(0xD505,0x3A);
	NT35510_WriteReg(0xD506,0x00);
	NT35510_WriteReg(0xD507,0x4A);
	NT35510_WriteReg(0xD508,0x00);
	NT35510_WriteReg(0xD509,0x5C);
	NT35510_WriteReg(0xD50A,0x00);
	NT35510_WriteReg(0xD50B,0x81);

	NT35510_WriteReg(0xD50C,0x00);
	NT35510_WriteReg(0xD50D,0xA6);
	NT35510_WriteReg(0xD50E,0x00);
	NT35510_WriteReg(0xD50F,0xE5);
	NT35510_WriteReg(0xD510,0x01);
	NT35510_WriteReg(0xD511,0x13);
	NT35510_WriteReg(0xD512,0x01);
	NT35510_WriteReg(0xD513,0x54);
	NT35510_WriteReg(0xD514,0x01);
	NT35510_WriteReg(0xD515,0x82);
	NT35510_WriteReg(0xD516,0x01);
	NT35510_WriteReg(0xD517,0xCA);
	NT35510_WriteReg(0xD518,0x02);
	NT35510_WriteReg(0xD519,0x00);
	NT35510_WriteReg(0xD51A,0x02);
	NT35510_WriteReg(0xD51B,0x01);
	NT35510_WriteReg(0xD51C,0x02);
	NT35510_WriteReg(0xD51D,0x34);
	NT35510_WriteReg(0xD51E,0x02);
	NT35510_WriteReg(0xD51F,0x67);
	NT35510_WriteReg(0xD520,0x02);
	NT35510_WriteReg(0xD521,0x84);
	NT35510_WriteReg(0xD522,0x02);
	NT35510_WriteReg(0xD523,0xA4);
	NT35510_WriteReg(0xD524,0x02);
	NT35510_WriteReg(0xD525,0xB7);
	NT35510_WriteReg(0xD526,0x02);
	NT35510_WriteReg(0xD527,0xCF);
	NT35510_WriteReg(0xD528,0x02);
	NT35510_WriteReg(0xD529,0xDE);
	NT35510_WriteReg(0xD52A,0x02);
	NT35510_WriteReg(0xD52B,0xF2);
	NT35510_WriteReg(0xD52C,0x02);
	NT35510_WriteReg(0xD52D,0xFE);
	NT35510_WriteReg(0xD52E,0x03);
	NT35510_WriteReg(0xD52F,0x10);
	NT35510_WriteReg(0xD530,0x03);
	NT35510_WriteReg(0xD531,0x33);
	NT35510_WriteReg(0xD532,0x03);
	NT35510_WriteReg(0xD533,0x6D);
	NT35510_WriteReg(0xD600,0x00);
	NT35510_WriteReg(0xD601,0x33);
	NT35510_WriteReg(0xD602,0x00);
	NT35510_WriteReg(0xD603,0x34);
	NT35510_WriteReg(0xD604,0x00);
	NT35510_WriteReg(0xD605,0x3A);
	NT35510_WriteReg(0xD606,0x00);
	NT35510_WriteReg(0xD607,0x4A);
	NT35510_WriteReg(0xD608,0x00);
	NT35510_WriteReg(0xD609,0x5C);
	NT35510_WriteReg(0xD60A,0x00);
	NT35510_WriteReg(0xD60B,0x81);

	NT35510_WriteReg(0xD60C,0x00);
	NT35510_WriteReg(0xD60D,0xA6);
	NT35510_WriteReg(0xD60E,0x00);
	NT35510_WriteReg(0xD60F,0xE5);
	NT35510_WriteReg(0xD610,0x01);
	NT35510_WriteReg(0xD611,0x13);
	NT35510_WriteReg(0xD612,0x01);
	NT35510_WriteReg(0xD613,0x54);
	NT35510_WriteReg(0xD614,0x01);
	NT35510_WriteReg(0xD615,0x82);
	NT35510_WriteReg(0xD616,0x01);
	NT35510_WriteReg(0xD617,0xCA);
	NT35510_WriteReg(0xD618,0x02);
	NT35510_WriteReg(0xD619,0x00);
	NT35510_WriteReg(0xD61A,0x02);
	NT35510_WriteReg(0xD61B,0x01);
	NT35510_WriteReg(0xD61C,0x02);
	NT35510_WriteReg(0xD61D,0x34);
	NT35510_WriteReg(0xD61E,0x02);
	NT35510_WriteReg(0xD61F,0x67);
	NT35510_WriteReg(0xD620,0x02);
	NT35510_WriteReg(0xD621,0x84);
	NT35510_WriteReg(0xD622,0x02);
	NT35510_WriteReg(0xD623,0xA4);
	NT35510_WriteReg(0xD624,0x02);
	NT35510_WriteReg(0xD625,0xB7);
	NT35510_WriteReg(0xD626,0x02);
	NT35510_WriteReg(0xD627,0xCF);
	NT35510_WriteReg(0xD628,0x02);
	NT35510_WriteReg(0xD629,0xDE);
	NT35510_WriteReg(0xD62A,0x02);
	NT35510_WriteReg(0xD62B,0xF2);
	NT35510_WriteReg(0xD62C,0x02);
	NT35510_WriteReg(0xD62D,0xFE);
	NT35510_WriteReg(0xD62E,0x03);
	NT35510_WriteReg(0xD62F,0x10);
	NT35510_WriteReg(0xD630,0x03);
	NT35510_WriteReg(0xD631,0x33);
	NT35510_WriteReg(0xD632,0x03);
	NT35510_WriteReg(0xD633,0x6D);

	//LV2 Page 0 enable
	NT35510_WriteReg(0xF000,0x55);
	NT35510_WriteReg(0xF001,0xAA);
	NT35510_WriteReg(0xF002,0x52);
	NT35510_WriteReg(0xF003,0x08);
	NT35510_WriteReg(0xF004,0x00);

	//Display control
	NT35510_WriteReg(0xB100, 0xCC);
	NT35510_WriteReg(0xB101, 0x00);

	//Source hold time
	NT35510_WriteReg(0xB600,0x05);
	//Gate EQ control
	NT35510_WriteReg(0xB700,0x70);
	NT35510_WriteReg(0xB701,0x70);

	//Source EQ control (Mode 2)
	NT35510_WriteReg(0xB800,0x01);
	NT35510_WriteReg(0xB801,0x03);
	NT35510_WriteReg(0xB802,0x03);
	NT35510_WriteReg(0xB803,0x03);

	//Inversion mode (2-dot)
	NT35510_WriteReg(0xBC00,0x02);
	NT35510_WriteReg(0xBC01,0x00);
	NT35510_WriteReg(0xBC02,0x00);

	//Timing control 4H w/ 4-delay
	NT35510_WriteReg(0xC900,0xD0);
	NT35510_WriteReg(0xC901,0x02);
	NT35510_WriteReg(0xC902,0x50);
	NT35510_WriteReg(0xC903,0x50);
	NT35510_WriteReg(0xC904,0x50);

	NT35510_WriteReg(0x3500,0x00);
	NT35510_WriteReg(0x3A00,0x55);  //16-bit/pixel
	NT35510_WriteReg(0x3600,0x00);		//默认为竖屏

	LCD_WR_REG(0x1100);
	delay_us(120);
	LCD_WR_REG(0x2900);

	NT35510_Clear(BLACK);
}

void NT35510_FillRect(uint16_t color, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
	uint32_t i;
	uint16_t width, height;

	width = x1-x0+1;
	height = y1-y0+1;

	NT35510_SetWindow(x0, y0, x1, y1);
	NT35510_WriteRAM_Prepare();
	for(i = 0; i < width * height; i++)
	{
		LCD_WR_DATA(color);
	}
}

void NT35510_Draw_Circle(uint16_t color,uint16_t x0,uint16_t y0,uint16_t r)
{
	int a,b;
	int di;
	a=0;b=r;
	di=3-(r<<1);             //判断下个点位置的标志
	while(a<=b)
	{
		NT35510_Fast_DrawPoint(color,x0+a,y0-b);             //5
		NT35510_Fast_DrawPoint(color,x0+b,y0-a);             //0
		NT35510_Fast_DrawPoint(color,x0+b,y0+a);             //4
		NT35510_Fast_DrawPoint(color,x0+a,y0+b);             //6
		NT35510_Fast_DrawPoint(color,x0-a,y0+b);             //1
		NT35510_Fast_DrawPoint(color,x0-b,y0+a);
		NT35510_Fast_DrawPoint(color,x0-a,y0-b);             //2
		NT35510_Fast_DrawPoint(color,x0-b,y0-a);             //7
		a++;
		//使用Bresenham算法画圆
		if(di<0)di +=4*a+6;
		else
		{
			di+=10+4*(a-b);
			b--;
		}
	}
}

//位图显示函数，位图中每个像素大小为16bit，高字节在前低字节在后
void NT35510_DrawBitmap(uint16_t color[], uint16_t x0, uint16_t y0, uint16_t width, uint16_t height)
{
	uint32_t i;
	uint16_t x1, y1;

	x1 = x0 + width - 1;
	y1 = y0 + height - 1;

	NT35510_SetWindow(x0, y0, x1, y1);
	NT35510_WriteRAM_Prepare();
	for(i = 0; i < width * height; i++)
	{
		LCD_WR_DATA(color[i]);
	}
}

int NT35510_IOCtrl(uint32_t cmd, uint32_t param)
{
    switch(cmd)
    {
    case LCD_CMD_SLEEP_IN:
        break;

    case LCD_CMD_SLEEP_OUT:
        break;

    case LCD_CMD_SET_DIR:
        break;

    default:
        break;
    }
	return 0;
}

const lcd_drv_t nt35510_module = {
	NT35510_CheckID,
	NT35510_Init,
	NT35510_DrawPixel,
	NT35510_FillRect,
	NT35510_DrawBitmap,
	NT35510_Clear,
	NT35510_IOCtrl
};
