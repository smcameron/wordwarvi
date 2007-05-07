
all:	wordwarvi	

wordwarvi:	wordwarvi.c
	gcc -Wall  wordwarvi.c -o wordwarvi `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0` `pkg-config --libs gthread-2.0`
