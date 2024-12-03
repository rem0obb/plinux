#include <stdio.h>
#include <plinux.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc >= 2)
    {
        if (pt_attach(atoi(argv[1])) != -1)
        {
            puts("[+] pt_attach pid succesful");
        }

        if (pt_detach(atoi(argv[1])) != -1)
        {
            puts("[+] pt_detach pid succesful");
        }
    }
    
    return 0;
}