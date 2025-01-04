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
#include "../paging/paging.h"
#include "../PMM/pmm.h"

struct heap kernel_heap;
struct heap_table kernel_heap_table;

uint32_t heap_size = 0;
uint32_t heap_start = HEAP_START_ADDRESS;

#define INITIAL_HEAP_SIZE (HEAP_BLOCK_SIZE * 4)

void heap_init() {
    kernel_heap_table.total_blocks = INITIAL_HEAP_SIZE / HEAP_BLOCK_SIZE;
    kernel_heap_table.entries = (HEAP_BLOCK_TABLE_ENTRY*)HEAP_START_ADDRESS;
    kernel_heap.table = &kernel_heap_table;
    kernel_heap.start_address = (void*)HEAP_START_ADDRESS;

    //initial heap size mapping
    for (int i = 0; i < kernel_heap_table.total_blocks; i++) {
        uint32_t phys_addr = pmm_alloc_pageframe();
        if (phys_addr == 0) break;
        mem_map_page(HEAP_START_ADDRESS + i * HEAP_BLOCK_SIZE, phys_addr, PAGE_FLAG_WRITE | PAGE_FLAG_PRESENT);
        kernel_heap_table.entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
    }
    heap_size = INITIAL_HEAP_SIZE;
}

//get the starting block for a new mapping
int heap_get_start_block(struct heap* heap, int total_blocks) 
{
    int current_run_start = -1;
    int current_run_length = 0;

    for (int i = 0; i < heap->table->total_blocks; i++) {
        if (heap->table->entries[i] == HEAP_BLOCK_TABLE_ENTRY_FREE) {
            if (current_run_start == -1) {
                current_run_start = i;
            }
            current_run_length++;
            if (current_run_length == total_blocks) {
                return current_run_start;
            }
        } else {
            current_run_start = -1;
            current_run_length = 0;
        }
    }
    return -1;
}

void* heap_malloc_blocks(struct heap* heap, int total_blocks) {
    int startblock = heap_get_start_block(heap, total_blocks);
    if (startblock < 0) return 0;

    void* address = heap->start_address + (startblock * HEAP_BLOCK_SIZE);
    int endblock = (startblock + total_blocks) - 1;
    HEAP_BLOCK_TABLE_ENTRY entry = HEAP_BLOCK_IS_FIRST;

    if (total_blocks > 1) entry |= HEAP_BLOCK_HAS_NEXT;
    for (int i = startblock; i <= endblock; i++) {
        if (i == startblock) {
            entry = HEAP_BLOCK_IS_FIRST | (total_blocks > 1 ? HEAP_BLOCK_HAS_NEXT : 0);
        } else if (i == endblock) {
            entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN;
        } else {
            entry = HEAP_BLOCK_TABLE_ENTRY_TAKEN | HEAP_BLOCK_HAS_NEXT;
        }
        heap->table->entries[i] = entry;
    }
    return address;
}

bool heap_expand(struct heap *heap, size_t size) {
    size_t additional_blocks = size / HEAP_BLOCK_SIZE;
    for (size_t i = 0; i < additional_blocks; i++) {
        uint32_t phys_addr = pmm_alloc_pageframe();
        if (phys_addr == 0) return false;
        mem_map_page(heap_start + heap_size, phys_addr, PAGE_FLAG_WRITE | PAGE_FLAG_PRESENT);
        heap->table->entries[heap->table->total_blocks++] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        heap_size += HEAP_BLOCK_SIZE;
    }
    return true;
}

void* heap_malloc(struct heap* heap, size_t size) {
    size_t aligned_number_to_allocate = (size + HEAP_BLOCK_SIZE - 1) & ~(HEAP_BLOCK_SIZE - 1);
    int decimal_total_blocks = (aligned_number_to_allocate + HEAP_BLOCK_SIZE - 1) / HEAP_BLOCK_SIZE;

    int startblock = heap_get_start_block(heap, decimal_total_blocks);
    if (startblock == -1) {
        if (!heap_expand(heap, decimal_total_blocks * HEAP_BLOCK_SIZE)) return 0;
        startblock = heap_get_start_block(heap, decimal_total_blocks);
        if (startblock == -1) return 0;
    }
    return heap_malloc_blocks(heap, decimal_total_blocks);
}

void* kmalloc(size_t size) {
    return heap_malloc(&kernel_heap, size);
}

void* kzalloc(size_t size) {
    void* ptr = kmalloc(size);
    if (ptr) memset(ptr, 0x00, size);
    return ptr;
}

void heap_mark_blocks_free(struct heap* heap, int startblock) {
    for (int i = startblock; i < heap->table->total_blocks; i++) {
        HEAP_BLOCK_TABLE_ENTRY entry = heap->table->entries[i];
        heap->table->entries[i] = HEAP_BLOCK_TABLE_ENTRY_FREE;
        if (!(entry & HEAP_BLOCK_HAS_NEXT)) break;
    }
}

int heap_address_to_block(struct heap* heap, void* address) {
    return ((int)(address - heap->start_address)) / HEAP_BLOCK_SIZE;
}

void heap_contract(struct heap *heap) {
    int last_block = heap->table->total_blocks - 1;
    while (last_block >= 0 && heap->table->entries[last_block] == HEAP_BLOCK_TABLE_ENTRY_FREE) {
        void *virtual_addr = heap->start_address + (last_block * HEAP_BLOCK_SIZE);
        mem_unmap_page((uint32_t)virtual_addr);
        heap->table->total_blocks--;
        heap_size -= HEAP_BLOCK_SIZE;
        last_block--;
    }
}

void* heap_free(struct heap* heap, void* ptr) {
    int startblock = heap_address_to_block(heap, ptr);
    heap_mark_blocks_free(heap, startblock);
    heap_contract(heap);
    return 0;
}

void kfree(void* ptr) {
    heap_free(&kernel_heap, ptr);
}


#define ALLOC_SIZE 16384
#define ALLOC_COUNT 1000
#define REUSE_COUNT 10

void test_heap_shrink_and_reuse() {
    kernel_write("Starting heap shrink and reuse test...\n");

    void* blocks[ALLOC_COUNT];
    int i;
    for (i = 0; i < ALLOC_COUNT; i++) {
        blocks[i] = kmalloc(ALLOC_SIZE);
        if (blocks[i] == NULL) {
            kernel_write("Allocation failed at index %d\n", i);
            break;
        }
    }
    int allocated_count = i;
    kernel_write("Allocated %d blocks to trigger expansion.\n", allocated_count);

    for (i = 0; i < allocated_count; i++) {
        kfree(blocks[i]);
    }
    kernel_write("Freed all %d blocks to trigger contraction.\n", allocated_count);

    void* reused_blocks[REUSE_COUNT];
    kernel_write("Reallocating %d blocks to check address reuse:\n", REUSE_COUNT);
    for (i = 0; i < REUSE_COUNT; i++) {
        reused_blocks[i] = kmalloc(ALLOC_SIZE);
        kernel_write("Reallocated block %d at address: %p\n", i, reused_blocks[i]);
        if (reused_blocks[i] != blocks[i]) {
            kernel_write("Unexpected address for reallocated block %d. Got: %p, expected: %p\n", i, reused_blocks[i], blocks[i]);
        }
    }

    kernel_write("Heap shrink and multiple address reuse test completed.\n");
}
