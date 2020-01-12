#include "dma.h"																	   	  
#include "delay.h"																	   	  
#include "lcd.h"																	   	  
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F103������ 
//DMA���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2015/1/15
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved	
//********************************************************************************
//V1.1 20150416
//��ӣ�MYDMA_SRAMLCD_Init��MYDMA_SRAMLCD_Enable������
//////////////////////////////////////////////////////////////////////////////////

u16 DMA1_MEM_LEN;//����DMAÿ�����ݴ��͵ĳ��� 		    
//DMA1�ĸ�ͨ������
//����Ĵ�����ʽ�ǹ̶���,���Ҫ���ݲ�ͬ��������޸�
//�Ӵ洢��->����ģʽ/8λ���ݿ��/�洢������ģʽ
//DMA_CHx:DMAͨ��CHx
//cpar:�����ַ
//cmar:�洢����ַ
//cndtr:���ݴ�����  
void MYDMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
	RCC->AHBENR|=1<<0;			//����DMA1ʱ��
	delay_ms(5);				//�ȴ�DMAʱ���ȶ�
	DMA_CHx->CPAR=cpar; 	 	//DMA1 �����ַ 
	DMA_CHx->CMAR=(u32)cmar; 	//DMA1,�洢����ַ
	DMA1_MEM_LEN=cndtr;      	//����DMA����������
	DMA_CHx->CNDTR=cndtr;    	//DMA1,����������
	DMA_CHx->CCR=0X00000000;	//��λ
	DMA_CHx->CCR|=1<<4;  		//�Ӵ洢����
	DMA_CHx->CCR|=0<<5;  		//��ͨģʽ
	DMA_CHx->CCR|=0<<6; 		//�����ַ������ģʽ
	DMA_CHx->CCR|=1<<7; 	 	//�洢������ģʽ
	DMA_CHx->CCR|=0<<8; 	 	//�������ݿ��Ϊ8λ
	DMA_CHx->CCR|=0<<10; 		//�洢�����ݿ��8λ
	DMA_CHx->CCR|=1<<12; 		//�е����ȼ�
	DMA_CHx->CCR|=0<<14; 		//�Ǵ洢�����洢��ģʽ		  	
} 
//����һ��DMA����
void MYDMA_Enable(DMA_Channel_TypeDef*DMA_CHx)
{
	DMA_CHx->CCR&=~(1<<0);       //�ر�DMA���� 
	DMA_CHx->CNDTR=DMA1_MEM_LEN; //DMA1,���������� 
	DMA_CHx->CCR|=1<<0;          //����DMA����
}	  
//SRAM --> LCD_RAM dma����
//caddr������Դ��ַ
//16λ,��SRAM���䵽LCD_RAM. 
void MYDMA_SRAMLCD_Init(u32 caddr)
{  
	RCC->AHBENR|=1<<1;						//����DMA2ʱ��  
	DMA2_Channel5->CPAR=caddr;				//DMA2��Դ�洢����ַ
	DMA2_Channel5->CMAR=(u32)&LCD->LCD_RAM;	//Ŀ���ַΪLCD_RAM
 	DMA2_Channel5->CNDTR=0;					//DMA2,�������������ݶ�Ϊ0
	DMA2_Channel5->CCR=0X00000000;			//��λ 
	
 	DMA2_Channel5->CCR|=0<<4;  				//�������
	DMA2_Channel5->CCR|=0<<5;  				//��ͨģʽ
	DMA2_Channel5->CCR|=1<<6; 				//�����ַ����ģʽ
	DMA2_Channel5->CCR|=0<<7; 				//�洢��������ģʽ
	DMA2_Channel5->CCR|=1<<8; 				//�������ݿ��Ϊ16λ
	DMA2_Channel5->CCR|=1<<10; 				//�洢�����ݿ��16λ
	DMA2_Channel5->CCR|=1<<12; 				//�е����ȼ�
	DMA2_Channel5->CCR|=1<<14; 				//�洢�����洢��ģʽ(����Ҫ�ⲿ����)	 
}  
//����һ��SRAM��LCD��DMA�Ĵ���
void MYDMA_SRAMLCD_Enable(void)
{	   
 	DMA2_Channel5->CCR&=~(1<<0); 		//�ر�DMA����  
	DMA2_Channel5->CNDTR=lcddev.width;	//���ô��䳤��  
	DMA2_Channel5->CCR|=1<<0;   		//����DMA����   
} 


 

























