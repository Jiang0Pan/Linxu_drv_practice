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
* Function:    sigterm_handler
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
static void signalio_handler(int signo)
{
  log_debug("receiver a signal from globalfifo,signalnum:%d\r\n", signo);
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
    int fd, oflags;
    
    fd = open("/dev/globalfifo", O_RDWR, S_IRUSR | S_IWUSR);
    if (-1 != fd) {
      signal(SIGIO, signalio_handler);
      
      fcntl(fd, F_SETOWN, getpid());
      oflags = fcntl(fd, F_GETFL);
      fcntl(fd, F_SETFL, oflags | FASYNC);

      while(1) 
      {
        sleep(1);
      }
    } else {
      log_debug("/dev/globalfifo open failure\r\n");
    }

    return 0;
}


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
