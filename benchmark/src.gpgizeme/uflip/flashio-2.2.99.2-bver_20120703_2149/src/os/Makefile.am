if WIN32_BUILD
noinst_LIBRARIES = libuflipwin32.a
else
if LINUX_BUILD
noinst_LIBRARIES = libufliplinux.a
else
noinst_LIBRARIES = libuflipdummy.a
endif
endif

libuflipwin32_a_SOURCES =	\
	functions.h				\
	win32.c					\
	$(NULL)

libufliplinux_a_SOURCES =	\
	functions.h				\
	linux.c					\
	posix_timer.c			\
	$(NULL)

libuflipdummy_a_SOURCES =       \
	functions.h                             \
	dummy.c                                 \
	posix_timer.c                   \
	$(NULL)

INCLUDES = -I@top_srcdir@/src
