/******************************************************************************
 *
 * motor_demo.ld - Linker configuration file for motor_demo.
 *
 * Copyright (c) 2011-2013 Texas Instruments Incorporated.  All rights reserved.
 * Software License Agreement
 * 
 * Texas Instruments (TI) is supplying this software for use solely and
 * exclusively on TI's microcontroller products. The software is owned by
 * TI and/or its suppliers, and is protected under applicable copyright
 * laws. You may not combine this software with "viral" open-source
 * software in order to form a larger program.
 * 
 * THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
 * NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
 * NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
 * CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES, FOR ANY REASON WHATSOEVER.
 * 
 * This is part of revision 10636 of the Stellaris Firmware Development Package.
 *
 *****************************************************************************/
OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vector_table)
SECTIONS
{
        . = 0;
        ro_start = .;
        .vectors : AT (0)
        { *(.vectors) }

        .text : AT (LOADADDR(.vectors) + SIZEOF(.vectors))
        { *(.text.startup) *(.text) }

        .rodata : AT (LOADADDR (.text) + SIZEOF(.text))
        { *(.rodata*) }

        ro_end = ALIGN(4);

        . = 536870912;
        .data.bitband : AT (0 + (ro_end - ro_start))
        {
              *(.data.bitband)
        }

        /* Check that bitband variables don't overflow */
        dummy = ASSERT(. <= 536870912 + 1048576, "bitband variables too large");

        .data : AT (LOADADDR(.data.bitband) + SIZEOF(.data.bitband))
        {
              *(.data)
              . = ALIGN(4);
        }

        data_load_addr = LOADADDR(.data.bitband);
        data_virt_addr = ADDR(.data.bitband);
        data_size = . - ADDR(.data.bitband);

        /* Check addresses are correct */
        dummy = ASSERT(data_load_addr % 4 == 0, "data_load_addr not word aligned");
        dummy = ASSERT(data_virt_addr % 4 == 0, "data_virt_addr not word aligned");
        dummy = ASSERT(data_size % 4 == 0, "data_size not word aligned");

        .bss :
        {
             *(.bss)
             *(COMMON)
             . = ALIGN(4);
        }

        bss_virt_addr = ADDR(.bss);
        bss_size = SIZEOF(.bss);

        .stack : {
               . = . + 4096;
               stack = .;
        }

        /* Aliases for all the bitband variables */
        rtos_internal_interrupt_event_bitband = 570425344 + ((rtos_internal_interrupt_event - 536870912) * 32);

        /* Debug sections */

        .debug_aranges  0 : { *(.debug_aranges) }
        .debug_pubnames 0 : { *(.debug_pubnames) }

        .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
        .debug_abbrev   0 : { *(.debug_abbrev) }
        .debug_line     0 : { *(.debug_line) }
        .debug_frame    0 : { *(.debug_frame) }
        .debug_str      0 : { *(.debug_str) }
        .debug_loc      0 : { *(.debug_loc) }
        .debug_macinfo  0 : { *(.debug_macinfo) }

        .debug_pubtypes 0 : { *(.debug_pubtypes) }
        .debug_ranges   0 : { *(.debug_ranges) }

        .debug_macro    0 : { *(.debug_macro) }

        /DISCARD/ : { *(*) }
}
