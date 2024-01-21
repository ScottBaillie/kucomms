///////////////////////////////////////////////////////////////

#ifndef KUCOMMS_DATAMESSAGE_H
#define KUCOMMS_DATAMESSAGE_H

///////////////////////////////////////////////////////////////

#define KERNEL_BUILD 0

extern "C" {
#include <kucomms/message_queue.h>
}

#include <vector>

///////////////////////////////////////////////////////////////

class DataMessage
{
public:
	DataMessage() {}

	DataMessage(const __u64 dataLength);
	DataMessage(struct Message * pmessage, bool zeroHeader);

	bool init(const __u64 dataLength);
	bool init(struct Message * pmessage, bool zeroHeader);

	struct Message * get();

	__u8 * get_data();

	__u64 get_data_length();
	__u64 get_message_length();

	void copy(struct Message * pmessage);

private:
	std::vector<__u8>	m_buffer;
	struct Message * 	m_message = 0;
};

///////////////////////////////////////////////////////////////

#endif

/////////////////////////////////////////////////////////////// 
