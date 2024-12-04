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

#pragma once

#include <plthook.h>
#include <sys/types.h>

long int plinux_attach(pid_t pid);

long int
plinux_detach(pid_t pid);

long int
plinux_continue(pid_t pid, int sig);

plthook_t *plinux_dlopen_self();
void *plinux_resolve(plthook_t *plthook, const char *nid);
plthook_t *plinux_dlopen(const char *lib);