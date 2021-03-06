
/*--------------------------------------------------------------------*/
/*--- Launching valgrind                              m_launcher.c ---*/
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

/* Note: this is a "normal" program and not part of Valgrind proper,
   and so it doesn't have to conform to Valgrind's arcane rules on
   no-glibc-usage etc. */


#include "pub_core_debuglog.h"
#include "pub_core_libcproc.h"  // For VALGRIND_LIB, VALGRIND_LAUNCHER
#include "pub_core_ume.h"

#include <sys/param.h> /* so its netbsd specific */
#include <assert.h>
#include <ctype.h>
#include <elf.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/user.h>
#include <unistd.h>


#ifndef PATH_MAX
#define PATH_MAX 4096 /* POSIX refers to this a lot but I dunno
                         where it is defined */
#endif 
/* Report fatal errors */
static void barf ( const char *format, ... )
{
   va_list vargs;

   va_start(vargs, format);
   fprintf(stderr, "valgrind: Cannot continue: ");
   vfprintf(stderr, format, vargs);
   fprintf(stderr, "\n");
   va_end(vargs);

   exit(1);
}

/* Search the path for the client program */
static const char *find_client(const char *clientname)
{
   static char fullname[PATH_MAX];
   const char *path = getenv("PATH");
   const char *colon;

   while (path)
   {
      if ((colon = strchr(path, ':')) == NULL)
      {
         strcpy(fullname, path);
         path = NULL;
      }
      else
      {
         memcpy(fullname, path, colon - path);
         fullname[colon - path] = '\0';
         path = colon + 1;
      }

      strcat(fullname, "/");
      strcat(fullname, clientname);

      if (access(fullname, R_OK|X_OK) == 0)
         return fullname;
   }

   return clientname;
}

/* Examine the client and work out which platform it is for */
static const char *select_platform(const char *clientname)
{
   int fd;
   unsigned char *header;
   const char *platform = NULL;
   long pagesize = sysconf(_SC_PAGESIZE);

   if (strchr(clientname, '/') == NULL)
      clientname = find_client(clientname);

   if ((fd = open(clientname, O_RDONLY)) < 0)
      return NULL;
   //   barf("open(%s): %s", clientname, strerror(errno));

   if ((header = mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
      return NULL;
   //   barf("mmap(%s): %s", clientname, strerror(errno));

   close(fd);

   if (header[0] == '#' && header[1] == '!') {
      char *interp = (char *)header + 2;
      char *interpend;

      while (*interp == ' ' || *interp == '\t')
         interp++;

      for (interpend = interp; !isspace(*interpend); interpend++)
         ;

      *interpend = '\0';

      platform = select_platform(interp);
   } else if (memcmp(header, ELFMAG, SELFMAG) == 0 &&
              header[EI_CLASS] == ELFCLASS32 &&
              header[EI_DATA] == ELFDATA2LSB) {
      const Elf32_Ehdr *ehdr = (Elf32_Ehdr *)header;

      if (ehdr->e_machine == EM_386 &&
          ehdr->e_ident[EI_OSABI] == ELFOSABI_SYSV) {
#ifdef NetBSD 
	      platform ="x86-netbsdelf2";
#else
         platform = "x86-linux";
#endif
      } else if (ehdr->e_machine == EM_PPC &&
                 ehdr->e_ident[EI_OSABI] == ELFOSABI_SYSV) {
         platform = "ppc32-linux";
      }
   } else if (memcmp(header, ELFMAG, SELFMAG) == 0 &&
              header[EI_CLASS] == ELFCLASS64 &&
              header[EI_DATA] == ELFDATA2LSB) {
      const Elf64_Ehdr *ehdr = (Elf64_Ehdr *)header;

      if (ehdr->e_machine == EM_X86_64 &&
          ehdr->e_ident[EI_OSABI] == ELFOSABI_SYSV) {
         platform = "amd64-linux";
      }
   }

   munmap(header, pagesize);

   return platform;
}

/* Where we expect to find all our aux files */
static const char *valgrind_lib = VG_LIBDIR;

int main(int argc, char** argv, char** envp)
{
   int i, j, loglevel, r;
   const char *toolname = NULL;
   const char *clientname = NULL;
   const char *platform;
   const char *cp;
   char *toolfile;
   char launcher_name[PATH_MAX+1];
   char* new_line;
   char** new_env;

   /* Start the debugging-log system ASAP.  First find out how many 
      "-d"s were specified.  This is a pre-scan of the command line.
      At the same time, look for the tool name. */
   loglevel = 0;
   for (i = 1; i < argc; i++) {
      if (argv[i][0] != '-') {
         clientname = argv[i];
         break;
      }
      if (0 == strcmp(argv[i], "--")) {
         if (i+1 < argc)
            clientname = argv[i+1];
         break;
      }
      if (0 == strcmp(argv[i], "-d")) 
         loglevel++;
      if (0 == strncmp(argv[i], "--tool=", 7)) 
         toolname = argv[i] + 7;
   }

   /* ... and start the debug logger.  Now we can safely emit logging
      messages all through startup. */
   VG_(debugLog_startup)(loglevel, "Stage 1");

   /* Make sure we know which tool we're using */
   if (toolname) {
      VG_(debugLog)(1, "launcher", "tool '%s' requested\n", toolname);
   } else {
      VG_(debugLog)(1, "launcher", 
                       "no tool requested, defaulting to 'memcheck'\n");
      toolname = "memcheck";
   }

   /* Work out what platform to use */
   if (clientname == NULL) {
      VG_(debugLog)(1, "launcher", "no client specified, defaulting platform to '%s'\n", VG_PLATFORM);
      platform = VG_PLATFORM;
   } else if ((platform = select_platform(clientname)) != NULL) {
      VG_(debugLog)(1, "launcher", "selected platform '%s'\n", platform);
   } else {
      VG_(debugLog)(1, "launcher", "no platform detected, defaulting platform to '%s'\n", VG_PLATFORM);
      platform = VG_PLATFORM;
   }
   
   /* Figure out the name of this executable (viz, the launcher), so
      we can tell stage2.  stage2 will use the name for recursive
      invokations of valgrind on child processes. */
/* #if defined VGO_netbsdelf2 */
/* // ok /proc/self/exe is not a symlink for us, that means we ust put */
/* // that as the laungher name and see how it goes.  -XXX this may be */
/* // broken */
/*    snprintf(launcher_name,PATH_MAX,"%s","/proc/self/exe"); */
/* #else */
#ifndef VGO_netbsdelf2
   memset(launcher_name, 0, PATH_MAX+1);
   r = readlink("/proc/self/exe", launcher_name, PATH_MAX);
   if (r == -1) {
      /* If /proc/self/exe can't be followed, don't give up.  Instead
         continue with an empty string for VALGRIND_LAUNCHER.  In the
         sys_execve wrapper, this is tested, and if found to be empty,
         fail the execve. */
      fprintf(stderr, "valgrind: warning (non-fatal): "
                      "readlink(\"/proc/self/exe\") failed.\n");
      fprintf(stderr, "valgrind: continuing, however --trace-children=yes "
                      "will not work.\n");
   }
/* #endif */

   /* tediously augment the env: VALGRIND_LAUNCHER=launcher_name */
   new_line = malloc(strlen(VALGRIND_LAUNCHER) + 1 
                     + strlen(launcher_name) + 1);
   if (new_line == NULL)
      barf("malloc of new_line failed.");
   strcpy(new_line, VALGRIND_LAUNCHER);
   strcat(new_line, "=");
   strcat(new_line, launcher_name);

   for (j = 0; envp[j]; j++)
      ;
   new_env = malloc((j+2) * sizeof(char*)); // allocate space for the ptrs
   if (new_env == NULL)
      barf("malloc of new_env failed.");
   printf("envp[0] is %s\n",envp[0]);
   for (i = 0; i < j; i++) // envp[0] is what?!
      new_env[i] = envp[i];
   new_env[i++] = new_line;
   new_env[i++] = NULL;
   assert(i == j+2);
#endif 
   /* Establish the correct VALGRIND_LIB. */
   cp = getenv(VALGRIND_LIB);

   if (cp != NULL)
      valgrind_lib = cp;

   /* Build the stage2 invokation, and execve it.  Bye! */
   toolfile = malloc(strlen(valgrind_lib) + strlen(toolname) + strlen(platform) + 3);
   if (toolfile == NULL)
      barf("malloc of toolfile failed.");
   sprintf(toolfile, "%s/%s/%s", valgrind_lib, platform, toolname);

   VG_(debugLog)(1, "launcher", "launching %s\n", toolfile);
// error here
   printf("toolfile : %s \n argv: %s \n valgrind_launcher %s  \n",toolfile, *argv,VALGRIND_LAUNCHER);
#ifdef VGO_netbsdelf2
   execve(toolfile, argv, envp);
#else
   execve(toolfile, argv, new_env);
#endif
   fprintf(stderr, "valgrind: failed to start tool '%s' for platform '%s': %s\n"
	   "Please set the VALGRIND_LIB environment variable to point to the\n"
	   ".in_place directory under valgrind toplevel sourcedir for now\n",
                   toolname, platform, strerror(errno));

   exit(1);
}
