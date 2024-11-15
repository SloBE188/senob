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

#include "thread.h"
#include "../mm/heap/heap.h"
#include "../mm/paging/paging.h"
#include "../mm/PMM/pmm.h"
#include "../../../libk/memory.h"
#include "../interrupts/idt.h"
#include "../gdt/gdt.h"
#include "../fatfs/ff.h"

