#include "process.h"
#include <stdint.h>
#include "../mm/heap/heap.h"
#include <string.h>
#include <stdio.h>
#include "../mm/paging/paging.h"
#include "../gdt/gdt.h"
#include "../fatfs/ff.h"
#include "../mm/PMM/pmm.h"
#include "../../../../drivers/video/vbe/vbe.h"
#include "sched.h"
#include "spinlock.h"


uint32_t UProcessID = 100;
uint32_t KProcessID = 20;
uint32_t threadID = 0;

struct process* root = NULL;
struct process* NIL = NULL;

struct spinlock rbTreeLock;


//Returns a new process id
uint32_t getProcessID(struct process* proc)
{
    if (proc->isUserProc)
    {
        UProcessID++;
        return UProcessID;
    }else
    {
        KProcessID++;
        return KProcessID;
    }
    
}

//Returns thread id
uint32_t getThreadID()
{
    threadID++;
    return threadID;
}


//iterates over the processes from the RB-BST and calls the function in callback with every found process.
//@callback = functio pointer. This function gets called everytime a process got found.
//@x = process (node) from which to search from.
void inOrderTraversal(struct process *x, void (*callback)(struct process*))
{
    if (x != NIL)
    {
        inOrderTraversal(x->left, callback);
        printf("%d %s ", x->pid, (x->color == 0) ? "Black" : "Red");
        callback(x);
        inOrderTraversal(x->right, callback);
    }
}

// left rotation in rb bst
void leftRotate(struct process *x)
{
    struct process *y = x->right;
    x->right = y->left;
    if (y->left != NIL)
    {
        y->left->parent = x;
    }
    y->parent = x->parent;
    if (x->parent == NULL)
    {
        root = y;
    }
    else if (x == x->parent->left)
    {
        x->parent->left = y;
    }
    else
    {
        x->parent->right = y;
    }
    y->left = x;
    x->parent = y;
}

//Right rotation in rb bst
void rightRotate(struct process *y)
{
    struct process *x = y->left;
    y->left = x->right;
    if (x->right != NIL)
    {
        x->right->parent = y;
    }
    x->parent = y->parent;
    if (y->parent == NULL)
    {
        root = x;
    }
    else if (y == y->parent->left)
    {
        y->parent->left = x;
    }
    else
    {
        y->parent->right = x;
    }
    x->right = y;
    y->parent = x;
}


//Balances the tree after a isnertion and checks for violations
//@newNode = new inserted node (process)
void fixupInsert(struct process *newNode)
{
    //parent yes & RED
    while (newNode->parent != NULL && newNode->parent->color == RED)
    {

        if (newNode->parent == NULL || newNode->parent->parent == NULL)
            break; //no fix needed

        //is the parent node a left or right node (grandnodes)?
        if (newNode->parent == newNode->parent->parent->left)
        {
            struct process *y = newNode->parent->parent->right; // uncle of node
            if (y != NIL && y->color == RED)
            {
                //nodes uncle is red
                newNode->parent->color = BLACK;       //set parent black (else it would violate the properties of a rb)
                y->color = BLACK;                      //uncle black
                newNode->parent->parent->color = RED; //grandparents red
                newNode = newNode->parent->parent;   //move to grandparent
            }
            else //uncle is black
            {    //newNode is a right child
                if (newNode == newNode->parent->right)
                {
                    //case 2
                    newNode = newNode->parent;
                    leftRotate(newNode);
                }
                //case 3, newNode is a left child
                newNode->parent->color = BLACK;
                newNode->parent->parent->color = RED;
                rightRotate(newNode->parent->parent);
            }
            //parent node is a right node from its parents
        }
        else
        {
            struct process *y = newNode->parent->parent->left;
            if (y->color == RED)
            {
                newNode->parent->color = BLACK;
                y->color = BLACK;
                newNode->parent->parent->color = RED;
                newNode = newNode->parent->parent;
            }
            else
            {
                if (newNode == newNode->parent->left)
                {
                    newNode = newNode->parent;
                    rightRotate(newNode);
                }
                newNode->parent->color = BLACK;
                newNode->parent->parent->color = RED;
                leftRotate(newNode->parent->parent);
            }
        }
    }
    root->color = BLACK; //has to be black ALWAYS
}


//Inserts a new node (a new node is a process) into the rb bst
//@newProcess = process to insert
void rbInsert(struct process* newProcess)
{

    kernel_write("inserting process with PID: %d\n", newProcess->pid);

    struct process *y = NULL; //follows the parent node
    struct process *x = root;

    //normal bst insertion (as long as x ist null(end of the path), y to x(root))
    while (x != NIL)
    {
        y = x;
        if (newProcess->pid < x->pid)
        {
            x = x->left;
        }
        else
        {
            x = x->right;
        }
    }

    newProcess->parent = y;
    if (y == NULL)
    {
        root = newProcess;
    }
    else if (newProcess->pid < y->pid)
    {
        y->left = newProcess;
    }
    else
    {
        y->right = newProcess;
    }

    newProcess->left = NIL;
    newProcess->right = NIL;
    newProcess->color = RED;

    //fix possible violations for rb tree
    fixupInsert(newProcess);
}

//Iterates threw the rb bst and searches for a process with the given pid
//@pid = pid of the process which the function should search for
struct process *rbSearch(uint32_t pid)
{
    acquire(&rbTreeLock);
    struct process *current = root;
    while (current != NIL && current->pid != pid)
    {
        if (pid < current->pid)
            current = current->left;
        else
            current = current->right;
    }
    release(&rbTreeLock);

    return current;
}



//Searches the leftmost process (node) so the one with the smallest pid
//@x = standpoint from where to search the leftmost process (node)
struct process* treeMinimum(struct process *x)
{
    while (x->left != NIL)
    {
        x = x->left;
    }
    return x;
}


//Replaces one process (node) with another one
//@u = process (node) to be replaced
//@v = process (node) that replaced u
void rbTransplant(struct process *u, struct process *v)
{
    //u gets replaced by  v as a parent
    if (u->parent == NULL)
    {
        root = v;
    }
    else if (u == u->parent->left)
    {
        u->parent->left = v;
    }
    else
    {
        u->parent->right = v;
    }
    v->parent = u->parent;
}


//Balances the tree after a deletion and checks for violations
void rbDeleteFixup(struct process *x)
{
    while (x != root && x->color == BLACK)
    {
        if (x == x->parent->left)
        {
            struct process *w = x->parent->right; //siblings from x
            //case 1: w is red
            if (w->color == RED)
            {
                w->color = BLACK;
                x->parent->color = RED;
                leftRotate(x->parent);
                w = x->parent->right;
            }
            //case 2: w is black and both kids are black
            if (w->left->color == BLACK && w->right->color == BLACK)
            {
                w->color = RED;
                x = x->parent;
            }
            else
            {
                //case 3: w is black, w->left is red, w->right is black
                if (w->right->color == BLACK)
                {
                    w->left->color = BLACK;
                    w->color = RED;
                    rightRotate(w);
                    w = x->parent->right;
                }
                //case 4: w->right is red
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                leftRotate(x->parent);
                x = root;
            }
        }
        else
        {
            //switch left and right up
            struct process *w = x->parent->left;
            if (w->color == RED)
            {
                w->color = BLACK;
                x->parent->color = RED;
                rightRotate(x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK)
            {
                w->color = RED;
                x = x->parent;
            }
            else
            {
                if (w->left->color == BLACK)
                {
                    w->right->color = BLACK;
                    w->color = RED;
                    leftRotate(w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rightRotate(x->parent);
                x = root;
            }
        }
    }
    x->color = BLACK;
}


//Deletes a process (node) from the given pid
//@pid = pid of the procedd (node) to be deleted
void rbDelete(uint32_t pid)
{
    // find node to delete
    struct process *z = rbSearch(pid);
    if (z == NIL)
    {
        return;
    }

    struct process *y = z;   //node to del or move
    struct process *x = NIL; //x is the child which will bereplacing y
    int yOriginalColor = y->color;

    //normal bst delete
    if (z->left == NIL)
    {
        //no left child
        x = z->right;
        rbTransplant(z, z->right);
    }
    else if (z->right == NIL)
    {
        //no right child
        x = z->left;
        rbTransplant(z, z->left);
    }
    else
    {
        //2 children ->  successor(most left node in right subtree)
        y = treeMinimum(z->right);
        yOriginalColor = y->color;
        x = y->right; //x will replace y

        if (y->parent == z)
        {
            x->parent = y;
        }
        else
        {
            //attach y->right to y->parent
            rbTransplant(y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }

        //z gets replaced by y
        rbTransplant(z, y);
        y->left = z->left;
        y->left->parent = y;

        //y which just replaced z has to take over zs color
        y->color = z->color;
    }

    //if y was a black node,fixuop
    if (yOriginalColor == BLACK)
    {
        rbDeleteFixup(x);
    }

    kfree(z->head_thread->kstack.stackStart);
    kfree(z->head_thread);
    kfree(z);
}

//Returns APIC-ID
uint32_t get_local_apic_id_cpuid() 
{
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile (
        "cpuid"
        : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
        : "a"(1)
    );
    //APIC-ID is in the bits 31:24 from ebx
    return (ebx >> 24) & 0xFF;
}


//Maps memory for a programm at the given address in the parameter "programAddress".
//The function then returns the amount of pages which the function had to map depending on the size of the program
uint32_t mapMemoryForProgramm(const char *filename, uint32_t programAddress)
{
    FRESULT res;
    FILINFO filestat;

    res = f_stat(filename, &filestat);
    if (res != FR_OK)
    {
        printf("Error looking for the stats from the file: %d\n", res);
        return 0;
    }

    uint32_t fileSize = filestat.fsize;
    uint32_t pagesNeeded = CEIL_DIV(fileSize, PAGE_SIZE);

    for (uint32_t i = 0; i < pagesNeeded + 256; i++)
    {
        void *vaddr = (void *)(programAddress + (i * PAGE_SIZE));
        mem_map_page((uint32_t)vaddr, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    }

    return pagesNeeded;
}

//Copies the program to the before mapped address.
//@filename = path to the program
//@pagedNeeded = amount of pages which mapMemoryForProgramm had to map for the program depending on the size of it
//@programAddress = desination address where the program should get copied to
void copyProgramToAddress(const char *filename, uint32_t pagesNeeded, uint32_t programAddress)
{
    FIL program;
    FRESULT res;
    BYTE buffer[PAGE_SIZE];
    UINT bytesRead;

    res = f_open(&program, filename, FA_READ);
    if (res != FR_OK)
    {
        printf("Error opening the file: %d\n", res);
        return;
    }

    //Copy the program into the mapped adressspace for it
    for (uint32_t i = 0; i < pagesNeeded; i++)
    {
        res = f_read(&program, buffer, PAGE_SIZE, &bytesRead);
        if (res != FR_OK)
        {
            printf("Error reading the file: %d\n", res);
            f_close(&program);
            return;
        }

        void *destAddress = (void *)(programAddress + (i * PAGE_SIZE));
        memcpy(destAddress, buffer, bytesRead);
    }

    res = f_close(&program);
    if (res != FR_OK)
    {
        printf("Error closing the file: %d\n", res);
    }
}

struct thread* createKernelThread(struct process* process, void *(function)());

//Creates a new kernel process with one thread.
//@function = function which should be executed in the first thread from this process
struct process* createKernelProcess(void *(function)())
{
    struct process* newProcess = (struct process*) kmalloc(sizeof(struct process));
    memset(newProcess, 0x00, sizeof(struct process));
    printf("Allocated process at: %p\n", newProcess);

    newProcess->isUserProc = 0;

    newProcess->pid = getProcessID(newProcess);


    newProcess->pageDirectory = kernel_directory;

    newProcess->tail_thread = NULL;
    newProcess->head_thread = NULL;

    struct thread* firstThread = createKernelThread(newProcess, function);

    acquire(&rbTreeLock);
    rbInsert(newProcess);
    release(&rbTreeLock);

    return newProcess;    
}

//Creates a thread for a kernel process
//@process = owner of the thread
//@function = function which should be executed in this thread
struct thread* createKernelThread(struct process* process, void *(function)())
{
    struct thread* newThread = (struct thread*) kmalloc(sizeof(struct thread));
    memset(newThread, 0x00, sizeof(struct thread));
    printf("Allocated Thread at: %p\n", newThread);

    newThread->threadID = getThreadID();
    newThread->owner = process;
    newThread->state = EMBRYO;    

    //Kernel stack
    newThread->kstack.ss0 = 0x10;
    uint8_t* kernelStack = (uint8_t*) kmalloc(4096);
    newThread->kstack.esp0 = (uint32_t) kernelStack + 4096 - 4;
    newThread->kstack.stackStart = (uint32_t) kernelStack;
 
    //Fill up registers of the thread
    newThread->regs = (struct regs*) kmalloc(sizeof(struct regs));
    uint32_t segmentSelector = 0x10;
    newThread->regs->ss = segmentSelector;
    newThread->regs->eflags = 0x202;
    newThread->regs->cs = 0x08;
    newThread->regs->eip = (uint32_t) function;
    newThread->regs->ds = segmentSelector;
    newThread->regs->es = segmentSelector;
    newThread->regs->fs = segmentSelector;
    newThread->regs->gs = segmentSelector;
    newThread->regs->esp = newThread->kstack.esp0;

    //Setting of the page directory (here it is the kernel directory and is already set), i have to set it anyways because else the cr3 entry is random and it gets loaded in the switchTask function, so it would crash
    newThread->regs->cr3 = newThread->owner->pageDirectory;

    //updateTssEsp0(newThread->kstack.esp0, 0);      //If i switch to a process directly (test or debugging reasons) i have to uncomment the tss update


    newThread->state = RUNNABLE;

    if (newThread->owner->head_thread == NULL)
    {
        newThread->owner->head_thread = newThread;
        newThread->owner->tail_thread = newThread;
    }else
    {
        newThread->prev = newThread->owner->tail_thread;
        newThread->next = NULL;
        newThread->owner->tail_thread->next = newThread;
        newThread->owner->tail_thread = newThread;
    }    

    return newThread;
}

//Creates a user process with one user thread
//@filename = path to the program
struct process* createUserProcess(const char* filename)
{
    struct process* newProcess = (struct process *) kmalloc(sizeof(struct process));
    memset(newProcess, 0x00, sizeof(struct process));
    kernel_write("Allocated process structure at: %p\n", newProcess);

    newProcess->pid = getProcessID(newProcess);

    newProcess->pageDirectory = mem_alloc_page_dir();
    mem_change_page_directory(newProcess->pageDirectory);

    uint32_t pagesNeeded = mapMemoryForProgramm(filename, 0x00400000);
    copyProgramToAddress(filename, pagesNeeded, 0x00400000);

    newProcess->tail_thread = NULL;
    newProcess->head_thread = NULL;

    struct thread* firstThread = createUserThread(newProcess);

    mem_change_page_directory(newProcess->pageDirectory);
    
    acquire(&rbTreeLock);
    rbInsert(newProcess);
    release(&rbTreeLock);

    return newProcess;
}


//Creates a thread for a user process
//@process = owner of the thread
struct thread* createUserThread(struct process* process)
{
    struct thread* newThread = (struct thread *) kmalloc(sizeof(struct thread));
    memset(newThread, 0x00, sizeof(struct thread));
    kernel_write("Allocated thread structure at: %p\n", newThread);

    newThread->threadID = getThreadID();
    newThread->owner = process;
    newThread->state = EMBRYO;


    //User stack
    uint32_t stackPages = 100;      //Defines the size of the stack (User stack), 100*4096 Bytes
    void* endOfStack = (void*)(USER_STACK_TOP - (PAGE_SIZE * stackPages));

    uint32_t physicalFrames[stackPages];        //Those are the Frames (4096 Bytes) in physical memory used for paging
    for (uint32_t i = 0; i < stackPages; i++)
    {
        void *vaddr = (void *)((uint32_t)endOfStack + (i * PAGE_SIZE));
        physicalFrames[i] = pmm_alloc_pageframe();
        mem_map_page((uint32_t)vaddr, physicalFrames[i], PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    }

    //User heap mapping
    for (size_t i = 0; i < 4096; i++)
    {
        void* virAddrHeap = 0x00800000 + (i * PAGE_SIZE);
        mem_map_page(virAddrHeap, pmm_alloc_pageframe(), PAGE_FLAG_PRESENT | PAGE_FLAG_WRITE | PAGE_FLAG_USER);
    }

    //Fill up registers of the thread
    newThread->regs = (struct regs*) kmalloc(sizeof(struct regs));
    uint32_t segmentSelector = 0x23;
    newThread->regs->ss = segmentSelector;
    newThread->regs->eflags = 0x202;
    newThread->regs->cs = 0x1B;
    newThread->regs->eip = PROGRAMM_VIRTUAL_ADDRESS_START;
    newThread->regs->ds = segmentSelector;
    newThread->regs->es = segmentSelector;
    newThread->regs->fs = segmentSelector;
    newThread->regs->gs = segmentSelector;
    uint32_t userStackPointer = USER_STACK_TOP - 4;
    newThread->regs->esp = userStackPointer;
    
    //Setting of the page directory which got allocated in the createUserProzess function
    newThread->regs->cr3 = newThread->owner->pageDirectory;

    //Kernel stack
    newThread->kstack.ss0 = 0x10;
    uint8_t* kernelStack = (uint8_t*) kmalloc(4096);
    newThread->kstack.esp0 = (uint32_t) kernelStack + 4096 - 4;
    newThread->kstack.stackStart = (uint32_t) kernelStack;

    //updateTssEsp0(newThread->kstack.esp0, 0);      //If i switch to a process directly (test or debugging reasons) i have to uncomment the tss update

    newThread->state = RUNNABLE;

    if (newThread->owner->head_thread == NULL)
    {
        newThread->owner->head_thread = newThread;
        newThread->owner->tail_thread = newThread;
    }else
    {
        newThread->prev = newThread->owner->tail_thread;
        newThread->next = NULL;
        newThread->owner->tail_thread->next = newThread;
        newThread->owner->tail_thread = newThread;
    }


    return newThread;
}


//Prepares Nodes for the RB-BST.
void nodePreparation()
{
    //Initialize NIL node
    NIL = (struct process *)kmalloc(sizeof(struct process));
    NIL->color = BLACK; //NIL has to be black
    NIL->left = NIL;
    NIL->right = NIL;
    NIL->parent = NULL;
    root = NIL;
}

//Idle process.
void idle()
{
    clear_screen_sys_2(COLOR_WHITE);
    while(1)
    {
        asm volatile ( "hlt");
    }
}

//Initializes idle.
uint32_t initIdle()
{
    struct process* idleProcess = createKernelProcess(idle);
    return idleProcess->pid;
}

//Initializes locks and prepares nodes.
void initProc()
{
    initLock(&rbTreeLock, "RB tree lock");
    nodePreparation();

    

}