/*
  ** @file           : drv_hello.c
  ** @brief          : hello driver source file
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kern_levels.h>
#include <linux/printk.h>
#include <linux/kernel.h>


/*
  ** define
*/
#define     log_debug(fmt, ...)     printk(KERN_DEBUG   pr_fmt(fmt), ##__VA_ARGS__)
#define     log_info(fmt, ...)      printk(KERN_INFO    pr_fmt(fmt), ##__VA_ARGS__)
#define     log_notice(fmt, ...)    printk(KERN_NOTICE  pr_fmt(fmt), ##__VA_ARGS__)
#define     log_warning(fmt, ...)   printk(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)

#define     log_err(fmt, ...)       printk(KERN_ERR     pr_fmt(fmt), ##__VA_ARGS__)
#define     log_crit(fmt, ...)      printk(KERN_CRIT    pr_fmt(fmt), ##__VA_ARGS__)
#define     log_alert(fmt, ...)     printk(KERN_ALERT   pr_fmt(fmt), ##__VA_ARGS__)
#define     log_emerg(fmt, ...)     printk(KERN_EMERG   pr_fmt(fmt), ##__VA_ARGS__)


/*
  ** static module param
*/
static char * module_charp_param = "dissecting Linux Device Driver";
module_param(module_charp_param, charp, S_IRUGO);

static int module_num_param = 4000;
module_param(module_num_param, int, S_IRUGO);


/* 
  ** static function list
*/
static int __init hello_param_init(void)
{
    log_debug("Hello param driver init\n");
    log_debug("module_charp_param:%s\n", module_charp_param);
    log_debug("module_num_param:%d\n", module_num_param);    
        
    return 0;    
}


static void __exit hello_param_exit(void)
{
    log_debug("Hello param driver exit\n");
}


module_init(hello_param_init);
module_exit(hello_param_exit);


MODULE_AUTHOR("JexJiang");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");
MODULE_VERSION("v1.0");


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
