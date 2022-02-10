/*
  ** @file           : drv_globalmen.c
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


/*
  ** define
*/
#define     GLOBALMEM_SIZE          (0x1000)
#define     MEM_CLEAR_CMD           (0x1)
#define     GLOBALMEM_MAJOR         (230)

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
struct globalmem_dev {
  struct cdev cdev;
  unsigned char mem[GLOBALMEM_SIZE];
};


/*
  ** static function declaration
*/
static ssize_t globalmem_read(struct file * filp, char __user * buf, size_t size, loff_t * ppos);
static ssize_t globalmem_write(struct file * filp, const char __user * buf, size_t size, loff_t * ppos);
static loff_t globalmem_llseek(struct file * filp, loff_t offset, int orig);
static long globalmem_ioctl(struct file * filp, unsigned int cmd, unsigned long arg);
static int globalmem_open(struct inode * inode, struct file * filp);
static int globalmem_release(struct inode * inode, struct file *filp);
static void globalmem_setup_cdev(struct globalmem_dev * dev, int index);


/*
  ** global variable
*/
static const struct file_operations globalmem_fops = {
  .owner = THIS_MODULE,
  .llseek = globalmem_llseek,
  .read = globalmem_read,
  .write = globalmem_write,
  .unlocked_ioctl = globalmem_ioctl,
  .open = globalmem_open,
  .release = globalmem_release,
};


/*
  ** static global variable
*/
static int globalmem_major = GLOBALMEM_MAJOR;
module_param(globalmem_major, int, S_IRUGO);

struct globalmem_dev * globalmem_devp;


/* 
  ** static function list
*/

/********************************************************************************************
* Function:    globalmem_read
* Description: globalmem read data
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
static ssize_t globalmem_read(struct file * filp, char __user * buf, size_t size, loff_t * ppos)
{
  unsigned long p = *ppos;
  unsigned int count = size;
  int ret = 0;
  struct globalmem_dev *dev = filp->private_data;

  if (p >= GLOBALMEM_SIZE)
    return 0;
  if (count > GLOBALMEM_SIZE - p)
    count = GLOBALMEM_SIZE - p;

  if (copy_to_user(buf, dev->mem + p, count)) {
    ret = -EFAULT;
  } else {
      *ppos += count;
      ret = count;

      log_debug("read %u bytes(s) from %lu\n", count, p);
  }

  return ret;
}


/********************************************************************************************
* Function:    globalmem_write
* Description: globalmem write data
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
static ssize_t globalmem_write(struct file * filp, const char __user * buf, size_t size, loff_t * ppos)
{
  unsigned long p = *ppos;
  unsigned int count = size;
  int ret = 0;
  struct globalmem_dev * dev = filp->private_data;
  
  if (p >= GLOBALMEM_SIZE)
    return 0;
  if (count > GLOBALMEM_SIZE - p)
    count = GLOBALMEM_SIZE - p;

  if (copy_from_user(dev->mem + p, buf, count))
    ret = -EFAULT;
  else {
    * ppos += count;
    ret = count;

    log_debug("written %u bytes(s) from %lu\n", count, p);
  }

  return ret;
}


/********************************************************************************************
* Function:    globalmem_llseek
* Description: globalmem llseek pos
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
static loff_t globalmem_llseek(struct file * filp, loff_t offset, int orig)
{
  loff_t ret = 0;
  switch (orig) {
  case 0:
    if (offset < 0) {
      ret = -EINVAL;
      break;
    }

    if((unsigned int)offset > GLOBALMEM_SIZE) {
      ret = -EINVAL;
      break;
    }

    filp->f_pos = (unsigned int)offset;
    ret = filp->f_pos;
    break;
  case 1:
    if ((filp->f_pos + offset) > GLOBALMEM_SIZE) {
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
* Function:    globalmem_ioctl
* Description: globalmem ioctl
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
static long globalmem_ioctl(struct file * filp, unsigned int cmd, unsigned long arg)
{
  struct globalmem_dev * dev = filp->private_data;

  switch (cmd)
  {
  case MEM_CLEAR_CMD:
    memset(dev->mem, 0, GLOBALMEM_SIZE);
    log_debug("globalmem is set to zero\n");
    break;
  
  default:
    return -EINVAL;
    break;
  }

  return 0;
}


/********************************************************************************************
* Function:    globalmem_open
* Description: globalmem open
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
static int globalmem_open(struct inode * inode, struct file * filp)
{
  filp->private_data = globalmem_devp;
  return 0;
}


/********************************************************************************************
* Function:    globalmem_release
* Description: globalmem relesase
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
static int globalmem_release(struct inode * inode, struct file *filp)
{
  return 0;
}


/********************************************************************************************
* Function:    globalmem_init
* Description: globalmem initial
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
static int __init globalmem_init(void)
{
    int ret;

    dev_t devno = MKDEV(globalmem_major, 0);
    
    if (globalmem_major) 
      ret = register_chrdev_region(devno, 1, "globalmem");
    else {
      ret = alloc_chrdev_region(&devno, 0, 1, "globalmem");
      globalmem_major = MAJOR(devno);
    }

    if (ret < 0) 
      return ret;

    globalmem_devp = kzalloc(sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!globalmem_devp) {
      ret = -ENOMEM;
      goto fail_malloc;
    }

    globalmem_setup_cdev(globalmem_devp, 0);
    return 0; 

fail_malloc:
    unregister_chrdev_region(devno, 1);
    return ret;
}


/********************************************************************************************
* Function:    globalmem_exit
* Description: globalmem exit
* Input:       Noen
* Output:      None
* Return:      None
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static void __exit globalmem_exit(void)
{
    cdev_del(&globalmem_devp->cdev);
    kfree(globalmem_devp);
    unregister_chrdev_region(MKDEV(globalmem_major, 0), 1);
}


/********************************************************************************************
* Function:    globalmem_setup_cdev
* Description: globalmem setup cdev struct 
* Input:       index: cdev index node
* Output:      dev: initialed cdev 
* Return:      None
* Others:      
* Revision history:
             1.Date:     2022-1-24
               Author:   JexJiang
               Modification: Function created

********************************************************************************************/
static void globalmem_setup_cdev(struct globalmem_dev * dev, int index)
{
  int err, devno = MKDEV(globalmem_major, index);

  cdev_init(&dev->cdev, &globalmem_fops);
  dev->cdev.owner = THIS_MODULE;

  err = cdev_add(&dev->cdev, devno, 1);
  if (err) 
    log_debug("Error %d adding globalmem%d", err, index);

}

/*
  ** module declaration
*/
module_init(globalmem_init);
module_exit(globalmem_exit);

MODULE_AUTHOR("JexJiang");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple Hello World Module");
MODULE_ALIAS("a simplest module");
MODULE_VERSION("v1.0");


/*
  ** (C) COPYRIGHT ShangHaiHeQian END OF FILE
*/
