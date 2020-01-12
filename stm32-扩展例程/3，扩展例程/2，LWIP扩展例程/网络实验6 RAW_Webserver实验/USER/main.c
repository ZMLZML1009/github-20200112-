#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "lcd.h"
#include "usart.h"	
#include "timer.h"
#include "sram.h"
#include "malloc.h"
#include "beep.h"
#include "tsensor.h"
#include "rtc.h"
#include "string.h"
#include "usmart.h"	
#include "dm9000.h"
#include "lwip/netif.h"
#include "lwip_comm.h"
#include "lwipopts.h"
#include "httpd.h"
/************************************************
 ALIENTEKս��STM32������LWIPʵ��
 ����RAW API��WebServer Serverʵ��
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
		LCD_ShowString(30,30,200,16,16,"WARSHIP STM32F103");
		LCD_ShowString(30,50,200,16,16,"LWIP Webserver Test");
		LCD_ShowString(30,70,200,16,16,"ATOM@ALIENTEK");
		LCD_ShowString(30,90,200,16,16,"2015/3/21"); 	
	}
	if(mode&1<<1)
	{
		LCD_Fill(30,110,lcddev.width,lcddev.height,WHITE);	//�����ʾ
		LCD_ShowString(30,110,200,16,16,"lwIP Init Successed");
		if(lwipdev.dhcpstatus==2)sprintf((char*)buf,"DHCP IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//��ӡ��̬IP��ַ
		else sprintf((char*)buf,"Static IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//��ӡ��̬IP��ַ
		LCD_ShowString(30,130,210,16,16,buf); 
		speed=DM9000_Get_SpeedAndDuplex();//�õ�����
		if(speed&1<<1)LCD_ShowString(30,150,200,16,16,"Ethernet Speed:10M");
		else LCD_ShowString(30,150,200,16,16,"Ethernet Speed:100M");
	}
}

int main(void)
{	 
	u32 i;
	delay_init();	    	//��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	 	//����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
 	LED_Init();			    //LED�˿ڳ�ʼ��
	LCD_Init();				//��ʼ��LCD
	KEY_Init();	 			//��ʼ������
	BEEP_Init();			//��������ʼ��
	RTC_Init();				//RTC��ʼ��
	T_Adc_Init();			//ADC��ʼ��
	TIM3_Int_Init(1000,719);//��ʱ��3Ƶ��Ϊ100hz
 	usmart_dev.init(72);	//��ʼ��USMART		
 	FSMC_SRAM_Init();		//��ʼ���ⲿSRAM
	my_mem_init(SRAMIN);	//��ʼ���ڲ��ڴ��
	my_mem_init(SRAMEX);	//��ʼ���ⲿ�ڴ��
	POINT_COLOR = RED; 		
	lwip_test_ui(1);		//����ǰ�벿��UI		    
	while(lwip_comm_init()) //lwip��ʼ��
	{
		LCD_ShowString(30,110,200,20,16,"LWIP Init Falied!");
		delay_ms(1200);
		LCD_Fill(30,110,230,130,WHITE); //�����ʾ
		LCD_ShowString(30,110,200,16,16,"Retrying...");  
	}
	LCD_ShowString(30,110,200,20,16,"LWIP Init Success!");
 	LCD_ShowString(30,130,200,16,16,"DHCP IP configing...");
#if LWIP_DHCP   //ʹ��DHCP
	while((lwipdev.dhcpstatus!=2)&&(lwipdev.dhcpstatus!=0XFF))//�ȴ�DHCP��ȡ�ɹ�/��ʱ���
	{
		lwip_periodic_handle();	//LWIP�ں���Ҫ��ʱ����ĺ���
		lwip_pkt_handle();
	}
#endif
	lwip_test_ui(2);		//���غ�벿��UI 
	delay_ms(500);			//��ʱ1s
	delay_ms(500);
	httpd_init();  			//Web Serverģʽ
  	while(1)
	{	
		lwip_periodic_handle();
		lwip_pkt_handle();  
		i++;
		if(i%5000==0)LED0=!LED0;
		delay_us(100);
	}
}
