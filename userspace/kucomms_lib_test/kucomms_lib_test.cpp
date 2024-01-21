///////////////////////////////////////////////////////////////

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <signal.h>

#include <kucomms/MessageManager.h>

///////////////////////////////////////////////////////////////

class KuCommsMessageHandler : public MessageHandler
{
public:
	KuCommsMessageHandler()
	{
	}

	bool hlr(const struct Message * message, MessageQueueWriter & tx_msgq, std::vector<MessageQueueWriter> & tx_msgq_list);

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
KuCommsMessageHandler::hlr(const struct Message * message, MessageQueueWriter & tx_msgq, std::vector<MessageQueueWriter> & tx_msgq_list)
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

//
// kucomms_test /dev/kucomms_test
//
int main(int argc, char ** argv)
{
	if (argc != 2) {
		printf("Must specify one device argument ( i.e. /dev/kucomms_myname)\n");
		return(-1);
	}

	signal(SIGTERM, terminate_signal_hanlder);

	KuCommsMessageHandler msghlr;
	KuCommsWorkHandler workhlr;
	KuCommsTimerHandler timerhlr;

	bool ok = MessageManager::run(
				argv[1],
				1024*1024,
				g_stopped,
				msghlr,
				workhlr,
				timerhlr);

	return 0;
}

///////////////////////////////////////////////////////////////
