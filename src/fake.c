#include <stdio.h>
#include "../include/server.h"

int main(){
if(startserver(3000) == 0){
	printf("I am the fake server, the evilest of them all!\n");
	while(1){
		char message[1024];
		printf("%d\n", getmsg(message));
		printf("recieved message %s\n", message);
	}
}	
return 0;
}
