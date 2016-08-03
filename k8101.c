#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>			/* kmalloc() */
#include <linux/usb.h>			/* USB stuff */
#include <linux/mutex.h>		/* mutexes */
#include <linux/ioctl.h>
#include <asm/uaccess.h>		/* copy_*_user */
#include <linux/errno.h>
#include <linux/types.h>
#include "k8101_usb.h"
#define VENDOR_ID 0x10cf
#define PRODUCT_ID 0x8101

/* size of the output buffer */
#define BUF_SIZE 0x40
/* allow maximum 256 bytes read */
#define MAX_READ 256
/* bulk out address */
#define BULK_OUT_ADDRESS 0x02

/* struct holding all of our device specific stuff */
struct usb_k8101_data {
    /* One structure for each connected device */
	struct usb_device* udev;					/* For saving the usb device pointer */
	u8* out_buffer;								/* output buffer */
	u8* in_buffer;								/* input buffer (from user-space not from device)*/
	u8 minor;									/* minor number */
	int is_open;								/* whether the port is open */
	int is_present;								/* whether the device is present */
	struct mutex lock;							/* race conditions avoidance */
};
/* Prevent races between open() and disconnect */
static DEFINE_MUTEX(open_disc_mutex);


static struct usb_device_id k8101_table [] = {
    { USB_DEVICE(VENDOR_ID, PRODUCT_ID) },
    { }
};

/* local function prototypes */
static int k8101_probe(struct usb_interface* interface, const struct usb_device_id* id);
static void k8101_disconnect(struct usb_interface *interface);
static int k8101_open(struct inode* inode, struct file* file);
static int k8101_release(struct inode* inode, struct file* file);
static ssize_t k8101_write(struct file* file, const char __user* user_buf, size_t count, loff_t* ppos);
	
static struct usb_driver k8101_driver = {
    .name = "k8101-usb-message-board",
    .id_table = k8101_table,
    .probe = k8101_probe,
    .disconnect = k8101_disconnect,
};

MODULE_DEVICE_TABLE (usb, k8101_table);

/* file operation pointers */

static struct file_operations k8101_fops = {
    .owner =    THIS_MODULE,
    .open =     k8101_open,
    .write =    k8101_write,
    .release =  k8101_release,
};

/* class driver information */
static struct usb_class_driver k8101_class = {
	.name = "msgboard%d",
	.fops = &k8101_fops,
	.minor_base = 0,
};

static int __init usb_k8101_init(void) {
    /* called on module loading */
	int retval;
	retval = usb_register(&k8101_driver);
	if (retval) {
		printk(KERN_ERR "k8101 driver registration failed");
		return retval;
	}
	printk(KERN_INFO "k8101 driver registration successful");
	return retval;
}

static void __exit usb_k8101_exit(void) {
    /* called on module unloading */
	usb_deregister(&k8101_driver);
	printk(KERN_INFO "k8101 driver deregistered");
}

static inline void k8101_delete(struct usb_k8101_data* dev) {
	if(!dev)
		return;
	kfree(dev->out_buffer);
	kfree(dev->in_buffer);
	kfree(dev);
}
static int k8101_probe(struct usb_interface* interface, const struct usb_device_id* id) {
    /* called when a USB device is connected to the computer. */
	struct usb_device* udev = interface_to_usbdev(interface);
	struct usb_k8101_data* dev = NULL;
	int retval;
	/* we do not use the interrupt interface, so ignore it */
	if (interface->cur_altsetting->desc.bNumEndpoints < 2) {
		printk(KERN_INFO "ignoring Interrupt interface");
		return -1;
	}
	dev = kzalloc(sizeof(struct usb_k8101_data), GFP_KERNEL);
	if (dev == NULL)
		return -ENOMEM;
	dev->udev = udev;
	dev->out_buffer = kzalloc(BUF_SIZE, GFP_KERNEL);
	if (!dev->out_buffer ) {
		k8101_delete(dev);
		return -ENOMEM;
	}
	dev->in_buffer = kzalloc(MAX_READ, GFP_KERNEL);
	if (!dev->in_buffer ) {
		k8101_delete(dev);
		return -ENOMEM;
	}
	retval = usb_register_dev(interface, &k8101_class);
	if (retval) {
		printk(KERN_ERR "not able to get a minor for this device");
		k8101_delete(dev);
		return retval;
	}
	dev->is_present = 1;
	mutex_init(&(dev->lock));
	printk(KERN_INFO "K8101 USB Message Board found at address %d is now attached to major %d minor %d\n", dev->udev->devnum, USB_MAJOR, dev->minor);
	usb_set_intfdata(interface, dev);
	return 0;
}

static void k8101_disconnect(struct usb_interface* interface) {
    /* called when unplugging a USB device. */
	struct usb_k8101_data* dev = usb_get_intfdata(interface);
	/*prevent an open call or wait for one to complete*/
	mutex_lock(&open_disc_mutex);
	usb_set_intfdata(interface, NULL);
	/* give back minor */
	usb_deregister_dev(interface, &k8101_class);
	if (!dev) {
		/* this should actually never happen */
		printk(KERN_INFO "IT ACTUALLY HAPPENED");
		mutex_unlock(&open_disc_mutex);
		return;
	}
	/* lock the device */
	mutex_lock(&dev->lock);
	dev->is_present = 0;
	printk(KERN_INFO "K8101 has been disconnected");
	/* if device is open k8101_release will take care cleaning up */
	if (dev->is_open) {
		mutex_unlock(&dev->lock);
		mutex_unlock(&open_disc_mutex);
		return;
	}
	mutex_unlock(&dev->lock);
	k8101_delete(dev);
	mutex_unlock(&open_disc_mutex);
}

static int k8101_open(struct inode* inode, struct file* file) {
    /* open syscall */
	struct usb_k8101_data* dev;
	struct usb_interface* interface = usb_find_interface(&k8101_driver, iminor(inode));
	if (!interface)
		return -ENODEV;
	/* against disconnect */
	mutex_lock(&open_disc_mutex);
	dev = usb_get_intfdata(interface);
	if (!dev) {
		mutex_unlock(&open_disc_mutex);
		return -ENODEV;
	}
	/* lock the device */
	mutex_lock(&dev->lock);
	if (dev->is_open || !dev->is_present) {
		/* device already open or disconnected*/
		mutex_unlock(&dev->lock);
		mutex_unlock(&open_disc_mutex);
		return -EBUSY;
	}
	dev->is_open = 1;
	mutex_unlock(&dev->lock);
	/* save object in the file's private structure */
	file->private_data = dev;
	printk(KERN_INFO "K8101 opened");
	mutex_unlock(&open_disc_mutex);
	return 0;
}

static ssize_t k8101_write(struct file* file, const char __user* user_buf, size_t count, loff_t* ppos) {
    /* write syscall */
	struct usb_k8101_data* dev;
	int written_size, result, write_size, thistime;
	int maxretry = 5;
	dev = file->private_data;
	if (count <= 0)
		return 0;
	mutex_lock(&dev->lock);
	/* device unplugged */
	if (!dev->is_present) {
		mutex_unlock(&dev->lock);
		return -ENODEV;
	}
	if (count > MAX_READ) {
		mutex_unlock(&dev->lock);
		return -EPERM;
	}
	if (copy_from_user(dev->in_buffer, user_buf, count)) {
		mutex_unlock(&dev->lock);
		return -EFAULT;
	}
	switch(dev->in_buffer[0]) {
		case 0: write_size = connect(dev->out_buffer);
			break;
		case 1: write_size = draw_pixel(dev->out_buffer, dev->in_buffer[1], dev->in_buffer[2]);
			break;
		case 2: write_size = draw_line(dev->out_buffer, dev->in_buffer[1], dev->in_buffer[2], dev->in_buffer[3], dev->in_buffer[4]);
			break;
		case 3: write_size = draw_square(dev->out_buffer, dev->in_buffer[1], dev->in_buffer[2], dev->in_buffer[3], dev->in_buffer[4]);
			break;
		case 4: write_size = write_date(dev->out_buffer, dev->in_buffer + 1, count - 1);
			break;
		case 5: write_size = buzz(dev->out_buffer, dev->in_buffer[1]);
			break;
		case 6: write_size = invert_screen(dev->out_buffer, dev->in_buffer[1]);
			break;
		case 7: write_size = clear_screen(dev->out_buffer);
			break;
		default: return -EPERM;
	}
	do {
		thistime = (write_size >= BUF_SIZE) ? BUF_SIZE : write_size;
		while (thistime) {
			result = usb_bulk_msg(dev->udev, usb_sndbulkpipe(dev->udev, BULK_OUT_ADDRESS),
								  dev->out_buffer, thistime, &written_size, 10000);
			if (result == -ETIMEDOUT) {
				if (!maxretry--) {
					mutex_unlock(&dev->lock);
					return -ETIME;
				}
				continue;
			}
			else if (!result && written_size) {
				dev->out_buffer += written_size;
				thistime -= written_size;
				write_size -= written_size;
			}
			else
				break;
		}
		if (result) {
			mutex_unlock(&dev->lock);
			return -EIO;
		}
	} while(write_size);
	printk(KERN_INFO "Wrote %lu bytes to k8101 device\n", count);
	mutex_unlock(&dev->lock);
	return count;
}

static int k8101_release(struct inode* inode, struct file* file) {
    /* close syscall */
	struct usb_k8101_data* dev;
	dev = file->private_data;
	if (!dev)
		return -ENODEV;
	/* lock the device */
	mutex_lock(&dev->lock);
	if (dev->is_open == 0) {
		/*already closed */
		mutex_unlock(&dev->lock);
		return -ENODEV;
	}
	dev->is_open = 0;
	if (!dev->is_present) {
		/* device was unplugged before the file was released */
		mutex_unlock(&dev->lock);
		k8101_delete(dev);
	}
	else {
		mutex_unlock(&dev->lock);
	}
	return 0;
}

module_init(usb_k8101_init);
module_exit(usb_k8101_exit);
MODULE_AUTHOR("Carlos Manrique");
MODULE_LICENSE("GPL");

