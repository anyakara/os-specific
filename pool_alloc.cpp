/**
 * @file pool_alloc.cpp
 * @author ananya akarra1@uci.edu
 * @brief extending a generic memory allocator to pooling techniques
 * @version 0.1
 * @date 2025-01-19
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 /* even gaming consoles involve memory pools for objects.
 underlying premise is that by using a pool allocator to allocate memory,
 we use blocks of predefined size with faster block determination.
 
 main concepts:
 Blocks(pools) and chunks within each block
 with each chunk having a predefined sizxe, and encodes the Object header,
 storing the meta-information needed for allocator/collector */


/**
 * @brief when chunk is free, next contains address of next chunk.
 * when allocated, space is used by the user.
 */

#include <cstddef>
#include <cstdlib>
struct chunk {
    chunk * next;
};

// NOTE: allocation pointer can directly handle next free chunk,
//         we don't need track of the space taken by next pointer


/**
 * @brief allocator class
 * features:
    * parameterized by # of chunks per block
    * keeps track of allocation pointer
    * bump-allocates chunks
    * requests a new larger block when needed
 */


class PoolAllocator {
    public:
    PoolAllocator(size_t chunksPerBlock) : mChunksPerBlock(chunksPerBlock) {}
    void *allocate(size_t size);
    void deallocate(void *ptr, size_t size);
    
    private:
    size_t mChunksPerBlock;

    /* allocation pointer */
    chunk *mAlloc = nullptr;

    /* allocates a larger block (pool) for chunks */
    chunk *allocateBlock(size_t size);
};

/**
 * @brief allocates a new block from Operating System.
 * 
 * Returns a chunk pointer set to the beginning of the block
 */

chunk *PoolAllocator::allocateBlock(size_t chunkSize) {
    size_t blockSize = mChunksPerBlock * chunkSize;

    // first chunk allocation
    chunk *blockBegin = reinterpret_cast<chunk *>(malloc(blockSize));

    // once block has beena allocated, chain all chunks in this block
    chunk *_chunk = blockBegin;

    for (int i = 0; i < mChunksPerBlock - 1; ++i) {
        _chunk->next = reinterpret_cast<chunk *>(reinterpret_cast<chunk *>(_chunk) + chunkSize);
        _chunk = _chunk->next;
    }
    _chunk->next = nullptr;
    return blockBegin;
}

/* free the current position of the allocation pointer mAlloc, advance, bump the 
allocation pointer further for future allocation requests. 

with chunk allocation, we return a free chunk at the current position of allocation
pointer mAlloc. Advance (nump) the allocation pointer further for future allocation. */

void *PoolAllocator::allocate(size_t size) {
    if (mAlloc == nullptr) {
        chunk* malloc = allocateBlock(size);
    }
    chunk *freeChunk = mAlloc;
    mAlloc = mAlloc->next;
    return freeChunk;
}; // (bump-allocate) chunks within a block and malloc-allocate blocks from os


/**
 * @brief deallocation of chunks is simpler.
 * Return it at front of the chunks list, setting the mAlloc pointing to it.
 // Place the chunk into the front of the chunks list.
 */

void PoolAllocator::deallocate(void *chunk_, size_t size) {

    // freed chunk's next pointer points to the current alloc pointer
    reinterpret_cast<chunk *>(chunk_)->next = mAlloc;

    // allocation pointer is now set to the returned (free) chunk;
    chunk *malloc = reinterpret_cast<chunk*>(chunk_);
};


// Allocation and deallocation has been taken care of. Step 2 would be to create a class
// with custom pool allocator.
// Objects with custom allocator: override `new` and `delete` operators.
// set up the pool allocator to handle allocation requests

struct Object {
    uint64_t data[2];

    static PoolAllocator allocator;
    
    static void *operator new(size_t size) {
        return allocator.allocate(size);
    }

    static void operator delete(void *ptr, size_t size) {
        return allocator.deallocate(ptr, size);
    }
};

// use 8 chunks per block
// PoolAllocator Object::allocator = 8;


int main(int argc, char const *argv[]) {
    int arraySize = 10;
    Object *object[arraySize];

    for (int i = 0; i < arraySize; ++i) {
        object[i] = new Object();
    }

    for (int i = arraySize; i >= 0; --i) {
        delete object[i];
    } // thse are custom operatrors creating and deleting
}

// memory pool allows for fixed_size blocks of allocation
// usage of pools of memory managment that allows dynamic memory allocation
// it can and has been acehived through the use of techniques such as malloc
// and C++'s operators new;
// the implementations however can suffer from fragmentation due to variable
// block sizes. it is not recommended to use in real time systems due to
// potential issues with performance. more efficient solution is preallocating
// a number of memory blocks with the same size called the memory pool.
// application can allocate, access, and free blocks represented by handles
// at run time.

