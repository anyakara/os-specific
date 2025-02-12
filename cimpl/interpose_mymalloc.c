// code/link/interpose/mymalloc.c
#ifdef RUNTIME
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

/* malloc wrapper function */
void *malloc(size_t size) {
    void *(*mallocp)(size_t size);
    char *error;

    mallocp = dlsym(RTLD_NEXT, "malloc"); /* get address of libc malloc */
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    char *ptr = mallocp(size);
    return ptr;
}

void free(void *ptr) {
    void (*freep)(void *) = NULL;
    char *error;

    if(!ptr) return;

    freep = dlsym(RTLD_NEXT, "free");

}
#endif