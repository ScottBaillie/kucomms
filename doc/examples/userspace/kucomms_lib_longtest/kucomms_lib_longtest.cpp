///////////////////////////////////////////////////////////////

#include <cstdlib>
#include <iostream>
#include <map>
#include <cstring>

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

__u64 g_sequence_wr = 0;
__u64 g_sequence_rd = 0;

std::map<__u64, std::vector<__u8> > g_id_to_message;

__u64 g_message_sent_count_0 = 0;
__u64 g_message_sent_count_1 = 0;
__u64 g_message_received_count_0 = 0;
__u64 g_message_received_count_1 = 0;
__u64 g_message_compare_error_count = 0;
__u64 g_message_add_error_count = 0;

__u64 g_timer_counter = 0;

///////////////////////////////////////////////////////////////

bool
KuCommsMessageHandler::hlr(const struct Message * message, MessageQueueWriter & tx_msgq, std::vector<MessageQueueWriter> & tx_msgq_list)
{
	if (message->m_type == 1) {
		g_message_received_count_1++;
		bool ok = tx_msgq_list[0].add(message);
		if (!ok) g_message_add_error_count++;
		g_message_sent_count_1++;
		return true;
	}
	g_message_received_count_0++;

	if (message->m_id != g_sequence_rd) {
		g_message_compare_error_count++;
		return true;
	}

	std::vector<__u8> & msgbuf = g_id_to_message[g_sequence_rd];

	if (msgbuf.size() != message_get_message_length(message->m_length)) {
		g_message_compare_error_count++;
		return true;
	}

	if (memcmp(msgbuf.data(),message,msgbuf.size()) != 0) {
		g_message_compare_error_count++;
		return true;
	}

	g_id_to_message.erase(g_sequence_rd);

	g_sequence_rd++;

	return true;
}

///////////////////////////////////////////////////////////////

void
format_message(DataMessage & msg, const __u64 id)
{
	msg.get()->m_type = 0;
	msg.get()->m_id = id;
	msg.get()->m_userValue = 0;
	for (__u32 u0=0; u0<msg.get_data_length(); u0++) msg.get_data()[u0] = rand() % 256;
}

bool
KuCommsWorkHandler::hlr(std::vector<MessageQueueWriter> & tx_msgq_list)
{
	bool ok;

	for (__u32 u0=0; u0<4; u0++) {
		DataMessage msg(rand() % 2048);
		format_message(msg, g_sequence_wr);

		std::vector<__u8> & msgbuf = g_id_to_message[g_sequence_wr];
		msgbuf.resize(msg.get_message_length());
		memcpy(msgbuf.data(), msg.get(), msg.get_message_length());

		ok = tx_msgq_list[0].add(msg.get());
		if (!ok) g_message_add_error_count++;
		g_message_sent_count_0++;

		g_sequence_wr++;
	}

	return false;
}

///////////////////////////////////////////////////////////////

void
KuCommsTimerHandler::hlr(const __u64 time, std::vector<MessageQueueWriter> & tx_msgq_list)
{
	g_timer_counter++;
	if ((g_timer_counter%10) == 0) {

		printf("avail_tx0=%llu : add_error_count=%llu : compare_error_count=%llu\n",
			tx_msgq_list[0].get_avail(),
			g_message_add_error_count,
			g_message_compare_error_count);

		printf("sent_count_0=%llu : sent_count_1=%llu : received_count_0=%llu : received_count_1=%llu\n",
			g_message_sent_count_0,
			g_message_sent_count_1,
			g_message_received_count_0,
			g_message_received_count_1);
	}
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
// kucomms_longtest /dev/kucomms_longtest
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
