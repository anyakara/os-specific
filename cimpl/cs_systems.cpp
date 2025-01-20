// dll.c
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>

// -------------------------------------------------------------------------------

typedef struct {
    long offset;
    long type:32, symbol:32;
    long addend;
} Elf64_Rela;

// ELF relocation entry -- each etnry identifies a refernece that must be relocated
// specifies how to comute the modified reference.

typedef struct {
    int name;
    char type:4, binding:4;
    char reserved;
    short section;
    long value;
    long size;
} Elf64_Symbol; // ELF symbol table entry


int x[2] = {1, 2};
int y[2] = {3, 4};
int z[2];

int main(void) {
    void *handle;
    void (*addvec)(int *, int *, int *, int);
    char *error;

    /* Dynamically load the shared library containing addvec() */
    handle = dlopen("./libvector.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", error);
        exit(1);
    }

    /* Now we can call addvec() just like any other function */
    addvec(x, y, z, 2);
    printf("z = [%d %d]\n", z[0], z[1]);

    /* Unload the shared library */
    if (dlclose(handle) < 0) {
        fprintf(stderr, "%s\n", dlerror());
        exit(1);
    }
    return 0;
}

// main routine of a simple shell program

