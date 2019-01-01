/**
  ******************************************************************************
  * @file    main.c
  * @author  chewei
  * @version V1.0
  * @date    2018-12-10
  * @brief   emwin + ucosIII 
  ******************************************************************************
  * 实验平台:秉火  STM32 F103 开发板 
  */


/*
*********************************************************************************************************
*                                             包含文件
*********************************************************************************************************
*/

#include <includes.h>
#include <bsp_key.h> 
#include "stdio.h"
/*
*********************************************************************************************************
*                                               宏定义
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             任务控制块TCB
*********************************************************************************************************
*/
OS_TCB   AppTaskStartTCB;
OS_TCB   AppTaskKeyScanTCB;
OS_TCB   AppTaskGUIBaseTCB;
OS_TCB   AppTaskTimerBaseTCB;
OS_TCB   AppTaskKey1TCB;
OS_TCB   AppTaskKey2TCB;
OS_TMR   my_tmr;   //声明软件定时器对象

OS_SEM SemOfKey;          //标志KEY1是否被单击的多值信号量
CPU_INT32U tim_cnt=0;
CPU_INT32U stoptim_cnt=0;


/*
*********************************************************************************************************
*                                              栈空间STACKS
*********************************************************************************************************
*/
__align(8) static  CPU_STK  AppTaskStartStk[APP_TASK_START_STK_SIZE];
__align(8) static  CPU_STK  AppTaskKeyScanStk[APP_TASK_KEY_SCAN_STK_SIZE];
__align(8) static  CPU_STK  AppTaskGUIBaseStk[APP_TASK_GUI_BASE_STK_SIZE];
__align(8) static  CPU_STK  AppTaskTimerBaseStk[APP_TASK_GUI_BASE_STK_SIZE];
__align(8) static  CPU_STK  AppTaskKey1Stk [ 128 ];
__align(8) static  CPU_STK  AppTaskKey2Stk [ 128 ];


/*
*********************************************************************************************************
*                                             函数声明
*********************************************************************************************************
*/

static  void  AppTaskStart  (void *p_arg);
static  void  BSPTaskCreate (void);
static  void  AppTaskCreate(void);
void AppTaskTouchScan(void );
void AppTaskTmr_ ( void * p_arg );
void TmrCallback (OS_TMR *p_tmr, void *p_arg);
void AppTaskKey1 ( void * p_arg );
void AppTaskKey2 ( void * p_arg );
void Test_flash(void);
extern  void Snack_Task(void);
/*
*********************************************************************************************************
*                                            
*********************************************************************************************************
*/


/**
  * @brief  主函数
  * @param  无  
  * @retval 无
  */
int  main (void)
{
    OS_ERR  os_err;

    OSInit(&os_err);                                               		/* 初始化 uC/OS-III.                                      */
		//创建启动任务
    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                // 任务TCB                               
                 (CPU_CHAR   *)"App Task Start", 								// 任务名称                             
                 (OS_TASK_PTR ) AppTaskStart,									  // 任务函数指针                                
                 (void       *) 0,																	// 可选输入数据
                 (OS_PRIO     ) APP_TASK_START_PRIO,							// 优先级
                 (CPU_STK    *)&AppTaskStartStk[0],							// 任务栈基地址
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10,		// 栈“水印”限制
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,        //栈大小
                 (OS_MSG_QTY  ) 5u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),//可选配置
                 (OS_ERR     *)&os_err);															//错误代码

		//开始执行任务，从此处开始由ucos系统调度
    OSStart(&os_err);                                              
		
		
}


/**
  * @brief  启动任务函数，主要完成ucos、BSP、GUI以及其它任务的初始化
  * @param  p_arg: OSTaskCreate创建时传入的数据指针
  * @retval 无
  */
static  void  AppTaskStart (void *p_arg)
{
    OS_ERR      os_err;
	  CPU_INT32U     cpu_clk_freq;
   (void)p_arg;

	BSP_Init();  //初始化BSP  		
	CPU_Init();	//初始化CPU
	BSP_Tick_Init();//初始化systick
  Mem_Init(); 	//初始化存储管理器                                     
	Key_Initial();
  Test_flash();

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&os_err); //计算无任务时CPU使用率                            
#endif

#ifdef CPU_CFG_INT_DIS_MEAS_EN
    CPU_IntDisMeasMaxCurReset();
#endif

	APP_TRACE_DBG(("正在创建应用任务...\n\r"));

	//创建驱动任务
	BSPTaskCreate();
	//创建应用任务
	AppTaskCreate();  
  cpu_clk_freq = BSP_CPU_ClkFreq();     

 //任务死循环
	while (DEF_TRUE) 
	{                                        
	    //LED2_TOGGLE ;
	    printf("cpu使用率:%.2f%%,%d\n,",((float)OSStatTaskCPUUsage/100),cpu_clk_freq);
	
  		/*
                  延时，所有任务函数的死循环内都应有至少1ms延时
		  特别是高优先级的任务，若无延时，其它低优先级任务可能会无机会执行
                */
	    OSTimeDly(1000u,OS_OPT_TIME_DLY,&os_err);
	}	
		
}

/**
  * @brief  创建驱动任务，如按键，定时扫描等
  * @param  无
  * @retval 无
  */
static  void  BSPTaskCreate (void)
{
	OS_ERR  os_err;

		//创建扫描任务
    OSTaskCreate((OS_TCB     *)&AppTaskKeyScanTCB,             // 任务TCB                               
		 (CPU_CHAR   *)"Key Scan", 										// 任务名称                             
	         (OS_TASK_PTR ) AppTaskTouchScan,									// 任务函数指针                                
		 (void       *) 0,						// 可选输入数据
		 (OS_PRIO     ) APP_TASK_KEY_SCAN_PRIO,				// 优先级
		 (CPU_STK    *)&AppTaskKeyScanStk[0],				// 任务栈基地址
		 (CPU_STK_SIZE) APP_TASK_KEY_SCAN_STK_SIZE / 10,	 	// 栈“水印”限制
		 (CPU_STK_SIZE) APP_TASK_KEY_SCAN_STK_SIZE,        		//栈大小
		 (OS_MSG_QTY  ) 0u,
		 (OS_TICK     ) 0u,
		 (void       *) 0,
		 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),	//可选配置
		 (OS_ERR     *)&os_err);					//错误代码


    OSTaskCreate((OS_TCB     *)&AppTaskTimerBaseTCB,             // 任务TCB                               
                 (CPU_CHAR   *)"GUI Base Test",                                // 任务名称                             
                 (OS_TASK_PTR ) AppTaskTmr_,                                   // 任务函数指针                                
                 (void       *) 0,                                             // 可选输入数据
                 (OS_PRIO     ) 2,                  // 优先级
                 (CPU_STK    *)&AppTaskTimerBaseStk[0],                        // 任务栈基地址
                 (CPU_STK_SIZE) 128 / 10,               // 栈“水印”限制
                 (CPU_STK_SIZE) 128,                //栈大小
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),//可选配置
                 (OS_ERR     *)&os_err);    


 /* 创建多值信号量 SemOfKey */
    OSSemCreate((OS_SEM      *)&SemOfKey,    //指向信号量变量的指针
                (CPU_CHAR    *)"SemOfKey",    //信号量的名字
                (OS_SEM_CTR   )5,             //表示现有资源数目
                (OS_ERR      *)&os_err);         //错误类型
                          
          

/* 创建 AppTaskKey1 任务 */
    OSTaskCreate((OS_TCB     *)&AppTaskKey1TCB,                             //任务控制块地址
                 (CPU_CHAR   *)"App Task Key1",                             //任务名称
                 (OS_TASK_PTR ) AppTaskKey1,                                //任务函数
                 (void       *) 0,                                          //传递给任务函数（形参p_arg）的实参
                 (OS_PRIO     ) 2,                      //任务的优先级
                 (CPU_STK    *)&AppTaskKey1Stk[0],                          //任务堆栈的基地址
                 (CPU_STK_SIZE) 128 / 10,             //任务堆栈空间剩下1/10时限制其增长
                 (CPU_STK_SIZE) 128,                  //任务堆栈空间（单位：sizeof(CPU_STK)）
                 (OS_MSG_QTY  ) 5u,                                         //任务可接收的最大消息数
                 (OS_TICK     ) 0u,                                         //任务的时间片节拍数（0表默认值OSCfg_TickRate_Hz/10）
                 (void       *) 0,                                          //任务扩展（0表不扩展）
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //任务选项
                 (OS_ERR     *)&os_err);                                       //返回错误类型

        /* 创建 AppTaskKey2 任务 */
    OSTaskCreate((OS_TCB     *)&AppTaskKey2TCB,                             //任务控制块地址
                 (CPU_CHAR   *)"App Task Key2",                             //任务名称
                 (OS_TASK_PTR ) AppTaskKey2,                                //任务函数
                 (void       *) 0,                                          //传递给任务函数（形参p_arg）的实参
                 (OS_PRIO     ) 3,                      //任务的优先级
                 (CPU_STK    *)&AppTaskKey2Stk[0],                          //任务堆栈的基地址
                 (CPU_STK_SIZE) 128 / 10,             //任务堆栈空间剩下1/10时限制其增长
                 (CPU_STK_SIZE) 128,                  //任务堆栈空间（单位：sizeof(CPU_STK)）
                 (OS_MSG_QTY  ) 5u,                                         //任务可接收的最大消息数
                 (OS_TICK     ) 0u,                                         //任务的时间片节拍数（0表默认值OSCfg_TickRate_Hz/10）
                 (void       *) 0,                                          //任务扩展（0表不扩展）
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), //任务选项
                 (OS_ERR     *)&os_err);                                       //返回错误类型

   /* 创建软件定时器 */
   OSTmrCreate ((OS_TMR              *)&my_tmr,             //软件定时器对象
                (CPU_CHAR            *)"MySoftTimer",       //命名软件定时器
                (OS_TICK              )10,                  //定时器初始值，依10Hz时基计算，即为1s
                (OS_TICK              )10,                  //定时器周期重载值，依10Hz时基计算，即为1s
                (OS_OPT               )OS_OPT_TMR_PERIODIC, //周期性定时
                (OS_TMR_CALLBACK_PTR  )TmrCallback,         //回调函数
                (void                *)"Timer Over!",       //传递实参给回调函数
                (OS_ERR              *)os_err);                //返回错误类型
                              
     /* 启动软件定时器 */                      
      
                    



}

/**
  * @brief  创建应用任务的函数，它会被启动任务调用以创建应用
  * @param  无
  * @retval 无
  */
static  void  AppTaskCreate (void)
{
	OS_ERR  os_err;

		//创建应用任务
	OSTaskCreate((OS_TCB     *)&AppTaskGUIBaseTCB,             // 任务TCB                               
							 (CPU_CHAR   *)"GUI Base Test", 									// 任务名称                             
							 (OS_TASK_PTR ) Snack_Task,									  // 任务函数指针                                
							 (void       *) 0,																	// 可选输入数据
							 (OS_PRIO     ) APP_TASK_GUI_BASE_PRIO,					// 优先级
							 (CPU_STK    *)&AppTaskGUIBaseStk[0],							// 任务栈基地址
							 (CPU_STK_SIZE) APP_TASK_GUI_BASE_STK_SIZE / 10,				// 栈“水印”限制
							 (CPU_STK_SIZE) APP_TASK_GUI_BASE_STK_SIZE,        		//栈大小
							 (OS_MSG_QTY  ) 0u,
							 (OS_TICK     ) 0u,
							 (void       *) 0,
							 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),//可选配置
							 (OS_ERR     *)&os_err);															//错误代码


}




/*********************************************END OF FILE**********************/

