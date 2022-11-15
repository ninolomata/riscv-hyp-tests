/** 
 * Bao, a Lightweight Static Partitioning Hypervisor 
 *
 * Copyright (c) Bao Project (www.bao-project.org), 2019-
 *
 * Authors:
 *      Jose Martins <jose.martins@bao-project.org>
 *
 * Bao is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License version 2 as published by the Free
 * Software Foundation, with a special exception exempting guest code from such
 * license. See the COPYING file in the top-level directory for details. 
 *
 */

#include "8250_uart.h"

int putchar(int ch)
{
    register char a0 asm("a0") = ch;
    asm volatile ("li t1, 0x11300" "\n\t"	//The base address of UART config registers
          "uart_status_simple: lb a1, 12(t1)" "\n\t"
          "andi a1,a1,0x2" "\n\t"
          "beqz a1, uart_status_simple" "\n\t"
	  			  "sb a0, 4(t1)"  "\n\t"
				  :
				  :
				  :"a0","t1","cc","memory");
    return 0;
}
