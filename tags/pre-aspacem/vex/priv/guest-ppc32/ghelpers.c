
/*---------------------------------------------------------------*/
/*---                                                         ---*/
/*--- This file (guest-ppc32/ghelpers.c) is                   ---*/
/*--- Copyright (c) 2004 OpenWorks LLP.  All rights reserved. ---*/
/*---                                                         ---*/
/*---------------------------------------------------------------*/

/*
   This file is part of LibVEX, a library for dynamic binary
   instrumentation and translation.

   Copyright (C) 2004 OpenWorks, LLP.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; Version 2 dated June 1991 of the
   license.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, or liability
   for damages.  See the GNU General Public License for more details.

   Neither the names of the U.S. Department of Energy nor the
   University of California nor the names of its contributors may be
   used to endorse or promote products derived from this software
   without prior written permission.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
   USA.
*/

#include "libvex_basictypes.h"
#include "libvex_guest_ppc32.h"
#include "libvex_ir.h"
#include "libvex.h"

#include "main/vex_util.h"
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


#define INT32_MIN               (-2147483647-1)



/* CALLED FROM GENERATED CODE: CLEAN HELPER */
/* Calculates CR7[LT,GT,EQ,SO] flags from the supplied
   thunk parameters.
   Returns values in high field (correct wrt actual CR)
 */
UInt ppc32g_calculate_cr7_all ( UInt op, UInt val, UInt xer_so )
{
   if (op) {   // val contains cr7 flags to be returned
      return (val & 0xF0000000);
   } else {    // val contains result to be tested
      Int sval = (Int)val;
      xer_so = xer_so & 1;
      return ( ((xer_so == 1) ? (1<<28) : 0)
               | ((sval == 0) ? (1<<29) : 0)
               | ((sval >  0) ? (1<<30) : 0)
               | ((sval <  0) ? (1<<31) : 0) );
   }
}


// Calculate XER_OV
UInt ppc32g_calculate_xer_ov ( UInt op, UInt res, UInt argL, UInt argR )
{
   switch (op) {
   case PPC32G_FLAG_OP_ADD:     // addo, addc
   case PPC32G_FLAG_OP_ADDE:    // addeo, addmeo, addzeo
      return ((argL^argR^-1) & (argL^res) & (1<<31)) ? 1:0;
      // i.e. ((both_same_sign) & (sign_changed) & (sign_mask))
      
   case PPC32G_FLAG_OP_DIVW:    // divwo
      return ((argL == INT32_MIN && argR == -1) || argR == 0) ? 1:0;
      
   case PPC32G_FLAG_OP_DIVWU:   // divwuo
      return (argR == 0) ? 1:0;
      
   case PPC32G_FLAG_OP_MULLW: { // mullwo
      /* OV true if result can't be represented in 32 bits
         i.e sHi != sign extension of sLo */
      Long l_res = (Long)((Int)argL) * (Long)((Int)argR);
      Int sHi = (Int)toUInt(l_res >> 32);
      Int sLo = (Int)l_res;
      return (sHi != (sLo >> /*s*/ 31)) ? 1:0;
   }

   case PPC32G_FLAG_OP_NEG:     // nego
      return (argL == 0x80000000) ? 1:0;
      
   case PPC32G_FLAG_OP_SUBF:    // subfo
   case PPC32G_FLAG_OP_SUBFC:   // subfco
   case PPC32G_FLAG_OP_SUBFE:   // subfeo, subfmeo, subfzeo
      return (((~argL)^argR^(-1)) & ((~argL)^res) & (1<<31)) ? 1:0;

   default:
      break;
   }

   vpanic("ppc32g_calculate_xer_ov(ppc32)");
   return 0; // notreached
}

// Calculate XER_CA
UInt ppc32g_calculate_xer_ca ( UInt op, UInt res,
                               UInt argL, UInt argR, UInt ca )
{
  switch (op) {
   case PPC32G_FLAG_OP_ADD:     // addc[o], addic
      return (res < argL) ? 1:0;

   case PPC32G_FLAG_OP_ADDE:    // adde[o], addze[o], addme[o]
      return (res < argL || (ca==1 && res==argL)) ? 1:0;

   case PPC32G_FLAG_OP_SUBFC:   // subfc[o]
   case PPC32G_FLAG_OP_SUBFI:   // subfic
      return (res <= argR) ? 1:0;

   case PPC32G_FLAG_OP_SUBFE:   // subfe[o], subfze[o], subfme[o]
      return ((res < argR) || (ca == 1 && res == argR)) ? 1:0;

   case PPC32G_FLAG_OP_SRAW:    // sraw
      if ((argR & 0x20) == 0) {  // shift <= 31
         // xer_ca = sign && (bits_shifted_out != 0)
         return (((argL & 0x80000000) &&
                  ((argL & (0xFFFFFFFF >> (32-argR))) != 0)) != 0) ? 1:0;
      }
      // shift > 31
      // xer_ca = sign && src != 0
      return (((argL & 0x80000000) && (argR != 0)) != 0) ? 1:0;

   case PPC32G_FLAG_OP_SRAWI:   // srawi
      // xer_ca = sign && (bits_shifted_out != 0)
      return (((argL & 0x80000000) &&
               ((argL & (0xFFFFFFFF >> (32-argR))) != 0)) != 0) ? 1:0;

   default:
      break;
   }
   vpanic("ppc32g_calculate_xer_ov(ppc32)");
   return 0; // notreached
}







/* Used by the optimiser to try specialisations.  Returns an
   equivalent expression, or NULL if none. */

#if 0
/* temporarily unused */
static Bool isU32 ( IRExpr* e, UInt n )
{
   return (e->tag == Iex_Const
           && e->Iex.Const.con->tag == Ico_U32
           && e->Iex.Const.con->Ico.U32 == n);
}
#endif
IRExpr* guest_ppc32_spechelper ( HChar* function_name,
                                 IRExpr** args )
{
   return NULL;
}


/*----------------------------------------------*/
/*--- The exported fns ..                    ---*/
/*----------------------------------------------*/

/* VISIBLE TO LIBVEX CLIENT */
#if 0
void LibVEX_GuestPPC32_put_flags ( UInt flags_native,
                                 /*OUT*/VexGuestPPC32State* vex_state )
{
   vassert(0); // FIXME
}
#endif

/* VISIBLE TO LIBVEX CLIENT */
UInt LibVEX_GuestPPC32_get_flags ( /*IN*/VexGuestPPC32State* vex_state )
{
   UInt flags = ppc32g_calculate_cr7_all(
      vex_state->guest_CC_OP,
      vex_state->guest_CC_DEP1,
      vex_state->guest_CC_DEP2
      );
   return flags;
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

   vex_state->guest_CIA  = 0;
   vex_state->guest_LR   = 0;
   vex_state->guest_CTR  = 0;

   vex_state->guest_CC_OP   = 0;
   vex_state->guest_CC_DEP1 = 0;
   vex_state->guest_CC_DEP2 = 0;

   vex_state->guest_CR0to6 = 0;

   vex_state->guest_XER = 0;

   vex_state->guest_EMWARN = 0;
   vex_state->guest_TISTART = 0;
   vex_state->guest_TILEN   = 0;
}


/*-----------------------------------------------------------*/
/*--- Describing the ppc32 guest state, for the benefit     ---*/
/*--- of iropt and instrumenters.                         ---*/
/*-----------------------------------------------------------*/

/* Figure out if any part of the guest state contained in minoff
   .. maxoff requires precise memory exceptions.  If in doubt return
   True (but this is generates significantly slower code).  
*/
Bool guest_ppc32_state_requires_precise_mem_exns ( Int minoff, 
                                                 Int maxoff)
{
   return True; // FIXME (also comment above)
}



#define ALWAYSDEFD(field)                           \
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
          .n_alwaysDefd = 1,
          /* flags thunk: only using last_result, which is always defd. */

          .alwaysDefd 
             = { /*  0 */ ALWAYSDEFD(guest_CC_OP)
               }
        };


/*---------------------------------------------------------------*/
/*--- end                              guest-ppc32/ghelpers.c ---*/
/*---------------------------------------------------------------*/
