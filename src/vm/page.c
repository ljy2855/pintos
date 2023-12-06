#include "page.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "string.h"
#include "filesys/file.h"
static unsigned hash_func(const struct hash_elem *e, void *aux UNUSED){
    const struct vm_entry *p = hash_entry(e, struct vm_entry, elem);
    return hash_int(p->vaddr);
}

static bool hash_less(const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
    const struct vm_entry *left = hash_entry(a, struct vm_entry, elem);
    const struct vm_entry *right = hash_entry(b, struct vm_entry, elem);
    return left->vaddr < right->vaddr;
}


static void hash_free_func(struct hash_elem *e, void *aux)
{
    struct thread *t = (struct thread *)aux;
    struct vm_entry *p = hash_entry(e, struct vm_entry, elem);
    if(p->is_loaded){
        palloc_free_page(p->kpage);
        pagedir_clear_page(t->pagedir,p->vaddr);
    }
    free(p); 
}

void init_vm_table(struct hash *table)
{

    hash_init(table, hash_func, hash_less, thread_current());
}

bool insert_vm_entry(struct hash *table, struct vm_entry *entry){
    struct hash_elem * old = hash_insert(table, &entry->elem);
    if(old == NULL)
        return true;
    else
        return false;
}
bool delete_vm_entry(struct hash *table, struct vm_entry *entry){
    struct hash_elem *removed = hash_delete(table, &entry->elem);
    if(removed == NULL)
        return false;
    else
        return true;
}

struct vm_entry *find_vm_entry(struct hash *table,void *addr){
    struct vm_entry temp;
    struct vm_entry *found_entry = NULL;

    memset(&temp, 0, sizeof(temp));
    temp.vaddr = pg_round_down(addr);

    struct hash_elem *found = hash_find(table, &temp.elem);

        
    if (found)
    {

        found_entry = hash_entry(found, struct vm_entry, elem);
    }

    return found_entry;
}

void destroy_table(struct hash *table){
    hash_apply(table, hash_free_func);
    hash_destroy(table, NULL);
}

bool load_mmap_entry(struct mmap_entry * entry,void * upage){
    uint32_t read_bytes = file_length(entry->file);
    uint32_t zero_bytes = PGSIZE - (read_bytes % PGSIZE);
    ASSERT ((read_bytes + zero_bytes) % PGSIZE == 0);
    ASSERT (pg_ofs (upage) == 0);
    if(read_bytes ==0)
        return false;
    struct thread *t = thread_current();
    off_t ofs = 0;
    while (read_bytes > 0 || zero_bytes > 0) 
    {
      /* Calculate how to fill this page.
         We will read PAGE_READ_BYTES bytes from FILE
         and zero the final PAGE_ZERO_BYTES bytes. */
      size_t page_read_bytes = read_bytes < PGSIZE ? read_bytes : PGSIZE;
      size_t page_zero_bytes = PGSIZE - page_read_bytes;

      struct vm_entry *new = (struct vm_entry *)malloc(sizeof(struct vm_entry));
      if(new == NULL)
        return false;
      memset(new, 0, sizeof(struct vm_entry));
      new->type = VM_FILE;
      new->vaddr = upage;
      new->writable = true;
      new->read_bytes = page_read_bytes;
      new->zero_bytes = page_zero_bytes;
      new->file = entry->file;
      new->offset = ofs;
      new->is_loaded = true;
      ASSERT(insert_vm_entry(&t->vm_table, new));
      list_push_back(&entry->vme_list,&new->mmap_elem);

      /* Advance. */
      read_bytes -= page_read_bytes;
      zero_bytes -= page_zero_bytes;
      ofs += page_read_bytes;
      upage += PGSIZE;

        }
  return true;

    

}

void remove_mmap_entry(struct mmap_entry *entry)
{
    struct vm_entry *mmap_vm_entry = NULL;
    struct list_elem *e;
    struct thread *t = thread_current();
    for (e = list_begin(&entry->vme_list); e != list_end(&entry->vme_list); e = list_next(e))
    {
        mmap_vm_entry = list_entry(e, struct vm_entry, mmap_elem);
        if (pagedir_is_dirty(t->pagedir, mmap_vm_entry->vaddr))
        {
             file_write_at(entry->file, mmap_vm_entry->kpage, PGSIZE, mmap_vm_entry->offset);

        }
        delete_vm_entry(&t->vm_table, mmap_vm_entry);
    }
}