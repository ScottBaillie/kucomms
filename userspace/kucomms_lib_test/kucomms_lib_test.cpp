///////////////////////////////////////////////////////////////

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <signal.h>

#include "MessageManager.h"

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

	void hlr(const __u64 time, std::vector<MessageQueueWriter> & tx_msgq_list);

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

//
// A work handler should return false if there is no work do.
// A work handler should return true if it knows that there is more work to do.
// A work handler should return false if it executes very quickly.
// If a work handler sleeps for more than a millisecond then it should return true.
// A work handler should not take longer than a millisecond to execute if possible.
//
// The work handler is scheduled as often as possible so if it returns true all
// of the time and there is no sleeping then a lot of CPU will be used.
// A sleep for a millisecond will occur when the work handler returns false.
//
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
KuCommsTimerHandler::hlr(const __u64 time, std::vector<MessageQueueWriter> & tx_msgq_list)
{
}

///////////////////////////////////////////////////////////////

static bool g_stopped = false;

///////////////////////////////////////////////////////////////

void
terminate_signal_hanlder(int sig)
{
	g_stopped = true;
}

///////////////////////////////////////////////////////////////

int main(int argc, char ** argv)
{
	signal(SIGTERM, terminate_signal_hanlder);

	KuCommsMessageHandler msghlr;
	KuCommsWorkHandler workhlr;
	KuCommsTimerHandler timerhlr;

	bool ok = MessageManager::run(
				"/dev/kucomms_test",
				1024*1024,
				g_stopped,
				msghlr,
				workhlr,
				timerhlr);

	return 0;
}

///////////////////////////////////////////////////////////////
