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

#include "keyboard.h"
#include "../../kernel/arch/x86-32/interrupts/idt.h"
#include "../../kernel/arch/x86-32/io/io.h"
#include <stdbool.h>
#include "../video/vga/vga.h"
#include "../../kernel/libk/stdiok.h"

#define KEYBOARD_IRQ 1
#define KEYBOARD_PORT 0x60
#define KEYBOARD_ENABLE_PORT 0xAE


//special keys
#define UNKNOWN 0xFFFFFFE0
#define ESC     0xFFFFFFE1
#define CTRL    0xFFFFFFE2
#define LSHFT   0xFFFFFFE3
#define RSHFT   0xFFFFFFE4
#define ALT     0xFFFFFFE5
#define F1      0xFFFFFFE6
#define F2      0xFFFFFFE7
#define F3      0xFFFFFFE8
#define F4      0xFFFFFFE9
#define F5      0xFFFFFFEA
#define F6      0xFFFFFFEB
#define F7      0xFFFFFFEC
#define F8      0xFFFFFFED
#define F9      0xFFFFFFEE
#define F10     0xFFFFFFEF
#define F11     0xFFFFFFF0
#define F12     0xFFFFFFF1
#define SCRLCK  0xFFFFFFF2
#define HOME    0xFFFFFFF3
#define UP      0xFFFFFFF4
#define LEFT    0xFFFFFFF5
#define RIGHT   0xFFFFFFF6
#define DOWN    0xFFFFFFF7
#define PGUP    0xFFFFFFF8
#define PGDOWN  0xFFFFFFF9
#define END     0xFFFFFFFA
#define INS     0xFFFFFFFB
#define DEL     0xFFFFFFFC
#define CAPS    0xFFFFFFFD
#define NONE    0xFFFFFFFE
#define ALTGR   0xFFFFFFDF
#define NUMLCK  0xFFFFFFC0



bool capsOn;
bool capsLock;


const uint32_t lowercase[128] = {
    UNKNOWN, ESC, '1', '2', '3', '4', '5', '6', '7', '8',
    '9', '0', '\'', '^', '\b', '\t', 'q', 'w', 'e', 'r',
    't', 'z', 'u', 'i', 'o', 'p', 'ü', '*', '\n', CTRL,
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'ö',
    'ä', '$', LSHFT, '<', 'y', 'x', 'c', 'v', 'b', 'n', 'm', ',',
    '.', '-', RSHFT, '*', ALT, ' ', CAPS, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUMLCK, SCRLCK, HOME, UP, PGUP, '-', LEFT, UNKNOWN, RIGHT,
    '+', END, DOWN, PGDOWN, INS, DEL, UNKNOWN, UNKNOWN, UNKNOWN, F11, F12, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN
};

const uint32_t uppercase[128] = {
    UNKNOWN, ESC, '+', '"', '*', 'ç', '%', '&', '/', '(',
    ')', '=', '?', '`', '\b', '\t', 'Q', 'W', 'E', 'R',
    'T', 'Z', 'U', 'I', 'O', 'P', 'Ü', '!', '\n', CTRL,
    'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'Ö',
    'Ä', '£', LSHFT, '>', 'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';',
    ':', '_', RSHFT, '*', ALT, ' ', CAPS, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, NUMLCK, SCRLCK, HOME, UP, PGUP, '-', LEFT, UNKNOWN, RIGHT,
    '+', END, DOWN, PGDOWN, INS, DEL, UNKNOWN, UNKNOWN, UNKNOWN, F11, F12, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN,
    UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN, UNKNOWN
};


void handle_special_key(uint32_t scancode, bool pressed) {
    switch (scancode) {
        case 42:  // Shift
            capsOn = pressed;
            break;
        case 58:  // Caps Lock
            if (pressed) {
                capsLock = !capsLock;
            }
            break;
    }
}

bool is_special_key(uint32_t scancode) {
    switch (scancode) {
        case 1:
        case 29:
        case 56:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 87:
        case 88:
        case 42:
        case 58:
            return true;
        default:
            return false;
    }
}


void irq1_handler(struct Interrupt_registers *regs) {
    char scancode = insb(KEYBOARD_PORT) & 0x7F;  // Scan-Code without highest Bit
    char shiftpressed = insb(KEYBOARD_PORT) & 0x80;  // Status from highest Bits

    bool pressed = shiftpressed == 0;

    if (is_special_key(scancode)) {
        handle_special_key(scancode, pressed);
    } else {
        if (pressed) {
            if (capsOn || capsLock) {
                printf("%c", uppercase[scancode]);
            } else {
                printf("%c", lowercase[scancode]);
            }
        }
    }
}



void init_keyboard() {
    capsOn = false;
    capsLock = false;
    irq_add_handler(KEYBOARD_IRQ, &irq1_handler);
    outb(KEYBOARD_PORT, KEYBOARD_ENABLE_PORT);
}
