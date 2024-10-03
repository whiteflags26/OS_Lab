#include <lib/gcc.h>
#include <lib/x86.h>
#include <lib/debug.h>

#include "import.h"

#define PT_PERM_UP  0
#define PT_PERM_PTU (PTE_P | PTE_W | PTE_U)

/**
 * Page directory pool for NUM_IDS processes.
 * mCertiKOS maintains one page structure for each process.
 * Each PDirPool[index] represents the page directory of the page structure
 * for the process # [index].
 * In mCertiKOS, we statically allocate page directories, and maintain the second
 * level page tables dynamically.
 * The unsigned int * type is meant to suggest that the contents of the array
 * are pointers to page tables. In reality they are actually page directory
 * entries, which are essentially pointers plus permission bits. The functions
 * in this layer will require casting between integers and pointers anyway and
 * in fact any 32-bit type is fine, so feel free to change it if it makes more
 * sense to you with a different type.
 */
unsigned int *PDirPool[NUM_IDS][1024] gcc_aligned(PAGESIZE);

/**
 * In mCertiKOS, we use identity page table mappings for the kernel memory.
 * IDPTbl is an array of statically allocated identity page tables that will be
 * reused for all the kernel memory.
 * That is, in every page directory, the entries that fall into the range of
 * addresses reserved for the kernel will point to an entry in IDPTbl.
 */
unsigned int IDPTbl[1024][1024] gcc_aligned(PAGESIZE);

// Sets the CR3 register with the start address of the page structure for process # [index].
void set_pdir_base(unsigned int index)
{
    //whiteflags26
    set_cr3(PDirPool[index]); // PdirPool[index] is the start address of the page structure for process # [index]
}

// Returns the page directory entry # [pde_index] of the process # [proc_index].
// This can be used to test whether the page directory entry is mapped.
unsigned int get_pdir_entry(unsigned int proc_index, unsigned int pde_index)
{
    // whiteflags26
    return (unsigned int) PDirPool[proc_index][pde_index]; // PDirPool[proc_index][pde_index] is the page directory 
                                                           // entry # [pde_index] of the process # [proc_index]
}

// Sets the specified page directory entry with the start address of physical
// page # [page_index].
// You should also set the permissions PTE_P, PTE_W, and PTE_U.
void set_pdir_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int page_index)
{
    // whiteflags26
    PDirPool[proc_index][pde_index] = (unsigned int *) ((page_index << 12) | PT_PERM_PTU);
    // pageindex is page frame number we shift it by 12 and 'bitwise or' permission bits in the zero bits
    // this how information is stored efficiently
}

// Sets the page directory entry # [pde_index] for the process # [proc_index]
// with the initial address of page directory # [pde_index] in IDPTbl.
// You should also set the permissions PTE_P, PTE_W, and PTE_U.
// This will be used to map a page directory entry to an identity page table.
void set_pdir_entry_identity(unsigned int proc_index, unsigned int pde_index)
{
    // whiteflags26
    unsigned int pdir = IDPTbl[pde_index];
    PDirPool[proc_index][pde_index] = (unsigned int *) (pdir | PT_PERM_PTU);
    // pdir already had its last 12 bits as 0 for gcc_aligned(PAGESIZE) so we just need to 'or' permission bits
}

// Removes the specified page directory entry (sets the page directory entry to 0).
// Don't forget to cast the value to (unsigned int *).
void rmv_pdir_entry(unsigned int proc_index, unsigned int pde_index)
{
    // whiteflags26
    PDirPool[proc_index][pde_index] = (unsigned int *) 0x00000000; 
}

// Returns the specified page table entry.
// Do not forget that the permission info is also stored in the page directory entries.
unsigned int get_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                            unsigned int pte_index)
{
    // whiteflags26
    unsigned int pde = PDirPool[proc_index][pde_index];
    pde &= 0xFFFFF000; //masking the last 12 bits
    unsigned int *ptbl_entry_address = (unsigned int *)(pde | (pte_index << 2)); //4 bytes per entry
    
    return *ptbl_entry_address; // accessing the value of the entry
}

// Sets the specified page table entry with the start address of physical page # [page_index]
// You should also set the given permission.
void set_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index, unsigned int page_index,
                    unsigned int perm)
{
    // whiteflags26
    // do the same thing as get_ptbl_entry but instead of returning the value, set the value
    unsigned int pdir = (unsigned int)PDirPool[proc_index][pde_index];
    pdir &= 0xFFFFF000;
    unsigned int *ptbl_entry_address = (unsigned int *)(pdir | (pte_index << 2));
    *ptbl_entry_address = (page_index << 12) | (perm & 0xFFF); //masking the last 12 bits
}

// Sets up the specified page table entry in IDPTbl as the identity map.
// You should also set the given permission.
void set_ptbl_entry_identity(unsigned int pde_index, unsigned int pte_index,
                             unsigned int perm)
{
    // whiteflags26
    // for IDptbl the physical address is the same as the virtual address
    // so we just need to set the permission bits
    // we already have the page dirctory entry and page table entry
    // this works like 10 10 12 bits for pde, pte, and permission bits
    IDPTbl[pde_index][pte_index] = (((pde_index << 10) | pte_index) << 12) | (perm & 0xFFF);
}

// Sets the specified page table entry to 0.
void rmv_ptbl_entry(unsigned int proc_index, unsigned int pde_index,
                    unsigned int pte_index)
{
    //whiteflags26
    // same as set_ptbl_entry but set the value to 0
    unsigned int pdir = (unsigned int)PDirPool[proc_index][pde_index];
    pdir &= 0xFFFFF000;
    unsigned int *ptbl_entry_address = (unsigned int *)(pdir | (pte_index << 2));
    *ptbl_entry_address = 0x00000000;
}
