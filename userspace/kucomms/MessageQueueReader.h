///////////////////////////////////////////////////////////////

#ifndef KUCOMMS_MESSAGEQUEUEREADER_H
#define KUCOMMS_MESSAGEQUEUEREADER_H

///////////////////////////////////////////////////////////////

#include <kucomms/DataMessage.h>

///////////////////////////////////////////////////////////////

class MessageQueueReader
{
public:
	MessageQueueReader()
	{
	}

	bool init(__u8 * buffer, const __u64 bufferLength);

	bool init(__u8 * buffer);

	__u64 get_avail();

	__u64 get_free();

	__u64 get_length();

	MessageQueueHeaderPtr get_mq();

	bool get(
		__u8 * get_buffer,
		const __u64 get_bufferLen,
		bool * error);

	bool get(struct Message ** pmessage);

	bool next_length(__u64 * bufferLen);

	bool get_begin(struct Message ** pmessage);

	void get_complete(const struct Message * message);

private:
	MessageQueueHeaderPtr		m_mq = 0;
	std::vector<__u8>		m_get_buffer;
};

/////////////////////////////////////////////////////////////// 

#endif

///////////////////////////////////////////////////////////////
