/**
 * @file mem_alloc.cpp
 * @author ananya karra (ananya.karra@gmail.com)
 * @brief generic memory allocator implementation enforcing
            concepts in programming languages similar to malloc
             function and principles from OS course
 * @version 0.1
 * @date 2025-01-18
 * 
 * @copyright Copyright (c) 2025
 * 
 */


/* garbage collector algorithms rely on three main modules known as
mutators, allocators, and collectors.

mutator is a user program - objects are created for their own purposes.
other modules should respect the mutators view on the object graph
no circumstances should a collector reclaim an alive object.

it should not allocate objects by itself. instead it delegates this 
generic task to the allocator module - 
this is the topic of our discussion. */

/**
 * @brief machine word size. depending on the architecture can be 4 or 8 bytes.
 */

#include <cstddef>
#include <cstdint>
#include <utility>
#include <assert.h>
using word_t = intptr_t;

/**
 * @brief allocated block of memory. contains the object header structure
 and the actual payload pointer.
 */

struct Block {
    // 1. object header
    /* block size */
    size_t size;
    /* whether this block is currently used. */
    bool used;

    /* next block in the list. */
    Block *next;

    /**
     * @brief Payload pointer
     */
    word_t data[1];
};


/* heap start. initialized on first allocation. */
static Block *heapStart = nullptr;

/* current top. updated on each allocation. */
static auto top = heapStart;


// memory alignment - for faster access, memory block should be aligned,
// that too by the size of the machine word


/**
 * @brief Aligns the size by the machine word
 * 
 */

inline size_t align(size_t n) {
    return (n + sizeof(word_t) - 1) & ~(sizeof(word_t) - 1);
};
// buffer of two bytes provided for alloaction - on a 32 bit
// architecture as an example

/**
 * @brief Bring up the topic of virtual memory and memory mapping, 
 and see what allocation actually means from the operating system perspective.
 * note that the heap grows upwards towards hgiher addresses, and the area
 between the stack and the heap is the unmapped area.
 * mapping is controlled by the position of the program break (brk) pointer.
 * provided the current top of the heap, the sbrk function increases (bumps)
 the value of the program break on the passed amount of bytes
 */


// procedure for requesting memory from OS
#include <unistd.h>
/**
 * @brief Returns total allocation size, reserving in addition the space
 for Block structure (object header + first data word).
 * word_t data[1] allocates one word inside the block structure, decrease it
 from the size request. if a user allocates only one word, it is in the block struct.
 */


inline size_t allocSize(size_t size_) {
    return size_ + sizeof(Block) - sizeof(std::declval<Block>().data);
}

/**
 * @brief Requests (maps) memory from OS.
 */

Block *requestFromOS(size_t size_) {
    // current heap break - position of newly alloacted block
    Block * block = (Block *)sbrk(0);

    // OOM - pass amt of bytes signal about OOM, out of memory, returning nullptr
    // otherwise return obtained (1) address of allocated block
    if (sbrk(allocSize(size_)) == (void *) - 1) return nullptr;
    return block;
}


/**
 * @brief returns the object header
 */

Block *getHeader(word_t *data) {
    return (Block *)((char*)data + sizeof(std::declval<Block>().data) -
                                    sizeof(Block));
};


/**
 * @brief First fit algorithm
 * Returns the first free block which fits the size
 traversal of all blocks starting at the beginning, of the heap (heapStart)
  intialized on first allocation and returns first found block if it fits 
  the size or the nullptr otherwise.
 * @param size 
 * @return Block* 
 */


Block *firstFit(size_t size) {
    Block * block = heapStart;
    
    while (block != nullptr) {
        if (block->used || block->size < size) {
            block = block->next;
            continue;
        }
        return block;
    }
    return nullptr;
};


/* Mode for seasrching a free block */
enum class SearchMode {
    FirstFit, NextFit, BestFit, FreeList, SegregatedList,
};

/* Previously found block. Updated in nextFit */
static Block *searchStart = heapStart;

// current search mode.
static auto searchMode = SearchMode::FirstFit;

/* Reset the heap to the original position. */
void resetHeap() {
    if (heapStart == nullptr) return; // heap is empty;
    // roll back to the beginning
    brk(heapStart);
    heapStart = nullptr;
    top = nullptr;
    searchStart = nullptr;
}

void init(SearchMode mode) {
    searchMode = mode;
    resetHeap();
}

// Next-fit algorithm
Block * nextFit(size_t size) {
    // implement here
};



// Implementation of Best-fit Search
Block *bestFit(size_t size) {
    // implementation
};


// Blocks splitting - if we find a block of suitable size, we use it.
// But if we find a block that is much more larger than the requested one.
// Implement instead a procedure to split a larger free block to smaller
// blocks -- taking from the smaller chunks when requested. 
// other portions stay free and unexamined, and all blocks can be used
// in future allocation requests.

// we build a list allocate function to make the larger block to smaller
// block splitting happen.


// ----------------------------------------------------------------

Block *split(Block *block, size_t size) {
    // to be implemented
};

inline bool canSplit(Block *block, size_t size) {
    // implement here
}

Block *listAllocate(Block *block, size_t size) {
    if (canSplit(block, size)) {
        block = split(block, size);
    }
    block->used = true;
    block->size = size;
    return block;
}


/* Add functionality called blocks coalescing where
on freeing the blocks, we can do the opposite to splitting
operationg and coalesce two or more adjacent blocks to
a larger one. */

// merging procedure
bool canCoalesce(Block* block) {
    return block->next && !block->next->used;
}

Block *coalesce(Block *block) {
    // ... implement here
};


/* Frees the previously allocated block ... */
void free(word_t *data) {
    Block* block = getHeader(data);
    if (canCoalesce(block)) {
        block = coalesce(block);
    }
    block->used = false;
}


// ----------------------------------------------------------------

// Concept of an Explicit Free-List

#include <list>

static std::list<Block *> free_list;

Block *freeList(size_t size) {
    for (const auto& block: free_list) {
        if (block->size < size) continue;
        free_list.remove(block);
        return listAllocate(block, size);
    }
    return nullptr;
}

// ----------------------------------------------------------------


/* Concept of Segregated-list search -- search through blocks of different sizes
 what about doing that search on blocks of similar sizes, grouping blocks
-- where instead of having one list of blocks, we have many lists of blocks
but each list contains only blocks of a certain size. */

Block *segregatedLists[] = {
    nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
};

inline int getBucket(size_t size) {
    return size / sizeof(word_t) - 1;
};

Block *segregatedFit(size_t size) {
    auto bucket = getBucket(size);
    auto originalHeapStart = heapStart;

    heapStart = segregatedLists[bucket];
    Block* block = firstFit(size);

    heapStart = originalHeapStart;
    return block;
};


// ----------------------------------------------------------------

// Optimization of storage
typedef struct {
    size_t header;
    word_t data[1];
} to_opt_block;

/* returns actual size */
inline size_t getSize(to_opt_block *block) {
    return block->header & ~1L;
};

/* check if the block is being used */
inline bool isUsed(to_opt_block *block) {
    return block->header & 1;
}

/* sets the used flag */
inline void setUsed(to_opt_block *block, bool used) {
    if (used) block->header |= 1;
    else block->header &= ~1;
}

// ----------------------------------------------------------------

// Implementation of a custom sbrk - mapping of external files
// memory is requested. mmap can be used for both, mapping 
// used to create anonymous mappings (not backed by any file)

#include <sys/mman.h>

/**
 * @brief allocation arena for custom sbrk example
 */

 // static void *arena = nullptr;
 static char *_brk = nullptr;
 static size_t arena_size = 4193404;

 // map a large chunk of anonymous memory
 // virtual heap for custom sbrk manipulation
 static void *arena = mmap(0, arena_size, PROT_READ | 
                            PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

// implement our own
void *_sbrk(intptr_t increment) {
    // to be implemented

    // 0. pre allocate large arena using `mmap` init program break to 
    // the beginning of this arena.

    // 1. if increment is 0, return current break position

    // 2. if current + increment exceeds top of arena, return -1

    // 3. otherwise, increase the program break on increment bytes
};

// ----------------------------------------------------------------

/**
 * @brief TO BE IMPLEMENTED // calls the different algorithms
 to find the closest block
 * @param size 
 * @return Block* 
 */


Block* findBlock(size_t size) {
    switch(searchMode) {
        case SearchMode::FirstFit:          // general purpose search algorithm
            return firstFit(size);
        case SearchMode::NextFit:           // general purpose search algorithm
            return nextFit(size);
        case SearchMode::BestFit:           // general purpose search algorithm
            return bestFit(size);
        case SearchMode::FreeList:          // optimizing search speed by linking only free blocks
            return freeList(size);
        case SearchMode::SegregatedList:
            return segregatedFit(size);     // optimizing search speed by size of blocks
    }
};


/**
 * @brief logic layout of objects is not important. we follow
 the interface of size of the block. choosing to minimic the malloc
 function -> choose to use the typed word_t* instead of void* 
 for the return type.
 */

// Allocates a block of memory of (at least) 'size' bytes.
word_t *alloc(size_t size) {
    size = align(size);

    // ------------------------------------------------------------
    // 1. Search for available free block.

    if (Block* block = findBlock(size)) {
        return block->data;
    }
    // ------------------------------------------------------------
    // 2. If block is not found in the free list, request from OS.

    Block * block = requestFromOS(size);
    
    block->size = size;
    block->used = true;


    // initialize heap;
    if (heapStart == nullptr) heapStart = block;

    // chain the blocks
    if (top != nullptr) top->next = block;

    top = block;

    // user payload
    return block->data;
};

/**
 * @brief sets the used flag to false
    * receives actual user pointer from which it finds the block
    * with header function and then updates the used flag
 * 
 * @param data 
 */


void free(word_t *data) {
    Block* block = getHeader(data);
    block->used = false;

    if (searchMode == SearchMode::FreeList) {
        free_list.push_back(block);
    } 
};


/**
 * @brief test main file logic
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */


// to test the allocation
int main(int argc, char const *argv[]) {
    word_t* p1 = alloc(3);
    Block* p1b = getHeader(p1);
    assert(p1b->size == sizeof(word_t));

    word_t *p2 = alloc(8);
    Block *p2b = getHeader(p2);
    assert(p2b->size == 8);

    free(p2);
    assert(p2b->used == false);

    
};