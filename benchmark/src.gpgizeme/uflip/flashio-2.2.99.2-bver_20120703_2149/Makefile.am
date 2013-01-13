SUBDIRS = src

EXTRA_DIST =	\
	Doxyfile	\
	data		\
	$(NULL)

doc: Doxyfile
	$(AM_V_GEN) doxygen Doxyfile

ChangeLog: Makefile
	(GIT_DIR=$(top_srcdir)/.git git log > .changelog.tmp \
	&& mv .changelog.tmp ChangeLog; rm -f .changelog.tmp) \
	|| (touch ChangeLog; echo 'Git directory not found: installing possibly empty changelog.' >&2)

dist-hook: ChangeLog

bench-tarball: all
	$(AM_V_GEN) rm -rf tmp &&\
		mkdir -p tmp/uFLIP &&\
		cd tmp/uFLIP &&\
		cp -p ../../src/FlashIO* . &&\
		cp -p ../../data/*/* . &&\
		cd .. && zip -r uFLIP uFLIP &&\
		cp -p uFLIP.zip ../. &&\
		cd .. && rm -rf tmp

.PHONY: doc ChangeLog
