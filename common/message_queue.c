///////////////////////////////////////////////////////////////

#include <kucomms/message_queue.h>

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
message_queue_get_length(
	struct MessageQueueHeader * pMessageQueueHeader)
{
	return(pMessageQueueHeader->m_length);
}

///////////////////////////////////////////////////////////////

__u64
message_queue_get_queue_length(
	const __u64 bufferLength)
{
	return(bufferLength - sizeof(struct MessageQueueHeader));
}

__u64
message_queue_get_buffer_length(
	const __u64 queueLength)
{
	return(queueLength + sizeof(struct MessageQueueHeader));
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

__u64
message_queue_get_avail(
	struct MessageQueueHeader * pMessageQueueHeader)
{
	return(message_queue_get_avail_l(pMessageQueueHeader,pMessageQueueHeader->m_length));
}

///////////////////////////////////////////////////////////////

__u64
message_queue_get_free(
	struct MessageQueueHeader * pMessageQueueHeader)
{
	return(message_queue_get_free_l(pMessageQueueHeader,pMessageQueueHeader->m_length));
}

///////////////////////////////////////////////////////////////

bool
message_queue_add(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message *message)
{
	return(message_queue_add_l(pMessageQueueHeader,pMessageQueueHeader->m_length,message));
}

///////////////////////////////////////////////////////////////

bool
message_queue_add_begin(
	struct MessageQueueHeader * pMessageQueueHeader,
	struct Message ** pmessage,
	const __u64 dataLength)
{
	return(message_queue_add_begin_l(pMessageQueueHeader,pMessageQueueHeader->m_length,pmessage,dataLength));
}

///////////////////////////////////////////////////////////////

void
message_queue_add_complete(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message * message)
{
	return(message_queue_add_complete_l(pMessageQueueHeader,pMessageQueueHeader->m_length,message));
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
	return(message_queue_add_callback_l(pMessageQueueHeader,pMessageQueueHeader->m_length,hlr,dataLength,userData,buffer,bufferLen,error));
}

///////////////////////////////////////////////////////////////

bool
message_queue_get(
	struct MessageQueueHeader * pMessageQueueHeader,
	__u8 * buffer,
	const __u64 bufferLen,
	bool * error)
{
	return(message_queue_get_l(pMessageQueueHeader,pMessageQueueHeader->m_length,buffer,bufferLen,error));
}

///////////////////////////////////////////////////////////////

bool
message_queue_next_length(
	struct MessageQueueHeader * pMessageQueueHeader,
	__u64 * bufferLen)
{
	return(message_queue_next_length_l(pMessageQueueHeader,pMessageQueueHeader->m_length,bufferLen));
}

///////////////////////////////////////////////////////////////

bool
message_queue_get_begin(
	struct MessageQueueHeader * pMessageQueueHeader,
	struct Message ** pmessage)
{
	return(message_queue_get_begin_l(pMessageQueueHeader,pMessageQueueHeader->m_length,pmessage));
}

///////////////////////////////////////////////////////////////

void
message_queue_get_complete(
	struct MessageQueueHeader * pMessageQueueHeader,
	const struct Message * message)
{
	return(message_queue_get_complete_l(pMessageQueueHeader,pMessageQueueHeader->m_length,message));
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

__u64
message_queue_get_avail_l_base(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	const __u64 rd,
	const __u64 wr)
{
	if (rd >= queueLength) return(0);
	if (wr >= queueLength) return(0);
	if (rd == wr) return(0);
	if (wr > rd) return(wr - rd);
	return(queueLength + wr - rd);
}

__u64
message_queue_get_avail_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength)
{
	const __u64 rd = pMessageQueueHeader->m_rd;
	const __u64 wr = pMessageQueueHeader->m_wr;
	return(message_queue_get_avail_l_base(pMessageQueueHeader,queueLength,rd,wr));
}

///////////////////////////////////////////////////////////////

__u64
message_queue_get_free_l_base(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	const __u64 rd,
	const __u64 wr)
{
	return(queueLength - 1 - message_queue_get_avail_l_base(pMessageQueueHeader,queueLength,rd,wr));
}

__u64
message_queue_get_free_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength)
{
	const __u64 rd = pMessageQueueHeader->m_rd;
	const __u64 wr = pMessageQueueHeader->m_wr;
	return(message_queue_get_free_l_base(pMessageQueueHeader,queueLength,rd,wr));
}

///////////////////////////////////////////////////////////////

bool
message_queue_add_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	const struct Message *message)
{
	__u64 headSize;
	__u8 * dst;
	__u8 * base;
	const __u64 messageSize = message->m_length + sizeof(struct Message);

	const __u64 rd = pMessageQueueHeader->m_rd;
	__u64 wr = pMessageQueueHeader->m_wr;

	if (messageSize > message_queue_get_free_l_base(pMessageQueueHeader,queueLength,rd,wr)) {
		return false;
	}

	if (wr >= queueLength) return false;
	headSize = queueLength - wr;
	dst = &pMessageQueueHeader->m_queue[wr];

	if (headSize >= messageSize) {
		memcpy(dst, message, messageSize);
	} else {
		base = &pMessageQueueHeader->m_queue[0];
		memcpy(dst,  message, headSize);
		memcpy(base, &((__u8 *)message)[headSize], messageSize-headSize);
	}

	wr = (wr + messageSize) % queueLength;
	pMessageQueueHeader->m_wr = wr;

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_queue_add_begin_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	struct Message ** pmessage,
	const __u64 dataLength)
{
	__u64 headSize;
	__u8 * dst;
	const __u64 messageSize = dataLength + sizeof(struct Message);

	const __u64 rd = pMessageQueueHeader->m_rd;
	__u64 wr = pMessageQueueHeader->m_wr;

	if (messageSize > message_queue_get_free_l_base(pMessageQueueHeader,queueLength,rd,wr)) {
		return false;
	}

	if (wr >= queueLength) return false;
	headSize = queueLength - wr;
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
message_queue_add_complete_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	const struct Message * message)
{
	const __u64 messageSize = message->m_length + sizeof(struct Message);
	__u64 wr;

	wr = pMessageQueueHeader->m_wr;
	if (wr >= queueLength) return;
	wr = (wr + messageSize) % queueLength;
	pMessageQueueHeader->m_wr = wr;
}

///////////////////////////////////////////////////////////////

bool
message_queue_add_callback_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
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

	ok = message_queue_add_begin_l(pMessageQueueHeader, queueLength, &pmessage, dataLength);
	if (!ok) return false;

	if (pmessage == 0) {
		if (bufferLen < messageSize) {
			*error = true;
			return false;
		}
		pmessage = (struct Message *)buffer;
		ok = hlr(pmessage, dataLength, userData);
		if (!ok) return false;
		ok = message_queue_add_l(pMessageQueueHeader, queueLength, pmessage);
		if (!ok) return false;
	} else {
		ok = hlr(pmessage, dataLength, userData);
		if (!ok) return false;
		message_queue_add_complete_l(pMessageQueueHeader, queueLength, pmessage);
	}

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_queue_get_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	__u8 * buffer,
	const __u64 bufferLen,
	bool * error)
{
	struct Message * message;
	__u64 headSize;
	__u8 * dst;
	__u8 * base;
	__u64 messageSize;
	__u64 rd = pMessageQueueHeader->m_rd;
	const __u64 wr = pMessageQueueHeader->m_wr;

	if (rd >= queueLength) {
		*error = true;
		return false;
	}

	if (wr >= queueLength) {
		*error = true;
		return false;
	}

	*error = false;

	if (rd == wr) {
		return false;
	}

	if (bufferLen < sizeof(struct Message)) {
		*error = true;
		return false;
	}

	base = &pMessageQueueHeader->m_queue[0];

	for (__u32 u0=0; u0<sizeof(struct Message); u0++) {
		buffer[u0] = base[rd];
		rd = (rd + 1) % queueLength;
	}

	message = (struct Message *)buffer;

	messageSize = message->m_length + sizeof(struct Message);

	if (messageSize >= queueLength) {
		*error = true;
		return false;
	}

	if (message->m_length == 0) {
		pMessageQueueHeader->m_rd = rd;
		return true;
	}

	if (bufferLen < messageSize) {
		*error = true;
		return false;
	}

	headSize = queueLength - rd;

	dst = buffer + sizeof(struct Message);

	if (headSize >= message->m_length) {
		memcpy(dst,          &base[rd], message->m_length);
	} else {
		memcpy(dst,          &base[rd], headSize);
		memcpy(dst+headSize, base,      message->m_length-headSize);
	}

	rd = (rd + message->m_length) % queueLength;
	pMessageQueueHeader->m_rd = rd;

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_queue_next_length_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	__u64 * bufferLen)
{
	struct Message message;
	__u8 * buffer = (__u8 *)&message;
	__u8 * base;
	__u64 messageSize;
	__u64 rd = pMessageQueueHeader->m_rd;
	const __u64 wr = pMessageQueueHeader->m_wr;

	if (rd >= queueLength) {
		return false;
	}

	if (wr >= queueLength) {
		return false;
	}

	if (rd == wr) {
		return false;
	}

	base = &pMessageQueueHeader->m_queue[0];

	for (__u32 u0=0; u0<sizeof(struct Message); u0++) {
		buffer[u0] = base[rd];
		rd = (rd + 1) % queueLength;
	}

	messageSize = message.m_length + sizeof(struct Message);

	if (messageSize >= queueLength) {
		return false;
	}

	*bufferLen = messageSize;

	return true;
}

///////////////////////////////////////////////////////////////

bool
message_queue_get_begin_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	struct Message ** pmessage)
{
	struct Message message;
	__u8 * buffer = (__u8 *)&message;
	__u64 headSize;
	__u8 * base;
	__u64 messageSize;
	__u64 rd = pMessageQueueHeader->m_rd;
	__u64 rd0 = rd;
	const __u64 wr = pMessageQueueHeader->m_wr;

	if (rd >= queueLength) {
		return false;
	}

	if (wr >= queueLength) {
		return false;
	}

	if (rd == wr) {
		return false;
	}

	base = &pMessageQueueHeader->m_queue[0];

	for (__u32 u0=0; u0<sizeof(struct Message); u0++) {
		buffer[u0] = base[rd];
		rd = (rd + 1) % queueLength;
	}

	messageSize = message.m_length + sizeof(struct Message);

	if (messageSize >= queueLength) {
		return false;
	}

	rd = rd0;
	headSize = queueLength - rd;

	if (headSize >= messageSize) {
		*pmessage = (struct Message *)&base[rd];
	} else {
		*pmessage = 0;
	}

	return true;
}

///////////////////////////////////////////////////////////////

void
message_queue_get_complete_l(
	struct MessageQueueHeader * pMessageQueueHeader,
	const __u64 queueLength,
	const struct Message * message)
{
	__u64 length = message->m_length;
 	__u64 messageSize = length + sizeof(struct Message);
	__u64 rd = pMessageQueueHeader->m_rd;

	if (rd >= queueLength) return;
	if (messageSize >= queueLength) return;

	rd = (rd + messageSize) % queueLength;
	pMessageQueueHeader->m_rd = rd;
}

///////////////////////////////////////////////////////////////
