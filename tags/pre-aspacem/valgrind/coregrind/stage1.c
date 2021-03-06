/*--------------------------------------------------------------------*/
/*--- Startup: preliminaries                              stage1.c ---*/
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

#define _FILE_OFFSET_BITS	64

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <unistd.h>

#include "memcheck/memcheck.h"
#include "pub_core_basics.h"
#include "pub_core_debuglog.h"
#include "pub_core_libcbase.h"
#include "pub_core_libcproc.h"
#include "ume.h"


static int stack[SIGSTKSZ*4];

// Initial stack pointer, which points to argc.
static void* init_sp;

/* Where we expect to find all our aux files (namely, stage2) */
static const char *valgrind_lib = VG_LIBDIR;

/* stage2's name */
static const char stage2[] = "stage2";

/*------------------------------------------------------------*/
/*--- Auxv modification                                    ---*/
/*------------------------------------------------------------*/

/* Modify the auxv the kernel gave us to make it look like we were
   execed as the shared object.

   This also inserts a new entry into the auxv table so we can
   communicate some extra information to stage2 (namely, the fd of the
   padding file, so it can identiry and remove the padding later).
*/
static void *fix_auxv(void *v_init_esp, const struct exeinfo *info,
                      int padfile)
{
   struct ume_auxv *auxv;
   int *newesp;
   int seen;
   int delta;
   int i;
#if defined(VGO_netbsdelf2)
   static const int new_entries = 14;
#else
   static const int new_entries = 2;
#endif
   /* make sure we're running on the private stack */
   assert(&delta >= stack && &delta < &stack[sizeof(stack)/sizeof(*stack)]);
   
   /* find the beginning of the AUXV table */
   auxv = find_auxv(v_init_esp);
   printf("auxv[0].u.a_val : %d, auxv[0].a_type : %d\n",auxv[0].u.a_val, auxv[0].a_type);
   /* Work out how we should move things to make space for the new
      auxv entry. It seems that ld.so wants a 16-byte aligned stack on
      entry, so make sure that's the case. */
#if defined(VGO_netbsdelf2)
   newesp = (int *)(((unsigned long)v_init_esp - new_entries * sizeof(*auxv)));
#else
   newesp = (int *)(((unsigned long)v_init_esp - new_entries * sizeof(*auxv)) & ~0xf);
#endif
   delta = (char *)v_init_esp - (char *)newesp;

   memmove(newesp, v_init_esp, (char *)auxv - (char *)v_init_esp);
   
   v_init_esp = (void *)newesp;
/* we need to see whats the size of delta, we are freeing up space to
   throw our own auxv headers in 3, */
   printf("Space for auxv %d\n",delta/sizeof(*auxv));
   printf("sizeof delta :%d , sizeof auxv : %d\n",delta, sizeof(*auxv));
/* if auxv is not aligned, we move it backward till its aligned. We
   find out how much space is required, walk tmp_auxv to the last auxv
   entry.  then move the junk forward. Remeber that the AT_NULL entry
   also needs to be moved forward. */

/*    if( ( delta%sizeof(*auxv) ) != 0 ) */
/*     {	    int j = 0; */
/*     printf("adjusting auxv\n"); */
/* 	    struct ume_auxv* tmp = auxv; /\* to top *\/ */
/* 	    void * new_auxv = (void *) auxv - delta%(sizeof(*auxv)); */
/* 	    for (j=0; tmp->a_type != AT_NULL;j++,tmp++ ); */
/* 	    memmove( new_auxv,auxv,(j+1)* sizeof(*auxv) ); */
/* 	    auxv = (struct ume_auxv *)new_auxv; */
/*     } */
   auxv -= delta/sizeof(*auxv) ; /* move it back even further for 2
				  * new */

   /* stage2 needs this so it can clean up the padding we leave in
      place when we start it */
   auxv[0].a_type = AT_UME_PADFD;
   auxv[0].u.a_val = padfile;
   /* This will be needed by valgrind itself so that it can
      subsequently execve() children.  This needs to be done here
      because /proc/self/exe will go away once we unmap stage1. */
   auxv[1].a_type = AT_UME_EXECFD;
#if defined(VGO_netbsdelf2)
  auxv[1].u.a_val = open("/proc/curproc/file", O_RDONLY);
/* fill in the rest */
   auxv[2].a_type= AT_PHDR;
   auxv[2].u.a_val = info->phdr;
   auxv[3].a_type = AT_PHNUM;
   auxv[3].u.a_val = info->phnum;
   auxv[4].a_type = AT_BASE;
   auxv[4].u.a_val = info->interp_base;
   auxv[5].a_type = AT_ENTRY;
   auxv[5].u.a_val = info->entry;
   auxv[6].a_type = AT_PAGESZ;
   auxv[6].u.a_val = 4096;
   auxv[7].a_type = AT_PHENT;
   auxv[7].u.a_val = 32; /*sizeof(info->phdr);*/
   auxv[8].a_type = AT_FLAGS;
   auxv[8].u.a_val = 0; /* ? */
   auxv[9].a_type = AT_EUID;
   auxv[9].u.a_val = geteuid();
   auxv[10].a_type = AT_RUID;
   auxv[10].u.a_val = getuid();
   auxv[11].a_type = AT_EGID;
   auxv[11].u.a_val = getegid();
   auxv[12].a_type = AT_RGID;
   auxv[12].u.a_val = getgid();
   auxv[13].a_type = AT_NULL;
#else
   auxv[1].u.a_val = open("/proc/self/exe", O_RDONLY);
#endif

   /* make sure the rest are sane */
   for(i = new_entries; i < delta/sizeof(*auxv); i++) {
      auxv[i].a_type = AT_IGNORE;
      auxv[i].u.a_val = 0;
   }

   /* OK, go through and patch up the auxv entries to match the new
      executable */
   seen = 0;
   for(; auxv->a_type != AT_NULL; auxv++) {
	       if (1)
	   printf("doing auxv %p %5lld: %p \n",
		  auxv, (Long)auxv->a_type, (Long)auxv->u.a_val);
/* 	   if(seen == 0xf) */
/* 		   break; */
	   switch(auxv->a_type) {
	   case AT_PHDR:
		   printf("seen 1\n");
		   seen |= 1;
		   auxv->u.a_val = info->phdr;
		   break;

	   case AT_PHNUM:
		   printf("seen 2\n");
		   seen |= 2;
		   auxv->u.a_val = info->phnum;
		   break;

	   case AT_BASE:
		   printf("seen 4\n");
		   seen |= 4;
		   auxv->u.a_val = info->interp_base;
		   break;

	   case AT_ENTRY:
		   printf("seen 8\n");
		   seen |= 8;
		   auxv->u.a_val = info->entry; 
		   break;

	   default:
		   break;

	   }
	       if (1)
	   printf("new auxv %p %5lld: %p \n",
		  auxv, (Long)auxv->a_type, (Long)auxv->u.a_val);

   }

   /* If we didn't see all the entries we need to fix up, then we
      can't make the new executable viable. */
   if (seen != 0xf) {
	   fprintf(stderr, "valgrind: we didn't see enough auxv entries (seen=%x)\n", seen);
	   exit(1);
   }

   return v_init_esp;
}


/*------------------------------------------------------------*/
/*--- Address space padding                                ---*/
/*------------------------------------------------------------*/

static void check_mmap(void* res, void* base, int len)
{
   if ((void*)-1 == res) {
      fprintf(stderr, "valgrind: padding mmap(%p, %d) failed during startup.\n"
                      "valgrind: is there a hard virtual memory limit set?\n",
                      base, len);
      exit(1);
   }
}

typedef struct {
   char* fillgap_start;
   char* fillgap_end;
   int   fillgap_padfile;
} fillgap_extra;

static int fillgap(char *segstart, char *segend, const char *perm, off_t off, 
                   int maj, int min, int ino, void* e)
{
   fillgap_extra* extra = e;

   if (segstart >= extra->fillgap_end)
      return 0;

   if (segstart > extra->fillgap_start) {
      void* res = mmap(extra->fillgap_start, segstart - extra->fillgap_start,
                       PROT_NONE, MAP_FIXED|MAP_PRIVATE, 
                       extra->fillgap_padfile, 0);
      check_mmap(res, extra->fillgap_start, segstart - extra->fillgap_start);
   }
   extra->fillgap_start = segend;
   
   return 1;
}

// Choose a name for the padfile, open it.
static 
int as_openpadfile(void)
{
   char buf[256];
   int padfile;
   int seq = 1;
   do {
      snprintf(buf, 256, "/tmp/.pad.%d.%d", getpid(), seq++);
      padfile = open(buf, O_RDWR|O_CREAT|O_EXCL, 0);
      unlink(buf);
      if (padfile == -1 && errno != EEXIST) {
         fprintf(stderr, "valgrind: couldn't open padfile\n");
         exit(44);
      }
   } while(padfile == -1);

   return padfile;
}

// Pad all the empty spaces in a range of address space to stop interlopers.
static
void as_pad(void *start, void *end, int padfile)
{
   fillgap_extra extra;
   extra.fillgap_start   = start;
   extra.fillgap_end     = end;
   extra.fillgap_padfile = padfile;

   foreach_map(fillgap, &extra);
	
   if (extra.fillgap_start < extra.fillgap_end) {
      void* res = mmap(extra.fillgap_start, 
                       extra.fillgap_end - extra.fillgap_start,
                       PROT_NONE, MAP_FIXED|MAP_PRIVATE, padfile, 0);
      check_mmap(res, extra.fillgap_start, 
                 extra.fillgap_end - extra.fillgap_start);
   }
}


/*------------------------------------------------------------*/
/*--- main() and related pieces                            ---*/
/*------------------------------------------------------------*/

static int prmap(char *start, char *end, const char *perm, off_t off, int maj,
                 int min, int ino, void* dummy) {
   printf("mapping %10p-%10p %s %02x:%02x %d\n",
          start, end, perm, maj, min, ino);
   return 1;
}


static void main2(void)
{
   int err, padfile;
   struct exeinfo info;
   extern char _end;
   int *esp;
   char buf[strlen(valgrind_lib) + sizeof(stage2) + 16];
   info.exe_end  = VG_PGROUNDDN(init_sp); // rounding down
   info.exe_base = KICKSTART_BASE;
   printf("info.exe_end = %p\n", info.exe_end);
#ifdef HAVE_PIE
   info.exe_base = VG_ROUNDDN(info.exe_end - 0x02000000, 0x10000000);
   assert(info.exe_base >= VG_PGROUNDUP(&_end));
   info.map_base = info.exe_base + 0x01000000  ;
#else
   // If this system doesn't have PIE (position-independent executables),
   // we have to choose a hardwired location for stage2.
   //   info.exe_base = VG_PGROUNDUP(&_end);
   printf("info.exe_base = %p\n", info.exe_base);
   info.map_base = KICKSTART_BASE  + 0x01000000 ;
   printf("info.map_base = %p\n", info.map_base);
#endif

   info.argv = NULL;

   snprintf(buf, sizeof(buf), "%s/%s", valgrind_lib, stage2);
	  printf("valgrind_lib = %s\n",valgrind_lib);
   err = do_exec(buf, &info);

   if (err != 0) {
      fprintf(stderr, "valgrind: failed to load %s: %s\n",
	      buf, strerror(err));
      exit(1);
   }

   /* Make sure stage2's dynamic linker can't tromp on the lower part
      of the address space. */
   padfile = as_openpadfile();
   as_pad(0, (void *)info.map_base, padfile); // map base is the start of our stuff 
   printf("init sp : %x\n", init_sp);

   esp = fix_auxv(init_sp, &info, padfile);
   printf("after fix_auxb\n");

   if (1) {
      printf("---------- launch stage 2 ----------\n");
      printf("eip=%p esp=%p\n", (void *)info.init_eip, esp);
      foreach_map(prmap, /*dummy*/NULL);
   }

   VG_(debugLog)(1, "stage1", "main2(): starting stage2\n");
   printf("jumping to stage 2 \n");
   printf("esp : %x \n eip : %x\n",esp, info.init_eip);

   jump_and_switch_stacks(
         (Addr) esp,            /* stack */
         (Addr) info.init_eip   /* Where to jump */
   );

   /*NOTREACHED*/
   assert(0); 
}


int main(int argc, char** argv)
{
   struct rlimit rlim;
   const char *cp;
   int i, loglevel;

   /* Start the debugging-log system ASAP.  First find out how many 
      "-d"s were specified.  This is a pre-scan of the command line. */
   loglevel = 0;
   for (i = 1; i < argc; i++) {
     if (argv[i][0] != '-')
        break;
     if (0 == strcmp(argv[i], "--")) 
        break;
     if (0 == strcmp(argv[i], "-d")) 
        loglevel++;
   }

   /* ... and start the debug logger.  Now we can safely emit logging
      messages all through startup. */
   VG_(debugLog_startup)(loglevel, "Stage 1");

   // Initial stack pointer is to argc, which is immediately before argv[0]
   // on the stack.  Nb: Assumes argc is word-aligned.
// in case of netbsd initial stack pointer will beeeeeeee return argc argv 
   init_sp = argv - 1;

   /* The Linux libc startup sequence leaves this in an apparently
      undefined state, but it really is defined, so mark it so. */
   VALGRIND_MAKE_READABLE(init_sp, sizeof(int));
   cp = getenv(VALGRINDLIB);

   if (cp != NULL)
      valgrind_lib = cp;

   /* Set the address space limit as high as it will go, since we make
      a lot of very large mappings. */
#ifndef VGO_netbsdelf2
   getrlimit(RLIMIT_AS, &rlim);
   rlim.rlim_cur = rlim.rlim_max;
   setrlimit(RLIMIT_AS, &rlim);
#endif
#ifdef VGO_netbsdelf2
   getrlimit(RLIMIT_RSS, &rlim);
   rlim.rlim_cur = rlim.rlim_max;
   setrlimit(RLIMIT_RSS, &rlim);
#endif 
   /* move onto another stack so we can play with the main one */
   VG_(debugLog)(1, "stage1", "main(): running main2() on new stack\n");
   jump_and_switch_stacks(
      (Addr) stack + sizeof(stack),  /* stack */
       (Addr) main2                   /* where to */

   );

   /*NOTREACHED*/
   assert(0); 
}

/*--------------------------------------------------------------------*/
/*--- end                                                 stage1.c ---*/
/*--------------------------------------------------------------------*/
