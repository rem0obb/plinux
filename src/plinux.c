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

long int plinux_step(pid_t pid)
{
    if (ptrace(PTRACE_SINGLESTEP, pid, NULL, NULL) == -1)
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

    jmp_reg.rax = 0; /* I need to reset rax
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

    if (plinux_getregs(pid, &jmp_reg) == -1)
    {
        printf("ERROR plinux_getregs\n");
        return -1;
    }

    printf("rax=%p\n", jmp_reg.rax);

    // Continue until return (rsp grows back)
    while (jmp_reg.rsp <= bak_reg.rsp)
    {
        // printf("[*] RSP = %p RIP = %p\n", jmp_reg.rsp, jmp_reg.rip);
        if (plinux_step(pid) == -1)
        {
            printf("ERROR plinux_step\n");
            return -1;
        }
        if (plinux_getregs(pid, &jmp_reg) == -1)
        {
            printf("ERROR plinux_getregs\n");
            return -1;
        }
    }

    // Restore original registers
    if (plinux_setregs(pid, &bak_reg))
        return -1;

    return jmp_reg.rax;
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