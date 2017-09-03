#include "stm32f1xx_hal.h"
#include "user_config.h"
#include "user_io.h"
#include "spi_flash.h"
#include "delay.h"

#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
				 
/* control command table */
#define W25X_WRITE_ENABLE		0x06 
#define W25X_WRITE_DISABLE		0x04 
#define W25X_READ_STATUS_REG	0x05 
#define W25X_WRITE_STATUS_REG	0x01 
#define W25X_READ_DATA			0x03 
#define W25X_FAST_READ_DATA		0x0B 
#define W25X_FAST_READ_DUAL		0x3B 
#define W25X_PAGE_PROGRAM		0x02 
#define W25X_BLOCK_ERASE		0xD8 
#define W25X_SECTOR_ERASE		0x20 
#define W25X_CHIP_ERASE			0xC7 
#define W25X_POWER_DOWN			0xB9 
#define W25X_RELEASE_POWER_DOWN	0xAB 
#define W25X_DEVICE_ID			0xAB 
#define W25X_MANUFACT_DEVICE_ID	0x90 
#define W25X_JEDEC_DEVICE_ID	0x9F

static uint16_t spi_flash_read_id(void);  	    //��ȡFLASH ID
static uint8_t spi_flash_read_sr(void);        //��ȡ״̬�Ĵ��� 
static void spi_flash_write_sr(uint8_t sr);  	//д״̬�Ĵ���
static void spi_flash_write_enable(void);  //дʹ�� 
static void spi_flash_write_disable(void);	//д����
static void spi_flash_write_nocheck(uint8_t* p_buffer,uint32_t write_addr,uint16_t num_byte_to_write);
static void spi_flash_wait_busy(void);           //�ȴ�����

uint16_t spi_flash_type = W25Q64;//Ĭ�Ͼ���25Q64

void spi_flash_init(void)
{	
	spi_flash_type = spi_flash_read_id();//��ȡFLASH ID.  
	while(spi_flash_read_id()!=W25Q64)							//��ⲻ��W25Q64
	{
		delay_ms(100);
		LED_TOGGLE();
	}
} 

//��ȡSPI_FLASH��״̬�Ĵ���
//BIT7  6   5   4   3   2   1   0
//SPR   RV  TB BP2 BP1 BP0 WEL BUSY
//SPR:Ĭ��0,״̬�Ĵ�������λ,���WPʹ��
//TB,BP2,BP1,BP0:FLASH����д��������
//WEL:дʹ������
//BUSY:æ���λ(1,æ;0,����)
//Ĭ��:0x00
uint8_t spi_flash_read_sr(void)   
{  
	uint8_t byte=0;   
	SPI_FLASH_CS(0);                            //ʹ������   
	spi2_read_write_byte(W25X_READ_STATUS_REG);    //���Ͷ�ȡ״̬�Ĵ�������    
	byte=spi2_read_write_byte(0Xff);             //��ȡһ���ֽ�  
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     
	return byte;   
} 
//дSPI_FLASH״̬�Ĵ���
//ֻ��SPR,TB,BP2,BP1,BP0(bit 7,5,4,3,2)����д!!!
void spi_flash_write_sr(uint8_t sr)   
{   
	SPI_FLASH_CS(0);                            //ʹ������   
	spi2_read_write_byte(W25X_WRITE_STATUS_REG);   //����дȡ״̬�Ĵ�������    
	spi2_read_write_byte(sr);               //д��һ���ֽ�  
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     	      
}   
//SPI_FLASHдʹ��	
//��WEL��λ   
void spi_flash_write_enable(void)   
{
	SPI_FLASH_CS(0);                            //ʹ������   
    spi2_read_write_byte(W25X_WRITE_ENABLE);      //����дʹ��  
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     	      
} 
//SPI_FLASHд��ֹ	
//��WEL����  
void spi_flash_write_disable(void)   
{  
	SPI_FLASH_CS(0);                            //ʹ������   
    spi2_read_write_byte(W25X_WRITE_DISABLE);     //����д��ָֹ��    
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     	      
} 			    
//��ȡоƬID W25X16��ID:0XEF14
uint16_t spi_flash_read_id(void)
{
	uint16_t temp = 0;	  
	SPI_FLASH_CS(0);				    
	spi2_read_write_byte(0x90);//���Ͷ�ȡID����	    
	spi2_read_write_byte(0x00); 	    
	spi2_read_write_byte(0x00); 	    
	spi2_read_write_byte(0x00); 	 			   
	temp|=spi2_read_write_byte(0xFF)<<8;  
	temp|=spi2_read_write_byte(0xFF);	 
	SPI_FLASH_CS(1);				    
	return temp;
}   		    
//��ȡSPI FLASH  
//��ָ����ַ��ʼ��ȡָ�����ȵ�����
//p_buffer:���ݴ洢��
//read_addr:��ʼ��ȡ�ĵ�ַ(24bit)
//num_byte_to_read:Ҫ��ȡ���ֽ���(���65535)
void spi_flash_read(uint8_t* p_buffer,uint32_t read_addr,uint16_t num_byte_to_read)   
{ 
 	uint16_t i;    												    
	SPI_FLASH_CS(0);                            //ʹ������   
    spi2_read_write_byte(W25X_READ_DATA);         //���Ͷ�ȡ����   
    spi2_read_write_byte((uint8_t)((read_addr)>>16));  //����24bit��ַ    
    spi2_read_write_byte((uint8_t)((read_addr)>>8));   
    spi2_read_write_byte((uint8_t)read_addr);   
    for(i=0;i<num_byte_to_read;i++)
	{ 
        p_buffer[i]=spi2_read_write_byte(0XFF);   //ѭ������  
    }
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     	      
}  
//SPI��һҳ(0~65535)��д������256���ֽڵ�����
//��ָ����ַ��ʼд�����256�ֽڵ�����
//p_buffer:���ݴ洢��
//write_addr:��ʼд��ĵ�ַ(24bit)
//num_byte_to_write:Ҫд����ֽ���(���256),������Ӧ�ó�����ҳ��ʣ���ֽ���!!!	 
void spi_flash_write_Page(uint8_t* p_buffer,uint32_t write_addr,uint16_t num_byte_to_write)
{
 	uint16_t i;  
    spi_flash_write_enable();                  //SET WEL 
	SPI_FLASH_CS(0);                            //ʹ������   
    spi2_read_write_byte(W25X_PAGE_PROGRAM);      //����дҳ����   
    spi2_read_write_byte((uint8_t)((write_addr)>>16)); //����24bit��ַ    
    spi2_read_write_byte((uint8_t)((write_addr)>>8));   
    spi2_read_write_byte((uint8_t)write_addr);   
    for(i=0;i<num_byte_to_write;i++)spi2_read_write_byte(p_buffer[i]);//ѭ��д��  
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ 
	spi_flash_wait_busy();					   //�ȴ�д�����
} 
//�޼���дSPI FLASH 
//����ȷ����д�ĵ�ַ��Χ�ڵ�����ȫ��Ϊ0XFF,�����ڷ�0XFF��д������ݽ�ʧ��!
//�����Զ���ҳ���� 
//��ָ����ַ��ʼд��ָ�����ȵ�����,����Ҫȷ����ַ��Խ��!
//p_buffer:���ݴ洢��
//write_addr:��ʼд��ĵ�ַ(24bit)
//num_byte_to_write:Ҫд����ֽ���(���65535)
//CHECK OK
void spi_flash_write_nocheck(uint8_t* p_buffer,uint32_t write_addr,uint16_t num_byte_to_write)   
{ 			 		 
	uint16_t pageremain;	   
	pageremain=256-write_addr%256; //��ҳʣ����ֽ���		 	    
	if(num_byte_to_write<=pageremain)pageremain=num_byte_to_write;//������256���ֽ�
	while(1)
	{	   
		spi_flash_write_Page(p_buffer,write_addr,pageremain);
		if(num_byte_to_write==pageremain)break;//д�������
	 	else //num_byte_to_write>pageremain
		{
			p_buffer+=pageremain;
			write_addr+=pageremain;	

			num_byte_to_write-=pageremain;			  //��ȥ�Ѿ�д���˵��ֽ���
			if(num_byte_to_write>256)pageremain=256; //һ�ο���д��256���ֽ�
			else pageremain=num_byte_to_write; 	  //����256���ֽ���
		}
	};	    
} 
//дSPI FLASH  
//��ָ����ַ��ʼд��ָ�����ȵ�����
//�ú�������������!
//p_buffer:���ݴ洢��
//write_addr:��ʼд��ĵ�ַ(24bit)
//num_byte_to_write:Ҫд����ֽ���(���65535)  		   
uint8_t spi_flash_buf[4096];
void spi_flash_write(uint8_t* p_buffer,uint32_t write_addr,uint16_t num_byte_to_write)   
{ 
	uint32_t secpos;
	uint16_t secoff;
	uint16_t secremain;	   
 	uint16_t i;    

	secpos=write_addr/4096;//������ַ 0~511 for w25x16
	secoff=write_addr%4096;//�������ڵ�ƫ��
	secremain=4096-secoff;//����ʣ��ռ��С   

	if(num_byte_to_write<=secremain)secremain=num_byte_to_write;//������4096���ֽ�
	while(1) 
	{	
		spi_flash_read(spi_flash_buf,secpos*4096,4096);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(spi_flash_buf[secoff+i]!=0XFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			spi_flash_erase_sector(secpos);//�����������
			for(i=0;i<secremain;i++)	   //����
			{
				spi_flash_buf[i+secoff]=p_buffer[i];	  
			}
			spi_flash_write_nocheck(spi_flash_buf,secpos*4096,4096);//д����������  

		}else spi_flash_write_nocheck(p_buffer,write_addr,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(num_byte_to_write==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;//������ַ��1
			secoff=0;//ƫ��λ��Ϊ0 	 

		   	p_buffer+=secremain;  //ָ��ƫ��
			write_addr+=secremain;//д��ַƫ��	   
		   	num_byte_to_write-=secremain;				//�ֽ����ݼ�
			if(num_byte_to_write>4096)secremain=4096;	//��һ����������д����
			else secremain=num_byte_to_write;			//��һ����������д����
		}	 
	};	 	 
}
//��������оƬ
//��Ƭ����ʱ��:
//W25X16:25s 
//W25X32:40s 
//W25X64:40s 
//�ȴ�ʱ�䳬��...
void spi_flash_erase_chip(void)   
{                                             
    spi_flash_write_enable();                  //SET WEL 
    spi_flash_wait_busy();   
  	SPI_FLASH_CS(0);                            //ʹ������   
    spi2_read_write_byte(W25X_CHIP_ERASE);        //����Ƭ��������  
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     	      
	spi_flash_wait_busy();   				   //�ȴ�оƬ��������
}   
//����һ������
//dst_addr:������ַ 0~511 for w25x16
//����һ��ɽ��������ʱ��:150ms
void spi_flash_erase_sector(uint32_t dst_addr)   
{   
	dst_addr*=4096;
    spi_flash_write_enable();                  //SET WEL 	 
    spi_flash_wait_busy();   
  	SPI_FLASH_CS(0);                            //ʹ������   
    spi2_read_write_byte(W25X_SECTOR_ERASE);      //������������ָ�� 
    spi2_read_write_byte((uint8_t)((dst_addr)>>16));  //����24bit��ַ    
    spi2_read_write_byte((uint8_t)((dst_addr)>>8));   
    spi2_read_write_byte((uint8_t)dst_addr);  
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     	      
    spi_flash_wait_busy();   				   //�ȴ��������
}  
//�ȴ�����
void spi_flash_wait_busy(void)   
{   
	while ((spi_flash_read_sr()&0x01)==0x01);   // �ȴ�BUSYλ���
}  
//�������ģʽ
void spi_flash_powerdown(void)   
{ 
  	SPI_FLASH_CS(0);                            //ʹ������   
    spi2_read_write_byte(W25X_POWER_DOWN);        //���͵�������  
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     	      
    delay_us(3);                               //�ȴ�TPD  
}   
//����
void spi_flash_wakeup(void)   
{  
  	SPI_FLASH_CS(0);                            //ʹ������   
    spi2_read_write_byte(W25X_RELEASE_POWER_DOWN);   //  send W25X_POWER_DOWN command 0xAB    
	SPI_FLASH_CS(1);                            //ȡ��Ƭѡ     	      
    delay_us(3);                               //�ȴ�TRES1
}   
