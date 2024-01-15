///////////////////////////////////////////////////////////////

#ifndef KUCOMMS_MESSAGE_MANAGER_H
#define KUCOMMS_MESSAGE_MANAGER_H

///////////////////////////////////////////////////////////////

#include "message_queue.h"

///////////////////////////////////////////////////////////////

#define MSGMGR_MSGQARRAY_SIZE 64
#define MSGMGR_WORK_INTERVAL 8
#define MSGMGR_TIMER1_INTERVAL 64
#define MSGMGR_TIMER2_INTERVAL 1024

///////////////////////////////////////////////////////////////

typedef bool (*MessageHandler_C)(struct Message * message, MessageQueueHeaderPtr tx_msgq, const __u64 rx_msgq_queueLength, const __u64 tx_msgq_queueLength, void * userData);

// Returning false means sleep, return true means dont sleep.
typedef bool (*WorkHandler_C)(void * userData);

typedef void (*TimerHandler_C)(const __u64 time, void * userData);

///////////////////////////////////////////////////////////////

struct MessageManagerStruct
{
	MessageQueueHeaderPtr		rx_msgq_array[MSGMGR_MSGQARRAY_SIZE];
	MessageQueueHeaderPtr		tx_msgq_array[MSGMGR_MSGQARRAY_SIZE];
	__u64				rx_msgq_len_array[MSGMGR_MSGQARRAY_SIZE];
	__u64				tx_msgq_len_array[MSGMGR_MSGQARRAY_SIZE];
	__u32				msgq_array_length;
	MessageHandler_C		msghlr;
	WorkHandler_C			workhlr;
	TimerHandler_C			timerhlr;
	void *				userData;
};

///////////////////////////////////////////////////////////////

bool message_manager_init(
	struct MessageManagerStruct * pMessageManager,
	MessageHandler_C msghlr,
	WorkHandler_C workhlr,
	TimerHandler_C timerhlr,
	void * userData);

///////////////////////////////////////////////////////////////

bool message_manager_add_msgq(
	struct MessageManagerStruct * pMessageManager,
	MessageQueueHeaderPtr rx_msgq,
	MessageQueueHeaderPtr tx_msgq,
	const __u64 rx_msgq_queueLength,
	const __u64 tx_msgq_queueLength);

///////////////////////////////////////////////////////////////

bool message_manager_run(
	struct MessageManagerStruct * pMessageManager,
	__u64 bufferLen,
	bool * stopped);

///////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////
