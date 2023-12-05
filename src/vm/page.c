#include "page.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "userprog/pagedir.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "string.h"
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
        palloc_free_page(p->vaddr);
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