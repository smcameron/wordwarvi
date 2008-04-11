PREFIX=/usr
DATADIR=${PREFIX}/share/wordwarvi
MANPAGEDIR=${PREFIX}/share/man/man6

# To compile withaudio, WITHAUDIO=yes, 
# for no audio support, change to WITHAUDIO=no, 
WITHAUDIO=yes
# WITHAUDIO=no

ifeq (${WITHAUDIO},yes)
SNDLIBS=-lsndfile -lportaudio
SNDFLAGS=-DWITHAUDIOSUPPORT
else
SNDLIBS=
SNDFLAGS=
endif

#DEBUG=-g
# DEBUG=
# PROFILE_FLAG=-pg
#PROFILE_FLAG=
#OPTIMIZE_FLAG=
OPTIMIZE_FLAG=-O2

LDFLAGS=${PROFILE_FLAG}

DEFINES=${SNDFLAGS} -DDATADIR=\"${DATADIR}/\"

ifeq (${WITHAUDIO},yes)
all:	wordwarvi thesounds wordwarvi.6.gz

else
all:	wordwarvi

endif

joystick.o:	joystick.c joystick.h Makefile
	gcc ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} -Wall -c joystick.c

thesounds:
	( cd sounds ; make  )

wordwarvi:	wordwarvi.c joystick.o Makefile version.h
	gcc ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} -Wall  ${DEFINES} joystick.o \
		wordwarvi.c -o wordwarvi -lm ${SNDLIBS} \
		`pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` `pkg-config --libs gthread-2.0`

wordwarvi.6.gz:	wordwarvi.6
	gzip -c wordwarvi.6 > wordwarvi.6.gz

install: wordwarvi wordwarvi.6.gz
	mkdir -p $(PREFIX)/bin
	mkdir -p $(DATADIR)/sounds
	install -p -m 755 wordwarvi $(PREFIX)/bin
	install -p -m 644 sounds/*.wav $(DATADIR)/sounds
	install -p -m 644 wordwarvi.6.gz $(MANPAGEDIR)

uninstall:
	/bin/rm -f ${PREFIX}/bin/wordwarvi
	/bin/rm -fr ${DATADIR}
	/bin/rm -f ${MANPAGEDIR}/wordwarvi.6.gz

tarball:
	mkdir -p d/wordwarvi-${VERSION}/sounds
	cp Makefile version.h joystick.c joystick.h changelog.txt wordwarvi.c wordwarvi.6 \
		README AUTHORS COPYING d/wordwarvi-${VERSION}
	cp sounds/*.ogg d/wordwarvi-${VERSION}/sounds
	cp sounds/Attribution.txt d/wordwarvi-${VERSION}/sounds
	cp sounds/Makefile d/wordwarvi-${VERSION}/sounds
	chown -R root:root d;
	( cd d; tar cvf ../wordwarvi-${VERSION}.tar ./wordwarvi-${VERSION} )
	gzip wordwarvi-${VERSION}.tar

clean:
	rm -f ./wordwarvi ./wordwarvi-*.tar.gz wordwarvi.6.gz
	rm -fr ./d
ifeq (${WITHAUDIO},yes)
	( cd sounds ; make clean )
endif
