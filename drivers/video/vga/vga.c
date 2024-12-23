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

    //loops threw all the line & columns and clears them with the for loop height_screen & width
    for (uint16_t y = 0; y < height_screen; y++)
    {
        for (uint16_t x = 0; x < width; x++)
        {
            vga_address[y * width + x] = ' ' | defaultColor;    //Every character has a character and a color
        }
        
    }
        
}

void newLine()
{
    if (line < height_screen - 1)
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
    for (uint16_t y = 0; y < height_screen; y++)
    {
        for (uint16_t x = 0; x < width; x++)
        {
            vga_address[(y-1) * width + x] = vga_address[y * width + x];
        }
    }
    for (uint16_t x = 0; x < width; x++)
    {
        vga_address[(height_screen-1) * width + x] = ' ' | currColor;
    }
    
}

void handleBackspace() {
    if (column > 0) {
        column--;
        vga_address[line * width + column] = ' ' | currColor;
    } else if (line > 0) {
        line--;
        column = width - 1;
        vga_address[line * width + column] = ' ' | currColor;
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
                if (column == width) {
                    newLine();
                }
                uint16_t tablen = 4 - (column % 4);
                while (tablen != 0) {
                    vga_address[line * width + (column++)] = ' ' | currColor;
                    tablen--;
                }
                break;
            case '\b':
                handleBackspace();
                break;
            default:
                if (column == width) {
                    newLine();
                }
                vga_address[line * width + (column++)] = *s | currColor;
                break;
        }
        s++;
    }
}
