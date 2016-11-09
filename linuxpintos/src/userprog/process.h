#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

// Structure used when a new thread is created. Lets the parent thread wait
// until the child thread has been created.
struct child_arguments{
	struct semaphore load_semaphore;
	char* fn_copy;
	struct child_info* childinfo;
};

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

#endif /* userprog/process.h */
