/*
  ** @file           : drv_globalfifo.c
  ** @brief          : global memory driver source file
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
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/kern_levels.h>
#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/sched/signal.h>
#include <linux/memory.h>


/*
  ** define
*/
#define     GLOBALFIFO_SIZE         (0x1000)
#define     MEM_CLEAR_CMD           (0x1)
#define     GLOBALFIFO_MAJOR        (230)

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
struct globalfifo_dev {
  struct cdev cdev;
  unsigned int current_len;
  unsigned char mem[GLOBALFIFO_SIZE];
  struct mutex mutex;
  wait_queue_head_t r_wait;
  wait_queue_head_t w_wait;
};


/*
  ** static function declaration
*/
static ssize_t globalfifo_read(struct file * filp, char __user * buf, size_t size, loff_t * ppos);
static ssize_t globalfifo_write(struct file * filp, const char __user * buf, size_t size, loff_t * ppos);
static loff_t globalfifo_llseek(struct file * filp, loff_t offset, int orig);
static long globalfifo_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
static int globalfifo_open(struct inode * inode, struct file * filp);
static int globalfifo_release(struct inode * inode, struct file *filp);
static void globalfifo_setup_cdev(struct globalfifo_dev * dev, int index);


/*
  ** global variable
*/
static const struct file_operations globalfifo_fops = {
  .owner = THIS_MODULE,
  .llseek = globalfifo_llseek,
  .read = globalfifo_read,
  .write = globalfifo_write,
  .unlocked_ioctl = globalfifo_ioctl,
  .open = globalfifo_open,
  .release = globalfifo_release,
};


/*
  ** static global variable
*/
static int globalfifo_major = GLOBALFIFO_MAJOR;
module_param(globalfifo_major, int, S_IRUGO);

struct globalfifo_dev * globalfifo_devp;


/* 
  ** static function list
*/

/********************************************************************************************
* Function:    globalfifo_read
* Description: globalfifo read data
* Input:       filp: struct file
*              size: read data size
*              ppos: pos offset
* Output:      buf: read buffer
* Return:      ssize_t: read data count
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static ssize_t globalfifo_read(struct file * filp, char __user * buf, size_t size, loff_t * ppos)
{
  int ret = 0;
  struct globalfifo_dev *dev = filp->private_data;

  DECLARE_WAITQUEUE(wait, current);

  mutex_lock(&dev->mutex);
  add_wait_queue(&dev->r_wait, &wait);

  while(dev->current_len == 0) {
    if (filp->f_flags & O_NONBLOCK) {
      ret = -EAGAIN;
      goto out;
    }

    __set_current_state(TASK_INTERRUPTIBLE);
    mutex_unlock(&dev->mutex);

    schedule();
    if(signal_pending(current)) {
      ret = -ERESTARTSYS;
      goto out2;
    }

    mutex_lock(&dev->mutex);
  }

  if (size > dev->current_len)
    size = dev->current_len;

  if (copy_to_user(buf, dev->mem, size)) {
    ret = -EFAULT;
    goto out;
  } else {
    memcpy(dev->mem, dev->mem + size, dev->current_len - size);
    dev->current_len -= size;
    log_debug("read %d bytes(s), current_len:%d\n", size, dev->current_len);

    wake_up_interruptible(&dev->w_wait);
    ret = size;
  }

out:
  mutex_unlock(&dev->mutex);

out2:
  remove_wait_queue(&dev->w_wait, &wait);
  set_current_state(TASK_RUNNING);

  return ret;
}


/********************************************************************************************
* Function:    globalfifo_write
* Description: globalfifo write data
* Input:       filp: struct file
*              buf: write buffer
*              size: write data size
*              ppos: pos offset
* Output:      None
* Return:      ssize_t: read data count
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static ssize_t globalfifo_write(struct file * filp, const char __user * buf, size_t size, loff_t * ppos)
{
  int ret = 0;
  struct globalfifo_dev * dev = filp->private_data;

  DECLARE_WAITQUEUE(wait, current);

  mutex_lock(&dev->mutex);
  add_wait_queue(&dev->w_wait, &wait);

  while(dev->current_len == GLOBALFIFO_SIZE) {
    if (filp->f_flags & O_NONBLOCK) {
      ret = -EAGAIN;
      goto out;
    }

    __set_current_state(TASK_INTERRUPTIBLE);

    mutex_unlock(&dev->mutex);
    schedule();

    if (signal_pending(current)) {
      ret = -ERESTARTSYS;
      goto out2;
    }

    mutex_lock(&dev->mutex);
  }

  if (size >= GLOBALFIFO_SIZE - dev->current_len)
    size = GLOBALFIFO_SIZE - dev->current_len;

  if (copy_from_user(dev->mem + dev->current_len, buf, size)) {
    ret = -EFAULT;
    goto out;
  } else {
    dev->current_len += size;
    log_debug("written %u bytes(s), current_len:%d\n", size, dev->current_len);

    wake_up_interruptible(&dev->r_wait);
    ret = size;
  }

out:
  mutex_unlock(&dev->mutex);
out2:
  remove_wait_queue(&dev->w_wait, &wait);
  set_current_state(TASK_RUNNING);

  return ret;
}


/********************************************************************************************
* Function:    globalfifo_llseek
* Description: globalfifo llseek pos
* Input:       filp: struct file
*              offset: pos offse
*              orig: pos flag
* Output:      None
* Return:      ssize_t: read data count
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static loff_t globalfifo_llseek(struct file * filp, loff_t offset, int orig)
{
  loff_t ret = 0;
  switch (orig) {
  case 0:
    if (offset < 0) {
      ret = -EINVAL;
      break;
    }

    if((unsigned int)offset > GLOBALFIFO_SIZE) {
      ret = -EINVAL;
      break;
    }

    filp->f_pos = (unsigned int)offset;
    ret = filp->f_pos;
    break;
  case 1:
    if ((filp->f_pos + offset) > GLOBALFIFO_SIZE) {
      ret = -EINVAL;
      break;
    }  

    if ((filp->f_pos + offset) < 0) {
      ret = -EINVAL;
      break;
    }
    filp->f_pos += offset;
    ret = filp->f_pos;
    break;

  default:
    ret = -EINVAL;
    break;

  }

  return ret;
}


/********************************************************************************************
* Function:    globalfifo_ioctl
* Description: globalfifo ioctl
* Input:       filp: struct file
*              cmd: command
*              arg: argue
* Output:      None
* Return:      ssize_t: read data count
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static long globalfifo_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
  struct globalfifo_dev * dev = filp->private_data;

  switch (cmd)
  {
  case MEM_CLEAR_CMD:
    memset(dev->mem, 0, GLOBALFIFO_SIZE);
    log_debug("globalfifo is set to zero\n");
    break;
  
  default:
    return -EINVAL;
    break;
  }

  return 0;
}


/********************************************************************************************
* Function:    globalfifo_open
* Description: globalfifo open
* Input:       inode: inode 
* Output:      filp: strcut file
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static int globalfifo_open(struct inode * inode, struct file * filp)
{
  filp->private_data = globalfifo_devp;
  return 0;
}


/********************************************************************************************
* Function:    globalfifo_release
* Description: globalfifo relesase
* Input:       inode: inode 
*              filp: strcut file
* Output:      None
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static int globalfifo_release(struct inode * inode, struct file *filp)
{
  return 0;
}


/********************************************************************************************
* Function:    globalfifo_init
* Description: globalfifo initial
* Input:       Noen
* Output:      None
* Return:      0: execute success
*              other: execute failure
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static int __init globalfifo_init(void)
{
    int ret;

    dev_t devno = MKDEV(globalfifo_major, 0);
    
    if (globalfifo_major) 
      ret = register_chrdev_region(devno, 1, "globalfifo");
    else {
      ret = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
      globalfifo_major = MAJOR(devno);
    }

    if (ret < 0) 
      return ret;

    globalfifo_devp = kzalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
    if (!globalfifo_devp) {
      ret = -ENOMEM;
      goto fail_malloc;
    }

    globalfifo_setup_cdev(globalfifo_devp, 0);
    mutex_init(&globalfifo_devp->mutex);
    init_waitqueue_head(&globalfifo_devp->r_wait);
    init_waitqueue_head(&globalfifo_devp->w_wait);

    return 0; 

fail_malloc:
    unregister_chrdev_region(devno, 1);
    return ret;
}


/********************************************************************************************
* Function:    globalfifo_exit
* Description: globalfifo exit
* Input:       Noen
* Output:      None
* Return:      None
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static void __exit globalfifo_exit(void)
{
    cdev_del(&globalfifo_devp->cdev);
    kfree(globalfifo_devp);
    unregister_chrdev_region(MKDEV(globalfifo_major, 0), 1);
}


/********************************************************************************************
* Function:    globalfifo_setup_cdev
* Description: globalfifo setup cdev struct 
* Input:       index: cdev index node
* Output:      dev: initialed cdev 
* Return:      None
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static void globalfifo_setup_cdev(struct globalfifo_dev * dev, int index)
{
  int err, devno = MKDEV(globalfifo_major, index);

  cdev_init(&dev->cdev, &globalfifo_fops);
  dev->cdev.owner = THIS_MODULE;

  err = cdev_add(&dev->cdev, devno, 1);
  if (err) 
    log_debug("Error %d adding globalfifo%d", err, index);

}

/*
  ** module declaration
*/
module_init(globalfifo_init);
module_exit(globalfifo_exit);

MODULE_AUTHOR("JexJiang");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");
MODULE_VERSION("v1.0");


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
