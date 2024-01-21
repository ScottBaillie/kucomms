///////////////////////////////////////////////////////////////

#ifndef KUCOMMS_MESSAGEMANAGER_H
#define KUCOMMS_MESSAGEMANAGER_H

///////////////////////////////////////////////////////////////

#include "MessageQueueWriter.h"
#include "MessageQueueReader.h"

#define KERNEL_BUILD 0

extern "C" {
#include <kucomms/message_manager.h>
}

#include <memory>
#include <thread>

///////////////////////////////////////////////////////////////

class MessageHandler
{
public:
	virtual ~MessageHandler() {}

	virtual bool hlr(const struct Message * message, MessageQueueWriter & tx_msgq, std::vector<MessageQueueWriter> & tx_msgq_list) = 0;
};

///////////////////////////////////////////////////////////////

class WorkHandler
{
public:
	virtual ~WorkHandler() {}

	virtual bool hlr(std::vector<MessageQueueWriter> & tx_msgq_list) = 0;
};

///////////////////////////////////////////////////////////////

class TimerHandler
{
public:
	virtual ~TimerHandler() {}

	virtual void hlr(const __u64 time, std::vector<MessageQueueWriter> & tx_msgq_list) = 0;
};

///////////////////////////////////////////////////////////////

struct MessageManagerUserData
{
	MessageHandler *			m_msghlr = 0;
	WorkHandler *				m_workhlr = 0;
	TimerHandler *				m_timerhlr = 0;
	std::vector<MessageQueueWriter> *	m_tx_msgq = 0;
};

///////////////////////////////////////////////////////////////

class MessageManager
{
public:
	MessageManager(
		MessageHandler & msghlr,
		WorkHandler & workhlr,
		TimerHandler & timerhlr,
		std::vector<MessageQueueReader> & rx_msgq,
		std::vector<MessageQueueWriter> & tx_msgq);

	~MessageManager();

	bool start();

	bool stop();

	static bool run(
		const std::string & devname,
		uint64_t length,
		bool & stopped,
		MessageHandler & msghlr,
		WorkHandler & workhlr,
		TimerHandler & timerhlr);

	static bool run(
		void * rx_msgq,
		const uint64_t rx_msgq_len,
		void * tx_msgq,
		const uint64_t tx_msgq_len,
		const bool init,
		bool & stopped,
		MessageHandler & msghlr,
		WorkHandler & workhlr,
		TimerHandler & timerhlr);

	static bool run(
		void * rx_msgq_1,
		const uint64_t rx_msgq_len_1,
		void * tx_msgq_1,
		const uint64_t tx_msgq_len_1,
		void * rx_msgq_2,
		const uint64_t rx_msgq_len_2,
		void * tx_msgq_2,
		const uint64_t tx_msgq_len_2,
		const bool init,
		bool & stopped,
		MessageHandler & msghlr,
		WorkHandler & workhlr,
		TimerHandler & timerhlr);

	static bool run(
		std::vector<MessageQueueReader> & rx_msgq_list,
		std::vector<MessageQueueWriter> & tx_msgq_list,
		bool & stopped,
		MessageHandler & msghlr,
		WorkHandler & workhlr,
		TimerHandler & timerhlr);

private:
	void threadFunction();

private:
	MessageHandler &			m_msghlr;
	WorkHandler &				m_workhlr;
	TimerHandler &				m_timerhlr;
	std::vector<MessageQueueReader>	&	m_rx_msgq;
	std::vector<MessageQueueWriter> &	m_tx_msgq;
	struct MessageManagerStruct		m_msgmgr;
	std::unique_ptr<std::thread>		m_thread;
	bool					m_stopped = false;
	MessageManagerUserData			m_userData;
	__u64					m_maxqLength = 0;
	bool					m_initialised = false;
};

///////////////////////////////////////////////////////////////

#endif

///////////////////////////////////////////////////////////////
