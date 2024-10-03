#include <lib/debug.h>
#include <lib/x86.h>
#include "import.h"

struct SContainer {
    int quota;      // maximum memory quota of the process
    int usage;      // the current memory usage of the process
    int parent;     // the id of the parent process
    int nchildren;  // the number of child processes
    int used;       // whether current container is used by a process
};

// mCertiKOS supports up to NUM_IDS processes
static struct SContainer CONTAINER[NUM_IDS];

/**
 * Initializes the container data for the root process (the one with index 0).
 * The root process is the one that gets spawned first by the kernel.
 */
void container_init(unsigned int mbi_addr)
{
    unsigned int real_quota;
    unsigned int nps;
    unsigned int i;
    // TODO: define your local variables here.

    pmem_init(mbi_addr);
    real_quota = 0;

    /**
     * TODO: Compute the available quota and store it into the variable real_quota.
     * It should be the number of the unallocated pages with the normal permission
     * in the physical memory allocation table.
     */

    nps = get_nps();
    
    //whiteflags26
    // Loop through all the pages to find the number of unallocated pages with normal permission
    for(i = 0; i < nps; i++){
        if(at_is_norm(i) == 1 && at_is_allocated(i) == 0){ 
            real_quota++;
        }
    }
    KERN_DEBUG("\nreal quota: %d\n\n", real_quota);

    CONTAINER[0].quota = real_quota;
    CONTAINER[0].usage = 0;
    CONTAINER[0].parent = 0;
    CONTAINER[0].nchildren = 0;
    CONTAINER[0].used = 1;
}

// Get the id of parent process of process # [id].
unsigned int container_get_parent(unsigned int id)
{
    // whiteflags26
    return CONTAINER[id].parent;
}

// Get the number of children of process # [id].
unsigned int container_get_nchildren(unsigned int id)
{
    // whiteflags26
    return CONTAINER[id].nchildren;
}

// Get the maximum memory quota of process # [id].
unsigned int container_get_quota(unsigned int id)
{
    // whiteflags26
    return CONTAINER[id].quota;
}

// Get the current memory usage of process # [id].
unsigned int container_get_usage(unsigned int id)
{
    // whiteflags26
    return CONTAINER[id].usage;
}

// Determines whether the process # [id] can consume an extra
// [n] pages of memory. If so, returns 1, otherwise, returns 0.
unsigned int container_can_consume(unsigned int id, unsigned int n)
{
    // whiteflags26
    return (CONTAINER[id].quota - CONTAINER[id].usage) >= n;
}

/**
 * Dedicates [quota] pages of memory for a new child process.
 * You can assume it is safe to allocate [quota] pages
 * (the check is already done outside before calling this function).
 * Returns the container index for the new child process.
 */
unsigned int container_split(unsigned int id, unsigned int quota)
{
    unsigned int child, nc;

    nc = CONTAINER[id].nchildren;
    child = id * MAX_CHILDREN + 1 + nc;  // container index for the child process

    if (NUM_IDS <= child) {
        return NUM_IDS;     //?????????? return -1;
    }

    //whitefflags26
    //updating the parent process Container structure
    CONTAINER[id].nchildren++;
    CONTAINER[id].usage += quota;

    //updating the child process Container structure
    CONTAINER[child].quota = quota;
    CONTAINER[child].usage = 0;
    CONTAINER[child].parent = id;
    CONTAINER[child].nchildren = 0;
    CONTAINER[child].used = 1;

    return child;
}

/**
 * Allocates one more page for process # [id], given that this will not exceed the quota.
 * The container structure should be updated accordingly after the allocation.
 * Returns the page index of the allocated page, or 0 in the case of failure.
 */
unsigned int container_alloc(unsigned int id)
{
    //whiteflags26

    unsigned int page_index_to_allocate = palloc(); //finding if there is any page to allocate
                                                    //if there is, then it will return the page index
                                                    //or else it will return 0
    
    if(page_index_to_allocate) {
        CONTAINER[id].usage++; //updating the usage of the process
    }

    return page_index_to_allocate; //will return page index if page is allocated, else 0
}

// Frees the physical page and reduces the usage by 1.
void container_free(unsigned int id, unsigned int page_index)
{
    //whiteflags26
    pfree(page_index); //freeing the page
    CONTAINER[id].usage--; //updating the usage of the process
}
