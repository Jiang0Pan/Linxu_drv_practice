/*
  ** @file           : drv_globalfifo_signal_1.c
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
#include <stdlib.h>


/*
  ** define
*/
#define   log_debug(fmt, ...)         printf("file:%s, function:%s, line:%d: "fmt"", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)


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
void sigterm_handler(int signo)
{
  log_debug("Hava Caught sig N.O. %d\n", signo);
  exit(0);
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
int main(int argc, char * argv[])
{
    signal(SIGINT, sigterm_handler);
    signal(SIGTERM, sigterm_handler);

    while(1);

    return 0;
}


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
