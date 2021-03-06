include $(top_srcdir)/Makefile.tool.am

noinst_PROGRAMS = 
if VG_X86_NETBSD
noinst_PROGRAMS += memcheck-x86-netbsd vgpreload_memcheck-x86-netbsd.so
endif
if VG_X86_LINUX
noinst_PROGRAMS += memcheck-x86-linux vgpreload_memcheck-x86-linux.so
endif
if VG_AMD64_LINUX
noinst_PROGRAMS += memcheck-amd64-linux vgpreload_memcheck-amd64-linux.so
endif
if VG_PPC32_LINUX
noinst_PROGRAMS += memcheck-ppc32-linux vgpreload_memcheck-ppc32-linux.so
endif

VGPRELOAD_MEMCHECK_SOURCES_COMMON = mac_replace_strmem.c

vgpreload_memcheck_x86_linux_so_SOURCES      = $(VGPRELOAD_MEMCHECK_SOURCES_COMMON)
vgpreload_memcheck_x86_linux_so_CPPFLAGS     = $(AM_CPPFLAGS_X86_LINUX)
vgpreload_memcheck_x86_linux_so_CFLAGS       = $(AM_CFLAGS_X86_LINUX) $(AM_CFLAGS_PIC) -O2
vgpreload_memcheck_x86_linux_so_CCASFLAGS    = $(AM_CCASFLAGS_X86_LINUX)
vgpreload_memcheck_x86_linux_so_DEPENDENCIES = $(LIBREPLACEMALLOC_X86_LINUX)
vgpreload_memcheck_x86_linux_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_X86_LINUX) \
	$(LIBREPLACEMALLOC_LDFLAGS_X86_LINUX)

vgpreload_memcheck_amd64_linux_so_SOURCES      = $(VGPRELOAD_MEMCHECK_SOURCES_COMMON)
vgpreload_memcheck_amd64_linux_so_CPPFLAGS     = $(AM_CPPFLAGS_AMD64_LINUX)
vgpreload_memcheck_amd64_linux_so_CFLAGS       = $(AM_CFLAGS_AMD64_LINUX) $(AM_CFLAGS_PIC) -O2
vgpreload_memcheck_amd64_linux_so_CCASFLAGS    = $(AM_CCASFLAGS_AMD64_LINUX)
vgpreload_memcheck_amd64_linux_so_DEPENDENCIES = $(LIBREPLACEMALLOC_AMD64_LINUX)
vgpreload_memcheck_amd64_linux_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_AMD64_LINUX) \
	$(LIBREPLACEMALLOC_LDFLAGS_AMD64_LINUX)

vgpreload_memcheck_ppc32_linux_so_SOURCES      = $(VGPRELOAD_MEMCHECK_SOURCES_COMMON)
vgpreload_memcheck_ppc32_linux_so_CPPFLAGS     = $(AM_CPPFLAGS_PPC32_LINUX)
vgpreload_memcheck_ppc32_linux_so_CFLAGS       = $(AM_CFLAGS_PPC32_LINUX) $(AM_CFLAGS_PIC) -O2
vgpreload_memcheck_ppc32_linux_so_CCASFLAGS    = $(AM_CCASFLAGS_PPC32_LINUX)
vgpreload_memcheck_ppc32_linux_so_DEPENDENCIES = $(LIBREPLACEMALLOC_PPC32_LINUX)
vgpreload_memcheck_ppc32_linux_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_PPC32_LINUX) \
	$(LIBREPLACEMALLOC_LDFLAGS_PPC32_LINUX)

vgpreload_memcheck_x86_netbsd_so_SOURCES      = $(VGPRELOAD_MEMCHECK_SOURCES_COMMON)
vgpreload_memcheck_x86_netbsd_so_CPPFLAGS     = $(AM_CPPFLAGS_X86_NETBSD)
vgpreload_memcheck_x86_netbsd_so_CFLAGS       = $(AM_CFLAGS_X86_NETBSD) $(AM_CFLAGS_PIC) -O2
vgpreload_memcheck_x86_netbsd_so_CCASFLAGS    = $(AM_CCASFLAGS_X86_NETBSD)
vgpreload_memcheck_x86_netbsd_so_DEPENDENCIES = $(LIBREPLACEMALLOC_X86_NETBSD)
vgpreload_memcheck_x86_netbsd_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_X86_NETBSD) \
	$(LIBREPLACEMALLOC_LDFLAGS_X86_NETBSD)

MEMCHECK_SOURCES_COMMON = \
	mac_leakcheck.c \
	mac_malloc_wrappers.c \
	mc_main.c \
	mac_shared.c \
	mc_translate.c

memcheck_x86_netbsd_SOURCES      = $(MEMCHECK_SOURCES_COMMON) ../coregrind/netbsd-ident.S
memcheck_x86_netbsd_CPPFLAGS     = $(AM_CPPFLAGS_X86_NETBSD)
memcheck_x86_netbsd_CFLAGS       = $(AM_CFLAGS_X86_NETBSD) -O2
memcheck_x86_netbsd_CCASFLAGS    = $(AM_CCASFLAGS_X86_NETBSD)
memcheck_x86_netbsd_DEPENDENCIES = $(COREGRIND_LIBS_X86_NETBSD)
memcheck_x86_netbsd_LDADD        = $(TOOL_LDADD_X86_NETBSD)
memcheck_x86_netbsd_LDFLAGS      = $(TOOL_LDFLAGS_X86_NETBSD)

memcheck_x86_linux_SOURCES      = $(MEMCHECK_SOURCES_COMMON)
memcheck_x86_linux_CPPFLAGS     = $(AM_CPPFLAGS_X86_LINUX)
memcheck_x86_linux_CFLAGS       = $(AM_CFLAGS_X86_LINUX) -O2
memcheck_x86_linux_CCASFLAGS    = $(AM_CCASFLAGS_X86_LINUX)
memcheck_x86_linux_DEPENDENCIES = $(COREGRIND_LIBS_X86_LINUX)
memcheck_x86_linux_LDADD        = $(TOOL_LDADD_X86_LINUX)
memcheck_x86_linux_LDFLAGS      = $(TOOL_LDFLAGS_X86_LINUX)

memcheck_amd64_linux_SOURCES      = $(MEMCHECK_SOURCES_COMMON)
memcheck_amd64_linux_CPPFLAGS     = $(AM_CPPFLAGS_AMD64_LINUX)
memcheck_amd64_linux_CFLAGS       = $(AM_CFLAGS_AMD64_LINUX) -O2
memcheck_amd64_linux_CCASFLAGS    = $(AM_CCASFLAGS_AMD64_LINUX)
memcheck_amd64_linux_DEPENDENCIES = $(COREGRIND_LIBS_AMD64_LINUX)
memcheck_amd64_linux_LDADD        = $(TOOL_LDADD_AMD64_LINUX)
memcheck_amd64_linux_LDFLAGS      = $(TOOL_LDFLAGS_AMD64_LINUX)

memcheck_ppc32_linux_SOURCES      = $(MEMCHECK_SOURCES_COMMON)
memcheck_ppc32_linux_CPPFLAGS     = $(AM_CPPFLAGS_PPC32_LINUX)
memcheck_ppc32_linux_CFLAGS       = $(AM_CFLAGS_PPC32_LINUX) -O2
memcheck_ppc32_linux_CCASFLAGS    = $(AM_CCASFLAGS_PPC32_LINUX)
memcheck_ppc32_linux_DEPENDENCIES = $(COREGRIND_LIBS_PPC32_LINUX)
memcheck_ppc32_linux_LDADD        = $(TOOL_LDADD_PPC32_LINUX)
memcheck_ppc32_linux_LDFLAGS      = $(TOOL_LDFLAGS_PPC32_LINUX)

mcincludedir = $(includedir)/valgrind

mcinclude_HEADERS = \
	memcheck.h

noinst_HEADERS =	\
	mac_shared.h	\
	mc_include.h

mac_replace_strmem.o: CFLAGS += -fno-omit-frame-pointer

mc_main.o: CFLAGS += -fomit-frame-pointer
