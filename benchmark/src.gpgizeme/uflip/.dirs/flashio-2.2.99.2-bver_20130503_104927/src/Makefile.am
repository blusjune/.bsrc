SUBDIRS = os

bin_PROGRAMS = FlashIO

FlashIO_SOURCES =	\
	main.c			\
	blocAlloc.c		\
	blocAlloc.h		\
	genBench.c		\
	genBench.h		\
	input.c			\
	input.h			\
	microbench.c	\
	microbench.h	\
	output.c		\
	output.h		\
	rndFormat.c		\
	rndFormat.h		\
	seqFormat.c		\
	seqFormat.h		\
	uflip_device.c	\
	uflip_device.h	\
	uflip_params.c	\
	uflip_params.h	\
	uflip_random.c	\
	uflip_random.h	\
	uflip_results.c	\
	uflip_results.h	\
	uflip_timer.c	\
	uflip_timer.h	\
	$(NULL)

if WIN32_BUILD
FlashIO_LDADD = os/libuflipwin32.a
else
if LINUX_BUILD
FlashIO_LDADD = os/libufliplinux.a
else
FlashIO_LDADD = os/libuflipdummy.a
endif
endif
