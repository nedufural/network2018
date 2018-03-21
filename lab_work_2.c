#include<stdio.h> //printf
#include<string.h> //memset
#include<stdlib.h> //for exit(0);
#include<sys/socket.h>
#include<errno.h> //For errno - the error number
#include<netdb.h> //hostent
#include<arpa/inet.h>
 
int main(){
// receive the domain name
    char domainName[200];
    int i;
    printf("Input the domain name :  ");
    scanf("%s", domainName); 
    printf("\n");
// changes the domain name 
    struct hostent *hst=gethostbyname(domainName);
//print the results
    printf("The Host Name->%s\n", hst->h_name);
    printf("The IP Address->%s\n",inet_ntoa(*(struct in_addr *)hst->h_addr_list[0]) );
}
