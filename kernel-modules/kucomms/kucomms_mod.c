/**********************************************************/

#include "kucomms_register.h"
#include "kucomms_fops.h"

#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/printk.h>

/**********************************************************/

#define CHRDEV_DEVICE_NAME "kucomms1"

/**********************************************************/

static int major;
static struct class *cls;

/**********************************************************/

static int __init init_kucomms(void)
{
	pr_info("init_kucomms : Entered\n");

	major = register_chrdev(0, CHRDEV_DEVICE_NAME, &kucomms_file_operations);

	if (major < 0) {
		pr_alert("init_kucomms : Registering char device failed with %d\n", major);
		return major;
	}

	cls = class_create(CHRDEV_DEVICE_NAME);
	device_create(cls, NULL, MKDEV(major, 0), NULL, CHRDEV_DEVICE_NAME);

	kucomms_callback_list_init();

	return 0;
}

/**********************************************************/

static void __exit exit_kucomms(void)
{
	pr_info("exit_kucomms : Exiting\n");

	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	unregister_chrdev(major, CHRDEV_DEVICE_NAME);
}

/**********************************************************/

module_init(init_kucomms);
module_exit(exit_kucomms);

/**********************************************************/

EXPORT_SYMBOL(kucomms_register);
EXPORT_SYMBOL(kucomms_unregister);

/**********************************************************/

MODULE_LICENSE("GPL"); 
