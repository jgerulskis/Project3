#include <string.h>
#include <iostream>
#include <stdio.h>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>

// router 
void startRouter(char *param);
void fowardData(int routerFD, std::map<std::string, char *> table);

// host
void startHost(char *param);
bool isDataToSend();
void sendData(struct sockaddr_in routerAddr, int socketFD, unsigned char *packet);
void recvData(struct sockaddr_in routerAddr, int socketFD);
void buildPkt(struct sockaddr_in routerAddr, int socketFD, char* TTL, int* IP);
int printPkt(char *packet);

int maxPackSize = 1013;

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
    while(true){
        fowardData(routerFD, table);
    }
}

void fowardData(int routerFD, std::map<std::string, char *> table) {

    //receive the datagram 
    char packets[100][maxPackSize];
    //char buffer[maxPackSize];

    recv(routerFD, packets[0], maxPackSize, 0);
   	int length = printPkt(packets[0]);
   	//printPkt(packets[0]);
   	int ttl= (int)packets[0][4];
   	int byteRead = 1000;

   	int numpack = 1;
   	while(byteRead < length){
   		recv(routerFD, packets[numpack], maxPackSize, 0);
   		byteRead += 1000;
   		numpack++;
   	}
   	//printPkt(packets[0]);
   	//printPkt(packets[1]);

   	double packSent = ceil(length/1000.0);
   	printf("Packet sent: %f\n", packSent);

   	//overlay IP
   	char* overIP = (char*)malloc(15);
	for(int i = 0; i < 4; i++){
		int temp = packets[0][i];
		strcat(overIP, std::to_string(temp).c_str());
		if(i != 3){
			strcat(overIP, ".");
		}
	}
	strcat(overIP, "\0");
	std::cout << overIP << std::endl;

   	char* SourceIP = (char*)malloc(15);
	for(int i = 0; i < 4; i++){
		int temp = packets[0][i+9];
		strcat(SourceIP, std::to_string(temp).c_str());
		if(i != 3){
			strcat(SourceIP, ".");
		}
	}
	strcat(SourceIP, "\0");
	std::cout << SourceIP << std::endl;

	char *vmIP = table.find(std::string(overIP))->second;

	//decrementing ttl
	ttl--;

    //build client addr
    //char *message = "hello host \n\nsincerely, \nthe router";
    struct sockaddr_in cliaddr;
    cliaddr.sin_addr.s_addr = inet_addr(vmIP);
    cliaddr.sin_port = htons(2012); 
    cliaddr.sin_family = AF_INET;  
    // send data to client

    FILE *f;
    f = fopen("ROUTER_log.txt", "a");

    if(table.find(overIP) == table.end()){
    	fprintf(f, "%u ", (unsigned)time(NULL));
    	fprintf(f, "%s ", SourceIP);
    	fprintf(f, "%s ", overIP);
    	fprintf(f, "%d ", 1);
    	fprintf(f, "NO_ROUTE_TO_HOST\n");
    	return;
    }

    if(ttl < numpack){
    	sendto(routerFD, &ttl, sizeof(int), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    }
    else{
    	sendto(routerFD, &numpack, sizeof(int), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
    }

   	int i;
   	for(i = 0; i < packSent && i < ttl; i++){
   		sendto(routerFD, packets[i], maxPackSize, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
   		fprintf(f, "%u ", (unsigned)time(NULL));
    	fprintf(f, "%s ", SourceIP);
    	fprintf(f, "%s ", overIP);
    	fprintf(f, "%d ", i+1);
    	fprintf(f, "SENT_OKAY\n");
   	}

   	if(ttl < numpack){
    	fprintf(f, "%u ", (unsigned)time(NULL));
    	fprintf(f, "%s ", SourceIP);
    	fprintf(f, "%s ", overIP);
    	fprintf(f, "%d ", ttl+1);
    	fprintf(f, "TTL_EXPIRED\n");
    }

   	fclose(f);
    free(overIP);
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
    fcntl(hostSocket, F_SETFL, O_NONBLOCK); 

    bind(hostSocket, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

    //parse OverlayIP
    char *token = strtok(hostIP, ".");
    int IP[4];
    int j = 0;
	while (token !=NULL){
		IP[j] = atoi(token);
		token = strtok(NULL, ".");
		j++;
	}
	if(j != 4){
		printf("Error: Incorrect Host OverlayIP\n");
		return;
	}

    // 4. Handle data
    while (true) {
    	if (isDataToSend()){
    	   	buildPkt(servaddr, hostSocket, timeToLive, IP);
            remove("tosend.bin");
    	}
    	recvData(servaddr, hostSocket);
	}
}

/**
 * NOTE: File needs to give program read permissions too.
 * @return If there is data waiting to be sent
 */
bool isDataToSend() {
    return access("tosend.bin", R_OK) != -1;
}

void sendData(struct sockaddr_in routerAddr, int socketFD, unsigned char* packet) {
    socklen_t len = sizeof(routerAddr);
    sendto(socketFD, packet, maxPackSize, 0, (struct sockaddr*)&routerAddr, sizeof(routerAddr)); 
}

void recvData(struct sockaddr_in routerAddr, int socketFD) {
	int numPackets = 0;
    char packets[1000][maxPackSize];
    socklen_t len = sizeof(routerAddr);
	recvfrom(socketFD, &numPackets, sizeof(numPackets), 0, (struct sockaddr*)&routerAddr, &len);
	
	//receiving packets
	for(int i = 0; i < numPackets; i++){
		recvfrom(socketFD, packets[i], maxPackSize, 0, (struct sockaddr*)&routerAddr, &len);
	}

	if(numPackets > 0){

		unsigned char num[4];
		for(int i = 3; i >= 0; i--){
			num[i] = packets[0][5+i];
		}
		int datalength = *(int*)num;
		double packSent = ceil(datalength/1000.0);

		FILE *f;

	  	//overlay IP
	   	char* overIP = (char*)calloc(1, 18);
		for(int i = 0; i < 4; i++){
			int temp = packets[0][i+9];
			strcat(overIP, std::to_string(temp).c_str());
			if(i != 3){
				strcat(overIP, "_");
			}
		}
		strcat(overIP, ".bin");
		strcat(overIP, "\0");
		std::cout << overIP << std::endl;
		//char * filename;
		
		//for(int i = 0; i < 18; i ++){
			//if(overIP[i] == "\0"){
				//break;
			//}
			//else{
				//filename+i = *overIP[i];

			//}
		//}

		f = fopen(overIP, "ab");

		//writing to file
		for(int j = 0; j < numPackets; j++){
			if(j == numPackets-1){
				fwrite(&packets[j], datalength%maxPackSize, 1, f);
			}
			else{
				fwrite(&packets[j], maxPackSize, 1, f);
			}
		}

		fclose(f);

		printf("File Received: \n\t Size: %d\n\t Packets Received: %d\n\t Missing Packets: \n", datalength, numPackets);
		for(int j = 0; j < packSent - numPackets; j++){
			printf("\t\t IP Identifier: %d\n", (int)(packSent-j));
		}
	}
}

void buildPkt(struct sockaddr_in routerAddr, int socketFD, char* TTL, int* overlayIP){
	unsigned char packet[maxPackSize];

	unsigned char overlayIPHeader[4];
    unsigned char contentLength[4];
    unsigned char content[1000];
    FILE *f;

    f = fopen("tosend.bin", "rb");
    fread(overlayIPHeader, sizeof(overlayIPHeader), 1, f);
    fread(contentLength, sizeof(contentLength), 1, f);

    for(int i = 0; i < 4; i++){
    	packet[i] = overlayIPHeader[i];
    }
    packet[4] = (unsigned char)atoi(TTL);

    //bytes are in reverse order
    for(int i = 0; i < 4; i++){
    	packet[i+5] = (contentLength)[i];
    }

    //source IP
    for(int i = 0; i < 4; i++){
    	packet[i+9] = (unsigned char)overlayIP[i];
    }

    int counter = 1;
    while(fread(packet+13, 1000, 1, f) == 1){
    	sendData(routerAddr, socketFD, packet);
    	memset(packet+13, 0, 1000);
    	usleep(100000);
    	counter++;
    }

    sendData(routerAddr, socketFD, packet);

    fclose(f);

	int datalength = *(int*)contentLength;
    printf("New File Transmission: \n\tSize: %d \n\tPackets Sent: %d \n", datalength, counter);
}

int printPkt(char *packet){
	printf("overlayIP: ");
	for(int i = 0; i < 4; i++){
		printf("%u.", packet[i]);
	}
	printf("\n");
	printf("TTL: %u\n", packet[4]);

	unsigned char num[4];
	for(int i = 3; i >= 0; i--){
		num[i] = packet[5+i];
	}
	int datalength = *(int*)num;
	printf("Data length: %d\n", datalength);

	printf("SourceIP: ");
	for(int i = 0; i < 4; i++){
		printf("%u.", packet[i+9]);
	}
	printf("\n");

	printf("Data:");
	for(int i = 0; i < datalength-(datalength % 1000); i++){
		printf("%c", packet[i+13]);
	}
	printf("\n");

	return datalength;
}