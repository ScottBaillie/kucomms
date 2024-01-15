/**********************************************************/

#include "kucomms_register.h"

#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/platform_device.h>

/**********************************************************/

#define KUCOMMS_PLATFORM_DEVICE_NAME "kucomms"

/**********************************************************/

static int major;
static struct class *cls;

/**********************************************************/

static struct platform_device kucomms_platform_device = {
	.name = KUCOMMS_PLATFORM_DEVICE_NAME,
	.id = -1,
};

/**********************************************************/

//show() methods should return the number of bytes printed into the buffer.
// show() or store() can always return errors. 
ssize_t create_device_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "none\n");
}

// store() should return the number of bytes used from the buffer. If the entire buffer has been used, just return the count argument.
// show() or store() can always return errors. 
ssize_t create_device_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	bool ok = kucomms_char_device_create(buf, strlen(buf));
	if (!ok) return(-1);

	return strlen(buf);
}

ssize_t remove_device_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "none\n");
}

ssize_t remove_device_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	bool ok = kucomms_char_device_remove(buf, strlen(buf));
	if (!ok) return(-1);

	return strlen(buf);
}

/**********************************************************/

static struct device_attribute createdev_device_attribute = {
        .attr = {
                .name = "create_device",
                .mode = S_IWUSR | S_IRUGO,
        },
        .show = create_device_show,
        .store = create_device_store,
};

static struct device_attribute removedev_device_attribute = {
        .attr = {
                .name = "remove_device",
                .mode = S_IWUSR | S_IRUGO,
        },
        .show = remove_device_show,
        .store = remove_device_store,
};

/**********************************************************/

static int __init init_kucomms(void)
{
	struct device * dev;
	int ret;

	major = platform_device_register(&kucomms_platform_device);

	if (major < 0) {
		pr_alert("init_kucomms : Registering char device failed with %d\n", major);
		return major;
	}

	cls = class_create(KUCOMMS_PLATFORM_DEVICE_NAME);
	dev = device_create(cls, NULL, MKDEV(major, 0), NULL, KUCOMMS_PLATFORM_DEVICE_NAME);

//	/sys/devices/virtual/kucomms/kucomms/create_device
	ret = device_create_file(dev, &createdev_device_attribute);

	if (ret) {
		pr_alert("init_kucomms : Error from device_create_file()\n");
		return -ENODEV;
	}

//	/sys/devices/virtual/kucomms/kucomms/remove_device
	ret = device_create_file(dev, &removedev_device_attribute);

	if (ret) {
		pr_alert("init_kucomms : Error from device_create_file()\n");
		device_remove_file(dev, &createdev_device_attribute);
		return -ENODEV;
	}

	kucomms_callback_list_init();
	kucomms_device_list_init();

	return 0;
}

/**********************************************************/

static void __exit exit_kucomms(void)
{
	device_destroy(cls, MKDEV(major, 0));
	class_destroy(cls);
	platform_device_unregister(&kucomms_platform_device);

	kucomms_char_device_remove_all();
}

/**********************************************************/

module_init(init_kucomms);
module_exit(exit_kucomms);

/**********************************************************/

EXPORT_SYMBOL(kucomms_register);
EXPORT_SYMBOL(kucomms_unregister_wait);

/**********************************************************/

MODULE_LICENSE("GPL"); 
