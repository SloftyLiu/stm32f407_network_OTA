#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "beep.h"
#include "usmart.h"
#include "sram.h"
#include "lan8720.h"
#include "timer.h"
#include "rtc.h"
#include "adc.h"
#include "malloc.h" 
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "tcp_client_demo.h"

/************************************************
 ALIENTEK ̽����STM32F407������ ʵ��58
 ����ͨ��ʵ��-HAL�⺯����
 ����֧�֣�www.openedv.com
 �Ա����̣�http://eboard.taobao.com 
 ��ע΢�Ź���ƽ̨΢�źţ�"����ԭ��"����ѻ�ȡSTM32���ϡ�
 ������������ӿƼ����޹�˾  
 ���ߣ�����ԭ�� @ALIENTEK
************************************************/

//����UI
//mode:
//bit0:0,������;1,����ǰ�벿��UI
//bit1:0,������;1,���غ�벿��UI
void lwip_test_ui(u8 mode)
{
	u8 speed;
	u8 buf[30]; 
	POINT_COLOR=RED;
	if(mode&1<<0)
	{
		LCD_Fill(30,30,lcddev.width,110,WHITE);	//�����ʾ
		LCD_ShowString(30,30,200,16,16,"Explorer STM32F4");
		LCD_ShowString(30,50,200,16,16,"Ethernet lwIP Test");
		LCD_ShowString(30,70,200,16,16,"Slofty");
		LCD_ShowString(30,90,200,16,16,"2025/7/5"); 	
	}
	if(mode&1<<1)
	{
		LCD_Fill(30,110,lcddev.width,lcddev.height,WHITE);	//�����ʾ
		LCD_ShowString(30,110,200,16,16,"lwIP Init Successed");
		if(lwipdev.dhcpstatus==2)sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//��ӡ��̬IP��ַ
		else sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//��ӡ��̬IP��ַ
		LCD_ShowString(30,130,210,16,16,buf); 
		speed=LAN8720_Get_Speed();//�õ�����
		if(speed&1<<1)LCD_ShowString(30,150,200,16,16,"Ethernet Speed:100M");
		else LCD_ShowString(30,150,200,16,16,"Ethernet Speed:10M");
	}
}

int main(void)
{
  HAL_Init();                   	//��ʼ��HAL��    
  Stm32_Clock_Init(336,8,2,7);  	//����ʱ��,168Mhz
	delay_init(168);               	//��ʼ����ʱ����
	uart_init(115200);             	//��ʼ��USART
	usmart_dev.init(84); 		    //��ʼ��USMART
	LED_Init();						//��ʼ��LED	
	BEEP_Init();					//��ʼ��������
	KEY_Init();						//��ʼ��KEY
	LCD_Init();                     //��ʼ��LCD
	SRAM_Init();					//��ʼ���ⲿSRAM  
	MY_ADC_Init();					//��ʼ��ADC
	RTC_Init();                     //��ʼ��RTC 
  TIM3_Init(1000-1,840-1);        //��ʱ��3��ʼ��,����10ms
	
	my_mem_init(SRAMIN);			//��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);			//��ʼ���ⲿ�ڴ��
	my_mem_init(SRAMCCM);			//��ʼ��CCM�ڴ��
	
  POINT_COLOR=RED;
	LED0=0;
	lwip_test_ui(1);			    //����ǰ�벿��UI
  LCD_ShowString(30,110,200,16,16,"lwIP Initing...");
	while(lwip_comm_init())         //lwip��ʼ��
	{
		LCD_ShowString(30,110,200,20,16,"LWIP Init Falied! ");
		delay_ms(500);
		LCD_ShowString(30,110,200,16,16,"Retrying...       ");
    delay_ms(500);
	}
	LCD_ShowString(30,110,200,20,16,"LWIP Init Success!");
 	LCD_ShowString(30,130,200,16,16,"DHCP IP configing...");  //�ȴ�DHCP��ȡ 
	
#if LWIP_DHCP   //ʹ��DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
	{  
		lwip_periodic_handle();	//LWIP�ں���Ҫ��ʱ����ĺ���
	}
#endif
	
  //lwip_test_ui(2);		//���غ�벿��UI

	while(1)
	{
		tcp_client_test();
		lwip_test_ui(3);//���¼���UI
	} 	
}

