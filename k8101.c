#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>			/* kmalloc() */
#include <linux/usb.h>			/* USB stuff */
#include <linux/mutex.h>		/* mutexes */
#include <linux/ioctl.h>
#include <asm/uaccess.h>		/* copy_*_user */

#define VENDOR_ID 0x10cf
#define PRODUCT_ID 0x8101

/* struct holding all of our device specific stuff */
struct usb_k8101_data {
    /* One structure for each connected device */
	struct usb_device* k8101_dev;				/* For saving the usb device pointer */
	car* out_buffer, in_buffer;					/* input and output buffers */
	int is_open;								/* whether the port is open */
	int is_present;								/* whether the device is present */
	struct mutex lock;							/* race conditions avoidance */
};
/* Prevent races between open() and disconnect */
static DEFINE_MUTEX(disconnect_mutex);

static struct usb_driver k8101_driver = {
    .name = "k8101 usb message board",
    .id_table = k8101_table,
    .probe = k8101_probe,
    .disconnect = k8101_disconnect,
};

static struct usb_device_id k8101_table [] = {
    { USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
    { }
};

MODULE_DEVICE_TABLE (usb, k8101_table);

/* file operation pointers */
static const struct k8101_fops = {
	.owner = THIS_MODULE,
	.write = k8101_write,
	.open = k8101_open,
	.release = k8101_k8101_release,
};

/* class driver information */
static struct usb_class_driver k8101_class = {
	.name = "k8101msgboard%d",
	.fops = &k8101_fops,
	.minor_base = 0,
};

static int __init usb_k8101_init(void) {
    /* called on module loading */
	int retval;
	retval = usb_register(&k8101_driver);
	if (retval) {
		printk(KERN_INFO "k8101 driver registration failed");
		return retval;
	}
	printk(KERN_INFO "k8101 driver registration successful");
	return 0;
}

static void __exit usb_k8101_exit(void) {
    /* called on module unloading */
	usb_deregister(&k8101_driver);
	printk(KERN_INFO "k8101 driver deregistered");
}

static int k8101_open(struct inode *inode, struct file *file)
{
    /* open syscall */
}
static int k8101_release(struct inode *inode, struct file *file)
{
    /* close syscall */
}

static ssize_t k8101_write(struct file *file, const char __user *user_buf, size_t
        count, loff_t *ppos);
{
    /* write syscall */
}

static struct file_operations k8101_fops = {
    .owner =    THIS_MODULE,
    .write =    k8101_write,
    .open =     k8101_open,
    .release =  k8101_release,
};

static int k8101_probe(struct usb_interface *interface, const struct usb_device_id
        *id)
{
    /* called when a USB device is connected to the computer. */
}

static void k8101_disconnect(struct usb_interface *interface)
{
    /* called when unplugging a USB device. */
}

module_init(usb_k8101_init);
module_exit(usb_k8101_exit);
MODULE_AUTHOR("Carlos Manrique");
MODULE_LICENSE("GPL");
