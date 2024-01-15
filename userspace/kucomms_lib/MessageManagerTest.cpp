///////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstdlib>
#include <cstdio>

#include <unistd.h>

#include "MessageManager.h"

///////////////////////////////////////////////////////////////

const uint32_t NUM_BUFFERS = 4;
const uint32_t BUFFER_SIZE = 1024*1024;
const uint32_t MESSAGE_COUNT_MAX = 1024*8;
const uint32_t MESSAGE_DATA_RANGE = 1024;

///////////////////////////////////////////////////////////////

class TestMessageHandler : public MessageHandler
{
public:
	TestMessageHandler(uint32_t instance) :
		m_instance(instance)
	{
	}

	bool hlr(struct Message * message, MessageQueueWriter & tx_msgq, std::vector<MessageQueueWriter> & tx_msgq_list);

public:
	uint32_t	m_messageCount = 0;
private:
	uint32_t	m_instance = 0;
};

///////////////////////////////////////////////////////////////

class TestWorkHandler : public WorkHandler
{
public:
	TestWorkHandler(uint32_t instance) :
		m_instance(instance)
	{
	}

	bool hlr(std::vector<MessageQueueWriter> & tx_msgq_list);

private:
	uint32_t	m_instance = 0;
	uint32_t	m_messageCount = 0;
};

///////////////////////////////////////////////////////////////

class TestTimerHandler : public TimerHandler
{
public:
	TestTimerHandler(uint32_t instance) :
		m_instance(instance)
	{
	}

	void hlr(const __u64 time, std::vector<MessageQueueWriter> & tx_msgq_list);

private:
	uint32_t	m_instance = 0;
};

///////////////////////////////////////////////////////////////

bool
TestMessageHandler::hlr(struct Message * message, MessageQueueWriter & tx_msgq, std::vector<MessageQueueWriter> & tx_msgq_list)
{
	if (m_instance == 1) {
		tx_msgq.add(message);
		return true;
	}

	if (message->m_length != (m_messageCount % MESSAGE_DATA_RANGE)) return true;

	m_messageCount++;

	return true;
}

///////////////////////////////////////////////////////////////

bool
TestWorkHandler::hlr(std::vector<MessageQueueWriter> & tx_msgq_list)
{
	if (m_instance == 1) {
		return false;
	}

	if (m_messageCount == MESSAGE_COUNT_MAX) {
		return false;
	}

	DataMessage msg(m_messageCount % MESSAGE_DATA_RANGE);
	tx_msgq_list[0].add(msg.get());

	m_messageCount++;

	return false;
}

///////////////////////////////////////////////////////////////

void
TestTimerHandler::hlr(const __u64 time, std::vector<MessageQueueWriter> & tx_msgq_list)
{
}

///////////////////////////////////////////////////////////////

bool
message_manager_test()
{
	std::vector<std::vector<__u8> > buffer_list(NUM_BUFFERS);
	for (uint32_t u0=0; u0<buffer_list.size(); u0++) buffer_list[u0].resize(BUFFER_SIZE);

	std::vector<MessageQueueReader> rx_msgq_list_1(2);
	std::vector<MessageQueueWriter> tx_msgq_list_1(2);

	std::vector<MessageQueueReader> rx_msgq_list_2(1);
	std::vector<MessageQueueWriter> tx_msgq_list_2(1);

	std::vector<MessageQueueReader> rx_msgq_list_3(1);
	std::vector<MessageQueueWriter> tx_msgq_list_3(1);

	rx_msgq_list_1[0].init(buffer_list[0].data(),buffer_list[0].size());
	tx_msgq_list_1[0].init(buffer_list[1].data(),buffer_list[1].size());
	rx_msgq_list_1[1].init(buffer_list[2].data(),buffer_list[2].size());
	tx_msgq_list_1[1].init(buffer_list[3].data(),buffer_list[3].size());

	rx_msgq_list_2[0].init(buffer_list[1].data());
	tx_msgq_list_2[0].init(buffer_list[0].data());

	rx_msgq_list_3[0].init(buffer_list[3].data());
	tx_msgq_list_3[0].init(buffer_list[2].data());

	TestMessageHandler mhlr1(1);
	TestWorkHandler whlr1(1);
	TestTimerHandler thlr1(1);

	TestMessageHandler mhlr2(2);
	TestWorkHandler whlr2(2);
	TestTimerHandler thlr2(2);

	TestMessageHandler mhlr3(3);
	TestWorkHandler whlr3(3);
	TestTimerHandler thlr3(3);

	MessageManager mmgr1(mhlr1,whlr1,thlr1,rx_msgq_list_1,tx_msgq_list_1);

	MessageManager mmgr2(mhlr2,whlr2,thlr2,rx_msgq_list_2,tx_msgq_list_2);

	MessageManager mmgr3(mhlr3,whlr3,thlr3,rx_msgq_list_3,tx_msgq_list_3);

	bool ok = mmgr1.start();
	if (!ok) return false;
	ok = mmgr2.start();
	if (!ok) return false;
	ok = mmgr3.start();
	if (!ok) return false;

	usleep(16*1000000);

	ok = mmgr1.stop();
	ok = mmgr2.stop();
	ok = mmgr3.stop();

	if (mhlr2.m_messageCount != MESSAGE_COUNT_MAX) {
		printf("message_manager_test : message count error : %u\n", mhlr2.m_messageCount);
		return false;
	}
	if (mhlr3.m_messageCount != MESSAGE_COUNT_MAX) {
		printf("message_manager_test : message count error : %u\n", mhlr2.m_messageCount);
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////
