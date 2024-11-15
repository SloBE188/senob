/*
 * Copyright (C) 2024 Nils Burkard
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>
#include "../mm/paging/paging.h"
#include "../mm/PMM/pmm.h"
#include <stdbool.h>
#include "../gdt/gdt.h"
#include "process.h"


#include <stdint-gcc.h>
#include <stdbool.h>

#define KERNEL_CODE_SEGMENT 0x08
#define KERNEL_DATA_SEGMENT 0x10
#define USER_CODE_SEGMENT 0x18
#define USER_DATA_SEGMENT 0x20
#define INTERRUPTS_ENABLED 0x200
#define RPL_USER 3

#define USER_STACK_TOP 0xB0000000
#define USER_STACK_BOTTOM 0xAFFFE000
#define USER_STACK_PAGES 16

#define KERNEL_STACK_SIZE 0x4000
#define USER_STACK_SIZE 0x4000



#endif