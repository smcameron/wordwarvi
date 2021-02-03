PREFIX?=/usr
BINDIR?=${PREFIX}/games
DATADIR=${PREFIX}/share/wordwarvi
MANDIR?=${PREFIX}/share/man
MANPAGEDIR=${MANDIR}/man6
PKG_CONFIG?=pkg-config

SCREENSAVERFLAG=
#SCREENSAVERFLAG=-DDO_INHIBIT_SCREENSAVER

# To compile withaudio, WITHAUDIO=yes, 
# for no audio support, change to WITHAUDIO=no, 
WITHAUDIO=yes
# WITHAUDIO=no

ifeq (${WITHAUDIO},yes)
SNDLIBS=`$(PKG_CONFIG) --libs portaudio-2.0 vorbisfile`
SNDFLAGS=-DWITHAUDIOSUPPORT `$(PKG_CONFIG) --cflags portaudio-2.0`
OGGOBJ=ogg_to_pcm.o
else
SNDLIBS=
SNDFLAGS=-DWWVIAUDIO_STUBS_ONLY
OGGOBJ=
endif

# "make OPENLASE=1" to compile support for openlase laser projector
# NOTE: this will cut a lot of standard stuff out of wordwarvi
# for performance reasons, and cut the frame rate to 20 fps,
# as openlase rendering is slow
#
# "make OPENLASE=1 OPENLASEGRAYSCALE=1" to compile for openlase
# but monochromatic, in case you don't have an RGB capable
# laser projector.
#
ifeq (${OPENLASE},1)
ifeq (${OPENLASEGRAYSCALE},1)
OPENLASECFLAG=-DOPENLASE -DOPENLASEGRAYSCALE
else
OPENLASECFLAG=-DOPENLASE
endif
OPENLASELIB=-lopenlase
OPENLASELIBDIR=-L.
else
OPENLASELIB=
OPENLASECFLAG=
OPENLASELIBDIR=
endif

CC ?= gcc
BUILD_CC ?= ${CC}

# DEBUG=-g
# DEBUG=
# PROFILE_FLAG=-pg
#PROFILE_FLAG=
#OPTIMIZE_FLAG=
# OPTIMIZE_FLAG=-O3
#OPTIMIZE_FLAG=-O3 -pedantic -D_FORTIFY_SOURCE=2 -Wformat -Wformat-security
CFLAGS ?= -O3 -pedantic
OPTIMIZE_FLAG = ${CFLAGS} ${CPPFLAGS}
WARNFLAG=-pedantic -W -Wall

LDFLAGS += ${PROFILE_FLAG}

DEFINES=${SNDFLAGS} -DDATADIR=\"${DATADIR}/\"

all:	wordwarvi wordwarvi.6.gz

HAS_PORTAUDIO_2_0:
ifeq (${WITHAUDIO},yes)
	$(PKG_CONFIG) --print-errors --exists portaudio-2.0
else
endif

HAS_VORBISFILE:
ifeq (${WITHAUDIO},yes)
	$(PKG_CONFIG) --print-errors --exists vorbisfile
else
endif

joystick.o:	joystick.c joystick.h Makefile
	$(CC) ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} -pthread ${WARNFLAG} -c joystick.c

ogg_to_pcm.o:	ogg_to_pcm.c ogg_to_pcm.h Makefile
	$(CC) ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} `$(PKG_CONFIG) --cflags vorbisfile` \
		-pthread ${WARNFLAG} -c ogg_to_pcm.c

wwviaudio.o:	wwviaudio.c wwviaudio.h ogg_to_pcm.h my_point.h Makefile
	$(CC) ${WARNFLAG} ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} \
		${DEFINES} \
		-pthread `$(PKG_CONFIG) --cflags vorbisfile` \
		-c wwviaudio.c

rumble.o:	rumble.c rumble.h Makefile
	$(CC) ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} `$(PKG_CONFIG) --cflags vorbisfile` \
		-pthread ${WARNFLAG} -c rumble.c

wwvi_font.o:	wwvi_font.c wwvi_font.h my_point.h Makefile
	$(CC) ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} -pthread ${WARNFLAG} -c wwvi_font.c

stamp:	stamp.c
	$(BUILD_CC) -o stamp stamp.c

wordwarvi:	wordwarvi.c joystick.o rumble.o ${OGGOBJ} wwviaudio.o wwvi_font.o \
		Makefile version.h stamp levels.h rumble.h
	./stamp > stamp.h
	$(CC) ${DEBUG} ${OPENLASECFLAG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} ${SCREENSAVERFLAG} -pthread ${WARNFLAG}  ${DEFINES} \
		joystick.o \
		rumble.o \
		wwvi_font.o \
		${OGGOBJ} \
		wwviaudio.o \
		wordwarvi.c -o wordwarvi ${OPENLASELIBDIR} ${OPENLASELIB} -lm ${SNDLIBS} \
		`$(PKG_CONFIG) --cflags gtk+-2.0` `$(PKG_CONFIG) --libs gtk+-2.0 gthread-2.0` ${LDFLAGS}
	/bin/rm stamp.h

wordwarvi.6.gz:	wordwarvi.6
	gzip -c wordwarvi.6 > wordwarvi.6.gz

install: wordwarvi wordwarvi.6.gz
	mkdir -p $(DESTDIR)$(BINDIR)
	mkdir -p $(DESTDIR)$(DATADIR)/sounds
	mkdir -p $(DESTDIR)$(MANPAGEDIR)
	install -p -m 755 wordwarvi $(DESTDIR)$(BINDIR)
	install -p -m 644 sounds/*.ogg $(DESTDIR)$(DATADIR)/sounds
	install -p -m 644 wordwarvi.6.gz $(DESTDIR)$(MANPAGEDIR)

uninstall:
	/bin/rm -f $(DESTDIR)$(BINDIR)/wordwarvi
	/bin/rm -fr $(DESTDIR)${DATADIR}
	/bin/rm -f $(DESTDIR)${MANPAGEDIR}/wordwarvi.6.gz

CHECK_VERSION:
	@echo Checking VERSION string ${VERSION}... 1>&2
	@echo ${VERSION} | grep '[0-9][0-9]*[.][0-9][0-9]*' > /dev/null 2>&1
	@echo VERSION=${VERSION} which looks ok. 1>&2
	@echo Checking that ${VERSION} matches what is in version.h 1>&2
	@grep ${VERSION} version.h > /dev/null 2>&1
	@echo VERSION ${VERSION} matches cursory check of version.h 1>&2

tarball:	CHECK_VERSION
	mkdir -p d/wordwarvi-${VERSION}/sounds
	cp Makefile version.h ogg_to_pcm.c ogg_to_pcm.h levels.h rumble.c rumble.h \
		joystick.c joystick.h changelog.txt wordwarvi.c wordwarvi.6 \
		wwviaudio.c wwviaudio.h my_point.h wwvi_font.h wwvi_font.c \
		stamp.c README AUTHORS COPYING \
		AAA_HOW_TO_MAKE_NEW_LEVELS.txt \
		changelog.txt wordwarvi_hacking.html bigrocket.png \
		d/wordwarvi-${VERSION}
	cp sounds/*.ogg d/wordwarvi-${VERSION}/sounds
	cp sounds/Attribution.txt d/wordwarvi-${VERSION}/sounds
	mkdir -p d/wordwarvi-${VERSION}/icons
	cp icons/*.png icons/*.xcf d/wordwarvi-${VERSION}/icons
	chown -R root:root d;
	( cd d; tar cvf ../wordwarvi-${VERSION}.tar ./wordwarvi-${VERSION} )
	gzip wordwarvi-${VERSION}.tar

scan-build:
	make clean
	scan-build -o /tmp/wordwarvi-scan-build-output make CC=clang
	xdg-open /tmp/wordwarvi-scan-build-output/*/index.html

clean:
	rm -f ./wordwarvi ./wordwarvi-*.tar.gz wordwarvi.6.gz stamp.h stamp
	rm -f ./joystick.o  ./ogg_to_pcm.o  ./rumble.o  ./wwviaudio.o ./wwvi_font.o
	rm -fr ./d
	rm -fr /tmp/wordwarvi-scan-build-output
