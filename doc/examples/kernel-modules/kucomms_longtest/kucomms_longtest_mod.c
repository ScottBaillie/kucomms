/**********************************************************/

#include "kucomms_register.h"

#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/time.h>

/**********************************************************/

static bool
kucomms_message_hlr(const struct Message * message, MessageQueueHeaderPtr tx_msgq, const __u64 rx_msgq_queueLength, const __u64 tx_msgq_queueLength, void * userData)
{
//	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	return true;
}

/**********************************************************/

static bool
kucomms_work_hlr(void * userData)
{
//	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	return false;
}

/**********************************************************/

static void
kucomms_timer_hlr(const __u64 time, void * userData)
{
//	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
}

/**********************************************************/

const char * devname = "kucomms_longtest";

/**********************************************************/

static int __init init_kucomms_longtest(void)
{
	bool ok;

	ok = kucomms_register(
		devname,
		strlen(devname),
		kucomms_message_hlr,
		kucomms_work_hlr,
		kucomms_timer_hlr,
		&g_data);

	if (!ok) return -ENODEV;

	return 0;
}

/**********************************************************/

static void __exit exit_kucomms_longtest(void)
{
	kucomms_unregister_wait(devname, strlen(devname));
}

/**********************************************************/

module_init(init_kucomms_longtest);
module_exit(exit_kucomms_longtest);

/**********************************************************/

MODULE_LICENSE("GPL"); 
