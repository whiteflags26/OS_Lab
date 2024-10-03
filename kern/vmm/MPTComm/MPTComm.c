#include <lib/x86.h>

#include "import.h"

#define VM_USERLO 0X40000000
#define VM_USERHI 0xF0000000
#define VM_USERLO_PI (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI (VM_USERHI / PAGESIZE)

/**
 * For each process from id 0 to NUM_IDS - 1,
 * set up the page directory entries so that the kernel portion of the map is
 * the identity map, and the rest of the page directories are unmapped.
 */
void pdir_init(unsigned int mbi_addr)
{
    //whiteflags26
    // TODO: Define your local variables here.
    unsigned int process_index, dir_index;;

    idptbl_init(mbi_addr);
    // Loop through all the processes
    // Loop through all the page directories
    // If the page directory is in the kernel portion of the map 
    // set up the page directory entries so that the kernel portion of the map is the identity map
    // else set the page directory entries so that the rest of the page directories are unmapped

    // (VM_USERLO_PI >> 10) or (VM_USERHI_PI >> 10) 
    // 12 bit is already gone when we divide by 4096
    // 10 bit ia gone by left shifting by 10
    // so the last 10 bits are the page directory index
    
    for(process_index = 0; process_index < NUM_IDS; process_index++){
        for(dir_index = 0; dir_index < 1024; dir_index++){
            if(dir_index < (VM_USERLO_PI >> 10) || dir_index >= (VM_USERHI_PI >> 10)){
                set_pdir_entry_identity(process_index, dir_index);
            }
            else rmv_pdir_entry(process_index, dir_index);
        }
    }
}

/**
 * Allocates a page (with container_alloc) for the page table,
 * and registers it in the page directory for the given virtual address,
 * and clears (set to 0) all page table entries for this newly mapped page table.
 * It returns the page index of the newly allocated physical page.
 * In the case when there's no physical page available, it returns 0.
 */
unsigned int alloc_ptbl(unsigned int proc_index, unsigned int vaddr)
{
    // whiteflags26
    unsigned int address;
    unsigned int* address_pointer;
    unsigned int page_index = container_alloc(proc_index);
    
    if(page_index == 0) return 0;
    //set page directory entry
    set_pdir_entry_by_va(proc_index, vaddr, page_index);

    //clear all page table entries for this newly mapped page table
    for(address = page_index * 1024; address < (page_index + 1) * 1024; address += 4){
        address_pointer = (unsigned int*)address;
        *address_pointer &= 0x00000000;
    }

    return page_index;
}

// Reverse operation of alloc_ptbl.
// Removes corresponding the page directory entry,
// and frees the page for the page table entries (with container_free).
void free_ptbl(unsigned int proc_index, unsigned int vaddr)
{
    // whiteflags26
    unsigned int pdir_index = get_pdir_entry_by_va(proc_index, vaddr);
    unsigned int page_index = pdir_index >> 12; //removing the last 12 bits which are the permission bits

    rmv_pdir_entry_by_va(proc_index, vaddr);
    container_free(proc_index, page_index);
}
