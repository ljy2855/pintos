#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "filesys/file.h"

#define MAX_FD 127
#define STDOUT 1
#define STDIN 0

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
struct thread * get_child(tid_t);
struct file *get_file(int fd);
int insert_fd(struct file *f);
void delete_fd(int fd);
#endif /* userprog/process.h */
