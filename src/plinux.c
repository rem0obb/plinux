/* 
 * Copyright (C) 2024 remoob
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3, or (at your option) any
 * later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING. If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <plinux.h>
#include <sys/ptrace.h>
#include <dlfcn.h>
#include <unistd.h>

/**
 * Attach to a process.
 * 
 * @param pid Process ID to attach to.
 * @return 0 on success, -1 on failure.
 */
long int plinux_attach(pid_t pid)
{
    if (ptrace(PT_ATTACH, pid, NULL, NULL) == -1)
    {
        perror("ptrace(plinux_ATTACH) failed");
        return -1;
    }
    return 0;
}

/**
 * Detach from a process.
 * 
 * @param pid Process ID to detach from.
 * @return 0 on success, -1 on failure.
 */
long int plinux_detach(pid_t pid)
{
    if (ptrace(PT_DETACH, pid, NULL, NULL) == -1)
    {
        perror("ptrace(plinux_DETACH) failed");
        return -1;
    }
    return 0;
}

/**
 * Continue a process after attaching.
 * 
 * @param pid Process ID to continue.
 * @param sig Signal to deliver (0 for no signal).
 * @return 0 on success, -1 on failure.
 */
long int plinux_continue(pid_t pid, int sig)
{
    if (ptrace(PT_CONTINUE, pid, (caddr_t)1, sig) == -1)
    {
        perror("ptrace(plinux_CONTINUE) failed");
        return -1;
    }
    return 0;
}

/**
 * Resolve the address of a symbol in the PLT.
 * 
 * @param plthook Pointer to an initialized plthook instance.
 * @param nid Symbol name to search for.
 * @return Address of the symbol, or NULL if not found.
 */
void *plinux_resolve(plthook_t *plthook, const char *nid)
{
    void **addr = NULL;
    const char *name;
    unsigned int pos = 0;

    while (plthook_enum(plthook, &pos, &name, &addr) == 0)
    {
        printf("%s\n", name);
        if (strcmp(name, nid) == 0)
        {
            printf("Symbol '%s' resolved to address: %p\n", name, addr);
            return addr;
        }
    }

    fprintf(stderr, "Symbol '%s' not found.\n", nid);
    return NULL;
}

/**
 * Open the PLT of the current process.
 * 
 * @return plthook_t* Pointer to the PLT hook instance, or NULL on failure.
 */
plthook_t *plinux_dlopen_self()
{
    plthook_t *plthook = NULL;

    if (plthook_open(&plthook, NULL) != 0)
    {
        fprintf(stderr, "plthook_open_by_handle failed: %s\n", plthook_error());
        return NULL;
    }

    return plthook;
}

/**
 * Open the PLT of the current process.
 * 
 * @return plthook_t* Pointer to the PLT hook instance, or NULL on failure.
 */
plthook_t *plinux_dlopen(const char* lib)
{
    plthook_t *plthook = NULL;

    // Open a handle to the current process using dlopen with NULL
    void *handle = dlopen(lib, RTLD_LAZY);
    if (!handle)
    {
        fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return NULL;
    }

    // Open the PLT using the handle
    if (plthook_open_by_handle(&plthook, handle) != 0)
    {
        fprintf(stderr, "plthook_open_by_handle failed: %s\n", plthook_error());
        dlclose(handle);
        return NULL;
    }

    // dlclose is safe to call here because plthook retains the handle
    dlclose(handle);

    return plthook;
}