##---------------------------------------------------------------------------
## These test core error checking, eg. "silly values" for malloc/calloc,
## pthread errors (and suppressions), signal handling errors, invalid fds for
## blocking syscalls, etc.
##---------------------------------------------------------------------------

noinst_SCRIPTS = filter_stderr filter_as_mmap filter_fdleak

EXTRA_DIST = $(noinst_SCRIPTS) \
	as_mmap.stderr.exp as_mmap.stderr.exp2 as_mmap.vgtest \
	as_shm.stderr.exp as_shm.stdout.exp as_shm.vgtest \
	erringfds.stderr.exp erringfds.stdout.exp erringfds.vgtest \
	fdleak_cmsg.stderr.exp fdleak_cmsg.stderr.exp2 fdleak_cmsg.vgtest \
	fdleak_creat.stderr.exp fdleak_creat.stderr.exp2 \
	fdleak_creat.stderr.exp3 fdleak_creat.vgtest \
	fdleak_dup.stderr.exp fdleak_dup.stderr.exp2 \
	fdleak_dup.stderr.exp3 fdleak_dup.vgtest \
	fdleak_dup2.stderr.exp fdleak_dup2.stderr.exp2 \
	fdleak_dup2.stderr.exp3 fdleak_dup2.vgtest \
	fdleak_fcntl.stderr.exp fdleak_fcntl.stderr.exp2 \
	fdleak_fcntl.stderr.exp3 fdleak_fcntl.stderr.exp4 \
	fdleak_fcntl.vgtest \
	fdleak_ipv4.stderr.exp fdleak_ipv4.stderr.exp2 \
	fdleak_ipv4.stdout.exp fdleak_ipv4.vgtest \
	fdleak_open.stderr.exp fdleak_open.stderr.exp2 fdleak_open.vgtest \
	fdleak_pipe.stderr.exp fdleak_pipe.stderr.exp2 fdleak_pipe.vgtest \
	fdleak_socketpair.stderr.exp fdleak_socketpair.stderr.exp2 \
	fdleak_socketpair.vgtest \
	pth_atfork1.stderr.exp pth_atfork1.stdout.exp pth_atfork1.vgtest \
	pth_cancel1.stderr.exp pth_cancel1.stdout.exp pth_cancel1.vgtest \
	pth_cancel2.stderr.exp pth_cancel2.vgtest \
	pth_cvsimple.stderr.exp pth_cvsimple.stdout.exp pth_cvsimple.vgtest \
	pth_empty.stderr.exp pth_empty.vgtest \
	pth_exit.stderr.exp pth_exit.vgtest \
	pth_exit2.stderr.exp pth_exit2.vgtest \
	pth_mutexspeed.stderr.exp \
	pth_mutexspeed.stdout.exp pth_mutexspeed.vgtest \
	pth_once.stderr.exp pth_once.stdout.exp pth_once.vgtest \
	pth_rwlock.stderr.exp pth_rwlock.vgtest \
	sigkill.stderr.exp sigkill.stderr.exp2 sigkill.vgtest \
	stack_changes.vgtest \
	res_search.stderr.exp res_search.stdout.exp res_search.vgtest \
	threadederrno.stderr.exp threadederrno.stdout.exp \
	threadederrno.vgtest \
	vgprintf.stderr.exp vgprintf.stdout.exp vgprintf.vgtest

check_PROGRAMS = \
	erringfds fdleak_cmsg fdleak_creat fdleak_dup fdleak_dup2 \
	fdleak_fcntl fdleak_ipv4 fdleak_open fdleak_pipe \
	fdleak_socketpair sigkill res_search \
	pth_atfork1 pth_cancel1 pth_cancel2 pth_cvsimple pth_empty \
	pth_exit pth_exit2 pth_mutexspeed pth_once pth_rwlock \
	as_mmap as_shm threadederrno \
	stack_changes vgprintf

AM_CFLAGS   = $(WERROR) -Winline -Wall -Wshadow -g -O0
AM_CPPFLAGS = -I$(top_builddir)/include
AM_CXXFLAGS = $(AM_CFLAGS)

vgprintf_SOURCES	= vgprintf.c

# Client address space checks
as_mmap_SOURCES		= as_mmap.c
as_shm_SOURCES		= as_shm.c

# Leak tests
fdleak_cmsg_SOURCES	= fdleak_cmsg.c
fdleak_creat_SOURCES	= fdleak_creat.c
fdleak_dup_SOURCES	= fdleak_dup.c
fdleak_dup2_SOURCES	= fdleak_dup2.c
fdleak_fcntl_SOURCES	= fdleak_fcntl.c
fdleak_ipv4_SOURCES	= fdleak_ipv4.c
fdleak_open_SOURCES	= fdleak_open.c
fdleak_pipe_SOURCES	= fdleak_pipe.c
fdleak_socketpair_SOURCES = fdleak_socketpair.c
sigkill_SOURCES		= sigkill.c

# Pthread ones
pth_atfork1_SOURCES	= pth_atfork1.c
pth_atfork1_LDADD	= -lpthread
pth_cancel1_SOURCES	= pth_cancel1.c
pth_cancel1_LDADD	= -lpthread
pth_cancel2_SOURCES	= pth_cancel2.c
pth_cancel2_LDADD	= -lpthread
pth_cvsimple_SOURCES	= pth_cvsimple.c
pth_cvsimple_LDADD	= -lpthread
pth_empty_SOURCES 	= pth_empty.c
pth_empty_LDADD 	= -lpthread
pth_exit_SOURCES 	= pth_exit.c
pth_exit_LDADD	 	= -lpthread
pth_exit2_SOURCES 	= pth_exit2.c
pth_exit2_LDADD	 	= -lpthread
pth_mutexspeed_SOURCES	= pth_mutexspeed.c
pth_mutexspeed_LDADD	= -lpthread
pth_once_SOURCES	= pth_once.c
pth_once_LDADD		= -lpthread
pth_rwlock_SOURCES	= pth_rwlock.c
pth_rwlock_LDADD	= -lpthread
res_search_SOURCES      = res_search.c
res_search_LDADD        = -lresolv -lpthread
threadederrno_SOURCES	= threadederrno.c
threadederrno_LDADD	= -lpthread

# Stack tests
stack_changes_SOURCES   = stack_changes.c
