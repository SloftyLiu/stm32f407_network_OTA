#include "tcp_client_demo.h" 
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "key.h"
#include "lcd.h"
#include "malloc.h"
#include "stdio.h"
#include "string.h"
#include "ff.h"

FATFS fs;

//TCP Client接收数据缓冲区
u8 tcp_client_recvbuf[TCP_CLIENT_RX_BUFSIZE];	
//TCP服务器发送数据内容
const u8 *tcp_client_sendbuf="Explorer STM32F4 TCP Client send data\r\n";

//TCP Client 测试全局状态标记变量
//bit7:0,没有数据要发送;1,有数据要发送
//bit6:0,没有收到数据;1,收到数据了.
//bit5:0,没有连接上服务器;1,连接上服务器了.
//bit4~0:保留
u8 tcp_client_flag;	 

//设置远端IP地址
void tcp_client_set_remoteip(void)
{
	u8 *tbuf;
	u16 xoff;
	u8 key;
	LCD_Clear(WHITE);
	POINT_COLOR=RED;
	LCD_ShowString(30,30,200,16,16,"Explorer STM32F4");
	LCD_ShowString(30,50,200,16,16,"TCP Client Test");
	LCD_ShowString(30,70,200,16,16,"Remote IP Set");  
	LCD_ShowString(30,90,200,16,16,"KEY0:+  KEY2:-");  
	LCD_ShowString(30,110,200,16,16,"KEY_UP:OK");  
	tbuf=mymalloc(SRAMIN,100);	//申请内存
	if(tbuf==NULL)return;
	//前三个IP保持和DHCP得到的IP一致
	lwipdev.remoteip[0]=lwipdev.ip[0];
	lwipdev.remoteip[1]=lwipdev.ip[1];
	lwipdev.remoteip[2]=lwipdev.ip[2]; 
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2]);//远端IP
	LCD_ShowString(30,150,210,16,16,tbuf); 
	POINT_COLOR=BLUE;
	xoff=strlen((char*)tbuf)*8+30;
	LCD_ShowxNum(xoff,150,lwipdev.remoteip[3],3,16,0); 
	while(1)
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)break;
		else if(key)
		{
			if(key==KEY0_PRES)lwipdev.remoteip[3]++;//IP增加
			if(key==KEY2_PRES)lwipdev.remoteip[3]--;//IP减少
			LCD_ShowxNum(xoff,150,lwipdev.remoteip[3],3,16,0X80);//显示新IP
		}
	}
	myfree(SRAMIN,tbuf); 
}
 

//TCP Client 测试
void tcp_client_test(void)
{
 	struct tcp_pcb *tcppcb;  	//定义一个TCP服务器控制块
	struct ip_addr rmtipaddr;  	//远端ip地址
	
	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	u8 t=0; 
	u8 connflag=0;		//连接标记
	
	tcp_client_set_remoteip();//先选择IP
	LCD_Clear(WHITE);	//清屏
	POINT_COLOR=RED; 	//红色字体
	LCD_ShowString(30,30,200,16,16,"Explorer STM32F4");
	LCD_ShowString(30,50,200,16,16,"TCP Client Test");
	LCD_ShowString(30,70,200,16,16,"Slofty");  
	LCD_ShowString(30,90,200,16,16,"KEY0:Send data");  
	LCD_ShowString(30,110,200,16,16,"KEY_UP:Quit");  
	tbuf=mymalloc(SRAMIN,200);	//申请内存
	if(tbuf==NULL)return ;		//内存申请失败了,直接退出
	sprintf((char*)tbuf,"Local IP:%d.%d.%d.%d",lwipdev.ip[0],lwipdev.ip[1],lwipdev.ip[2],lwipdev.ip[3]);//服务器IP
	LCD_ShowString(30,130,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote IP:%d.%d.%d.%d",lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]);//远端IP
	LCD_ShowString(30,150,210,16,16,tbuf);  
	sprintf((char*)tbuf,"Remote Port:%d",TCP_CLIENT_PORT);//客户端端口号
	LCD_ShowString(30,170,210,16,16,tbuf);
	POINT_COLOR=BLUE;
	LCD_ShowString(30,190,210,16,16,"STATUS:Disconnected"); 
	tcppcb=tcp_new();	//创建一个新的pcb
	if(tcppcb)			//创建成功
	{
		IP4_ADDR(&rmtipaddr,lwipdev.remoteip[0],lwipdev.remoteip[1],lwipdev.remoteip[2],lwipdev.remoteip[3]); 
		tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);  //连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
 	}else res=1;
	while(res==0)
	{
		key=KEY_Scan(0);
		if(key==WKUP_PRES)break;
		if(key==KEY0_PRES)//KEY0按下了,发送数据
		{
			tcp_client_usersent(tcppcb);    //发送数据
		}
		if(tcp_client_flag&1<<6)//是否收到数据?
		{
			//LCD_Fill(30,230,lcddev.width-1,lcddev.height-1,WHITE);//清上一次数据
			//LCD_ShowString(30,230,lcddev.width-30,lcddev.height-230,16,tcp_client_recvbuf);//显示接收到的数据			
			tcp_client_flag&=~(1<<6);//标记数据已经被处理了.
		}
		if(tcp_client_flag&1<<5)//是否连接上?
		{
			if(connflag==0)
			{ 
				LCD_ShowString(30,190,lcddev.width-30,lcddev.height-190,16,"STATUS:Connected   ");//提示消息		
				POINT_COLOR=BLUE;//蓝色字体
				connflag=1;//标记连接了
			} 
		}else if(connflag)
		{
 			LCD_ShowString(30,190,190,16,16,"STATUS:Disconnected");
			LCD_Fill(30,210,lcddev.width-1,lcddev.height-1,WHITE);//清屏
			connflag=0;	//标记连接断开了
		} 
		lwip_periodic_handle();
		delay_ms(2);
		t++;
		if(t==200)
		{
			if(connflag==0&&(tcp_client_flag&1<<5)==0)//未连接上,则尝试重连
			{ 
				tcp_client_connection_close(tcppcb,0);//关闭连接
				tcppcb=tcp_new();	//创建一个新的pcb
				if(tcppcb)			//创建成功
				{ 
					tcp_connect(tcppcb,&rmtipaddr,TCP_CLIENT_PORT,tcp_client_connected);//连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
				}
			}
			t=0;
			LED0=!LED0;
		}		
	}
	tcp_client_connection_close(tcppcb,0);//关闭TCP Client连接
	myfree(SRAMIN,tbuf);
} 
//lwIP TCP连接建立后调用回调函数
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct tcp_client_struct *es=NULL;  
	if(err==ERR_OK)   
	{
		es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //申请内存
		if(es) //内存申请成功
		{
 			es->state=ES_TCPCLIENT_CONNECTED;//状态为连接成功
			es->pcb=tpcb;  
			es->p=NULL; 
			tcp_arg(tpcb,es);        			//使用es更新tpcb的callback_arg
			tcp_recv(tpcb,tcp_client_recv);  	//初始化LwIP的tcp_recv回调功能   
			tcp_err(tpcb,tcp_client_error); 	//初始化tcp_err()回调函数
			tcp_sent(tpcb,tcp_client_sent);		//初始化LwIP的tcp_sent回调功能
			tcp_poll(tpcb,tcp_client_poll,1); 	//初始化LwIP的tcp_poll回调功能 
 			tcp_client_flag|=1<<5; 				//标记连接到服务器了
			err=ERR_OK;
		}else
		{ 
			tcp_client_connection_close(tpcb,es);//关闭连接
			err=ERR_MEM;	//返回内存分配错误
		}
	}else
	{
		tcp_client_connection_close(tpcb,0);//关闭连接
	}
	return err;
}

typedef enum 
{
	UPDATE_FREE = 0,
	UPDATE_BUSY,
	UPDATE_OVER
} update_state_typedef;
update_state_typedef update_state = UPDATE_FREE;

FATFS fs;
FIL file;
u32 file_writed = 0;
u32 file_size = 0;

void update_process(u8 *buf, u32 size)
{
	switch(update_state)
	{
		case UPDATE_FREE:
		{
			if(0 == memcmp(buf,"update start", 12))
			{
				u8 res = 0;
				file_size = 0;
				file_writed = 0;
				sscanf(buf, "update start:%d", &file_size);
				printf("update start:%d\r\n",file_size);
				res=f_mount(&fs,"1:",1); 				//挂载FLASH.	
				if(res==FR_OK)//FLASH磁盘,FAT文件系统正常
				{
					printf("Flash disk OK!\r\n");
				}
				res=f_open(&file, "1:/firmware.bin", FA_WRITE | FA_CREATE_ALWAYS);
				if(res==FR_OK)//FLASH磁盘,FAT文件系统正常
				{
					printf("firmware file OK!\r\n");
				}				
				update_state = UPDATE_BUSY;
			}
			break;
		}
		case UPDATE_BUSY:
		{
			UINT bw;
      f_write(&file, buf, size, &bw);
			file_writed += size;
			if(file_writed == file_size)
			{
				update_state = UPDATE_OVER;
			}
			printf("Update progress : %d%%\r\n", file_writed * 100 / file_size);
			break;
		}
		case UPDATE_OVER:
			if(0 == memcmp(buf,"update finish", size))
			{
				f_close(&file);
				printf("Update finsh!\r\n");
				update_state = UPDATE_FREE;
				NVIC_SystemReset();
			}
			break;
	}		
}


//lwIP tcp_recv()函数的回调函数
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
	u32 data_len=0;
	struct pbuf *q;
	struct tcp_client_struct *es;
	err_t ret_err; 
	static u32 pack_length = 0;
	static u32 pack_buf_pointer = 0;
	static u8 pack_buf[TCP_CLIENT_RX_BUFSIZE];
	
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_client_struct *)arg; 
	if(p==NULL)//如果从服务器接收到空的数据帧就关闭连接
	{
		es->state=ES_TCPCLIENT_CLOSING;//需要关闭TCP 连接了 
 		es->p=p; 
		ret_err=ERR_OK;
	}else if(err!= ERR_OK)//当接收到一个非空的数据帧,但是err!=ERR_OK
	{ 
		if(p)pbuf_free(p);//释放接收pbuf
		ret_err=err;
	}else if(es->state==ES_TCPCLIENT_CONNECTED)	//当处于连接状态时
	{
		//printf("rec %d\r\n",p->tot_len);
	
		memset(tcp_client_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);
		for(q=p;q!=NULL;q=q->next)
		{
			if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_client_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//????
			else memcpy(tcp_client_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > TCP_CLIENT_RX_BUFSIZE) 
			{
				break;
			}	
		}
		
		u32 tcp_buf_pointer = 0;

		while(tcp_buf_pointer < data_len)
		{
			if(pack_buf_pointer == 0)
			{
				//获取包头，包头包含一包的长度信息
				pack_length = *(u32 *)((u8 *)(tcp_client_recvbuf) + tcp_buf_pointer);
				tcp_buf_pointer += sizeof(u32);
				memset(pack_buf,0,TCP_CLIENT_RX_BUFSIZE);
				if(pack_length <= (data_len - tcp_buf_pointer))
				{
					//当前tcp buffer中的数据足够一包，直接取
					memcpy(pack_buf, ((u8*)(tcp_client_recvbuf) + tcp_buf_pointer), pack_length);
					tcp_buf_pointer += pack_length;
					update_process(pack_buf, pack_length);
				}
				else
				{
					//当前tcp buffer中的数据不足够一包，将当前数据暂存起来，下一轮tcp buffer继续接收
					pack_buf_pointer = 0;
					memset(pack_buf,0,TCP_CLIENT_RX_BUFSIZE);
					memcpy(pack_buf, ((u8*)(tcp_client_recvbuf) + tcp_buf_pointer), data_len - tcp_buf_pointer);
					pack_buf_pointer = data_len - tcp_buf_pointer;
					tcp_buf_pointer += data_len - tcp_buf_pointer;
				}
			}
			else
			{
				if( pack_length - pack_buf_pointer <= data_len)
				{
					//当前tcp buffer中的数据加上之前暂存的数据足够一包
					memcpy(((u8*)(pack_buf) + pack_buf_pointer), tcp_client_recvbuf, pack_length - pack_buf_pointer);
					update_process(pack_buf, pack_length);
					tcp_buf_pointer += pack_length - pack_buf_pointer;
					pack_buf_pointer = 0;			
				}
				else
				{
					//当前tcp buffer中的数据加上之前暂存的数据仍旧不足一包，继续暂存，下一轮tcp buffer继续接收
					memcpy(((u8*)(pack_buf) + pack_buf_pointer), tcp_client_recvbuf, data_len);
					pack_buf_pointer += data_len;
					tcp_buf_pointer += data_len;
				}
			}
		}

			
		tcp_client_flag|=1<<6;		//标记接收到数据了
		tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
		pbuf_free(p);  	//释放内存
		ret_err=ERR_OK;
	}else  //接收到数据但是连接已经关闭,
	{ 
		tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
		es->p=NULL;
		pbuf_free(p); //释放内存
		ret_err=ERR_OK;
	}
	return ret_err;
}

//lwIP tcp_err函数的回调函数
void tcp_client_error(void *arg,err_t err)
{  
	//这里我们不做任何处理
}

//LWIP数据发送，用户应用程序调用此函数来发送数据
//tpcb:TCP控制块
//返回值:0，成功；其他，失败
err_t tcp_client_usersent(struct tcp_pcb *tpcb)
{
    err_t ret_err;
	struct tcp_client_struct *es; 
	es=tpcb->callback_arg;
	if(es!=NULL)  //连接处于空闲可以发送数据
	{
        es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)tcp_client_sendbuf),PBUF_POOL);	//申请内存 
        pbuf_take(es->p,(char*)tcp_client_sendbuf,strlen((char*)tcp_client_sendbuf));	//将tcp_client_sentbuf[]中的数据拷贝到es->p_tx中
        tcp_client_senddata(tpcb,es);//将tcp_client_sentbuf[]里面复制给pbuf的数据发送出去
        tcp_client_flag&=~(1<<7);	//清除数据发送标志
        if(es->p)pbuf_free(es->p);	//释放内存
		ret_err=ERR_OK;
	}else
	{ 
		tcp_abort(tpcb);//终止连接,删除pcb控制块
		ret_err=ERR_ABRT;
	}
	return ret_err;
}

//lwIP tcp_poll的回调函数
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_client_struct *es; 
	es=(struct tcp_client_struct*)arg;
    if(es->state==ES_TCPCLIENT_CLOSING)         //连接断开
	{ 
 		tcp_client_connection_close(tpcb,es);   //关闭TCP连接
	} 
	ret_err=ERR_OK;
    return ret_err;
} 
//lwIP tcp_sent的回调函数(当从远端主机接收到ACK信号后发送数据)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_client_struct *es;
	LWIP_UNUSED_ARG(len);
	es=(struct tcp_client_struct*)arg;
	if(es->p)tcp_client_senddata(tpcb,es);//发送数据
	return ERR_OK;
}
//此函数用来发送数据
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	struct pbuf *ptr; 
 	err_t wr_err=ERR_OK;
	while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb))) //将要发送的数据加入到发送缓冲队列中
	{
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1);
		if(wr_err==ERR_OK)
		{  
			es->p=ptr->next;			//指向下一个pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf的ref加一
			pbuf_free(ptr);				//释放ptr 
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);		//将发送缓冲队列中的数据立即发送出去
	} 
} 
//关闭与服务器的连接
void tcp_client_connection_close(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	//移除回调
	tcp_abort(tpcb);//终止连接,删除pcb控制块
	tcp_arg(tpcb,NULL);  
	tcp_recv(tpcb,NULL);
	tcp_sent(tpcb,NULL);
	tcp_err(tpcb,NULL);
	tcp_poll(tpcb,NULL,0);  
	if(es)mem_free(es); 
	tcp_client_flag&=~(1<<5);//标记连接断开了
}
