
/*--------------------------------------------------------------------*/
/*--- Machine-related stuff.                    pub_tool_machine.h ---*/
/*--------------------------------------------------------------------*/

/*
   This file is part of Valgrind, a dynamic binary instrumentation
   framework.

   Copyright (C) 2000-2005 Julian Seward
      jseward@acm.org

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307, USA.

   The GNU General Public License is contained in the file COPYING.
*/

#ifndef __PUB_TOOL_MACHINE_H
#define __PUB_TOOL_MACHINE_H

#if defined(VGA_x86)
#  define VG_MIN_INSTR_SZB          1  // min length of native instruction
#  define VG_MAX_INSTR_SZB         16  // max length of native instruction
#  define VG_CLREQ_SZB             18  // length of a client request, may
                                       //   be larger than VG_MAX_INSTR_SZB
#  if defined(VGO_linux)
#     define VG_STACK_REDZONE_SZB      0  // number of addressable bytes below SP
// It appears that NetBSD uses a redzone, even on x86.
// See /usr/src/sys/arch/i386/compile/STABLE/machine/proc.h
// It defines the redzone to be twice the page size
#  elif defined(VGO_netbsdelf2)
#     include "vki-x86-netbsd.h"
#     define VG_STACK_REDZONE_SZB      (VKI_PAGE_SIZE * 2)  // number of addressable bytes below SP
#  else
#    error Unknown OS (arch = x86)
#  endif
#elif defined(VGA_amd64)
#  define VG_MIN_INSTR_SZB          1
#  define VG_MAX_INSTR_SZB         16
#  define VG_CLREQ_SZB             18
#  define VG_STACK_REDZONE_SZB    128
#elif defined(VGA_ppc32)
#  define VG_MIN_INSTR_SZB          4
#  define VG_MAX_INSTR_SZB          4 
#  define VG_CLREQ_SZB             24
#  define VG_STACK_REDZONE_SZB      0
#else
#  error Unknown arch
#endif

// Guest state accessors
extern Addr VG_(get_SP) ( ThreadId tid );
extern Addr VG_(get_IP) ( ThreadId tid );
extern Addr VG_(get_FP) ( ThreadId tid );
extern Addr VG_(get_LR) ( ThreadId tid );

extern void VG_(set_SP) ( ThreadId tid, Addr sp );
extern void VG_(set_IP) ( ThreadId tid, Addr ip );

// For get/set, 'area' is where the asked-for shadow state will be copied
// into/from.
extern void VG_(get_shadow_regs_area) ( ThreadId tid, OffT guest_state_offset,
                                        SizeT size, UChar* area );
extern void VG_(set_shadow_regs_area) ( ThreadId tid, OffT guest_state_offset,
                                        SizeT size, const UChar* area );

// Apply a function 'f' to all the general purpose registers in all the
// current threads.
// This is very Memcheck-specific -- it's used to find the roots when
// doing leak checking.
extern void VG_(apply_to_GP_regs)(void (*f)(UWord val));

// This iterator lets you inspect each live thread's stack bounds.  The
// params are all 'out' params.  Returns False at the end.
extern void VG_(thread_stack_reset_iter) ( void );
extern Bool VG_(thread_stack_next)       ( ThreadId* tid, Addr* stack_min,
                                                          Addr* stack_max );

#endif   // __PUB_TOOL_MACHINE_H

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
