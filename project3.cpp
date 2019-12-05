#include <string.h>
#include <iostream>
#include <stdio.h>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// router 
void startRouter(char *param);

// host
void startHost(char *param);
void sendDataToRouter(char *routerIP, char *hostIP, char *TTL);
bool isDataToSend();
void receiveDataAndSendToHost(std::map<char*, char*> table);


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
	char *token = strtok(param, ",:");
	std::map<char*, char*> table; 
	while (token !=NULL){
		char *overlayIP = token;
		char *vmIP = strtok(NULL, ",:");

        if(overlayIP == NULL || vmIP == NULL){
            printf("Invalid parameters, run as ./project3 --router <list of host IP mappings with each pair as \"overlapIP:vmIP\" and each pairing separated by \",\">\n");
            return;
        }

		table[overlayIP] = vmIP;
		token = strtok(NULL, ",:");
	}

    std::map<char*, char*>::iterator it = table.begin();

    printf("Routing Table: \n");
    while(it != table.end()){
        printf("\t%s : %s \n", it->first, it->second);
        it++;
    }
    receiveDataAndSendToHost(table);
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
    sendDataToRouter(routerIP, hostIP, timeToLive);
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
void sendDataToRouter(char* routerIP, char* hostIP, char* TTL) {

    char buffer[100]; 
    char *message = "Hello Router"; 
    int sockfd, n; 
    struct sockaddr_in servaddr; 
      
    // clear servaddr 
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_addr.s_addr = inet_addr(routerIP);
    servaddr.sin_port = htons(2012); 
    servaddr.sin_family = AF_INET; 
      
    // create datagram socket 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
      
    // connect to server 
    if(connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) 
    { 
        printf("\n Error : Connect Failed \n"); 
        exit(0); 
    } 
  
    // request to send datagram 
    // no need to specify server address in sendto 
    // connect stores the peers IP and port 
    sendto(sockfd, message, 1000, 0, (struct sockaddr*)NULL, sizeof(servaddr)); 
    sendto(sockfd, hostIP, 1000, 0, (struct sockaddr*)NULL, sizeof(servaddr)); 
    sendto(sockfd, TTL, 1000, 0, (struct sockaddr*)NULL, sizeof(servaddr)); 
      
    // waiting for response 
    recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)NULL, NULL); 
    puts(buffer); 
  
    // close the descriptor 
    close(sockfd); 

}

void receiveDataAndSendToHost(std::map<char*, char*> table){
    char data[100];
    char hostIP[100];
    char TTL[100];

    char *message = "Hello Client"; 
    int listenfd;
    socklen_t len; 
    struct sockaddr_in servaddr, cliaddr; 
    bzero(&servaddr, sizeof(servaddr)); 
  
    // Create a UDP Socket 
    listenfd = socket(AF_INET, SOCK_DGRAM, 0);         
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    servaddr.sin_port = htons(2012); 
    servaddr.sin_family = AF_INET;  
   
    // bind server address to socket descriptor 
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)); 
       
    //receive the datagram 
    len = sizeof(cliaddr); 
    int n = recvfrom(listenfd, data, sizeof(data), 
            0, (struct sockaddr*)&cliaddr,&len); //receive message from server 
    data[n] = '\0';   
    puts(data);       

    int m = recvfrom(listenfd, hostIP, sizeof(hostIP), 
            0, (struct sockaddr*)&cliaddr,&len); //receive message from server
    puts(table[hostIP]); 

    hostIP[m] = '\0'; 
    puts(hostIP); 

    int o = recvfrom(listenfd, TTL, sizeof(TTL), 
            0, (struct sockaddr*)&cliaddr,&len); //receive message from server 
    TTL[o] = '\0'; 
    puts(TTL); 
           
    // send the response 
    sendto(listenfd, message, 1000, 0, 
        (struct sockaddr*)&cliaddr, sizeof(cliaddr)); 
}
