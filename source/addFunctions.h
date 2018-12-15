#ifndef ADDFUNC_H
#define ADDFUNC_H
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/time.h>
#include <stdlib.h>
#include "bigChars.h"

struct cursor{
	int p;
	int x;
	int y;
	};
struct board{
	int b[10];
};
struct ft{
  struct board * b;
  int sockfd;
  int * turn;
  pthread_mutex_t * mute;
};

int retPos(int p, int * x, int * y);
int printBoard(struct board sb);
void printCursor(struct cursor c, struct board sb);
void printSym(const struct cursor c, int p1, struct board * sb);
int checkForWin(const struct board sb);
void nullBoard(struct board * sb);
void * get_in_addr(struct sockaddr * sa);
int connectToServer(char * const arg, char * s, const char * PORT);
void * recvBoard(void *);

#endif
