#include "addFunctions.h"
int retPos(int p, int * x, int * y){
	if (p > 8 || p < 0)
		return -1;
	switch (p){
		case 0:
		*x = 2;
		*y = 2;
		break;
		case 1:
		*x = 11;
		*y = 2;
		break;
		case 2:
		*x = 20;
		*y = 2;
		break;
		case 3:
		*x = 2;
		*y = 11;
		break;
		case 4:
		*x = 11;
		*y = 11;
		break;
		case 5:
		*x = 20;
		*y = 11;
		break;
		case 6:
		*x = 2;
		*y = 20;
		break;
		case 7:
		*x = 11;
		*y = 20;
		break;
		case 8:
		*x = 20;
		*y = 20;
		break;
		}
		return 0;
}

int printBoard(struct board sb){
	int bcintX [2] = {405029505, 2168595480};
	int bcintO [2] = {2172748158, 2122416513};
	mt_gotoXY(1,1);
	char u[2] = {BOXCHAR_REC, '\0'};
	for (int i = 0; i < 28; i++){
		for (int j = 0; j < 28; j++){
			if (i == 0 || i == 9 || i == 18 || i == 27)
				u[0] = BOXCHAR_REC;
			else if (j == 0 || j == 9 || j == 18 || j == 27)
				u[0] = BOXCHAR_REC;
			else
				u[0] = ' ';
			mt_setfgcolor(cl_green);
			bc_printA(u);
			mt_setfgcolor(cl_default);
		}
		printf("\n");
	}
	int x = 1, y = 1;
	for (int i = 0; i < 9; i++){
		retPos(i, &x, &y);
	        if (sb.b[i] == 1)
			bc_printbigchar(bcintX, x, y, cl_red, cl_default);
		else if (sb.b[i] == 2)
			bc_printbigchar(bcintO, x, y, cl_red, cl_default);
	}
	return 0;
}

void printCursor(struct cursor c, struct board sb){
	int bcintC [2] = {605552640, 6180};
	mt_gotoXY(1,1);
	retPos(c.p, &(c.x), &(c.y));
	mt_gotoXY(22,22);
	bc_printcourse(bcintC, c.p, cl_cyan, cl_default);
}

void printSym(const struct cursor c, int p1, struct board * sb){
  if (sb->b[c.p] == 0){
    sb->b[c.p] = p1;
  }
}

int checkForWin(const struct board sb){
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

void nullBoard(struct board * sb){
  for (int i = 0; i < 9; i++)
    sb->b[i] = 0;
}

void * get_in_addr(struct sockaddr * sa){
  if (sa->sa_family == AF_INET){
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int connectToServer(char * arg, char * s, const char * PORT){
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(arg, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and connect to the first we can
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
			 p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }

    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      perror("client: connect");
      close(sockfd);
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, INET6_ADDRSTRLEN);
  printf("client: connecting to %s\n", s);
  freeaddrinfo(servinfo); // all done with this structure
  return sockfd;
}

void * recvBoard(void * p){
  struct ft * inf = (struct ft *) p;
  while (inf->b->b[9] == 0){
    int rcvbytes = recv(inf->sockfd, inf->b, sizeof(struct board), 0);
    if (rcvbytes == 0){
      printf("Server down...");
      exit(EXIT_FAILURE);
    }
    pthread_mutex_lock(inf->mute);
    *(inf->turn) = 1;
    pthread_mutex_unlock(inf->mute);
    while (*(inf->turn) != 2){
      sleep(1);
    }
    send(inf->sockfd, inf->b, sizeof(struct board), 0);
    *(inf->turn) = 0;
  }
  printf("End of thread\n");
  return NULL;
}
