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

#include "vga.h"

uint16_t column = 0;
uint16_t line = 0;
uint16_t* const vga_address = (uint16_t* const) 0xB8000;
const uint16_t defaultColor = (COLOR8_BLACK << 8) | (COLOR8_LIGHT_GREY << 12);
uint16_t currColor = defaultColor;

void reset()
{
    line = 0;
    column = 0;
    currColor = defaultColor;

    //loops threw all the line & columns and clears them with the for loop heightvga & widthvga
    for (uint16_t y = 0; y < heightvga; y++)
    {
        for (uint16_t x = 0; x < widthvga; x++)
        {
            vga_address[y * widthvga + x] = ' ' | defaultColor;    //Every character has a character and a color
        }
        
    }
        
}

void newLine()
{
    if (line < heightvga - 1)
    {
        line++;
        column = 0;
    }else
    {
        scrollUp();
        column = 0;
    }
    
}

void scrollUp()
{
    for (uint16_t y = 0; y < heightvga; y++)
    {
        for (uint16_t x = 0; x < widthvga; x++)
        {
            vga_address[(y-1) * widthvga + x] = vga_address[y * widthvga + x];
        }
    }
    for (uint16_t x = 0; x < widthvga; x++)
    {
        vga_address[(heightvga-1) * widthvga + x] = ' ' | currColor;
    }
    
}

void handleBackspace() {
    if (column > 0) {
        column--;
        vga_address[line * widthvga + column] = ' ' | currColor;
    } else if (line > 0) {
        line--;
        column = widthvga - 1;
        vga_address[line * widthvga + column] = ' ' | currColor;
    }
}

void print(const char* s) {
    while (*s) {
        switch (*s) {
            case '\n':
                newLine();
                break;
            case '\r':
                column = 0;
                break;
            case '\t':
                if (column == widthvga) {
                    newLine();
                }
                uint16_t tablen = 4 - (column % 4);
                while (tablen != 0) {
                    vga_address[line * widthvga + (column++)] = ' ' | currColor;
                    tablen--;
                }
                break;
            case '\b':
                handleBackspace();
                break;
            default:
                if (column == widthvga) {
                    newLine();
                }
                vga_address[line * widthvga + (column++)] = *s | currColor;
                break;
        }
        s++;
    }
}
