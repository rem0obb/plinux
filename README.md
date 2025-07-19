# PLINUX

PLINUX is a C library for remote process manipulation on Linux using `ptrace`. It allows remote function calls, symbol resolution at runtime, and direct memory writing to another process.

> [!NOTE]  
> I did not create this technique, I just replicated it in Linux processes, this technique is used in Buzzer Nines, for injecting ELF binaries into the PS5  [project here](https://github.com/buzzer-re/NineS)


## Example Usage

$ sudo ./example 1234  
[*] Successfully attached to process 1234  
[*] Resolved address for 'malloc': 0x7f...  
[*] Memory malloc allocated in '7f...'  
[*] Resolved address for 'puts': 0x7f...  
[*] Successfully detached from process 1234

[example here](/example/)

## Features

- Attach and detach from processes using `ptrace`  
- Resolve remote symbols like `malloc` and `puts`  
- Call remote functions with arguments  
- Write directly into the memory of a target process  

## API Documentation

### long int plinux_attach(pid_t pid)
Attaches to the target process using ptrace(PTRACE_ATTACH).

Returns:
- 0 on success
- -1 on failure

---

### long int plinux_detach(pid_t pid)
Detaches from the target process using ptrace(PTRACE_DETACH).

Returns:
- 0 on success
- -1 on failure

---

### long int plinux_continue(pid_t pid, int sig)
Continues execution of a stopped process using ptrace(PTRACE_CONT), optionally sending a signal.

Parameters:
- pid: Target process ID
- sig: Signal to send (0 = none)

Returns:
- 0 on success
- -1 on failure

---

### long int plinux_write_memory(pid_t pid, void *remote_addr, const void *data, size_t size)
Writes a block of memory to the target process using ptrace(PTRACE_POKEDATA).

Parameters:
- pid: Target process ID
- remote_addr: Target memory address
- data: Local data pointer
- size: Number of bytes to write

Returns:
- 0 on success
- -1 on failure

---

### long int plinux_step(pid_t pid)
Executes a single instruction in the target process using ptrace(PTRACE_SINGLESTEP).

Returns:
- 0 on success
- -1 on failure

---

### long int plinux_getregs(pid_t pid, struct user_regs_struct *r)
Fetches the general purpose registers from the target process.

Parameters:
- pid: Target process ID
- r: Pointer to user_regs_struct to store register values

Returns:
- 0 on success
- -1 on failure

---

### long int plinux_setregs(pid_t pid, const struct user_regs_struct *r)
Sets the general purpose registers of the target process.

Parameters:
- pid: Target process ID
- r: Pointer to user_regs_struct containing register values to set

Returns:
- 0 on success
- -1 on failure

---

### long plinux_call(pid_t pid, void *addr, ...)
Calls a remote function at the specified address with variadic arguments.

Parameters:
- pid: Target process ID
- addr: Address of the remote function
- ...: Function arguments (follow architecture calling convention)

Returns:
- Return value of the function (e.g., RAX on x86_64)

---

### void *plinux_resolve(pid_t pid, const char *symbol_name)
Resolves the address of a symbol (e.g., malloc, puts) in the target process.

Parameters:
- pid: Target process ID
- symbol_name: Name of the symbol to resolve

Returns:
- Pointer to the symbol address
- NULL on failure

## Requirements

- Linux  
- GCC  
- sudo/root privileges  
- ASLR disabled

## License

Copyright (C) 2024 remoob

This program is free software; you can redistribute it and/or modify it  
under the terms of the GNU General Public License as published by the Free Software Foundation;  
either version 3, or (at your option) any later version.

This program is distributed in the hope that it will be useful,  
but WITHOUT ANY WARRANTY; without even the implied warranty of  
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the  
GNU General Public License for more details.

You should have received a copy of the GNU General Public License  
along with this program. If not, see http://www.gnu.org/licenses/

## Disclaimer

This software is provided for educational purposes only.  
Unauthorized interaction with other processes may be illegal.  
Use responsibly.
