///////////////////////////////////////////////////////////////

#ifndef KUCOMMS_MESSAGEQUEUEWRITER_H
#define KUCOMMS_MESSAGEQUEUEWRITER_H

///////////////////////////////////////////////////////////////

#include "DataMessage.h"

///////////////////////////////////////////////////////////////

class InitMessage
{
public:
	virtual ~InitMessage() {}

	virtual bool init(struct Message * message, const __u64 dataLength) = 0;
};

///////////////////////////////////////////////////////////////

class MessageQueueWriter
{
public:
	MessageQueueWriter()
	{
	}

	bool init(__u8 * buffer, const __u64 bufferLength);

	bool init(__u8 * buffer);

	__u64 get_avail();

	__u64 get_free();

	__u64 get_length();

	MessageQueueHeaderPtr get_mq();

	bool add(const struct Message * message);

	bool add_begin(
		struct Message ** pmessage,
		const __u64 dataLength);

	void add_complete(const struct Message * message);

	bool add_callback(
		InitMessageFn hlr,
		const __u64 dataLength,
		void * userData,
		__u8 * add_buffer,
		const __u64 add_bufferLen,
		bool * error);

	bool add_callback(
		InitMessage & initMessage,
		const __u64 dataLength);

private:
	MessageQueueHeaderPtr		m_mq = 0;
	std::vector<__u8>		m_add_buffer;
};

///////////////////////////////////////////////////////////////  

#endif

///////////////////////////////////////////////////////////////
