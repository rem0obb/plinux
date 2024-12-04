#include <stdio.h>
#include <plinux.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <symbol_name>\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (plinux_attach(atoi(argv[1])) != -1)
    {
        const char *nid = argv[2];
        void *addr = plinux_resolve(atoi(argv[1]), nid);
        if (addr)
        {
            printf("Resolved address: %p\n", addr);
        }

        return addr ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    return 0;
}
