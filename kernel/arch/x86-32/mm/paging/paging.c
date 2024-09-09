#include "paging.h"
#include "../heap/heap.h"
#include "../../../../libk/memory.h"
#include "../../kernel.h"

#define PAGE_PRESENT    0x1      // Seite ist im Speicher
#define PAGE_RW         0x2      // Schreibrechte erlaubt
#define PAGE_USER       0x4      // User-mode (1=User, 0=Supervisor)
#define PAGE_SUPERVISOR 0x0      // Supervisor-only Zugriff

// extract bits 12-21 of the virtual address for the page table index
#define PAGE_TABLE_INDEX(virt_addr) (((virt_addr) >> 12) & 0x03FF)
// extract bits 22-31 of the virtual address for the page directory index
#define PAGE_DIRECTORY_INDEX(virt_addr) (((virt_addr) >> 22) & 0x03FF)

#define KERNEL_BASE 0xC0000000

// Statische Definition für Prozess Page Directories und Page Tables
static struct page_directory process_page_directories[4] __attribute__((aligned(PAGE_SIZE)));
static struct page_table process_page_tables[8][1024] __attribute__((aligned(PAGE_SIZE)));

uint32_t virtual_to_physical(uint32_t virt_addr) {
    return virt_addr - 0xC0000000;
}

extern uint32_t kernel_directory[1024];


struct page_directory* create_page_directory(int process_id) {
    struct page_directory* dir = &process_page_directories[process_id];


    for (int i = 768; i < 1024; i++) {
        dir->entries[i] = kernel_directory[i];
    }


    for (int i = 0; i < 768; i++) {
        dir->entries[i] = 0x00000001; // leeres mapping
    }

    return dir;
}

// Funktion zum Mapping eines virtuellen Bereichs auf einen physischen für den Prozess
void map_process_space(struct page_directory* dir, uint32_t virt_addr, uint32_t phys_addr) {
    uint32_t pd_index = PAGE_DIRECTORY_INDEX(virt_addr);
    uint32_t pt_index = PAGE_TABLE_INDEX(virt_addr);

    struct page_table* table = &process_page_tables[pd_index]; 

    // set entry in the page directory
    dir->entries[pd_index] = ((uint32_t)table) | PAGE_PRESENT | PAGE_RW | PAGE_USER;

    // map virtual to physical address
    table->entries[pt_index] = phys_addr | PAGE_PRESENT | PAGE_RW | PAGE_USER;
}

void load_process_directory(int process_id) {
    struct page_directory* dir = &process_page_directories[process_id];
    uint32_t phys_addr = virtual_to_physical((uint32_t)dir);
    load_page_directory(phys_addr); // load CR3
}

