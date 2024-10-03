#include <lib/x86.h>

#include "import.h"

//#define VA_PDIR_MASK 0xFFC00000
#define VA_PTBL_MASK 0x3FF000
#define PAGE_SIZE 4096
#define VM_USERLO  0x40000000
#define VM_USERHI  0xF0000000
#define PT_PERM_PWG (PTE_P | PTE_W | PTE_G)
#define PT_PERM_PW (PTE_P | PTE_W)

/**
 * Returns the page table entry corresponding to the virtual address,
 * according to the page structure of process # [proc_index].
 * Returns 0 if the mapping does not exist.
 */
unsigned int get_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // whiteflags26

    // get the page directory entry
    unsigned int pde_index = vaddr >> 22; // 10 10 12 structure
    unsigned int pde = get_pdir_entry(proc_index, pde_index);
    // check if the page directory entry is present
    if((pde & PTE_P) == 0) return 0;
    
    // get the page table entry
    unsigned int pte_index = ((vaddr & VA_PTBL_MASK ) >> 12) ; // 10 10 12 structure
    unsigned int pte = get_ptbl_entry(proc_index, pde_index, pte_index);
    // check if the page table entry is present
    if((pte & PTE_P) == 0) return 0;

    return pte;
}

// Returns the page directory entry corresponding to the given virtual address.
unsigned int get_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // whiteflags26
    // same thing done in get_ptbl_entry_by_va
    unsigned int pde_index = vaddr >> 22;
    unsigned int pde = get_pdir_entry(proc_index, pde_index);
    
    if((pde & PTE_P) == 0) return 0;

    return pde;
}

// Removes the page table entry for the given virtual address.
void rmv_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    // whiteflags26
     // same thing done in get_ptbl_entry_by_va
    unsigned int pde_index = vaddr >> 22; 
    unsigned int pde = get_pdir_entry(proc_index, pde_index);
    
    if((pde & PTE_P) == 0) return;
    
    unsigned int pte_index = ((vaddr & VA_PTBL_MASK ) >> 12) ; 
    unsigned int pte = get_ptbl_entry(proc_index, pde_index, pte_index);
    
    if((pte & PTE_P) == 0) return;

    rmv_ptbl_entry(proc_index, pde_index, pte_index);

}

// Removes the page directory entry for the given virtual address.
void rmv_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr)
{
    //whiteflags26
    // same thing done in rmv_ptbl_entry_by_va
    unsigned int pde_index = vaddr >> 22; 
    unsigned int pde = get_pdir_entry(proc_index, pde_index);
    
    if((pde & PTE_P == 0)) return;

    rmv_pdir_entry(proc_index, pde_index);

}

// Maps the virtual address [vaddr] to the physical page # [page_index] with permission [perm].
// You do not need to worry about the page directory entry. just map the page table entry.
void set_ptbl_entry_by_va(unsigned int proc_index, unsigned int vaddr,
                          unsigned int page_index, unsigned int perm)
{
    //whiteflags26
    // same thing done in get_ptbl_entry_by_va
    unsigned int pde_index = vaddr >> 22;
    
    unsigned int pte_index = ((vaddr & VA_PTBL_MASK ) >> 12) ;
   
    set_ptbl_entry(proc_index, pde_index, pte_index, page_index, perm);
}

// Registers the mapping from [vaddr] to physical page # [page_index] in the page directory.
void set_pdir_entry_by_va(unsigned int proc_index, unsigned int vaddr,
                          unsigned int page_index)
{
    // whiteflags26
    // same thing done in rmv_ptbl_entry_by_va
    unsigned int pde_index = vaddr >> 22;
    set_pdir_entry(proc_index, pde_index, page_index);
}

// Initializes the identity page table.
// The permission for the kernel memory should be PTE_P, PTE_W, and PTE_G,
// While the permission for the rest should be PTE_P and PTE_W.
void idptbl_init(unsigned int mbi_addr)
{   
    // whiteflags26
    // TODO: Define your local variables here.
    unsigned int address;

    container_init(mbi_addr);
    for (address = 0; address < 0xfffff000; address += PAGE_SIZE) {
        unsigned int pde_index = address >> 22;
        unsigned int pte_index = (address & VA_PTBL_MASK) >> 12;
        // check if the address is in the kernel memory
        // if yes set permission to PTE_P, PTE_W, and PTE_G
        // else set permission to PTE_P and PTE_W

        unsigned int perm = (address < VM_USERLO || address >= VM_USERHI) ? PT_PERM_PWG : PT_PERM_PW;
        
        set_ptbl_entry_identity(pde_index, pte_index, perm);
    }
}