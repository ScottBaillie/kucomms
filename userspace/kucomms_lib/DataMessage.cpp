///////////////////////////////////////////////////////////////

#include "DataMessage.h"

///////////////////////////////////////////////////////////////

DataMessage::DataMessage(const __u64 dataLength)
{
	init(dataLength);
}

DataMessage::DataMessage(struct Message * pmessage, bool zeroHeader)
{
	init(pmessage, zeroHeader);
}

bool
DataMessage::init(const __u64 dataLength)
{
	this->m_buffer.resize(dataLength+sizeof(struct Message));
	this->m_message = (struct Message *)this->m_buffer.data();

	this->m_message->m_length = dataLength;
	this->m_message->m_type = 0;
	this->m_message->m_id = 0;
	this->m_message->m_userValue = 0;

	return true;
}

bool
DataMessage::init(struct Message * pmessage, bool zeroHeader)
{
	this->m_message = pmessage;

	if (zeroHeader) {
		this->m_message->m_length = 0;
		this->m_message->m_type = 0;
		this->m_message->m_id = 0;
		this->m_message->m_userValue = 0;
	}

	return true;
}

struct Message *
DataMessage::get()
{
	return(this->m_message);
}

__u8 *
DataMessage::get_data()
{
	__u8 * data = ((__u8 *)this->m_message) + sizeof(struct Message);
	return(data);
}

__u64
DataMessage::get_data_length()
{
	return(this->m_message->m_length);
}

__u64
DataMessage::get_message_length()
{
	return(this->m_message->m_length+sizeof(struct Message));
}

void
DataMessage::copy(struct Message * pmessage)
{
	this->init(pmessage->m_length);

	memcpy(	this->m_message,
		pmessage,
		pmessage->m_length+sizeof(struct Message));
}

///////////////////////////////////////////////////////////////
