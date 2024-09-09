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


#define PAGE_PRESENT    0x1      // Seite ist im Speicher
#define PAGE_RW         0x2      // Schreibrechte erlaubt
#define PAGE_USER       0x4      // User-mode (1=User, 0=Supervisor)
#define PAGE_SUPERVISOR 0x0      // Supervisor-only Zugriff$

// Extrahiere die Bits 12–21 der virtuellen Adresse für den Page Table Index
#define PAGE_TABLE_INDEX(virt_addr) (((virt_addr) >> 12) & 0x03FF)
// Extrahiere die Bits 22–31 der virtuellen Adresse für den Page Directory Index
#define PAGE_DIRECTORY_INDEX(virt_addr) (((virt_addr) >> 22) & 0x03FF)


#define KERNEL_BASE 0xC0000000

static uint32_t page_directory[4][1024] __attribute__((aligned(PAGE_SIZE)));
static uint32_t page_tables[4][1024] __attribute__((aligned(PAGE_SIZE)));

// Funktion zur Umwandlung virtueller Adressen in physische
uint32_t virtual_to_physical(uint32_t virt_addr) {
    return virt_addr - 0xC0000000;  // Annahme: Virtuelle Adressen sind 1:1 gemappt
}

extern uint32_t kernel_directory[1024];



uint32_t* create_page_directory(int process_id)
{
    uint32_t* dir = page_directory[process_id];

    for (int i = 768; i < 1024; i++)
    {
        dir[i] = kernel_directory[i];
    }

    for (int i = 0; i < 768; i++)
    {
        dir[i] = 0x00000001;
    }

    return dir;      
}


// Beispiel, um Pages zu mappen (statische Tables verwendet)
void map_process_page(int process_id, uint32_t virt_addr, uint32_t phys_addr) {
    uint32_t* dir = page_directory[process_id];
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virt_addr);
    uint32_t pt_index = PAGE_TABLE_INDEX(virt_addr);

    // Page Table sicherstellen
    if (!(dir[pd_index] & PAGE_PRESENT)) {
        dir[pd_index] = ((uint32_t) &page_tables[process_id][pd_index]) | PAGE_PRESENT | PAGE_RW | PAGE_USER;
    }

    // Page Table Eintrag setzen
    uint32_t* table = (uint32_t*)(dir[pd_index] & 0xFFFFF000);
    table[pt_index] = phys_addr | PAGE_PRESENT | PAGE_RW | PAGE_USER;
}


/*struct page_directory* create_page_directory() {
    struct page_directory* dir = (struct page_directory*) kzalloc(sizeof(struct page_directory));

    for (int i = 768; i < 780; i++) {
        dir->entries[i] = kernel_directory[i] | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;
    }

    // Heap Mapping (0xD0000000 - 0xD63FFFFF)
    dir->entries[832] = 0x03000083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD0000000 - 0xD03FFFFF
    dir->entries[833] = 0x03400083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD0400000 - 0xD07FFFFF
    dir->entries[834] = 0x03800083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD0800000 - 0xD0BFFFFF
    dir->entries[835] = 0x03C00083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD0C00000 - 0xD0FFFFFF
    dir->entries[836] = 0x04000083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD1000000 - 0xD13FFFFF
    dir->entries[837] = 0x04400083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD1400000 - 0xD17FFFFF
    dir->entries[838] = 0x04800083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD1800000 - 0xD1BFFFFF
    dir->entries[839] = 0x04C00083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD1C00000 - 0xD1FFFFFF
    dir->entries[840] = 0x05000083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD2000000 - 0xD23FFFFF
    dir->entries[841] = 0x05400083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD2400000 - 0xD27FFFFF
    dir->entries[842] = 0x05800083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD2800000 - 0xD2BFFFFF
    dir->entries[843] = 0x05C00083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD2C00000 - 0xD2FFFFFF
    dir->entries[844] = 0x06000083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD3000000 - 0xD33FFFFF
    dir->entries[845] = 0x06400083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD3400000 - 0xD37FFFFF
    dir->entries[846] = 0x06800083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD3800000 - 0xD3BFFFFF
    dir->entries[847] = 0x06C00083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD3C00000 - 0xD3FFFFFF
    dir->entries[848] = 0x07000083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD4000000 - 0xD43FFFFF
    dir->entries[849] = 0x07400083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD4400000 - 0xD47FFFFF
    dir->entries[850] = 0x07800083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD4800000 - 0xD4BFFFFF
    dir->entries[851] = 0x07C00083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD4C00000 - 0xD4FFFFFF
    dir->entries[852] = 0x08000083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD5000000 - 0xD53FFFFF
    dir->entries[853] = 0x08400083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD5400000 - 0xD57FFFFF
    dir->entries[854] = 0x08800083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD5800000 - 0xD5BFFFFF
    dir->entries[855] = 0x08C00083 | PAGE_PRESENT | PAGE_RW | PAGE_SUPERVISOR;  // 0xD5C00000 - 0xD5FFFFFF

    for (int i = 0; i < 768; i++) {
        dir->entries[i] = 0x00000002;  // Not present, RW, Supervisor
    }

    return dir;   
}

struct page_table* get_or_create_page_table(struct page_directory* dir, uint32_t virt_addr) {
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virt_addr);


    if (!(dir->entries[pd_index] & PAGE_PRESENT)) {

        struct page_table* new_table = (struct page_table*) kzalloc(sizeof(struct page_table));
        uint32_t phys_new_table = virtual_to_physical((uint32_t)new_table);
        dir->entries[pd_index] = phys_new_table | PAGE_PRESENT | PAGE_RW | PAGE_USER;

        return new_table;
    }

    return (struct page_table*)(dir->entries[pd_index] & 0xFFFFF000);  // maske für physische Adresse
}*/

/*void map_user_space(struct page_directory* dir, uint32_t virt_addr, uint32_t phys_addr) {
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virt_addr);

    struct page_table* table = get_or_create_page_table(dir, virt_addr);

    uint32_t pt_index = PAGE_TABLE_INDEX(virt_addr);

    table->entries[pt_index] = phys_addr | PAGE_PRESENT | PAGE_RW | PAGE_USER;
}*/

uint32_t* get_current_page_directory() {
    uint32_t cr3;
    asm volatile ("mov %%cr3, %0" : "=r" (cr3));
    return (uint32_t*)cr3;
}

void switch_to_kernel_directory() {
    uint32_t physical_address = virtual_to_physical((uint32_t)kernel_directory);
    load_page_directory(physical_address);
}

void load_process_directory(int process_id) {
    load_page_directory((uint32_t)page_directory[process_id] - KERNEL_BASE); // CR3 laden
}