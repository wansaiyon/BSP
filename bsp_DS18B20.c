/**
  ******************************************************************************
  * �ļ�����: bsp_DS18B20.c 
  * ��    ��: ӲʯǶ��ʽ�����Ŷ�
  * ��    ��: V1.0
  * ��д����: 2015-10-04
  * ��    ��: DS18B20�¶ȴ������ײ���������
  ******************************************************************************
  * ˵����
  * ����������Ӳʯstm32������YS-F1Proʹ�á�
  * 
  * �Ա���
  * ��̳��http://www.ing10bbs.com
  * ��Ȩ��ӲʯǶ��ʽ�����Ŷ����У��������á�
  ******************************************************************************
  */
/* ����ͷ�ļ� ----------------------------------------------------------------*/
#include "bsp_DS18B20.h"

/* ˽�����Ͷ��� --------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
#define Delay_ms(x)   HAL_Delay(x)
/* ˽�б��� ------------------------------------------------------------------*/
/* ��չ���� ------------------------------------------------------------------*/
/* ˽�к���ԭ�� --------------------------------------------------------------*/
static void DS18B20_Mode_IPU(void);
static void DS18B20_Mode_Out_PP(void);
static void DS18B20_Rst(void);
static uint8_t DS18B20_Presence(void);
static uint8_t DS18B20_ReadBit(void);
static uint8_t DS18B20_ReadByte(void);
static void DS18B20_WriteByte(uint8_t dat);
static void DS18B20_SkipRom(void);
static void DS18B20_MatchRom(void);

/* ������ --------------------------------------------------------------------*/

static uint8_t  fac_us=0;           //��ʱ΢���Ƶ��
static uint16_t fac_ms=0;           //��ʱ�����Ƶ��

void delay_init()
{
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK_DIV8); //ѡ��ʱ��Դ-�ⲿʱ��-HCLK/8
    fac_us=SystemCoreClock/8000000; // 72/8 ��ʱ1΢��9��ʱ������
    fac_ms=(uint16_t)fac_us*1000;   // ��ʱ1����9000��Cysticʱ������
}

/**
 * nus : ��ʱ����΢��
 **/
static void DS18B20_Delay(uint32_t nus)
{
	uint32_t temp;
    //nus*fac_usֵ����ܳ���SysTick->LOAD(24λ)-1
    SysTick->LOAD=nus*fac_us;    // ��������ֵ:n(us)*��ʱ1us��Ҫ���ٸ�SysTickʱ������
    SysTick->VAL=0x00;                       // VAL��ʼ��Ϊ0
    SysTick->CTRL|=SysTick_CTRL_ENABLE_Msk ; // ʹ��SysTick��ʱ��
    do
    {
        temp=SysTick->CTRL;
    }while((temp&0x01)&&!(temp&(1<<16)));    // �ȴ�����ʱ�䵽��(λ16)
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk; // �ر�ʹ��
    SysTick->VAL =0X00;                      // ����VAL
}

/**
 * nms : ��ʱ���ٺ���
 * @ע������ ����Ҫ����500ms���ٽ�ֵδʵ�ʲ����������׼
 **/
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
  * ��������: DS18B20 ��ʼ������
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
uint8_t DS18B20_Init(void)
{
  DS18B20_Dout_GPIO_CLK_ENABLE();
  
  DS18B20_Mode_Out_PP();
	
	DS18B20_Dout_HIGH();
	
	DS18B20_Rst();
  
  return DS18B20_Presence ();
}


/**
  * ��������: ʹDS18B20-DATA���ű�Ϊ��������ģʽ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void DS18B20_Mode_IPU(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* �������蹦��GPIO���� */
  GPIO_InitStruct.Pin   = DS18B20_Dout_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull  = GPIO_PULLUP;
  HAL_GPIO_Init(DS18B20_Dout_PORT, &GPIO_InitStruct);
	
}

/**
  * ��������: ʹDS18B20-DATA���ű�Ϊ�������ģʽ
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void DS18B20_Mode_Out_PP(void)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  /* �������蹦��GPIO���� */
  GPIO_InitStruct.Pin = DS18B20_Dout_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(DS18B20_Dout_PORT, &GPIO_InitStruct); 	 
}

/**
  * ��������: �������ӻ����͸�λ����
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void DS18B20_Rst(void)
{
	/* ��������Ϊ������� */
	DS18B20_Mode_Out_PP();
	
	DS18B20_Dout_LOW();
  
	/* �������ٲ���480us�ĵ͵�ƽ��λ�ź� */
	DS18B20_Delay(750);
	
	/* �����ڲ�����λ�źź��轫�������� */
	DS18B20_Dout_HIGH();
	
	/*�ӻ����յ������ĸ�λ�źź󣬻���15~60us���������һ����������*/
	DS18B20_Delay(15);
}

/**
  * ��������: ���ӻ����������صĴ�������
  * �������: ��
  * �� �� ֵ: 0���ɹ���1��ʧ��
  * ˵    ������
  */
static uint8_t DS18B20_Presence(void)
{
	uint8_t pulse_time = 0;
	
	/* ��������Ϊ�������� */
	DS18B20_Mode_IPU();
	
	/* �ȴ���������ĵ�������������Ϊһ��60~240us�ĵ͵�ƽ�ź� 
	 * �����������û����������ʱ�������ӻ����յ������ĸ�λ�źź󣬻���15~60us���������һ����������
	 */
	while( DS18B20_Data_IN() && pulse_time<100 )
	{
		pulse_time++;
		DS18B20_Delay(1);
	}	
	/* ����100us�󣬴������嶼��û�е���*/
	if( pulse_time >=100 )
		return 1;
	else
		pulse_time = 0;
	
	/* �������嵽�����Ҵ��ڵ�ʱ�䲻�ܳ���240us */
	while( !DS18B20_Data_IN() && pulse_time<240 )
	{
		pulse_time++;
		DS18B20_Delay(1);
	}	
	if( pulse_time >=240 )
		return 1;
	else
		return 0;
}

/**
  * ��������: ��DS18B20��ȡһ��bit
  * �������: ��
  * �� �� ֵ: ��ȡ��������
  * ˵    ������
  */
static uint8_t DS18B20_ReadBit(void)
{
	uint8_t dat;
	
	/* ��0�Ͷ�1��ʱ������Ҫ����60us */	
	DS18B20_Mode_Out_PP();
	/* ��ʱ�����ʼ���������������� >1us <15us �ĵ͵�ƽ�ź� */
	DS18B20_Dout_LOW();
	DS18B20_Delay(10);
	
	/* ���ó����룬�ͷ����ߣ����ⲿ�������轫�������� */
	DS18B20_Mode_IPU();
	//Delay_us(2);
	
	if( DS18B20_Data_IN() == GPIO_PIN_SET )
		dat = 1;
	else
		dat = 0;
	
	/* �����ʱ������ο�ʱ��ͼ */
	DS18B20_Delay(45);
	
	return dat;
}

/**
  * ��������: ��DS18B20��һ���ֽڣ���λ����
  * �������: ��
  * �� �� ֵ: ����������
  * ˵    ������
  */
static uint8_t DS18B20_ReadByte(void)
{
	uint8_t i, j, dat = 0;	
	
	for(i=0; i<8; i++) 
	{
		j = DS18B20_ReadBit();		
		dat = (dat) | (j<<i);
	}
	
	return dat;
}

/**
  * ��������: дһ���ֽڵ�DS18B20����λ����
  * �������: dat����д������
  * �� �� ֵ: ��
  * ˵    ������
  */
static void DS18B20_WriteByte(uint8_t dat)
{
	uint8_t i, testb;
	DS18B20_Mode_Out_PP();
	
	for( i=0; i<8; i++ )
	{
		testb = dat&0x01;
		dat = dat>>1;		
		/* д0��д1��ʱ������Ҫ����60us */
		if (testb)
		{			
			DS18B20_Dout_LOW();
			/* 1us < �����ʱ < 15us */
			DS18B20_Delay(8);
			
			DS18B20_Dout_HIGH();
			DS18B20_Delay(58);
		}		
		else
		{			
			DS18B20_Dout_LOW();
			/* 60us < Tx 0 < 120us */
			DS18B20_Delay(70);
			
			DS18B20_Dout_HIGH();		
			/* 1us < Trec(�ָ�ʱ��) < �����*/
			DS18B20_Delay(2);
		}
	}
}

/**
  * ��������: ����ƥ�� DS18B20 ROM
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void DS18B20_SkipRom ( void )
{
	DS18B20_Rst();	   	
	DS18B20_Presence();	 	
	DS18B20_WriteByte(0XCC);		/* ���� ROM */	
}

/**
  * ��������: ִ��ƥ�� DS18B20 ROM
  * �������: ��
  * �� �� ֵ: ��
  * ˵    ������
  */
static void DS18B20_MatchRom ( void )
{
	DS18B20_Rst();	   	
	DS18B20_Presence();	 	
	DS18B20_WriteByte(0X55);		/* ƥ�� ROM */	
}


/*
 * �洢���¶���16 λ�Ĵ�������չ�Ķ����Ʋ�����ʽ
 * ��������12λ�ֱ���ʱ������5������λ��7������λ��4��С��λ
 *
 *         |---------����----------|-----С�� �ֱ��� 1/(2^4)=0.0625----|
 * ���ֽ�  | 2^3 | 2^2 | 2^1 | 2^0 | 2^(-1) | 2^(-2) | 2^(-3) | 2^(-4) |
 *
 *
 *         |-----����λ��0->��  1->��-------|-----------����-----------|
 * ���ֽ�  |  s  |  s  |  s  |  s  |    s   |   2^6  |   2^5  |   2^4  |
 *
 * 
 * �¶� = ����λ + ���� + С��*0.0625
 */
/**
  * ��������: ������ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * �������: ��
  * �� �� ֵ: �¶�ֵ
  * ˵    ������
  */
float DS18B20_GetTemp_SkipRom ( void )
{
	uint8_t tpmsb, tplsb;
	short s_tem;
	float f_tem;
	
	
	DS18B20_SkipRom ();
	DS18B20_WriteByte(0X44);				/* ��ʼת�� */
	
	
	DS18B20_SkipRom ();
  DS18B20_WriteByte(0XBE);				/* ���¶�ֵ */
	
	tplsb = DS18B20_ReadByte();		 
	tpmsb = DS18B20_ReadByte(); 
	
	
	s_tem = tpmsb<<8;
	s_tem = s_tem | tplsb;
	
	if( s_tem < 0 )		/* ���¶� */
		f_tem = (~s_tem+1) * 0.0625;	
	else
		f_tem = s_tem * 0.0625;
	
	return f_tem; 	
}

/**
  * ��������: ��ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * �������: ds18b20_id�����ڴ�� DS18B20 ���кŵ�������׵�ַ
  * �� �� ֵ: ��
  * ˵    ������
  */
void DS18B20_ReadId ( uint8_t * ds18b20_id )
{
	uint8_t uc;
		
	DS18B20_WriteByte(0x33);       //��ȡ���к�
	
	for ( uc = 0; uc < 8; uc ++ )
	  ds18b20_id [ uc ] = DS18B20_ReadByte();	
}

/**
  * ��������: ��ƥ�� ROM ����»�ȡ DS18B20 �¶�ֵ 
  * �������: ds18b20_id����� DS18B20 ���кŵ�������׵�ַ
  * �� �� ֵ: �¶�ֵ
  * ˵    ������
  */
float DS18B20_GetTemp_MatchRom ( uint8_t * ds18b20_id )
{
	uint8_t tpmsb, tplsb, i;
	short s_tem;
	float f_tem;
	
	
	DS18B20_MatchRom ();            //ƥ��ROM
	
  for(i=0;i<8;i++)
		DS18B20_WriteByte ( ds18b20_id [ i ] );	
	
	DS18B20_WriteByte(0X44);				/* ��ʼת�� */

	
	DS18B20_MatchRom ();            //ƥ��ROM
	
	for(i=0;i<8;i++)
		DS18B20_WriteByte ( ds18b20_id [ i ] );	
	
	DS18B20_WriteByte(0XBE);				/* ���¶�ֵ */
	
	tplsb = DS18B20_ReadByte();		 
	tpmsb = DS18B20_ReadByte(); 
	
	
	s_tem = tpmsb<<8;
	s_tem = s_tem | tplsb;
	
	if( s_tem < 0 )		/* ���¶� */
		f_tem = (~s_tem+1) * 0.0625;	
	else
		f_tem = s_tem * 0.0625;
	
	return f_tem; 		
}

/******************* (C) COPYRIGHT 2015-2020 ӲʯǶ��ʽ�����Ŷ� *****END OF FILE****/