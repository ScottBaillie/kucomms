///////////////////////////////////////////////////////////////

#ifndef KUCOMMS_MESSAGE_MANAGER_H
#define KUCOMMS_MESSAGE_MANAGER_H

///////////////////////////////////////////////////////////////

#include "message_queue.h"

///////////////////////////////////////////////////////////////

#define MSGMGR_MSGQARRAY_SIZE 64
#define MSGMGR_WORK_INTERVAL 8

///////////////////////////////////////////////////////////////

//
// A message handler gets called when a message arrives.
//
typedef bool (*MessageHandler_C)(const struct Message * message, MessageQueueHeaderPtr tx_msgq, const __u64 rx_msgq_queueLength, const __u64 tx_msgq_queueLength, void * userData);

//
// A work handler should return false if there is no work do.
// A work handler should return true if it knows that there is more work to do.
// A work handler should return false if it executes very quickly.
// If a work handler sleeps for more than the work handler poll period then it should return true.
// A work handler should execute as quickly as possible.
// The average execution time of the work handler should not exceed the poll period.
//
// The work handler is scheduled as often as possible so if it returns true all
// of the time and there is no sleeping then a lot of CPU will be used.
// A sleep for a period will occur when the work handler returns false.
//
typedef bool (*WorkHandler_C)(void * userData);

//
// A timer handler gets called periodically.
//
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
	__u32				sleep_milli;
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
