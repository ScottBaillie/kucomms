///////////////////////////////////////////////////////////////

#include "message_manager.h"

///////////////////////////////////////////////////////////////

#if KERNEL_BUILD
#include <linux/delay.h>
#include <linux/time.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/vmalloc.h>
#else
#include <unistd.h>
#include <stdlib.h>
#endif

///////////////////////////////////////////////////////////////

bool
message_manager_init(
	struct MessageManagerStruct * pMessageManager,
	MessageHandler_C msghlr,
	WorkHandler_C workhlr,
	TimerHandler_C timerhlr,
	void * userData)
{
	pMessageManager->msgq_array_length = 0;
	pMessageManager->msghlr = msghlr;
	pMessageManager->workhlr = workhlr;
	pMessageManager->timerhlr = timerhlr;
	pMessageManager->userData = userData;
	for (__u32 u0=0;u0<MSGMGR_MSGQARRAY_SIZE;u0++) {
		pMessageManager->rx_msgq_array[u0] = 0;
		pMessageManager->tx_msgq_array[u0] = 0;
		pMessageManager->rx_msgq_len_array[u0] = 0;
		pMessageManager->tx_msgq_len_array[u0] = 0;
	}
	return true;
}

///////////////////////////////////////////////////////////////

bool
message_manager_add_msgq(
	struct MessageManagerStruct * pMessageManager,
	MessageQueueHeaderPtr rx_msgq,
	MessageQueueHeaderPtr tx_msgq,
	const __u64 rx_msgq_queueLength,
	const __u64 tx_msgq_queueLength)
{
	if (pMessageManager->msgq_array_length == MSGMGR_MSGQARRAY_SIZE) return false;

	pMessageManager->rx_msgq_array[pMessageManager->msgq_array_length] = rx_msgq;
	pMessageManager->tx_msgq_array[pMessageManager->msgq_array_length] = tx_msgq;

	pMessageManager->rx_msgq_len_array[pMessageManager->msgq_array_length] = rx_msgq_queueLength;
	pMessageManager->tx_msgq_len_array[pMessageManager->msgq_array_length] = tx_msgq_queueLength;

	pMessageManager->msgq_array_length++;

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_manager_run(
	struct MessageManagerStruct * pMessageManager,
	__u64 bufferLen,
	bool * stopped)
{
	bool ok;
	bool error = false;
	struct MessageQueueHeader * rx_msgq;
	struct MessageQueueHeader * tx_msgq;
	struct Message * pmessage;
	__u8 * buffer;
	__u64 avail;
	__u64 counter = 0;
	__u64 rx_msgq_queueLength;
	__u64 tx_msgq_queueLength;

#if KERNEL_BUILD
	buffer = vmalloc(bufferLen);
#else
	buffer = malloc(bufferLen);
#endif

	while (*stopped == false) {

#if KERNEL_BUILD
		if (kthread_should_stop()) break;
#endif

		avail = 0;

		for (__u32 u0=0;u0<pMessageManager->msgq_array_length;u0++) {
			rx_msgq = pMessageManager->rx_msgq_array[u0];
			tx_msgq = pMessageManager->tx_msgq_array[u0];
			rx_msgq_queueLength = pMessageManager->rx_msgq_len_array[u0];
			tx_msgq_queueLength = pMessageManager->tx_msgq_len_array[u0];
			ok = message_queue_get_begin_l(rx_msgq, rx_msgq_queueLength, &pmessage);
			if (!ok) continue;
			if (pmessage == 0) {
				ok = message_queue_get_l(rx_msgq, rx_msgq_queueLength, buffer, bufferLen, &error);
				if (error) break;
				if (!ok) continue;
				pmessage = (struct Message *)buffer;

				ok = pMessageManager->msghlr(pmessage,
							tx_msgq,
							tx_msgq_queueLength,
							pMessageManager->userData);
			} else {
				ok = pMessageManager->msghlr(pmessage,
							tx_msgq,
							tx_msgq_queueLength,
							pMessageManager->userData);

				message_queue_get_complete_l(rx_msgq, rx_msgq_queueLength, pmessage);
			}

			avail += message_queue_get_avail_l(rx_msgq, rx_msgq_queueLength);

			counter += 1;

			if ((counter%MSGMGR_WORK_INTERVAL) == 0) {
				ok = pMessageManager->workhlr(pMessageManager->userData);
			}

			if ((counter%MSGMGR_TIMER1_INTERVAL) == 0) {
				pMessageManager->timerhlr(pMessageManager->userData);
			}
		}

		if (error) break;

		if (avail != 0) continue;

		ok = pMessageManager->workhlr(pMessageManager->userData);

		counter += 1;

		if ((counter%MSGMGR_TIMER2_INTERVAL) == 0) {
			pMessageManager->timerhlr(pMessageManager->userData);
		}

		if (ok) continue;

#if KERNEL_BUILD
		usleep_range(1000,1500);
#else
		usleep(1000);
#endif
	}

#if KERNEL_BUILD
	vfree(buffer);
#else
	free(buffer);
#endif

	return !error;
}

///////////////////////////////////////////////////////////////
