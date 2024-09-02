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

#include "paging.h"
#include "../heap/heap.h"
#include "../../../../libk/memory.h"
#include "../../kernel.h"

extern void load_page_directory(uint32_t* dir);

extern uint32_t kernel_directory[1024];





uint32_t* get_current_page_directory() {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r" (cr3));
    return (uint32_t*)cr3;
}


