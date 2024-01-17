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

__u64 g_sequence_wr = 0;
__u64 g_sequence_rd = 0;

__u64 g_message_sent_count_0 = 0;
__u64 g_message_sent_count_1 = 0;
__u64 g_message_received_count_0 = 0;
__u64 g_message_received_count_1 = 0;
__u64 g_message_error_count = 0;

__u64 g_timer_counter = 0;

bool g_first_message_received = false;

/**********************************************************/

static bool
kucomms_message_hlr(const struct Message * message, MessageQueueHeaderPtr tx_msgq, const __u64 rx_msgq_queueLength, const __u64 tx_msgq_queueLength, void * userData)
{
	__u64 dataLength;

	g_first_message_received = true;

	if (message->m_type == 0) {
		g_message_received_count_0++;
		message_queue_add_l(tx_msgq, tx_msgq_queueLength, message);
		g_message_sent_count_0++;
		return true;
	}
	g_message_received_count_1++;

	if (message->m_id != g_sequence_rd) {
		g_message_error_count++;
		return true;
	}

	dataLength = g_sequence_rd % 2048;

	if (message->m_length != dataLength) {
		g_message_error_count++;
		return true;
	}

	for (__u32 u0=0; u0<dataLength; u0++) {
		if (message->m_data[u0] != (g_sequence_rd % 256)) {
			g_message_error_count++;
			return true;
		}
	}

	g_sequence_rd++;

	return true;
}

/**********************************************************/

static bool
kucomms_work_hlr(void * userData)
{
	struct kucomms_file_data * pfd = (struct kucomms_file_data *)userData;
	struct Message * message;
	__u64 dataLength;

	if (!g_first_message_received) return false;

	dataLength = g_sequence_wr % 2048;
	message = vmalloc(message_get_message_length(dataLength));

	message->m_length = dataLength;
	message->m_type = 1;
	message->m_id = g_sequence_wr;
	message->m_userValue = 0;

	for (__u32 u0=0; u0<dataLength; u0++) message->m_data[u0] = g_sequence_wr % 256;

	message_queue_add_tx0(pfd, message);

	g_message_sent_count_1++;

	vfree(message);

	g_sequence_wr++;

	return false;
}

/**********************************************************/

static void
kucomms_timer_hlr(const __u64 time, void * userData)
{
	g_timer_counter++;
	if ((g_timer_counter%10) == 0) {
		pr_info("sent_count_0=%llu : sent_count_1=%llu : received_count_0=%llu : received_count_1=%llu : g_message_error_count=%llu\n",
			g_message_sent_count_0,
			g_message_sent_count_1,
			g_message_received_count_0,
			g_message_received_count_1,
			g_message_error_count);
	}
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
		0);

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
