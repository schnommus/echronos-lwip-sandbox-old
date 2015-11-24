/*
 * eChronos Real-Time Operating System
 * Copyright (C) 2015  National ICT Australia Limited (NICTA), ABN 62 102 206 173.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, version 3, provided that these additional
 * terms apply under section 7:
 *
 *   No right, title or interest in or to any trade mark, service mark, logo
 *   or trade name of of National ICT Australia Limited, ABN 62 102 206 173
 *   ("NICTA") or its licensors is granted. Modified versions of the Program
 *   must be plainly marked as such, and must not be distributed using
 *   "eChronos" as a trade mark or product name, or misrepresented as being
 *   the original Program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * @TAG(NICTA_AGPL)
 */

/*<module>
  <code_gen>template</code_gen>
  <schema>
   <entry name="delay_amount" type="int" default="10000"/>
  </schema>
</module>*/

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "drivers/io.h"

#define DELAY_AMOUNT {{delay_amount}}

static inline void
delay(void)
{
    volatile int i;
    for (i = 0; i < DELAY_AMOUNT; i++)
    {
    }
}

unsigned long g_ulTickCount = 0;

void
SysTickHandler(void)
{
    //
    // Increment the tick counter.
    //
    g_ulTickCount++;

    //
    // Every 100 ticks (1 second), toggle the LEDs
    //
    if((g_ulTickCount % 3) == 0)
    {
        LED_Toggle(BOTH_LEDS);
    }
}

int
main(void)
{
    //
    // Set the clocking to directly from the crystal
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
                       SYSCTL_XTAL_16MHZ);

    // Initialize the LED driver, then turn one LED on
    //
    LEDsInit();
    LED_On(LED_1);

    // Set up and enable the SysTick timer to use as a time reference.
    // It will be set up for a 10 ms tick.
    //
    ROM_SysTickPeriodSet(ROM_SysCtlClockGet() / 100);
    ROM_SysTickEnable();
    ROM_SysTickIntEnable();

    for(;;) { }
}
