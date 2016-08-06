#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int sockfd, portno, pid,newsockfd;
int clientsockfd[50];
int disconnected[50];
int cnt;
socklen_t clilen;

pthread_mutex_t lock;

struct sockaddr_in serv_addr, cli_addr;

void writeToall(char *msg,int x)
{
	int i;

	pthread_mutex_lock(&lock);

	for(i=0;i<=cnt;i++) {
		if((disconnected[i] == 0) && (i!=x)) {
//			printf("Writing to: %d\n", i);
			int w = write(clientsockfd[i],msg,strlen(msg));

			if(w<0) error("Error in writing");
		}
	}
	pthread_mutex_unlock(&lock);
}

void *dostuff(void * args)
{

//  int *i = (int *) args;
   char buffer[256];
 
int i = cnt;

  while(1) {
   bzero(buffer,256);

   int n = read(clientsockfd[i],buffer,255);
//printf("In dostuff %d\n", i); 
  
//	printf("Client %d:%d\n", i,n);
   if (n < 0) error("ERROR reading from socket");

   printf("Client%d : %s", i,buffer);

   writeToall(buffer,i);
  }
return NULL;
   
}

int main(int argc, char *argv[])
{
    int i;

    struct arg *args;

    pthread_mutex_init(&lock,NULL);

    for(i=0;i<50;i++) disconnected[i] = 1;

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }

     sockfd = socket(AF_INET, SOCK_STREAM, 0);

     if (sockfd < 0) error("ERROR opening socket");

     bzero((char *) &serv_addr, sizeof(serv_addr));

     portno = atoi(argv[1]);

     serv_addr.sin_family = AF_INET;

     serv_addr.sin_addr.s_addr = INADDR_ANY;

     serv_addr.sin_port = htons(portno);

     if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     listen(sockfd,5);

     clilen = sizeof(cli_addr);

     cnt = 0;

     while (1) {
         newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);	
	 cnt++;

	 if(cnt>=50) {
		 exit(1);
	 }

	 for(i=1;i<50;i++) {
		 if(disconnected[i]==1) {
			 disconnected[i] = 0;
			 break;
		 }
	 }
	 clientsockfd[i] = newsockfd;

	 printf("Client %d connected...\n", cnt);

	 pthread_t t;

	 pthread_create(&t,NULL,dostuff,NULL);
     }

     close(sockfd);

     return 0;
}
