#include "sys.h"
#include "delay.h"
#include "usart.h" 
#include "led.h" 		 	 
#include "lcd.h"  
#include "key.h"     
#include "usmart.h" 
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
#include "exfuns.h"   
#include "text.h"
#include "piclib.h"	
#include "string.h"		
#include "beep.h" 
#include "timer.h" 
#include "ov2640.h"
#include "dma.h"
//ALIENTEK STM32F103������ ��չʵ��6
//OV2640����� ʵ�� 
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾

#define OV2640_JPEG_WIDTH	1024	//JPEG���յĿ��	
#define OV2640_JPEG_HEIGHT	768		//JPEG���յĸ߶�

u8* ov2640_framebuf;				//֡����
extern u8 ov_frame;					//��timer.c���涨��	   
 
//�ļ������������⸲�ǣ�
//mode:0,����.bmp�ļ�;1,����.jpg�ļ�.
//bmp��ϳ�:����"0:PHOTO/PIC13141.bmp"���ļ���
//jpg��ϳ�:����"0:PHOTO/PIC13141.jpg"���ļ���
void camera_new_pathname(u8 *pname,u8 mode)
{	 
	u8 res;					 
	u16 index=0;
	while(index<0XFFFF)
	{
		if(mode==0)sprintf((char*)pname,"0:PHOTO/PIC%05d.bmp",index);
		else sprintf((char*)pname,"0:PHOTO/PIC%05d.jpg",index);
		res=f_open(ftemp,(const TCHAR*)pname,FA_READ);//���Դ�����ļ�
		if(res==FR_NO_FILE)break;		//���ļ���������=����������Ҫ��.
		index++;
	}
}
//OV2640�ٶȿ���
//����LCD�ֱ��ʵĲ�ͬ�����ò�ͬ�Ĳ���
void ov2640_speed_ctrl(void)
{
	u8 clkdiv,pclkdiv;			//ʱ�ӷ�Ƶϵ����PCLK��Ƶϵ��
	if(lcddev.width==240)		//2.8��LCD
	{
		clkdiv=1;
		pclkdiv=28;
	}else if(lcddev.width==320)	//3.5��LCD
	{
		clkdiv=3;
		pclkdiv=15;
	}else						//4.3/7��LCD
	{
		clkdiv=15;
		pclkdiv=4;
	} 
	SCCB_WR_Reg(0XFF,0X00);		
	SCCB_WR_Reg(0XD3,pclkdiv);	//����PCLK��Ƶ
	SCCB_WR_Reg(0XFF,0X01);
	SCCB_WR_Reg(0X11,clkdiv);	//����CLK��Ƶ	
}
//OV2640����jpgͼƬ
//pname:Ҫ�����jpg��Ƭ·��+����
//����ֵ:0,�ɹ�
//    ����,�������
u8 ov2640_jpg_photo(u8 *pname)
{
	FIL* f_jpg;
	u8 res=0;
	u32 bwr;
	u32 i=0;
	u32 jpeglen=0;
	u8* pbuf;
	
	f_jpg=(FIL *)mymalloc(SRAMIN,sizeof(FIL));	//����FIL�ֽڵ��ڴ����� 
	if(f_jpg==NULL)return 0XFF;					//�ڴ�����ʧ��.
	OV2640_JPEG_Mode();							//�л�ΪJPEGģʽ 
  	OV2640_OutSize_Set(OV2640_JPEG_WIDTH,OV2640_JPEG_HEIGHT); 
	SCCB_WR_Reg(0XFF,0X00);
	SCCB_WR_Reg(0XD3,30);
	SCCB_WR_Reg(0XFF,0X01);
	SCCB_WR_Reg(0X11,0X1); 
	for(i=0;i<10;i++)		//����10֡���ȴ�OV2640�Զ����ںã��ع��ƽ��֮��ģ�
	{
		while(OV2640_VSYNC==1);	 
		while(OV2640_VSYNC==0);	  
	}  
	while(OV2640_VSYNC==1)	//��ʼ�ɼ�jpeg����
	{
		while(OV2640_HREF)
		{  
			while(OV2640_PCLK==0); 
			ov2640_framebuf[jpeglen]=OV2640_DATA;
			while(OV2640_PCLK==1);  
			jpeglen++;
		} 
	}		
	res=f_open(f_jpg,(const TCHAR*)pname,FA_WRITE|FA_CREATE_NEW);//ģʽ0,���߳��Դ�ʧ��,�򴴽����ļ�	 
	if(res==0)
	{
		printf("jpeg data size:%d\r\n",jpeglen);	//���ڴ�ӡJPEG�ļ���С
		pbuf=(u8*)ov2640_framebuf;
		for(i=0;i<jpeglen;i++)//����0XFF,0XD8
		{
			if((pbuf[i]==0XFF)&&(pbuf[i+1]==0XD8))break;
		}
		if(i==jpeglen)res=0XFD;//û�ҵ�0XFF,0XD8
		else		//�ҵ���
		{
			pbuf+=i;//ƫ�Ƶ�0XFF,0XD8��
			res=f_write(f_jpg,pbuf,jpeglen-i,&bwr);
			if(bwr!=(jpeglen-i))res=0XFE; 
		}
	} 
	f_close(f_jpg);  
	OV2640_RGB565_Mode();	//RGB565ģʽ 
	myfree(SRAMIN,f_jpg); 
	return res;
}  


 int main(void)
 {	 
	u8 res;							 
	u8 *pname;					//��·�����ļ��� 
	u8 key;						//��ֵ		   
	u8 sd_ok=1;					//0,sd��������;1,SD������.
 	u16 pixcnt=0;				//����ͳ��
	u16 linecnt=0;				//����ͳ��
	 
	delay_init();	    	 //��ʱ������ʼ��	  
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 	//���ڳ�ʼ��Ϊ115200
	usmart_dev.init(72);		//��ʼ��USMART		
  	LED_Init();		  			//��ʼ����LED���ӵ�Ӳ���ӿ�
	KEY_Init();					//��ʼ������
	LCD_Init();			   		//��ʼ��LCD  
	BEEP_Init();        		//��������ʼ��	 
	W25QXX_Init();				//��ʼ��W25Q128
 	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ��
	exfuns_init();				//Ϊfatfs��ر��������ڴ�  
 	f_mount(fs[0],"0:",1); 		//����SD�� 
 	f_mount(fs[1],"1:",1); 		//����FLASH. 
	POINT_COLOR=RED;      
 	while(font_init()) 			//����ֿ�
	{	    
		LCD_ShowString(30,50,200,16,16,"Font Error!");
		delay_ms(200);				  
		LCD_Fill(30,50,240,66,WHITE);//�����ʾ	     
	}  	 
 	Show_Str(30,50,200,16,"STM32F103 ������",16,0);				    	 
	Show_Str(30,70,200,16,"OV2640�����ʵ��",16,0);			    	 
	Show_Str(30,90,200,16,"KEY0:����(bmp��ʽ)",16,0);			    	 
	Show_Str(30,110,200,16,"KEY1:����(jpg��ʽ)",16,0);					    	 
	Show_Str(30,130,200,16,"2015��4��16��",16,0);
	res=f_mkdir("0:/PHOTO");		//����PHOTO�ļ���
	if(res!=FR_EXIST&&res!=FR_OK) 	//�����˴���
	{		    
		Show_Str(30,150,240,16,"SD������,�޷�����!",16,0); 	 		  	     
		sd_ok=0;  	
	} 
	ov2640_framebuf=mymalloc(SRAMIN,52*1024);//����֡����
 	pname=mymalloc(SRAMIN,30);		//Ϊ��·�����ļ�������30���ֽڵ��ڴ�		    
 	while(!pname||!ov2640_framebuf)	//�ڴ�������
 	{	    
		Show_Str(30,150,240,16,"�ڴ����ʧ��!",16,0);
		delay_ms(200);				  
		LCD_Fill(30,150,240,146,WHITE);//�����ʾ	     
		delay_ms(200);				  
	}   											  
	while(OV2640_Init())			//��ʼ��OV2640
	{
		Show_Str(30,150,240,16,"OV2640 ����!",16,0);
		delay_ms(200);
	    LCD_Fill(30,150,239,206,WHITE);
		delay_ms(200);
	}
 	Show_Str(30,170,200,16,"OV2640 ����",16,0);
	delay_ms(1500);	 		 
	//TIM6_Int_Init(10000,7199);	//10Khz����Ƶ��,1�����ж�,�����򲻴�ӡ֡��									  
	OV2640_RGB565_Mode();			//RGB565ģʽ
  	OV2640_OutSize_Set(lcddev.width,lcddev.height); 
	ov2640_speed_ctrl();
	MYDMA_SRAMLCD_Init((u32)ov2640_framebuf);	 
 	while(1)
	{
		while(OV2640_VSYNC)			//�ȴ�֡�ź�
		{
			key=KEY_Scan(0);		//��֧������
			if(key==KEY0_PRES||key==KEY1_PRES)
			{ 	
				LED1=0;				//DS1��
				if(sd_ok)			//SD�������ſ������� 
				{ 
					if(key==KEY0_PRES)	//BMP����
					{
						camera_new_pathname(pname,0);			//�õ��ļ���
						res=bmp_encode(pname,0,0,lcddev.width,lcddev.height,0);
					}else if(key==KEY1_PRES)					//JPG����
					{
						camera_new_pathname(pname,1);			//�õ��ļ���	
 						res=ov2640_jpg_photo(pname); 
						OV2640_OutSize_Set(lcddev.width,lcddev.height); 	
						ov2640_speed_ctrl(); 
					}
 					if(res)//��������
					{
						Show_Str(30,130,240,16,"д���ļ�����!",16,0);		 
					}else 
					{
						Show_Str(30,130,240,16,"���ճɹ�!",16,0);
						Show_Str(30,150,240,16,"����Ϊ:",16,0);
						Show_Str(30+42,150,240,16,pname,16,0);		    
						BEEP=1;	//�������̽У���ʾ�������
						delay_ms(100);
					} 
				}else 					//��ʾSD������
				{					    
					Show_Str(40,130,240,12,"SD������!",12,0);
					Show_Str(40,150,240,12,"���չ��ܲ�����!",12,0);			    
				}
				LED1=1;				//DS1��
				BEEP=0;				//�رշ����� 
				delay_ms(1800);
			}  
		} 
		LCD_SetCursor(0,0);			//��������
		LCD_WriteRAM_Prepare();		//��ʼд��GRAM	 
 		linecnt=0;					//��ͳ������
		pixcnt=0;					//���ؼ���������
		while(linecnt<lcddev.height)	
		{
			while(OV2640_HREF)
			{  
				while(OV2640_PCLK==0);
				ov2640_framebuf[pixcnt++]=OV2640_DATA;   
				while(OV2640_PCLK==1); 
				while(OV2640_PCLK==0); 
				ov2640_framebuf[pixcnt++]=OV2640_DATA; 
				while(OV2640_PCLK==1);		
			}  
			if(pixcnt)
			{
				MYDMA_SRAMLCD_Enable();	//����DMA���ݴ���
				pixcnt=0;
				linecnt++;
			}
		} 
		ov_frame++;
		LED0=!LED0;		
	} 	   										    
}

















