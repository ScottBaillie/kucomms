/**********************************************************/

#include <kucomms/kucomms_register.h>

#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/delay.h>
#include <linux/time.h>

/**********************************************************/

struct kucomms_test_data
{
	__u32 var1;
	__u32 var2;
	__u32 var3;
};

/**********************************************************/

static struct kucomms_test_data g_data;

/**********************************************************/

static bool
kucomms_test_message_hlr(const struct Message * message, MessageQueueHeaderPtr tx_msgq, const __u64 rx_msgq_queueLength, const __u64 tx_msgq_queueLength, void * userData)
{
	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
//	struct kucomms_test_data * pdata = (struct kucomms_test_data*)pfd->cbdata.userData;


	//  send the message received back to the sender
	message_queue_add_tx0(pfd, message);

	return true;
}

/**********************************************************/

static bool
kucomms_test_work_hlr(void * userData)
{
//	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	return false;
}

/**********************************************************/

static void
kucomms_test_timer_hlr(const __u64 time, void * userData)
{
//	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
}

/**********************************************************/

const char * devname = "kucomms_test";

/**********************************************************/

static int __init init_kucomms_test(void)
{
	bool ok;

	ok = kucomms_register(
		devname,
		strlen(devname),
		kucomms_test_message_hlr,
		kucomms_test_work_hlr,
		kucomms_test_timer_hlr,
		&g_data);

	if (!ok) return -ENODEV;

	return 0;
}

/**********************************************************/

static void __exit exit_kucomms_test(void)
{
	kucomms_unregister_wait(devname, strlen(devname));
}

/**********************************************************/

module_init(init_kucomms_test);
module_exit(exit_kucomms_test);

/**********************************************************/

MODULE_LICENSE("GPL"); 
