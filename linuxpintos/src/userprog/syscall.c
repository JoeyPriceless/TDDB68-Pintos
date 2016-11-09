#include "userprog/syscall.h"
#include "userprog/process.h"
#include "userprog/pagedir.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#define max_batch 100

static void syscall_handler (struct intr_frame *);
void exit(int *f);
void halt(void);
void close(int *f);
int open(int *f);
int write(int *f);
int read(int *f);
int wait(int *f);
int exec(int *f);
void exit(int *f);
bool create(int *f);
bool check_addr(int *f, int n_args);
bool check_string(const char *s);

void syscall_init (void){
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

void halt(void){
	power_off();
}

void close(int *f){

	if(!check_addr(f,1)) exit(-1);

	int fd = (int)*(f+1);
	if(1 < fd && fd < TABLE_SIZE) {
		file_close(thread_current()->table[fd]);
		thread_current()->table[fd] = NULL;
	}
}

int open(int *f){

	const char* name = (char *)*(f+1);
	
	if(!check_addr(f,1)) exit(-1);
	if(!check_string(name)) exit(-1);

	int fd;
	for(fd = 2; fd < TABLE_SIZE; fd++){
		if(thread_current()->table[fd] == NULL){
			bool error = ((thread_current()->table[fd] = filesys_open(name)) == NULL);
			if (error) return -1;
			else return fd;
		}
	}
	return -1;
}

int write(int *f){

	int fd = (int)*(f+1);
	const void *buffer = (void *)*(f+2);
	int size = (int)*(f+3);

	if(!check_addr(f,3)) exit(-1);
	if(!check_string(buffer)) exit(-1);
	
	if(!(0 < fd && fd < TABLE_SIZE)) return -1;
	
	if (fd == 1){
		int counter = size;
		while(counter>max_batch){
			putbuf(buffer + (size - counter),max_batch);
			counter -= max_batch;
		}
		putbuf(buffer + (size - counter),counter);
		return size;
		}
	else if(thread_current()->table[fd] == NULL) return -1;
	else return file_write(thread_current()->table[fd],buffer,size);
}

int read(int *f){

	int fd = (int)*(f+1);
	void *buffer = (void *)*(f+2);
	int size = (int)*(f+3);

	if(!check_addr(f,3)) exit(-1);
	if(!check_string(buffer)) exit(-1);
	
	if(!(0 <= fd && fd < TABLE_SIZE)) return -1;
	
	if (fd == 0){
		int counter = size;
		while(counter-- > 0) input_getc();

		return size;
	}
	else if(thread_current()->table[fd] == NULL) return -1;
	else return file_read(thread_current()->table[fd],buffer,size);
}

int wait(int *f){

	int child_tid = f[1];

	if(!check_addr(f,1)) exit(-1);
	
	return process_wait(child_tid); // child exit code
}

int exec(int *f){

	const char* cmd_line = f[1];

	if(!check_addr(f,1)) exit(-1);
	if(!check_string(cmd_line)) exit(-1);

	return process_execute(cmd_line); //child pid
}

void exit(int *f){

	int exit_status;
	if(f == -1) exit_status = -1;
	else if(!check_addr(f,1)) exit_status = -1;
	else exit_status = f[1];
	
	thread_current()->childinfo->exit_status = exit_status;

	printf("%s: exit(%d)\n",thread_current()->name, exit_status);

	thread_exit();
}

bool create(int *f){

	const char *file_name = (char *)*(f+1);
	int size = (int)*(f+2);

	if(!check_addr(f,2)) exit(-1);
	if(!check_string(file_name)) exit(-1);

	if(file_name == NULL) exit(0);
	
	return filesys_create(file_name, size);
}

void seek (int *f){

	int fd = f[1];
	off_t pos = f[2];

	if(!check_addr(f,2)) exit(-1);

	return file_seek (thread_current()->table[fd], pos);

}

unsigned tell (int *f){

	int fd = f[1];
	
	if(!check_addr(f,1)) exit(-1);

	return file_tell (thread_current()->table[fd]); 
}

int file_size (int *f){

	int fd = f[1];

	if(!check_addr(f,1)) exit(-1);

	return file_length (thread_current()->table[fd]); 
}

bool remove(int *f){

	const char* file_name = f[1];

	if(!check_addr(f,1)) exit(-1);
	if(!check_string(file_name)) exit(-1);

	return filesys_remove (file_name);
}

bool check_addr(int *f, int n_args){
	if(f == NULL) return false;
	int length = n_args * 4;
	int offset;
	for(offset = 0; offset <= length; offset++){
		if(!is_user_vaddr(f+offset)  || pagedir_get_page(thread_current()->pagedir, f+offset) == NULL) return 0;
	}
	return 1;
}

bool check_string(const char *s){
	if(s == NULL) return false;
	int offset = 0;
	while(true){
		if(!is_user_vaddr(s+offset)  || pagedir_get_page(thread_current()->pagedir, s+offset) == NULL) return 0;
		else if(*((char*)(s+offset)) == '\0') return 1;
		offset++;
	}
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
	int *id = f->esp;

	if(!check_addr(id,1)) exit(-1);

	switch(*id){
		case SYS_CREATE:
			f->eax = create(id);
			break;
		case SYS_HALT:
			halt();
			break;
		case SYS_READ:
			f->eax = read(id);
			break;
		case SYS_WRITE:
			f->eax = write(id);
			break;
		case SYS_CLOSE:
			close(id);
			break;
		case SYS_EXIT:
			exit(id);
			break;
		case SYS_EXEC:
			f->eax = exec(id);
			break;
		case SYS_WAIT:
		    f->eax = wait(id);
			break;
		case SYS_OPEN:
			f->eax = open(id);
			break;
		case SYS_SEEK:
			seek(id);
			break;
		case SYS_TELL:
			f->eax = tell(id);
			break;
		case SYS_FILESIZE:
			f->eax = file_size(id);
			break;
		case SYS_REMOVE:
			f->eax = remove(id);
			break;
		default:
			printf("Nothing to see here");
	}
}
