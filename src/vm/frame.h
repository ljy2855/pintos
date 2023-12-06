#include "swap.h"
#include "threads/palloc.h"
struct page *alloc_page(enum palloc_flags flags);
void free_page(void *kaddr);

