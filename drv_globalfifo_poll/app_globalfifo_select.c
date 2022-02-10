/*
  ** @file           : drv_globalfifo_select.c
  ** @brief          : global fifo select application memory driver source file
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/select.h>


/*
  ** define
*/
#define   log_debug(fmt, ...)         printf("file:%s, function:%s, line:%d: "fmt"", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define   FIFO_CLEAR_CMD              (0x1)
#define   BUFFER_SIZE                 (20)


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
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
int main(int argc, char * argv[])
{
    int fd, num;
    char rd_ch[BUFFER_SIZE];

    fd_set rfds, wfds;

    fd = open("/dev/globalfifo", O_RDONLY | O_NONBLOCK);
    if (-1 != fd) {
        if (ioctl(fd, FIFO_CLEAR_CMD, 0) < 0) 
            log_debug("ioctl command execute failure\r\n");

        while(1) {
            FD_ZERO(&rfds);
            FD_ZERO(&wfds);
            FD_SET(fd, &rfds);
            FD_SET(fd, &wfds);

            select(fd + 1, &rfds, &wfds, NULL, NULL);
            if (FD_ISSET(fd, &rfds)) {
                log_debug("Poll monitor: can be read\r\n");
            }
            if(FD_ISSET(fd, &wfds)) {
                log_debug("Poll monitor: can be written\r\n");
            }
        }
    } else {
        log_debug("/dev/globalfifo open failure\r\n");
    }
}


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
