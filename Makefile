PREFIX=/usr
DATADIR=${PREFIX}/share/wordwarvi
MANDIR?=${PREFIX}/share/man
MANPAGEDIR=${MANDIR}/man6

# To compile withaudio, WITHAUDIO=yes, 
# for no audio support, change to WITHAUDIO=no, 
WITHAUDIO=yes
# WITHAUDIO=no

ifeq (${WITHAUDIO},yes)
SNDLIBS=`pkg-config --libs portaudio-2.0 vorbisfile`
SNDFLAGS=-DWITHAUDIOSUPPORT `pkg-config --cflags portaudio-2.0`
OGGOBJ=ogg_to_pcm.o
else
SNDLIBS=
SNDFLAGS=
OGGOBJ=
endif

# DEBUG=-g
# DEBUG=
# PROFILE_FLAG=-pg
#PROFILE_FLAG=
#OPTIMIZE_FLAG=
# OPTIMIZE_FLAG=-O3
OPTIMIZE_FLAG=-O3

LDFLAGS=${PROFILE_FLAG}

DEFINES=${SNDFLAGS} -DDATADIR=\"${DATADIR}/\"

all:	wordwarvi wordwarvi.6.gz

HAS_PORTAUDIO_2_0:
ifeq (${WITHAUDIO},yes)
	pkg-config --print-errors --exists portaudio-2.0
else
endif

HAS_VORBISFILE:
ifeq (${WITHAUDIO},yes)
	pkg-config --print-errors --exists vorbisfile
else
endif

joystick.o:	joystick.c joystick.h Makefile
	gcc ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} -pthread -Wall -c joystick.c

ogg_to_pcm.o:	ogg_to_pcm.c ogg_to_pcm.h Makefile
	gcc ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} `pkg-config --cflags vorbisfile` \
		-pthread -Wall -c ogg_to_pcm.c

stamp:	stamp.c
	gcc -o stamp stamp.c	

wordwarvi:	wordwarvi.c joystick.o ${OGGOBJ} Makefile version.h stamp levels.h
	./stamp > stamp.h
	gcc ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} -pthread -Wall  ${DEFINES} \
		joystick.o \
		${OGGOBJ} \
		wordwarvi.c -o wordwarvi -lm ${SNDLIBS} \
		`pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0 gthread-2.0`
	/bin/rm stamp.h

wordwarvi.6.gz:	wordwarvi.6
	gzip -c wordwarvi.6 > wordwarvi.6.gz

install: wordwarvi wordwarvi.6.gz
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(DATADIR)/sounds
	mkdir -p $(DESTDIR)$(MANPAGEDIR)
	install -p -m 755 wordwarvi $(DESTDIR)$(PREFIX)/bin
	install -p -m 644 sounds/*.ogg $(DESTDIR)$(DATADIR)/sounds
	install -p -m 644 wordwarvi.6.gz $(DESTDIR)$(MANPAGEDIR)

uninstall:
	/bin/rm -f $(DESTDIR)${PREFIX}/bin/wordwarvi
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
	cp Makefile version.h ogg_to_pcm.c ogg_to_pcm.h levels.h \
		joystick.c joystick.h changelog.txt wordwarvi.c wordwarvi.6 \
		stamp.c README AUTHORS COPYING \
		AAA_HOW_TO_MAKE_NEW_LEVELS.txt \
		changelog.txt \
		d/wordwarvi-${VERSION}
	cp sounds/*.ogg d/wordwarvi-${VERSION}/sounds
	cp sounds/Attribution.txt d/wordwarvi-${VERSION}/sounds
	chown -R root:root d;
	( cd d; tar cvf ../wordwarvi-${VERSION}.tar ./wordwarvi-${VERSION} )
	gzip wordwarvi-${VERSION}.tar

clean:
	rm -f ./wordwarvi ./wordwarvi-*.tar.gz wordwarvi.6.gz stamp.h stamp
	rm -fr ./d
