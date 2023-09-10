#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "process.h"
#define STDOUT 1
#define STDIN 0
static void syscall_handler (struct intr_frame *);
void check_valid_address(void * addr);

void exit (int status);
void halt (void);
tid_t exec (const char *cmd_line);
int wait (tid_t pid);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);

//bool create (const char *file, unsigned initial_size);
//bool remove (const char *file);
//int open (const char *file);
//int filesize (int fd);
//void seek (int fd, unsigned position);
//unsigned tell (int fd);
//void close (int fd);

void check_valid_address(void * addr){
  if (!is_user_vaddr(addr)){
    exit(-1);
  }
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  int call_number =  *(int*)f->esp;
  switch (call_number)
  {
  case SYS_HALT:
    /**
     * terminate pintos
    */
    halt();
    
    break;
  case SYS_WRITE:
    /**
     * esp[0] = system call number
     * esp[1] = fd
     * 
     * esp[2] = buffer
     * esp[3] = size
    */
    
    check_valid_address(f->esp);
    f->eax = write(*((uint32_t*)f->esp+1),*((uint32_t*)f->esp+2),*((uint32_t*)f->esp+3));
    break;
  case SYS_EXIT:
    //hex_dump(f->esp, f->esp, 100, 1);
    /**
     * esp[0] = system call number
     * esp[1] = status
    */
    check_valid_address(f->esp);
    exit(*((uint32_t *)f->esp + 1));
    break;
  case SYS_WAIT:
    /**
     * esp[0] = system call number
     * esp[1] = wait pid
    */
    check_valid_address(f->esp);
    wait(*((uint32_t *)f->esp + 1));

  case SYS_EXEC:
    /**
     * esp[0] = system call number
     * esp[1] = cmd_line
    */
    check_valid_address(f->esp);
    f->eax = exec(*((uint32_t *)f->esp + 1));
  case SYS_READ:
    /**
     * esp[0] = system call number
     * esp[1] = fd
     * esp[2] = buffer
     * esp[3] = size
    */
    check_valid_address(f->esp);
    f->eax = read(*((uint32_t*)f->esp+1),*((uint32_t*)f->esp+2),*((uint32_t*)f->esp+3));
  default:
    break;
  }

  //printf ("system call! %d\n",call_number);
  //thread_exit ();
}
void halt(void){
  shutdown_power_off();
}

void exit (int status){
  struct thread * current_thread = thread_current();
  current_thread->exit_num = status;
  printf("%s: exit(%d)\n", thread_name(),status);
  thread_exit();
}
tid_t exec (const char *cmd_line){
  return process_execute(cmd_line);
}
int wait (tid_t pid){
  return process_wait(pid);
}

int write (int fd, const void *buffer, unsigned size){
  //printf("%d %s %d\n",fd,buffer,size);
  if(fd == STDOUT){
    //if write stdout
    putbuf(buffer,size);
    return size;
  }
  return -1;
}
int read (int fd, void *buffer, unsigned size){
  unsigned int i;
  if(fd == STDIN){
    for(i = 0; i < size ; i++)
      *((uint8_t*)buffer+1) = (uint8_t)input_getc();
    return i;
  }
  return -1;
}