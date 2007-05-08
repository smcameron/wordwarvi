
all:	wordwarvi	

wordwarvi:	wordwarvi.c
	gcc -g -Wall  wordwarvi.c -o wordwarvi -lm -lsndfile -lportaudio `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` `pkg-config --libs gthread-2.0`

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
