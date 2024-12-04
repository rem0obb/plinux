#include <stdio.h>
#include <stdlib.h>
#include <plinux.h>

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
        fprintf(stderr, "Invalid PID: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    if (plinux_attach(pid) == -1)
    {
        perror("Failed to attach to process");
        return EXIT_FAILURE;
    }
    printf("Successfully attached to process %d\n", pid);
    
    const char* symbol_name = "malloc";
    void *addr_symbol = plinux_resolve(pid, symbol_name);

    if (!addr_symbol)
    {
        fprintf(stderr, "Failed to resolve symbol: %s\n", symbol_name);
        plinux_detach(pid); // Garante que o processo será desconectado
        return EXIT_FAILURE;
    }
    
    printf("Resolved address for symbol '%s': %p\n", symbol_name, addr_symbol);

    if (plinux_call(pid, addr_symbol, 100) == -1)
    {
        fprintf(stderr, "Failed to call function at %p\n", addr_symbol);
        plinux_detach(pid);
        return EXIT_FAILURE;
    }

    printf("Function call to %p completed successfully.\n", addr_symbol);

    // Desanexa do processo
    if (plinux_detach(pid) == -1)
    {
        perror("Failed to detach from process");
        return EXIT_FAILURE;
    }
    printf("Successfully detached from process %d\n", pid);

    return EXIT_SUCCESS;
}
