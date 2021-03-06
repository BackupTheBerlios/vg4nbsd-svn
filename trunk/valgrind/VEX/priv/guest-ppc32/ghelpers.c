
/*---------------------------------------------------------------*/
/*---                                                         ---*/
/*--- This file (guest-ppc32/ghelpers.c) is                   ---*/
/*--- Copyright (C) OpenWorks LLP.  All rights reserved.      ---*/
/*---                                                         ---*/
/*---------------------------------------------------------------*/

/*
   This file is part of LibVEX, a library for dynamic binary
   instrumentation and translation.

   Copyright (C) 2004-2005 OpenWorks LLP.  All rights reserved.

   This library is made available under a dual licensing scheme.

   If you link LibVEX against other code all of which is itself
   licensed under the GNU General Public License, version 2 dated June
   1991 ("GPL v2"), then you may use LibVEX under the terms of the GPL
   v2, as appearing in the file LICENSE.GPL.  If the file LICENSE.GPL
   is missing, you can obtain a copy of the GPL v2 from the Free
   Software Foundation Inc., 51 Franklin St, Fifth Floor, Boston, MA
   02110-1301, USA.

   For any other uses of LibVEX, you must first obtain a commercial
   license from OpenWorks LLP.  Please contact info@open-works.co.uk
   for information about commercial licensing.

   This software is provided by OpenWorks LLP "as is" and any express
   or implied warranties, including, but not limited to, the implied
   warranties of merchantability and fitness for a particular purpose
   are disclaimed.  In no event shall OpenWorks LLP be liable for any
   direct, indirect, incidental, special, exemplary, or consequential
   damages (including, but not limited to, procurement of substitute
   goods or services; loss of use, data, or profits; or business
   interruption) however caused and on any theory of liability,
   whether in contract, strict liability, or tort (including
   negligence or otherwise) arising in any way out of the use of this
   software, even if advised of the possibility of such damage.

   Neither the names of the U.S. Department of Energy nor the
   University of California nor the names of its contributors may be
   used to endorse or promote products derived from this software
   without prior written permission.
*/

#include "libvex_basictypes.h"
#include "libvex_emwarn.h"
#include "libvex_guest_ppc32.h"
#include "libvex_ir.h"
#include "libvex.h"

#include "main/vex_util.h"
#include "guest-generic/bb_to_IR.h"
#include "guest-ppc32/gdefs.h"


/* This file contains helper functions for ppc32 guest code.
   Calls to these functions are generated by the back end.
   These calls are of course in the host machine code and 
   this file will be compiled to host machine code, so that
   all makes sense.  

   Only change the signatures of these helper functions very
   carefully.  If you change the signature here, you'll have to change
   the parameters passed to it in the IR calls constructed by
   guest-ppc32/toIR.c.
*/


/*---------------------------------------------------------------*/
/*--- Misc integer helpers.                                   ---*/
/*---------------------------------------------------------------*/

/* CALLED FROM GENERATED CODE */
/* DIRTY HELPER (non-referentially-transparent) */
/* Horrible hack.  On non-ppc32 platforms, return 1. */
/* Reads a complete, consistent 64-bit TB value. */
ULong ppc32g_dirtyhelper_MFTB ( void )
{
#  if defined(__powerpc__)
   ULong res;
   UInt  lo, hi1, hi2;
   while (1) {
      __asm__ __volatile__ ("\n"
         "\tmftbu %0\n"
         "\tmftb %1\n"
         "\tmftbu %2\n"
         : "=r" (hi1), "=r" (lo), "=r" (hi2)
      );
      if (hi1 == hi2) break;
   }
   res = ((ULong)hi1) << 32;
   res |= (ULong)lo;
   return res;
#  else
   return 1ULL;
#  endif
}


/* CALLED FROM GENERATED CODE */
/* DIRTY HELPER (reads guest state, writes guest mem) */
void ppc32g_dirtyhelper_LVS ( VexGuestPPC32State* gst,
                              UInt vD_off, UInt sh, UInt shift_right )
{
  static
  UChar ref[32] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
                    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                    0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F };
  U128* pU128_src;
  U128* pU128_dst;

  vassert( vD_off       <= sizeof(VexGuestPPC32State)-8 );
  vassert( sh           <= 15 );
  vassert( shift_right  <=  1 );
  if (shift_right)
     sh = 16-sh;
  /* else shift left  */

  pU128_src = (U128*)&ref[sh];
  pU128_dst = (U128*)( ((UChar*)gst) + vD_off );

  (*pU128_dst)[0] = (*pU128_src)[0];
  (*pU128_dst)[1] = (*pU128_src)[1];
  (*pU128_dst)[2] = (*pU128_src)[2];
  (*pU128_dst)[3] = (*pU128_src)[3];
}


/* Helper-function specialiser. */

IRExpr* guest_ppc32_spechelper ( HChar* function_name,
                                 IRExpr** args )
{
   return NULL;
}


/*----------------------------------------------*/
/*--- The exported fns ..                    ---*/
/*----------------------------------------------*/

/* VISIBLE TO LIBVEX CLIENT */
UInt LibVEX_GuestPPC32_get_CR ( /*IN*/VexGuestPPC32State* vex_state )
{
#  define FIELD(_n)                                    \
      ( ( (UInt)                                       \
           ( (vex_state->guest_CR##_n##_321 & (7<<1))  \
             | (vex_state->guest_CR##_n##_0 & 1)       \
           )                                           \
        )                                              \
        << (4 * (7-(_n)))                              \
      )

   return 
      FIELD(0) | FIELD(1) | FIELD(2) | FIELD(3)
      | FIELD(4) | FIELD(5) | FIELD(6) | FIELD(7);

#  undef FIELD
}


/* VISIBLE TO LIBVEX CLIENT */
void LibVEX_GuestPPC32_put_CR ( UInt cr_native,
                                /*OUT*/VexGuestPPC32State* vex_state )
{
   UInt t;

#  define FIELD(_n)                                           \
      do {                                                    \
         t = cr_native >> (4*(7-(_n)));                       \
         vex_state->guest_CR##_n##_0 = toUChar(t & 1);        \
         vex_state->guest_CR##_n##_321 = toUChar(t & (7<<1)); \
      } while (0)

   FIELD(0);
   FIELD(1);
   FIELD(2);
   FIELD(3);
   FIELD(4);
   FIELD(5);
   FIELD(6);
   FIELD(7);

#  undef FIELD
}


/* VISIBLE TO LIBVEX CLIENT */
UInt LibVEX_GuestPPC32_get_XER ( /*IN*/VexGuestPPC32State* vex_state )
{
   UInt w = 0;
   w |= ( ((UInt)vex_state->guest_XER_BC) & 0xFF );
   w |= ( (((UInt)vex_state->guest_XER_SO) & 0x1) << 31 );
   w |= ( (((UInt)vex_state->guest_XER_OV) & 0x1) << 30 );
   w |= ( (((UInt)vex_state->guest_XER_CA) & 0x1) << 29 );
   return w;
}


/* VISIBLE TO LIBVEX CLIENT */
void LibVEX_GuestPPC32_put_XER ( UInt xer_native,
                                 /*OUT*/VexGuestPPC32State* vex_state )
{
   vex_state->guest_XER_BC = toUChar(xer_native & 0xFF);
   vex_state->guest_XER_SO = toUChar((xer_native >> 31) & 0x1);
   vex_state->guest_XER_OV = toUChar((xer_native >> 30) & 0x1);
   vex_state->guest_XER_CA = toUChar((xer_native >> 29) & 0x1);
}


/* VISIBLE TO LIBVEX CLIENT */
void LibVEX_GuestPPC32_initialise ( /*OUT*/VexGuestPPC32State* vex_state )
{
   vex_state->guest_GPR0  = 0;
   vex_state->guest_GPR1  = 0;
   vex_state->guest_GPR2  = 0;
   vex_state->guest_GPR3  = 0;
   vex_state->guest_GPR4  = 0;
   vex_state->guest_GPR5  = 0;
   vex_state->guest_GPR6  = 0;
   vex_state->guest_GPR7  = 0;
   vex_state->guest_GPR8  = 0;
   vex_state->guest_GPR9  = 0;
   vex_state->guest_GPR10 = 0;
   vex_state->guest_GPR11 = 0;
   vex_state->guest_GPR12 = 0;
   vex_state->guest_GPR13 = 0;
   vex_state->guest_GPR14 = 0;
   vex_state->guest_GPR15 = 0;
   vex_state->guest_GPR16 = 0;
   vex_state->guest_GPR17 = 0;
   vex_state->guest_GPR18 = 0;
   vex_state->guest_GPR19 = 0;
   vex_state->guest_GPR20 = 0;
   vex_state->guest_GPR21 = 0;
   vex_state->guest_GPR22 = 0;
   vex_state->guest_GPR23 = 0;
   vex_state->guest_GPR24 = 0;
   vex_state->guest_GPR25 = 0;
   vex_state->guest_GPR26 = 0;
   vex_state->guest_GPR27 = 0;
   vex_state->guest_GPR28 = 0;
   vex_state->guest_GPR29 = 0;
   vex_state->guest_GPR30 = 0;
   vex_state->guest_GPR31 = 0;

   vex_state->guest_FPR0  = 0;
   vex_state->guest_FPR1  = 0;
   vex_state->guest_FPR2  = 0;
   vex_state->guest_FPR3  = 0;
   vex_state->guest_FPR4  = 0;
   vex_state->guest_FPR5  = 0;
   vex_state->guest_FPR6  = 0;
   vex_state->guest_FPR7  = 0;
   vex_state->guest_FPR8  = 0;
   vex_state->guest_FPR9  = 0;
   vex_state->guest_FPR10 = 0;
   vex_state->guest_FPR11 = 0;
   vex_state->guest_FPR12 = 0;
   vex_state->guest_FPR13 = 0;
   vex_state->guest_FPR14 = 0;
   vex_state->guest_FPR15 = 0;
   vex_state->guest_FPR16 = 0;
   vex_state->guest_FPR17 = 0;
   vex_state->guest_FPR18 = 0;
   vex_state->guest_FPR19 = 0;
   vex_state->guest_FPR20 = 0;
   vex_state->guest_FPR21 = 0;
   vex_state->guest_FPR22 = 0;
   vex_state->guest_FPR23 = 0;
   vex_state->guest_FPR24 = 0;
   vex_state->guest_FPR25 = 0;
   vex_state->guest_FPR26 = 0;
   vex_state->guest_FPR27 = 0;
   vex_state->guest_FPR28 = 0;
   vex_state->guest_FPR29 = 0;
   vex_state->guest_FPR30 = 0;
   vex_state->guest_FPR31 = 0;

   /* Initialise the vector state. */
#  define VECZERO(_vr) _vr[0]=_vr[1]=_vr[2]=_vr[3] = 0;

   VECZERO(vex_state->guest_VR0 );
   VECZERO(vex_state->guest_VR1 );
   VECZERO(vex_state->guest_VR2 );
   VECZERO(vex_state->guest_VR3 );
   VECZERO(vex_state->guest_VR4 );
   VECZERO(vex_state->guest_VR5 );
   VECZERO(vex_state->guest_VR6 );
   VECZERO(vex_state->guest_VR7 );
   VECZERO(vex_state->guest_VR8 );
   VECZERO(vex_state->guest_VR9 );
   VECZERO(vex_state->guest_VR10);
   VECZERO(vex_state->guest_VR11);
   VECZERO(vex_state->guest_VR12);
   VECZERO(vex_state->guest_VR13);
   VECZERO(vex_state->guest_VR14);
   VECZERO(vex_state->guest_VR15);
   VECZERO(vex_state->guest_VR16);
   VECZERO(vex_state->guest_VR17);
   VECZERO(vex_state->guest_VR18);
   VECZERO(vex_state->guest_VR19);
   VECZERO(vex_state->guest_VR20);
   VECZERO(vex_state->guest_VR21);
   VECZERO(vex_state->guest_VR22);
   VECZERO(vex_state->guest_VR23);
   VECZERO(vex_state->guest_VR24);
   VECZERO(vex_state->guest_VR25);
   VECZERO(vex_state->guest_VR26);
   VECZERO(vex_state->guest_VR27);
   VECZERO(vex_state->guest_VR28);
   VECZERO(vex_state->guest_VR29);
   VECZERO(vex_state->guest_VR30);
   VECZERO(vex_state->guest_VR31);

#  undef VECZERO

   vex_state->guest_CIA  = 0;
   vex_state->guest_LR   = 0;
   vex_state->guest_CTR  = 0;

   vex_state->guest_XER_SO = 0;
   vex_state->guest_XER_OV = 0;
   vex_state->guest_XER_CA = 0;
   vex_state->guest_XER_BC = 0;

   vex_state->guest_CR0_321 = 0;
   vex_state->guest_CR0_0   = 0;
   vex_state->guest_CR1_321 = 0;
   vex_state->guest_CR1_0   = 0;
   vex_state->guest_CR2_321 = 0;
   vex_state->guest_CR2_0   = 0;
   vex_state->guest_CR3_321 = 0;
   vex_state->guest_CR3_0   = 0;
   vex_state->guest_CR4_321 = 0;
   vex_state->guest_CR4_0   = 0;
   vex_state->guest_CR5_321 = 0;
   vex_state->guest_CR5_0   = 0;
   vex_state->guest_CR6_321 = 0;
   vex_state->guest_CR6_0   = 0;
   vex_state->guest_CR7_321 = 0;
   vex_state->guest_CR7_0   = 0;

   vex_state->guest_FPROUND = (UInt)PPC32rm_NEAREST;

   vex_state->guest_VRSAVE = 0;

   vex_state->guest_VSCR = 0x00010000;  // Non-Java mode

   vex_state->guest_EMWARN = EmWarn_NONE;

   vex_state->guest_TISTART = 0;
   vex_state->guest_TILEN   = 0;

   vex_state->guest_RESVN   = 0;
}


/*-----------------------------------------------------------*/
/*--- Describing the ppc32 guest state, for the benefit   ---*/
/*--- of iropt and instrumenters.                         ---*/
/*-----------------------------------------------------------*/

/* Figure out if any part of the guest state contained in minoff
   .. maxoff requires precise memory exceptions.  If in doubt return
   True (but this is generates significantly slower code).  

   By default we enforce precise exns for guest R1 (stack pointer),
   CIA (current insn address) and LR (link register).  These are the
   minimum needed to extract correct stack backtraces from ppc32
   code. [[NB: not sure if keeping LR up to date is actually
   necessary.]]
*/
Bool guest_ppc32_state_requires_precise_mem_exns ( Int minoff, 
                                                   Int maxoff )
{
   Int lr_min  = offsetof(VexGuestPPC32State, guest_LR);
   Int lr_max  = lr_min + 4 - 1;
   Int r1_min  = offsetof(VexGuestPPC32State, guest_GPR1);
   Int r1_max  = r1_min + 4 - 1;
   Int cia_min = offsetof(VexGuestPPC32State, guest_CIA);
   Int cia_max = cia_min + 4 - 1;

   if (maxoff < lr_min || minoff > lr_max) {
      /* no overlap with LR */
   } else {
      return True;
   }

   if (maxoff < r1_min || minoff > r1_max) {
      /* no overlap with R1 */
   } else {
      return True;
   }

   if (maxoff < cia_min || minoff > cia_max) {
      /* no overlap with CIA */
   } else {
      return True;
   }

   return False;
}


#define ALWAYSDEFD(field)                             \
    { offsetof(VexGuestPPC32State, field),            \
      (sizeof ((VexGuestPPC32State*)0)->field) }

VexGuestLayout
   ppc32Guest_layout 
      = { 
          /* Total size of the guest state, in bytes. */
          .total_sizeB = sizeof(VexGuestPPC32State),

          /* Describe the stack pointer. */
          .offset_SP = offsetof(VexGuestPPC32State,guest_GPR1),
          .sizeof_SP = 4,

          /* Describe the instruction pointer. */
          .offset_IP = offsetof(VexGuestPPC32State,guest_CIA),
          .sizeof_IP = 4,

          /* Describe any sections to be regarded by Memcheck as
             'always-defined'. */
          .n_alwaysDefd = 7,

          .alwaysDefd 
	  = { /*  0 */ ALWAYSDEFD(guest_CIA),
	      /*  1 */ ALWAYSDEFD(guest_EMWARN),
	      /*  2 */ ALWAYSDEFD(guest_TISTART),
	      /*  3 */ ALWAYSDEFD(guest_TILEN),
	      /*  4 */ ALWAYSDEFD(guest_VSCR),
	      /*  5 */ ALWAYSDEFD(guest_FPROUND),
	      /*  6 */ ALWAYSDEFD(guest_RESVN)
            }
        };


/*---------------------------------------------------------------*/
/*--- end                              guest-ppc32/ghelpers.c ---*/
/*---------------------------------------------------------------*/
