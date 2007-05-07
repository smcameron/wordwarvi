
all:	wordwarvi	

wordwarvi:	wordwarvi.c
	gcc -Wall -g wordwarvi.c -o wordwarvi `pkg-config --cflags gtk+-2.0` `pkg-config --libs gtk+-2.0`

