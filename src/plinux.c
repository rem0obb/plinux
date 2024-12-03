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

#include <plinux.h>
#include <sys/ptrace.h>
#include <plthook.h>

long int pt_attach(pid_t pid)
{
    return ptrace(PT_ATTACH, pid, 0, 0);
}

long int
pt_detach(pid_t pid)
{
    return ptrace(PT_DETACH, pid, 0, 0);
}

long int
pt_continue(pid_t pid, int sig)
{
    return ptrace(PT_CONTINUE, pid, (caddr_t)1, sig);
}

intptr_t
pt_resolve(pid_t pid, const char* nid) {
  intptr_t addr;

  //if((addr=kernel_dynlib_resolve(pid, 0x1, nid))) {
  //  return addr;
  //}

  return 0;//kernel_dynlib_resolve(pid, 0x2001, nid);
}