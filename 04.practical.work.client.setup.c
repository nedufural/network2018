#include<stdio.h>
#include<string.h>   
#include<sys/socket.h>
#include<arpa/inet.h> 
#include<unistd.h> 
#include<netdb.h>   
 

int main()
{

	char hostName[200];
	struct sockaddr_in saddr;
	struct hostent *h;
	int sockfd;
	unsigned short port = 8784;
    printf("Input the Host Name :  ");
    scanf("%s", hostName);
if ((sockfd=socket(AF_INET, SOCK_STREAM, 0)) < 0) {
	printf("Error creating socket\n");

}
if ((h=gethostbyname(hostName)) == NULL) {
	printf("Unknown host\n");

}
else{
	// changes the domain name 
    struct hostent *hst=gethostbyname(hostName);
	//print the results
    printf("The IP Address->%s\n",inet_ntoa(*(struct in_addr *)hst->h_addr_list[0]) );
}
	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	memcpy((char *) &saddr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	saddr.sin_port = htons(port);
if (connect(sockfd, (struct sockaddr *) &saddr, sizeof(saddr)) < 0) {
	printf("Cannot connect\n");}
else{
	printf("Connected to server\n");}
}
