///////////////////////////////////////////////////////////////

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>

#include "MessageManager.h"

///////////////////////////////////////////////////////////////

bool message_queue_test();
bool message_manager_test();

///////////////////////////////////////////////////////////////

class KuCommsMessageHandler : public MessageHandler
{
public:
	KuCommsMessageHandler()
	{
	}

	bool hlr(struct Message * message, MessageQueueWriter & tx_msgq, std::vector<MessageQueueWriter> & tx_msgq_list);

public:
	uint32_t	m_messageCount = 0;
private:
};

///////////////////////////////////////////////////////////////

class KuCommsWorkHandler : public WorkHandler
{
public:
	KuCommsWorkHandler()
	{
	}

	bool hlr(std::vector<MessageQueueWriter> & tx_msgq_list);

public:
	uint32_t	m_messageCount = 0;
private:
};

///////////////////////////////////////////////////////////////

class KuCommsTimerHandler : public TimerHandler
{
public:
	KuCommsTimerHandler()
	{
	}

	void hlr(std::vector<MessageQueueWriter> & tx_msgq_list);

private:
};

///////////////////////////////////////////////////////////////

bool
KuCommsMessageHandler::hlr(struct Message * message, MessageQueueWriter & tx_msgq, std::vector<MessageQueueWriter> & tx_msgq_list)
{
	m_messageCount++;

	printf("KuCommsMessageHandler::hlr : message length=%lu\n", message->m_length);

	return true;
}

///////////////////////////////////////////////////////////////

bool
KuCommsWorkHandler::hlr(std::vector<MessageQueueWriter> & tx_msgq_list)
{
	if (m_messageCount == 4) {
		return false;
	}

	DataMessage msg(m_messageCount);
	tx_msgq_list[0].add(msg.get());

	m_messageCount++;

	return false;
}

///////////////////////////////////////////////////////////////

void
KuCommsTimerHandler::hlr(std::vector<MessageQueueWriter> & tx_msgq_list)
{
}

///////////////////////////////////////////////////////////////

int main(int argc, char ** argv)
{
	bool stopped;

	KuCommsMessageHandler msghlr;
	KuCommsWorkHandler workhlr;
	KuCommsTimerHandler timerhlr;

	bool ok = MessageManager::run(
				"/dev/kucomms1",
				1024*1024,
				stopped,
				msghlr,
				workhlr,
				timerhlr);

	return 0;
}

///////////////////////////////////////////////////////////////
