///////////////////////////////////////////////////////////////

#include <kucomms/MessageQueueReader.h>

///////////////////////////////////////////////////////////////

bool
MessageQueueReader::init(__u8 * buffer, const __u64 bufferLength)
{
	this->m_mq = (MessageQueueHeaderPtr)buffer;
	bool ok = message_queue_init(this->m_mq, bufferLength);
	return ok;
}

bool
MessageQueueReader::init(__u8 * buffer)
{
	this->m_mq = (MessageQueueHeaderPtr)buffer;
	return true;
}

__u64
MessageQueueReader::get_avail()
{
	return(message_queue_get_avail(this->m_mq));
}

__u64
MessageQueueReader::get_free()
{
	return(message_queue_get_free(this->m_mq));
}

__u64
MessageQueueReader::get_length()
{
	return(message_queue_get_length(this->m_mq));
}

MessageQueueHeaderPtr
MessageQueueReader::get_mq()
{
	return(this->m_mq);
}

bool
MessageQueueReader::get(
	__u8 * get_buffer,
	const __u64 get_bufferLen,
	bool * error)
{
	bool ok = message_queue_get(
			this->m_mq,
			get_buffer,
			get_bufferLen,
			error);

	return(ok);
}

bool
MessageQueueReader::get(struct Message ** pmessage)
{
	bool error;

	if (this->m_get_buffer.size() < this->m_mq->m_length) {
		this->m_get_buffer.resize(this->m_mq->m_length);
	}

	bool ok = message_queue_get(
			this->m_mq,
			this->m_get_buffer.data(),
			this->m_get_buffer.size(),
			&error);

	if (ok)
		*pmessage = (struct Message *)this->m_get_buffer.data();
	else
		*pmessage = 0;

	return(ok);
}

bool
MessageQueueReader::next_length(__u64 * bufferLen)
{
	return(message_queue_next_length(this->m_mq, bufferLen));
}

bool
MessageQueueReader::get_begin(struct Message ** pmessage)
{
	return(message_queue_get_begin(this->m_mq, pmessage));
}

void
MessageQueueReader::get_complete(const struct Message * message)
{
	return(message_queue_get_complete(this->m_mq, message));
}

///////////////////////////////////////////////////////////////
