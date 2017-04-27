//
//  rcvr.cpp
//  fileprinter
//
//  Created by Jayme Cartwright on 4/26/17.
//  Copyright © 2017 Jayme Cartwright. All rights reserved.
//

#include <stdio.h>
#include <string.h>    //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr- utility function prototypes
#include <unistd.h> // write
#include <netinet/in.h> //struct sockaddr_in; byte ordering macros
#include <sys/types.h> //prerequisite typedefs
#include <netdb.h> //nw info lookup for prototypes and structures
#include <stdlib.h> //namespace std
#include <errno.h> //names for 'erno' values (error numbers)
#include <signal.h>
#include <iostream> //namespace std


using namespace std;

void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET){
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}



int main(int argc , char *argv[]){

    struct addrinfo hints, *servinfo, *p;
    int sock, status, numbytes;
    
    //initial argument check to see if all are there
    if (argc != 3) {
        fprintf(stderr,"forgot hostname or port\n");
        exit(1);
    }
    
    
    //creates TCP socket & if socket value < 0, writes error.
    if((sock = socket(AF_INET, SOCK_DGRAM, 0))< 0){
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    cout << "Socket created\n";
    
    
    //memory allocation
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM; //UDP socket
    
    
    
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return 1;
    }
    
    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        break;
    }
    
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to create socket\n");
        return 2;
    }
    
    
    if ((numbytes = sendto(sock, argv[2], strlen(argv[2]), 0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("client: sendto");
        exit(1);
    }
    
     freeaddrinfo(servinfo);
    
    printf("client: sent %d bytes to %s\n", numbytes, argv[1]);
    close(sock);
    return 0;
    
}
