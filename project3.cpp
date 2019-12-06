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
void fowardData(int routerFD);

// host
void startHost(char *param);
bool isDataToSend();
void sendData(struct sockaddr_in routerAddr, int socketFD);
void recvData(struct sockaddr_in routerAddr, int socketFD);


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
}

// Router code below here

void startRouter(char *param) {
    // 1. Parse params
	char *token = strtok(param, ",:");
	std::map<std::string, char *> table; 
	while (token !=NULL){
		char* overlayIP = token;
		char* vmIP = strtok(NULL, ",:");

        if(overlayIP == NULL || vmIP == NULL){
            printf("Invalid parameters, run as ./project3 --router <list of host IP mappings with each pair as \"overlapIP:vmIP\" and each pairing separated by \",\">\n");
            return;
        }

		table[std::string(overlayIP)] = vmIP;
		token = strtok(NULL, ",:");
	}

    std::map<std::string, char*>::iterator it = table.begin();

    printf("Routing Table: \n");
    while(it != table.end()){
        std::cout << "\t" << it->first << " : " << it->second << std::endl;
        it++;
    }

    char data[100];
    char hostIP[100];
    char TTL[100];

    std::string ip(hostIP);
    std::cout << "Overlay IP: " << ip << std::endl;

    char* hostvmIP = table.find(ip)->second;
    std::cout << "VM IP: " << hostvmIP << std::endl;

    // 2. Set up and bind server
    struct sockaddr_in servaddr; 
    bzero(&servaddr, sizeof(servaddr)); 
  
    int routerFD = socket(AF_INET, SOCK_DGRAM, 0);         
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(2012); 
    servaddr.sin_family = AF_INET;  
   
    // 3. bind server address to socket descriptor 
    bind(routerFD, (struct sockaddr*)&servaddr, sizeof(servaddr));

    // 4. foward data
    fowardData(routerFD);
}

void fowardData(int routerFD) {

    //receive the datagram 
    char buffer[100];
    int n = recv(routerFD, buffer, sizeof(buffer), 0);
    buffer[n] = '\0'; 
    printf("Host Sent: %s", buffer);

    //build client addr
    char *message = "hello host \n\nsincerely, \nthe router";
    struct sockaddr_in cliaddr;
    cliaddr.sin_addr.s_addr = inet_addr(buffer);  // TODO: make client ip
    cliaddr.sin_port = htons(2012); 
    cliaddr.sin_family = AF_INET;  
    // send data to client
    sendto(routerFD, message, 1000, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr)); 
}

// =======================
// Host code below here

void startHost(char *param) {
    const char delimeter[2] = ",";
    
    // 1. Parse parameters
    char* routerIP = strtok(param, delimeter);
    char* hostIP = strtok(NULL, delimeter);
    char* timeToLive = strtok(NULL, delimeter);
    if (routerIP == NULL || hostIP == NULL || timeToLive == NULL) {
        printf("Invalid parameters, run as ./project3 --host routerIP,hostIP,TTL\n");
        return;
    }
    printf("Starting host with parameters router IP: %s, host IP: %s, TTL:, %s\n", routerIP, hostIP, timeToLive);
    
    // 2. Set up server credentials
    struct sockaddr_in servaddr;  
    bzero(&servaddr, sizeof(servaddr)); 
    servaddr.sin_addr.s_addr = inet_addr(routerIP);
    servaddr.sin_port = htons(2012); 
    servaddr.sin_family = AF_INET; 

    // 3. Set up host socket 
    int hostSocket = socket(AF_INET, SOCK_DGRAM, 0); 

    // 4. Handle data
    sendData(servaddr, hostSocket);
    recvData(servaddr, hostSocket);
}

/**
 * NOTE: File needs to give program read permissions too.
 * @return If there is data waiting to be sent
 */
bool isDataToSend() {
    return access("tosend.bin", R_OK) != -1;
}

void sendData(struct sockaddr_in routerAddr, int socketFD) {
    char *message = "10.0.2.15"; 
    socklen_t len = sizeof(routerAddr);
    sendto(socketFD, message, 1000, 0, (struct sockaddr*)&routerAddr, sizeof(routerAddr)); 
}

void recvData(struct sockaddr_in routerAddr, int socketFD) {
    char buffer[100];
    socklen_t len = sizeof(routerAddr);
    int n = recvfrom(socketFD, buffer, sizeof(buffer), 0, (struct sockaddr*)&routerAddr, &len);
    buffer[n] = '\0'; 
    printf("Response: %s", buffer);
}