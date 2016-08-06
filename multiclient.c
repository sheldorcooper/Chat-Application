#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <pthread.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int sockfd, portno, n;
struct sockaddr_in serv_addr;
struct hostent *server;

void *writing(void *arg)
{
	int w;
	char buffer[256];

	while(1) {
		fgets(buffer,255,stdin);

//		printf("You wrote : %s\n", buffer);
			
		w = write(sockfd,buffer,strlen(buffer));

//		printf("I wrote in socket\n");
		if(w<0) error("Error in writing");

		bzero(buffer,256);
	}

	return NULL;
}

void *reading(void *arg)
{
	int r;

	char buffer[256];

	while(1) {
		r = read(sockfd,buffer,255);

		if(r<0) error("Error in reading");
		printf("%s", buffer);

		bzero(buffer,256);
	}

	return NULL;
}

int main(int argc, char *argv[])
{

    char buffer[256];

    void *exit_status1;
    void *exit_status2;

    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) 
        error("ERROR opening socket");

    server = gethostbyname(argv[1]);

    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;

    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,server->h_length);

    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) error("ERROR connecting");
	
	printf("You are connected\n");

	pthread_t toread,towrite;

	pthread_create(&toread,NULL,reading,NULL);
	pthread_create(&towrite,NULL,writing,NULL);

	pthread_join(toread,&exit_status1);
	pthread_join(towrite,&exit_status2);

    	close(sockfd);
	
    	return 0;
}
