#include <includes.h>
#include <bsp_key.h> 


extern OS_SEM SemOfKey;          //标志KEY1是否被单击的多值信号量
extern CPU_INT32U tim_cnt;
extern CPU_INT32U stoptim_cnt;
extern   OS_TCB   AppTaskGUIBaseTCB;
extern OS_TMR      my_tmr;   //声明软件定时器对象
void  AppTaskTouchScan  (void )
{

	OS_ERR  os_err;

	while(1)
	{
			GUI_TOUCH_Exec();
			LED2_TOGGLE ;
			//延时，所有任务函数的死循环内都应有至少1ms延时
		  //特别是高优先级的任务，若无延时，其它低优先级任务可能会无机会执行
			OSTimeDly(10, OS_OPT_TIME_DLY,&os_err);
	}
}
	

void  AppTaskTmr_ ( void * p_arg )
{

 	OS_ERR           err;

	CPU_SR_ALLOC();                                       //使用到临界段（在关/开中断时）时必需该宏，该宏声明和定义一个局部变
                                                        //量，用于保存关中断前的 CPU 状态寄存器 SR（临界段关中断只需保存SR）
                                                        //，开中断时将该值还原。
 (void)p_arg;

	while (DEF_TRUE) {                                    //任务体，通常都写成一个死循环    
		
		OSTimeDly ( 1000, OS_OPT_TIME_DLY, & err );         //延时1000个时钟节拍（1s）
		tim_cnt++;
		OS_CRITICAL_ENTER();                                //进入临界段，不希望下面串口打印遭到中断

		printf("\nTime=%02d:%02d",tim_cnt/60,tim_cnt%60);
		OS_CRITICAL_EXIT();                                //进入临界段，不希望下面串口打印遭到中断
		
	}
		
}


/*
*********************************************************************************************************
*                                          TMR TASK
*********************************************************************************************************
*/
void TmrCallback (OS_TMR *p_tmr, void *p_arg) //软件定时器MyTmr的回调函数
{
	CPU_SR_ALLOC();      //使用到临界段（在关/开中断时）时必需该宏，该宏声明和定义一个局部变
											 //量，用于保存关中断前的 CPU 状态寄存器 SR（临界段关中断只需保存SR）
											 //，开中断时将该值还原。  
  //printf ( "%s", ( char * ) p_arg );

stoptim_cnt++;	

	OS_CRITICAL_ENTER();                 //进入临界段，不希望下面串口打印遭到中断
	
printf("stoptime=%d:%d",stoptim_cnt/60,stoptim_cnt%60);
	
	OS_CRITICAL_EXIT();                               

	
}







/*
*********************************************************************************************************
*                                          KEY1 TASK
*********************************************************************************************************
*/
  void  AppTaskKey1 ( void * p_arg )
{
	OS_ERR      err;
	OS_SEM_CTR  ctr;
	CPU_SR_ALLOC();  //使用到临界段（在关/开中断时）时必需该宏，该宏声明和定义一个局部变
									 //量，用于保存关中断前的 CPU 状态寄存器 SR（临界段关中断只需保存SR）
									//，开中断时将该值还原。	
	uint8_t ucKey1Press = 0;
		
	(void)p_arg;
				 
	while (DEF_TRUE) {                                                         //任务体
		if( Key_Scan ( macKEY1_GPIO_PORT, macKEY1_GPIO_PIN, 1, & ucKey1Press ) ) //如果KEY1被单击
		{
			ctr = OSSemPend ((OS_SEM   *)&SemOfKey,               //等待该信号量 SemOfKey
								       (OS_TICK   )0,                       //下面选择不等待，该参无效
								       (OS_OPT    )OS_OPT_PEND_BLOCKING,//如果没信号量可用不等待
								       (CPU_TS   *)0,                       //不获取时间戳
								       (OS_ERR   *)&err);                   //返回错误类型
			
			OS_CRITICAL_ENTER();                                  //进入临界段
			
			if ( err == OS_ERR_NONE )                      
				printf ( "\r\nKEY1被单击：成功申请到停车位，剩下%d个停车位。", ctr );
			else if ( err == OS_ERR_PEND_WOULD_BLOCK )
				printf ( "\r\nkey1被单击：不好意思，现在停车场已满，请等待！" );
			
			OS_CRITICAL_EXIT(); 
            OSTaskSuspend ( &AppTaskGUIBaseTCB, & err );
            stoptim_cnt=0;
            OSTmrStart ((OS_TMR   *)&my_tmr, //软件定时器对象
                        (OS_ERR   *)err);    //返回错误类型

      


		}
		
		OSTimeDlyHMSM ( 0, 0, 0, 20, OS_OPT_TIME_DLY, & err );  //每20ms扫描一次
		
	}
	
}


/*
*********************************************************************************************************
*                                          KEY2 TASK
*********************************************************************************************************
*/
  void  AppTaskKey2 ( void * p_arg )
{
	OS_ERR      err;
	OS_SEM_CTR  ctr;
	CPU_SR_ALLOC();  //使用到临界段（在关/开中断时）时必需该宏，该宏声明和定义一个局部变
									 //量，用于保存关中断前的 CPU 状态寄存器 SR（临界段关中断只需保存SR）
									 //，开中断时将该值还原。
	uint8_t ucKey2Press = 0;
	
	
	(void)p_arg;

					 
	while (DEF_TRUE) {                                                         //任务体
		if( Key_Scan ( macKEY2_GPIO_PORT, macKEY2_GPIO_PIN, 1, & ucKey2Press ) ) //如果KEY2被单击
		{
		  ctr = OSSemPost((OS_SEM  *)&SemOfKey,                                  //发布SemOfKey
							        (OS_OPT   )OS_OPT_POST_ALL,                            //发布给所有等待任务
							        (OS_ERR  *)&err);                                      //返回错误类型

			OS_CRITICAL_ENTER();                                                   //进入临界段
			
			printf ( "\r\nKEY2被单击：释放1个停车位，剩下%d个停车位。", ctr );
			
			OS_CRITICAL_EXIT();
			OSTmrStop((OS_TMR   *)&my_tmr,OS_OPT_TMR_NONE,0,(OS_ERR   *)err);
            OSTaskResume ( &AppTaskGUIBaseTCB, & err );

		}
		
		OSTimeDlyHMSM ( 0, 0, 0, 20, OS_OPT_TIME_DLY, & err );                    //每20ms扫描一次
		
	}
	
}
