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
        plthook_t *plthook = plinux_dlopen_self();
        if (!plthook)
        {
            return EXIT_FAILURE;
        }

        void *addr = plinux_resolve(plthook, nid);
        if (addr)
        {
            printf("Resolved address: %p\n", addr);
        }

        plthook_close(plthook);
        return addr ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    return 0;
}
