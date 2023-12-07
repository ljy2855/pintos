#include "swap.h"
#include "userprog/pagedir.h"
#include <list.h>
#include "filesys/file.h"
#include <debug.h>
#include "devices/block.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "bitmap.h"
#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)
static struct block *swap_block;
static struct bitmap *swap_bitmap;
static struct lock swap_lock;

struct page *select_victim(void);
size_t find_free_swap_slot(void);
size_t write_swap_page(void *kaddr);

struct list_elem *clock_pointer = NULL;

struct page *select_victim()
{
    struct thread *t = thread_current();
    if (clock_pointer == NULL)
        clock_pointer = list_begin(&lru_pages);
    struct page *cur_page;
    while (true)
    {
        cur_page = list_entry(clock_pointer, struct page, elem);
        if (cur_page->entry->is_loaded && t == cur_page->entry->t)
        {
            if (!pagedir_is_accessed(cur_page->entry->t->pagedir, cur_page->entry->vaddr))
            {
                // 접근되지 않은 페이지 발견
                break;
            }

            // 접근 비트를 초기화하고 다음 페이지로 이동
            pagedir_set_accessed(cur_page->entry->t->pagedir, cur_page->entry->vaddr, false);
        }
        clock_pointer = list_next(clock_pointer);
        if (clock_pointer == list_end(&lru_pages))
            clock_pointer = list_begin(&lru_pages);
    }

    return cur_page;
}

void *swap_out()
{
    struct page *freed_page = select_victim();

    ASSERT(freed_page);
    struct vm_entry *entry = freed_page->entry;

    ASSERT(entry);
    void *kaddr = freed_page->kaddr;
    switch (entry->type)
    {
    case VM_BIN:
        if (pagedir_is_dirty(entry->t->pagedir, entry->vaddr))
        {
            // TODO disk write
            entry->swap_index = write_swap_page(freed_page->kaddr);
            ASSERT(entry->swap_index != BITMAP_ERROR);

            entry->type = VM_ANON;
        }
        entry->is_loaded = false;
        pagedir_clear_page(entry->t->pagedir, entry->vaddr);
        break;
    case VM_FILE:
        if (pagedir_is_dirty(entry->t->pagedir, entry->vaddr))
        {
            lock_acquire(&swap_lock);
            file_write_at(entry->file, kaddr, entry->read_bytes, entry->offset);
            lock_release(&swap_lock);
        }
        entry->is_loaded = false;
        pagedir_clear_page(entry->t->pagedir, entry->vaddr);
        break;

    case VM_ANON:
        entry->swap_index = write_swap_page(freed_page->kaddr);
        entry->is_loaded = false;
        ASSERT(entry->swap_index != BITMAP_ERROR);

        pagedir_clear_page(entry->t->pagedir, entry->vaddr);
        break;
    }

    return kaddr;
}

void swap_init(void)
{
    swap_block = block_get_role(BLOCK_SWAP);
    if (swap_block == NULL)
    {
        PANIC("No swap block device found.");
    }

    swap_bitmap = bitmap_create(block_size(swap_block) / SECTORS_PER_PAGE);
    bitmap_set_all(swap_bitmap, false);

    lock_init(&swap_lock);
}

size_t find_free_swap_slot(void)
{
    lock_acquire(&swap_lock);
    size_t free_slot = bitmap_scan_and_flip(swap_bitmap, 0, 1, false);
    lock_release(&swap_lock);
    return free_slot;
}

size_t write_swap_page(void *kaddr)
{
    size_t free_slot = find_free_swap_slot();
    if (free_slot == BITMAP_ERROR)
    {
        PANIC("No free swap slots available.");
    }

    lock_acquire(&swap_lock);
    for (int i = 0; i < SECTORS_PER_PAGE; i++)
    {
        block_sector_t sector = free_slot * SECTORS_PER_PAGE + i;
        block_write(swap_block, sector, kaddr + i * BLOCK_SECTOR_SIZE);
    }
    lock_release(&swap_lock);
    return free_slot;
}

void read_swap_page(size_t disk_index, void *kaddr)
{
    lock_acquire(&swap_lock);
    for (int i = 0; i < SECTORS_PER_PAGE; i++)
    {
        block_sector_t sector = disk_index * SECTORS_PER_PAGE + i;
        block_read(swap_block, sector, kaddr + i * BLOCK_SECTOR_SIZE);
    }
    bitmap_flip(swap_bitmap, disk_index);
    lock_release(&swap_lock);
}