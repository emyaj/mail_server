//
//  client.cpp
//  fileprinter
//
//  Created by Jayme Cartwright on 4/19/17.
//  Copyright © 2017 Jayme Cartwright. All rights reserved.
//

#include <stdio.h> //printf
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
    
    int sock, status;
    struct addrinfo hints, *servinfo, *p;
    char str_ip[INET6_ADDRSTRLEN];


    //initial argument check to see if all are there
    if (argc != 3) {
        fprintf(stderr,"usage: hostname or port\n");
        exit(1);
    }
    
    //creates TCP socket & if socket value < 0, writes error.
    if((sock = socket(AF_INET, SOCK_STREAM, 0))< 0){
        printf("\n Error : Could not create socket \n");
        return 1;
    }
    puts("Socket created");
    cout << "Socket created";
    
    
    //memory allocation
    memset(&hints, 0, sizeof hints); // make sure the struct is empty
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; //TCP stream socket

    //takes arguments from command line for getaddrinfo()
    if ((status = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(1);
    }
    
    
    // loop through all the results and bind to the first one we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        //socket(int domain, int type, int protocol)
        if ((sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        //tries to connect, if it cannot, closes socket & produces error
        if (connect(sock, p->ai_addr, p->ai_addrlen) == -1) {
            close(sock);
            perror("client: connect");
            continue;
        }
        break; 
    }
    
    //exiting the loop means the client failed to connect
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    
    
    //should convert IPv4 and IPv6 addresses from binary to text form, stores in var s
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), str_ip, sizeof str_ip);
    printf("client: connecting to %s\n", str_ip);
    
    freeaddrinfo(servinfo);
    
    
    
    
    
    
}






