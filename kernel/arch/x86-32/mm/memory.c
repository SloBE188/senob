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

#include "memory.h"

void init_memory(struct multiboot_info* bootinfo)
{

    printf("&d\n", bootinfo->vbe_mode_info);
    for (int i = 0; i < bootinfo->mmap_length; i += sizeof(struct multiboot_mmap_entry)){
        struct multiboot_mmap_entry *mmmt = (struct multiboot_mmap_entry*)(bootinfo->mmap_addr + i);

        printf("Low addr: %x | High Addr: %x | Low Length: %x | High Length: %x | size: %x | Type: %d\n", mmmt->addr_low, mmmt->addr_high, mmmt->len_low, mmmt->len_high, mmmt->size, mmmt->type);
    }

    

}