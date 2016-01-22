#include "interrupt.h"
#include "interrupt_handler.h"

#include <memory.h>
#include <stdio.h>

#ifndef QEMU
#define IVT_OFFSET (unsigned int) 0xFFFF0000
#endif

#ifdef QEMU
#define IVT_OFFSET (unsigned int) 0x0
#endif

void init_interrupt_handling(void) {
  //build interrupt vector table
  *(unsigned int*) (IVT_OFFSET + 0x00) = 0xe59ff018;	//TODO: reset
  *(unsigned int*) (IVT_OFFSET + 0x04) = 0xe59ff014;	//ldr pc, [pc, #20] ; 0x20 undefined instruction
  *(unsigned int*) (IVT_OFFSET + 0x08) = 0xe59ff014;	//ldr pc, [pc, #20] ; 0x24 software interrupt
  *(unsigned int*) (IVT_OFFSET + 0x0c) = 0xe59ff014;	//ldr pc, [pc, #20] ; 0x28 prefetch abort
  *(unsigned int*) (IVT_OFFSET + 0x10) = 0xe59ff014;	//ldr pc, [pc, #20] ; 0x2c data abort
  *(unsigned int*) (IVT_OFFSET + 0x14) = 0xe59ff014;	//ldr pc, [pc, #20] ; 0x30 reserved
  *(unsigned int*) (IVT_OFFSET + 0x18) = 0xe59ff014;	//ldr pc, [pc, #20] ; 0x34 IRQ
  *(unsigned int*) (IVT_OFFSET + 0x1c) = 0xe59ff014;	//ldr pc, [pc, #20] ; 0x38 FIQ

  *(unsigned int*) (IVT_OFFSET + 0x20) = (unsigned int) 0;
  //ATTENTION: don't use software interrupts in supervisor mode
  *(unsigned int*) (IVT_OFFSET + 0x24) = (unsigned int) &irq_handler;
  *(unsigned int*) (IVT_OFFSET + 0x28) = (unsigned int) 0;
  *(unsigned int*) (IVT_OFFSET + 0x2c) = (unsigned int) 0;
  *(unsigned int*) (IVT_OFFSET + 0x30) = (unsigned int) 0;
  *(unsigned int*) (IVT_OFFSET + 0x34) = (unsigned int) &irq_handler;
  *(unsigned int*) (IVT_OFFSET + 0x38) = (unsigned int) 0;

  //setting up IRQ mode stack
  asm(
    "mrs  r0, cpsr\n"
    "bic  r0, #0x1f\n" // Clear mode bits
    "orr  r0, #0x12\n" // Select IRQ mode
    "msr  cpsr, r0\n"  // Enter IRQ mode
    "mov  sp, %0\n"    // set stack pointer
    "bic  r0, #0x1f\n" // Clear mode bits
    "orr  r0, #0x13\n" // Select SVC mode
    "msr  cpsr, r0\n"  // Enter SVC mode
    : : "r" (IRQ_STACK_ADDRESS)
  );

  // Enable IRQs
  asm(
    "mrs  r1, cpsr\n"
    "bic  r1, r1, #0x80\n"
    "msr  cpsr_c, r1\n"
  );

#ifndef QEMU
  // set V bit in c1 register in cp15 to
  // locate interrupt vector table to 0xFFFF0000
  asm volatile (
    "MRC p15, 0, r0, c1, c0, 0\n"
    "ORR r0, #0x2000\n"
    "MCR p15, 0, r0, c1, c0, 0\n"
  );
#endif

#ifdef QEMU
  // Setting up primary interrupt controller
  *PIC_INTENABLE |= TIMER1_INTBIT;	// unmask interrupt bit for timer1
#endif
  return;
}
