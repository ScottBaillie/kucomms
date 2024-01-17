/**********************************************************/

#include "kucomms_register.h"
#include "kucomms_fops.h"

#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/mm.h>
#include <linux/mutex.h>
#include <linux/cdev.h>

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
#define KUCOMMS_DEVLIST_SIZE 16

/**********************************************************/

static struct kucomms_callback_data cblist[KUCOMMS_CBLIST_SIZE];

static struct kucomms_char_device_data devlist[KUCOMMS_DEVLIST_SIZE];

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

struct kucomms_char_device_data * kucomms_find_device_data(const char* name, __u32 len);

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

	if (len >= KUCOMMS_FNAME_SIZE) goto exit;
	if ((name==0) || (len==0)) goto exit;
	if ((msghlr==0) || (workhlr==0) || (timerhlr==0)) goto exit;
	if (kucomms_find_callback_data(name,len)!=0) goto exit;
	if (kucomms_find_device_data(name,len)==0) goto exit;

	for (__u32 u0=0; u0<KUCOMMS_CBLIST_SIZE; u0++) {
		if (cblist[u0].filename_len != 0) continue;
		cblist[u0].msghlr = msghlr;
		cblist[u0].workhlr = workhlr;
		cblist[u0].timerhlr = timerhlr;
		cblist[u0].userData = userData;
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

void
kucomms_unregister_wait(const char* name, __u32 len)
{
	bool ok;

	while (true) {
		ok = kucomms_unregister(name, len);
		if (ok) break;
		usleep_range(1000000,1500000);
	}
}

/**********************************************************/

struct kucomms_char_device_data *
kucomms_find_device_data(const char* name, __u32 len)
{
	__u32 u1;

	if ((name==0) || (len==0)) return 0;
	for (__u32 u0=0; u0<KUCOMMS_DEVLIST_SIZE; u0++) {
		if (devlist[u0].filename_len == 0) continue;
		if (devlist[u0].filename_len != len) continue;
		for (u1=0; u1<len; u1++) {
			if (devlist[u0].filename[u1] != name[u1]) break;
		}
		if (u1 != len) continue;
		return &devlist[u0];
	}
	return 0;
}

/**********************************************************/

void
kucomms_device_list_init(void)
{
	for (__u32 u0=0; u0<KUCOMMS_DEVLIST_SIZE; u0++) {
		devlist[u0].major = 0;
		devlist[u0].cls = 0;
		devlist[u0].filename_len = 0;
	}
}

/**********************************************************/

bool
kucomms_char_device_create(const char* name, __u32 len)
{
	const char * basename = "kucomms_";

	if ((name==0) || (len==0)) return false;
	if (len >= KUCOMMS_FNAME_SIZE) return false;
	if (len <= strlen(basename)) return false;

	for (__u32 u0=0; u0<strlen(basename); u0++) {
		if (name[u0] != basename[u0]) return false;
	}

	for (__u32 u0=0; u0<len; u0++) {
		if ((name[u0]>='a') && (name[u0]<='z')) continue;
		if (name[u0]=='_') continue;
		if ((name[u0]==0x0a) && (u0==(len-1))) break;
		return false;
	}

	if (name[len-1]==0x0a) len = len - 1;

	int ret = mutex_lock_interruptible(&cblist_mutex);
	if (ret == -EINTR) { // Deal with signal
	}

	if (kucomms_find_device_data(name,len)!=0) goto exit;

	for (__u32 u0=0; u0<KUCOMMS_DEVLIST_SIZE; u0++) {
		if (devlist[u0].filename_len != 0) continue;

		devlist[u0].filename_len = len;
		memcpy(devlist[u0].filename, name, len);
		devlist[u0].filename[len] = 0;

		devlist[u0].major = register_chrdev(0, devlist[u0].filename, &kucomms_file_operations);

		if (devlist[u0].major < 0) {
			devlist[u0].major = 0;
			devlist[u0].filename_len = 0;
			goto exit;
		}

		devlist[u0].cls = class_create(devlist[u0].filename);
		device_create(devlist[u0].cls, NULL, MKDEV(devlist[u0].major, 0), NULL, devlist[u0].filename);

		mutex_unlock(&cblist_mutex);
		return true;
	}

exit:
	mutex_unlock(&cblist_mutex);

	return false;
}

/**********************************************************/

bool
kucomms_char_device_remove(const char* name, __u32 len)
{
	const char * basename = "kucomms_";

	if ((name==0) || (len==0)) return false;
	if (len >= KUCOMMS_FNAME_SIZE) return false;
	if (len <= strlen(basename)) return false;

	for (__u32 u0=0; u0<strlen(basename); u0++) {
		if (name[u0] != basename[u0]) return false;
	}

	for (__u32 u0=0; u0<len; u0++) {
		if ((name[u0]>='a') && (name[u0]<='z')) continue;
		if (name[u0]=='_') continue;
		if ((name[u0]==0x0a) && (u0==(len-1))) break;
		return false;
	}

	if (name[len-1]==0x0a) len = len - 1;

	int ret = mutex_lock_interruptible(&cblist_mutex);
	if (ret == -EINTR) { // Deal with signal
	}
	struct kucomms_char_device_data * pdevdata = 0;

	if (kucomms_find_callback_data(name,len)!=0) goto exit;

	pdevdata = kucomms_find_device_data(name, len);

	if (pdevdata) {
		device_destroy(pdevdata->cls, MKDEV(pdevdata->major, 0));
		class_destroy(pdevdata->cls);
		unregister_chrdev(pdevdata->major, pdevdata->filename);

		pdevdata->major = 0;
		pdevdata->cls = 0;
		pdevdata->filename_len = 0;
	}

exit:
	mutex_unlock(&cblist_mutex);

	return (pdevdata!=0);
}

/**********************************************************/

bool
kucomms_char_device_remove_all(void)
{
	for (__u32 u0=0; u0<KUCOMMS_DEVLIST_SIZE; u0++) {
		if (devlist[u0].filename_len == 0) continue;

		device_destroy(devlist[u0].cls, MKDEV(devlist[u0].major, 0));
		class_destroy(devlist[u0].cls);
		unregister_chrdev(devlist[u0].major, devlist[u0].filename);
	}

	return true;
}

/**********************************************************/

bool
message_queue_add_tx0(
	struct kucomms_file_data * pfd,
	const struct Message * message)
{
	MessageQueueHeaderPtr tx_msgq = pfd->msgmgr.tx_msgq_array[0];
	__u64 tx_msgq_queueLength = pfd->msgmgr.tx_msgq_len_array[0];
	return(message_queue_add_l(tx_msgq, tx_msgq_queueLength, message));
}

__u64
message_queue_get_avail_tx0(
	struct kucomms_file_data * pfd)
{
	MessageQueueHeaderPtr tx_msgq = pfd->msgmgr.tx_msgq_array[0];
	__u64 tx_msgq_queueLength = pfd->msgmgr.tx_msgq_len_array[0];
	return(message_queue_get_avail_l(tx_msgq,tx_msgq_queueLength));
}

__u64
message_queue_get_free_tx0(
	struct kucomms_file_data * pfd)
{
	MessageQueueHeaderPtr tx_msgq = pfd->msgmgr.tx_msgq_array[0];
	__u64 tx_msgq_queueLength = pfd->msgmgr.tx_msgq_len_array[0];
	return(message_queue_get_free_l(tx_msgq,tx_msgq_queueLength));
}

__u64
message_queue_get_avail_rx0(
	struct kucomms_file_data * pfd)
{
	MessageQueueHeaderPtr rx_msgq = pfd->msgmgr.rx_msgq_array[0];
	__u64 rx_msgq_queueLength = pfd->msgmgr.rx_msgq_len_array[0];
	return(message_queue_get_avail_l(rx_msgq,rx_msgq_queueLength));
}

__u64
message_queue_get_free_rx0(
	struct kucomms_file_data * pfd)
{
	MessageQueueHeaderPtr rx_msgq = pfd->msgmgr.rx_msgq_array[0];
	__u64 rx_msgq_queueLength = pfd->msgmgr.rx_msgq_len_array[0];
	return(message_queue_get_free_l(rx_msgq,rx_msgq_queueLength));
}

/**********************************************************/
