#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<string.h>

/* MAIN FUNCTION */
int main(int argc, char *argv[]){
	srand(time(NULL));
	
	if(argc != 2){
		printf("Usage: keygen LENGTH\n, argv[0]");
		exit(1);
	}

	int kLen = atoi(argv[1]);
	
	int i;
	int rNum;
	int aChar;
	for (i = 0; i < kLen; i++){
		rNum = rand() % 27;
		
		if (rNum == 26){
			printf(" ");
		}
		else{
			aChar = 65 + rNum;
			printf("%c", aChar);
		}
	}
	
	printf("\n");
}