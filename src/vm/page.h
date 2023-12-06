#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include <list.h>
#include <stdlib.h>
#include <threads/thread.h>

typedef int mapid_t ;

enum vm_type
{
    VM_BIN,
    VM_FILE,
    VM_ANON,
};

struct vm_entry{
    enum vm_type type;
    void *vaddr;
    void *kpage;
    bool writable;
    bool is_loaded;
    struct file *file;
    size_t offset;
    size_t read_bytes;
    size_t zero_bytes;
    struct hash_elem elem;

    struct thread *t;
    struct list_elem mmap_elem;

    size_t swap_index;
};

struct mmap_entry{
    mapid_t map_id;
    struct file * file;
    struct list_elem elem;
    struct list vme_list;
};

struct page{
    struct vm_entry *entry;
    struct list_elem elem;
    void *kaddr;
};

struct list lru_pages;

void init_vm_table(struct hash *table);
bool insert_vm_entry(struct hash *table, struct vm_entry *entry);
bool delete_vm_entry(struct hash *table, struct vm_entry *entry);
struct vm_entry *find_vm_entry(struct hash *table, void *addr);
void destroy_table(struct hash *table);

bool load_mmap_entry(struct mmap_entry * entry,void * upage);
void remove_mmap_entry(struct mmap_entry *entry);

#endif