
/*--------------------------------------------------------------------*/
/*--- Doing system calls.                       pub_core_syscall.h ---*/
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

#ifndef __PUB_CORE_SYSCALL_H
#define __PUB_CORE_SYSCALL_H

//--------------------------------------------------------------------
// PURPOSE: This module contains the code for actually executing syscalls.
//--------------------------------------------------------------------

/* Do a syscall on this platform, with 6 args, and return the result
   in canonical format in a SysRes value. */

// We use a full prototype for VG_(do_syscall) rather than "..." to ensure
// that all arguments get converted to a UWord appropriately.  Not doing so
// can cause problems when passing 32-bit integers on 64-bit platforms,
// because the top 32-bits might not be zeroed appropriately, eg. as would
// happen with the 6th arg on AMD64 which is passed on the stack.


/* Macros make life easier. */
#ifndef VGP_x86_netbsdelf2
extern SysRes VG_(do_syscall) ( UWord sysno, 
                                UWord, UWord, UWord, 
                                UWord, UWord, UWord );

#define vgPlain_do_syscall0(s)             VG_(do_syscall)((s),0,0,0,0,0,0)
#define vgPlain_do_syscall1(s,a)           VG_(do_syscall)((s),(a),0,0,0,0,0)
#define vgPlain_do_syscall2(s,a,b)         VG_(do_syscall)((s),(a),(b),0,0,0,0)
#define vgPlain_do_syscall3(s,a,b,c)       VG_(do_syscall)((s),(a),(b),(c),0,0,0)
#define vgPlain_do_syscall4(s,a,b,c,d)     VG_(do_syscall)((s),(a),(b),\
                                                           (c),(d),0,0)
#define vgPlain_do_syscall5(s,a,b,c,d,e)   VG_(do_syscall)((s),(a),(b),\
                                                           (c),(d),(e),0)
#define vgPlain_do_syscall6(s,a,b,c,d,e,f) VG_(do_syscall)((s),(a),(b),\
                                                           (c),(d),(e),(f))

#else
extern  SysRes VG_(do_syscall) ( UWord sysno, UWord a1, UWord a2, UWord a3, 
				 UWord a4, UWord a5, UWord a6,ULong a7 );

#define vgPlain_do_syscall0(s)             VG_(do_syscall)((s),0,0,0,0,0,0,0)
#define vgPlain_do_syscall1(s,a)           VG_(do_syscall)((s),(a),0,0,0,0,0,0)
#define vgPlain_do_syscall2(s,a,b)         VG_(do_syscall)((s),(a),(b),0,0,0,0,0)
#define vgPlain_do_syscall3(s,a,b,c)       VG_(do_syscall)((s),(a),(b),(c),0,0,0,0)
#define vgPlain_do_syscall4(s,a,b,c,d)     VG_(do_syscall)((s),(a),(b),\
                                                           (c),(d),0,0,0)
#define vgPlain_do_syscall5(s,a,b,c,d,e)   VG_(do_syscall)((s),(a),(b),\
                                                           (c),(d),(e),0,0)
#define vgPlain_do_syscall6(s,a,b,c,d,e,f) VG_(do_syscall)((s),(a),(b),\
                                                           (c),(d),(e),(f),0)

#define vgPlain_do_syscall7(s,a,b,c,d,e,f,g) VG_(do_syscall)((s),(a),(b),\
                                                           (c),(d),(e),(f),(g))
#endif
/* This whole cludge is required because our Mmap has  7 arguments! - kailash*/
extern SysRes VG_(mk_SysRes_x86_linux)      ( Word val );
extern SysRes VG_(mk_SysRes_x86_netbsdelf2) ( Word val );
extern SysRes VG_(mk_SysRes_amd64_linux)    ( Word val );
extern SysRes VG_(mk_SysRes_ppc32_linux)    ( UInt val, UInt errflag );
extern SysRes VG_(mk_SysRes_Error)          ( UWord val );
extern SysRes VG_(mk_SysRes_Success)        ( UWord val );


/* Return a string which gives the name of an error value.  Note,
   unlike the standard C syserror fn, the returned string is not
   malloc-allocated or writable -- treat it as a constant. */

extern const HChar* VG_(strerror) ( UWord errnum );


#endif   // __PUB_CORE_SYSCALL_H

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
