/**********************************************************/

#include "kucomms_register.h"

#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/mutex.h>

//#include <linux/errno.h>
//#include <linux/mmdebug.h>
//#include <linux/gfp.h>
//#include <linux/bug.h>
//#include <linux/list.h>
//#include <linux/mmzone.h>
//#include <linux/rbtree.h>
//#include <linux/atomic.h>
//#include <linux/debug_locks.h>
//#include <linux/mm_types.h>
//#include <linux/mmap_lock.h>
//#include <linux/range.h>
//#include <linux/pfn.h>
//#include <linux/percpu-refcount.h>
//#include <linux/bit_spinlock.h>
//#include <linux/shrinker.h>
//#include <linux/resource.h>
//#include <linux/page_ext.h>
//#include <linux/err.h>
//#include <linux/page-flags.h>
//#include <linux/page_ref.h>
//#include <linux/overflow.h>
//#include <linux/sizes.h>
//#include <linux/sched.h>
//#include <linux/pgtable.h>
//#include <linux/kasan.h>
//#include <linux/memremap.h>
//#include <linux/slab.h>

/**********************************************************/

#define KUCOMMS_CBLIST_SIZE 16

/**********************************************************/

static struct kucomms_callback_data cblist[KUCOMMS_CBLIST_SIZE];

struct mutex cblist_mutex;

/**********************************************************/

void
kucomms_callback_list_init(void)
{
	for (__u32 u0=0; u0<KUCOMMS_CBLIST_SIZE; u0++) {
		cblist[u0].msghlr = 0;
		cblist[u0].workhlr = 0;
		cblist[u0].timerhlr = 0;
		cblist[u0].userData = 0;
		cblist[u0].filename_len = 0;
		cblist[u0].open = false;
	}

	mutex_init(&cblist_mutex);
}

/**********************************************************/

struct kucomms_callback_data *
kucomms_find_callback_data(const char* name, __u32 len)
{
	__u32 u1;

	if ((name==0) || (len==0)) return 0;
	for (__u32 u0=0; u0<KUCOMMS_CBLIST_SIZE; u0++) {
		if (cblist[u0].filename_len == 0) continue;
		if (cblist[u0].filename_len != len) continue;
		for (u1=0; u1<len; u1++) {
			if (cblist[u0].filename[u1] != name[u1]) break;
		}
		if (u1 != len) continue;
		return &cblist[u0];
	}
	return 0;
}

/**********************************************************/

struct kucomms_callback_data *
kucomms_find_and_open(const char* name, __u32 len, bool * open)
{
	int ret = mutex_lock_interruptible(&cblist_mutex);
	if (ret == -EINTR) { // Deal with signal
	}

	struct kucomms_callback_data * pcbdata;
	*open = false;
	pcbdata = kucomms_find_callback_data(name, len);
	if (pcbdata) {
		*open = pcbdata->open;
		pcbdata->open = true;
	}

	mutex_unlock(&cblist_mutex);

	return pcbdata;
}

/**********************************************************/

void
kucomms_find_and_close(const char* name, __u32 len)
{
	int ret = mutex_lock_interruptible(&cblist_mutex);
	if (ret == -EINTR) { // Deal with signal
	}

	struct kucomms_callback_data * pcbdata;
	pcbdata = kucomms_find_callback_data(name, len);
	if (pcbdata) {
		pcbdata->open = false;
	}

	mutex_unlock(&cblist_mutex);
}

/**********************************************************/

bool
kucomms_register(
	const char* name,
	__u32 len,
	MessageHandler_C msghlr,
	WorkHandler_C workhlr,
	TimerHandler_C timerhlr,
	void * userData)
{
	int ret = mutex_lock_interruptible(&cblist_mutex);
	if (ret == -EINTR) { // Deal with signal
	}

	if ((name==0) || (len==0)) goto exit;
	if ((msghlr==0) || (workhlr==0) || (timerhlr==0)) goto exit;
	if (kucomms_find_callback_data(name,len)!=0) goto exit;

	for (__u32 u0=0; u0<KUCOMMS_CBLIST_SIZE; u0++) {
		if (cblist[u0].filename_len != 0) continue;
		cblist[u0].msghlr = msghlr;
		cblist[u0].workhlr = workhlr;
		cblist[u0].timerhlr = timerhlr;
		cblist[u0].userData = userData;
		if (len > KUCOMMS_FNAME_SIZE) len = KUCOMMS_FNAME_SIZE;
		cblist[u0].filename_len = len;
		memcpy(cblist[u0].filename, name, len);
		mutex_unlock(&cblist_mutex);
		return true;
	}

exit:
	mutex_unlock(&cblist_mutex);

	return false;
}

/**********************************************************/

bool
kucomms_unregister(const char* name, __u32 len)
{
	int ret = mutex_lock_interruptible(&cblist_mutex);
	if (ret == -EINTR) { // Deal with signal
	}

	struct kucomms_callback_data * pcbdata = kucomms_find_callback_data(name,len);
	if (pcbdata) {
		if (pcbdata->open) {
			mutex_unlock(&cblist_mutex);
			return false;
		}
		pcbdata->msghlr = 0;
		pcbdata->workhlr = 0;
		pcbdata->timerhlr = 0;
		pcbdata->userData = 0;
		pcbdata->filename_len = 0;
		pcbdata->open = false;
	}

	mutex_unlock(&cblist_mutex);

	return true;
}

/**********************************************************/
