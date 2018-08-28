//system header files declaration
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>

//pre defined variables 
#define PORT 8487
#define BufferSize 1024

//receiving user input with pipe
void *user_message_input ();
//handles socket connectio and pipe reading [0]
void *connection_controller ();
//check valid host name
void getHostName (char *hostname);

char messages[BufferSize];
struct sockaddr_in saddr;
struct hostent *ht;
struct in_addr *addr;
int sockfd, fl, _pipefd[2];;


void *user_message_input ()
{

  while (1)
    {
      printf ("input msg: ");
//receive msg using stdin
      fgets (messages, sizeof (messages), stdin);
//end of message line
      if ((strlen (messages) > 1) && (messages[strlen (messages)] == '\n'))
	{
	  messages[strlen (messages) - 1] = '\0';
	}
//write into the pipe end [1]
      write (_pipefd[1], messages, sizeof (messages));
    }


}

void *connection_controller ()
{
  while (1)
    {
      fd_set set;		// declaration of the set
      FD_ZERO (&set);		// clear the set
      FD_SET (sockfd, &set);	// add listening sockfd to set
      FD_SET (_pipefd[0], &set);	//add pipe reading end to the set

/*int maxfd = sockfd; // a required value to pass to select()
for (int i = 0; i < 100; i++) {
// add connected client sockets to set
if (clientfds[i] > 0) FD_SET(clientfds[i], &set);
if (clientfds[i] > maxfd) maxfd = clientfds[i];
}*/

// poll and wait, blocked indefinitely
      select (sockfd > _pipefd[0] ? sockfd + 1 : _pipefd[0] + 1, &set, NULL,
	      NULL, NULL);

// a B+listeningB; socket?
      if (FD_ISSET (sockfd, &set))
	{
	  if (read (sockfd, messages, sizeof (messages)) > 0)
	    {
	      printf ("\n%s \n:input msg ", messages);
	      fflush (stdout);
	    }
	  else
	    {
	      printf ("\n disconnected \n");
	      // close read and write end of the pipe , socket socket and terminate program successfully
	      shutdown (sockfd, SHUT_RDWR);
	      close (sockfd);
	      close (_pipefd[0]);
	      close (_pipefd[1]);
	      exit (0);
	    }
	}
      //read 
      if (FD_ISSET (_pipefd[0], &set))
	{
	  if (read (_pipefd[0], messages, sizeof (messages)) > 0)
	    {
	      write (sockfd, messages, sizeof (messages));
	    }
	  else
	    {

	      // close read and write end of the pipe , socket and terminate program socket in event of error
	      shutdown (sockfd, SHUT_RDWR);
	      close (sockfd);
	      close (_pipefd[0]);
	      close (_pipefd[1]);
	      exit (1);
	    }
	}
    }
}
  void getHostName (char *hostname)
  {
    ht = gethostbyname (hostname);
    if (ht != NULL)
      {
	int i = 0;
	printf ("host address for %s: \n", hostname);
	while (ht->h_addr_list[i] != NULL)
	  printf ("%s\n",
		  inet_ntoa (*(struct in_addr *) ht->h_addr_list[i++]));
      }
    else
      {
	printf ("Cannot get host name.\n");
      }
  }

  int main(int argc, char **argv)
  {
    if (argc < 2)
      {
	char hostname[256];
	printf ("Please enter domain name: ");
	scanf ("%s", hostname);
	getHostName (hostname);
      }
    else
      {
	int i;
	for (i = 1; i < argc; i++)
	  {
	    getHostName (argv[i]);
	  }
      }
    if ((sockfd = socket (AF_INET, SOCK_STREAM, 0)) < 0)
      {
	printf ("Error creating socket!\n");
	shutdown (sockfd, SHUT_RDWR);
	close (sockfd);
	exit (1);
      }





    memset (&saddr, 0, sizeof (saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    saddr.sin_port = htons (PORT);

	if (listen (sockfd, 10) < 0)
	{
	printf ("Error listening\n");
	shutdown (sockfd, SHUT_RDWR); close (sockfd); exit (1);}

	 if (connect(sockfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
	printf ("Error accepting connection\n");
	shutdown (sockfd, SHUT_RDWR); close (sockfd); exit (1);}


	// make it nonblocking 
//"fcntl()" is to find out the characteristics of the file descriptor
//O_APPEND — Set append mode.
//O_DSYNC — Write according to synchronized I/O data integrity completion.
//O_NONBLOCK — Non-blocking mode.
//O_RSYNC — Synchronized read I/O operations.
//O_SYNC — Write according to synchronized I/O file integrity completion.*/

	fl = fcntl (sockfd, F_GETFL, 0);
	fl |= O_NONBLOCK;
	fcntl (sockfd, F_SETFL, fl);
	pipe (_pipefd);

//creating the threads to handle connections and inputs from clents
	pthread_t user_message_input_thread;
	pthread_t connection_controller_thread;	
	void *status;
	pthread_create (&user_message_input_thread, NULL, user_message_input,NULL); 
	pthread_create (&connection_controller_thread, NULL,connection_controller, NULL);
	pthread_join (connection_controller_thread, &status);
    	pthread_join (user_message_input_thread, &status);
return 0;

}

