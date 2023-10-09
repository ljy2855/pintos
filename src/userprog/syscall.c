#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "process.h"
#include "pagedir.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);
void check_valid_address(void * addr);

void halt (void);
tid_t exec (const char *cmd_line);
int wait (tid_t pid);
int read (int fd, void *buffer, unsigned size);
int write (int fd, const void *buffer, unsigned size);
int fibonacci(int n);
int max_of_four_int(int a,int b,int c,int d);


bool create (const char *file_name, unsigned initial_size);
bool remove (const char *file_name);
int open (const char *file_name);
int filesize (int fd);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

bool check_bad_fd(int fd){
  if(fd <= STDOUT || fd > MAX_FD)
    return false;
  return true;
}

bool check_unmapped_address(void * addr){
  struct thread * t = thread_current();
  void * pte = pagedir_get_page(t->pagedir,addr);
  if(pte == NULL)
    return false;
  return true;
}


void check_valid_address(void * addr){
  if (!is_user_vaddr(addr) || addr == NULL || addr <= 0x4000 || !check_unmapped_address(addr)){
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
  check_valid_address(f->esp);
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
    
    
    check_valid_address(((uint32_t*)f->esp+3));
    f->eax = write(*((uint32_t*)f->esp+1),*((uint32_t*)f->esp+2),*((uint32_t*)f->esp+3));
    break;
  case SYS_EXIT:
    //hex_dump(f->esp, f->esp, 100, 1);
    /**
     * esp[0] = system call number
     * esp[1] = status
    */
 
    check_valid_address(((uint32_t *)f->esp + 1)); // Check sc-bad-sp
    exit(*((uint32_t *)f->esp + 1));
    break;
  case SYS_WAIT:
    /**
     * esp[0] = system call number
     * esp[1] = wait pid
    */
 
    check_valid_address(((uint32_t *)f->esp + 1));
    f->eax = wait(*((uint32_t *)f->esp + 1));
    break;
  case SYS_EXEC:
    /**
     * esp[0] = system call number
     * esp[1] = cmd_line
    */
 
    check_valid_address(((uint32_t *)f->esp + 1)); 
    f->eax = exec(*((uint32_t *)f->esp + 1));
    break;
  case SYS_READ:
    /**
     * esp[0] = system call number
     * esp[1] = fd
     * esp[2] = buffer
     * esp[3] = size
    */


    check_valid_address(((uint32_t*)f->esp+3));
    f->eax = read(*((uint32_t*)f->esp+1),*((uint32_t*)f->esp+2),*((uint32_t*)f->esp+3));
    break;

  case SYS_OPEN:
    /**
     * esp[0] = system call number
     * esp[1] = file_name (buffer)
    */
    check_valid_address(((uint32_t *)f->esp + 1));
    f->eax = open((const char *)*((uint32_t *)f->esp + 1));
    break;
  case SYS_CLOSE:
    check_valid_address(((uint32_t *)f->esp + 1));
    close((int)*((uint32_t *)f->esp + 1));
    break;
  case SYS_CREATE:
    /**
     * esp[0] = system call number
     * esp[1] = file_name (buffer)
     * esp[2] = inital size
     */
    check_valid_address(((uint32_t *)f->esp + 2));
    f->eax = create(*((uint32_t *)f->esp + 1),*((uint32_t *)f->esp + 2));
    break;
  case SYS_REMOVE:
    check_valid_address(((uint32_t *)f->esp + 1));
    f->eax = remove((int)*((uint32_t *)f->esp + 1));
    break;
  
  case SYS_FILESIZE:
    check_valid_address(((uint32_t *)f->esp + 1));
    f->eax = filesize((int)*((uint32_t *)f->esp + 1));
    break;

  case SYS_SEEK:
    check_valid_address(((uint32_t *)f->esp + 1));
    seek((int)*((uint32_t *)f->esp + 1), *((uint32_t *)f->esp + 2));
    break;
  
  case SYS_TELL:
    check_valid_address(((uint32_t *)f->esp + 1));
    f->eax = tell((int)*((uint32_t *)f->esp + 1));
    break;
  case SYS_FIBO:
    /**
     * esp[0] = system call number
     * esp[1] = n
    */

    check_valid_address(((uint32_t*)f->esp+1));
    f->eax = fibonacci(*((uint32_t *)f->esp + 1));
    break;

  case SYS_MAX_FOUR:
    /**
     * esp[0] = system call number
     * esp[1] = a
     * esp[2] = b
     * esp[3] = c
     * esp[4] = d
    */


    check_valid_address(((uint32_t*)f->esp+4));
    f->eax = max_of_four_int(*((uint32_t*)f->esp+1),*((uint32_t*)f->esp+2),*((uint32_t*)f->esp+3),*((uint32_t*)f->esp+4));
    break;
  default:
    break;
  }


}

/* Proj 1*/
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
  if(!check_unmapped_address(cmd_line))
    return -1;

  tid_t tid = process_execute(cmd_line);
  struct thread * child_thread = get_child(tid);
  bool success;
  sema_down(&child_thread->load_lock);
  success = child_thread->load_success;
  if(!success)
    return -1;
  return tid;
}
int wait (tid_t pid){
  return process_wait(pid);
}

int write (int fd, const void *buffer, unsigned size){
  //printf("%d %s %d\n",fd,buffer,size);
  check_valid_address(buffer);
  if(fd == STDOUT){
    //if write stdout
    putbuf(buffer,size);
    return size;
  }
  if (!check_bad_fd(fd))
  {
    exit(-1);
  }
  struct file *f = get_file(fd);
  if (f == NULL)
    exit(-1);
  else
  {
    return file_write(f, buffer, size);
  }
  return -1;
}
int read (int fd, void *buffer, unsigned size){
  unsigned int i;
  check_valid_address(buffer);
  if(fd == STDIN){
    for(i = 0; i < size ; i++)
      *((uint8_t*)buffer+i) = (uint8_t)input_getc();
    return i;
  }
  if(!check_bad_fd(fd)){
    exit(-1);
  }
  struct file *f = get_file(fd);
  if(f == NULL)
    exit(-1);
  else{
    return file_read(f, buffer, size); 
  }
  return -1;
}

int fibonacci(int n){
  if(n == 0)
    return 0;
  if(n == 1)
    return 1;
  if(n  == 2)
    return 1;
  
  return fibonacci(n-1) + fibonacci(n-2);
}
int max_of_four_int(int a,int b,int c,int d){
  int max = a;

  if(max < b)
    max = b;
  if(max < c)
    max = c;
  if(max < d)
    max = d;
  return max;
}


/* Proj 2*/
int open(const char *file_name){
  check_valid_address(file_name);
  struct file * f = filesys_open(file_name);
  if(f == NULL)
    return -1;
  return insert_fd(f);
}

void close(int fd){
  //check close stdin, stdout or range out of fd_table
  if(fd <= STDOUT || fd > MAX_FD)
    exit(-1);
  delete_fd(fd);
}
bool create(const char *file_name, unsigned initial_size){
  check_valid_address(file_name);
  return filesys_create(file_name, initial_size);
}

bool remove(const char *file_name){
  check_valid_address(file_name);

  return filesys_remove(file_name);
}

int filesize(int fd){
  if(!check_bad_fd(fd))
    exit(-1);
  struct file *f = get_file(fd);
  if(f == NULL)
    return -1;
  return file_length(f);
}
void seek(int fd, unsigned position){
  if (!check_bad_fd(fd))
    exit(-1);
  struct file *f = get_file(fd);
  if (f == NULL)
    exit(-1);
  return file_seek(f, position);
}
unsigned tell(int fd){
  if (!check_bad_fd(fd))
    exit(-1);
  struct file *f = get_file(fd);
  if (f == NULL)
    return -1;
  return file_tell(f);
}