#include <stdio.h>
#include <stdlib.h>
#include <plinux.h> // Certifique-se de que o cabeçalho correto está incluído

int main(int argc, char *argv[])
{
    // Verifica os argumentos
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <PID> <symbol_name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Converte argumentos de entrada
    pid_t pid = atoi(argv[1]);
    const char *symbol_name = argv[2];

    if (pid <= 0)
    {
        fprintf(stderr, "Invalid PID: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    // Tenta anexar ao processo
    if (plinux_attach(pid) == -1)
    {
        perror("Failed to attach to process");
        return EXIT_FAILURE;
    }
    printf("Successfully attached to process %d\n", pid);

    // Resolve o endereço do símbolo
    void *addr = plinux_resolve(pid, symbol_name);
    if (!addr)
    {
        fprintf(stderr, "Failed to resolve symbol: %s\n", symbol_name);
        plinux_detach(pid); // Garante que o processo será desconectado
        return EXIT_FAILURE;
    }
    printf("Resolved address for symbol '%s': %p\n", symbol_name, addr);

    // Faz a chamada remota
    const char* rax;
    if ((rax = (char*)plinux_call(pid, addr, 100)) == -1)
    {
        fprintf(stderr, "Failed to call function at %p\n", addr);
        plinux_detach(pid);
        return EXIT_FAILURE;
    }

    printf("%p\n", rax);

    printf("Function call to %p completed successfully.\n", addr);

    // Desanexa do processo
    if (plinux_detach(pid) == -1)
    {
        perror("Failed to detach from process");
        return EXIT_FAILURE;
    }
    printf("Successfully detached from process %d\n", pid);

    return EXIT_SUCCESS;
}
