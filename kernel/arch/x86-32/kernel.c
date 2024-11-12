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

//#include "../../../drivers/video/vga/vga.h"
#include "kernel.h"
#include "gdt/gdt.h"
#include "interrupts/idt.h"
#include "interrupts/pit.h"
#include "../../../drivers/keyboard/keyboard.h"
#include "multiboot.h"
#include "../../libk/stdiok.h"
#include "../../../drivers/video/vbe/vbe.h"
#include "../../../drivers/video/vbe/wm/window.h"
#include "../../../drivers/video/vbe/font.h"
#include "mm/heap/heap.h"
#include "mm/paging/paging.h"
#include "mm/PMM/pmm.h"
#include "sys/process.h"
#include "sys/thread.h"
#include "../../libk/memory.h"
#include "disk/ramdisk.h"
#include "fatfs/ff.h"
#include "fatfs/diskio.h"


extern void rust_testfunction();

#define magic 0x1BADB002

void kernel_panic(const char* message) 
{
    printf("Kernel Panic: %s\n", message);
    while (1);
}

struct vbe_info vbeinfo;
void kernel_main(uint32_t magic_value, struct multiboot_info* multibootinfo)
{
    init_gdt();
    idt_init();
    init_keyboard();
    if (magic_value != 0x2BADB002)
    {
        printf("Invalid magic value: %x\n", magic_value);
        kernel_panic("Invalid magic value");
    }
    

    vbeinfo.framebuffer_addr = multibootinfo->framebuffer_addr;
    vbeinfo.framebuffer_pitch = multibootinfo->framebuffer_pitch;
    vbeinfo.framebuffer_width = multibootinfo->framebuffer_width;
    vbeinfo.framebuffer_height = multibootinfo->framebuffer_height;
    vbeinfo.framebuffer_bpp = multibootinfo->framebuffer_bpp;

    init_vbe(&vbeinfo);
    set_vbe_info(&vbeinfo);
    //draw_rectangle(212, 300, 400, 100, COLOR_BLUE, &vbeinfo);
    //draw_string(450, 300, "Herzlich willkommen bei senob ;)", COLOR_GREEN, &vbeinfo);

    init_ramdisk_disk(multibootinfo);

    uint32_t physicalAllocStart = 0x100000 * 16;

    if (using_ramdisk)
    {
        physicalAllocStart = (ramdisk.phys_addr + ramdisk.size + 0xFFF) & 0xFFFFF000;

    }

    init_memory(multibootinfo->mem_upper * 1024, physicalAllocStart);       //mem_upper comes in KiB (hex 0x1fb80), so it gets multiplied by 1024 so i got it in bytes
    heap_init();
    printf("Module: %d\n", multibootinfo->mods_count);
    create_ramdisk();

    disk_initialize(0);
    disk_status(0);

    char buffer[512];
    char readin[20] = "nilsnilsnilsnilsnils";
    //disk_read_from_offset(buffer, 0x6200, 512);
    //disk_read_sector(buffer, 49, 1);
    disk_read(0, buffer, 64, 1);
    printf("Wurde gelesen: %s\n", buffer);
    //disk_write_sector(readin, 75, 1);
    disk_write(0, readin, 75, 1);

    char result[20];
    disk_read(0, result, 75, 1);
    printf("2Wurde gelesen: %s\n", result);



    FATFS fs;
    FRESULT res;

    res = f_mount(&fs, "0:", 1);
    printf("result of mount: %d\n", res);
    
    if (res != FR_OK)
    {
        printf("Filesystem mount failed.\n");
    }else
    {
        printf("Filesystem mounted successfully!\n");
    }
    
    FIL fil;        // File object
    FRESULT res1;
    char buffer4[64]; 

    res1 = f_open(&fil, "0:test.txt", FA_READ);
    if (res1 == FR_OK) {
        UINT br;    //referenzparameter
        res1 = f_read(&fil, buffer4, sizeof(buffer4) - 1, &br);
        if (res1 == FR_OK) {
            buffer4[br] = '\0'; // Null-terminating string
            printf("File content: %s\n", buffer4);
        }
        f_close(&fil);
    } else {
        printf("Failed to open file with error code: %d\n", res1);
    }


    FIL logfile;
    UINT byteswritten;

    f_open(&logfile, "logs.txt", FA_WRITE | FA_CREATE_ALWAYS);
    char* logstarttext = "The start of the logs";
    f_write(&logfile, logstarttext, strlen(logstarttext), &byteswritten);
    f_close(&logfile);



    DIR dir;
    FILINFO fno;

    if (f_opendir(&dir, "0:/") == FR_OK) {
        while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0] != 0) {
            printf("Gefunden: %s\n", fno.fname);
        }
        f_closedir(&dir);
    }


    //FRESULT f_read
	//FIL* fp, 	/* Open file to be read */
	//void* buff,	/* Data buffer to store the read data */
	//UINT btr,	/* Number of bytes to read */
	//UINT* br	/* Number of bytes read */

    FIL readlog;
    FRESULT check;
    
    char buffrrr[512];
    UINT bytesreadd;

    check = f_open(&readlog, "0:logs.txt", FA_READ);
    if (check == FR_OK)
    {
        check = f_read(&readlog, &buffrrr, strlen(buffrrr) - 1, &bytesreadd);
        if (check == FR_OK)
        {
            buffrrr[bytesreadd] = '\0';
            printf("Read: %s\n", buffrrr);
        }
        f_close(&readlog);        
    }else{
        printf("cry\n");
    }
    

    
    FRESULT check2;
    check2 = f_mkdir("0:/senob");


    FIL fnfile;
    UINT byteswritten2;

    f_open(&fnfile, "0:/senob/fnfile.txt", FA_WRITE | FA_CREATE_ALWAYS);
    char* fn = "it is what it is";
    f_write(&fnfile, logstarttext, strlen(logstarttext), &byteswritten2);
    f_close(&fnfile);



    DIR dir2;
    FILINFO fno2;

    if (f_opendir(&dir2, "0:/senob") == FR_OK) {
        while (f_readdir(&dir2, &fno2) == FR_OK && fno2.fname[0] != 0) {
            printf("Gefunden: %s\n", fno2.fname);
        }
        f_closedir(&dir2);
    }
    /*char buffer2 = 'A';
    
    disk_write_sector(buffer2, 75, 1);
    disk_read_sector(buffer2, 75, 1);
    printf("Wurde gelesen: %c\n", buffer2);*/
    
    


    while (1){}
    
}