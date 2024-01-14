///////////////////////////////////////////////////////////////

#ifndef KUCOMMS_MESSAGEQUEUE_H
#define KUCOMMS_MESSAGEQUEUE_H

///////////////////////////////////////////////////////////////

#include "include.h"

///////////////////////////////////////////////////////////////

#if KERNEL_BUILD
#else
#include <stdbool.h>
#endif

#include <linux/string.h>
#include <linux/types.h>

///////////////////////////////////////////////////////////////

struct Message
{
	__u64		m_length;
	__u64		m_type;
	__u64		m_id;
	__u64		m_userValue;
	__u8		m_data[0];
};

///////////////////////////////////////////////////////////////

typedef struct Message * MessagePtr;

///////////////////////////////////////////////////////////////

struct MessageQueueHeader
{
	__u64		m_magic;
	__u64		m_length;
	__u64		m_rd;
	__u64		m_wr;
	__u8		m_queue[0];
};

///////////////////////////////////////////////////////////////

typedef struct MessageQueueHeader * MessageQueueHeaderPtr;

///////////////////////////////////////////////////////////////

typedef bool (*InitMessageFn)(struct Message * message, const __u64 dataLength, void * userData);

///////////////////////////////////////////////////////////////

bool message_queue_init(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 bufferLength);

__u64 message_queue_get_length(
	struct MessageQueueHeader * pMessageQueueHeader);

__u64 message_queue_get_queue_length(
	const __u64 bufferLength);

///////////////////////////////////////////////////////////////

__u64 message_queue_get_avail(
	struct MessageQueueHeader * pMessageQueueHeader);

__u64 message_queue_get_free(
	struct MessageQueueHeader * pMessageQueueHeader);

bool message_queue_add(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message * message);

bool message_queue_add_begin(
	struct MessageQueueHeader * pMessageQueueHeader,
	struct Message ** pmessage,
	const __u64 dataLength);

void message_queue_add_complete(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message * message);

bool message_queue_add_callback(
	struct MessageQueueHeader * pMessageQueueHeader,
	InitMessageFn hlr,
	const __u64 dataLength,
	void * userData,
	__u8 * buffer,
	const __u64 bufferLen,
	bool * error);

bool message_queue_get(
	struct MessageQueueHeader * pMessageQueueHeader,
	__u8 * buffer,
	const __u64 bufferLen,
	bool * error);

bool message_queue_next_length(
	struct MessageQueueHeader * pMessageQueueHeader,
	__u64 * bufferLen);

bool message_queue_get_begin(
	struct MessageQueueHeader * pMessageQueueHeader,
	struct Message ** pmessage);

void message_queue_get_complete(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message * message);

///////////////////////////////////////////////////////////////

__u64 message_queue_get_avail_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength);

__u64 message_queue_get_free_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength);

bool message_queue_add_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	const struct Message * message);

bool message_queue_add_begin_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	struct Message ** pmessage,
	const __u64 dataLength);

void message_queue_add_complete_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	const struct Message * message);

bool message_queue_add_callback_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	InitMessageFn hlr,
	const __u64 dataLength,
	void * userData,
	__u8 * buffer,
	const __u64 bufferLen,
	bool * error);

bool message_queue_get_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	__u8 * buffer,
	const __u64 bufferLen,
	bool * error);

bool message_queue_next_length_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	__u64 * bufferLen);

bool message_queue_get_begin_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	struct Message ** pmessage);

void message_queue_get_complete_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	const struct Message * message);


///////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////// 
