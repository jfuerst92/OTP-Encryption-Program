/**************************************************
* otp_enc_d.c
*Author: Joseph Fuerst
* Description: Encrypts  text sent to it and sends the 
* encrypted results back
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
/**************************************************
* error
* Description: Prints an error message to stderr, code from supplied
* client.c file. 
**************************************************/
void error(const char *msg) {
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
	int sockfd; //socket
	int nSfd; //new sfd for child
	int portno; //port
	int status; 
	int nChildren = 0; //num of active children
	int v = 1;
	char buf[BSIZE]; //buffer
	struct sockaddr_in serv_addr; //server address
	struct sockaddr_in cli_addr; //client address
	socklen_t cAlen; 
	pid_t pid;
	
	//check for valid arg num
	if (argc != 2) {
		error("Usage: otp_enc_d PORT &\n");
		exit(1);
	}
	//socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
	{
		error("ERROR: cannot open socket.\n");
	}
	//input server info
	bzero((char*) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;				
	serv_addr.sin_addr.s_addr = INADDR_ANY;		
	serv_addr.sin_port = htons(portno); 
	//fprintf(stderr, "binding 48\n");
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		error("ERROR: Bind failed!\n");
	}

	//listen for up to 5
	listen(sockfd, 5);
	//infinite while loop to process requests
	while (1) 
	{
		int nSfd;
		int status;
		char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "; //letter array to assist in encryption
		char buffer[BSIZE]; //buffers
		char textBuf[BSIZE];
		char keyBuf[BSIZE];
		char rejection[] = "no"; //responses
		char ok[] = "ok";
		int rVal;
		int val = 1;
		int cVal = 0;
		char process[50];
			
		int k;
		//check for children
		for(k=0; k < nChildren; k++)
		{
			if (waitpid(-1, &status, WNOHANG) == -1)
			{
				error("ERROR: could not wait!\n");  
			}
			if(WIFEXITED(status))
			{
				nChildren--;
			}
		}
        // accept the connection
        cAlen = sizeof(cli_addr);
        nSfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cAlen);
        if (nSfd < 0) {
            error("ERROR: accept failed.\n");
        }

        //fork a new process to handle connection
        pid = fork();
        if (pid < 0) {
            error("ERROR: fork failed");
            exit(1);
        }

        // no errors, child
        if(pid == 0)
		{
			
			//check that the client has access privilege
			cVal = recv(nSfd, process, sizeof(process), 0);
			if (cVal < 0) 
			{
				error("ERROR: socket read failed");	
			}
			//compare process name to what was sent
			if(strcmp(process, "e") != 0) //if error, send rejection and return
			{
				cVal = write(nSfd, rejection, strlen(rejection));
					
				return;
			}
			else //send ok
			{
				cVal = write(nSfd, ok, strlen(ok));
				
			}
			
            //read text to be encrypted
            bzero(buffer, BSIZE);
            rVal = read(nSfd, buffer, BSIZE);
            if (rVal < 0) {
                error("ERROR reading from socket.\n");
            }

           
            FILE *fp;
            fp = fopen(buffer, "r");
            if (fp == NULL) {
                exit(1);
            }

           //store the text info
            char textBuf[BSIZE];
            fgets(textBuf, BSIZE, fp);
			
            fclose(fp);

            //read the key to encrypt with
            bzero(buffer, BSIZE);
            rVal = read(nSfd, buffer, BSIZE);
            if (rVal < 0) {
                error("ERROR reading from socket.\n");
            }

            
            fp = fopen(buffer, "r");
            if (fp == NULL) {
                error("ERROR opening key file.\n");
            }

            //store the key info
            char key[BSIZE];
            fgets(key, BSIZE, fp);
            fclose(fp);

            //begin encrypting the text
            char eData[strlen(textBuf)];
            int textVal;   //ascii val of current text
            int keyVal;   //ascii val of current key
            char curLetter;
            int newNum;

            // for the length of the message, encrypt
			int i;
            for(i = 0; i < strlen(textBuf); i++) {

                //text characters
				int j;
                for(j = 0; j < strlen(letters); j++){
                    if(textBuf[i] == letters[j]){
                        textVal = j;
                    }
                }

                //key characters
                for(j = 0; j < strlen(letters); j++){
                    if(key[i] == letters[j]){
                        keyVal = j;
                    }
                }

                //store the new character value in eData
                curLetter = (textVal + keyVal) % 27;
                eData[i] = letters[curLetter];
            }
            eData[strlen(textBuf) -1] = '\0';

            //send eData as a response
            bzero(buffer, BSIZE);
            snprintf(buffer, BSIZE, "%s", eData);
            rVal = write(nSfd, buffer, strlen(buffer));
            if (rVal < 0) {
                error("ERROR writing to socket.\n");
            }
        } 
		else //parent
		{
			nChildren += 1;
		}
		close(nSfd);
       
    }
	close(sockfd);
	return 0;
	
	
}