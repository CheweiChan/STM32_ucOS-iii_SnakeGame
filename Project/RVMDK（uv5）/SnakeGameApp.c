/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   GUIDEMO_Touch.c
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 iSO STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
	
#include "includes.h"
#include "GUI.h"
#include <time.h>
#include <math.h>

#define SnakeSize 3
#define FoodSize 2
#define SnakeSpeed 100
#define test 321


/**********************************************************************/
struct point{
    int x;//240
    int y;//320
};
int temp=100,temp2=100;
int right=0,up=0;
int RL=0,UD=0;
int length=30,i,Xmove=0,Ymove=0;
struct point snake[100];
struct point food;
extern CPU_INT32U tim_cnt;
void KeyInit(void)
{
    GUI_DrawLine(0,40,240,40);
    GUI_DrawLine(60,40,60,0);
    GUI_DrawLine(120,40,120,0);
    GUI_DrawLine(180,40,180,0);
    GUI_DispStringAt("<--",80,20);
    GUI_DispStringAt("-->",20,20);
    GUI_DispStringAt("up",145,20);
    GUI_DispStringAt("down",200,20);
}

void Direction_judgment(int xPhys,int yPhys)
{
    if(xPhys>10 &&yPhys>10)
    {
    if(2030>xPhys)
    {
    if(Xmove==0)
    {
        Xmove=1;Ymove=0;
        RL=1;UD=0;
        if(1140<xPhys) right=1;
        else right=0;
    }
    }
    else
    {
    if(Ymove==0)
    {
        Ymove=1;Xmove=0;
        RL=0;UD=1;
        if(2900>xPhys)    up=1;
        else  up=0;
    }
    }
    }
    
    if(RL)
    {  
        if(right) temp2-=SnakeSize+2;
        else temp2+=SnakeSize+2;
    }
    if(UD)
    {
        if(up) temp+=SnakeSize+2;
        else temp-=SnakeSize+2;
    }


}
void Snake_Bodyinit(void)
{
    for(i=0;i<length;i++)
      {
    snake[i].x=temp2;
     snake[i].y=temp;
      }
    food.x=rand()%240;
    food.y=rand()%320;

}
void Snake_BodyDraw(void)
{
    for(i=0;i<length;i++)
        {
        GUI_FillCircle(snake[i].x,snake[i].y,SnakeSize);
    }   
  
}
void Snake_MoveSetting(void)
{
    GUI_SetColor(GUI_WHITE);
    GUI_FillCircle(snake[length-1].x, snake[length-1].y,SnakeSize);
    GUI_SetColor(GUI_BLUE);
   // GUI_ClearRect(snake[length-1].x+3, snake[length-1].y+3,snake[length-1].x-3, snake[length-1].y-3);

     for(i=length;i>1;i--)
     {
     snake[i-1].x=snake[i-2].x;
     snake[i-1].y=snake[i-2].y;
    
    }
if(temp>=320)temp=43;
else if(temp<=43)temp=320;

if(temp2>=240)temp2=0;
else if(temp2<=0)temp2=240;

     snake[0].x=temp2;
     snake[0].y=temp;
}

void FoodSetting(void)
{
    GUI_SetColor(GUI_RED);
    GUI_FillCircle(food.x,food.y,FoodSize);  
    GUI_SetColor(GUI_BLUE);
 if(snake[0].y<=food.y+3 && snake[0].x<=food.x+3)
 {
    if(snake[0].y>=food.y-3 && snake[0].x>=food.x-3)
    {
    GUI_SetColor(GUI_WHITE);
    GUI_FillCircle(food.x,food.y,FoodSize);
    GUI_SetColor(GUI_BLUE);
    food.x=rand()%240;
    food.y=rand()%320;
    length++;
    }
 }
}

void Snack_Task(void) {
    GUI_PID_STATE TouchState;
    int xPhys, yPhys;
    char buf[20];
  //2900//2030//1140
  GUI_SetBkColor(GUI_WHITE);
  GUI_Clear();
  Snake_Bodyinit();
  GUI_SetColor(GUI_BLACK);//setting string color
  KeyInit();
  

  while(1)
    {
      sprintf(buf,"%02d:%02d",tim_cnt/60,tim_cnt%60);
      GUI_DispStringAt(buf,5,5);
    //  printf(buf);
      GUI_TOUCH_GetState(&TouchState);  /* Get the touch position in pixel */
      xPhys = GUI_TOUCH_GetxPhys();     /* Get the A/D mesurement result in x */
      yPhys = GUI_TOUCH_GetyPhys();     /* Get the A/D mesurement result in y */
      //printf("(%d,%d)\n",xPhys,yPhys);
      
      Direction_judgment(xPhys,yPhys);
      Snake_MoveSetting();
      FoodSetting();
      Snake_BodyDraw();
for(i=0;i<length;i++)
{
if((temp2 == snake[i+1].x) && (temp == snake[i+1].y) &&temp2 !=100 &&temp !=100)
{
goto end;
}
}
    //GUI_DispStringAt("*",230,310);

    GUI_Delay(SnakeSpeed);

  }
end:
   GUI_DispStringAt("GameOver\n",60,150);
}


/**********************************end of file***********************************************/



