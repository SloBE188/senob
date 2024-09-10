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

#include "pmm.h"
#include "../../../../libk/memory.h"
#include <stdbool.h>


#define PAGE_FRAMES (0x100000000 / 0x1000 / 8) // dez 131072
#define PAGE_FRAME_SIZE 4096

uint8_t physical_memory_bitmap[PAGE_FRAMES / 8];    //every bit represents one page


static uint32_t page_frame_min_index;
static uint32_t page_frame_max_index;
static uint32_t total_allocated;

void pmm_init(uint32_t mem_low, uint32_t mem_high)
{
    page_frame_min_index = CEIL_DIV(mem_low, PAGE_FRAME_SIZE);
    page_frame_max_index = mem_high / PAGE_FRAME_SIZE;
    total_allocated = 0;

    memset(physical_memory_bitmap, 0, sizeof(physical_memory_bitmap));
    
}

bool is_page_frame_used(uint32_t pf_index) {
    uint8_t byte = physical_memory_bitmap[pf_index >> 3];
    return byte >> (pf_index & 7) & 1;
}

void set_page_frame_used_or_free(uint32_t pf_index, bool used) {
    uint8_t byte = physical_memory_bitmap[pf_index >> 3];
    byte ^= (-used ^ byte) & (1 << (pf_index & 7));

    physical_memory_bitmap[pf_index >> 3] = byte;
}

uint32_t pmm_alloc_pageframe() {

	uint32_t start = page_frame_min_index / 8 + ((page_frame_min_index & 7) != 0 ? 1 : 0);
	uint32_t end = page_frame_max_index / 8 - ((page_frame_max_index & 7) != 0 ? 1 : 0);

	for (uint32_t b = start; b < end; b++) {
		uint8_t byte = physical_memory_bitmap[b];
        // check for a byte which isnt fully used
		if (byte == 0xFF)
			continue;

        // iterate threw the 8 bits from the byte
		for (uint32_t i = 0; i < 8; i++) {
			bool used = byte >> i & 1;

			if (!used) {
				byte ^= (-1 ^ byte) & (1 << i);
				physical_memory_bitmap[b] = byte;
				total_allocated++;
				uint32_t addr = (b * 8 + i) * PAGE_FRAME_SIZE;
				return addr;
			}
		}
	}

	//assert_msg(false, "OUT OF MEMORY");
	return 0;
}

void pmm_free_pageframe(uint32_t addr) {
	uint32_t pf_index = addr / PAGE_FRAME_SIZE;


    if (is_page_frame_used(pf_index))
    {
        printf("Page is already free");
        return;
    }

	set_page_frame_used_or_free(pf_index, 0);
	total_allocated--;
}

uint32_t pmm_get_total_allocated_pages() {
    return total_allocated;
}
