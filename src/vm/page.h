#include <hash.h>
#include <stdlib.h>

enum vm_type
{
    VM_BIN,
    VM_FILE,
    VM_ANON,
};

struct vm_entry{
    enum vm_type type;
    void *vaddr;
    bool writable;
    bool is_loaded;
    struct file *file;
    size_t offset;
    size_t read_bytes;
    size_t zero_bytes;
    struct hash_elem elem;
};

void init_vm_table(struct hash *table);
bool insert_vm_entry(struct hash *table, struct vm_entry *entry);
bool delete_vm_entry(struct hash *table, struct vm_entry *entry);
struct vm_entry *find_vm_entry(struct hash *table, void *addr);
void destroy_table(struct hash *table);