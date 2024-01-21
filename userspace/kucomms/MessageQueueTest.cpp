///////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstdlib>

#include <kucomms/MessageQueueReader.h>
#include <kucomms/MessageQueueWriter.h>

///////////////////////////////////////////////////////////////

class TestInitMessage : public InitMessage
{
public:
	TestInitMessage(std::vector<DataMessage> & msglist)
		: m_msglist(msglist)
	{
	}

	virtual bool init(struct Message * message, const __u64 dataLength);

private:
	std::vector<DataMessage> &	m_msglist;
	uint32_t			m_index = 0;
};

bool TestInitMessage::init(struct Message * message, const __u64 dataLength)
{
	DataMessage & msg = this->m_msglist[this->m_index];

	if (dataLength != msg.get_data_length()) return false;

	memcpy(message, msg.get(), msg.get_message_length());

	this->m_index = (this->m_index+1) % this->m_msglist.size();

	return true;
}

///////////////////////////////////////////////////////////////

const uint32_t BUFFER_SIZE = 1024*1024;
const uint32_t MSGLIST_SIZE = 128;

const uint32_t LENGTH_MIN = 1024;
const uint32_t LENGTH_RANGE = 1024;

const uint32_t NUM_LOOPS = 1024*16;

///////////////////////////////////////////////////////////////

bool
message_queue_test()
{
	bool ok;
	int ret;
	struct Message * message;
	std::vector<__u8> buffer(BUFFER_SIZE);

	MessageQueueReader mqrd;
	MessageQueueWriter mqwr;

	mqrd.init(buffer.data(),buffer.size());
	mqwr.init(buffer.data());

	std::vector<DataMessage> msglist(MSGLIST_SIZE);

	TestInitMessage im(msglist);

	for (uint32_t u0=0; u0<NUM_LOOPS; u0++) {
		for (uint32_t u1=0; u1<msglist.size(); u1++) {
			__u64 dataLength = LENGTH_MIN + rand() % LENGTH_RANGE;
			msglist[u1].init(dataLength);
			for (uint32_t u2=0; u2<dataLength; u2++) {
				msglist[u1].get_data()[u2] = rand() % 256;
			}
		}
		for (uint32_t u1=0; u1<msglist.size(); u1++) {
			ok = mqwr.add_callback(im, msglist[u1].get_data_length());
			if (!ok) return false;
		}
		for (uint32_t u1=0; u1<msglist.size(); u1++) {
			ok = mqrd.get(&message);
			if (!ok) return false;
			ret = memcmp(message, msglist[u1].get(), msglist[u1].get_message_length());
			if (ret != 0) return false;
		}
		for (uint32_t u1=0; u1<msglist.size(); u1++) {
			ok = mqwr.add(msglist[u1].get());
			if (!ok) return false;
		}
		for (uint32_t u1=0; u1<msglist.size(); u1++) {
			ok = mqrd.get(&message);
			if (!ok) return false;
			ret = memcmp(message, msglist[u1].get(), msglist[u1].get_message_length());
			if (ret != 0) return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////
