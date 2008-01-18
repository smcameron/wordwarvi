
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

ifeq (${WITHAUDIO},yes)
all:	wordwarvi thesounds

else
all:	wordwarvi

endif

thesounds:
	( cd sounds ; make  )

wordwarvi:	wordwarvi.c
	gcc -g -Wall  ${SNDFLAGS} wordwarvi.c -o wordwarvi -lm ${SNDLIBS} `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` `pkg-config --libs gthread-2.0`

tarball:
	mkdir -p d/wordwarvi/sounds
	cp Makefile wordwarvi.c AUTHORS COPYING d/wordwarvi
	cp sounds/*.wav d/wordwarvi/sounds
	cp sounds/Attribution.txt d/wordwarvi/sounds
	( cd d; tar cvf ../wordwarvi.tar ./wordwarvi )
	gzip wordwarvi.tar

clean:
	rm -f ./wordwarvi ./wordwarvi.tar.gz
	rm -fr ./d
ifeq (${WITHAUDIO},yes)
	( cd sounds ; make clean )
endif
