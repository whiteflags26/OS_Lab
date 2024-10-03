#include <lib/x86.h>
#include <lib/debug.h>

#include "import.h"

/**
 * Sets the entire page map for process 0 as the identity map.
 * Note that part of the task is already completed by pdir_init.
 */
void pdir_init_kern(unsigned int mbi_addr)
{
    // TODO: Define your local variables here.
    unsigned int pdir_index;

    pdir_init(mbi_addr);

    // whiteflags26
    // Loop through all the page directories and set the page table entries as the identity map
    // for process 0
    for (pdir_index = 0; pdir_index < 1024; pdir_index++)
    {
        set_pdir_entry_identity(0, pdir_index);
    }
}

/**
 * Maps the physical page # [page_index] for the given virtual address with the given permission.
 * In the case when the page table for the page directory entry is not set up,
 * you need to allocate the page table first.
 * In the case of error, it returns the constant MagicNumber defined in lib/x86.h,
 * otherwise, it returns the physical page index registered in the page directory,
 * (the return value of get_pdir_entry_by_va or alloc_ptbl).
 */
unsigned int map_page(unsigned int proc_index, unsigned int vaddr,
                      unsigned int page_index, unsigned int perm)
{
    // whiteflags26
    unsigned int pde = get_pdir_entry_by_va(proc_index, vaddr);
    unsigned int new_page_index;

    if((pde & PTE_P) == 0) {
        new_page_index = alloc_ptbl(proc_index, vaddr);
        
        if(new_page_index == 0) return MagicNumber;
    }
    set_ptbl_entry_by_va(proc_index, vaddr, page_index, perm);
    pde = get_pdir_entry_by_va(proc_index, vaddr);


    return pde >> 12; // removing the permission bits
}

/**
 * Remove the mapping for the given virtual address (with rmv_ptbl_entry_by_va).
 * You need to first make sure that the mapping is still valid,
 * e.g., by reading the page table entry for the virtual address.
 * Nothing should be done if the mapping no longer exists.
 * You do not need to unmap the page table from the page directory.
 * It should return the corresponding page table entry.
 */
unsigned int unmap_page(unsigned int proc_index, unsigned int vaddr)
{
    // whiteflags26
    unsigned int pte = get_ptbl_entry_by_va(proc_index, vaddr);
    // if pte is 0 then the mapping no longer exists
    if(pte != 0) {
        rmv_ptbl_entry_by_va(proc_index, vaddr);
    }
    
    return get_ptbl_entry_by_va(proc_index, vaddr);
}
