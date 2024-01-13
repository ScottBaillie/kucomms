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

static bool
kucomms_test_work_hlr(void * userData)
{
	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	struct kucomms_test_data * pdata = (struct kucomms_test_data*)pfd->cbdata.userData;

	pdata->var2 = 0;

	return true;
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
