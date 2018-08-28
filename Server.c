//declarations of all header files
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

//predefine variables
#define PORT 8487
#define MAX_NUMBER_CLIENT 50
#define BUFFER_SIZE 1024

//sockets and clients declarations
unsigned int clen;
int sockfd, fl,clientfd;
//client and server address
struct sockaddr_in saddr, caddr;
int number_of_client = 0;
int clientfds[MAX_NUMBER_CLIENT] = {0};
//parent to child pipe
int p_c[MAX_NUMBER_CLIENT][2] = {{0}};
//child to parent pipe
int c_p[MAX_NUMBER_CLIENT][2] = {{0}};

//pre defined functions
void server_to_all_clients(char *messages);


void server_to_all_clients(char *messages)
{
char send_to_clients[BUFFER_SIZE];
//sprintf in place of printf writing to buffer
  sprintf(send_to_clients, "From Server: ");
  strcat(send_to_clients, messages);
  for (int i = 0; i < MAX_NUMBER_CLIENT; i++) 
{
    if (clientfds[i] > 0) 
{
      write(p_c[i][1], send_to_clients, sizeof(send_to_clients));
    }
  }
}

int main (int argc, char **argv)
{
 if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Error creating socket!\n");
    return 1;
  }
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

// non blocking 
//O_NONBLOCK — Non-blocking mode.
  fl = fcntl(sockfd, F_GETFL, 0);
  fl |= O_NONBLOCK;
  fcntl(sockfd, F_SETFL, fl);

  memset(&saddr, 0, sizeof(saddr));
  saddr.sin_family = AF_INET;
  saddr.sin_addr.s_addr = htonl(INADDR_ANY);
  saddr.sin_port = htons(PORT);
    if (bind (sockfd, (struct sockaddr *) &saddr, sizeof (saddr)) < 0)
	{
	printf ("Error binding\n");
	shutdown (sockfd, SHUT_RDWR); close (sockfd); exit (1);}
	if (listen (sockfd, 5) < 0)
	{
	printf ("Error listening\n");
	shutdown (sockfd, SHUT_RDWR); close (sockfd); exit (1);}

	 if (connect(sockfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
	printf ("Error accepting connection\n");
	shutdown (sockfd, SHUT_RDWR); close (sockfd); exit (1);}

while (1) {
fd_set set; // declaration of the set
FD_ZERO(&set); // clear the set
FD_SET(sockfd, &set); // add listening sockfd to set
 int maxfd = sockfd;
for (int i = 0; i < MAX_NUMBER_CLIENT; i++) {
// add connected child to parent pipe to set

if (clientfds[i] > 0) {
FD_SET(c_p[i][0], &set);
 if (c_p[i][0] > maxfd) {
          maxfd = c_p[i][0];
        }
}
}
// poll and wait, blocked indefinitely
select(maxfd+1, &set, NULL, NULL, NULL);
if (FD_ISSET(0, &set)) {
      char command[BUFFER_SIZE];
      fgets(command, sizeof(command), stdin);
      if (strlen(command) == 1) {
        fflush(stdout);
      } else {
        if ((strlen(command) > 0) && (command[strlen (command) - 1] == '\n')) {
          command[strlen (command) - 1] = '\0';
        }
        char cmd_copy[BUFFER_SIZE];
        strcpy(cmd_copy, command);
        char *first_token = strtok(cmd_copy, " ");
        
        if (strcmp(first_token, "send to all clients") == 0) {
          char *messages;
          messages = strtok(NULL, "");
          if (messages != NULL) {
            server_to_all_clients(messages);
          }
          printf("> ");
          fflush(stdout);
        } 
    if (FD_ISSET(sockfd, &set)) {
      clen=sizeof(caddr);
      clientfd = accept(sockfd, (struct sockaddr *) &caddr, &clen);
      //O_NONBLOCK — Non-blocking mode.
      fl = fcntl(clientfd, F_GETFL, 0);
      fl |= O_NONBLOCK;
      fcntl(sockfd, F_SETFL, fl);
      
      if(number_of_client == MAX_NUMBER_CLIENT) {
         shutdown(clientfd, SHUT_RDWR);
			close(clientfd);
      } else {  
        int i;
        for (i = 0; i < MAX_NUMBER_CLIENT; i++) {
          if (clientfds[i] == 0){
            clientfds[i] = clientfd;
            pipe (p_c[i]);
            pipe (c_p[i]);
            number_of_client++;
            printf("\nThere are %d clients connected)\n> ", i, number_of_client);
            fflush(stdout);
            break;
          }
        }
        
        switch(fork()) {
          case -1:
            printf("\nError creating process!\n");
            return 1;
          case 0:
            close(p_c[i][1]);
            close(c_p[i][0]);
            
            while(1) {
              fd_set set;
              FD_ZERO(&set);
              FD_SET(p_c[i][0], &set);
              FD_SET(clientfds[i], &set);
              
              int maxfd = clientfds[i] > p_c[i][0] ? clientfds[i] : p_c[i][0];
              
              select(maxfd + 1, &set, NULL, NULL, NULL);
              
              if (FD_ISSET(p_c[i][0], &set)) {
                char messages[BUFFER_SIZE];
                if(read(p_c[i][0], messages, sizeof(messages)) > 0) {
                  write(clientfds[i], messages, sizeof(messages));
                } else {
                  return 1;
                }
              }
              
            }
            break;
          default:
            close(p_c[i][0]);
            close(c_p[i][1]);
        }
      }
    }
    
    for (int i=0; i<MAX_NUMBER_CLIENT; i++) {
      if (clientfds[i] > 0 && FD_ISSET(c_p[i][0], &set)) {
        char messages[BUFFER_SIZE];
        if(read(c_p[i][0], messages, sizeof(messages)) < 0){
          return 1;
        }
        printf("\nClient %d sent: %s\n> ", i, messages);
        fflush(stdout);  
        } 
    }
  }
}
}
}
