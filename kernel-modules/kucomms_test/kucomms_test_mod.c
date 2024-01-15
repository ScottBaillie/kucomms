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
kucomms_test_message_hlr(struct Message * message, MessageQueueHeaderPtr tx_msgq, const __u64 rx_msgq_queueLength, const __u64 tx_msgq_queueLength, void * userData)
{
	pr_info("kucomms_test_message_hlr : Entered : message length=%llu\n", message->m_length);

//	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
//	struct kucomms_test_data * pdata = (struct kucomms_test_data*)pfd->cbdata.userData;


	//  send the message received back to the sender
	message_queue_add_l(tx_msgq, tx_msgq_queueLength, message);


	return true;
}

/**********************************************************/

//
// A work handler should return false if there is no work do.
// A work handler should return true if it knows that there is more work to do.
// A work handler should return false if it executes very quickly.
// If a work handler sleeps for more than a millisecond then it should return true.
// A work handler should not take longer than a millisecond to execute if possible.
//
// The work handler is scheduled as often as possible so if it returns true all
// of the time and there is no sleeping then a lot of CPU will be used.
// A sleep for a millisecond will occur when the work handler returns false.
//
static bool
kucomms_test_work_hlr(void * userData)
{
//	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
//	struct kucomms_test_data * pdata = (struct kucomms_test_data*)pfd->cbdata.userData;
	return false;
}

/**********************************************************/

static void
kucomms_test_timer_hlr(void * userData)
{
//	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
//	struct kucomms_test_data * pdata = (struct kucomms_test_data*)pfd->cbdata.userData;
}

/**********************************************************/

const char * devname = "kucomms_test";

/**********************************************************/

static int __init init_kucomms_test(void)
{
	bool ok;
	pr_info("init_kucomms_test : Entered\n");

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
	bool ok;

	while (true) {
		ok = kucomms_unregister(devname, strlen(devname));
		if (ok) break;
		usleep_range(1000000,1500000);
	}

	pr_info("exit_kucomms_test : Exiting\n");
}

/**********************************************************/

module_init(init_kucomms_test);
module_exit(exit_kucomms_test);

/**********************************************************/

MODULE_LICENSE("GPL"); 
