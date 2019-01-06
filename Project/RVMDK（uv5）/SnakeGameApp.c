/**
  ******************************************************************************
  * @file    main.c
  * @author  chewei
  * @version V1.0
  * @date    2017.12.1
  * @brief   emWin+ucosiii
  ******************************************************************************
  ******************************************************************************
  */ 
	
#include "includes.h"
#include "GUI.h"
#include <time.h>
#include <math.h>

#define SnakeSize 3
#define FoodSize 2
#define SnakeSpeed 100


/**********************************************************************/
struct point{
    int x;//240
    int y;//320
};
int snackPosition_Y=100,snackPosition_X=100;
int moveToRight=0,moveToUp=0;
int length=30,i,xDirectMove=0,yDirectMove=0;
struct point snake[100];
struct point food;
extern CPU_INT32U tim_cnt;
void KeyInit(void)
{
    GUI_SetColor(GUI_BLACK);
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
/* 
4000    2900   2030     1140    0
 |                              |
 | down | up   | right   | left |
 |                              |
*/
    if(xPhys>10 && yPhys>10)//avoid the initial point(0,0)
    {
        if(2030>xPhys)//if touch xphys <2030,move to x-direct
        {
            if(xDirectMove==0)
            {
                xDirectMove=1;yDirectMove=0;
                if(1140<xPhys) moveToRight=1;
                else moveToRight=0;
            }
        }
        else//if touch xphys >2030,move to y-direct
        {
            if(yDirectMove==0)
            {
                yDirectMove=1;xDirectMove=0;
                if(2900>xPhys)    moveToUp=1;
                else  moveToUp=0;
            }
        }
    }
    
    if(xDirectMove)
    {  
        if(moveToRight) snackPosition_X-=SnakeSize+2;
        else snackPosition_X+=SnakeSize+2;
    }
    if(yDirectMove)
    {
        if(moveToUp) snackPosition_Y+=SnakeSize+2;
        else snackPosition_Y-=SnakeSize+2;
    }


}
void Snake_Bodyinit(void)
{
    for(i=0;i<length;i++)
    {
        snake[i].x=snackPosition_X;
        snake[i].y=snackPosition_Y;
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
    GUI_FillCircle(snake[length-1].x, snake[length-1].y,SnakeSize);//remove last snack point
    GUI_SetColor(GUI_BLUE);

    for(i=length;i>1;i--)
    {
        snake[i-1].x=snake[i-2].x;
        snake[i-1].y=snake[i-2].y;  
    }
    if(snackPosition_Y>=320)snackPosition_Y=43;
    else if(snackPosition_Y<=43)snackPosition_Y=320;
    
    if(snackPosition_X>=240)snackPosition_X=0;
    else if(snackPosition_X<=0)snackPosition_X=240;

    snake[0].x=snackPosition_X;
    snake[0].y=snackPosition_Y;
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
    char showTime[20];
    GUI_SetBkColor(GUI_WHITE);
    GUI_Clear();
    Snake_Bodyinit();
    KeyInit();
  

while(1)
{
    sprintf(showTime,"%02d:%02d",tim_cnt/60,tim_cnt%60);
    GUI_DispStringAt(showTime,5,5);
    GUI_TOUCH_GetState(&TouchState);  /* Get the touch position in pixel */
    xPhys = GUI_TOUCH_GetxPhys();     /* Get the A/D mesurement result in x */
    yPhys = GUI_TOUCH_GetyPhys();     /* Get the A/D mesurement result in y */
      
    Direction_judgment(xPhys,yPhys);
    Snake_MoveSetting();
    FoodSetting();
    Snake_BodyDraw();
    for(i=0;i<length;i++)
    {
        if((snackPosition_X == snake[i+1].x) && (snackPosition_Y == snake[i+1].y) &&snackPosition_X !=100 &&snackPosition_Y !=100)
        {
            goto end;
        }
    }
    GUI_Delay(SnakeSpeed);
}
end:
   GUI_DispStringAt("GameOver\n",60,150);
}


/**********************************end of file***********************************************/



