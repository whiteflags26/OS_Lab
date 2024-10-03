#include <lib/debug.h>
#include "import.h"

#define PAGESIZE     4096
#define VM_USERLO    0x40000000
#define VM_USERHI    0xF0000000
#define VM_USERLO_PI (VM_USERLO / PAGESIZE)
#define VM_USERHI_PI (VM_USERHI / PAGESIZE)

/**
 * The initialization function for the allocation table AT.
 * It contains two major parts:
 * 1. Calculate the actual physical memory of the machine, and sets the number
 *    of physical pages (NUM_PAGES).
 * 2. Initializes the physical allocation table (AT) implemented in the MATIntro layer
 *    based on the information available in the physical memory map table.
 *    Review import.h in the current directory for the list of available
 *    getter and setter functions.
 */
void pmem_init(unsigned int mbi_addr)
{
    unsigned int nps;

    // Define your local variables here.

    unsigned int tble_row; //whiteflags26
    unsigned int last_row_strt_addrs;
    unsigned int last_row_len;
    unsigned int last_row_end_addrs;

    unsigned int i;
    unsigned int strt_addrs;
    unsigned int len;
    unsigned int perm;
    unsigned int page_indx;

    // Calls the lower layer initialization primitive.
    // The parameter mbi_addr should not be used in the further code.
    devinit(mbi_addr);

    /**
     * Calculate the total number of physical pages provided by the hardware and
     * store it into the local variable nps.
     * Hint: Think of it as the highest address in the ranges of the memory map table,
     *       divided by the page size.
     */
    
    tble_row = get_size(); //whiteflags26

    if(tble_row == 0) {
        nps = 0;
    }
    
    else{
        last_row_strt_addrs = get_mms(tble_row - 1); 
        last_row_len = get_mml(tble_row - 1);
        last_row_end_addrs = last_row_strt_addrs + last_row_len - 1;
        nps = (last_row_end_addrs + 1) / PAGESIZE;
    }

    set_nps(nps);  // Setting the value computed above to NUM_PAGES.

    /**
     * Initialization of the physical allocation table (AT).
     *
     * In CertiKOS, all addresses < VM_USERLO or >= VM_USERHI are reserved by the kernel.
     * That corresponds to the physical pages from 0 to VM_USERLO_PI - 1,
     * and from VM_USERHI_PI to NUM_PAGES - 1.
     * The rest of the pages that correspond to addresses [VM_USERLO, VM_USERHI)
     * can be used freely ONLY IF the entire page falls into one of the ranges in
     * the memory map table with the permission marked as usable.
     *
     * Hint:
     * 1. You have to initialize AT for all the page indices from 0 to NPS - 1.
     * 2. For the pages that are reserved by the kernel, simply set its permission to 1.
     *    Recall that the setter at_set_perm also marks the page as unallocated.
     *    Thus, you don't have to call another function to set the allocation flag.
     * 3. For the rest of the pages, explore the memory map table to set its permission
     *    accordingly. The permission should be set to 2 only if there is a range
     *    containing the entire page that is marked as available in the memory map table.
     *    Otherwise, it should be set to 0. Note that the ranges in the memory map are
     *    not aligned by pages, so it may be possible that for some pages, only some of
     *    the addresses are in a usable range. Currently, we do not utilize partial pages,
     *    so in that case, you should consider those pages as unavailable.
     */

    //whiteflags26
    for(i = 0; i < VM_USERLO_PI; i++) {
        at_set_perm(i, 1);
    }
    for(i = VM_USERHI_PI; i < nps; i++) {
        at_set_perm(i, 1);
    }

    for(i = VM_USERLO_PI; i < VM_USERHI_PI; i++) {
        at_set_perm(i, 0);
    }

    for(i = 0; i < tble_row; i++) {
        strt_addrs = get_mms(i);
        len = get_mml(i);
        
        if(is_usable(i) == 1) perm = 2;
        else perm = 0;

        page_indx = strt_addrs / PAGESIZE;
        if(page_indx * PAGESIZE < strt_addrs) page_indx++;
        
        while((page_indx + 1) * PAGESIZE <= strt_addrs + len) {
            if(page_indx >= VM_USERLO_PI && page_indx < VM_USERHI_PI) {
                at_set_perm(page_indx, perm);
            }
            
            page_indx++;
            if(page_indx >= VM_USERHI_PI) break;
        }

    }
}
