#include <string.h>
#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// router 
void startRouter(char *param);

// host
void startHost(char *param);
void sendDataToRouter(char *routerIP);
bool isDataToSend();


int main(int argc, char *argv[]) {
    
    if (argc != 3) {
        printf("Invalid parameters, exiting\n");
        return -1;
    }

    bool isRouter = (strcmp(argv[1], "--router") == 0);
    if (!isRouter && !(strcmp(argv[1], "--host") == 0)) { // ensure that parameter says either router or host
        printf("Invalid parameters, exiting\n");
        return -1;
    }

    if (isRouter) {
        startRouter(argv[2]);
    } else {
        startHost(argv[2]);
    }

    // const char delimeter[2] = ",";
    // char *token;
    
    // /* get the first token */
    // token = strtok(argv[2], delimeter);
    
    // /* walk through other tokens */
    // while( token != NULL ) {
    //     printf( "%s\n", token );
    //     token = strtok(NULL, delimeter);
    // }
}

// Router code below here

void startRouter(char *param) {
	//char *token = strtok(param, ",");
	//while (token !=NULL){
		//print("%s\n", token);
		//token = strtok(NULL, ",");
	//}
}

// =======================
// Host code below here

void startHost(char *param) {
    const char delimeter[2] = ",";
    char *token;
    
    /* routerIP,hostIP,TTL */
    char* routerIP = strtok(param, delimeter);
    char* hostIP = strtok(NULL, delimeter);
    char* timeToLive = strtok(NULL, delimeter);
    if (routerIP == NULL || hostIP == NULL || timeToLive == NULL) {
        printf("Invalid parameters, run as ./project3 --host routerIP,hostIP,TTL\n");
        return;
    }
    printf("Starting host with parameters router IP: %s, host IP: %s, TTL:, %s\n", routerIP, hostIP, timeToLive);
}

/**
 * NOTE: File needs to give program read permissions too.
 * @return If there is data waiting to be sent
 */
bool isDataToSend() {
    return access("tosend.bin", R_OK) != -1;
}

/**
 * @param routerIP - The IP of the router
 */
void sendDataToRouter(char* routerIP) {
    struct sockaddr_in servaddr, cliaddr; 
    int host = socket(AF_INET, SOCK_DGRAM, 0);     
    bzero(&servaddr, sizeof(servaddr));    
    servaddr.sin_addr.s_addr = inet_addr(routerIP); 
    servaddr.sin_port = htons(2012); 
    servaddr.sin_family = AF_INET;  
   
    // bind server address to socket descriptor 
    bind(host, (struct sockaddr*)&servaddr, sizeof(servaddr)); 

    // send data here

}
