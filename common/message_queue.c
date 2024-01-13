///////////////////////////////////////////////////////////////

#include "message_queue.h"

///////////////////////////////////////////////////////////////

bool
message_queue_init(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 bufferLength)
{
	pMessageQueueHeader->m_magic = 0x1122334455667788;
	pMessageQueueHeader->m_length = bufferLength - sizeof(struct MessageQueueHeader);
	pMessageQueueHeader->m_rd = 0;
	pMessageQueueHeader->m_wr = 0;

	return true;
}

///////////////////////////////////////////////////////////////

__u64
message_queue_get_avail(
	struct MessageQueueHeader * pMessageQueueHeader)
{
	const __u64 rd = pMessageQueueHeader->m_rd;
	const __u64 wr = pMessageQueueHeader->m_wr;
	if (rd == wr) return(0);
	if (wr > rd) return(wr - rd);
	return(pMessageQueueHeader->m_length + wr - rd);
}

///////////////////////////////////////////////////////////////

__u64
message_queue_get_free(
	struct MessageQueueHeader * pMessageQueueHeader)
{
	return(pMessageQueueHeader->m_length - 1 - message_queue_get_avail(pMessageQueueHeader));
}

///////////////////////////////////////////////////////////////

__u64
message_queue_get_length(
	struct MessageQueueHeader * pMessageQueueHeader)
{
	return(pMessageQueueHeader->m_length);
}

///////////////////////////////////////////////////////////////

bool
message_queue_add(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message *message)
{
	__u64 headSize;
	__u8 * dst;
	__u8 * base;
	const __u64 length = pMessageQueueHeader->m_length;
	const __u64 messageSize = message->m_length + sizeof(struct Message);
	__u64 wr;

	if (messageSize > message_queue_get_free(pMessageQueueHeader)) {
		return false;
	}

	wr = pMessageQueueHeader->m_wr;
	headSize = length - wr;
	dst = &pMessageQueueHeader->m_queue[wr];

	if (headSize >= messageSize) {
		memcpy(dst, message, messageSize);
	} else {
		base = &pMessageQueueHeader->m_queue[0];
		memcpy(dst,  message, headSize);
		memcpy(base, &((__u8 *)message)[headSize], messageSize-headSize);
	}

	wr = (wr + messageSize) % length;
	pMessageQueueHeader->m_wr = wr;

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_queue_add_begin(
	struct MessageQueueHeader * pMessageQueueHeader,
	struct Message ** pmessage,
	const __u64 dataLength)
{
	__u64 headSize;
	__u8 * dst;
	const __u64 length = pMessageQueueHeader->m_length;
	const __u64 messageSize = dataLength + sizeof(struct Message);
	__u64 wr;

	if (messageSize > message_queue_get_free(pMessageQueueHeader)) {
		return false;
	}

	wr = pMessageQueueHeader->m_wr;
	headSize = length - wr;
	dst = &pMessageQueueHeader->m_queue[wr];

	if (headSize >= messageSize) {
		*pmessage = (struct Message *)dst;
	} else {
		*pmessage = 0;
	}

	return true;
}

///////////////////////////////////////////////////////////////

void
message_queue_add_complete(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message * message)
{
	const __u64 length = pMessageQueueHeader->m_length;
	const __u64 messageSize = message->m_length + sizeof(struct Message);
	__u64 wr;

	wr = pMessageQueueHeader->m_wr;
	wr = (wr + messageSize) % length;
	pMessageQueueHeader->m_wr = wr;
}

///////////////////////////////////////////////////////////////

bool
message_queue_add_callback(
	struct MessageQueueHeader * pMessageQueueHeader,
	InitMessageFn hlr,
	const __u64 dataLength,
	void * userData,
	__u8 * buffer,
	const __u64 bufferLen,
	bool * error)
{
	const __u64 messageSize = dataLength + sizeof(struct Message);
	struct Message * pmessage = 0;
	bool ok;

	*error = false;

	ok = message_queue_add_begin(pMessageQueueHeader, &pmessage, dataLength);
	if (!ok) return false;

	if (pmessage == 0) {
		if (bufferLen < messageSize) {
			*error = true;
			return false;
		}
		pmessage = (struct Message *)buffer;
		ok = hlr(pmessage, dataLength, userData);
		if (!ok) return false;
		ok = message_queue_add(pMessageQueueHeader, pmessage);
		if (!ok) return false;
	} else {
		ok = hlr(pmessage, dataLength, userData);
		if (!ok) return false;
		message_queue_add_complete(pMessageQueueHeader, pmessage);
	}

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_queue_get(
	struct MessageQueueHeader * pMessageQueueHeader,
	__u8 * buffer,
	const __u64 bufferLen,
	bool * error)
{
	struct Message * message;
	__u64 headSize;
	__u8 * dst;
	__u8 * base;
	__u64 length;
	__u64 messageSize;
	__u64 rd = pMessageQueueHeader->m_rd;
	const __u64 wr = pMessageQueueHeader->m_wr;

	*error = false;

	if (rd == wr) {
		return false;
	}

	if (bufferLen < sizeof(struct Message)) {
		*error = true;
		return false;
	}

	length = pMessageQueueHeader->m_length;
	base = &pMessageQueueHeader->m_queue[0];

	for (__u32 u0=0; u0<sizeof(struct Message); u0++) {
		buffer[u0] = base[rd];
		rd = (rd + 1) % length;
	}

	message = (struct Message *)buffer;

	if (message->m_length == 0) {
		pMessageQueueHeader->m_rd = rd;
		return true;
	}

	messageSize = message->m_length + sizeof(struct Message);

	if (bufferLen < messageSize) {
		*error = true;
		return false;
	}

	headSize = length - rd;

	dst = buffer + sizeof(struct Message);

	if (headSize >= message->m_length) {
		memcpy(dst,          &base[rd], message->m_length);
	} else {
		memcpy(dst,          &base[rd], headSize);
		memcpy(dst+headSize, base,      message->m_length-headSize);
	}

	rd = (rd + message->m_length) % length;
	pMessageQueueHeader->m_rd = rd;

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_queue_next_length(
	struct MessageQueueHeader * pMessageQueueHeader,
	__u64 * bufferLen)
{
	struct Message message;
	__u8 * buffer = (__u8 *)&message;
	__u8 * base;
	__u64 length;
	__u64 messageSize;
	__u64 rd = pMessageQueueHeader->m_rd;
	const __u64 wr = pMessageQueueHeader->m_wr;

	if (rd == wr) {
		return false;
	}

	length = pMessageQueueHeader->m_length;
	base = &pMessageQueueHeader->m_queue[0];

	for (__u32 u0=0; u0<sizeof(struct Message); u0++) {
		buffer[u0] = base[rd];
		rd = (rd + 1) % length;
	}

	messageSize = message.m_length + sizeof(struct Message);

	*bufferLen = messageSize;

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_queue_get_begin(
	struct MessageQueueHeader * pMessageQueueHeader,
	struct Message ** pmessage)
{
	struct Message message;
	__u8 * buffer = (__u8 *)&message;
	__u64 headSize;
	__u8 * base;
	__u64 length;
	__u64 messageSize;
	__u64 rd = pMessageQueueHeader->m_rd;
	const __u64 wr = pMessageQueueHeader->m_wr;

	if (rd == wr) {
		return false;
	}

	length = pMessageQueueHeader->m_length;
	base = &pMessageQueueHeader->m_queue[0];

	for (__u32 u0=0; u0<sizeof(struct Message); u0++) {
		buffer[u0] = base[rd];
		rd = (rd + 1) % length;
	}

	messageSize = message.m_length + sizeof(struct Message);

	rd = pMessageQueueHeader->m_rd;
	headSize = length - rd;

	if (headSize >= messageSize) {
		*pmessage = (struct Message *)&base[rd];
	} else {
		*pmessage = 0;
	}

	return true;
}

///////////////////////////////////////////////////////////////

void
message_queue_get_complete(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message * message)
{
	__u64 length = pMessageQueueHeader->m_length;
	__u64 messageSize = message->m_length + sizeof(struct Message);
	__u64 rd = pMessageQueueHeader->m_rd;

	rd = (rd + messageSize) % length;
	pMessageQueueHeader->m_rd = rd;
}

///////////////////////////////////////////////////////////////
