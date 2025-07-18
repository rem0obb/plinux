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

    uint64_t alloc = plinux_call(pid, malloc_addr, 10);
    if (alloc == (uint64_t)-1)
    {
        fprintf(stderr, "Failed to call function at %p\n", malloc_symbol);
        plinux_detach(pid);
        return EXIT_FAILURE;
    }

    printf("[*] Memory malloc allocated in '%lx'\n", alloc);

    if (plinux_detach(pid) == -1)
    {
        perror("[*] Failed to detach from process");
        return EXIT_FAILURE;
    }
    printf("[*] Successfully detached from process %d\n", pid);

    return EXIT_SUCCESS;
}
