/*
  ** @file           : drv_second_timer.c
  ** @brief          : second timer device driver source file
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
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/kern_levels.h>
#include <linux/printk.h>
#include <linux/kernel.h>


/*
  ** define
*/
#define     SECOND_MAJOR            (248)

#define     log_debug(fmt, ...)     printk(KERN_DEBUG   pr_fmt(fmt), ##__VA_ARGS__)
#define     log_info(fmt, ...)      printk(KERN_INFO    pr_fmt(fmt), ##__VA_ARGS__)
#define     log_notice(fmt, ...)    printk(KERN_NOTICE  pr_fmt(fmt), ##__VA_ARGS__)
#define     log_warning(fmt, ...)   printk(KERN_WARNING pr_fmt(fmt), ##__VA_ARGS__)

#define     log_err(fmt, ...)       printk(KERN_ERR     pr_fmt(fmt), ##__VA_ARGS__)
#define     log_crit(fmt, ...)      printk(KERN_CRIT    pr_fmt(fmt), ##__VA_ARGS__)
#define     log_alert(fmt, ...)     printk(KERN_ALERT   pr_fmt(fmt), ##__VA_ARGS__)
#define     log_emerg(fmt, ...)     printk(KERN_EMERG   pr_fmt(fmt), ##__VA_ARGS__)


/*
  ** struct
*/
struct second_dev {
  struct cdev cdev;
  atomic_t counter;
  struct timer_list s_timer;
};


/*
  ** static function declaration
*/
static void second_timer_handler(unsigned long arg);
static int second_open(struct inode * inode, struct file * filp);
static ssize_t second_read(struct file * filp, char __user * buf, size_t count, loff_t * ppos);
static int second_release(struct inode * inode, struct file * filp);
static void second_setup_cdev(struct second_dev * dev, int index);


/* 
  ** static global variable 
*/
static int second_major = SECOND_MAJOR;
static struct second_dev * second_devp;
module_param(second_major, int, S_IRUGO);

static struct file_operations second_fops = {
  .owner = THIS_MODULE,
  .open = second_open,
  .read = second_read,
  .release = second_release,
};


/* 
  ** static function list
*/

/********************************************************************************************
* Function:    second_timer_handler
* Description: second timer handler
* Input:       arg: timer handler param
* Output:      None
* Return:      None
* Others:      
* Revision history:
             1.Date:     2022-2-9
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static void second_timer_handler(unsigned long arg)
{
  mod_timer(&second_devp->s_timer, jiffies + HZ);
  atomic_inc(&second_devp->counter);

  log_debug("current jiffies is %ld\n", jiffies);
}


/********************************************************************************************
* Function:    second_open
* Description: second open
* Input:       inode: strcut inode pointer
*              filp: struct fil pointer
* Output:      None
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-2-9
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static int second_open(struct inode * inode, struct file * filp)
{
  init_timer(&second_devp->s_timer);
  second_devp->s_timer.function = &second_timer_handler;
  second_devp->s_timer.expirse = jiffies + HZ;

  add_timer(&second_devp->s_timer);

  atomic_set(&second_devp->counter, 0);

  return 0;
}


/********************************************************************************************
* Function:    second_read
* Description: second read count
* Input:       filp: struct fil pointer
*              count: read buffer size
*              ppos: read file pos
* Output:      buf: read buffer
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-2-9
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static ssize_t second_read(struct file * filp, char __user * buf, size_t count, loff_t * ppos)
{
  int counter;

  counter = atomic_read(&second_devp->counter);
  if (put_user(counter, (int *)buf))
    return -EFAULT;
  else
    return sizeof(unsigned int);
}


/********************************************************************************************
* Function:    second_release
* Description: second release
* Input:       inode: strcut inode pointer
*              filp: struct fil pointer
* Output:      None
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-2-9
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static int second_release(struct inode * inode, struct file * filp)
{
  del_timer(&second_devp->s_timer);
  
  return 0;
}


/********************************************************************************************
* Function:    second_setup_cdev
* Description: second setup cdev
* Input:       dev: strcut second_dev pointer
*              index: index
* Output:      None
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-2-9
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static void second_setup_cdev(struct second_dev * dev, int index)
{
  int err, devno = MKDEV(second_major, index);

  cdev_init(&dev->cdev, &second_fops);
  dev->cdev.owner = THIS_MODULE;
  err = cdev_add(&dev->cdev, devno, 1);
  if (err) 
    log_debug("Failed to add second device\n");
}


/********************************************************************************************
* Function:    second_init
* Description: second init
* Input:       None
* Output:      None
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-2-9
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static int __init second_init(void)
{
    int ret;
    dev_t devno = MKDEV(second_major, 0);

    if (second_major) 
      ret = register_chrdev_region(devno, 1, "second");
    else {
      ret = alloc_chrdev_region(&devno, 0, 1, "second");
      second_major = MAJOR(devno);
    }

    if (ret < 0)
      return ret;
    
    second_devp = kzalloc(sizeof(struct second_dev), GFP_KERNEL);
    if (!second_devp) {
      ret = -ENOMEM;
      goto fail_malloc;
    }

    second_setup_cdev(second_devp, 0);

    return 0;    

fail_malloc:
  unregister_chrdev_region(devno, 1);
  return ret;
}


/********************************************************************************************
* Function:    second_exit
* Description: second exit
* Input:       None
* Output:      None
* Return:      None
* Others:      
* Revision history:
             1.Date:     2022-2-9
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static void __exit second_exit(void)
{
    cdev_del(&second_devp->cdev);
    kfree(second_devp);
    unregister_chrdev_region(MKDEV(second_major, 0), 1);
}


module_init(second_init);
module_exit(second_exit);


MODULE_AUTHOR("JexJiang");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Second Timer Module");
MODULE_VERSION("v1.0");


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
