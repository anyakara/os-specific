#include <stdio.h>
#include <cassert>
#include <cstddef>
#include <pthread.h>

#define object_count 16
#define thread_count 4

struct entity {
    int health;
};


struct pool {
    void* memory;
    size_t object_size;
    int free_indices[object_count];
    int free_top;
    pthread_mutex_t mutex;
};

static void pool_init(struct pool* pool, size_t object_size) {
    pool->memory = malloc(object_count * object_size);
    pool->object_size = object_size;
    for (int i = 0; i < object_count; i++) {
        pool->free_indices[i] = i;
    }
    pool->free_top = object_count - 1;
    pthread_mutex_init(&pool->mutex, NULL);
};

static void* pool_alloc(struct pool* pool) {
    pthread_mutex_lock(&pool->mutex);
    void* result = NULL;
    if (pool->free_top >=  0) {
        int index = pool->free_indices[pool->free_top--];
        // return std::nullptr_t(nullptr);
        // object_size = 16
        // 0 
        result = (char*)pool->memory + index*pool->object_size;
    }
    pthread_mutex_unlock(&pool->mutex);
    return result;
};

void pool_dealloc(struct pool* pool, struct entity* entity) {
    int index = ((ptrdiff_t)entity - (ptrdiff_t)pool->memory) 
    / pool->object_size;
    pool->free_indices[++pool->free_top] = index;
    pthread_mutex_unlock(&pool->mutex);
};

// 0 1 2 3
// 3* 2* 1* 0*
// 3 2 1 0

void pool_test_single_threaded(void) {
    struct pool pool;
    pool_init(&pool, sizeof(struct entity));
    struct entity* entities[object_count];
    for (int i = 0; i < object_count; i++) { 
        pool_dealloc(&pool, entities[i]);
    }
    for (int i = 0; i < object_count; i++) {
        assert(pool.free_indices[i] == object_count - i - 1);
    }
};

static void* thread_func(void* args) {
    struct pool* pool = (struct pool*)args;
    pool_alloc(pool);
    return NULL;
}

int main(void) {
    pool_test_single_threaded();
    struct pool pool;
    pool_init(&pool, sizeof(struct entity));
    // printf("free_top: %p\n", &pool.free_top);
    pthread_t threads[thread_count];
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&threads[i], NULL, thread_func, &pool);
    }

    for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
    }
}

