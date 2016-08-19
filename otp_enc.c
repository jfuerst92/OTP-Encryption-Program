/**************************************************
* otp_enc.c
*Author: Joseph Fuerst
* Description: Sends a text file and a key to a program
*that will encrypt it, then return the encrypted file., socketing code from supplied
* client.c and server.c files and class lectures. 
**************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define BSIZE 10000
/**************************************************
* error
* Description: Prints an error message to stderr, code from supplied
* client.c file. 
**************************************************/
void error(const char *msg) {
    perror(msg);
    exit(1);
}
/**************************************************
* checkContent
* Description: does some input validation on the text that is given to it
**************************************************/
int checkContent(char* arg1, char* arg2)
{
	FILE* text;
	FILE* key;
	int tBytes;
	int kBytes;
	//open files
	text = fopen(arg1, "r");
	key = fopen(arg2, "r");
	if (text == NULL)
	{
		error("ERROR: Cannot open text file.\n");
		return 1;
	}
	else if (key == NULL) 
	{
		error("ERROR: Cannot open key file.\n");
		return 1;
	}
	char curChar;
 

	while((curChar = fgetc(text)) != EOF)
	{
		if(curChar == '$')
		{
			error( "ERROR: text file has invalid characters\n");
			exit(1);
		}
    }
	//get the bytes to make sure key is large enough
	fseek(text, 0, SEEK_END);
	tBytes = ftell(text);
	fseek(key, 0, SEEK_END);
	kBytes = ftell(key);
	
	if (tBytes > kBytes)
	{
		//error if not
		error("Key is too small for this file. ");
		return 1;
	}
	
	fclose(key);
	fclose(text);
	
	return 0;
	
}

int main(int argc, char* argv[])
{
	FILE* text; //file pointers for text and key
	FILE* key;
	char curChar; //current char being analyzed
	int v = 1;
	int sockfd; //socket file descriptor
	int portno;
	int nChar; //number of characters
	struct sockaddr_in serv_addr; //server address
	struct hostent *server;
	int check = 0;
	char buffer[BSIZE];
	
	
	//check validity of the input
	if (argc != 4)
	{
		printf("USAGE: otp_enc PLAINTEXTFILE KEYFILE PORT\n");
		exit(1);
	}
	//socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error("ERROR: cannot open socket.\n");
		exit(1);
	}
	//server set to localhost
	server = gethostbyname("localhost");
    if (server == NULL) {
        fprintf(stderr,"ERROR: host unavailable\n"); 
        exit(0);
	}
	
	//set up socket info
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &v, sizeof(int));
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[3]);
	//server = gethostbyname("localhost");
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);  
	serv_addr.sin_port = htons(portno);
	//serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	 
	//connect
	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
	{
		error("ERROR: cannot connect to port\n");
		exit(2);
	}
	
	//This part checks that this is connecting to the right server process
	char process[] = "e";
	int n;
	n = write(sockfd, process, strlen(process)); //send e indicationg encryption client
	if (n < 0) 
	{
		error("ERROR: cannot write to socket");
	}
	//get a resonse from the server
	char rep[50];
	n = recv(sockfd, rep, sizeof(rep), 0);
	if (n < 0) 
	{
		error("ERROR reading from socket");	
	}
	//check for if the server rejects
	if(strcmp(rep, "no") == 0)	//if so, error
	{
		
		
		error("ERROR: not authorzied to connect on that port");
		exit(1);
	}
	

	
	//store text data in buffer
    snprintf(buffer, BSIZE, "%s", argv[1]);
	
	//check validity of input
	check = checkContent(argv[1], argv[2]);
	if (check == 1)
	{
		exit(1);
	}
	
	//now send the files to the server and wait for a response.
	int wVal;
	int rVal;
	wVal = write(sockfd, buffer, strlen(buffer)); //write contents of text file
	if (wVal < 0) {
		error("ERROR: write to socket failed.\n");	
	}
	
	snprintf(buffer, BSIZE, "%s", argv[2]);
	wVal = write(sockfd, buffer, strlen(buffer)); //write contents of key
	if (wVal < 0) {
		error("ERROR: write to socket failed.\n");	
	}

	bzero(buffer, BSIZE);
	rVal = read(sockfd, buffer, BSIZE); //read the responding encrypted text
	if (wVal < 0) {
		error("ERROR: Read from socket failed.\n");	
	}

	printf("%s\n", buffer);  //print out the result
	
	 close(sockfd);
	 
	 return 0;
	
	
	
	
	
}