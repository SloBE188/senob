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
#include "../../../../libk/memory.h"


struct heap kernel_heap;
struct heap_table kernel_heap_table;

void heap_init() {
    kernel_heap_table.total_blocks = HEAP_SIZE / HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)HEAP_START_ADDRESS;

    kernel_heap.table = &kernel_heap_table;
    kernel_heap.start_address = (void*)HEAP_START_ADDRESS;

    for (size_t i = 0; i < kernel_heap_table.total_blocks; i++) {
        kernel_heap_table.entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
    }
}


int heap_get_start_block(struct heap* heap, int total_blocks)
{
    struct heap_table* table = heap->table;
    int blockcount = 0;
    int blockstart = -1;

    for (size_t i = 0; i < table->total_blocks; i++)
    {
        if (heap->table->entries[i] != HEAP_BLOCK_TABLE_ENTRY_FREE)
        {
            blockcount = 0;
            blockstart = -1;
            continue;
        }

        if (blockstart == -1)
        {
            blockstart = i;
        }
        blockcount++;
        if (blockcount == total_blocks)
        {
            break;
        }
        
    }

    if(blockstart == -1)
    {
        printf("No startblock was found\n");
    }
    

    return blockstart;
}
static uint32_t heap_align_value_to_upper(uint32_t val)
{
    if ((val % HEAP_BLOCK_SIZE) == 0)
    {
        return val;
    }

    val = (val - ( val % HEAP_BLOCK_SIZE));
    val += HEAP_BLOCK_SIZE;
    return val;
}
void* heap_malloc_blocks(struct heap* heap, int total_blocks)
{
    void * address = 0;

    int startblock = heap_get_start_block(heap, total_blocks);
    if (startblock < 0)
    {
        goto out;
    }

    // Convert start block index to memory address
    address = heap->start_address + (startblock * HEAP_BLOCK_SIZE);
    
    // Mark the blocks as taken
    int endblock = (startblock + total_blocks) - 1;
    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_IS_FIRST;

    if (total_blocks > 1)
    {
        entry |= HEAP_BLOCK_HAS_NEXT;
    }
    for (int i = startblock; i <= endblock; i++)
    {
        if (i == startblock)
        {
            entry = HEAP_BLOCK_IS_FIRST;
            if (total_blocks > 1)
            {
                entry |= HEAP_BLOCK_HAS_NEXT;
            }
        }
        else if (i == endblock)
        {
            entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        }
        else
        {
            entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_HAS_NEXT;
        }

        heap->table->entries[i] = entry;
    }

    
out:
    return address;
}


void* heap_malloc(struct heap* heap, size_t size)
{
    size_t aligned_number_to_allocate = heap_align_value_to_upper(size);
    int decimal_total_blocks = aligned_number_to_allocate / HEAP_BLOCK_SIZE;
    return heap_malloc_blocks(heap, decimal_total_blocks);
}

void* kmalloc(size_t size)
{
    return heap_malloc(&kernel_heap, size);
}

void* kzalloc(size_t size)
{
    void* ptr = kmalloc(size);
    if (ptr == 0)
    {
        return 0;
    }
    memset(ptr, 0x00, size);

    return ptr;
}



void heap_mark_blocks_free(struct heap* heap, int startblock)
{
    struct heap_table* table = heap->table;
    for (int i = startblock; i < table->total_blocks; i++)
    {
        HEAP_BLOCK_TABLE_ENTRY entry = table->entries[i];
        table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!entry & HEAP_BLOCK_HAS_NEXT)
        {
            break;
        }
        
    }
    
}

int heap_address_to_block(struct heap* heap, void* address)
{
    return ((int)(address - heap->start_address)) / HEAP_BLOCK_SIZE;
}

void* heap_free(struct heap* heap, void* ptr)
{
    heap_mark_blocks_free(heap, heap_address_to_block(heap, ptr));
}

void kfree(void* ptr)
{
    heap_free(&kernel_heap, ptr);
}