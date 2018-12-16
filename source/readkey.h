#ifndef KEYBOARD_H
#define KEYBOARD_H
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

enum keys {
	KEY_f5,
	KEY_f6,
	KEY_up,
	KEY_down,
	KEY_left,
	KEY_right,
	KEY_enter,
	KEY_another,
	KEY_t
};

int rk_readkey(int * key);
int rk_mytermsave();
int rk_mytermrestore();
int rk_mytermregime(int regime, int vtime, int vmin, int echo, int sigint);
#endif
