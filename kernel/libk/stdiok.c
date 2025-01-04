#include "stdiok.h"
#include "../../drivers/video/vbe/vbe.h"

#define FONT_WIDTH  8
#define FONT_HEIGHT 8
uint32_t cursor_x = 0;
uint32_t cursor_y = 0;
//static struct vbe_info* global_vbe_info;
extern struct vbe_info* globalvbeinfo;


void putc_kernel(char c) {
    if (globalvbeinfo == 0X00) {
        return;
    }

    if (c == '\n') {
        cursor_x = 0;
        cursor_y += FONT_HEIGHT;
        if (cursor_y >= globalvbeinfo->framebuffer_height) {
            cursor_y = 0; // line break (kind of scrolling lol)
        }
    } else if (c == '\r') {
        cursor_x = 0;

    } else if (c == '\t') {
        cursor_x += 4 * FONT_WIDTH;
        if (cursor_x >= globalvbeinfo->framebuffer_width) {
            cursor_x = 0;
            cursor_y += FONT_HEIGHT;
            if (cursor_y >= globalvbeinfo->framebuffer_height) {
                cursor_y = 0;
            }
        }
    } else if (c == '\b') {
        // Backspace
        if (cursor_x >= FONT_WIDTH) {
            cursor_x -= FONT_WIDTH; // move cursor back
        } else if (cursor_y >= FONT_HEIGHT) {
            cursor_y -= FONT_HEIGHT;
            cursor_x = globalvbeinfo->framebuffer_width - FONT_WIDTH;
        }
        // delete the font at the cursor position
        draw_rectangle(cursor_x, cursor_y, FONT_WIDTH, FONT_HEIGHT, COLOR_LIGHT_GREY, globalvbeinfo);
    } else {
        // draw font
        draw_char(cursor_x, cursor_y, c, COLOR_WHITE, globalvbeinfo);
        cursor_x += FONT_WIDTH;
        if (cursor_x >= globalvbeinfo->framebuffer_width) {
            cursor_x = 0;
            cursor_y += FONT_HEIGHT;
            if (cursor_y >= globalvbeinfo->framebuffer_height) {
                cursor_y = 0;
            }
        }
    }
}


void puts_kernel(const char* s) {
    while (*s) {
        putc_kernel(*s);
        s++;
    }
}

void kernel_write(const char* fmt, ...) {
    int* argp = (int*)&fmt;
    int state = PRINTF_STATE_START;
    int length = PRINTF_LENGTH_START;
    int radix = 10;
    bool sign = false;

    argp++;
    while (*fmt) {
        switch (state) {
            case PRINTF_STATE_START:
                if (*fmt == '%') {
                    state = PRINTF_STATE_LENGTH;
                } else {
                    putc_kernel(*fmt);
                }
                break;
            case PRINTF_STATE_LENGTH:
                if (*fmt == 'h') {
                    length = PRINTF_LENGTH_SHORT;
                    state = PRINTF_STATE_SHORT;
                } else if (*fmt == 'l') {
                    length = PRINTF_LENGTH_LONG;
                    state = PRINTF_STATE_LONG;
                } else {
                    goto PRINTF_STATE_SPEC_;
                }
                break;
            case PRINTF_STATE_SHORT:
                if (*fmt == 'h') {
                    length = PRINTF_LENGTH_SHORT_SHORT;
                    state = PRINTF_STATE_SPEC;
                } else {
                    goto PRINTF_STATE_SPEC_;
                }
                break;
            case PRINTF_STATE_LONG:
                if (*fmt == 'l') {
                    length = PRINTF_LENGTH_LONG_LONG;
                    state = PRINTF_STATE_SPEC;
                } else {
                    goto PRINTF_STATE_SPEC_;
                }
                break;
            case PRINTF_STATE_SPEC:
            PRINTF_STATE_SPEC_:
                switch (*fmt) {
                    case 'c':
                        putc_kernel((char)*argp);
                        argp++;
                        break;
                    case 's':
                        if (length == PRINTF_LENGTH_LONG || length == PRINTF_LENGTH_LONG_LONG) {
                            puts_kernel(*(const char**)argp);
                            argp += 2;
                        } else {
                            puts_kernel(*(const char**)argp);
                            argp++;
                        }
                        break;
                    case '%':
                        putc_kernel('%');
                        break;
                    case 'd':
                    case 'i':
                        radix = 10;
                        sign = true;
                        argp = printf_number_kernel(argp, length, sign, radix);
                        break;
                    case 'u':
                        radix = 10;
                        sign = false;
                        argp = printf_number_kernel(argp, length, sign, radix);
                        break;
                    case 'X':
                    case 'x':
                    case 'p':
                        radix = 16;
                        sign = false;
                        argp = printf_number_kernel(argp, length, sign, radix);
                        break;
                    case 'o':
                        radix = 8;
                        sign = false;
                        argp = printf_number_kernel(argp, length, sign, radix);
                        break;
                    default:
                        break;
                }
                state = PRINTF_STATE_START;
                length = PRINTF_LENGTH_START;
                radix = 10;
                sign = false;
                break;
        }
        fmt++;
    }
}

const char possibleChars[] = "0123456789abcdef";

int* printf_number_kernel(int* argp, int length, bool sign, int radix) {
    char buffer[32] = "";
    uint32_t number;
    int number_sign = 1;
    int pos = 0;

    switch (length) {
        case PRINTF_LENGTH_SHORT_SHORT:
        case PRINTF_LENGTH_SHORT:
        case PRINTF_LENGTH_START:
            if (sign) {
                int n = *argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (uint32_t)n;
            } else {
                number = *(uint32_t*)argp;
            }
            argp++;
            break;
        case PRINTF_LENGTH_LONG:
            if (sign) {
                long int n = *(long int*)argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (uint32_t)n;
            } else {
                number = *(uint32_t*)argp;
            }
            argp += 2;
            break;
        case PRINTF_LENGTH_LONG_LONG:
            if (sign) {
                long long int n = *(long long int*)argp;
                if (n < 0) {
                    n = -n;
                    number_sign = -1;
                }
                number = (uint32_t)n;
            } else {
                number = *(uint32_t*)argp;
            }
            argp += 4;
            break;
    }

    do {
        uint32_t rem = number % radix;
        number = number / radix;

        buffer[pos++] = possibleChars[rem];
    } while (number > 0);

    if (sign && number_sign < 0) {
        buffer[pos++] = '-';
    }

    while (--pos >= 0) {
        putc_kernel(buffer[pos]);
    }

    return argp;
}
