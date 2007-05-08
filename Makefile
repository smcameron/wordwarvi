
all:	wordwarvi	

wordwarvi:	wordwarvi.c
	gcc -g -Wall  wordwarvi.c -o wordwarvi -lm -lsndfile -lportaudio `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` `pkg-config --libs gthread-2.0`
