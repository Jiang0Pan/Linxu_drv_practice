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
    int fd;
    int counter = 0;
    int old_counter = 0;

    fd = open("/dev/second", O_RDONLY);
    if (-1 != fd) {
        while (1) {
            read (fd, &counter, sizeof(unsigned int));
            if (counter != old_counter) {
                log_debug("second after open /dev/second:%d\n", counter);
                old_counter = counter;
            }
        }
    } else {
        log_debug("/dev/second open failure\n");
    }

}


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
