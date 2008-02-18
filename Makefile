
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

ifeq (${WITHAUDIO},yes)
all:	wordwarvi thesounds

else
all:	wordwarvi

endif

thesounds:
	( cd sounds ; make  )

wordwarvi:	wordwarvi.c
	gcc ${DEBUG} ${PROFILE_FLAG} ${OPTIMIZE_FLAG} -Wall  ${SNDFLAGS} wordwarvi.c -o wordwarvi -lm ${SNDLIBS} `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` `pkg-config --libs gthread-2.0`

tarball:
	mkdir -p d/wordwarvi-${VERSION}/sounds
	cp Makefile wordwarvi.c README AUTHORS COPYING d/wordwarvi-${VERSION}
	cp sounds/*.ogg d/wordwarvi-${VERSION}/sounds
	cp sounds/Attribution.txt d/wordwarvi-${VERSION}/sounds
	cp sounds/Makefile d/wordwarvi-${VERSION}/sounds
	chown -R root:root d;
	( cd d; tar cvf ../wordwarvi-${VERSION}.tar ./wordwarvi-${VERSION} )
	gzip wordwarvi-${VERSION}.tar

clean:
	rm -f ./wordwarvi ./wordwarvi-*.tar.gz
	rm -fr ./d
ifeq (${WITHAUDIO},yes)
	( cd sounds ; make clean )
endif
