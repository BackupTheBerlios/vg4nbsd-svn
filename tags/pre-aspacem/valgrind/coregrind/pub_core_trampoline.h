
/*--------------------------------------------------------------------*/
/*--- The trampoline code page.              pub_core_trampoline.h ---*/
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

#ifndef __PUB_CORE_TRAMPOLINE_H
#define __PUB_CORE_TRAMPOLINE_H

//--------------------------------------------------------------------
// PURPOSE: This module defines our trampoline code page, which we copy
// over the client's, for arcane signal return and syscall purposes...
//--------------------------------------------------------------------

// Platform-specifics aren't neatly factored out here, since some of the
// constants are not used on all platforms.  But it's non-obvious how
// to do it better.

extern const Char VG_(trampoline_code_start);      // x86 + amd64
extern const Int  VG_(trampoline_code_length);     // x86 + amd64

extern const Int  VG_(tramp_sigreturn_offset);     // x86
extern const Int  VG_(tramp_rt_sigreturn_offset);  // x86 + amd64
extern const Int  VG_(tramp_syscall_offset);       // x86
extern const Int  VG_(tramp_gettimeofday_offset);  // amd64
extern const Int  VG_(tramp_time_offset);          // amd64
 
#endif   // __PUB_CORE_TRAMPOLINE_H

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
