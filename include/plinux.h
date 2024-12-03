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

#include <sys/types.h>
#include <stdint.h>

long int pt_attach(pid_t pid);

long int
pt_detach(pid_t pid);

long int
pt_continue(pid_t pid, int sig);

intptr_t
pt_resolve(pid_t pid, const char *nid);