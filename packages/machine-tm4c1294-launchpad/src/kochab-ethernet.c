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

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "drivers/pinout.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/interrupt.h"
#include "driverlib/emac.h"
#include "utils/locator.h"
#include "utils/lwiplib.h"
#include "utils/uartstdio.h"
#include "utils/ustdlib.h"

#include "rtos-kochab.h"

#define debug_println(text) UARTprintf(text);UARTprintf("\n")

#define SYSTICK_INT_PRIORITY    0x80
#define ETHERNET_INT_PRIORITY   0xC0

#ifdef DEBUG
void __error__( char *pcFilename, uint32_t ui32Line ) {
    for(;;);
}
#endif

void nmi() { for(;;); }
void hardfault() { for(;;); }
void memmanage() { for(;;); }
void busfault() { for(;;); }
void usagefault() { for(;;); }


uint32_t g_ui32SysClock;
uint32_t g_ui32IPAddress;

void
ConfigureUART(void)
{
    //
    // Enable the GPIO Peripheral used by the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    //
    // Enable UART0
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    //
    // Configure GPIO Pins for UART mode.
    //
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    //
    // Initialize the UART for console I/O.
    //
    UARTStdioConfig(0, 115200, g_ui32SysClock);
}

bool tick_irq(void) {
    rtos_timer_tick();
    return true;
}


void
fatal(const RtosErrorId error_id)
{
    UARTprintf("eChronos: Internal OS error: %i\n", error_id);
    for (;;)
    {
    }
}

static char* aMessage = "This is a message!";

/*
void
fn_a(void)
{
    uint8_t count;

    UARTprintf( "Task A: sleeping for 10000 ticks" );
    rtos_sleep( 10000 );

    debug_println("task a: taking lock");
    rtos_mutex_lock(RTOS_MUTEX_ID_M0);
    if (rtos_mutex_try_lock(RTOS_MUTEX_ID_M0))
    {
        debug_println("unexpected mutex not locked.");
    }
    debug_println("task a: releasing lock");
    rtos_mutex_unlock(RTOS_MUTEX_ID_M0);

    for (count = 0; count < 10; count++)
    {
        debug_println("task a");
        if (count % 5 == 0)
        {
            debug_println("unblocking b");
            rtos_signal_send_set(RTOS_TASK_ID_B, RTOS_SIGNAL_SET_TEST);
        }
    }


    debug_println("Task A: Posting a message...");

    rtos_message_queue_put( RTOS_MESSAGE_QUEUE_ID_TEST_QUEUE, &aMessage );

    debug_println("A now waiting for ticks");
    for (;;)
    {

        (void) rtos_signal_wait_set(RTOS_SIGNAL_SET_TIMER);
        debug_println("tick");
        rtos_signal_send_set(RTOS_TASK_ID_B, RTOS_SIGNAL_SET_TEST);
    }
}

void
fn_b(void)
{
    UARTprintf( "Task B: sleeping for 10000 ticks" );
    rtos_sleep( 10000 );
    
    debug_println("task b: attempting lock");
    rtos_mutex_lock(RTOS_MUTEX_ID_M0);
    debug_println("task b: got lock");

    while (1) {
        if (rtos_signal_poll_set(RTOS_SIGNAL_SET_TEST)) {
            debug_println("task b blocking");
            (void) rtos_signal_wait_set(RTOS_SIGNAL_SET_TEST);
            debug_println("task b unblocked");
        }


        void *theMessage;
        if( rtos_message_queue_try_get( RTOS_MESSAGE_QUEUE_ID_TEST_QUEUE, &theMessage ) ) {
            UARTprintf("Task B: Got a message! It's: %s\n", theMessage);
        }
    }
}
*/

void
DisplayIPAddress(uint32_t ui32Addr)
{
    char pcBuf[16];

    // Convert the IP Address into a string.
    usprintf(pcBuf, "%d.%d.%d.%d", ui32Addr & 0xff, (ui32Addr >> 8) & 0xff,
            (ui32Addr >> 16) & 0xff, (ui32Addr >> 24) & 0xff);

    // Display the string.
    UARTprintf(pcBuf);
}

void
lwIPHostTimerHandler(void)
{
    uint32_t ui32NewIPAddress;

    // Get the current IP address.
    ui32NewIPAddress = lwIPLocalIPAddrGet();


    // See if the IP address has changed.
    if(ui32NewIPAddress != g_ui32IPAddress)
    {
        // See if there is an IP address assigned.
        if(ui32NewIPAddress == 0xffffffff)
        {
            // Indicate that there is no link.
            UARTprintf("Waiting for link.\n");
        }
        else if(ui32NewIPAddress == 0)
        {
            // There is no IP address, so indicate that the DHCP process is
            // running.
            UARTprintf("Waiting for IP address.\n");
        }
        else
        {
            // Display the new IP address.
            UARTprintf("IP Address: ");
            DisplayIPAddress(ui32NewIPAddress);
            UARTprintf("\n");
            UARTprintf("Open a browser and enter the IP address.\n");
        }

        // Save the new IP address.
        g_ui32IPAddress = ui32NewIPAddress;
    }

    // If there is not an IP address.
    if((ui32NewIPAddress == 0) || (ui32NewIPAddress == 0xffffffff))
    {
       // Do nothing and keep waiting.
    }
}

int
main(void)
{
    uint32_t ui32User0, ui32User1;
    uint8_t pui8MACArray[8];

    //Needed by the PHY
    SysCtlMOSCConfigSet(SYSCTL_MOSC_HIGHFREQ);

    g_ui32SysClock = MAP_SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                SYSCTL_OSC_MAIN | SYSCTL_USE_PLL |
                SYSCTL_CFG_VCO_480), 120000000);

    PinoutSet(true, false);

    MAP_SysTickPeriodSet(g_ui32SysClock / (1000/SYS_TICK_INTERVAL) );
    MAP_SysTickEnable();
    MAP_SysTickIntEnable();

    ConfigureUART();

    UARTprintf( "Starting eChronos HTTP demo\n" );

    // Configure the hardware MAC address for Ethernet Controller filtering of
    // incoming packets.  The MAC address will be stored in the non-volatile
    // USER0 and USER1 registers.
    MAP_FlashUserGet(&ui32User0, &ui32User1);
    if((ui32User0 == 0xffffffff) || (ui32User1 == 0xffffffff))
    {
        // Let the user know there is no MAC address
        UARTprintf("No MAC programmed!\n");

        while(1)
        {
        }
    }

    // Tell the user what we are doing just now.
    UARTprintf("Waiting for IP.\n");

    // Convert the 24/24 split MAC address from NV ram into a 32/16 split
    // MAC address needed to program the hardware registers, then program
    // the MAC address into the Ethernet Controller registers.
    pui8MACArray[0] = ((ui32User0 >>  0) & 0xff);
    pui8MACArray[1] = ((ui32User0 >>  8) & 0xff);
    pui8MACArray[2] = ((ui32User0 >> 16) & 0xff);
    pui8MACArray[3] = ((ui32User1 >>  0) & 0xff);
    pui8MACArray[4] = ((ui32User1 >>  8) & 0xff);
    pui8MACArray[5] = ((ui32User1 >> 16) & 0xff);

    // Initialze the lwIP library, using DHCP.
    lwIPInit(g_ui32SysClock, pui8MACArray, 0, 0, 0, IPADDR_USE_DHCP);

    // Setup the device locator service.
    LocatorInit();
    LocatorMACAddrSet(pui8MACArray);
    LocatorAppTitleSet("EK-TM4C1294XL enet_io");

    // Initialize a sample httpd server.
    httpd_init();

    // Set the interrupt priorities.  We set the SysTick interrupt to a higher
    // priority than the Ethernet interrupt to ensure that the file system
    // tick is processed if SysTick occurs while the Ethernet handler is being
    // processed.  This is very likely since all the TCP/IP and HTTP work is
    // done in the context of the Ethernet interrupt.
    MAP_IntPrioritySet(INT_EMAC0, ETHERNET_INT_PRIORITY);
    MAP_IntPrioritySet(FAULT_SYSTICK, SYSTICK_INT_PRIORITY);


    rtos_start();

    /* Should never reach here, but if we do, an infinite loop is
       easier to debug than returning somewhere random. */
    for (;;) ;
}
