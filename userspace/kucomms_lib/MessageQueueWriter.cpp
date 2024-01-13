///////////////////////////////////////////////////////////////

#include "MessageQueueWriter.h"

///////////////////////////////////////////////////////////////

bool
MessageQueueWriter_InitMessageFn(struct Message * message, const __u64 dataLength, void * userData)
{
	InitMessage * pInitMessage = (InitMessage *)userData;
	return(pInitMessage->init(message, dataLength));
}

///////////////////////////////////////////////////////////////

bool
MessageQueueWriter::init(__u8 * buffer, const __u64 bufferLength)
{
	this->m_mq = (MessageQueueHeaderPtr)buffer;
	bool ok = message_queue_init(this->m_mq, bufferLength);
	return ok;
}

bool
MessageQueueWriter::init(__u8 * buffer)
{
	this->m_mq = (MessageQueueHeaderPtr)buffer;
	return true;
}

__u64
MessageQueueWriter::get_avail()
{
	return(message_queue_get_avail(this->m_mq));
}

__u64
MessageQueueWriter::get_free()
{
	return(message_queue_get_free(this->m_mq));
}

__u64
MessageQueueWriter::get_length()
{
	return(message_queue_get_length(this->m_mq));
}

MessageQueueHeaderPtr
MessageQueueWriter::get_mq()
{
	return(this->m_mq);
}

bool
MessageQueueWriter::add(const struct Message * message)
{
	return(message_queue_add(this->m_mq, message));
}

bool
MessageQueueWriter::add_begin(
	struct Message ** pmessage,
	const __u64 dataLength)
{
	return(message_queue_add_begin(this->m_mq, pmessage, dataLength));
}

void
MessageQueueWriter::add_complete(const struct Message * message)
{
	return(message_queue_add_complete(this->m_mq, message));
}

bool
MessageQueueWriter::add_callback(
	InitMessageFn hlr,
	const __u64 dataLength,
	void * userData,
	__u8 * add_buffer,
	const __u64 add_bufferLen,
	bool * error)
{
	return(message_queue_add_callback(this->m_mq,hlr,dataLength,userData,add_buffer,add_bufferLen,error));
}

bool
MessageQueueWriter::add_callback(
	InitMessage & initMessage,
	const __u64 dataLength)
{
	bool error;

	__u64 messageSize = dataLength + sizeof(struct Message);

	if (this->m_add_buffer.size() < messageSize) {
		this->m_add_buffer.resize(messageSize);
	}

	bool ok = message_queue_add_callback(
			this->m_mq,
			MessageQueueWriter_InitMessageFn,
			dataLength,
			&initMessage,
			this->m_add_buffer.data(),
			this->m_add_buffer.size(),
			&error);

	return(ok);
}

/////////////////////////////////////////////////////////////// 
