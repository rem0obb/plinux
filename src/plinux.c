/* Copyright (C) 2024 remoob

This program is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING. If not, see
<http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <plinux.h>
#include <sys/ptrace.h>
#include <dlfcn.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <stdarg.h>
#include <sys/user.h>
#include <signal.h>

/**
 * Node structure for a dynamically growing list of libraries.
 */
typedef struct LibraryNode
{
    char *path;
    struct LibraryNode *next;
} LibraryNode;

LibraryNode *parse_libraries_from_maps(pid_t pid)
{
    char maps_path[256];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    FILE *maps_file = fopen(maps_path, "r");
    if (!maps_file)
    {
        perror("[*] Failed to open maps file");
        return NULL;
    }

    char line[512];
    LibraryNode *head = NULL;
    LibraryNode *tail = NULL;

    while (fgets(line, sizeof(line), maps_file))
    {
        char *path = strchr(line, '/');
        if (path)
        {
            path[strcspn(path, "\n")] = '\0';

            LibraryNode *current = head;
            int duplicate = 0;
            while (current)
            {
                if (strcmp(current->path, path) == 0)
                {
                    duplicate = 1;
                    break;
                }
                current = current->next;
            }

            if (!duplicate)
            {
                LibraryNode *new_node = malloc(sizeof(LibraryNode));
                if (!new_node)
                {
                    perror("[*] Memory allocation failed");
                    fclose(maps_file);
                    while (head)
                    {
                        LibraryNode *temp = head;
                        head = head->next;
                        free(temp->path);
                        free(temp);
                    }
                    return NULL;
                }
                new_node->path = strdup(path);
                new_node->next = NULL;

                if (!head)
                    head = tail = new_node;
                else
                    tail->next = new_node, tail = new_node;
            }
        }
    }

    fclose(maps_file);
    return head;
}

void *search_symbol_in_library(const char *library, const char *symbol_name)
{
    void *handle = dlopen(library, RTLD_NOLOAD | RTLD_LAZY);
    if (!handle)
        return NULL;

    void *symbol_address = dlsym(handle, symbol_name);
    dlclose(handle);

    return symbol_address;
}

long int plinux_attach(pid_t pid)
{
    if (ptrace(PTRACE_ATTACH, pid, NULL, NULL) == -1)
    {
        perror("[*] ptrace(PTRACE_ATTACH) failed");
        return -1;
    }

    if (waitpid(pid, NULL, 0) < 0)
    {
        perror("[*] waitpid failed");
        return -1;
    }

    return 0;
}

long int plinux_detach(pid_t pid)
{
    if (ptrace(PTRACE_DETACH, pid, NULL, NULL) == -1)
    {
        perror("[*] ptrace(PTRACE_DETACH) failed");
        return -1;
    }
    return 0;
}

long int plinux_getregs(pid_t pid, struct user_regs_struct *r)
{
    return ptrace(PTRACE_GETREGS, pid, 0, r);
}

long int plinux_setregs(pid_t pid, const struct user_regs_struct *r)
{
    return ptrace(PTRACE_SETREGS, pid, NULL, r);
}

long int plinux_continue(pid_t pid, int sig)
{
    if (ptrace(PT_CONTINUE, pid, (off_t)1, sig) == -1)
    {
        perror("[*] ptrace(PT_CONTINUE) failed");
        return -1;
    }

    return 0;
}

long int plinux_step(pid_t pid)
{
    if (ptrace(PTRACE_SINGLESTEP, pid, (off_t)1, 0) == -1)
    {
        perror("[*] ptrace(PTRACE_SINGLESTEP) failed");
        return -1;
    }

    if (waitpid(pid, NULL, 0) < 0)
    {
        perror("[*] waitpid failed");
        return -1;
    }

    return 0;
}

long plinux_call(pid_t pid, void *addr, ...)
{
    struct user_regs_struct jmp_reg;
    struct user_regs_struct bak_reg;
    va_list ap;

    if (plinux_getregs(pid, &bak_reg))
    {
        perror("[*] plinux_getregs failed");
        return -1;
    }

    memcpy(&jmp_reg, &bak_reg, sizeof(jmp_reg));
    jmp_reg.rip = (uint64_t)addr;
    jmp_reg.rax = 0; /* I need to reset rax*/

    /*System V AMD64 ABI
    The first six integer or pointer arguments are passed in registers
    RDI, RSI, RDX, RCX, R8, R9 (R10 is used as a static chain pointer in case of nested functions[28]: 21 )
    */
    va_start(ap, addr);
    jmp_reg.rdi = va_arg(ap, uint64_t);
    jmp_reg.rsi = va_arg(ap, uint64_t);
    jmp_reg.rdx = va_arg(ap, uint64_t);
    jmp_reg.rcx = va_arg(ap, uint64_t);
    jmp_reg.r8 = va_arg(ap, uint64_t);
    jmp_reg.r9 = va_arg(ap, uint64_t);
    va_end(ap);

    if (plinux_setregs(pid, &jmp_reg))
        return -1;

    // Continue until return (rsp grows back)
    while (jmp_reg.rsp <= bak_reg.rsp)
    {
        // printf("[*] RSP = %p RIP = %p\n", jmp_reg.rsp, jmp_reg.rip);
        if (plinux_step(pid) == -1)
            return -1;
        if (plinux_getregs(pid, &jmp_reg) == -1)
            return -1;
    }

    // Restore original registers
    if (plinux_setregs(pid, &bak_reg))
        return -1;

    return jmp_reg.rax;
}

long int plinux_write_memory(pid_t pid, void *remote_addr, const void *data, size_t size)
{
    size_t i;
    union u
    {
        long val;
        char chars[sizeof(long)];
    } d;

    for (i = 0; i < size; i += sizeof(long))
    {
        memset(d.chars, 0, sizeof(long));
        size_t copy_size = (size - i >= sizeof(long)) ? sizeof(long) : (size - i);
        memcpy(d.chars, data + i, copy_size);

        if (ptrace(PTRACE_POKETEXT, pid, (uint8_t *)remote_addr + i, d.val) == -1)
        {
            perror("[*] ptrace(PTRACE_POKETEXT) failed");
            return -1;
        }
    }

    return 0;
}

void *plinux_resolve(pid_t pid, const char *symbol_name)
{
    LibraryNode *libraries = parse_libraries_from_maps(pid);
    if (!libraries)
        return NULL;

    void *symbol_address = NULL;
    LibraryNode *current = libraries;

    while (current)
    {
        symbol_address = search_symbol_in_library(current->path, symbol_name);
        if (symbol_address)
            break;
        current = current->next;
    }

    while (libraries)
    {
        LibraryNode *temp = libraries;
        libraries = libraries->next;
        free(temp->path);
        free(temp);
    }

    return symbol_address;
}