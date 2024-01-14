/**********************************************************/

#include "message_manager.h"

/**********************************************************/

#define KUCOMMS_FNAME_SIZE 64

/**********************************************************/

struct kucomms_callback_data
{
	MessageHandler_C msghlr;
	WorkHandler_C workhlr;
	TimerHandler_C timerhlr;
	void * userData;
	struct Message * message;
	char filename[KUCOMMS_FNAME_SIZE];
	__u32 filename_len;
	bool open;
};

/**********************************************************/

struct kucomms_file_data
{
	void * vaddr;
	__u64 vaddr_len;
	struct task_struct *thread;
	struct kucomms_callback_data cbdata;
	MessageQueueHeaderPtr rx_msgq;
	MessageQueueHeaderPtr tx_msgq;
	struct MessageManagerStruct msgmgr;
};

/**********************************************************/

void kucomms_callback_list_init(void);

struct kucomms_callback_data * kucomms_find_and_open(const char* name, __u32 len, bool * open);

void kucomms_find_and_close(const char* name, __u32 len);

bool
kucomms_register(
	const char* name,
	__u32 len,
	MessageHandler_C msghlr,
	WorkHandler_C workhlr,
	TimerHandler_C timerhlr,
	void * userData);

bool kucomms_unregister(const char* name, __u32 len);

/**********************************************************/
