#include <string.h>
#include <iostream>
#include <stdio.h>

void startRouter();
void startHost();


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

}
