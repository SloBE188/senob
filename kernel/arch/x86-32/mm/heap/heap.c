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

#include "heap.h"


struct heap kernel_heap;
struct heap_table kernel_heap_table;

heap_init()
{
    kernel_heap_table.total_blocks = HEAP_SIZE/HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)(HEAP_START_ADDRESS - kernel_heap_table.total_blocks);     //0xCFFFFFE7

    kernel_heap.table = &kernel_heap_table;
    kernel_heap.start_address = (void*)HEAP_START_ADDRESS;

    for (int i = 0; i < kernel_heap_table.total_blocks; i++)
    {
        kernel_heap_table.entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
    }
    
}

void *malloc(size_t size) {
    size_t total_blocks_needed = (size + HEAP_BLOCK_SIZE - 1) / HEAP_BLOCK_SIZE;

    size_t block_index = 0;
    size_t found_blocks = 0;

    // Suche nach einem zusammenhängenden Bereich von freien Blöcken
    for (block_index = 0; block_index < kernel_heap_table.total_blocks; block_index++) {
        if (kernel_heap_table.entries[block_index] == HEAP_BLOCK_TABLE_ENTRY_FREE) {
            found_blocks++;
        } else {
            found_blocks = 0;
        }

        if (found_blocks == total_blocks_needed) {
            break;
        }
    }

    // Wenn kein ausreichender Bereich gefunden wurde, gib NULL zurück
    if (found_blocks < total_blocks_needed) {
        return NULL;
    }

    // Markiere die Blöcke als belegt und setze die entsprechenden Flags
    size_t start_block = block_index - (total_blocks_needed - 1);

    kernel_heap_table.entries[start_block] = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_IS_FIRST;
    for (size_t i = 1; i < total_blocks_needed; i++) {
        kernel_heap_table.entries[start_block + i] = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_HAS_NEXT;
    }

    // Gib die Startadresse des zugewiesenen Speicherbereichs zurück
    return (void *)((uintptr_t)kernel_heap.start_address + (start_block * HEAP_BLOCK_SIZE));
}


void free(void *ptr) {
    uintptr_t addr = (uintptr_t)ptr;
    size_t start_block = (addr - (uintptr_t)kernel_heap.start_address) / HEAP_BLOCK_SIZE;

    // Gehe durch die Blöcke und markiere sie als frei
    for (size_t i = start_block; i < kernel_heap.table->total_blocks; i++) {
        HEAP_BLOCK_TABLE_ENTRY entry = kernel_heap_table.entries[i];
        kernel_heap_table.entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;

        // Wenn das aktuelle Block keinen "next" mehr hat, höre auf
        if (!(entry & HEAP_BLOCK_HAS_NEXT)) {
            break;
        }
    }
}
