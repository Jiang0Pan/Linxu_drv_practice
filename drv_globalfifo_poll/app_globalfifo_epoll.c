/*
  ** @file           : drv_globalfifo_epoll.c
  ** @brief          : global fifo epoll application memory driver source file
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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>


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
      struct epoll_event ev_globalfifo;
      int err;
      int epfd;

      if (ioctl(fd, FIFO_CLEAR_CMD, 0) < 0)
        log_debug("ioctl commamd failure\r\n");

      epfd = epoll_create(1);
      if (epfd < 0) {
        perror("epoll_create()");
        return -1;
      }

      bzero(&ev_globalfifo, sizeof(struct epoll_event));
      ev_globalfifo.events = EPOLLIN | EPOLLPRI;

      err = epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev_globalfifo);
      if (err < 0) {
        perror("epoll_ctl()");
        return -1;
      }

      err = epoll_wait(epfd, &ev_globalfifo, 1, 15000);
      if (err < 0) {
        perror("epoll_wait()");
      } else if (err == 0) {
        log_debug("No data input in FIFO within 15 seconds.\r\n");
      } else {
        log_debug("FIFO is not empty\r\n");
      }

      err = epoll_ctl(epfd, EPOLL_CTL_DEL, fd, &ev_globalfifo);
      if (0 > err) {
        perror("epoll_ctl");
      }

    } else {
        log_debug("/dev/globalfifo open failure\r\n");
    }
}


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
