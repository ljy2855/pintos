#ifndef VM_SWAP_H
#define VM_SWAP_H
#include "page.h"

void * swap_out();
void swap_init(void);
void read_swap_page(size_t disk_index, void * kaddr);

struct lock  alloc_lock;
#endif