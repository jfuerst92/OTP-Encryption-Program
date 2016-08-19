/**************************************************
* otp_dec_d.c
*Author: Joseph Fuerst
* Description: Decrypts an encrypted text sent to it and sends the 
* decrypted results back
**************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>

#define BSIZE	10000

void error(const char *msg) {
    perror(msg);
    exit(1);
}
/**************************************************
* error
* Description: Prints an error message to stderr, code from supplied
* client.c file. 
**************************************************/
int main(int argc, char *argv[])
{
	int sockfd; //socket fd
	int nSfd; //sdf for the child
	int portno; //port
	int status;
	int nChildren = 0; //number of running children
	int v = 1; //value for reusing socket
	char buf[BSIZE]; //buffer 
	struct sockaddr_in serv_addr; //server address
	struct sockaddr_in cli_addr; //client address
	socklen_t cAlen; 
	pid_t pid;
	
	//check for the correct num of args
	if (argc != 2) 
	{
		error("Usage: otp_enc_d PORT &\n");
		exit(1);
	}
	//socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error("ERROR: cannot open socket.\n");
	}
	//set socket info
	bzero((char*) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;				
	serv_addr.sin_addr.s_addr = INADDR_ANY;		
	serv_addr.sin_port = htons(portno); 
	//bind the socket
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error("ERROR: Bind failed!\n");
	}
	
	//listen for up to 5
	listen(sockfd, 5);

	//infinite while loop runs to get multiple requests
	while (1) 
	{
		int nSfd; //new socket
		int status;
		char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; //array of letters for decrypting
		char buffer[BSIZE]; 
		char textBuf[BSIZE];
		char keyBuf[BSIZE];
		int rVal; 
		int val = 1;
		FILE* tFile; //file pointer
		int textVal;    //ascii rep for text
		int keyVal;   //ascii rep for key
		char curLetter; //current letter being analyzed
		int newNum; 
		int cVal = 0;
		char process[50]; //process name
		char rejection[] = "no"; //responses
		char ok[] = "ok";
			
		int k;
		//check for children
		for(k=0; k < nChildren; k++)
		{
			if (waitpid(-1, &status, WNOHANG) == -1)
			{
				error("ERROR: wait failed\n");  
			}
			if(WIFEXITED(status))
			{
				nChildren--;
			}
		}
        //accept socket
        cAlen = sizeof(cli_addr);
        nSfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cAlen);
        if (nSfd < 0) {
            error("ERROR: accept failed\n");
        }

        //fork new process
        pid = fork();

        
        if (pid < 0) {
            error("ERROR: Fork failed");
            exit(1);
        }

       //child process
        if(pid == 0)
		{
		
			//make sure calling process is valid
			cVal = recv(nSfd, process, sizeof(process), 0);
			if (cVal < 0) 
			{
				error("ERROR reading from socket");	
			}
			//check process name
			if(strcmp(process, "d") != 0) //if not valid, error and return
			{
				cVal = write(nSfd, rejection, strlen(rejection));
				//error("ERROR: This program is not authorized to work with otp_enc_d");				
				return;
			}
			else //valid, send the ok.
			{
				cVal = write(nSfd, ok, strlen(ok));
				
			}
		
            //read the encrypted message
            bzero(buffer, BSIZE);
            rVal = read(nSfd, buffer, BSIZE);
            if (rVal < 0) 
			{
                error("ERROR reading from socket.\n");
            }
			
            tFile = fopen(buffer, "r");
            if (tFile == NULL) 
			{
                exit(1);
            }
			//store the info in to be manipulated
            
            char textBuf[BSIZE];
            fgets(textBuf, BSIZE, tFile);
			
            fclose(tFile);

            //read the key data
            bzero(buffer, BSIZE);
            rVal = read(nSfd, buffer, BSIZE);
            if (rVal < 0) {
                error("ERROR reading from socket.\n");
            }

            
            tFile = fopen(buffer, "r");
            if (tFile == NULL) {
                error("ERROR opening key file.\n");
            }

           //store the key info
            char key[BSIZE];
            fgets(key, BSIZE, tFile);
            fclose(tFile);

           //now decrypt the data
            char eData[strlen(textBuf)];
           
			int i;
            for(i = 0; i < strlen(textBuf); i++) {

                // text letters
				int j;
                for(j = 0; j < strlen(letters); j++){
                    if(textBuf[i] == letters[j]){
                        textVal = j;
                    }
                }
				//watch for spaces
				if(textBuf[i] == ' ')
				{
					textVal = 26;
				}

                // key letters
                for(j = 0; j < strlen(letters); j++){
                    if(key[i] == letters[j]){
                        keyVal = j;
                    }
                }
				
				if(keyBuf[i] == ' ')
				{
					textVal = 26;
				}
				

                // get the value of the current letter
                curLetter = (textVal - keyVal) % 27;
				if(curLetter < 0)
				{
                    curLetter = curLetter + 27;           
                }
				//give it a proper ascii value so it displays an actual letter.
                eData[i] = curLetter + 65;
				//check for spaces
				if (curLetter == 26)
				{
					eData[i] = ' ';
				}
				
            }
            eData[strlen(textBuf) -1] = '\0';

            //send the eData as a response
            bzero(buffer, BSIZE);
            snprintf(buffer, BSIZE, "%s", eData);
            rVal = write(nSfd, buffer, strlen(buffer));
            if (rVal < 0) {
                error("ERROR writing to socket.\n");
            }
        } 
        close(nSfd);
    }
	
	close(sockfd);
	return 0;
	
	
}