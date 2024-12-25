#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

#define USER_STACK_TOP 0xB0000000
#define PROGRAMM_VIRTUAL_ADDRESS_START 0x00400000

struct process
{
    uint32_t pid;
    char filename[20];

    uint32_t* page_directory;


    struct thread* head_thread;
    struct thread* tail_thread;

    uint32_t priority;
    


} __attribute__((packed));

struct rb_node
{
    struct process* proc;
    uint32_t color;

    struct rb_node* parent;
    struct rb_node* left;
    struct rb_node* right;

};



struct registers_save 
{
    uint32_t eax;     //  0
    uint32_t ebx;     //  4
    uint32_t ecx;     //  8
    uint32_t edx;     //  12

    uint32_t esp;     //  16
    uint32_t ebp;     //  20

    uint32_t edi;     //  24
    uint32_t eip;     //  28
    uint32_t eflags;  //  32

    uint32_t cs;      //  36
    uint32_t ss;      //  40
    uint32_t ds;      //  44
    uint32_t es;      //  48
    uint32_t fs;      //  52
    uint32_t gs;      //  56

    uint32_t cr3;     //  60
};

struct thread
{
    uint32_t thread_id;
    struct process* owner;
    struct thread* next;
    struct thread* prev;

    struct
    {
        uint32_t eax;     //  0
        uint32_t ebx;     //  4
        uint32_t ecx;     //  8
        uint32_t edx;     //  12

        uint32_t esp;     //  16
        uint32_t ebp;     //  20

        uint32_t edi;     //  24
        uint32_t eip;     //  28
        uint32_t eflags;  //  32

        uint32_t cs;      //  36
        uint32_t ss;      //  40
        uint32_t ds;      //  44
        uint32_t es;      //  48
        uint32_t fs;      //  52
        uint32_t gs;      //  56

        uint32_t cr3;     //  60
    } regs __attribute__ ((packed));

    
    struct
    {
        uint32_t esp0;
        uint16_t ss0;
        uint32_t stack_start;
    } kstack __attribute__ ((packed));


};

struct process* create_process(const char* filename);
extern void switch_task(struct registers_save* regs);
void switch_to_thread(struct thread* thread);
void copy_program_to_address(const char* filename, uint32_t pages_needed, uint32_t program_address);
uint32_t map_program_to_address(const char* filename, uint32_t program_address);
struct registers_save* save_thread_state(struct thread* thread);
void test_avl_tree();
uint32_t init_proc();

#endif
