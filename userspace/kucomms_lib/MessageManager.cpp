///////////////////////////////////////////////////////////////

#include "MessageManager.h"

#include <cstdio>
#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdlib.h>

///////////////////////////////////////////////////////////////

bool
MessageManager_MessageHandler(const struct Message * message, MessageQueueHeaderPtr tx_msgq, const __u64 rx_msgq_queueLength, const __u64 tx_msgq_queueLength, void * userData)
{
	try {
		MessageManagerUserData * data = (MessageManagerUserData *)userData;

		std::vector<MessageQueueWriter> & mqlist = *data->m_tx_msgq;

		for (uint32_t u0=0; u0<mqlist.size(); u0++) {
			if (mqlist[u0].get_mq() == tx_msgq) {
				bool ok = data->m_msghlr->hlr(message, mqlist[u0], mqlist);
				return ok;
			}
		}
	}
	catch (const std::exception & e) {
		printf("MessageManager_MessageHandler : Exception %s\n", e.what());
	}
	catch (...) {
		printf("MessageManager_MessageHandler : Exception caught in ...\n");
	}
	return false;
}

///////////////////////////////////////////////////////////////

bool
MessageManager_WorkHandler(void * userData)
{
	try {
		MessageManagerUserData * data = (MessageManagerUserData *)userData;
		bool ok = data->m_workhlr->hlr(*data->m_tx_msgq);
		return ok;
	}
	catch (const std::exception & e) {
		printf("MessageManager_WorkHandler : Exception %s\n", e.what());
	}
	catch (...) {
		printf("MessageManager_WorkHandler : Exception caught in ...\n");
	}
	return false;
}

///////////////////////////////////////////////////////////////

void
MessageManager_TimerHandler(const __u64 time, void * userData)
{
	try {
		MessageManagerUserData * data = (MessageManagerUserData *)userData;
		data->m_timerhlr->hlr(time, *data->m_tx_msgq);
	}
	catch (const std::exception & e) {
		printf("MessageManager_TimerHandler : Exception %s\n", e.what());
	}
	catch (...) {
		printf("MessageManager_TimerHandler : Exception caught in ...\n");
	}
}

///////////////////////////////////////////////////////////////

MessageManager::MessageManager(
	MessageHandler & msghlr,
	WorkHandler & workhlr,
	TimerHandler & timerhlr,
	std::vector<MessageQueueReader> & rx_msgq,
	std::vector<MessageQueueWriter> & tx_msgq) :
		m_msghlr(msghlr),
		m_workhlr(workhlr),
		m_timerhlr(timerhlr),
		m_rx_msgq(rx_msgq),
		m_tx_msgq(tx_msgq)
{
	if (rx_msgq.size() != tx_msgq.size()) return;
	if (rx_msgq.size() == 0) return;

	m_userData.m_msghlr = &m_msghlr;
	m_userData.m_workhlr = &m_workhlr;
	m_userData.m_timerhlr = &m_timerhlr;
	m_userData.m_tx_msgq = &m_tx_msgq;

	bool ok = message_manager_init(
			&m_msgmgr,
			MessageManager_MessageHandler,
			MessageManager_WorkHandler,
			MessageManager_TimerHandler,
			&m_userData);

	if (!ok) return;

	for (uint32_t u0=0; u0<m_rx_msgq.size(); u0++) {
		ok = message_manager_add_msgq(
			&m_msgmgr,
			m_rx_msgq[u0].get_mq(),
			m_tx_msgq[u0].get_mq(),
			m_rx_msgq[u0].get_length(),
			m_tx_msgq[u0].get_length());

		if (!ok) return;

		if (m_rx_msgq[u0].get_length() > m_maxqLength) m_maxqLength = m_rx_msgq[u0].get_length();
	}

	m_initialised = true;
}

///////////////////////////////////////////////////////////////

MessageManager::~MessageManager()
{
	stop();
}

///////////////////////////////////////////////////////////////

bool
MessageManager::start()
{
	if (!m_initialised) return false;
	if (m_thread) return false;
	m_stopped = false;
	m_thread.reset(new std::thread(&MessageManager::threadFunction,this));
	return true;
}

///////////////////////////////////////////////////////////////

bool
MessageManager::stop()
{
	if (!m_thread) return true;
	m_stopped = true;
	m_thread->join();
	m_thread.reset();
	return true;
}

///////////////////////////////////////////////////////////////

bool
MessageManager::run(
	const std::string & devname,
	uint64_t length,
	bool & stopped,
	MessageHandler & msghlr,
	WorkHandler & workhlr,
	TimerHandler & timerhlr)
{
	int fd = ::open(devname.c_str(), O_RDWR);

	if (fd < 0) {
		printf("main : Error from open\n");
		return false;
	}

	int prot = PROT_READ | PROT_WRITE;
	int flags = MAP_SHARED;

	void * ptr = ::mmap(0, length, prot, flags, fd, 0);

	if (ptr == MAP_FAILED) {
		printf("main : Error from mmap\n");
		::close(fd);
		return false;
	}

	bool ok = MessageManager::run(
		((uint8_t*)ptr)+(length/2),
		length/2,
		ptr,
		length/2,
		false,
		stopped,
		msghlr,
		workhlr,
		timerhlr);

	::close(fd);

	int ret = ::munmap(ptr, length);
	if (ret == -1) {
		printf("main : Error from munmap\n");
		return false;
	}

	return ok;
}

///////////////////////////////////////////////////////////////

bool
MessageManager::run(
	void * rx_msgq,
	const uint64_t rx_msgq_len,
	void * tx_msgq,
	const uint64_t tx_msgq_len,
	const bool init,
	bool & stopped,
	MessageHandler & msghlr,
	WorkHandler & workhlr,
	TimerHandler & timerhlr)
{
	std::vector<MessageQueueReader> rx_msgq_list(1);
	std::vector<MessageQueueWriter> tx_msgq_list(1);

	if (init) {
		rx_msgq_list[0].init((uint8_t*)rx_msgq, rx_msgq_len);
		tx_msgq_list[0].init((uint8_t*)tx_msgq, tx_msgq_len);
	} else {
		rx_msgq_list[0].init((uint8_t*)rx_msgq);
		tx_msgq_list[0].init((uint8_t*)tx_msgq);
	}

	MessageManager mmgr(msghlr,workhlr,timerhlr,rx_msgq_list,tx_msgq_list);

	bool ok = mmgr.start();
	if (!ok) {
		printf("main : Error from mmgr.start\n");
		return false;
	}

	while (!stopped) {
		::usleep(500*1000);
	}

	ok = mmgr.stop();

	return ok;
}

///////////////////////////////////////////////////////////////

void
MessageManager::threadFunction()
{
	bool ok = message_manager_run(&m_msgmgr, m_maxqLength, &m_stopped);
}

///////////////////////////////////////////////////////////////
