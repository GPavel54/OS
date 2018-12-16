#define _GNU_SOURCE // for pthread_tryjoin_np
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <sys/time.h>
#include "bigChars.h"

#define BUFFLEN 150
#define CHATBUF 60

const char _DEFAULT_PORT[5] = "7777";
const char _CHAT_PORT[5] = "7778";

struct forThread{
  int id;
  int sockfd;
  char ip[INET6_ADDRSTRLEN];
  int maxgames;
  int ** games;
};

struct forChat{
  int sfds[3];
};

void createMenu(char * m, int ** games, int maxgames){
  strcpy(m, "0 - new game\n\0");
  //int j = 1;
  for (int i = 0; i < maxgames; i++){
    if (games[i][0] == 0)
      continue;
    int someInt = i + 1;
    char str[3];
    sprintf(str, "%d", someInt);
    strcat(m, str);
    sprintf(str, "%d", someInt - 1);
    strcat(m, " - to connect to ");
    strcat(m, str);
    strcat(m, " game\n");
  }
}

int checkForWin(const struct board sb){
  for (int i = 0; i < 9; i++)
    printf("%d ", sb.b[i]);
  printf("\n");
  if ((sb.b[0] != 0) && (sb.b[0] == sb.b[1]) && (sb.b[0]== sb.b[2]))
      return sb.b[0];
  else if ((sb.b[0] != 0) && (sb.b[0] == sb.b[4]) && (sb.b[0] == sb.b[8]))
      return sb.b[0];
  else if ((sb.b[0] != 0) && (sb.b[0] == sb.b[3]) && (sb.b[0] == sb.b[6]))
      return sb.b[0];
  else if ((sb.b[1] != 0) && (sb.b[1] == sb.b[4]) && (sb.b[1] == sb.b[7]))
      return sb.b[1];
  else if ((sb.b[2] != 0) && (sb.b[2] == sb.b[5]) && (sb.b[2]  == sb.b[8]))
      return sb.b[2];
  else if ((sb.b[2] != 0) && (sb.b[2] == sb.b[4]) && (sb.b[2] == sb.b[6]))
      return sb.b[2];
  else if ((sb.b[3] != 0) && (sb.b[3] == sb.b[4]) && (sb.b[3] == sb.b[5]))
      return sb.b[3];
  else if ((sb.b[6] != 0) && (sb.b[6] == sb.b[7]) && (sb.b[6] == sb.b[8]))
      return sb.b[6];
  else if ((sb.b[0] != 0) && (sb.b[1] != 0) && (sb.b[2] != 0) && (sb.b[3] != 0) &&
	   (sb.b[4] != 0) && (sb.b[5] != 0) && (sb.b[6] != 0) && (sb.b[7] != 0) &&
	   (sb.b[8] != 0))
    return 3;
  return 0;
}

void * get_in_addr(struct sockaddr * sa){
  if (sa->sa_family == AF_INET){
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void deleteGame(int ** games, int p){
  games[p][0] = 0;
  games[p][1] = 0;
  games[p][2] = 0;
}

int prepareSocket(char * remoteIP, const char * PORT){
  struct addrinfo hints, *ai, *p;
  int listener; 
  int yes = 1;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if (getaddrinfo(NULL, PORT, &hints, &ai) != 0) {
    printf("getaddrinfo: \n");
    return 1;
  }

  for (p = ai; p != NULL; p = p->ai_next){
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0)
      continue;

    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0){
      close(listener);
      continue;
    }
    break;
  }

  if (p == NULL){
    printf("Failed to bind.\n");
    return 1;
  }
  freeaddrinfo(ai);
  if (listen(listener, 2) == -1){
    perror("listen ");
    return 1;
  }
  return listener;
}

void * threadForChat(void * arg){
  char remoteIP[40], buffer[60];
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  sin_size = sizeof their_addr;
  struct forChat * fc = (struct forChat *) arg;
  fc->sfds[0] = prepareSocket(remoteIP, _CHAT_PORT);
  printf("Создан поток для чата\n");
  fc->sfds[1] = accept(fc->sfds[0], (struct sockaddr *)&their_addr, &sin_size);
  if (fc->sfds[1] == -1){
    perror("sockfd ");
    return NULL;
  }
  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), remoteIP, sizeof remoteIP);
  printf("Got first player %s\n", remoteIP);
  fc->sfds[2] = accept(fc->sfds[0], (struct sockaddr *)&their_addr, &sin_size);
  if (fc->sfds[2] == -1){
    perror("sockfd ");
    return NULL;
  }
  inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), remoteIP, sizeof remoteIP);
  printf("Got second player in chat thread %s\n", remoteIP);
  close(fc->sfds[0]);
  int rcvbytes;
  fd_set readfds;
  int ret;
  while (1){
    FD_ZERO(&readfds);
    FD_SET(fc->sfds[1], &readfds);
    FD_SET(fc->sfds[2], &readfds);
    ret = select(fc->sfds[2] + 1, &readfds, NULL, NULL, NULL);
    printf("Out from select\n");
    if (ret == -1){
      printf("error with select\n");
      return NULL;
    }
    if (FD_ISSET(fc->sfds[1], &readfds)){
      rcvbytes = recv(fc->sfds[1], buffer, 60, 0);
      printf("Receive message from first player.\n");
      if (rcvbytes == 0){
        printf("Player 1 lost connection\n");
        return NULL;
      }
      send(fc->sfds[2], buffer, 60, 0);
      printf("Sent message to 2 player.\n");
    }
    if (FD_ISSET(fc->sfds[2], &readfds)){
      rcvbytes = recv(fc->sfds[2], buffer, 60, 0);
      printf("Receive message from second player.\n");
      if (rcvbytes == 0){
        printf("Player 2 lost connection\n");
        return NULL;
      }
      send(fc->sfds[1], buffer, 60, 0);
      printf("Sent message to 1 player.\n");
    }
  }
}

void * threadForUser(void * arg){
  char menu[BUFFLEN], again[BUFFLEN] = "again\n";
  char sendOk[BUFFLEN] = "ok\n";
  char secondP[BUFFLEN] = "You are the second player. Waiting for first player move.\n";
  char nextTime[BUFFLEN] = "Cannot create more games. Try later.";
  int answer, check = 0, rcvbytes;
  struct forThread * myInfo = (struct forThread *)arg;
  while (check == 0){
    createMenu(menu, myInfo->games, myInfo->maxgames);
    printf("Send menu to %s\n", myInfo->ip);
    send(myInfo->sockfd, menu, BUFFLEN, 0);
    
    rcvbytes = recv(myInfo->sockfd, &answer, sizeof(int), 0);
    if (rcvbytes == 0){
      printf("client down..\n");
      return NULL;
    }
    printf("Receive answer %d from %d\n", answer, myInfo->sockfd);
    if (answer == 0)
      break;
    if (answer > myInfo->maxgames){
      send(myInfo->sockfd, again, BUFFLEN, 0);
      continue;
    }
    if (myInfo->games[answer - 1][0] == 0){
      send(myInfo->sockfd, again, BUFFLEN, 0);
      continue;
    }
    if (myInfo->games[answer - 1][1] != 0 && myInfo->games[answer - 1][2] == 0){
      myInfo->games[answer - 1][2] = myInfo->sockfd;
      send(myInfo->sockfd, sendOk, BUFFLEN, 0);
      return NULL;
    }
  }
  int i = 0;
  for (; i < myInfo->maxgames; i++)
    if (myInfo->games[i][0] == 0)
      break;
  if (i == myInfo->maxgames){
    send(myInfo->sockfd, nextTime, BUFFLEN, 0);
    return NULL;
  }
  send(myInfo->sockfd, sendOk, BUFFLEN, 0);
  myInfo->games[i][0] = 1;
  myInfo->games[i][1] = myInfo->sockfd;
  pthread_t thrd;
  struct forChat fc = {{0, 0, 0}};
  pthread_create(&thrd, NULL, threadForChat, (void *)&fc);
  char first[BUFFLEN] = "You are the first player. Waiting 60 seconds for second.\n";
  send(myInfo->sockfd, first, BUFFLEN, 0);
  struct timeval tv1, tv2;
  gettimeofday(&tv1, NULL);
  gettimeofday(&tv2, NULL);
  int connected = 0;
  while (tv2.tv_sec - tv1.tv_sec < 60){
    if (myInfo->games[i][2] != 0){
      connected = 1;
      break;
    }
    gettimeofday(&tv2, NULL);
  }
  if (connected == 0){
    close(myInfo->sockfd);
    return NULL;
  } else if (connected == 1){
    printf("Ok, let's start!\n");
  }
  struct board b = {{0}};
  send(myInfo->games[i][2], secondP, BUFFLEN, 0);
  int win = 0;
    while (win == 0){
    send(myInfo->games[i][1], &b, sizeof(struct board), 0);
    printf("Send board to 1 player\n");
    rcvbytes = recv(myInfo->games[i][1], &b, sizeof(struct board), 0);
    if (rcvbytes == 0){
      printf("First player lost connection\n");
      close(myInfo->games[i][1]);
      close(myInfo->games[i][2]);
      deleteGame(myInfo->games, i);
      return NULL;
    }
    printf("Recieve board from 1 player\n");
    win = checkForWin(b);
    if (win != 0){
      b.b[9] = win;
      send(myInfo->games[i][2], &b, sizeof(struct board), 0);
      send(myInfo->games[i][1], &b, sizeof(struct board), 0);
      break;
    }
    send(myInfo->games[i][2], &b, sizeof(struct board), 0);
    printf("Send board to 2 player\n");
    rcvbytes = recv(myInfo->games[i][2], &b, sizeof(struct board), 0);
    if (rcvbytes == 0){
      printf("Second player lost connection\n");
      close(myInfo->games[i][1]);
      close(myInfo->games[i][2]);
      deleteGame(myInfo->games, i);
      return NULL;
    }
    printf("Receive board from 2 player\n");
    win = checkForWin(b);
    if (win != 0){
      b.b[9] = win;
      send(myInfo->games[i][1], &b, sizeof(struct board), 0);
      send(myInfo->games[i][2], &b, sizeof(struct board), 0);
      break;
    }
  }
  close(fc.sfds[1]);
  close(fc.sfds[2]);
  deleteGame(myInfo->games, i);
  printf("Game over.");
  return NULL;
}

int main(int argc, char ** argv){
  if (argc != 2){
    printf("To start game use: %s number of games.\n", argv[0]);
    return 1;
  }
  long maxgames = (strtol(argv[1], NULL, 10));
  int listener, sockfd;
  char remoteIP[INET6_ADDRSTRLEN];
  struct board * b;
  b = malloc(maxgames * sizeof(struct board));
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  sin_size = sizeof their_addr;
  struct forThread * ff = NULL; 
  int id = 1;
  pthread_t thread[10];
  int arr_of_threads[10] = {0};
  int ** games;
  games = malloc(sizeof(int *) * maxgames);
  for (int i = 0; i < maxgames; i++)
    games[i] = malloc(3 * sizeof(int));
  for (int i = 0; i < maxgames; i++){
    games[i][0] = 0;
    games[i][1] = 0;
    games[i][2] = 0;
  }
  listener = prepareSocket(remoteIP, _DEFAULT_PORT);
  for (;;){
    sockfd = accept(listener, (struct sockaddr *)&their_addr, &sin_size);
    if (sockfd == -1){
      perror("sockfd ");
      return 1;
    }
    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr*)&their_addr), remoteIP, sizeof remoteIP);
    ff =realloc(ff, sizeof(struct forThread) * id);
    ff[id - 1].id = id;
    ff[id - 1].sockfd = sockfd;
    ff[id - 1].maxgames = maxgames;
    ff[id - 1].games = games;
    strcpy(ff[id -1].ip, remoteIP);
    pthread_create(thread + id - 1, NULL, threadForUser, (void *)&ff[id - 1]); //возвращаемое значение не проверяется
    arr_of_threads[id -1] = 1;
    id++;
    for (int j = 0; j < id; j++){
      if (arr_of_threads[j] == 1){
	      int err = 0;
	      err = pthread_tryjoin_np(*(thread + j), NULL);
	      if (err == 0){
	        arr_of_threads[j] = 0;
	        printf("Поток %d закончил работу\n", j);
	      }
      }
    }
  }
  free(b);
  free(games);
  free(ff);
  return 0;
}
