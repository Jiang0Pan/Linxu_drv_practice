/*
  ** @file           : drv_globalfifo_signal.c
  ** @brief          : global fifo signal application memory driver source file
  **
  ** @attention
  **
  ** Copyright (c) 2022 ShangHaiHeQian.
  ** All rights reserved.
  **
  ** This software is licensed by ShangHaiHeQian under Ultimate Liberty license
  **
*/


/*
  ** include 
*/
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>


/*
  ** define
*/
#define   log_debug(fmt, ...)         printf("file:%s, function:%s, line:%d: "fmt"", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define   MAX_LEN                     (100)


/********************************************************************************************
* Function:    intput_handler
* Description: signal handler function
* Input:       signo: signal number           
* Output:      None
* Return:      None
* Others:      
* Revision history:
             1.Date:     2022-1-30
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
void intput_handler(int signo)
{
  char data[MAX_LEN] = {0x0};
  int len;

  len = read(STDIN_FILENO, &data, MAX_LEN);
  if (len == MAX_LEN) {
    data[len - 1] = '\0';
  } else {
    data[len] = '\0';
  }

  log_debug("input available:%s\n", data);
}


/********************************************************************************************
* Function:    main
* Description: main function
* Input:       argc: arg count
*              argv: arg list                
* Output:      None
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-1-30
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
/* 
  ** 对SIGIO信号安装input_handler()作为处理函数
  ** 将本进程设置为STDIN_FILENO的文件拥有者
  ** 启用异步通知机制时，需要对设备设置FASYNC标志
*/
int main(int argc, char * argv[])
{
    int oflags;

    signal(SIGIO, intput_handler);

    fcntl(STDIN_FILENO, F_SETOWN, getpid());
    oflags = fcntl(STDIN_FILENO, F_GETFL);
    fcntl(STDIN_FILENO, F_SETFL, oflags | FASYNC);\

    while(1);

    return 0;
}


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
