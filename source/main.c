#include "bigChars.h"
#include "terminal.h"
#include "readkey.h"
#include <stdio.h>

const char _DEFAULT_PORT[5] = "7777";


int main(int argc, char ** argv){	
	if (argc != 2){
	  printf("To start game use: %s hostname\n", argv[0]);
	  return 1;
	}
        char s[INET6_ADDRSTRLEN];
	int sockfd = connectToServer(argv[1], s, _DEFAULT_PORT);
	char buffer[BUFFLEN];
	char ** allChat;
	int messages = 0;
	char myMessage[CHATBUF];
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
        rcvbytes = recv(sockfd, buffer, BUFFLEN, 0);
        if (rcvbytes == 0){
	    printf("Server down...\n");
	    return 0;
	}
	printf("%s", buffer);
	if (strstr(buffer, "60") != NULL)
	  iam = 1;
	else
	  iam = 2;
        struct board sb = {{0}};
        mt_clrscr();
	int a, needtosend = 0, read = 0;
	char answer = '\0';
	struct cursor cursor;
	cursor.p = 0;
	int myturn = 3;
	retPos(cursor.p, &(cursor.x), &(cursor.y));
	pthread_t thread, thread_c;
	pthread_mutex_t mt = PTHREAD_MUTEX_INITIALIZER, cmt = PTHREAD_MUTEX_INITIALIZER;
	struct ftc chat = {myMessage, &cmt, &read};
	pthread_create(&thread_c, NULL, chatThread, (void *) &chat);
	struct ft ft;
	ft.b = &sb;
	ft.sockfd = sockfd;
	ft.turn = &myturn;
	ft.mute = &mt;
	pthread_create(&thread, NULL, recvBoard, (void *) &ft);
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL);
	while (1){
	  if (read == 1){
	  	messages++;
	  	pthread_mutex_lock(&cmt);
	  	allChat = realloc(allChat, sizeof(char *) * messages);
	  	allChat[messages - 1] = malloc(sizeof(char) * CHATBUF);
	  	for (int i = 0; i < CHATBUF; i++)
	  		allChat[messages - 1][i] = '\0';
	  	strcat(allChat[messages - 1], "\033[1;31mEnemy: \033[0m");
	    strcat(allChat[messages - 1], myMessage);
	  	read = 0;
	  	pthread_mutex_unlock(&cmt);
	    allChat[messages - 1][59] = '\0';
	  }
	  gettimeofday(&tv2, NULL);
	  if (tv2.tv_sec - tv1.tv_sec > 2){
	    gettimeofday(&tv1, NULL);
	    pthread_mutex_lock(&mt);
	    pthread_mutex_unlock(&mt);
	  }
	  printBoard(sb);
	  printCursor(cursor, sb);
	  printChat(allChat, messages);
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
			needtosend = 1;
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
			needtosend = 1;
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
	    case KEY_t:
	      pthread_mutex_lock(&cmt);
	      mt_gotoXY(1, 30);
	      fgets(myMessage, CHATBUF, stdin);
	      messages++;
	      allChat = realloc(allChat, sizeof(char *) * messages);
	  	  allChat[messages - 1] = malloc(sizeof(char) * CHATBUF);
	  	  for (int i = 0; i < CHATBUF; i++)
	  	    allChat[messages - 1][i] = '\0';
	      read = 2;
	      strcat(allChat[messages - 1], "\033[1;34mMe: \033[0m");
	      strcat(allChat[messages - 1], myMessage);
	      pthread_mutex_unlock(&cmt);
	      mt_gotoXY(1, 30);
	      printf("                      ");
	      break;
	    }
	    if (needtosend == 1){
	      pthread_mutex_lock(&mt);
	      myturn = 2;
	      pthread_mutex_unlock(&mt);
	      printBoard(sb);
	      printCursor(cursor, sb);
	      needtosend = 0;
	    }
	}
	printBoard(sb);
	mt_gotoXY(1,29);
	char winner[][20] = {"first player", "second player"};
	if (sb.b[9] != 3)
	  printf("Game over, the winner is %s\n", winner[sb.b[9] - 1]);
	else
	  printf("Game over. Draw.\n");
	close(sockfd);
}
