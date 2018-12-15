#include "bigChars.h"
#include "terminal.h"
#include "readkey.h"

#define BUFFLEN 150
const char _DEFAULT_PORT[5] = "7777";
const char _CHAT_PORT[5] = "7778";


int main(int argc, char ** argv){	
	if (argc != 2){
	  printf("To start game use: %s hostname\n", argv[0]);
	  return 1;
	}
        char s[INET6_ADDRSTRLEN];
	int sockfd = connectToServer(argv[1], s, _DEFAULT_PORT);
	char buffer[BUFFLEN];
	int ans = -1;
	int rcvbytes = 0;

	//menu part
	while (1){
	  rcvbytes = recv(sockfd, &buffer, BUFFLEN, 0);
	  if (rcvbytes < 0){
	    printf("Error with response from server\n");
	    return 0;
	  }
	  if (rcvbytes == 0){
	    printf("Server down...\n");
	    return 0;
	  }
	  if (strstr(buffer, "new game") != NULL){
	    printf("%s\n", buffer);
	  }
	  if (strstr(buffer, "ok") != NULL){
	    printf("Server return:\n%s\n", buffer);
	    break;
	  }
	  if (strstr(buffer, "create more") != NULL){
	    printf("Server return:\n%s\n", buffer);
	    return 0;
	  }
	  if (strstr(buffer, "again") != NULL){
	    printf("Server return:\n%s\n", buffer);
	    rcvbytes = recv(sockfd, &buffer, BUFFLEN, 0);
	    if (rcvbytes == 0){
	      printf("Server down...\n");
	      return 0;
	    }
	    printf("%s", buffer);
	  }
	  ans = -1;
	  do {
	    printf("Type answer:");
	    scanf("%d", &ans);
	    if (ans < 0)
	      ans = -1;
	    char c;
	    do {
	      c = getchar();
	    } while (c != '\n' && c != EOF);
	  } while (ans == -1);
	  send(sockfd, &ans, sizeof(int), 0);
	}
	//******
	//after create or connect to game
	int iam;
        rcvbytes = recv(sockfd, buffer, 150, 0);
        if (rcvbytes == 0){
	    printf("Server down...\n");
	    return 0;
	}
	printf("%s", buffer);
	if (strstr(buffer, "60") != NULL)
	  iam = 1;
	else
	  iam = 2;
        struct board sb = {{0}}, myb = {{0}};
        mt_clrscr();
	int a;
	char answer = '\0';
	struct cursor cursor;
	cursor.p = 0;
	int myturn = 3;
	retPos(cursor.p, &(cursor.x), &(cursor.y));
	pthread_t thread;
	pthread_mutex_t mt = PTHREAD_MUTEX_INITIALIZER;
	struct ft ft;
	ft.b = &sb;
	ft.sockfd = sockfd;
	ft.turn = &myturn;
	ft.mute = &mt;
	pthread_create(&thread, NULL, recvBoard, (void *) &ft);
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);
	while (1){
	  gettimeofday(&tv2, NULL);
	  if (tv2.tv_sec - tv1.tv_sec > 2){
	    gettimeofday(&tv1, NULL);
	    pthread_mutex_lock(&mt);
	    pthread_mutex_unlock(&mt);
	  }
	  printBoard(sb);
	  printCursor(cursor, sb);
	  if (sb.b[9] != 0)
	    break;
	  if (myturn == 1){
	    mt_gotoXY(1, 29);
	    printf("Your turn!                       ");
	  } else if (myturn == 0){
	    mt_gotoXY(1, 29);
	    printf("Waiting your enemy.           ");
	  } else if (myturn == 3 && iam == 2){
	    mt_gotoXY(1, 29);
	    printf("Waiting your enemy.           ");
	  } else if (myturn == 3){
	    mt_gotoXY(1, 29);
	    printf("Waiting for someone to connect.         ");
	  }
	    rk_readkey(&a);
	    switch(a){
	    case KEY_right:
	      if (cursor.p < 8)
		cursor.p++;
	      break;
	    case KEY_left:
	      if (cursor.p > 0)
		cursor.p--;
	      break;
	    case KEY_up:
	      if (cursor.p > 2)
		cursor.p -= 3;
	      break;
	    case KEY_down:
	      if (cursor.p < 6)
		cursor.p += 3;
	      break;
	    case KEY_f5:
	      if ((sb.b[cursor.p] == 0) && (iam == 1) && (myturn == 1)){
		printSym(cursor, 1, &sb);
		printf("\a");
	      }else if (myturn != 1){
		mt_gotoXY(1,29);
		printf("It's not your turn..");
	      }
	      else if (iam == 2){
		mt_gotoXY(1,29);
		printf("You are playing for O ..");
	      }
	      break;
	    case KEY_f6:
	      if ((sb.b[cursor.p] == 0) && (iam == 2) && (myturn == 1)){
		printSym(cursor, 2, &sb);
		printf("\a");
	      }
	      else if (myturn != 1){
		mt_gotoXY(1,29);
		printf("It's not your turn..");
	      }
	      else if (iam == 1){
		mt_gotoXY(1,29);
		printf("You are playing for X ..");
	      }
	      break;
	    }
	    if (((a == KEY_f6) && (iam == 2) || (a == KEY_f5) && (iam == 1)) && (myturn == 1)){
	      pthread_mutex_lock(&mt);
	      myturn = 2;
	      pthread_mutex_unlock(&mt);
	      printBoard(myb);
	      printCursor(cursor, sb); 
	    }
	}
	printBoard(sb);
	mt_gotoXY(1,29);
	printf("Game over, the winner is %d\n", sb.b[9]);
	close(sockfd);
}
