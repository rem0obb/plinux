#include <stdio.h>
#include <stdlib.h>
#include <plinux.h>
#include <dlfcn.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <PID>\n", argv[0]);
        return EXIT_FAILURE;
    }

    pid_t pid = atoi(argv[1]);

    if (pid <= 0)
    {
        fprintf(stderr, "[*] Invalid PID: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (plinux_attach(pid) == -1)
    {
        perror("[*] Failed to attach to process");
        return EXIT_FAILURE;
    }

    printf("[*] Successfully attached to process %d\n", pid);

    const char *malloc_symbol = "malloc";
    void *malloc_addr = plinux_resolve(pid, malloc_symbol);

    if (!malloc_addr)
    {
        fprintf(stderr, "[*] Failed to resolve symbol: %s\n", malloc_symbol);
        plinux_detach(pid);
        return EXIT_FAILURE;
    }

    printf("[*] Resolved address for '%s': %p\n", malloc_symbol, malloc_addr);

    char *alloc = (char *)plinux_call(pid, malloc_addr, 1000); // call malloc
    printf("[*] Memory malloc allocated in '%lx'\n", alloc);

    const char *hello = "Hello World";
    plinux_write_memory(pid, alloc, hello, strlen(hello));

    const char *puts_symbol = "puts";
    void *puts_addr = plinux_resolve(pid, puts_symbol);

    if (!puts_addr)
    {
        fprintf(stderr, "[*] Failed to resolve symbol: %s\n", puts_symbol);
        plinux_detach(pid);
        return EXIT_FAILURE;
    }

    printf("[*] Resolved address for '%s': %p\n", puts_symbol, puts_addr);

    plinux_call(pid, puts, alloc, RTLD_NOW | RTLD_LAZY); // call puts

    if (plinux_detach(pid) == -1)
    {
        perror("[*] Failed to detach from process");
        return EXIT_FAILURE;
    }
    printf("[*] Successfully detached from process %d\n", pid);

    return EXIT_SUCCESS;
}
