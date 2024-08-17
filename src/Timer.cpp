#include "Timer.h"


/********************************** 定时器配置 **********************************/
//定义一个定时器对象
hw_timer_t *timer1 = NULL;
uint16_t tim1_IRQ_count = 0;

void timer1_interrupt()
{//中断服务函数
  tim1_IRQ_count++;
  if(mqtt.status == 0 && mqtt.mqtt_maunual_flag == 0)  //若未连接
    mqtt.mqtt_connect_flag = 1;  //则设标志位为1，需要连接
  else if(mqtt.status == 1 && mqtt.mqtt_maunual_flag == 0)
    mqtt.mqtt_connect_flag = 0;
}

//100ms定时器
void timer1_init()
{
  timer1 = timerBegin(0,80,true);  //分频到80MHz/80 = 1NHz 即1us计数器+1
  // 配置定时器中断服务函数
  //定时器0的中断回调函数为timer_interrupt(),true表示边沿触发
  timerAttachInterrupt(timer1,timer1_interrupt,true);

  // 设置定时器0的计数值
  // 定时器0的计数值为1000，true为允许自动重载计数值
  // 计数值单位为us,100000 us = 100ms  ,要每0.1s中断一次 即计数100000
  timerAlarmWrite(timer1,100000,true); 

  // 使能定时器
  timerAlarmEnable(timer1);
}

//初始化所有的定时器，让外部初始化调用
void timer_init()
{
    timer1_init();
}