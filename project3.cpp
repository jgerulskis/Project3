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
void sendData(struct sockaddr_in routerAddr, int socketFD, unsigned char *packet);
void recvData(struct sockaddr_in routerAddr, int socketFD);
void buildPkt(struct sockaddr_in routerAddr, int socketFD, char* TTL);
void printPkt(char *packet);


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

    //char data[100];
    //char hostIP[100];
    //char TTL[100];

    //std::string ip(hostIP);
    //std::cout << "Overlay IP: " << ip << std::endl;

    //char* hostvmIP = table.find(ip)->second;
    //std::cout << "VM IP: " << hostvmIP << std::endl;

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
    char buffer[1009];
    recv(routerFD, buffer, sizeof(buffer), 0);
   	printPkt(buffer);

    //build client addr
    char *message = "hello host \n\nsincerely, \nthe router";
    struct sockaddr_in cliaddr;
    cliaddr.sin_addr.s_addr = inet_addr("10.0.2.4");  // TODO: make client ip
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

    struct sockaddr_in cliaddr; 
    bzero(&cliaddr, sizeof(cliaddr)); 

    cliaddr.sin_addr.s_addr = INADDR_ANY; 
    cliaddr.sin_port = htons(2012); 
    cliaddr.sin_family = AF_INET;   

    // 3. Set up host socket 
    int hostSocket = socket(AF_INET, SOCK_DGRAM, 0); 

    bind(hostSocket, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

    // 4. Handle data
    buildPkt(servaddr, hostSocket, timeToLive);
    recvData(servaddr, hostSocket);
}

/**
 * NOTE: File needs to give program read permissions too.
 * @return If there is data waiting to be sent
 */
bool isDataToSend() {
    return access("tosend.bin", R_OK) != -1;
}

void sendData(struct sockaddr_in routerAddr, int socketFD, unsigned char* packet) {
    //char *message = "10.0.2.4"; 
    socklen_t len = sizeof(routerAddr);
    sendto(socketFD, packet, 1009, 0, (struct sockaddr*)&routerAddr, sizeof(routerAddr)); 
}

void recvData(struct sockaddr_in routerAddr, int socketFD) {
    char buffer[1009];
    socklen_t len = sizeof(routerAddr);
    recvfrom(socketFD, buffer, sizeof(buffer), 0, (struct sockaddr*)&routerAddr, &len); 
    printf("Response: %s", buffer);
}

void buildPkt(struct sockaddr_in routerAddr, int socketFD, char* TTL){
	unsigned char packet[1009];

	unsigned char overlayIPHeader[4];
    unsigned char contentLength[4];
    unsigned char content[1000];
    FILE *f;

    f = fopen("test.bin", "rb");
    fread(overlayIPHeader, sizeof(overlayIPHeader), 1, f);
    fread(contentLength, sizeof(contentLength), 1, f);

    //unsigned char content[contentLength];
    //fread(content, sizeof(content), 1, f);

    for(int i = 0; i < 4; i++){
    	packet[i] = overlayIPHeader[i];
    }
    packet[4] = TTL[0]&0xff;
    //bytes are in reverse order
    for(int i = 0; i < 4; i++){
    	packet[i+5] = contentLength[i];
    }

    while(fread(packet+9, 1000, 1, f) == 1000){
    	sendData(routerAddr, socketFD, packet);
    }
    sendData(routerAddr, socketFD, packet);
}

void printPkt(char *packet){
	printf("overlayIP: ");
	for(int i = 0; i < 4; i++){
		printf("%u.", packet[i]);
	}
	printf("\n");
	printf("TTL: %u\n", packet[4]);

	unsigned char num[4];
	for(int i = 0; i < 4; i++){
		num[i] = packet[5+i];
	}
	int x = *(int*)num;
	printf("Data length: %d\n", x);

	for(int i = 0; i < x; i++){
		printf("%u", packet[i+9]);
	}
}