/**********************************************************/

#include "kucomms_register.h"
#include "kucomms_fops.h"

#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/vmalloc.h>

//#include <linux/errno.h>
//#include <linux/mmdebug.h>
//#include <linux/gfp.h>
//#include <linux/bug.h>
//#include <linux/list.h>
//#include <linux/mmzone.h>
//#include <linux/rbtree.h>
//#include <linux/atomic.h>
//#include <linux/debug_locks.h>
#include <linux/delay.h>
#include <linux/mm.h>
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

bool
kucomms_message_hlr(struct Message * message, MessageQueueHeaderPtr tx_msgq, const __u64 rx_msgq_queueLength, const __u64 tx_msgq_queueLength, void * userData)
{
	bool ok = false;
	__u64 length = message->m_length;
	__u64 messageSize = length + sizeof(struct Message);

	if (messageSize >= rx_msgq_queueLength) {
		return false;
	}

	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;

	memcpy(pfd->cbdata.message,message,messageSize);

	pfd->cbdata.message->m_length = length;

	if (pfd->cbdata.msghlr) ok = pfd->cbdata.msghlr(pfd->cbdata.message, tx_msgq, rx_msgq_queueLength, tx_msgq_queueLength, userData);
	return ok;
}

/**********************************************************/

bool
kucomms_work_hlr(void * userData)
{
	bool ok = false;
	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	if (pfd->cbdata.workhlr) ok = pfd->cbdata.workhlr(userData);
	return ok;
}

/**********************************************************/

void
kucomms_timer_hlr(void * userData)
{
	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	if (pfd->cbdata.timerhlr) pfd->cbdata.timerhlr(userData);
}

/**********************************************************/

static int kucomms_thread(void *arg)
{
	bool ok;
	bool stopped = false;
	struct kucomms_file_data * pfd = (struct kucomms_file_data *)arg;

	pr_info("kucomms_thread : Entered : %lu\n", (unsigned long)pfd->vaddr);

	ok = message_manager_run(
		&pfd->msgmgr,
		pfd->vaddr_len/2,
		&stopped);

	pr_info("kucomms_thread : Exited\n");

	return 0;
}

/**********************************************************/

static loff_t kucomms_fops_llseek(struct file *, loff_t, int)
{
	pr_info("kucomms_fops_llseek : Entered\n");
	return 0;
}

static ssize_t kucomms_fops_read(struct file *, char __user *, size_t, loff_t *)
{
	pr_info("kucomms_fops_read : Entered\n");
	return 0;
}

static ssize_t kucomms_fops_write(struct file *, const char __user *, size_t, loff_t *)
{
	pr_info("kucomms_fops_write : Entered\n");
	return 0;
}

static ssize_t kucomms_fops_read_iter(struct kiocb *, struct iov_iter *)
{
	pr_info("kucomms_fops_read_iter : Entered\n");
	return 0;
}

static ssize_t kucomms_fops_write_iter(struct kiocb *, struct iov_iter *)
{
	pr_info("kucomms_fops_write_iter : Entered\n");
	return 0;
}

static int kucomms_fops_iopoll(struct kiocb *kiocb, struct io_comp_batch *,unsigned int flags)
{
	pr_info("kucomms_fops_iopoll : Entered\n");
	return 0;
}

//static int kucomms_fops_iterate(struct file *, struct dir_context *)
//{
//	pr_info("kucomms_fops_iterate : Entered\n");
//	return 0;
//}

static int kucomms_fops_iterate_shared(struct file *, struct dir_context *)
{
	pr_info("kucomms_fops_iterate_shared : Entered\n");
	return 0;
}

static __poll_t kucomms_fops_poll(struct file *, struct poll_table_struct *)
{
	pr_info("kucomms_fops_poll : Entered\n");
	return 0;
}

static long kucomms_fops_unlocked_ioctl(struct file *, unsigned int, unsigned long)
{
	pr_info("kucomms_fops_unlocked_ioctl : Entered\n");
	return 0;
}

static long kucomms_fops_compat_ioctl(struct file *, unsigned int, unsigned long)
{
	pr_info("kucomms_fops_compat_ioctl : Entered\n");
	return 0;
}

///////////////////////////////////////////////////////////////

#define KUCOMMS_MAX_MEM_ALLOC_SIZE (1024*1024*1024)

///////////////////////////////////////////////////////////////

//
// SAB : this gets called when userspace calls mmap
//
static int kucomms_fops_mmap(struct file *filp, struct vm_area_struct *vma)
{
	bool ok;
	int ret;
	void * userData;
	void * vaddr;
	struct Message * message;
	struct kucomms_file_data * pfd;
	unsigned long len = vma->vm_end - vma->vm_start;

	pfd = (struct kucomms_file_data *)filp->private_data;

	if (pfd->vaddr) {
		pr_err("kucomms_fops_mmap : Memory cannot be mapped more than once\n");
		return -EIO;
	}

	if (len > KUCOMMS_MAX_MEM_ALLOC_SIZE) {
		pr_err("kucomms_fops_mmap : Maximum mem allocation size exceeded\n");
		return -EIO;
	}

	vaddr = vmalloc_user(len);

	if (vaddr == 0) {
		pr_err("kucomms_fops_mmap : Error from vmalloc\n");
		return -EIO;
	}

	ret = remap_vmalloc_range(vma, vaddr, 0);

	if (ret != 0) {
		pr_err("kucomms_fops_mmap : Error from remap_vmalloc_range\n");
		vfree(vaddr);
		return -EIO;
	}

	message = (struct Message *)vmalloc(message_queue_get_queue_length(len/2));
	if (!message) {
		pr_info("kucomms_fops_mmap : Error from vmalloc\n");
		vfree(vaddr);
		return -EIO;
	}

	pfd->rx_msgq = (MessageQueueHeaderPtr)vaddr;
	pfd->tx_msgq = (MessageQueueHeaderPtr)((__u8*)vaddr+(len/2));

	ok = message_queue_init(pfd->rx_msgq, len/2);

	if (!ok) {
		pr_err("kucomms_fops_mmap : Error from message_queue_init\n");
		vfree(message);
		vfree(vaddr);
		return -EIO;
	}

	ok = message_queue_init(pfd->tx_msgq, len/2);

	if (!ok) {
		pr_err("kucomms_fops_mmap : Error from message_queue_init\n");
		vfree(message);
		vfree(vaddr);
		return -EIO;
	}

	userData = pfd;

	ok = message_manager_init(
		&pfd->msgmgr,
		kucomms_message_hlr,
		kucomms_work_hlr,
		kucomms_timer_hlr,
		userData);

	if (!ok) {
		pr_err("kucomms_fops_mmap : Error from message_manager_init\n");
		vfree(message);
		vfree(vaddr);
		return -EIO;
	}

	ok = message_manager_add_msgq(
		&pfd->msgmgr,
		pfd->rx_msgq,
		pfd->tx_msgq,
		message_queue_get_queue_length(len/2),
		message_queue_get_queue_length(len/2));

	if (!ok) {
		pr_err("kucomms_fops_mmap : Error from message_manager_add_msgq\n");
		vfree(message);
		vfree(vaddr);
		return -EIO;
	}

	pfd->vaddr = vaddr;
	pfd->vaddr_len = len;
	pfd->cbdata.message = message;

	wake_up_process(pfd->thread);

	return 0;
}

//
// SAB : this gets called when userspace calls open
//
static int kucomms_fops_open(struct inode * inodep, struct file * filp)
{
	struct kucomms_callback_data * pcbdata;
	struct kucomms_file_data * pfd;
	struct task_struct * thread;
	bool open;


	pcbdata = kucomms_find_and_open(filp->f_path.dentry->d_name.name, filp->f_path.dentry->d_name.len, &open);

	if (pcbdata == 0) {
		pr_info("kucomms_fops_open : There are no registered callbacks\n");
		return -1;
	}

	if (open) {
		pr_info("kucomms_fops_open : Device file already open\n");
		return -1;
	}

	pfd = (struct kucomms_file_data *)vmalloc(sizeof(struct kucomms_file_data));
	if (!pfd) {
		pr_info("kucomms_fops_open : Error from vmalloc\n");
		kucomms_find_and_close(filp->f_path.dentry->d_name.name, filp->f_path.dentry->d_name.len);
		return -1;
	}

	thread = kthread_create(kucomms_thread, pfd, "KThread kucomms");
	if (IS_ERR(thread)) {
		pr_info("kucomms_fops_open : Error from kthread_create\n");
		kucomms_find_and_close(filp->f_path.dentry->d_name.name, filp->f_path.dentry->d_name.len);
		vfree(pfd);
		return -1;
	}

	// inc ref count
	try_module_get(THIS_MODULE);

	pfd->vaddr = 0;
	pfd->vaddr_len = 0;
	pfd->thread = thread;
	pfd->cbdata.msghlr = 0;
	pfd->cbdata.workhlr = 0;
	pfd->cbdata.timerhlr = 0;
	pfd->cbdata.userData = 0;
	pfd->cbdata.message = 0;

	if (pcbdata) pfd->cbdata = *pcbdata;

	pfd->rx_msgq = 0;
	pfd->tx_msgq = 0;

	filp->private_data = pfd;

	return 0;
}

//
// SAB : this gets called when userspace calls close + unmap
//
static int kucomms_fops_flush(struct file * filp, fl_owner_t id)
{
	pr_info("kucomms_fops_flush : Entered\n");
	return 0;
}

//
// SAB : this gets called when userspace calls close + unmap
//
static int kucomms_fops_release(struct inode * inodep, struct file * filp)
{
	struct kucomms_file_data * pfd;

	kucomms_find_and_close(filp->f_path.dentry->d_name.name, filp->f_path.dentry->d_name.len);

	pr_info("kucomms_fops_release : Entered\n");

	pfd = (struct kucomms_file_data *)filp->private_data;

	kthread_stop(pfd->thread);

	if (pfd->cbdata.message) vfree(pfd->cbdata.message);

	if (pfd->vaddr) vfree(pfd->vaddr);

	vfree(filp->private_data);

	// dec ref count
	module_put(THIS_MODULE);

	pr_info("kucomms_fops_release : Exited\n");

	return 0;
}

static int kucomms_fops_fsync(struct file *, loff_t, loff_t, int datasync)
{
	pr_info("kucomms_fops_fsync : Entered\n");
	return 0;
}

static int kucomms_fops_fasync(int, struct file *, int)
{
	pr_info("kucomms_fops_fasync : Entered\n");
	return 0;
}

static int kucomms_fops_lock(struct file *, int, struct file_lock *)
{
	pr_info("kucomms_fops_lock : Entered\n");
	return 0;
}

//static ssize_t kucomms_fops_sendpage(struct file *, struct page *, int, size_t, loff_t *, int)
//{
//	pr_info("kucomms_fops_sendpage : Entered\n");
//	return 0;
//}

//
// SAB : this gets called when userspace calls open + mmap
// This task is normally performed by the memory management code;
// this method exists to allow drivers to enforce any alignment requirements a particular device may have.
// Most drivers can leave this method NULL. 
//
#if 0
static unsigned long kucomms_fops_get_unmapped_area(struct file *, unsigned long, unsigned long, unsigned long, unsigned long)
{
	pr_info("kucomms_fops_get_unmapped_area : Entered\n");
	return 0;
}
#endif

static int kucomms_fops_check_flags(int)
{
	pr_info("kucomms_fops_check_flags : Entered\n");
	return 0;
}

static int kucomms_fops_flock(struct file *, int, struct file_lock *)
{
	pr_info("kucomms_fops_flock  : Entered\n");
	return 0;
}

static ssize_t kucomms_fops_splice_write(struct pipe_inode_info *, struct file *, loff_t *, size_t, unsigned int)
{
	pr_info("kucomms_fops_splice_write : Entered\n");
	return 0;
}

static ssize_t kucomms_fops_splice_read(struct file *, loff_t *, struct pipe_inode_info *, size_t, unsigned int)
{
	pr_info("kucomms_fops_splice_read : Entered\n");
	return 0;
}

static int kucomms_fops_setlease(struct file *, int, struct file_lock **, void **)
{
	pr_info("kucomms_fops_setlease : Entered\n");
	return 0;
}

static long kucomms_fops_fallocate(struct file *file, int mode, loff_t offset,loff_t len)
{
	pr_info("kucomms_fops_fallocate : Entered\n");
	return 0;
}

static void kucomms_fops_show_fdinfo(struct seq_file *m, struct file *f)
{
	pr_info("kucomms_fops_show_fdinfo : Entered\n");
}

static ssize_t kucomms_fops_copy_file_range(struct file *, loff_t, struct file *,loff_t, size_t, unsigned int)
{
	pr_info("kucomms_fops_copy_file_range : Entered\n");
	return 0;
}

static loff_t kucomms_fops_remap_file_range(struct file *file_in, loff_t pos_in,struct file *file_out, loff_t pos_out,loff_t len, unsigned int remap_flags)
{
	pr_info("kucomms_fops_remap_file_range : Entered\n");
	return 0;
}

static int kucomms_fops_fadvise(struct file *, loff_t, loff_t, int)
{
	pr_info("kucomms_fops_fadvise : Entered\n");
	return 0;
}

static int kucomms_fops_uring_cmd(struct io_uring_cmd *ioucmd, unsigned int issue_flags)
{
	pr_info("kucomms_fops_uring_cmd : Entered\n");
	return 0;
}

static int kucomms_fops_uring_cmd_iopoll(struct io_uring_cmd *, struct io_comp_batch *,unsigned int poll_flags)
{
	pr_info("kucomms_fops_uring_cmd_iopoll : Entered\n");
	return 0;
}

/**********************************************************/

struct file_operations kucomms_file_operations =
{
	.llseek			= kucomms_fops_llseek,
	.read			= kucomms_fops_read,
	.write			= kucomms_fops_write,
	.read_iter		= kucomms_fops_read_iter,
	.write_iter		= kucomms_fops_write_iter,
	.iopoll			= kucomms_fops_iopoll,
//	.iterate		= kucomms_fops_iterate,
	.iterate_shared		= kucomms_fops_iterate_shared,
	.poll			= kucomms_fops_poll,
	.unlocked_ioctl		= kucomms_fops_unlocked_ioctl,
	.compat_ioctl		= kucomms_fops_compat_ioctl,
	.mmap			= kucomms_fops_mmap,
	.open			= kucomms_fops_open,
	.flush			= kucomms_fops_flush,
	.release		= kucomms_fops_release,
	.fsync			= kucomms_fops_fsync,
	.fasync			= kucomms_fops_fasync,
	.lock			= kucomms_fops_lock,
//	.sendpage		= kucomms_fops_sendpage,
//	.get_unmapped_area	= kucomms_fops_get_unmapped_area,
	.check_flags		= kucomms_fops_check_flags,
	.flock			= kucomms_fops_flock,
	.splice_write		= kucomms_fops_splice_write,
	.splice_read		= kucomms_fops_splice_read,
	.setlease		= kucomms_fops_setlease,
	.fallocate		= kucomms_fops_fallocate,
	.show_fdinfo		= kucomms_fops_show_fdinfo,
	.copy_file_range	= kucomms_fops_copy_file_range,
	.remap_file_range	= kucomms_fops_remap_file_range,
	.fadvise		= kucomms_fops_fadvise,
	.uring_cmd		= kucomms_fops_uring_cmd,
	.uring_cmd_iopoll	= kucomms_fops_uring_cmd_iopoll,
};

/**********************************************************/
