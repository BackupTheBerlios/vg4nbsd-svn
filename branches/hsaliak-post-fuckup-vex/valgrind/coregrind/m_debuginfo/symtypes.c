
/*--------------------------------------------------------------------*/
/*--- Extract type info from debug info.                symtypes.h ---*/
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

#include "pub_core_basics.h"
#include "pub_core_debuginfo.h"
#include "pub_core_debuglog.h"    // For VG_(debugLog_vprintf)
#include "pub_core_libcbase.h"
#include "pub_core_libcassert.h"
#include "pub_core_libcprint.h"
#include "pub_core_libcsignal.h"
#include "pub_core_machine.h"
#include "pub_core_mallocfree.h"

#include "priv_symtypes.h"

typedef enum {
   TyUnknown,			/* unknown type */
   TyUnresolved,		/* unresolved type */
   TyError,			/* error type */

   TyVoid,			/* void */

   TyInt,			/* integer */
   TyBool,			/* boolean */
   TyChar,			/* character */
   TyFloat,			/* float */

   TyRange,			/* type subrange */

   TyEnum,			/* enum */

   TyPointer,			/* pointer */
   TyArray,			/* array */
   TyStruct,			/* structure/class */
   TyUnion,			/* union */

   TyTypedef			/* typedef */
} TyKind;

static const Char *ppkind(TyKind k)
{
   switch(k) {
#define S(x)	case x:		return #x
      S(TyUnknown);
      S(TyUnresolved);
      S(TyError);
      S(TyVoid);
      S(TyInt);
      S(TyBool);
      S(TyChar);
      S(TyRange);
      S(TyFloat);
      S(TyEnum);
      S(TyPointer);
      S(TyArray);
      S(TyStruct);
      S(TyUnion);
      S(TyTypedef);
#undef S
   default:
      return "Ty???";
   }
}

/* struct/union field */
typedef struct _StField {
   UInt		offset;		/* offset into structure (0 for union) (in bits) */
   UInt		size;		/* size (in bits) */
   SymType	*type;		/* type of element */
   Char	*name;			/* name of element */
} StField;

/* enum tag */
typedef struct _EnTag {
   const Char   *name;		/* name */
   UInt		val;		/* value */
} EnTag;

struct _SymType {
   TyKind	kind;			/* type descriminator */
   UInt		size;			/* sizeof(type) */
   Char		*name;			/* useful name */

   union {
      /* TyInt,TyBool,TyChar */
      struct {
	 Bool		issigned;	/* signed or not */
      } t_scalar;

      /* TyFloat */
      struct {
	 Bool		isdouble;	/* is double prec */
      } t_float;

      /* TyRange */
      struct {
	 Int		min;
	 Int		max;
	 SymType	*type;
      } t_range;

      /* TyPointer */
      struct {
	 SymType	*type;		/* *type */
      } t_pointer;

      /* TyArray */
      struct {
	 SymType	*idxtype;
	 SymType	*type;
      } t_array;

      /* TyEnum */
      struct {
	 UInt		ntag;		/* number of tags */
	 EnTag		*tags;		/* tags */
      } t_enum;

      /* TyStruct, TyUnion */
      struct {
	 UInt		nfield;		/* number of fields */
	 UInt		nfieldalloc;	/* number of fields allocated */
	 StField	*fields;	/* fields */
      } t_struct;

      /* TyTypedef */
      struct {
	 SymType	*type;		/* type */
      } t_typedef;

      /* TyUnresolved - reference to unresolved type */
      struct {
	 /* some kind of symtab reference */
	 SymResolver	*resolver;	/* symtab reader's resolver */
	 void		*data;		/* data for resolver */
      } t_unresolved;
   } u;
};


Bool ML_(st_isstruct)(SymType *ty)
{
   return ty->kind == TyStruct;
}

Bool ML_(st_isunion)(SymType *ty)
{
   return ty->kind == TyUnion;
}

Bool ML_(st_isenum)(SymType *ty)
{
   return ty->kind == TyEnum;
}

static inline SymType *alloc(SymType *st)
{
   if (st == NULL) {
      st = VG_(arena_malloc)(VG_AR_SYMTAB, sizeof(*st));
      st->kind = TyUnknown;
      st->name = NULL;
   }

   return st;
}

static void resolve(SymType *st)
{
   if (st->kind != TyUnresolved)
      return;

   (*st->u.t_unresolved.resolver)(st, st->u.t_unresolved.data);

   if (st->kind == TyUnresolved)
      st->kind = TyError;
}

SymType *ML_(st_mkunresolved)(SymType *st, SymResolver *resolver, void *data)
{
   st = alloc(st);
   
   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown);

   st->kind = TyUnresolved;
   st->size = 0;
   st->u.t_unresolved.resolver = resolver;
   st->u.t_unresolved.data = data;

   return st;
}

void ML_(st_unresolved_setdata)(SymType *st, SymResolver *resolver, void *data)
{
   if (st->kind != TyUnresolved)
      return;

   st->u.t_unresolved.resolver = resolver;
   st->u.t_unresolved.data = data;
}

Bool ML_(st_isresolved)(SymType *st)
{
   return st->kind != TyUnresolved;
}

void ML_(st_setname)(SymType *st, Char *name)
{
   if (st->name != NULL)
      st->name = name;
}

SymType *ML_(st_mkvoid)(SymType *st)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown);
   
   st->kind = TyVoid;
   st->size = 1;		/* for address calculations */
   st->name = "void";
   return st;
}

SymType *ML_(st_mkint)(SymType *st, UInt size, Bool isSigned)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown);
   
   st->kind = TyInt;
   st->size = size;
   st->u.t_scalar.issigned = isSigned;

   return st;
}

SymType *ML_(st_mkfloat)(SymType *st, UInt size)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown);
   
   st->kind = TyFloat;
   st->size = size;
   st->u.t_scalar.issigned = True;

   return st;
}

SymType *ML_(st_mkbool)(SymType *st, UInt size)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown);
   
   st->kind = TyBool;
   st->size = size;

   return st;
}


SymType *ML_(st_mkpointer)(SymType *st, SymType *ptr)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown);

   st->kind = TyPointer;
   st->size = sizeof(void *);
   st->u.t_pointer.type = ptr;

   return st;
}

SymType *ML_(st_mkrange)(SymType *st, SymType *ty, Int min, Int max)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown);

   st->kind = TyRange;
   st->size = 0;		/* ? */
   st->u.t_range.type = ty;
   st->u.t_range.min = min;
   st->u.t_range.max = max;

   return st;
}

SymType *ML_(st_mkstruct)(SymType *st, UInt size, UInt nfields)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown || st->kind == TyStruct);

   vg_assert(st->kind != TyStruct || st->u.t_struct.nfield == 0);

   st->kind = TyStruct;
   st->size = size;
   st->u.t_struct.nfield = 0;
   st->u.t_struct.nfieldalloc = nfields;
   if (nfields != 0)
      st->u.t_struct.fields = VG_(arena_malloc)(VG_AR_SYMTAB, sizeof(StField) * nfields);
   else
      st->u.t_struct.fields = NULL;
   
   return st;
}

SymType *ML_(st_mkunion)(SymType *st, UInt size, UInt nfields)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown || st->kind == TyUnion);

   vg_assert(st->kind != TyUnion || st->u.t_struct.nfield == 0);

   st->kind = TyUnion;
   st->size = size;
   st->u.t_struct.nfield = 0;
   st->u.t_struct.nfieldalloc = nfields;
   if (nfields != 0)
      st->u.t_struct.fields = VG_(arena_malloc)(VG_AR_SYMTAB, sizeof(StField) * nfields);
   else
      st->u.t_struct.fields = NULL;

   return st;
}

void ML_(st_addfield)(SymType *st, Char *name, SymType *type, UInt off, UInt size)
{
   StField *f;

   vg_assert(st->kind == TyStruct || st->kind == TyUnion);

   if (st->u.t_struct.nfieldalloc == st->u.t_struct.nfield) {
      StField *n = VG_(arena_malloc)(VG_AR_SYMTAB, 
				     sizeof(StField) * (st->u.t_struct.nfieldalloc + 2));
      VG_(memcpy)(n, st->u.t_struct.fields, sizeof(*n) * st->u.t_struct.nfield);
      if (st->u.t_struct.fields != NULL)
	 VG_(arena_free)(VG_AR_SYMTAB, st->u.t_struct.fields);
      st->u.t_struct.nfieldalloc++;
      st->u.t_struct.fields = n;
   }

   f = &st->u.t_struct.fields[st->u.t_struct.nfield++];
   f->name = name;
   f->type = type;
   f->offset = off;
   f->size = size;
}


SymType *ML_(st_mkenum)(SymType *st, UInt ntags)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown || st->kind == TyEnum);

   st->kind = TyEnum;
   st->u.t_enum.ntag = 0;
   st->u.t_enum.tags = NULL;
   
   return st;
}

SymType *ML_(st_mkarray)(SymType *st, SymType *idxtype, SymType *type)
{
   st = alloc(st);

   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown);

   st->kind = TyArray;
   st->u.t_array.type = type;
   st->u.t_array.idxtype = idxtype;
   
   return st;
}

SymType *ML_(st_mktypedef)(SymType *st, Char *name, SymType *type)
{
   st = alloc(st);

   vg_assert(st != type);
   vg_assert(st->kind == TyUnresolved || st->kind == TyUnknown ||
	     st->kind == TyStruct || st->kind == TyUnion ||
	     st->kind == TyTypedef);
   
   st->kind = TyTypedef;
   st->name = name;
   st->u.t_typedef.type = type;

   return st;
}


SymType *ML_(st_basetype)(SymType *type, Bool do_resolve)
{
   while (type->kind == TyTypedef || (do_resolve && type->kind == TyUnresolved)) {
      if (do_resolve)
	 resolve(type);

      if (type->kind == TyTypedef)
	 type = type->u.t_typedef.type;
   }

   return type;
}

UInt ML_(st_sizeof)(SymType *ty)
{
   return ty->size;
}

#ifndef TEST
/*
  Hash of visited addresses, so we don't get stuck in loops.  It isn't
  simply enough to keep track of addresses, since we need to interpret
  the memory according to the type.  If a given location has multiple
  pointers with different types (for example, void * and struct foo *),
  then we need to look at it under each type.
*/
struct visited {
   Addr		a;
   SymType	*ty;
   struct visited *next;
};

#define VISIT_HASHSZ	1021

static struct visited *visit_hash[VISIT_HASHSZ];

static inline Bool test_visited(Addr a, SymType *type)
{
   struct visited *v;
   UInt b = (UInt)a % VISIT_HASHSZ;
   Bool ret = False;

   for(v = visit_hash[b]; v != NULL; v = v->next) {
      if (v->a == a && v->ty == type) {
	 ret = True;
	 break;
      }
   }
   
   return ret;
}

static Bool has_visited(Addr a, SymType *type)
{
   static const Bool debug = False;
   Bool ret;

   ret = test_visited(a, type);

   if (!ret) {
      UInt b = (UInt)a % VISIT_HASHSZ;
      struct visited * v = VG_(arena_malloc)(VG_AR_SYMTAB, sizeof(*v));

      v->a = a;
      v->ty = type;
      v->next = visit_hash[b];
      visit_hash[b] = v;
   }
   
   if (debug)
      VG_(printf)("has_visited(a=%p, ty=%p) -> %d\n", a, type, ret);

   return ret;
}

static void clear_visited(void)
{
   UInt i;
   
   for(i = 0; i < VISIT_HASHSZ; i++) {
      struct visited *v, *n;
      for(v = visit_hash[i]; v != NULL; v = n) {
	 n = v->next;
	 VG_(arena_free)(VG_AR_SYMTAB, v);
      }
      visit_hash[i] = NULL;
   }
}

static 
void bprintf(void (*send)(HChar, void*), void *send_arg, const Char *fmt, ...)
{
   va_list vargs;

   va_start(vargs, fmt);
   VG_(debugLog_vprintf)(send, send_arg, fmt, vargs);
   va_end(vargs);
}

#define SHADOWCHUNK	0	/* no longer have a core allocator */

#if SHADOWCHUNK
static ShadowChunk *findchunk(Addr a)
{
   Bool find(ShadowChunk *sc) {
      return a >= sc->data && a < (sc->data+sc->size);
   }
   return VG_(any_matching_mallocd_ShadowChunks)(find);
}
#endif

static struct vki_sigaction sigbus_saved;
static struct vki_sigaction sigsegv_saved;
static vki_sigset_t  blockmask_saved;
static jmp_buf valid_addr_jmpbuf;

static void valid_addr_handler(int sig)
{
   //VG_(printf)("OUCH! %d\n", sig);
   __builtin_longjmp(valid_addr_jmpbuf, 1);
}

/* catch badness signals because we're going to be
   playing around in untrusted memory */
static void setup_signals(void)
{
   Int res;
   struct vki_sigaction sigbus_new;
   struct vki_sigaction sigsegv_new;
   vki_sigset_t         unblockmask_new;

   /* Temporarily install a new sigsegv and sigbus handler, and make
      sure SIGBUS, SIGSEGV and SIGTERM are unblocked.  (Perhaps the
      first two can never be blocked anyway?)  */

   sigbus_new.ksa_handler = valid_addr_handler;
   sigbus_new.sa_flags = VKI_SA_ONSTACK | VKI_SA_RESTART;
   sigbus_new.sa_restorer = NULL;
   res = VG_(sigemptyset)( &sigbus_new.sa_mask );
   vg_assert(res == 0);

   sigsegv_new.ksa_handler = valid_addr_handler;
   sigsegv_new.sa_flags = VKI_SA_ONSTACK | VKI_SA_RESTART;
   sigsegv_new.sa_restorer = NULL;
   res = VG_(sigemptyset)( &sigsegv_new.sa_mask );
   vg_assert(res == 0+0);

   res =  VG_(sigemptyset)( &unblockmask_new );
   res |= VG_(sigaddset)( &unblockmask_new, VKI_SIGBUS );
   res |= VG_(sigaddset)( &unblockmask_new, VKI_SIGSEGV );
   res |= VG_(sigaddset)( &unblockmask_new, VKI_SIGTERM );
   vg_assert(res == 0+0+0);

   res = VG_(sigaction)( VKI_SIGBUS, &sigbus_new, &sigbus_saved );
   vg_assert(res == 0+0+0+0);

   res = VG_(sigaction)( VKI_SIGSEGV, &sigsegv_new, &sigsegv_saved );
   vg_assert(res == 0+0+0+0+0);

   res = VG_(sigprocmask)( VKI_SIG_UNBLOCK, &unblockmask_new, &blockmask_saved );
   vg_assert(res == 0+0+0+0+0+0);
}

static void restore_signals(void)
{
   Int res;

   /* Restore signal state to whatever it was before. */
   res = VG_(sigaction)( VKI_SIGBUS, &sigbus_saved, NULL );
   vg_assert(res == 0 +0);

   res = VG_(sigaction)( VKI_SIGSEGV, &sigsegv_saved, NULL );
   vg_assert(res == 0 +0 +0);

   res = VG_(sigprocmask)( VKI_SIG_SETMASK, &blockmask_saved, NULL );
   vg_assert(res == 0 +0 +0 +0);
}

/* if false, setup and restore signals for every access */
#define LAZYSIG		1

static Bool is_valid_addr(Addr a)
{
   static SymType faulted = { TyError };
   static const Bool debug = False;
   volatile Bool ret = False;

   if ((a > VKI_PAGE_SIZE) && !test_visited(a, &faulted)) {
      if (!LAZYSIG)
	 setup_signals();
   
      if (__builtin_setjmp(valid_addr_jmpbuf) == 0) {
	 volatile UInt *volatile ptr = (volatile UInt *)a;

	 *ptr;

	 ret = True;
      } else {
	 /* cache bad addresses in visited table */
	 has_visited(a, &faulted);
	 ret = False;
      }

      if (!LAZYSIG)
	 restore_signals();
   }

   if (debug)
      VG_(printf)("is_valid_addr(%p) -> %d\n", a, ret);

   return ret;
}

static Int free_varlist(Variable *list)
{
   Variable *next;
   Int count = 0;

   for(; list != NULL; list = next) {
      next = list->next;
      count++;
      if (list->name)
	 VG_(arena_free)(VG_AR_SYMTAB, list->name);
      VG_(arena_free)(VG_AR_SYMTAB, list);
   }
   return count;
}

/* Composite: struct, union, array
   Non-composite: everything else
 */
static inline Bool is_composite(SymType *ty)
{
   switch(ty->kind) {
   case TyUnion:
   case TyStruct:
   case TyArray:
      return True;

   default:
      return False;
   }
}

/* There's something at the end of the rainbow */
static inline Bool is_followable(SymType *ty)
{
   return ty->kind == TyPointer || is_composite(ty);
}

/* Result buffer */
static Char *describe_addr_buf;
static UInt describe_addr_bufidx;
static UInt describe_addr_bufsz;

/* Add a character to the result buffer */
static void describe_addr_addbuf(HChar c,void *p) {
   if ((describe_addr_bufidx+1) >= describe_addr_bufsz) {
      Char *n;
    
      if (describe_addr_bufsz == 0)
         describe_addr_bufsz = 8;
      else
         describe_addr_bufsz *= 2;
    
      /* use tool malloc so that the tool can free it */
      n = VG_(malloc)(describe_addr_bufsz);
      if (describe_addr_buf != NULL && describe_addr_bufidx != 0)
         VG_(memcpy)(n, describe_addr_buf, describe_addr_bufidx);
      if (describe_addr_buf != NULL)
         VG_(free)(describe_addr_buf);
      describe_addr_buf = n;
   }
   describe_addr_buf[describe_addr_bufidx++] = c;
   describe_addr_buf[describe_addr_bufidx] = '\0';
}

#define MAX_PLY		7	/* max depth we go */
#define MAX_ELEMENTS	5000	/* max number of array elements we scan */
#define MAX_VARS	10000	/* max number of variables total traversed */

Char *VG_(describe_addr)(ThreadId tid, Addr addr)
{
   static const Bool debug = False;
   static const Bool memaccount = False; /* match creates to frees */
   Addr eip;			/* thread's EIP */
   Variable *list;		/* worklist */
   Variable *keeplist;		/* container variables */
   Variable *found;		/* the chain we found */
   Int created=0, freed=0;
   Int numvars = MAX_VARS;

   describe_addr_buf = NULL;
   describe_addr_bufidx = 0;
   describe_addr_bufsz = 0;

   clear_visited();

   found = NULL;
   keeplist = NULL;

   eip = VG_(get_IP)(tid);
   list = ML_(get_scope_variables)(tid);

   if (memaccount) {
      Variable *v;

      for(v = list; v != NULL; v = v->next)
	 created++;
   }

   if (debug) {
      Char file[100];
      Int line;
      if (!VG_(get_filename_linenum)(eip, file, sizeof(file), 
                                          NULL, 0, NULL, &line))
	 file[0] = 0;
      VG_(printf)("describing address %p for tid=%d @ %s:%d\n", addr, tid, file, line);
   }

   if (LAZYSIG)
      setup_signals();

   /* breadth-first traversal of all memory visible to the program at
      the current point */
   while(list != NULL && found == NULL) {
      Variable **prev = &list;
      Variable *var, *next;
      Variable *newlist = NULL, *newlistend = NULL;

      if (debug)
	 VG_(printf)("----------------------------------------\n");

      for(var = list; var != NULL; var = next) {
	 SymType *type = var->type;
	 Bool keep = False;

	 /* Add a new variable to the list */
         // (the declaration avoids a compiler warning)
	 //static void newvar(Char *name, SymType *ty, Addr valuep, UInt size);
         void newvar(Char *name, SymType *ty, Addr valuep, UInt size) {
	    Variable *v;

	    /* have we been here before? */
	    if (has_visited(valuep, ty))
	       return;
	    
	    /* are we too deep? */
	    if (var->distance > MAX_PLY)
	       return;

	    /* have we done too much? */
	    if (numvars-- == 0)
	       return;

	    if (memaccount)
	       created++;
	    
	    v = VG_(arena_malloc)(VG_AR_SYMTAB, sizeof(*v));

	    if (name)
	       v->name = VG_(arena_strdup)(VG_AR_SYMTAB, name);
	    else
	       v->name = NULL;
	    v->type = ML_(st_basetype)(ty, False);
	    v->valuep = valuep;
	    v->size = size == -1 ? ty->size : size;
	    v->container = var;
	    v->distance = var->distance + 1;
	    v->next = NULL;

	    if (newlist == NULL)
	       newlist = newlistend = v;
	    else {
	       newlistend->next = v;
	       newlistend = v;
	    }
	    
	    if (debug)
	       VG_(printf)("    --> %d: name=%s type=%p(%s %s) container=%p &val=%p\n", 
			   v->distance, v->name, 
			   v->type, ppkind(v->type->kind), 
			   v->type->name ? (char *)v->type->name : "",
			   v->container, v->valuep);
	    keep = True;
	    return;
	 }

	 next = var->next;

	 if (debug)
	    VG_(printf)("  %d: name=%s type=%p(%s %s) container=%p &val=%p\n", 
			var->distance, var->name, 
			var->type, ppkind(var->type->kind), 
			var->type->name ? (char *)var->type->name : "",
			var->container, var->valuep);
   
	 if (0 && has_visited(var->valuep, var->type)) {
	    /* advance prev; we're keeping this one on the doomed list */
	    prev = &var->next;
	    continue;
	 }

	 if (!is_composite(var->type) && 
	     addr >= var->valuep && addr < (var->valuep + var->size)) {
	    /* at hit - remove it from the list, add it to the
	       keeplist and set found */
	    found = var;
	    *prev = var->next;
	    var->next = keeplist;
	    keeplist = var;
	    break;
	 }

	 type = ML_(st_basetype)(type, True);
	 
	 switch(type->kind) {
	 case TyUnion:
	 case TyStruct: {
	    Int i;

	    if (debug)
	       VG_(printf)("    %d fields\n", type->u.t_struct.nfield);
	    for(i = 0; i < type->u.t_struct.nfield; i++) {
	       StField *f = &type->u.t_struct.fields[i];
	       newvar(f->name, f->type, var->valuep + (f->offset / 8), (f->size + 7) / 8);
	    }
	    break;
	 }

	 case TyArray: {
	    Int i; 
	    Int offset;		/* offset of index for non-0-based arrays */
	    Int min, max;	/* range of indicies we care about (0 based) */
	    SymType *ty = type->u.t_array.type;
	    vg_assert(type->u.t_array.idxtype->kind == TyRange);

	    offset = type->u.t_array.idxtype->u.t_range.min;
	    min = 0;
	    max = type->u.t_array.idxtype->u.t_range.max - offset;

	    if ((max-min+1) == 0) {
#if SHADOWCHUNK
	       /* zero-sized array - look at allocated memory */
	       ShadowChunk *sc = findchunk(var->valuep);

	       if (sc != NULL) {
		  max = ((sc->data + sc->size - var->valuep) / ty->size) + min;
		  if (debug)
		     VG_(printf)("    zero sized array: using min=%d max=%d\n",
				 min, max);
	       }
#endif
	    }

	    /* If this array's elements can't take us anywhere useful,
	       just look to see if an element itself is being pointed
	       to; otherwise just skip the whole thing */
	    if (!is_followable(ty)) {
	       UInt sz = ty->size * (max+1);

	       if (debug)
		  VG_(printf)("    non-followable array (sz=%d): checking addr %p in range %p-%p\n",
			      sz, addr, var->valuep, (var->valuep + sz));
	       if (ty->size > 0 && addr >= var->valuep && addr <= (var->valuep + sz))
		  min = max = (addr - var->valuep) / ty->size;
	       else
		  break;
	    }

	    /* truncate array if it's too big */
	    if (max-min+1 > MAX_ELEMENTS)
	       max = min+MAX_ELEMENTS;
	    
	    if (debug)
	       VG_(printf)("    array index %d - %d\n", min, max);
	    for(i = min; i <= max; i++) {
	       Char b[10];
	       VG_(sprintf)(b, "[%d]", i+offset);
	       newvar(b, ty, var->valuep + (i * ty->size), -1);
	    }

	    break;
	 }

	 case TyPointer:
	    /* follow */
	    /* XXX work out a way of telling whether a pointer is
	       actually a decayed array, and treat it accordingly */
	    if (is_valid_addr(var->valuep))
	       newvar(NULL, type->u.t_pointer.type, *(Addr *)var->valuep, -1);
	    break;

	 case TyUnresolved:
	    VG_(printf)("var %s is unresolved (type=%p)\n", var->name, type);
	    break;

	 default:
	    /* Simple non-composite, non-pointer type */
	    break;
	 }
	 
	 if (keep) {
	    /* ironically, keep means remove it from the list */
	    *prev = next;
	    
	    /* being kept - add it if not already there */
	    if (keeplist != var) {
	       var->next = keeplist;
	       keeplist = var;
	    }
	 } else {
	    /* advance prev; we're keeping it on the doomed list */
	    prev = &var->next;
	 }
      }

      /* kill old list */
      freed += free_varlist(list);
      list = NULL;

      if (found) {
	 /* kill new list too */
	 freed += free_varlist(newlist);
	 newlist = newlistend = NULL;
      } else {
	 /* new list becomes old list */
	 list = newlist;
      }
   }

   if (LAZYSIG)
      restore_signals();

   if (found != NULL) {
      Int len = 0;
      Char file[100];
      Int line;

      /* Try to generate an idiomatic C-like expression from what
	 we've found. */

      {
	 Variable *v;
	 for(v = found; v != NULL; v = v->container) {
	    if (debug)
	       VG_(printf)("v=%p (%s) %s\n",
			   v, v->name ? v->name : (Char *)"",
			   ppkind(v->type->kind));
	    
	    len += (v->name ? VG_(strlen)(v->name) : 0) + 5;
	 }
      }

      /* now that we know how long the expression will be
	 (approximately) build it up */
      {
	 Char expr[len*2];
	 Char *sp = &expr[len];	/* pointer at start of string */
	 Char *ep = sp;		/* pointer at end of string */
	 //  static void genstring(Variable *v, Variable *inner);  // avoid warning
         void genstring(Variable *v, Variable *inner) {
	    Variable *c = v->container;

	    if (c != NULL)
	       genstring(c, v);

	    if (v->name != NULL) {
	       len = VG_(strlen)(v->name);
	       VG_(memcpy)(ep, v->name, len);
	       ep += len;
	    }

	    switch(v->type->kind) {
	    case TyPointer:
	       /* pointer-to-structure/union handled specially */
	       if (inner == NULL ||
		   !(inner->type->kind == TyStruct || inner->type->kind == TyUnion)) {
		  *--sp = '*';
		  *--sp = '(';
		  *ep++ = ')';
	       }
	       break;

	    case TyStruct:
	    case TyUnion:
	       if (c && c->type->kind == TyPointer) {
		  *ep++ = '-';
		  *ep++ = '>';
	       } else
		  *ep++ = '.';
	       break;

	    default:
	       break;
	    }
	 }

	 {
	    Bool ptr = True;

	    /* If the result is already a pointer, just use that as
	       the value, otherwise generate &(...) around the
	       expression. */
	    if (found->container && found->container->type->kind == TyPointer) {
	       vg_assert(found->name == NULL);

	       found->name = found->container->name;
	       found->container->name = NULL;
	       found->container = found->container->container;
	    } else {
               bprintf(describe_addr_addbuf, 0, "&(");
	       ptr = False;
	    }

	    genstring(found, NULL);

	    if (!ptr)
	       *ep++ = ')';
	 }

	 *ep++ = '\0';

	 bprintf(describe_addr_addbuf, 0, sp);

	 if (addr != found->valuep)
	    bprintf(describe_addr_addbuf, 0, "+%d", addr - found->valuep);

	 if (VG_(get_filename_linenum)(eip, file, sizeof(file), 
                                            NULL, 0, NULL, &line))
	    bprintf(describe_addr_addbuf, 0, " at %s:%d", file, line, addr);
      }
   }

   freed += free_varlist(keeplist);

   if (memaccount)
      VG_(printf)("created %d, freed %d\n", created, freed);

   clear_visited();

   if (debug)
      VG_(printf)("returning buf=%s\n", describe_addr_buf);

   return describe_addr_buf;
}
#endif /* TEST */

/*--------------------------------------------------------------------*/
/*--- end                                                          ---*/
/*--------------------------------------------------------------------*/
