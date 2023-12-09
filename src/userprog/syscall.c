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
#include "threads/synch.h"
#include "vm/page.h"
#include "string.h"
#include "threads/palloc.h"


static void syscall_handler(struct intr_frame *);
void check_valid_address(void *addr);

void halt(void);
tid_t exec(const char *cmd_line);
int wait(tid_t pid);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
int fibonacci(int n);
int max_of_four_int(int a, int b, int c, int d);

bool create(const char *file_name, unsigned initial_size);
bool remove(const char *file_name);
int open(const char *file_name);
int filesize(int fd);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);

mapid_t mmap(int fd, void *addr);
void munmap(mapid_t mapid);

struct semaphore file_write_lock;
struct semaphore file_read_lock;
struct semaphore file_lock;
struct lock mapid_lock;
unsigned read_cnt;

/**
 * Check fd is out of range
 */
bool check_bad_fd(int fd)
{
  if (fd <= STDOUT || fd > MAX_FD)
    return false;
  return true;
}
/**
 * Check bad memory access by vm_entry
*/
bool check_vm_address(void *addr, bool write)
{
  struct vm_entry *entry = find_vm_entry(&thread_current()->vm_table, addr);
  if (entry == NULL)
  {
    return false;
  }

  if (!entry->writable && write)
    return false;
  return true;
}
/**
 * Check bad buffer address 
*/
bool check_valid_buffer(void *addr, unsigned size, bool write)
{
  size_t cur;
  bool success = true;
  for (cur = 0; cur < size; cur += PGSIZE)
  {
    if (!check_vm_address(addr + cur, write))
    {
      success = false;
      break;
    }
  }
  return success;
}


void check_valid_address(void *addr)
{
  if (!is_user_vaddr(addr) || addr < (void *)0x08048000 || !check_vm_address(addr, false))
  {
    exit(-1);
  }
}

void syscall_init(void)
{
  sema_init(&file_write_lock, 1);
  sema_init(&file_read_lock, 1);
  sema_init(&file_lock, 1);
  lock_init(&mapid_lock);
  read_cnt = 0;
  intr_register_int(0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler(struct intr_frame *f UNUSED)
{
  check_valid_address(f->esp);
  int call_number = *(int *)f->esp;

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

    check_valid_address(((uint32_t *)f->esp + 3));
    f->eax = write(*((uint32_t *)f->esp + 1), *((uint32_t *)f->esp + 2), *((uint32_t *)f->esp + 3));
    break;
  case SYS_EXIT:
    // hex_dump(f->esp, f->esp, 100, 1);
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

    check_valid_address(((uint32_t *)f->esp + 3));
    f->eax = read(*((uint32_t *)f->esp + 1), *((uint32_t *)f->esp + 2), *((uint32_t *)f->esp + 3));
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
    f->eax = create(*((uint32_t *)f->esp + 1), *((uint32_t *)f->esp + 2));
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
    check_valid_address(((uint32_t *)f->esp + 2));
    seek((int)*((uint32_t *)f->esp + 1), *((uint32_t *)f->esp + 2));
    break;

  case SYS_TELL:
    check_valid_address(((uint32_t *)f->esp + 1));
    f->eax = tell((int)*((uint32_t *)f->esp + 1));
    break;

  case SYS_MMAP:
    check_valid_address(((uint32_t *)f->esp + 2));
    f->eax = mmap((int)*((uint32_t *)f->esp + 1), (void *)*((uint32_t *)f->esp + 2));
    break;

  case SYS_MUNMAP:
    check_valid_address(((uint32_t *)f->esp + 1));
    munmap((mapid_t) * ((uint32_t *)f->esp + 1));
    break;
  case SYS_FIBO:
    /**
     * esp[0] = system call number
     * esp[1] = n
     */

    check_valid_address(((uint32_t *)f->esp + 1));
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

    check_valid_address(((uint32_t *)f->esp + 4));
    f->eax = max_of_four_int(*((uint32_t *)f->esp + 1), *((uint32_t *)f->esp + 2), *((uint32_t *)f->esp + 3), *((uint32_t *)f->esp + 4));
    break;
  default:
    break;
  }
}

/* Proj 1*/
void halt(void)
{
  shutdown_power_off();
}

void exit(int status)
{
  struct thread *current_thread = thread_current();
  current_thread->exit_num = status;
  printf("%s: exit(%d)\n", thread_name(), status);
  for (struct list_elem *e = list_begin(&current_thread->mmap_list); e != list_end(&current_thread->mmap_list);)
  {
    struct mmap_entry *entry = list_entry(e, struct mmap_entry, elem);
    sema_down(&file_lock);
    remove_mmap_entry(entry);
    sema_up(&file_lock);
    e = list_next(e);
    free(entry);
  }
  
  thread_exit();
}

tid_t exec(const char *cmd_line)
{
  // check cmd_line buffer is point wrong address
  if (!check_vm_address(cmd_line, false))
    return -1;
  sema_down(&file_lock);
  tid_t tid = process_execute(cmd_line);
  struct thread *child_thread = get_child(tid);
  bool success;

  // wait child process load file
  sema_down(&child_thread->load_lock);
  success = child_thread->load_success;
  sema_up(&file_lock);
  if (!success)
  {
    return -1;
  }

  return tid;
}

int wait(tid_t pid)
{
  return process_wait(pid);
}

int write(int fd, const void *buffer, unsigned size)
{
  if (!check_valid_buffer(buffer, size, false))
  {
    exit(-1);
  }
  unsigned writen_bytes;
  if (fd == STDOUT)
  {
    // if write stdout
    putbuf(buffer, size);
    return size;
  }
  if (!check_bad_fd(fd))
  {
    exit(-1);
  }
  struct file *f = get_file(fd);
  if (f == NULL)
    exit(-1);

  sema_down(&file_write_lock);
  writen_bytes = file_write(f, buffer, size);
  sema_up(&file_write_lock);

  return writen_bytes;
}
int read(int fd, void *buffer, unsigned size)
{
  unsigned int i;
  unsigned readn_bytes;
  if (!check_valid_buffer(buffer, size, true))
  {
    exit(-1);
  }
  if (fd == STDIN)
  {
    for (i = 0; i < size; i++)
      *((uint8_t *)buffer + i) = (uint8_t)input_getc();
    return i;
  }
  if (!check_bad_fd(fd))
  {
    exit(-1);
  }
  struct file *f = get_file(fd);
  if (f == NULL)
    exit(-1);

  sema_down(&file_read_lock);
  read_cnt++;
  if (read_cnt)
    sema_down(&file_write_lock);
  sema_up(&file_read_lock);

  readn_bytes = file_read(f, buffer, size);

  sema_down(&file_read_lock);
  read_cnt--;
  if (read_cnt == 0)
    sema_up(&file_write_lock);
  sema_up(&file_read_lock);

  return readn_bytes;
}

int fibonacci(int n)
{
  if (n == 0)
    return 0;
  if (n == 1)
    return 1;
  if (n == 2)
    return 1;

  return fibonacci(n - 1) + fibonacci(n - 2);
}
int max_of_four_int(int a, int b, int c, int d)
{
  int max = a;

  if (max < b)
    max = b;
  if (max < c)
    max = c;
  if (max < d)
    max = d;
  return max;
}

/* Proj 2*/
int open(const char *file_name)
{
  if (!check_vm_address(file_name, false))
  {
    exit(-1);
  }
  sema_down(&file_lock);
  struct file *f = filesys_open(file_name);
  sema_up(&file_lock);
  if (f == NULL)
  {
    // PANIC("FILE NOT FOUND");
    return -1;
  }

  return insert_fd(f);
}

void close(int fd)
{
  // check close stdin, stdout or range out of fd_table
  if (!check_bad_fd(fd))
    exit(-1);

  delete_fd(fd);
}
bool create(const char *file_name, unsigned initial_size)
{
  if (!check_vm_address(file_name, false))
  {
    exit(-1);
  }
  sema_down(&file_lock);
  bool success = filesys_create(file_name, initial_size);
  sema_up(&file_lock);
  return success;
}

bool remove(const char *file_name)
{
  if (!check_vm_address(file_name, false))
  {
    exit(-1);
  }
  sema_down(&file_lock);
  bool success = filesys_remove(file_name);
  sema_up(&file_lock);
  return success;
}

int filesize(int fd)
{
  if (!check_bad_fd(fd))
    exit(-1);
  struct file *f = get_file(fd);
  if (f == NULL)
    return -1;
  return file_length(f);
}
void seek(int fd, unsigned position)
{
  if (!check_bad_fd(fd))
    exit(-1);
  struct file *f = get_file(fd);
  if (f == NULL)
    exit(-1);
  file_seek(f, position);
}
unsigned tell(int fd)
{
  if (!check_bad_fd(fd))
    exit(-1);
  struct file *f = get_file(fd);
  if (f == NULL)
    return -1;
  return file_tell(f);
}

static mapid_t
allocate_mapid(void)
{
  static mapid_t next_mapid = 1;
  mapid_t mapid;

  lock_acquire(&mapid_lock);
  mapid = next_mapid++;
  lock_release(&mapid_lock);

  return mapid;
}

mapid_t mmap(int fd, void *addr)
{

  if (!check_bad_fd(fd))
    exit(-1);
  struct file *f = get_file(fd);
  unsigned file_size = file_length(f);
  if (f == NULL)
    return -1;

  if (!is_user_vaddr(addr + file_size) 
  || find_vm_entry(&thread_current()->vm_table, addr) 
  || addr < (void *)0x08048000 
  || pg_ofs(addr) != 0)
    return -1;
    
  struct mmap_entry *new = malloc(sizeof(struct mmap_entry));
  memset(new, 0, sizeof(struct mmap_entry));
  new->file = file_reopen(f);
  new->map_id = allocate_mapid();
  list_push_back(&thread_current()->mmap_list, &new->elem);
  list_init(&new->vme_list);
  if (load_mmap_entry(new, addr))
    return new->map_id;
  return -1;
}

void munmap(mapid_t mapid)
{
  struct list_elem *e;
  struct mmap_entry *entry = NULL;
  struct thread *t = thread_current();

  for (e = list_begin(&t->mmap_list); e != list_end(&t->mmap_list); e = list_next(e))
  {
    entry = list_entry(e, struct mmap_entry, elem);
    if (entry->map_id == mapid)
      break;
  }
  if (entry != NULL)
  {
    sema_down(&file_lock);
    remove_mmap_entry(entry);
    sema_up(&file_lock);
    list_remove(&entry->elem);
    free(entry);
    }
}
