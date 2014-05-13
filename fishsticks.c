#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>    
#include <signal.h>
#include <time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
   	 
#define MAXLENGTH 128    

// Function prototypes 
int hostGame(char*);
int joinGame(char*);    
int userMove(int, int);
int printObj(int, int);   
 
char buf[MAXLENGTH];	 
       	 
int main(int argc, char** argv){
          	 
  if(argc != 3){
	printf("The proper usage is:\n  ./filename host <port>\n  ./filename join <ip:port>\n");}
   	  
  if(strcmp(argv[1], "host") == 0){
	hostGame(argv[2]);}
   	 
  else if(strcmp(argv[1], "join") == 0){
	joinGame(argv[2]);}
   	 
  return 0;
}

// Host waits till a player joins and the game can start    
int hostGame(char* port){
  int sockfd;
  int clientfd;
  struct sockaddr_in addr;
  int recvd = 0;
   	 
  printf("Hosting game... \n");
   	 
  // Socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
	perror("Error: hostGame() socket error.");
	exit(1);
  }
  // Set address values
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(atoi(port));

  // Bind the socket
  if (bind(sockfd, (struct sockaddr*) &addr, sizeof(struct sockaddr_in)) < 0){
	perror("Error: hostGame() bind error.");
	exit(1);
  }
   	 
  // Listen for connections
  if(listen(sockfd, 5) < 0){
	perror("Error: hostGame() listen error.");
	exit(1);
  }
   	 
  printf("Waiting for other player... \n");

  // Accept a connection 
  if((clientfd = accept(sockfd, 0,0)) < 0){
	perror("Error: hostGame() accept error.");
	exit(1);
  }
   	 
  printf("Player connected. Game start.\n");
   	 
  // Initialize random seed
  srand(time(NULL));
   	 
  // Generate a random number
  int gameObj = (rand() % 20) + 20;
  printf("Number of fishsticks: %d\n", gameObj);

  // Game starts
  while(gameObj > 0){
	gameObj = userMove(gameObj, clientfd);
	if(send(clientfd, buf, sizeof(buf),0) < 0){
  	perror("Error: hostGame send error.");
  	exit(1);
	}
	printObj(gameObj, 0);

	printf("Waiting for opponent's move...\n");
	if((recvd = recv(clientfd, buf, sizeof(buf), MSG_WAITALL)) < 0){
  	perror("Error: joinGame recv error.");
  	exit(1);
	}
	gameObj = atoi(buf);
	printObj(gameObj, 1);
  }
}

// Join a game initialized by a host 	 
int joinGame(char* ipaddr){
  char* ip;
  char* portstr;
  int sockfd;
  int clientfd;
  int recvd = 0;
  int gameObj = 1;
   	 
  // Put ip address into ip
  portstr = strtok(ipaddr, ":");
  ip = portstr;
      	 
  // Put port number into portstr as a string
  portstr = strtok(NULL, ":");
   	 
  printf("Connecting to %s:%s...\n", ip, portstr);
 
  // Set address values 	 
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = inet_addr(ip);
  addr.sin_port = htons(atoi(portstr));
   	
  // Socket
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
	perror("Error: joinGame() socket error.");
	exit(1);
  }

  // Set address values
  struct sockaddr_in newaddr;
  bzero(&newaddr, sizeof(newaddr));
  newaddr.sin_family = AF_INET;
  newaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  newaddr.sin_port = htons(atoi(portstr));
 
  // Bind the socket
  if (bind(sockfd, (struct sockaddr*) &newaddr, sizeof(struct sockaddr_in)) < 0){
	perror("Error: joinGame() bind error.");
	exit(1);
  }
   	 
  // Connect to host
  if((clientfd = connect(sockfd, (struct sockaddr *) &addr, sizeof(addr))) < 0){
	printf("Error: joinGame() unable to connect to host.\n");
	exit(1);
  }
  // Receive host s first move	 
  printf("Waiting for opponent's move...\n");
  if((recvd = recv(sockfd, buf, sizeof(buf), MSG_WAITALL)) < 0){
	perror("Error: joinGame recv error.");
	exit(1);
  }
   	 
  gameObj = atoi(buf);
  printf("%d fishsticks left.\n", gameObj);
   	 
  // Game starts
  while(gameObj > 0){
	gameObj = userMove(gameObj, sockfd);
	write(sockfd, buf, sizeof(buf));
	printObj(gameObj, 0);

	printf("Waiting for opponent's move...\n");
	if((recvd = recv(sockfd, buf, sizeof(buf), MSG_WAITALL)) < 0){
  	perror("Error: joinGame recv error.");
  	exit(1);
	}
	gameObj = atoi(buf);
	printObj(gameObj, 1);
  }    
  return 0;   
}

// Function that prints number of remaining fish sticks and also determines if player has // won or lost  
int printObj(int gameObj, int player){
  if(gameObj > 0){
	printf("%d fishsticks left.\n", gameObj);
  }
  // Game ends when no more fish sticks are left
  if(gameObj <= 0){
	printf("No fishsticks left.\n");
	if(player == 0){
  	 printf("YOU LOSE!\n");}
	else if(player == 1){
  	printf("YOU WIN!\n");}
	exit(1);
  }
  return 0;
}



// Function that represents a player s turn
int userMove(int gameObj, int sockfd){
  int input;
  // Player can only remove 1, 2, or 3 fish sticks per turn	 
  printf("Enter a number of fishsticks to remove (1, 2, or 3): ");
  scanf("%d", &input);
  // Check if input is valid, if not, ask the user to re-enter input	 
  if (input == 1 || input == 2 || input == 3) {                                       	 
	printf("You took %d fishsticks. \n", input);
	gameObj = gameObj - input;
	snprintf(buf, sizeof(buf), "%d", gameObj);
  }
  else {
	printf("Please input a valid number of fishsticks. \n");
	userMove(gameObj, sockfd);
  }	 
  return gameObj;
}

