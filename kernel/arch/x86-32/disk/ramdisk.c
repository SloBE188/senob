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

#include "ramdisk.h"
#include "../mm/paging/paging.h"
#include "../mm/PMM/pmm.h"
#include <stdbool.h>

bool using_ramdisk = false;
struct ramdisk ramdisk;

void create_ramdisk()
{
    if (!using_ramdisk)
        return;
    
    uint32_t pages_needed_for_ramdisk = CEIL_DIV(ramdisk.size, PAGE_SIZE);
    for (int i = 0; i < pages_needed_for_ramdisk; i++)
        {
            uint32_t offset = i * PAGE_SIZE;
            mem_map_page(RAMDISKVIRTUALADRESS + offset, ramdisk.phys_addr + offset, 0);
        }
    
    
}