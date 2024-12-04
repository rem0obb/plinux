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
 * Node structure for a dynamically growing list of libraries.
 */
typedef struct LibraryNode
{
    char *path;
    struct LibraryNode *next;
} LibraryNode;

/**
 * Parse all libraries and files from /proc/[pid]/maps.
 *
 * @param pid Process ID.
 * @return Pointer to the head of the library list, or NULL on failure.
 */
LibraryNode *parse_libraries_from_maps(pid_t pid)
{
    char maps_path[256];
    snprintf(maps_path, sizeof(maps_path), "/proc/%d/maps", pid);

    FILE *maps_file = fopen(maps_path, "r");
    if (!maps_file)
    {
        perror("Failed to open maps file");
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
                    perror("Memory allocation failed");
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

                // Append to the list
                if (!head)
                {
                    head = tail = new_node;
                }
                else
                {
                    tail->next = new_node;
                    tail = new_node;
                }
            }
        }
    }

    fclose(maps_file);
    return head;
}

/**
 * Search for a symbol in a library using dlopen and dlsym.
 *
 * @param library Path to the library.
 * @param symbol_name Symbol name to search for.
 * @return Address of the symbol, or NULL if not found.
 */
void *search_symbol_in_library(const char *library, const char *symbol_name)
{
    void *handle = dlopen(library, RTLD_LAZY);
    if (!handle)
    {
        return NULL;
    }

    void *symbol_address = dlsym(handle, symbol_name);
    dlclose(handle); // Close the handle after usage
    return symbol_address;
}

/**
 * Resolve the address of a symbol in a process's loaded libraries.
 * 
 * @param pid Process ID to inspect.
 * @param symbol_name Symbol name to search for.
 * @return Address of the symbol, or NULL if not found.
 */
void* plinux_resolve(pid_t pid, const char* symbol_name) {
    LibraryNode* libraries = parse_libraries_from_maps(pid);
    if (!libraries) {
        return NULL;
    }

    void* symbol_address = NULL;
    LibraryNode* current = libraries;

    while (current) {
        symbol_address = search_symbol_in_library(current->path, symbol_name);

        if (symbol_address) {
            // Stop searching if the symbol is found
            break;
        }

        current = current->next;
    }

    // Free the library list
    while (libraries) {
        LibraryNode* temp = libraries;
        libraries = libraries->next;
        free(temp->path);
        free(temp);
    }

    return symbol_address;
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
plthook_t *plinux_dlopen(const char *lib)
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