#include "frame.h"
#include <debug.h>
struct page *alloc_page(enum palloc_flags flags){
    // lock_acquire(&alloc_lock);
    void *kaddr = palloc_get_page(flags);
    struct page *new_page;
    if (kaddr == NULL)
    {
        //TODO SWAP OUT
        kaddr = swap_out();
        free_page(kaddr);
        kaddr = palloc_get_page(flags);

    }
    ASSERT(kaddr);
    new_page = malloc(sizeof(struct page));
    new_page->kaddr = kaddr;
    list_push_back(&lru_pages, &new_page->elem);
    
    ASSERT(new_page != NULL);
    // lock_release(&alloc_lock);
    return new_page;
}
void free_page(void *kaddr){
    // lock_acquire(&alloc_lock);
    struct list_elem *e ;
    struct page *p = NULL;
    for (e = list_begin(&lru_pages); e != list_end(&lru_pages); e = list_next(e))
    {
        p = list_entry(e, struct page, elem);
        if(p->kaddr == kaddr)
            break;
    }
    ASSERT(p)
    palloc_free_page(kaddr);
    list_remove(&p->elem);
    free(p);
    // lock_release(&alloc_lock);
}