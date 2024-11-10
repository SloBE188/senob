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
#include "../../../libk/memory.h"

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
            mem_map_page(RAMDISKVIRTUALADRESS + offset, ramdisk.phys_addr + offset, PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE);
        }
    
    
}


void init_ramdisk_disk(struct multiboot_info* mbinfo)
{

    struct multiboot_module* mod;
    if (mbinfo->mods_count > 0)
    {
        mod = (struct multiboot_module*)mbinfo->mods_addr;
        printf("Ram Disk module loaded at physical addr: %x\n", mod->mod_start);
    }
    using_ramdisk = true;
    
    memset(&ramdisk, 0x00, sizeof(struct ramdisk));
    ramdisk.phys_addr = mod->mod_start;
    ramdisk.size = mod->mod_end - mod->mod_start;

}

void disk_read_sector(void* buffer, uint32_t sector)
{
    if(sector + SECTOR_SIZE >= ramdisk.size)
    {
        printf("Not able to read sector, its out of space from the ramdisk.\n");
        return;
    }
    memcpy(buffer, RAMDISKVIRTUALADRESS + sector * SECTOR_SIZE, SECTOR_SIZE);
}

void disk_read_from_offset(void* buffer, uint32_t offset, uint32_t size)
{
    if (offset>= ramdisk.size)
    {
        printf("Not able to read sector, its out of space from the ramdisk.\n");
        return;
    }
    
        
    uint32_t address = RAMDISKVIRTUALADRESS + offset;
    printf("Reading from address %x with size %u\n", address, size);
    memcpy(buffer, (void*)address, size);
}


//DRESULT disk_write (
//  BYTE pdrv,        /* [IN] Physical drive number */
//  const BYTE* buff, /* [IN] Pointer to the data to be written */
//  LBA_t sector,     /* [IN] Sector number to write from */
//  UINT count        /* [IN] Number of sectors to write */
//);

void disk_write_sector(void* buffer, uint32_t sector, uint32_t count)
{
    if (sector + count * SECTOR_SIZE >= ramdisk.size)
    {
        printf("You wanna write out of the ramdisk, isnt possible.\n");
        return;
    }
    
    memcpy(RAMDISKVIRTUALADRESS + sector * SECTOR_SIZE, buffer, count * SECTOR_SIZE);
}

