include $(top_srcdir)/Makefile.tool.am

SUBDIRS += hp2ps

noinst_PROGRAMS = 
if VG_X86_NETBSD
noinst_PROGRAMS += massif-x86-netbsd vgpreload_massif-x86-netbsd.so
endif
if VG_X86_LINUX
noinst_PROGRAMS += massif-x86-linux vgpreload_massif-x86-linux.so
endif
if VG_AMD64_LINUX
noinst_PROGRAMS += massif-amd64-linux vgpreload_massif-amd64-linux.so
endif
if VG_PPC32_LINUX
noinst_PROGRAMS += massif-ppc32-linux vgpreload_massif-ppc32-linux.so
endif

vgpreload_massif_x86_netbsd_so_SOURCES      = 
vgpreload_massif_x86_netbsd_so_CPPFLAGS     = $(AM_CPPFLAGS_X86_NETBSD)
vgpreload_massif_x86_netbsd_so_CFLAGS       = $(AM_CFLAGS_X86_NETBSD) $(AM_CFLAGS_PIC)
vgpreload_massif_x86_netbsd_so_DEPENDENCIES = $(LIBREPLACEMALLOC_X86_NETBSD)
vgpreload_massif_x86_netbsd_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_X86_NETBSD) \
	$(LIBREPLACEMALLOC_LDFLAGS_X86_NETBSD)

vgpreload_massif_x86_linux_so_SOURCES      = 
vgpreload_massif_x86_linux_so_CPPFLAGS     = $(AM_CPPFLAGS_X86_LINUX)
vgpreload_massif_x86_linux_so_CFLAGS       = $(AM_CFLAGS_X86_LINUX) $(AM_CFLAGS_PIC)
vgpreload_massif_x86_linux_so_DEPENDENCIES = $(LIBREPLACEMALLOC_X86_LINUX)
vgpreload_massif_x86_linux_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_X86_LINUX) \
	$(LIBREPLACEMALLOC_LDFLAGS_X86_LINUX)

vgpreload_massif_amd64_linux_so_SOURCES      = 
vgpreload_massif_amd64_linux_so_CPPFLAGS     = $(AM_CPPFLAGS_AMD64_LINUX)
vgpreload_massif_amd64_linux_so_CFLAGS       = $(AM_CFLAGS_AMD64_LINUX) $(AM_CFLAGS_PIC)
vgpreload_massif_amd64_linux_so_DEPENDENCIES = $(LIBREPLACEMALLOC_AMD64_LINUX)
vgpreload_massif_amd64_linux_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_AMD64_LINUX) \
	$(LIBREPLACEMALLOC_LDFLAGS_AMD64_LINUX)

vgpreload_massif_ppc32_linux_so_SOURCES      = 
vgpreload_massif_ppc32_linux_so_CPPFLAGS     = $(AM_CPPFLAGS_PPC32_LINUX)
vgpreload_massif_ppc32_linux_so_CFLAGS       = $(AM_CFLAGS_PPC32_LINUX) $(AM_CFLAGS_PIC)
vgpreload_massif_ppc32_linux_so_DEPENDENCIES = $(LIBREPLACEMALLOC_PPC32_LINUX)
vgpreload_massif_ppc32_linux_so_LDFLAGS      = \
	$(PRELOAD_LDFLAGS_PPC32_LINUX) \
	$(LIBREPLACEMALLOC_LDFLAGS_PPC32_LINUX)

MASSIF_SOURCES_COMMON = ms_main.c

massif_x86_netbsd_SOURCES      = $(MASSIF_SOURCES_COMMON)
massif_x86_netbsd_CPPFLAGS     = $(AM_CPPFLAGS_X86_NETBSD)
massif_x86_netbsd_CFLAGS       = $(AM_CFLAGS_X86_NETBSD)
massif_x86_netbsd_DEPENDENCIES = $(COREGRIND_LIBS_X86_NETBSD)
massif_x86_netbsd_LDADD        = $(TOOL_LDADD_X86_NETBSD)
massif_x86_netbsd_LDFLAGS      = $(TOOL_LDFLAGS_X86_NETBSD)

massif_x86_linux_SOURCES      = $(MASSIF_SOURCES_COMMON)
massif_x86_linux_CPPFLAGS     = $(AM_CPPFLAGS_X86_LINUX)
massif_x86_linux_CFLAGS       = $(AM_CFLAGS_X86_LINUX)
massif_x86_linux_DEPENDENCIES = $(COREGRIND_LIBS_X86_LINUX)
massif_x86_linux_LDADD        = $(TOOL_LDADD_X86_LINUX)
massif_x86_linux_LDFLAGS      = $(TOOL_LDFLAGS_X86_LINUX)

massif_amd64_linux_SOURCES      = $(MASSIF_SOURCES_COMMON)
massif_amd64_linux_CPPFLAGS     = $(AM_CPPFLAGS_AMD64_LINUX)
massif_amd64_linux_CFLAGS       = $(AM_CFLAGS_AMD64_LINUX)
massif_amd64_linux_DEPENDENCIES = $(COREGRIND_LIBS_AMD64_LINUX)
massif_amd64_linux_LDADD        = $(TOOL_LDADD_AMD64_LINUX)
massif_amd64_linux_LDFLAGS      = $(TOOL_LDFLAGS_AMD64_LINUX)

massif_ppc32_linux_SOURCES      = $(MASSIF_SOURCES_COMMON)
massif_ppc32_linux_CPPFLAGS     = $(AM_CPPFLAGS_PPC32_LINUX)
massif_ppc32_linux_CFLAGS       = $(AM_CFLAGS_PPC32_LINUX)
massif_ppc32_linux_DEPENDENCIES = $(COREGRIND_LIBS_PPC32_LINUX)
massif_ppc32_linux_LDADD        = $(TOOL_LDADD_PPC32_LINUX)
massif_ppc32_linux_LDFLAGS      = $(TOOL_LDFLAGS_PPC32_LINUX)
