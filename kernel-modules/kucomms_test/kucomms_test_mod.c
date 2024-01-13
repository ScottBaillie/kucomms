/**********************************************************/

#include "kucomms_register.h"

#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/module.h>
#include <linux/printk.h>

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
kucomms_test_message_hlr(struct Message * message, MessageQueueHeaderPtr tx_msgq, void * userData)
{
	pr_info("kucomms_test_message_hlr : Entered\n");

	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	struct kucomms_test_data * pdata = (struct kucomms_test_data*)pfd->cbdata.userData;

	pdata->var1 = 0;

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
	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	struct kucomms_test_data * pdata = (struct kucomms_test_data*)pfd->cbdata.userData;

	pdata->var2 = 0;

	return false;
}

/**********************************************************/

static void
kucomms_test_timer_hlr(void * userData)
{
	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	struct kucomms_test_data * pdata = (struct kucomms_test_data*)pfd->cbdata.userData;

	pdata->var3 = 0;
}

/**********************************************************/

static int __init init_kucomms_test(void)
{
	bool ok;
	pr_info("init_kucomms_test : Entered\n");

	ok = kucomms_register(
		"kucomms1",
		8,
		kucomms_test_message_hlr,
		kucomms_test_work_hlr,
		kucomms_test_timer_hlr,
		&g_data);

	return 0;
}

/**********************************************************/

static void __exit exit_kucomms_test(void)
{
	bool ok;
	pr_info("exit_kucomms_test : Exiting\n");

	ok = kucomms_unregister("kucomms", 7);
}

/**********************************************************/

module_init(init_kucomms_test);
module_exit(exit_kucomms_test);

/**********************************************************/

MODULE_LICENSE("GPL"); 
